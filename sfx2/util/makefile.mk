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

PRJ=..

ENABLE_EXCEPTIONS=TRUE
PRJNAME=sfx2
TARGET=sfx
#sfx.hid generieren
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=  $(SLB)$/appl.lib		\
            $(SLB)$/explorer.lib	\
            $(SLB)$/doc.lib			\
            $(SLB)$/view.lib		\
            $(SLB)$/control.lib		\
            $(SLB)$/notify.lib		\
            $(SLB)$/menu.lib		\
            $(SLB)$/inet.lib		\
            $(SLB)$/toolbox.lib		\
            $(SLB)$/statbar.lib		\
            $(SLB)$/dialog.lib		\
            $(SLB)$/bastyp.lib		\
            $(SLB)$/config.lib

.IF "$(ENABLE_LAYOUT)" == "TRUE"
LIB1FILES += $(SLB)$/layout.lib
.ENDIF # ENABLE_LAYOUT == TRUE

HELPIDFILES=\
            ..\inc\sfx2\sfxsids.hrc	\
            ..\source\inc\helpid.hrc

.IF "$(GUI)"!="UNX"
LIB2TARGET= $(LB)$/$(TARGET).lib
LIB2FILES=  $(LB)$/isfx.lib
LIB2DEPN=$(SHL1TARGETN)
.ENDIF

SHL1TARGET= sfx$(DLLPOSTFIX)
SHL1IMPLIB= isfx
SHL1USE_EXPORTS=name

SHL1STDLIBS+=\
        $(FWELIB) \
        $(BASICLIB) \
        $(SVTOOLLIB) \
        $(TKLIB) \
        $(VCLLIB) \
        $(SVLLIB)	\
        $(SOTLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(SAXLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(LIBXML2LIB) \


.IF "$(GUI)"=="WNT"

SHL1STDLIBS+=\
        $(UWINAPILIB) \
        $(ADVAPI32LIB) \
        $(SHELL32LIB) \
        $(GDI32LIB) \
        $(OLE32LIB) \
        $(UUIDLIB)
.ELSE # WNT
.IF "$(OS)" == "MACOSX"
SHL1STDLIBS+= -framework Cocoa
.ENDIF # MACOSX
.ENDIF # WNT


SHL1DEPN += $(shell @$(FIND) $(SLO) -type f -name "*.OBJ" -print)

SHL1LIBS=   $(LIB1TARGET)

#SHL1OBJS=   $(SLO)$/sfxdll.obj

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME	=$(SHL1TARGET)
DEFLIB1NAME	=sfx
DEF1DES		=Sfx

SFXSRSLIST=\
        $(SRS)$/appl.srs \
        $(SRS)$/sfx.srs \
        $(SRS)$/doc.srs \
        $(SRS)$/view.srs \
        $(SRS)$/menu.srs \
        $(SRS)$/dialog.srs \
                $(SRS)$/bastyp.srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(SFXSRSLIST)

# gtk quick-starter
.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
CFLAGS+=$(PKGCONFIG_CFLAGS)

SHL3TARGET=qstart_gtk$(DLLPOSTFIX)
SHL3LIBS=$(SLB)$/quickstart.lib
SHL3DEPN=$(SHL1IMPLIBN) $(SHL1TARGETN)
# libs for gtk plugin
SHL3STDLIBS=$(SHL1STDLIBS) $(SFX2LIB) $(EGGTRAYLIB)
SHL3STDLIBS+=$(PKGCONFIG_LIBS:s/ -lpangoxft-1.0//)
# hack for faked SO environment
.IF "$(PKGCONFIG_ROOT)"!=""
SHL3SONAME+=-z nodefs
SHL3NOCHECK=TRUE
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""
.ENDIF # "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
.ENDIF # "$(GUI)"=="UNX"

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk
