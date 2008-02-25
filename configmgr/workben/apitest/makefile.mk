#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:42:53 $
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


