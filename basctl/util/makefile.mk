#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:57:29 $
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

PRJNAME=basctl
TARGET=basctl
#svx.hid generieren
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------

# --- Basctl - DLL ----------

.IF "$(header)" == ""

HELPIDFILES=    ..$/inc$/helpid.hrc

SHL1TARGET= basctl$(DLLPOSTFIX)
SHL1IMPLIB= basctl
SHL1BASE  = 0x1d800000
SHL1STDLIBS= \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(BASICLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(XMLSCRIPTLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(SHELLLIB)
.ENDIF # WNT


SHL1LIBS=       $(SLB)$/basctl.lib
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=basctl.map

DEF1NAME        =$(SHL1TARGET)

LIB1TARGET      =$(SLB)$/basctl.lib
LIB1FILES       = \
            $(LIBPRE) $(SLB)$/basicide.lib		\
            $(LIBPRE) $(SLB)$/dlged.lib			\
            $(LIBPRE) $(SLB)$/accessibility.lib

SRSFILELIST=\
                $(SRS)$/basicide.srs \
                $(SRS)$/dlged.srs

SRSFILELIST+=   $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=basctl
RESLIB1IMAGES =$(PRJ)$/res
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(header)" == ""

