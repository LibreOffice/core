#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: ihi $ $Date: 2008-02-04 15:48:00 $
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

PRJNAME=desktop
TARGET=spl
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/splash.obj \
            $(SLO)$/firststart.obj \
            $(SLO)$/services_spl.obj

SHL1DEPN=   makefile.mk
SHL1OBJS=   $(SLOFILES) \
            $(SLO)$/pages.obj \
            $(SLO)$/wizard.obj \
            $(SLO)$/migration.obj \
            $(SLO)$/cfgfilter.obj


SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(VCLLIB)			\
    $(SVLLIB)           \
    $(SVTOOLLIB)        \
    $(COMPHELPERLIB)    \
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(VOSLIB)			\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)           \
    $(SFXLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(SLO)$/splash.obj : $(INCCOM)$/introbmpnames.hxx

.INCLUDE .IGNORE : $(MISC)$/intro_bmp_names.mk

.IF "$(INTO_BITMAPS:f)"!="$(LASTTIME_INTRO_BITMAPS)"
DO_PHONY=.PHONY
.ENDIF			# "$(INTRO_BITMAPS:f)"!="$(LASTTIME_INTRO_BITMAPS)"

$(INCCOM)$/introbmpnames.hxx $(DO_PHONY):
    echo const char INTRO_BITMAP_STRINGLIST[]=$(EMQ)"$(INTRO_BITMAPS:f:t",")$(EMQ)"$(EMQ); > $@
    echo LASTTIME_INTRO_BITMAPS=$(INTRO_BITMAPS:f) > $(MISC)$/intro_bmp_names.mk
