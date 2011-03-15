#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJNAME=sc
TARGET=scalc3
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Resourcen ----------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/miscdlgs.srs	\
    $(SRS)$/docshell.srs	\
    $(SRS)$/ui.srs		\
    $(SRS)$/dbgui.srs	\
    $(SRS)$/drawfunc.srs \
    $(SRS)$/core.srs 	\
    $(SRS)$/styleui.srs	\
    $(SRS)$/formdlgs.srs \
    $(SRS)$/pagedlg.srs	\
    $(SRS)$/navipi.srs	\
    $(SRS)$/cctrl.srs	\


RESLIB1NAME=sc
RESLIB1IMAGES=\
    $(PRJ)$/res					\
    $(PRJ)$/res/imglst/apptbx	\
    $(PRJ)$/res/imglst/dbgui	\
    $(PRJ)$/res/imglst/navipi

RESLIB1SRSFILES=\
    $(RESLIB1LIST)

# --- StarClac DLL

SHL1TARGET= sc$(DLLPOSTFIX)
SHL1USE_EXPORTS=name
SHL1IMPLIB= sci

# dynamic libraries
SHL1STDLIBS=       \
    $(BASICLIB)	\
    $(SFXLIB)		\
    $(SVTOOLLIB)	\
    $(SVLLIB)		\
    $(SVXCORELIB)		\
    $(EDITENGLIB)		\
    $(SVXLIB)		\
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(VCLLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)	\
    $(VBAHELPERLIB)	\
    $(TKLIB)		\
    $(SALLIB)		\
    $(SALHELPERLIB)		\
    $(TOOLSLIB)	\
    $(I18NISOLANGLIB) \
    $(UNOTOOLSLIB) \
    $(SOTLIB)		\
    $(XMLOFFLIB)	\
    $(AVMEDIALIB) \
    $(FORLIB) \
    $(FORUILIB)

SHL1LIBS=$(LIB3TARGET) $(LIB4TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME= $(LIB3TARGET:b) $(LIB4TARGET:b)

.IF "$(GUI)" == "WNT"
SHL1RES=    $(RCTARGET)
.ENDIF

# --- Linken der Applikation ---------------------------------------

LIB3TARGET=$(SLB)$/scalc3.lib
LIB3FILES=	\
    $(SLB)$/app.lib \
    $(SLB)$/docshell.lib \
    $(SLB)$/view.lib \
    $(SLB)$/undo.lib \
    $(SLB)$/attrdlg.lib \
    $(SLB)$/namedlg.lib \
    $(SLB)$/miscdlgs.lib \
    $(SLB)$/formdlgs.lib \
    $(SLB)$/cctrl.lib \
    $(SLB)$/dbgui.lib \
    $(SLB)$/pagedlg.lib \
    $(SLB)$/drawfunc.lib \
    $(SLB)$/navipi.lib

LIB3FILES+= \
            $(SLB)$/unoobj.lib


LIB4TARGET=$(SLB)$/scalc3c.lib
LIB4FILES=	\
    $(SLB)$/data.lib \
    $(SLB)$/tool.lib \
    $(SLB)$/xml.lib \
    $(SLB)$/accessibility.lib

SHL2TARGET= scd$(DLLPOSTFIX)
SHL2IMPLIB= scdimp
SHL2VERSIONMAP=$(SOLARENV)/src/component.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(TOOLSLIB) \
            $(UCBHELPERLIB)	\
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SOTLIB) \
            $(SALLIB)

SHL2OBJS=   $(SLO)$/scdetect.obj \
        $(SLO)$/detreg.obj
SHL2DEPN+=	makefile.mk

# split out filters
SHL6TARGET= scfilt$(DLLPOSTFIX)
SHL6IMPLIB= scfiltimp
SHL6LIBS= \
    $(SLB)$/ftools.lib \
    $(SLB)$/excel.lib \
    $(SLB)$/xcl97.lib \
    $(SLB)$/lotus.lib \
    $(SLB)$/qpro.lib \
    $(SLB)$/dif.lib \
    $(SLB)$/html.lib \
    $(SLB)$/rtf.lib \
    $(SLB)$/scflt.lib
