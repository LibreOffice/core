#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: vg $ $Date: 2007-12-07 11:43:59 $
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
PRJNAME=avmediaxine
TARGET=avmediaxine

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

.IF "$(GUI)" == "UNX"  && "$(GUIBASE)"!="aqua"

SLOFILES= \
        $(SLO)$/xineuno.obj      \
        $(SLO)$/manager.obj     \
        $(SLO)$/window.obj      \
        $(SLO)$/player.obj

EXCEPTIONSFILES= \
        $(SLO)$/xineuno.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS= $(CPPULIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB)
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

.INCLUDE :  	target.mk
