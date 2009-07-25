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
# $Revision: 1.20 $
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

PRJNAME=starmath
TARGET=smath3
LIBTARGET=NO
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

RESLIB1NAME=sm
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=\
    $(SRS)$/smres.srs \
    $(SOLARCOMMONRESDIR)$/sfx.srs

SHL1TARGET= sm$(DLLPOSTFIX)
SHL1IMPLIB= smimp

SHL1VERSIONMAP= sm.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

SHL1STDLIBS= \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(XMLOFFLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB)	\
            $(I18NPAPERLIB)     \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2TARGET= smd$(DLLPOSTFIX)
SHL2IMPLIB= smdimp
SHL2VERSIONMAP= sm.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(TOOLSLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2DEPN=	makefile.mk
SHL2VERSIONMAP= smd.map
SHL2OBJS=   $(SLO)$/smdetect.obj \
            $(SLO)$/detreg.obj \
            $(SLO)$/eqnolefilehdr.obj

SHL1DEPN=	makefile.mk
SHL1LIBS=   $(SLB)$/starmath.lib

.IF "$(GUI)" != "UNX"
.IF "$(GUI)$(COM)" != "WNTGCC"
SHL1OBJS=   $(SLO)$/smdll.obj
.ENDIF
.ENDIF # ! UNX

.IF "$(GUI)" == "WNT"
SHL1RES=	$(RCTARGET)
.ENDIF # WNT

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

