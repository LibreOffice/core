#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.53 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 12:05:59 $
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

PRJNAME=sd
TARGET=sdraw3
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
USE_DEFFILE=TRUE

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
    $(SRS)$/accessibility.srs	\
    $(SRS)$/notes.srs			\
    $(SRS)$/animui.srs			\
    $(SRS)$/slideshow.srs		\
    $(SRS)$/uitable.srs			\
    $(SOLARCOMMONRESDIR)$/sfx.srs

# --- StarDraw DLL

SHL1TARGET= sd$(DLLPOSTFIX)
SHL1USE_EXPORTS=ordinal
SHL1IMPLIB= sdi

# dynamic libraries
SHL1STDLIBS+= \
    $(SVXLIB) \
    $(SFXLIB) \
    $(BASICLIB) \
    $(CPPCANVASLIB) \
    $(BASEGFXLIB) \
    $(GOODIESLIB) \
    $(BASEGFXLIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB) \
    $(SOTLIB) \
    $(CANVASTOOLSLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(I18NISOLANGLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(VOSLIB) \
    $(CANVASLIB) \
    $(SALLIB) \
    $(AVMEDIALIB)

SHL1LIBS= $(LIB3TARGET) $(LIB5TARGET) $(LIB6TARGET)
SHL1DEPN+=	makefile.mk

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEF1NAME	=$(SHL1TARGET)
DEFLIB1NAME = $(TARGET) $(LIB5TARGET:b) $(LIB6TARGET:b)

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
            $(SLB)$/docshell.lib    \
            $(SLB)$/dlg.lib			\
            $(SLB)$/core.lib		\
            $(SLB)$/undo.lib		\
            $(SLB)$/helper.lib		\
            $(SLB)$/xml.lib			\
            $(SLB)$/cgm.lib			\
            $(SLB)$/uitable.lib		\
            $(SLB)$/grf.lib

LIB5TARGET=$(SLB)$/sdraw3_2.lib
LIB5FILES=      \
            $(SLB)$/html.lib		\
            $(SLB)$/filter.lib		\
            $(SLB)$/unoidl.lib		\
            $(SLB)$/ppt.lib 		\
            $(SLB)$/animui.lib		\
            $(SLB)$/accessibility.lib	\
            $(SLB)$/toolpanel.lib		\
            $(SLB)$/uitools.lib			\
            $(SLB)$/tpcontrols.lib		\
            $(SLB)$/slsshell.lib

LIB6TARGET=$(SLB)$/sdraw3_3.lib
LIB6FILES=      \
            $(SLB)$/func.lib        \
            $(SLB)$/func_2.lib        \
            $(SLB)$/slsmodel.lib		\
            $(SLB)$/slsview.lib			\
            $(SLB)$/slscontroller.lib	\
            $(SLB)$/slscache.lib		\
            $(SLB)$/notes.lib			\
            $(SLB)$/slideshow.lib		\
            $(SLB)$/framework_pane.lib			\
            $(SLB)$/framework_view.lib			\
            $(SLB)$/framework_configuration.lib	\
            $(SLB)$/framework_module.lib		\
            $(SLB)$/framework_toolbar.lib		\
            $(SLB)$/framework_tools.lib			\
            $(SLB)$/framework_command.lib

# sdd
SHL2TARGET= sdd$(DLLPOSTFIX)
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
                        $(SOTLIB) \
            $(TOOLSLIB) \
            $(UCBHELPERLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2OBJS=   $(SLO)$/sddetect.obj \
        $(SLO)$/detreg.obj
SHL2DEPN+=	makefile.mk

# sdui
SHL4TARGET= sdui$(DLLPOSTFIX)
SHL4IMPLIB= sduiimp
SHL4VERSIONMAP= sdui.map
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=       $(SHL4TARGET)
SHL4LIBS=   $(SLB)$/sdui_all.lib

LIB4TARGET=	$(SLB)$/sdui_all.lib
LIB4FILES=	\
    $(SLB)$/sdui.lib \
    $(SLB)$/func_ui.lib \
    $(SLB)$/html_ui.lib

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
    $(I18NISOLANGLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(VOSLIB) \
    $(CANVASLIB) \
    $(SALLIB)
# $(ISDLIB) is build in SHL1TARGET
.IF "$(GUI)" == "UNX"
SHL4DEPN=$(SHL1TARGETN)
.ELSE
SHL4DEPN=$(SHL1IMPLIBN)
.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @$(TYPE) sd.flt > $@


