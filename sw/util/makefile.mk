#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.67 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:32:30 $
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

PRJNAME=sw
TARGET=sw
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
USE_DEFFILE=TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein -----------------------------------------------------------

.IF "$(OS)"=="IRIX"
LINKFLAGS+=-Wl,-LD_LAYOUT:lgot_buffer=40
.ENDIF

sw_res_files= \
    $(SRS)$/app.srs          \
    $(SRS)$/dialog.srs       \
    $(SRS)$/chrdlg.srs       \
    $(SRS)$/config.srs       \
    $(SRS)$/dbui.srs	    \
    $(SRS)$/dochdl.srs       \
    $(SRS)$/docvw.srs        \
    $(SRS)$/envelp.srs       \
    $(SRS)$/fldui.srs        \
    $(SRS)$/fmtui.srs        \
    $(SRS)$/frmdlg.srs       \
    $(SRS)$/globdoc.srs      \
    $(SRS)$/index.srs        \
    $(SRS)$/lingu.srs        \
    $(SRS)$/misc.srs         \
    $(SRS)$/ribbar.srs       \
    $(SRS)$/shells.srs       \
    $(SRS)$/smartmenu.srs    \
    $(SRS)$/table.srs        \
    $(SRS)$/uiview.srs       \
    $(SRS)$/undo.srs        \
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
    $(SVXLIB) \
    $(SFXLIB) \
    $(XMLOFFLIB) \
    $(BASICLIB) \
    $(GOODIESLIB) \
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
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(VOSLIB) \
    $(SALLIB) \
    $(SALHELPERLIB) \
    $(ICUUCLIB) \
    $(I18NUTILLIB)	\
        $(AVMEDIALIB) \
        $(LIBXML2LIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(ADVAPI32LIB)
.ENDIF # WNT


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1BASE=	0x1e000000
DEF1NAME	=$(SHL1TARGET)
DEFLIB1NAME=swall $(SWLIBFILES:b)

SHL2TARGET= swd$(DLLPOSTFIX)
SHL2IMPLIB= swdimp
SHL2VERSIONMAP= swd.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=       $(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(TOOLSLIB) \
        $(UCBHELPERLIB) \
        $(UNOTOOLSLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2OBJS=   $(SLO)$/swdetect.obj \
        $(SLO)$/swdet2.obj \
        $(SLO)$/detreg.obj

.IF "$(product)"==""
SHL2OBJS+=  \
        $(SLO)$/errhdl.obj
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
            $(SVXLIB) \
            $(SFX2LIB) \
            $(BASICLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(SVLLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB)	\
            $(CPPUHELPERLIB)	\
            $(CPPULIB) \
            $(SALLIB) \
            $(SOTLIB)

.IF "$(ENABLE_LAYOUT)" == "TRUE"
LINKFLAGS+= -L../$(PRJ)/layout/$(INPATH)/lib
SHL3STDLIBS += -ltklayout$(UPD)$(DLLPOSTFIX)
.ENDIF # ENABLE_LAYOUT == TRUE

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
    $(SLO)$/insrc.obj \
    $(SLO)$/instable.obj \
    $(SLO)$/insrule.obj \
    $(SLO)$/javaedit.obj \
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


.INCLUDE :  target.mk

