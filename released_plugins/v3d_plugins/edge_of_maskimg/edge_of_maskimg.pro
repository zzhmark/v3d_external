# a demo program written by Hanchuan Peng
# 2009-05-30

TEMPLATE      = lib
CONFIG       += plugin warn_off 
INCLUDEPATH  += ../../basic_c_fun
HEADERS       = edge_of_maskimg.h
SOURCES       = edge_of_maskimg.cpp
SOURCES      += ../../basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(edge_of_maskimg)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = ../../v3d/plugins/Edge_of_Maskimg    #better set a subdirectory here so that the plugin will be arranged nicely 

