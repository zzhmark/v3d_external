
#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "newwindowplugin.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(newwindow, NewWindowPlugin);

void Invert(V3DPluginCallback &callback, QWidget *parent);
void Threshold(V3DPluginCallback &callback, QWidget *parent);
void Compute(char op, V3DPluginCallback &callback, QWidget *parent);

const QString title = "NewWindowPlugin demo";
QStringList NewWindowPlugin::menulist() const
{
    return QStringList()
    << tr("Invert Color (in current window)")
    << tr("Threshold...")
    << tr("Add 2 Images...")
    << tr("Differ 2 Images...")
    << tr("Modulate 2 Images...");
}

void NewWindowPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("Invert Color (in current window)"))
    {
    	Invert(callback, parent);
    }
    else
    if (menu_name == tr("Threshold..."))
    {
    	Threshold(callback, parent);
    }
    else
    if (menu_name == tr("Add 2 Images..."))
    {
    	Compute('+', callback, parent);
    }
    else
    if (menu_name == tr("Differ 2 Images..."))
    {
    	Compute('-', callback, parent);
    }
    else
    if (menu_name == tr("Modulate 2 Images..."))
    {
    	Compute('*', callback, parent);
    }
}

void Invert(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* image = callback.getImage(oldwin);
	if (! image)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (image->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}
	
	Image4DProxy<Image4DSimple> p(image);
	for (uint8* ip=p.begin(); ip<=p.end(); ip++)
	{
		*ip = 255 - *ip;
	}

	callback.setImageName(oldwin, callback.getImageName(oldwin)+"_invert");
	callback.updateImageWindow(oldwin);
}

void Threshold(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* image = callback.getImage(oldwin);
	if (! image)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (image->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}
	
	bool ok;
    int threshold = QInputDialog::getInteger(parent, QObject::tr("Threshold"),
                                             QObject::tr("Enter threshold:"),
                                             100, 0, 255, 1, &ok);
    if (! ok) return;

	V3DLONG N = image->getTotalBytes();
	unsigned char* newdata1d = new unsigned char[N];
	Image4DSimple tmp;
	tmp.setData(newdata1d, image->sz0,image->sz1,image->sz2,image->sz3, image->datatype);

	Image4DProxy<Image4DSimple> p0(image);
	Image4DProxy<Image4DSimple> p(&tmp);
	uint8 *ip0, *ip;
	for (ip0=p0.begin(), ip=p.begin();
		ip<=p.end(); ip0++, ip++)
    {
		if (*ip0 <= threshold)  *ip = 0;
		else *ip = *ip0;
	}

	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &tmp);
	callback.setImageName(newwin, callback.getImageName(oldwin)+"_threshold");
    callback.updateImageWindow(newwin);
}

void Compute(char op, V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();
	if (win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("Need at least 1 images."));
		return;
	}

	QStringList items;
	for (int i=0; i<win_list.size(); i++) items << callback.getImageName(win_list[i]);

	QDialog d(parent);
	QComboBox* combo1 = new QComboBox(); combo1->addItems(items);
	QComboBox* combo2 = new QComboBox(); combo2->addItems(items);
	QPushButton* ok     = new QPushButton("OK");
	QPushButton* cancel = new QPushButton("Cancel");
	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow(QObject::tr("image1: "), combo1);
	formLayout->addRow(QObject::tr("image2: "), combo2);
	formLayout->addRow(ok, cancel);
	d.setLayout(formLayout);
	d.setWindowTitle(QString("image1 %1 image2").arg(op));

	d.connect(ok,     SIGNAL(clicked()), &d, SLOT(accept()));
	d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));
	if (d.exec()!=QDialog::Accepted)
		return;

	int i1 = combo1->currentIndex();
	int i2 = combo2->currentIndex();

	Image4DSimple* image1 = callback.getImage(win_list[i1]);
	Image4DSimple* image2 = callback.getImage(win_list[i2]);
	
	
	if (!image1 || !image2)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (image1->getDatatype()!=V3D_UINT8 || image2->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}
	

	V3DLONG N1 = image1->getTotalBytes();
	unsigned char* newdata1d = new unsigned char[N1];

	Image4DSimple tmp;
	tmp.setData(newdata1d, image1->sz0,image1->sz1,image1->sz2,image1->sz3, image1->datatype);

	Image4DProxy<Image4DSimple> p1(image1);
	Image4DProxy<Image4DSimple> p2(image2);
	Image4DProxy<Image4DSimple> p(&tmp);

	Image4DProxy_foreach(p, x,y,z,c)
	{
		float f = 0;
		float f1 = 0;
		float f2 = 0;
		if (p1.is_inner(x,y,z,c)) f1 = (*p1.at_uint8(x,y,z,c))/255.f;
		if (p2.is_inner(x,y,z,c)) f2 = (*p2.at_uint8(x,y,z,c))/255.f;

		switch (op)
		{
		case '+': f = f1 + f2; break;
		case '-': f = f1 - f2; break;
		case '*': f = f1 * f2; break;
		}
		f = fabs(f);
		if (f>1) f = 1;

		*p.at_uint8(x,y,z,c) = (unsigned char)(f*255);
	}


	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &tmp);
	callback.setImageName(newwin, "new_image_arithmetic");
    callback.updateImageWindow(newwin);
}
