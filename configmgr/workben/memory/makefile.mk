#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:43:18 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..
PRJINC=$(PRJ)$/source

PRJNAME=configmgr

TARGET=memorytest
TARGET2=memorytesthack
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# ... common for all test executables ..............................
APPSTDLIBS=\
            $(SALLIB) \
            $(VOSLIB) \
            $(CPPULIB)	\
            $(CPPUHELPERLIB)	\
            $(COMPHELPERLIB)	\

# ... cfgapi ..............................
# APP1STDLIBS = $(APPSTDLIBS)
# 
# APP1STDLIBS+=$(STDLIBCPP)
# 
# APP1TARGET= $(TARGET)
# APP1OBJS=	\
# 	  $(SLO)$/cfgapi2.obj	\
# 	  $(SLO)$/strimpl.obj \
# 	  $(SLO)$/typeconverter.obj \
# 	  $(SLO)$/simpletypehelper.obj \
# 	  $(SLO)$/memory.obj \

# ... cfgapi ..............................
APP2STDLIBS = $(APPSTDLIBS)

APP2STDLIBS+= $(STDLIBCPP)

# CFLAGS+=-DWITHOUTAPI

APP2TARGET= $(TARGET2)
APP2OBJS=	\
    $(SLO)$/memorytests.obj	\
    $(SLO)$/testmodules.obj	\
    $(SLO)$/treeload.obj \
    $(SLO)$/accessimpl.obj \
    $(SLO)$/adminproviderimpl.obj \
    $(SLO)$/apiaccessobj.obj \
    $(SLO)$/apifactory.obj \
    $(SLO)$/apifactoryimpl.obj \
    $(SLO)$/apinodeaccess.obj \
    $(SLO)$/apinodeupdate.obj \
    $(SLO)$/apinotifierimpl.obj \
    $(SLO)$/apiserviceinfo.obj \
    $(SLO)$/apitreeaccess.obj \
    $(SLO)$/apitreeimplobj.obj \
    $(SLO)$/attributelist.obj \
    $(SLO)$/attributeparser.obj \
    $(SLO)$/binarybasereader.obj \
    $(SLO)$/binarydecide.obj \
    $(SLO)$/binaryreader.obj \
    $(SLO)$/binarywritehandler.obj \
    $(SLO)$/binarywriter.obj \
    $(SLO)$/bootstrap.obj \
    $(SLO)$/broadcaster.obj \
    $(SLO)$/cachewritescheduler.obj \
    $(SLO)$/changes.obj \
    $(SLO)$/cmtree.obj \
    $(SLO)$/cmtreemodel.obj \
    $(SLO)$/collectchanges.obj \
    $(SLO)$/committer.obj \
    $(SLO)$/confeventhelpers.obj \
    $(SLO)$/confevents.obj \
    $(SLO)$/configexcept.obj \
    $(SLO)$/configgroup.obj \
    $(SLO)$/confignotifier.obj \
    $(SLO)$/configpath.obj \
    $(SLO)$/configsession.obj \
    $(SLO)$/configset.obj \
    $(SLO)$/confname.obj \
    $(SLO)$/confprovider2.obj \
    $(SLO)$/confproviderimpl2.obj \
    $(SLO)$/confsvccomponent.obj \
    $(SLO)$/disposetimer.obj \
    $(SLO)$/elementaccess.obj \
    $(SLO)$/elementimpl.obj \
    $(SLO)$/encodename.obj \
    $(SLO)$/filehelper.obj \
    $(SLO)$/generatecache.obj \
    $(SLO)$/groupaccess.obj \
    $(SLO)$/groupimpl.obj \
    $(SLO)$/groupobjects.obj \
    $(SLO)$/groupupdate.obj \
    $(SLO)$/invalidatetree.obj \
    $(SLO)$/listenercontainer.obj \
    $(SLO)$/loader.obj \
    $(SLO)$/localizednodebuilder.obj \
    $(SLO)$/localizedtreeactions.obj \
    $(SLO)$/localsession.obj \
    $(SLO)$/matchlocale.obj \
    $(SLO)$/mergechange.obj \
    $(SLO)$/namehelper.obj \
    $(SLO)$/nodechange.obj \
    $(SLO)$/nodechangeimpl.obj \
    $(SLO)$/nodechangeinfo.obj \
    $(SLO)$/nodefactory.obj \
    $(SLO)$/nodeimpl.obj \
    $(SLO)$/nodeimplobj.obj \
    $(SLO)$/noderef.obj \
    $(SLO)$/notifycallback.obj \
    $(SLO)$/oslstream.obj \
    $(SLO)$/pathhelper.obj \
    $(SLO)$/portalstream.obj \
    $(SLO)$/propertiesfilterednotifier.obj \
    $(SLO)$/propertyinfohelper.obj \
    $(SLO)$/propertysetaccess.obj \
    $(SLO)$/propsetaccessimpl.obj \
    $(SLO)$/provider.obj \
    $(SLO)$/providerfactory.obj \
    $(SLO)$/providerimpl.obj \
    $(SLO)$/receivethread.obj \
    $(SLO)$/redirector.obj \
    $(SLO)$/remotesession.obj \
    $(SLO)$/roottree.obj \
    $(SLO)$/saxadapter.obj \
    $(SLO)$/saxtools.obj \
    $(SLO)$/sessionfactory.obj \
    $(SLO)$/sessionstream.obj \
    $(SLO)$/setaccess.obj \
    $(SLO)$/setnodeimpl.obj \
    $(SLO)$/setobjects.obj \
    $(SLO)$/setupdate.obj \
    $(SLO)$/simpletypehelper.obj \
    $(SLO)$/socketstream.obj \
    $(SLO)$/strimpl.obj \
    $(SLO)$/synchronize.obj \
    $(SLO)$/template.obj \
    $(SLO)$/templateimpl.obj \
    $(SLO)$/timestamp.obj \
    $(SLO)$/tracer.obj \
    $(SLO)$/translatechanges.obj \
    $(SLO)$/treeactions.obj \
    $(SLO)$/treebuildercallback.obj \
    $(SLO)$/treecache.obj \
    $(SLO)$/treedata.obj \
    $(SLO)$/treeimpl.obj \
    $(SLO)$/treeiterators.obj \
    $(SLO)$/treenodefactory.obj \
    $(SLO)$/trivialbufferedfile.obj \
    $(SLO)$/typeconverter.obj \
    $(SLO)$/updatehandler.obj \
    $(SLO)$/updateimpl.obj \
    $(SLO)$/updatetree.obj \
    $(SLO)$/userimpl.obj \
    $(SLO)$/valueconverter.obj \
    $(SLO)$/valuehandler.obj \
    $(SLO)$/valuenodebuilder.obj \
    $(SLO)$/writesubtreeasbinaryhandler.obj \
    $(SLO)$/xmlformater.obj \
    $(SLO)$/xmltreebuilder.obj \
    $(SLO)$/valuemembernode.obj \
    $(SLO)$/anypair.obj \

