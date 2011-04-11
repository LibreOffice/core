#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
PRJ=..$/..

PRJNAME=cpputools
TARGET=uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

NO_OFFUH=TRUE
CPPUMAKERFLAGS+= -C

UNOTYPES= \
     com.sun.star.uno.TypeClass \
     com.sun.star.uno.XAggregation \
     com.sun.star.uno.XWeak \
     com.sun.star.uno.XComponentContext \
     com.sun.star.lang.XTypeProvider \
    com.sun.star.lang.XMain \
     com.sun.star.lang.XInitialization \
     com.sun.star.lang.XComponent \
     com.sun.star.lang.XSingleServiceFactory \
     com.sun.star.lang.XSingleComponentFactory \
     com.sun.star.lang.XMultiServiceFactory \
     com.sun.star.lang.XMultiComponentFactory \
     com.sun.star.container.XSet \
     com.sun.star.container.XHierarchicalNameAccess \
    com.sun.star.loader.XImplementationLoader \
    com.sun.star.registry.XSimpleRegistry \
    com.sun.star.registry.XRegistryKey \
    com.sun.star.connection.XAcceptor \
    com.sun.star.connection.XConnection \
    com.sun.star.bridge.XBridgeFactory \
    com.sun.star.bridge.XBridge

# --- Files --------------------------------------------------------

DEPOBJFILES=$(OBJ)$/unoexe.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(DEPOBJFILES)  
APP1RPATH=UREBIN

# Include all relevant (see ure/source/README) dynamic libraries, so that C++
# UNO components running in the uno executable have a defined environment
# (stlport, unxlngi6 libstdc++.so.6, and wntmsci10 uwinapi.dll are already
# included via APP1STDLIB, unxlngi6 libgcc_s.so.1 and wntmsci10 msvcr71.dll and
# msvcp71.dll are magic---TODO):
APP1STDLIBS= \
    $(SALLIB)		\
    $(SALHELPERLIB) \
    $(CPPULIB)		\
    $(CPPUHELPERLIB)\
    $(LIBXML2LIB)

.INCLUDE :  target.mk

