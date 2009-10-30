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
# $Revision: 1.61 $
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

PRJNAME=svx
TARGET=svx
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)"=="IRIX"
LINKFLAGS+=-Wl,-LD_LAYOUT:lgot_buffer=30
.ENDIF

RSCLOCINC+=-I$(PRJ)$/source$/svdraw

# --- Svx - DLL ----------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/svdraw.lib \
    $(SLB)$/form.lib

.IF "(GUIBASE)" == "WIN"
LIB1FILES+=$(SLB)$/ibrwimp.lib
.ENDIF # (WIN)

LIB2TARGET= $(SLB)$/$(TARGET)_2.lib
LIB2FILES=\
    $(SLB)$/items.lib     \
    $(SLB)$/dialogs.lib	\
    $(SLB)$/mnuctrls.lib  \
    $(SLB)$/options.lib   \
    $(SLB)$/stbctrls.lib  \
    $(SLB)$/tbxctrls.lib  \
    $(SLB)$/unoedit.lib   \
    $(SLB)$/smarttags.lib

LIB3TARGET= $(SLB)$/$(TARGET)_3.lib
LIB3FILES=\
    $(SLB)$/unodraw.lib	\
    $(SLB)$/unogallery.lib\
    $(SLB)$/accessibility.lib	\
    $(SLB)$/customshapes.lib

LIB4TARGET= $(SLB)$/$(TARGET)_4.lib
LIB4FILES=\
    $(SLB)$/fmcomp.lib \
    $(SLB)$/engine3d.lib \
    $(SLB)$/table.lib

# Objects needed for the svxcore library.
LIB5TARGET= $(SLB)$/$(TARGET)_5.lib
LIB5FILES=\
    $(SLB)$/accessibility-core.lib \
    $(SLB)$/animation.lib \
    $(SLB)$/attribute.lib \
    $(SLB)$/contact.lib \
    $(SLB)$/customshapes-core.lib \
    $(SLB)$/dialogs-core.lib \
    $(SLB)$/editeng.lib \
    $(SLB)$/engine3d-core.lib \
    $(SLB)$/event.lib \
    $(SLB)$/fmcomp-core.lib \
    $(SLB)$/form-core.lib \
    $(SLB)$/gal.lib \
    $(SLB)$/items-core.lib \
    $(SLB)$/options-core.lib

LIB6TARGET= $(SLB)$/$(TARGET)_6.lib
LIB6FILES=\
    $(SLB)$/outliner.lib \
    $(SLB)$/overlay.lib \
    $(SLB)$/primitive2d.lib \
    $(SLB)$/primitive3d.lib \
    $(SLB)$/properties.lib \
    $(SLB)$/svdraw-core.lib \
    $(SLB)$/svxlink.lib \
    $(SLB)$/svxrtf-core.lib \
    $(SLB)$/svxrtf.lib \
    $(SLB)$/table-core.lib \
    $(SLB)$/tbxctrls-core.lib  \
    $(SLB)$/toolbars.lib \
    $(SLB)$/unodraw-core.lib \
    $(SLB)$/unoedit-core.lib   \
    $(SLB)$/xml.lib \
    $(SLB)$/xout.lib

HELPIDFILES=    ..$/inc$/helpid.hrc

# svxcore
SHL1TARGET= svxcore$(DLLPOSTFIX)
SHL1IMPLIB= isvxcore
SHL1USE_EXPORTS=name
SHL1LIBS= $(LIB5TARGET) $(LIB6TARGET)

SHL1STDLIBS= \
             $(AVMEDIALIB) \
             $(SFX2LIB) \
             $(XMLOFFLIB) \
             $(GOODIESLIB) \
             $(BASEGFXLIB) \
             $(DRAWINGLAYERLIB) \
             $(LNGLIB) \
             $(BASICLIB) \
             $(SVTOOLLIB) \
             $(TKLIB) \
             $(VCLLIB) \
             $(SVLLIB) \
             $(SOTLIB) \
             $(UNOTOOLSLIB) \
             $(TOOLSLIB) \
             $(I18NISOLANGLIB) \
             $(I18NPAPERLIB) \
             $(COMPHELPERLIB) \
             $(UCBHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(CPPULIB) \
             $(VOSLIB) \
             $(SALLIB) \
             $(ICUUCLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)_5 $(TARGET)_6

# svx
SHL2TARGET= svx$(DLLPOSTFIX)
SHL2IMPLIB= i$(TARGET)
SHL2USE_EXPORTS=name
SHL2DEPN=$(SHL1TARGETN)

SHL2LIBS= $(LIB1TARGET) $(LIB2TARGET) $(LIB3TARGET) $(LIB4TARGET)
SHL2STDLIBS= \
            $(SVXCORELIB) \
            $(SFX2LIB) \
            $(XMLOFFLIB) \
            $(GOODIESLIB) \
            $(BASEGFXLIB) \
            $(DRAWINGLAYERLIB) \
            $(BASICLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(I18NISOLANGLIB) \
            $(XMLSCRIPTLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB)	\
            $(CPPUHELPERLIB)	\
            $(CPPULIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(ICUUCLIB)

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+=$(SHELLLIB)
.ENDIF # WNT

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
DEFLIB2NAME=$(TARGET) $(TARGET)_2 $(TARGET)_3 $(TARGET)_4

.IF "$(debug)$(dbgutil)"!=""
SHL2STDLIBS += $(CPPUHELPERLIB)
.ENDIF # "$(debug)$(dbgutil)"!=""

# ------------------------------------------------------------------------------

# Resource files
SRSFILELIST=\
        $(SRS)$/svdstr.srs \
        $(SRS)$/editeng.srs \
        $(SRS)$/outliner.srs \
        $(SRS)$/dialogs.srs \
        $(SRS)$/drawdlgs.srs \
        $(SRS)$/mnuctrls.srs \
        $(SRS)$/stbctrls.srs \
        $(SRS)$/tbxctrls.srs \
        $(SRS)$/options.srs \
        $(SRS)$/svxitems.srs \
        $(SRS)$/form.srs \
        $(SRS)$/fmcomp.srs \
        $(SRS)$/engine3d.srs \
        $(SRS)$/unodraw.srs \
        $(SRS)$/svxlink.srs \
        $(SRS)$/accessibility.srs \
        $(SRS)$/table.srs \
        $(SRS)$/toolbars.srs \
        $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=svx
RESLIB1IMAGES=$(PRJ)$/res $(PRJ)$/source/svdraw
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

