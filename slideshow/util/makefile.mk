#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 15:45:55 $
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

PRJ=..

PRJNAME=slideshow
TARGET=slideshow
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

SHL1DLLPRE=
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/transitions.lib	\
    $(SLB)$/activities.lib	\
    $(SLB)$/animationnodes.lib	\
    $(SLB)$/shapes.lib \
    $(SLB)$/slide.lib \
    $(SLB)$/engine.lib

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(TOOLSLIB) \
             $(CPPULIB) \
             $(SALLIB) \
             $(VCLLIB) \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(BASEGFXLIB) \
             $(CANVASTOOLSLIB) \
             $(CPPCANVASLIB) \
             $(UNOTOOLSLIB) \
             $(GOODIESLIB) \
             $(TKLIB) \
             $(AVMEDIALIB)

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# ==========================================================================

TARGET2=slideshowtest
LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=$(LIB1FILES)

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)
SHL2IMPLIB=i$(TARGET2)
SHL2STDLIBS= $(SHL1STDLIBS)

SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2LIBS=$(SLB)$/$(TARGET2).lib

DEF2NAME	=$(SHL2TARGET)
DEF2LIBNAME = $(TARGET2)
DEF2DEPN	=$(MISC)$/$(SHL2TARGET).flt

DEF2DES		=SlideShowTest

# ==========================================================================

.INCLUDE :	target.mk

$(MISC)$/$(SHL2TARGET).flt: makefile.mk \
                            $(TARGET2).flt
    +$(TYPE) $(TARGET2).flt > $@

