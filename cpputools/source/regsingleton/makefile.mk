#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
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
TARGET=regsingleton
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

UNOUCRDEP=$(BUILDSOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(BUILDSOLARBINDIR)$/udkapi.rdb

NO_OFFUH=TRUE
CPPUMAKERFLAGS+= -C

UNOTYPES= \
     com.sun.star.uno.TypeClass \
     com.sun.star.uno.XAggregation \
     com.sun.star.uno.XWeak \
     com.sun.star.uno.XComponentContext \
     com.sun.star.lang.XTypeProvider \
     com.sun.star.lang.XComponent \
     com.sun.star.lang.XSingleServiceFactory \
     com.sun.star.lang.XSingleComponentFactory \
     com.sun.star.lang.XMultiServiceFactory \
     com.sun.star.lang.XMultiComponentFactory \
     com.sun.star.container.XHierarchicalNameAccess \
    com.sun.star.registry.XSimpleRegistry \
    com.sun.star.registry.XRegistryKey

# --- Files --------------------------------------------------------

DEPOBJFILES= \
    $(OBJ)$/regsingleton.obj 

APP1TARGET=$(TARGET)
APP1OBJS=$(DEPOBJFILES)  

APP1STDLIBS= \
    $(SALLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)

.IF "$(debug)" != ""
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

.INCLUDE :  target.mk

