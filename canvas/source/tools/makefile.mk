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
# $Revision: 1.15 $
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

PRJNAME=canvas
TARGET=canvastools
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF
.IF "$(profiler)"!="" || "$(PROFILER)"!=""
CDEFS+= -DPROFILER
.ENDIF

#CFLAGS +:= /Ox /Ot					# THIS IS IMPORTANT



SLOFILES =	\
    $(SLO)$/cachedprimitivebase.obj \
    $(SLO)$/canvascustomspritehelper.obj \
    $(SLO)$/canvastools.obj \
    $(SLO)$/elapsedtime.obj \
    $(SLO)$/linepolypolygonbase.obj \
    $(SLO)$/parametricpolypolygon.obj \
    $(SLO)$/prioritybooster.obj \
    $(SLO)$/propertysethelper.obj \
    $(SLO)$/spriteredrawmanager.obj \
    $(SLO)$/surface.obj \
    $(SLO)$/surfaceproxy.obj \
    $(SLO)$/surfaceproxymanager.obj \
    $(SLO)$/pagemanager.obj \
    $(SLO)$/page.obj \
    $(SLO)$/verifyinput.obj

SHL1TARGET= 	$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= 	i$(TARGET)
SHL1STDLIBS=	$(SALLIB) $(CPPULIB) $(BASEGFXLIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(VCLLIB) $(TKLIB) $(TOOLSLIB)

.IF "$(ENABLE_AGG)"=="YES"
    SLOFILES += $(SLO)$/bitmap.obj \
                $(SLO)$/image.obj

    .IF "$(AGG_VERSION)"!=""
        CDEFS += -DAGG_VERSION=$(AGG_VERSION)
    .ENDIF
    SHL1STDLIBS += $(AGGLIB)
.ENDIF

SHL1LIBS=		$(SLB)$/$(TARGET).lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=Canvastools
DEFLIB1NAME	=$(TARGET)

.IF "$(GUI)" == "WNT"
SHL1STDLIBS += $(WINMMLIB) $(KERNEL32LIB)
.ENDIF


# ==========================================================================

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk $(TARGET).flt
    @$(TYPE) $(TARGET).flt > $@
