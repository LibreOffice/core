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

PRJNAME=editeng
TARGET=editeng
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# -------------

HELPIDFILES=    ..$/inc$/helpid.hrc

# editeng
SHL1TARGET= editeng$(DLLPOSTFIX)
SHL1IMPLIB= iediteng
SHL1USE_EXPORTS=name
SHL1LIBS= \
    $(SLB)$/items.lib     \
    $(SLB)$/misc.lib	\
    $(SLB)$/rtf.lib	\
    $(SLB)$/uno.lib	\
    $(SLB)$/accessibility.lib	\
    $(SLB)$/editeng.lib \
    $(SLB)$/outliner.lib \
    $(SLB)$/xml.lib 

SHL1STDLIBS= \
             $(XMLOFFLIB) \
             $(BASEGFXLIB) \
             $(LNGLIB) \
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
             $(SALHELPERLIB) \
             $(ICUUCLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# ------------------------------------------------------------------------------

# Resource files
SRSFILELIST=\
        $(SRS)$/editeng.srs \
        $(SRS)$/outliner.srs \
        $(SRS)$/misc.srs \
        $(SRS)$/items.srs

RESLIB1NAME=editeng
RESLIB1IMAGES=$(PRJ)$/res $(PRJ)$/source/svdraw
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

