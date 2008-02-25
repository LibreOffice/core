#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:07:28 $
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