#	$(SLO)$/memory.obj \


# 	  $(SLO)$/strimpl.obj \
# 	  $(SLO)$/typeconverter.obj \
# 	  $(SLO)$/simpletypehelper.obj \
# 	  $(SLO)$/memory.obj \
# 	  $(SLO)$/loadwithtreemanager.obj \
# 	  $(SLO)$/localsession.obj \
# 	  $(SLO)$/configsession.obj \
# 	  $(SLO)$/confname.obj \
# 	  $(SLO)$/tracer.obj \
# 	  $(SLO)$/generatecache.obj \
# 	  $(SLO)$/treeactions.obj \
# 	  $(SLO)$/cmtreemodel.obj \
# 	  $(SLO)$/cmtree.obj \
# 	  $(SLO)$/xmltreebuilder.obj \
# 	  $(SLO)$/xmlformater.obj \
# 	  $(SLO)$/binaryreader.obj \
# 	  $(SLO)$/binarybasereader.obj \
# 	  $(SLO)$/binarywriter.obj \
# 	  $(SLO)$/binarywritehandler.obj \
# 	  $(SLO)$/binarydecide.obj \
# 	  $(SLO)$/writesubtreeasbinaryhandler.obj \
# 	  $(SLO)$/oslstream.obj \
# 	  $(SLO)$/pathhelper.obj \
# 	  $(SLO)$/treecache.obj \
# 	  $(SLO)$/updatetree.obj \
# 	  $(SLO)$/attributeparser.obj \
# 	  $(SLO)$/updatehandler.obj \
# 	  $(SLO)$/synchronize.obj \
# 	  $(SLO)$/filehelper.obj \
# 	  $(SLO)$/mergechange.obj \
# 	  $(SLO)$/providerimpl.obj \
# 	  $(SLO)$/bootstrap.obj \
# 	  $(SLO)$/matchlocale.obj \
# 	  $(SLO)$/changes.obj \
# 	  $(SLO)$/configexcept.obj \
# 	  $(SLO)$/valuehandler.obj \
# 	  $(SLO)$/treenodefactory.obj \
# 	  $(SLO)$/attributelist.obj \
# 	  $(SLO)$/namehelper.obj \
# 	  $(SLO)$/localizedtreeactions.obj \
# 	  $(SLO)$/trivialbufferedfile.obj \
# 	  $(SLO)$/cachewritescheduler.obj \
# 	  $(SLO)$/disposetimer.obj \
# 	  $(SLO)$/noderef.obj \
# 	  $(SLO)$/treedata.obj \
# 	  $(SLO)$/confevents.obj \
# 	  $(SLO)$/treebuildercallback.obj \
# 	  $(SLO)$/loader.obj \
# 	  $(SLO)$/apifactoryimpl.obj \
# 	  $(SLO)$/apitreeimplobj.obj \

#	$(SLO)$/.obj \


.INCLUDE :  target.mk


