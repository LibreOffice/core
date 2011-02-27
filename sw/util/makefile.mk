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

PRJNAME=sw
TARGET=sw
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
USE_DEFFILE=TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein -----------------------------------------------------------

sw_res_files= \
    $(SRS)$/app.srs          \
    $(SRS)$/dialog.srs       \
    $(SRS)$/chrdlg.srs       \
    $(SRS)$/config.srs       \
    $(SRS)$/dbui.srs         \
    $(SRS)$/dochdl.srs       \
    $(SRS)$/docvw.srs        \
    $(SRS)$/envelp.srs       \
    $(SRS)$/fldui.srs        \
    $(SRS)$/fmtui.srs        \
    $(SRS)$/frmdlg.srs       \
    $(SRS)$/globdoc.srs      \
    $(SRS)$/index.srs        \
    $(SRS)$/layout.srs       \
    $(SRS)$/lingu.srs        \
    $(SRS)$/misc.srs         \
    $(SRS)$/ribbar.srs       \
    $(SRS)$/shells.srs       \
    $(SRS)$/smartmenu.srs    \
    $(SRS)$/table.srs        \
    $(SRS)$/uiview.srs       \
    $(SRS)$/undo.srs         \
    $(SRS)$/unocore.srs      \
    $(SRS)$/utlui.srs        \
    $(SRS)$/web.srs          \
    $(SRS)$/wrtsh.srs        \
    $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=sw
RESLIB1IMAGES=$(PRJ)$/imglst $(PRJ)$/res
RESLIB1SRSFILES= \
    $(sw_res_files)

SWLIBFILES       = \
        $(SLB)$/core1.lib	\
        $(SLB)$/core2.lib	\
        $(SLB)$/core3.lib	\
        $(SLB)$/core4.lib	\
        $(SLB)$/filter.lib	\
        $(SLB)$/ui1.lib	\
        $(SLB)$/ui2.lib

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1USE_EXPORTS=name
SHL1IMPLIB= _$(TARGET)
SHL1LIBS= $(SLB)$/swall.lib $(SWLIBFILES)

LIB1TARGET      =$(SLB)$/swall.lib

LIB1OBJFILES= $(OUT)$/slo$/swmodule.obj \
    $(OUT)$/slo$/swdll.obj

