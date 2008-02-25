#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:59:32 $
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
PRJNAME=avmedia
TARGET=avmediaQuickTime

.IF "$(GUIBASE)"=="aqua"

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

CFLAGSCXX += -x objective-c++ -fobjc-exceptions

SLOFILES= \
        $(SLO)$/quicktimeuno.obj  \
        $(SLO)$/manager.obj       \
        $(SLO)$/window.obj        \
        $(SLO)$/framegrabber.obj        \
        $(SLO)$/player.obj

EXCEPTIONSFILES= \
        $(SLO)$/quicktimeuno.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
             $(CPPULIB) \
             $(SALLIB)  \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(TOOLSLIB) \
             $(VCLLIB) 

SHL1STDLIBS+= \
             -framework Cocoa \
             -framework QTKit \
             -framework QuickTime

# build DLL
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP= $(TARGET).map

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

.ELSE
dummy:
    @echo " Nothing to build for GUIBASE=$(GUIBASE)"
.ENDIF
