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
PRJINC=$(PRJ)$/source

PRJNAME=configmgr

TARGET=cfgapi
TARGET2=cfgreg
TARGET3=cfgadduser
TARGET4=cfgadmin
TARGET5=cfgupdate
TARGET6=cfgapi_timetest
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
            $(CPPUHELPERLIB)

# ... cfgapi ..............................
APP1STDLIBS = $(APPSTDLIBS)

APP1STDLIBS+=$(STDLIBCPP)

APP1TARGET= $(TARGET)
APP1OBJS=	\
    $(SLO)$/cfgapi.obj	\
    $(SLO)$/strimpl.obj \
    $(SLO)$/typeconverter.obj \
    $(SLO)$/simpletypehelper.obj \

# ... cfgapi_timetest ..............................
APP6STDLIBS = $(APPSTDLIBS)

APP6STDLIBS+=$(STDLIBCPP)

APP6TARGET= $(TARGET6)
APP6OBJS=	\
    $(SLO)$/cfgapi_timetest.obj	\
    $(SLO)$/strimpl.obj \
    $(SLO)$/typeconverter.obj \
    $(SLO)$/simpletypehelper.obj \

# ... cfgreg ..............................
APP2STDLIBS = $(APPSTDLIBS)

APP2STDLIBS+=$(STDLIBCPP)

APP2TARGET= $(TARGET2)
APP2OBJS=	\
    $(SLO)$/cfgregistry.obj	\

# ... cfgadduser ..............................
APP3STDLIBS = $(APPSTDLIBS)

APP3STDLIBS+=$(STDLIBCPP)

APP3TARGET= $(TARGET3)
APP3OBJS=	\
    $(SLO)$/cfgadduser.obj	\

# ... cfgadmin ..............................
APP4STDLIBS = $(APPSTDLIBS)

APP4STDLIBS+=$(STDLIBCPP)

APP4TARGET= $(TARGET4)
APP4OBJS=	\
    $(SLO)$/cfgadmin.obj	\
    $(SLO)$/strimpl.obj \
    $(SLO)$/typeconverter.obj \
    $(SLO)$/simpletypehelper.obj \

# ... cfgupdate ..............................
APP5STDLIBS = $(APPSTDLIBS)

APP5STDLIBS+=$(STDLIBCPP)

APP5TARGET= $(TARGET5)
APP5OBJS=	\
    $(SLO)$/cfgupdate.obj	\
    $(SLO)$/strimpl.obj \
    $(SLO)$/typeconverter.obj \
    $(SLO)$/simpletypehelper.obj \

.INCLUDE :  target.mk