SHL6VERSIONMAP= scfilt.map
SHL6DEF=$(MISC)$/$(SHL6TARGET).def
DEF6NAME= $(SHL6TARGET)
SHL6DEPN=$(SHL1TARGETN)
SHL6STDLIBS= \
    $(ISCLIB) \
    $(BASICLIB)	\
    $(SFXLIB)		\
    $(SVTOOLLIB)	\
    $(SVLLIB)		\
    $(SVXCORELIB)		\
    $(EDITENGLIB)		\
    $(MSFILTERLIB)		\
    $(SVXLIB)		\
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(VCLLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(TKLIB)		\
    $(SALLIB)		\
    $(TOOLSLIB)	\
    $(UNOTOOLSLIB) \
    $(SOTLIB)		\
    $(OOXLIB)       \
    $(SAXLIB) \
    $(FORLIB)

# add for scui
SHL8TARGET= scui$(DLLPOSTFIX)
SHL8IMPLIB= scuiimp
SHL8VERSIONMAP= scui.map
SHL8DEF=$(MISC)$/$(SHL8TARGET).def
SHL8DEPN=$(SHL1TARGETN)
DEF8NAME=$(SHL8TARGET)

SHL8STDLIBS= \
            $(ISCLIB) \
            $(EDITENGLIB)		\
            $(SVXCORELIB) \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(I18NISOLANGLIB) \
            $(COMPHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB) \
            $(FORLIB) \
            $(FORUILIB)
.IF "$(ENABLE_LAYOUT)" == "TRUE"
SHL8STDLIBS+=$(TKLIB)
.ENDIF # ENABLE_LAYOUT == TRUE

SHL8LIBS=   $(SLB)$/scui.lib
LIB8TARGET = $(SLB)$/scui.lib

LIB8FILES=$(SLB)$/styleui.lib	\
        $(SLB)$/optdlg.lib

LIB8OBJFILES = \
        $(SLO)$/scuiexp.obj     \
        $(SLO)$/scdlgfact.obj \
        $(SLO)$/tpsubt.obj		\
        $(SLO)$/tptable.obj	\
        $(SLO)$/tpstat.obj	\
        $(SLO)$/tabpages.obj	\
        $(SLO)$/tpsort.obj		\
        $(SLO)$/sortdlg.obj		\
        $(SLO)$/validate.obj	\
        $(SLO)$/textdlgs.obj		\
        $(SLO)$/subtdlg.obj		\
        $(SLO)$/tphf.obj		\
        $(SLO)$/scuitphfedit.obj	\
        $(SLO)$/hfedtdlg.obj	\
        $(SLO)$/attrdlg.obj	\
        $(SLO)$/scuiimoptdlg.obj	\
        $(SLO)$/strindlg.obj		\
        $(SLO)$/tabbgcolordlg.obj   \
        $(SLO)$/shtabdlg.obj		\
        $(SLO)$/scendlg.obj		\
        $(SLO)$/pvfundlg.obj	\
        $(SLO)$/pfiltdlg.obj	\
        $(SLO)$/namepast.obj		\
        $(SLO)$/namecrea.obj		\
        $(SLO)$/mvtabdlg.obj		\
        $(SLO)$/mtrindlg.obj		\
        $(SLO)$/linkarea.obj		\
        $(SLO)$/lbseldlg.obj		\
        $(SLO)$/instbdlg.obj		\
        $(SLO)$/inscodlg.obj		\
        $(SLO)$/inscldlg.obj		\
        $(SLO)$/groupdlg.obj		\
        $(SLO)$/filldlg.obj			\
        $(SLO)$/delcodlg.obj		\
        $(SLO)$/delcldlg.obj		\
        $(SLO)$/datafdlg.obj		\
        $(SLO)$/dapitype.obj	\
        $(SLO)$/dapidata.obj	\
        $(SLO)$/crdlg.obj			\
        $(SLO)$/scuiasciiopt.obj	\
        $(SLO)$/textimportoptions.obj	\
        $(SLO)$/scuiautofmt.obj	\
        $(SLO)$/dpgroupdlg.obj	\
        $(SLO)$/editfield.obj

TARGET_VBA=vbaobj
SHL9TARGET=$(TARGET_VBA)$(DLLPOSTFIX).uno
SHL9IMPLIB=	i$(TARGET_VBA)

SHL9VERSIONMAP=$(SOLARENV)/src/component.map
SHL9DEF=$(MISC)$/$(SHL9TARGET).def
DEF9NAME=$(SHL9TARGET)
SHL9RPATH=OOO

SHL9STDLIBS= \
        $(VBAHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(VCLLIB) \
        $(CPPULIB) \
        $(COMPHELPERLIB) \
        $(SVLIB) \
        $(TOOLSLIB) \
        $(SALLIB)\
        $(BASICLIB)	\
        $(SFXLIB)	\
        $(EDITENGLIB)		\
        $(SVXCORELIB)	\
        $(SVTOOLLIB)    \
        $(SVLLIB) \
        $(ISCLIB) \
        $(VCLLIB) \
        $(TKLIB) \
        $(MSFILTERLIB)		\
        $(FORLIB) \
        $(UNOTOOLSLIB)

SHL9DEPN=$(SHL1TARGETN) $(SHL8TARGETN)
SHL9LIBS=$(SLB)$/$(TARGET_VBA).lib

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
    LIBCOMPNAME=$(LOCAL_COMMON_OUT)$/lib/lib$(SHL9TARGET)$(DLLPOST)
.ELSE
    LIBCOMPNAME=$(COMMONBIN)$/$(SHL9TARGET)$(DLLPOST)
.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR:	$(MISC)$/linkinc.ls  $(COMP)

ALLTAR : $(MISC)/sc.component $(MISC)/scd.component $(MISC)/vbaobj.component

$(MISC)/sc.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sc.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sc.component

$(MISC)/scd.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        scd.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt scd.component

$(MISC)/vbaobj.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        vbaobj.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL9TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt vbaobj.component
