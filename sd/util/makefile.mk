#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: rt $ $Date: 2004-07-13 15:04:18 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

PRJNAME=sd
TARGET=sdraw3
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Resources ----------------------------------------------------

RESLIB1NAME=sd
RESLIB1IMAGES=$(PRJ)$/res/imagelst $(PRJ)$/res 
RESLIB1SRSFILES=\
    $(SRS)$/app.srs				\
    $(SRS)$/dlg.srs				\
    $(SRS)$/core.srs			\
    $(SRS)$/html.srs			\
    $(SRS)$/sdslots.srs			\
    $(SRS)$/accessibility.srs		\
    $(SRS)$/notes.srs			\
    $(SOLARCOMMONRESDIR)$/sfx.srs

# --- StarDraw DLL

SHL1TARGET= sd$(UPD)$(DLLPOSTFIX)
#SHL1VERSIONMAP= sd.map
SHL1IMPLIB= sdi


# on MacOSX static libs must be at end of libraries to link with
.IF "$(OS)" != "MACOSX"
# static libraries
SHL1STDLIBS= $(SCHLIB)
.ELSE
SHL1STDLIBS=
.ENDIF # MACOSX


# dynamic libraries
SHL1STDLIBS+= \
    $(SVXLIB) \
    $(SFXLIB) \
    $(BASICLIB) \
    $(BASEGFXLIB) \
    $(GOODIESLIB) \
    $(SO2LIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB) \
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(VOSLIB) \
    $(CANVASLIB) \
    $(SALLIB)


# on MacOSX static libs must be at end of libraries to link with
.IF "$(OS)" == "MACOSX"
# add back in static libraries
SHL1STDLIBS+= $(SCHLIB)
.ENDIF # MACOSX



SHL1DEPN=   $(L)$/itools.lib
SHL1LIBS=   $(LIB3TARGET)
SHL1DEPN+=	makefile.mk


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEF1NAME	=$(SHL1TARGET)
DEFLIB1NAME = $(TARGET)

.IF "$(GUI)" == "WNT"
SHL1RES=    $(RCTARGET)
.ENDIF

# --- Linken der Applikation ---------------------------------------

LIB2TARGET=$(SLB)$/sdmod.lib
LIB2OBJFILES=   \
            $(SLO)$/sdmod1.obj      \
            $(SLO)$/sdmod2.obj      \
            $(SLO)$/sdmod.obj

LIB3TARGET=$(SLB)$/sdraw3.lib
LIB3FILES=      \
            $(SLB)$/view.lib        \
            $(SLB)$/app.lib			\
            $(SLB)$/func.lib        \
            $(SLB)$/docshell.lib    \
            $(SLB)$/dlg.lib			\
            $(SLB)$/core.lib		\
            $(SLB)$/xml.lib			\
            $(SLB)$/cgm.lib			\
            $(SLB)$/grf.lib			\
            $(SLB)$/bin.lib			\
            $(SLB)$/html.lib		\
            $(SLB)$/filter.lib		\
            $(SLB)$/unoidl.lib		\
            $(SLB)$/accessibility.lib	\
            $(SLB)$/toolpanel.lib		\
            $(SLB)$/tpcontrols.lib		\
            $(SLB)$/slsshell.lib		\
            $(SLB)$/slsmodel.lib		\
            $(SLB)$/slsview.lib			\
            $(SLB)$/slscontroller.lib	\
            $(SLB)$/slscache.lib		\
            $(SLB)$/notes.lib		


SHL2TARGET= sdd$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= sddimp
SHL2VERSIONMAP= sdd.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVXLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(TOOLSLIB) \
            $(UCBHELPERLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2OBJS=   $(SLO)$/sddetect.obj \
        $(SLO)$/detreg.obj
SHL2DEPN+=	makefile.mk

# add for sdui
SHL4TARGET= sdui$(UPD)$(DLLPOSTFIX)
SHL4IMPLIB= sduiimp
SHL4VERSIONMAP= sdui.map
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=       $(SHL4TARGET)
SHL4LIBS=   $(SLB)$/sdui_all.lib

LIB4TARGET = $(SLB)$/sdui_all.lib
LIB4OBJFILES= $(SLO)$/pubdlg.obj
LIB4FILES = $(SLB)$/sdui.lib

SHL4STDLIBS= \
        $(ISDLIB) \
    $(SVXLIB) \
    $(SFXLIB) \
    $(BASICLIB) \
    $(BASEGFXLIB) \
    $(GOODIESLIB) \
    $(SO2LIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB) \
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(VOSLIB) \
    $(CANVASLIB) \
    $(SALLIB)
            





# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk
    
$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @+$(TYPE) sd.flt > $@