# dynamic libraries
SHL1STDLIBS+= \
    $(LNGLIB) \
    $(SVXCORELIB) \
    $(EDITENGLIB) \
    $(SVXLIB) \
    $(SFXLIB) \
    $(XMLOFFLIB) \
    $(BASICLIB) \
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB)	\
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(I18NISOLANGLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(SALHELPERLIB) \
    $(ICUUCLIB) \
    $(I18NUTILLIB) \
    $(AVMEDIALIB)

.IF "$(DBG_LEVEL)">="2"
SHL1STDLIBS+= $(LIBXML2LIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(ADVAPI32LIB)
.ENDIF # WNT


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1BASE=	0x1e000000
DEF1NAME	=$(SHL1TARGET)
DEFLIB1NAME=swall $(SWLIBFILES:b)

SHL2TARGET= swd$(DLLPOSTFIX)
SHL2IMPLIB= swdimp
SHL2VERSIONMAP=$(SOLARENV)/src/component.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=       $(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
    $(UNOTOOLSLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(TOOLSLIB) \
        $(UCBHELPERLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2OBJS=   $(SLO)$/swdetect.obj \
        $(SLO)$/swdet2.obj \
        $(SLO)$/detreg.obj \
        $(SLO)$/iodetect.obj

.IF "$(DBG_LEVEL)">="2"
SHL2STDLIBS+= \
        $(LIBXML2LIB)
.ENDIF

SHL2DEPN+=  makefile.mk

# add for swui
SHL3TARGET= swui$(DLLPOSTFIX)
SHL3IMPLIB= swuiimp
SHL3VERSIONMAP= swui.map
SHL3DEF=$(MISC)$/$(SHL3TARGET).def
SHL3DEPN=$(SHL1TARGETN)
DEF3NAME=       $(SHL3TARGET)

SHL3STDLIBS= \
        $(ISWLIB) \
            $(SVXCORELIB) \
            $(EDITENGLIB) \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(SVLLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(I18NISOLANGLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB)	\
            $(CPPUHELPERLIB)	\
            $(CPPULIB) \
            $(SALLIB) \
            $(SOTLIB)
.IF "$(DBG_LEVEL)">="2"
SHL3STDLIBS+= $(LIBXML2LIB)
.ENDIF

SHL3LIBS=   $(SLB)$/swui.lib
LIB3TARGET = $(SLB)$/swui.lib

LIB3OBJFILES = \
        $(SLO)$/swuiexp.obj     \
    $(SLO)$/SwRewriter.obj \
        $(SLO)$/swdlgfact.obj	\
        $(SLO)$/addrdlg.obj \
        $(SLO)$/ascfldlg.obj \
        $(SLO)$/break.obj \
        $(SLO)$/bookmark.obj \
        $(SLO)$/changedb.obj \
        $(SLO)$/chardlg.obj \
        $(SLO)$/convert.obj \
        $(SLO)$/cption.obj \
        $(SLO)$/dbinsdlg.obj \
        $(SLO)$/docfnote.obj	\
        $(SLO)$/docstdlg.obj \
         $(SLO)$/envlop1.obj \
         $(SLO)$/envfmt.obj  \
         $(SLO)$/envprt.obj  \
         $(SLO)$/label1.obj  \
          $(SLO)$/labfmt.obj  \
          $(SLO)$/labelexp.obj \
           $(SLO)$/labprt.obj  \
           $(SLO)$/drpcps.obj \
           $(SLO)$/pardlg.obj \
           $(SLO)$/pattern.obj \
            $(SLO)$/pggrid.obj \
            $(SLO)$/pgfnote.obj \
            $(SLO)$/rowht.obj \
            $(SLO)$/selglos.obj \
            $(SLO)$/splittbl.obj \
             $(SLO)$/srtdlg.obj	\
             $(SLO)$/tautofmt.obj	\
             $(SLO)$/tblnumfm.obj \
             $(SLO)$/uiborder.obj \
             $(SLO)$/wrap.obj	\
             $(SLO)$/colwd.obj	\
             $(SLO)$/tabledlg.obj \
             $(SLO)$/fldtdlg.obj \
             $(SLO)$/fldedt.obj \
             $(SLO)$/fldpage.obj \
             $(SLO)$/flddb.obj \
             $(SLO)$/flddinf.obj \
             $(SLO)$/flddok.obj \
             $(SLO)$/fldfunc.obj \
             $(SLO)$/fldref.obj \
             $(SLO)$/FldRefTreeListBox.obj \
             $(SLO)$/fldvar.obj \
             $(SLO)$/swrenamexnameddlg.obj	\
             $(SLO)$/swmodalredlineacceptdlg.obj	\
        $(SLO)$/abstract.obj \
    $(SLO)$/frmdlg.obj \
    $(SLO)$/tmpdlg.obj \
    $(SLO)$/frmpage.obj \
    $(SLO)$/glosbib.obj \
    $(SLO)$/glossary.obj \
    $(SLO)$/inpdlg.obj \
    $(SLO)$/insfnote.obj \
    $(SLO)$/instable.obj \
    $(SLO)$/insrule.obj \
    $(SLO)$/javaedit.obj \
    $(SLO)$/titlepage.obj \
    $(SLO)$/linenum.obj \
    $(SLO)$/mailmrge.obj \
    $(SLO)$/multmrk.obj \
    $(SLO)$/mergetbl.obj \
    $(SLO)$/outline.obj \
    $(SLO)$/num.obj \
    $(SLO)$/column.obj \
    $(SLO)$/cnttab.obj \
    $(SLO)$/cntex.obj \
    $(SLO)$/uiregionsw.obj \
    $(SLO)$/optload.obj \
        $(SLO)$/optcomp.obj \
        $(SLO)$/optpage.obj \
        $(SLO)$/swuiccoll.obj \
        $(SLO)$/numpara.obj \
        $(SLO)$/swdialmgr.obj \
        $(SLO)$/swuiidxmrk.obj \
        $(SLO)$/DropDownFieldDialog.obj \
        $(SLO)$/macassgn.obj \
        $(SLO)$/wordcountdialog.obj \
        $(SLO)$/mailconfigpage.obj \
        $(SLO)$/addresslistdialog.obj \
        $(SLO)$/createaddresslistdialog.obj \
        $(SLO)$/customizeaddresslistdialog.obj \
        $(SLO)$/dbtablepreviewdialog.obj \
        $(SLO)$/mailmergewizard.obj \
        $(SLO)$/mmdocselectpage.obj \
        $(SLO)$/mmlayoutpage.obj \
        $(SLO)$/mmoutputpage.obj \
        $(SLO)$/mmoutputtypepage.obj \
        $(SLO)$/mmaddressblockpage.obj \
        $(SLO)$/mmgreetingspage.obj \
        $(SLO)$/mmmergepage.obj \
        $(SLO)$/mmpreparemergepage.obj \
        $(SLO)$/selectdbtabledialog.obj

.IF "$(GUI)$(COM)" == "WNTMSC"
.IF "$(ENABLE_PCH)" != "" && ( "$(PRJNAME)"!="sw" || "$(BUILD_SPECIAL)"!="TRUE" )
#target sw
SHL1OBJS += $(SLO)$/pchname.obj \
            $(SLO)$/pchname_ex.obj
#target swd
SHL2OBJS += $(SLO)$/pchname.obj \
            $(SLO)$/pchname_ex.obj
#target swui
SHL3OBJS += $(SLO)$/pchname.obj \
            $(SLO)$/pchname_ex.obj
.ENDIF # "$(ENABLE_PCH)" != ""
.ENDIF # "$(GUI)$(COM)" == "WNTMSC"

SHL4TARGET=msword$(DLLPOSTFIX)
SHL4VERSIONMAP=msword.map
SHL4LIBS=$(SLB)$/rtf.lib $(SLB)$/ww8.lib
SHL4DEPN=$(SHL1TARGETN)
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=$(SHL4TARGET)

SHL4STDLIBS= \
    $(ISWLIB) \
    $(OOXLIB) \
    $(SAXLIB) \
    $(SVXCORELIB) \
       $(EDITENGLIB) \
    $(MSFILTERLIB) \
    $(SFXLIB) \
    $(BASEGFXLIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB)	\
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(I18NISOLANGLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(SALLIB) \
    $(ICUUCLIB) \
    $(BASICLIB)     \
                $(MSFILTERLIB) \
    $(I18NUTILLIB)

.IF "$(ENABLE_VBA)" == "YES"
#target vba
TARGET_VBA=vbaswobj
SHL5TARGET=$(TARGET_VBA)$(DLLPOSTFIX).uno
SHL5IMPLIB=     i$(TARGET_VBA)

SHL5VERSIONMAP=$(SOLARENV)/src/component.map
SHL5DEF=$(MISC)$/$(SHL5TARGET).def
DEF5NAME=$(SHL5TARGET)
SHL5STDLIBS= \
                $(ISWLIB) \
                $(CPPUHELPERLIB) \
                $(VCLLIB) \
                $(CPPULIB) \
                $(COMPHELPERLIB) \
                $(SVLIB) \
                $(UNOTOOLSLIB) \
                $(TOOLSLIB) \
                $(SALLIB)\
                $(VBAHELPERLIB) \
                $(BASICLIB)     \
                $(SFXLIB)       \
                $(SVXLIB)       \
                $(SVTOOLLIB)    \
                $(SVLLIB) \
                $(VCLLIB) \
                $(TKLIB) \
                $(I18NISOLANGLIB) \
                $(EDITENGLIB) \
                $(SVXCORELIB) \
                $(MSFILTERLIB)

.IF "$(GUI)"=="WNT"
SHL5STDLIBS+=$(SHELLLIB)
.ENDIF #WNT

SHL5DEPN=$(SHL1TARGETN)
SHL5LIBS=$(SLB)$/$(TARGET_VBA).lib
.ENDIF # .IF "$(ENABLE_VBA)" == "YES"

.INCLUDE :  target.mk
