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

PRJNAME=svx
TARGET=svx
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

RSCLOCINC+=-I$(PRJ)$/source$/svdraw

HELPIDFILES=    ..$/inc$/helpid.hrc

# svxcore
SHL1TARGET= svxcore$(DLLPOSTFIX)
SHL1IMPLIB= isvxcore
SHL1USE_EXPORTS=name

SHL1OBJS= \
    $(SLO)$/coreservices.obj


SHL1LIBS= \
    $(SLB)$/animation.lib \
    $(SLB)$/attribute.lib \
    $(SLB)$/contact.lib \
    $(SLB)$/customshapes-core.lib \
    $(SLB)$/dialogs-core.lib \
    $(SLB)$/engine3d-core.lib \
    $(SLB)$/event.lib \
    $(SLB)$/fmcomp-core.lib \
    $(SLB)$/form-core.lib \
    $(SLB)$/gal.lib \
    $(SLB)$/items-core.lib \
    $(SLB)$/overlay.lib \
    $(SLB)$/primitive2d.lib \
    $(SLB)$/primitive3d.lib \
    $(SLB)$/properties.lib \
    $(SLB)$/svdraw-core.lib \
    $(SLB)$/table-core.lib \
    $(SLB)$/tbxctrls-core.lib \
    $(SLB)$/toolbars.lib \
    $(SLB)$/unodraw-core.lib \
    $(SLB)$/xml.lib \
    $(SLB)$/xout.lib


SHL1STDLIBS= \
             $(EDITENGLIB) \
             $(AVMEDIALIB) \
             $(SFX2LIB) \
             $(XMLOFFLIB) \
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
             $(SALLIB) \
             $(ICUUCLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=$(SHL1LIBS:b)

# svx
SHL2TARGET= svx$(DLLPOSTFIX)
SHL2IMPLIB= i$(TARGET)
SHL2USE_EXPORTS=name
SHL2DEPN=$(SHL1TARGETN)

SHL2LIBS= \
    $(SLB)$/svdraw.lib \
    $(SLB)$/form.lib \
    $(SLB)$/items.lib     \
    $(SLB)$/dialogs.lib	\
    $(SLB)$/mnuctrls.lib  \
    $(SLB)$/stbctrls.lib  \
    $(SLB)$/tbxctrls.lib  \
    $(SLB)$/smarttags.lib \
    $(SLB)$/unodraw.lib	\
    $(SLB)$/unogallery.lib\
    $(SLB)$/accessibility.lib	\
    $(SLB)$/customshapes.lib \
    $(SLB)$/fmcomp.lib \
    $(SLB)$/engine3d.lib \
    $(SLB)$/table.lib

.IF "(GUIBASE)" == "WIN"
SHL2LIBS+=$(SLB)$/ibrwimp.lib
.ENDIF # (WIN)

SHL2STDLIBS= \
            $(EDITENGLIB) \
            $(SVXCORELIB) \
            $(SFX2LIB) \
            $(XMLOFFLIB) \
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
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(ICUUCLIB)

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+=$(SHELLLIB)
.ENDIF # WNT

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
DEFLIB2NAME=$(SHL2LIBS:b)

.IF "$(debug)$(dbgutil)"!=""
SHL2STDLIBS += $(CPPUHELPERLIB)
.ENDIF # "$(debug)$(dbgutil)"!=""

# ------------------------------------------------------------------------------

# Resource files
SRSFILELIST=\
        $(SRS)$/svdstr.srs \
        $(SRS)$/dialogs.srs \
        $(SRS)$/drawdlgs.srs \
        $(SRS)$/mnuctrls.srs \
        $(SRS)$/stbctrls.srs \
        $(SRS)$/tbxctrls.srs \
        $(SRS)$/svxitems.srs \
        $(SRS)$/form.srs \
        $(SRS)$/fmcomp.srs \
        $(SRS)$/engine3d.srs \
        $(SRS)$/unodraw.srs \
        $(SRS)$/accessibility.srs \
        $(SRS)$/table.srs \
        $(SRS)$/toolbars.srs \
        $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=svx
RESLIB1IMAGES=$(PRJ)$/res $(PRJ)$/source/svdraw
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

