#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:01:29 $
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

PRJNAME=SVX
TARGET=svx
#svx.hid generieren
GEN_HID=TRUE
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.IF "$(COM)"=="ICC"
#LINKFLAGS+=/SEGMENTS:1024 /PACKD:32768
LINKFLAGS+=/SEGMENTS:1024
.ENDIF

RSCLOCINC+=-I$(PRJ)$/source$/svdraw

.IF "$(GUI)"=="WNT"
LIBCMT += $(LIBCIMT)
.ENDIF

# --- Allgemein ----------------------------------------------------

# --- Svx - DLL ----------

.IF "$(header)" == ""

HELPIDFILES=    ..$/inc$/helpid.hrc

SHL1TARGET= svx$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= svx
SVXLOKAL=	$(LB)$/svx.lib
SHL1BASE  = 0x1d800000
SHL1STDLIBS= \
            $(SALLIB) \
            $(VOSLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)	\
            $(SVLIB) \
            $(SO2LIB) \
            $(SOTLIB) \
            $(XMLOFFLIB) \
            $(SFX2LIB) \
            $(GOODIESLIB) \
            $(BASICLIB) \
            $(INETLIBSH) \
            $(CHANELLIB) \
            $(SVMEMLIB) \
            $(TKLIB) \
            $(ONELIB) \
            $(RTLLIB) \
            $(CPPULIB) \
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)	\
            $(UNOTOOLSLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
            $(SHELLLIB)
.ENDIF

.IF "$(TF_UCB)" == ""
SHL1STDLIBS+=\
        $(CHAOSLIB)
.ENDIF

.IF "$(BIG_SVX)"==""
.IF "$(GUI)"=="OS2"
SHL1STDLIBS+=\
            $(LB)$/dl1.lib \
            $(LB)$/dl2.lib
.ELSE
SHL1STDLIBS+=\
            $(LB)$/dl.lib
.ENDIF
.ENDIF

.IF "$(SOLAR_JAVA)" != ""
SHL1STDLIBS+=\
        $(SJLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1DEPN=       $(SLB)$/svx.lib $(LB)$/dl.lib
.ENDIF

SHL1LIBS=       $(SLB)$/svx.lib

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME        =$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME=svx
DEF1DES		= Rtf, Edt, Outliner, SvDraw, Form, Fmcomp, Engine3D, MSFilter
DEF1EXPORTFILE	= svx.dxp

.IF "$(BIG_SVX)"==""

SHL2TARGET= dl$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= dl
SVXLOKAL+=	$(LB)$/dl.lib
SHL2BASE  = 0x1db00000
SHL2STDLIBS= \
            $(LB)$/svx.lib \
            $(SALLIB) \
            $(VOSLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)	\
            $(SVLIB) \
            $(SO2LIB) \
            $(SOTLIB) \
            $(SFX2LIB) \
            $(GOODIESLIB) \
            $(BASICLIB) \
            $(INETLIBSH) \
            $(CHANELLIB) \
            $(SVMEMLIB) \
            $(TKLIB) \
            $(ONELIB) \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(RTLLIB) \
            $(UNOTOOLSLIB) \
            $(UCBHELPERLIB) 

.IF "$(TF_UCB)" == ""
SHL2STDLIBS+=\
        $(CHAOSLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+=\
            $(SHELLLIB)
.ENDIF

.IF "$(SOLAR_JAVA)" != ""
SHL2STDLIBS+=\
        $(SJLIB)
.ENDIF


SHL2DEPN=       $(SLB)$/dl.lib $(LB)$/svx.lib

SHL2LIBS=       $(SLB)$/dl.lib

.IF "$(COM)"=="ICC"
SHL2OBJS=       $(SLO)$/sidll.obj
.ENDIF
SHL2OBJS+=      $(SLO)$/svxempty.obj

SHL2DEF=        $(MISC)$/$(SHL2TARGET).def
DEF2NAME        =$(SHL2TARGET)
DEF2DEPN        =$(MISC)$/$(SHL2TARGET).flt
DEFLIB2NAME=dl
DEF2DES     =SvDraw, Form, Fmcomp, Engine3D, XOutDev, MSFilter

.ENDIF

LIBEXTRAFILES=\
        $(LIBPRE) $(SLB)$/svdraw.lib \
        $(LIBPRE) $(SLB)$/form.lib \
        $(LIBPRE) $(SLB)$/fmcomp.lib \
        $(LIBPRE) $(SLB)$/engine3d.lib \
        $(LIBPRE) $(SLB)$/msfilter.lib \
        $(LIBPRE) $(SLB)$/xout.lib

LIB1TARGET      =$(SLB)$/svx.lib
LIB1FILES       = \
            $(LIBPRE) $(SLB)$/items.lib     \
            $(LIBPRE) $(SLB)$/svxlink.lib   \
            $(LIBPRE) $(SLB)$/svxrtf.lib    \
            $(LIBPRE) $(SLB)$/editeng.lib   \
            $(LIBPRE) $(SLB)$/outliner.lib \
            $(LIBPRE) $(SLB)$/dialogs.lib\
            $(LIBPRE) $(SLB)$/mnuctrls.lib  \
            $(LIBPRE) $(SLB)$/options.lib   \
            $(LIBPRE) $(SLB)$/stbctrls.lib  \
            $(LIBPRE) $(SLB)$/tbxctrls.lib  \
            $(LIBPRE) $(SLB)$/unoedit.lib   \
            $(LIBPRE) $(SLB)$/unodraw.lib	\
            $(LIBPRE) $(SLB)$/gal.lib


.IF "$(SVXLIGHT)" != ""
LIB3TARGET= $(LB)$/svxl.lib
LIB3ARCHIV= $(LB)$/libsvxl.a
LIB3FILES=  \
            $(LB)$/sxl_editeng.lib \
            $(LB)$/sxl_engine3d.lib \
            $(LB)$/sxl_form.lib \
            $(LB)$/sxl_items.lib \
            $(LB)$/sxl_outliner.lib \
            $(LB)$/sxl_svdraw.lib \
            $(LB)$/sxl_xout.lib \
            $(LB)$/sxl_options.lib
.ENDIF

.IF "$(BIG_SVX)"==""
LIB2TARGET      =$(SLB)$/dl.lib
LIB2FILES       = $(LIBEXTRAFILES)
.ELSE
LIB1FILES+=$(LIBEXTRAFILES)
.ENDIF

.IF "$(GUI)" == "OS2" || "(GUIBASE)" == "WIN"
LIB1FILES  += \
            $(LIBPRE) $(SLB)$/ibrwimp.lib
.ENDIF


SRSFILELIST=\
                $(SRS)$/svdstr.srs      \
                $(SRS)$/editeng.srs     \
                $(SRS)$/outliner.srs \
                $(SRS)$/dialogs.srs     \
                $(SRS)$/drawdlgs.srs \
                $(SRS)$/mnuctrls.srs \
                $(SRS)$/stbctrls.srs \
                $(SRS)$/tbxctrls.srs \
                $(SRS)$/options.srs     \
                $(SRS)$/svxitems.srs \
                $(SRS)$/form.srs \
                $(SRS)$/fmcomp.srs \
                $(SRS)$/engine3d.srs \
                $(SRS)$/unodraw.srs \
                $(SRS)$/svxlink.srs 
                
.IF "$(GUI)" != "MAC"
SRSFILELIST+=   $(SOLARVERSION)$/$(INPATH)$/res$(UPDMINOREXT)$/sfx.srs
.ELSE
.IF "$(UPDMINOR)" != ""
SRSFILELIST+=   $(SOLARVERSION)$/$(INPATH)$/res.$(UPDMINOREXT)$/sfx.srs
.ELSE
SRSFILELIST+=   $(SOLARVERSION)$/$(INPATH)$/res$/sfx.srs
.ENDIF
.ENDIF

RESLIB1NAME=svx
RESLIB1SRSFILES= $(SRSFILELIST)

.IF "$(depend)" != ""

ALL:
    @echo nothing to depend on

.ELSE
.IF "$(GUI)"=="WNT"


.IF "$(BIG_SVX)"==""
ALL:      \
        $(MAKELANGDIR)  \
            $(SLB)$/dl.lib  $(SLB)$/svx.lib \
            $(LB)$/dl.lib   $(LB)$/svx.lib \
          $(MISC)$/linkinc.ls                   \
          $(SRS)$/hidother.hid                  \
          ALLTAR
.ELSE
ALL:      \
        $(MAKELANGDIR)  \
            $(SLB)$/svx.lib \
            $(LB)$/svx.lib \
          $(MISC)$/linkinc.ls                   \
          $(SRS)$/hidother.hid                  \
          ALLTAR
.ENDIF

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
ALL: \
        $(MAKELANGDIR)  \
        ALLTAR
.ENDIF

.IF "$(GUI)"=="OS2"
ALL:      \
        $(MAKELANGDIR)  \
        implib_defs     \
        implib1         \
        implib2 		\
        implib3 		\
        implib4 		\
        ALLTAR
.ENDIF
.ENDIF			#F "$(depend)" != ""

# --- Targets -------------------------------------------------------


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo lcl > $@
    @echo +getImplementation >> $@
    @echo Impl >> $@
    @echo IMPL >> $@
    @echo START >> $@
    @echo CharAttribList >> $@
    @echo CharPosArray >> $@
    @echo ContentAttribs >> $@
    @echo ContentAttribsInfo >> $@
    @echo ContentInfo >> $@
    @echo ContentNode >> $@
    @echo DeletedNodeInfo >> $@
    @echo EditAttrib >> $@
    @echo EditCharAttrib >> $@
    @echo EditDbg >> $@
    @echo EditDoc >> $@
    @echo EditEngineItemPool >> $@
    @echo EditHTMLParser >> $@
    @echo EditLine >> $@
    @echo EditNodeIdx >> $@
    @echo EditPaM >> $@
    @echo EditPosition >> $@
    @echo EditRTFParser >> $@
    @echo EditSel >> $@
    @echo EditSpellWrapper >> $@
    @echo EditStyleSheet >> $@
    @echo EditUndoConnectParas >> $@
    @echo EditUndoDelContent >> $@
    @echo EditUndoInsertChars >> $@
    @echo EditUndoInsertFeature >> $@
    @echo EditUndoManager >> $@
    @echo EditUndoMoveParagraphs >> $@
    @echo EditUndoRemoveChars >> $@
    @echo EditUndoRemoveFeature >> $@
    @echo EditUndoSetAttribs >> $@
    @echo EditUndoSetParaAttribs >> $@
    @echo EditUndoSetStyleSheet >> $@
    @echo EditUndoSplitPara >> $@
    @echo GlobalEditData >> $@
    @echo IdleFormattter >> $@
    @echo ImpEditEngine >> $@
    @echo ImpEditView >> $@
    @echo InternalEditStatus >> $@
    @echo ParaPortion >> $@
    @echo ParagraphList >> $@
    @echo RTFPardAttrMapIds >> $@
    @echo RTFPlainAttrMapIds >> $@
    @echo SelRange >> $@
    @echo SortedPositions_SAR >> $@
    @echo SvxACorrChars >> $@
    @echo SvxColorList >> $@
    @echo SvxFontTable >> $@
    @echo SvxRTFStyleType >> $@
    @echo SvxTabStopArr_SAR >> $@
    @echo TextPortionList >> $@
    @echo XEditAttrib >> $@
    @echo XParaPortionList >> $@
    @echo BinTextObject >> $@
    @echo DialogsResMgr >> $@
    @echo E3dDragMethod >> $@
    @echo E3dTriangle >> $@
.IF "$(GUI)" != "MAC"
    @echo GetImp >> $@
.ENDIF
    @echo ImpA >> $@
    @echo ImpB >> $@
    @echo ImpBrw >> $@
    @echo ImpC >> $@
    @echo ImpCheck >> $@
    @echo ImpColor >> $@
    @echo ImpD >> $@
    @echo ImpDrag >> $@
    @echo ImpEdCtrl >> $@
    @echo ImpEdit >> $@
    @echo ImpF >> $@
    @echo ImpGet >> $@
    @echo ImpH >> $@
    @echo ImpHide >> $@
    @echo ImpI >> $@
    @echo ImpItem >> $@
    @echo ImpJ >> $@
    @echo ImpL >> $@
    @echo ImpList >> $@
    @echo ImpM >> $@
    @echo ImpMulti >> $@
    @echo ImportInfo >> $@
    @echo ImpP >> $@
    @echo ImpPaint >> $@
    @echo ImpPast >> $@
    @echo ImpRecalc >> $@
    @echo ImpReset >> $@
    @echo ImpRemove >> $@
    @echo ImpRef >> $@
    @echo ImpReset >> $@
    @echo ImpS >> $@
    @echo ImpScroll >> $@
    @echo ImpSdrGDIMetaFileImport >> $@
    @echo ImpSet >> $@
    @echo ImpShow >> $@
    @echo ImpT >> $@
    @echo ImpX >> $@
    @echo OLUndoAttr >> $@
    @echo OLUndoDepth >> $@
    @echo OLUndoExpand >> $@
    @echo OLUndoHeight >> $@
    @echo OutlinerEditEng >> $@
    @echo SdrGraphicLink >> $@
    @echo SdrItemBrowser >> $@
    @echo SdrOleLink >> $@
    @echo SpellCache >> $@
    @echo SvFileObject >> $@
    @echo WrongList >> $@
    @echo WrongRanges >> $@
    @echo XIOCompat >> $@
    @echo _SdrItemBrowserControl >> $@
    @echo _SdrItemBrowserWindow >> $@
    @echo +FmXGridPeer >> $@
    @echo +FmXGridControl >> $@
    @echo FmX >> $@
    @echo FmExp >> $@
    @echo ODatabaseForm >> $@
.IF "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
    @echo WEP>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
.ENDIF
.IF "$(COM)"=="MSC"
    @echo ??_5>>$@
    @echo ??_7>>$@
    @echo ??_8>>$@
    @echo ??_9>>$@
    @echo ??_C>>$@
    @echo ??_E>>$@
    @echo ??_F>>$@
    @echo ??_G>>$@
    @echo ??_H>>$@
    @echo ??_I>>$@
    @echo __CT>>$@
#   @echo ?CreateType@>>$@
#   @echo ?LinkStub>>$@
.ENDIF
.IF "$(COM)"=="ICC"
    @echo _alloc >> $@
    @echo _lower_bound >> $@
    @echo _stl_prime >> $@
.ENDIF

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo lcl > $@
    @echo +getImplementation >> $@
    @echo Impl >> $@
    @echo IMPL >> $@
    @echo START >> $@
    @echo CharAttribList >> $@
    @echo CharPosArray >> $@
    @echo ContentAttribs >> $@
    @echo ContentAttribsInfo >> $@
    @echo ContentInfo >> $@
    @echo ContentNode >> $@
    @echo DeletedNodeInfo >> $@
    @echo EditAttrib >> $@
    @echo EditCharAttrib >> $@
    @echo EditDbg >> $@
    @echo EditDoc >> $@
    @echo EditEngineItemPool >> $@
    @echo EditHTMLParser >> $@
    @echo EditLine >> $@
    @echo EditNodeIdx >> $@
    @echo EditPaM >> $@
    @echo EditPosition >> $@
    @echo EditRTFParser >> $@
    @echo EditSel >> $@
    @echo EditSpellWrapper >> $@
    @echo EditStyleSheet >> $@
    @echo EditUndoConnectParas >> $@
    @echo EditUndoDelContent >> $@
    @echo EditUndoInsertChars >> $@
    @echo EditUndoInsertFeature >> $@
    @echo EditUndoManager >> $@
    @echo EditUndoMoveParagraphs >> $@
    @echo EditUndoRemoveChars >> $@
    @echo EditUndoRemoveFeature >> $@
    @echo EditUndoSetAttribs >> $@
    @echo EditUndoSetParaAttribs >> $@
    @echo EditUndoSetStyleSheet >> $@
    @echo EditUndoSplitPara >> $@
    @echo GlobalEditData >> $@
    @echo IdleFormattter >> $@
    @echo ImpEditEngine >> $@
    @echo ImpEditView >> $@
    @echo InternalEditStatus >> $@
    @echo ParaPortion >> $@
    @echo ParagraphList >> $@
    @echo RTFPardAttrMapIds >> $@
    @echo RTFPlainAttrMapIds >> $@
    @echo SelRange >> $@
    @echo SortedPositions_SAR >> $@
    @echo SvxACorrChars >> $@
    @echo SvxColorList >> $@
    @echo SvxFontTable >> $@
    @echo SvxRTFStyleType >> $@
    @echo SvxTabStopArr_SAR >> $@
    @echo TextPortionList >> $@
    @echo XEditAttrib >> $@
    @echo XParaPortionList >> $@
    @echo BinTextObject >> $@
    @echo DialogsResMgr >> $@
    @echo E3dDragMethod >> $@
    @echo E3dTriangle >> $@
.IF "$(GUI)" != "MAC"
    @echo GetImp >> $@
.ENDIF
    @echo ImpA >> $@
    @echo ImpB >> $@
    @echo ImpBrw >> $@
    @echo ImpC >> $@
    @echo ImpCheck >> $@
    @echo ImpColor >> $@
    @echo ImpD >> $@
    @echo ImpDrag >> $@
    @echo ImpEdCtrl >> $@
    @echo ImpEdit >> $@
    @echo ImpF >> $@
    @echo ImpGet >> $@
    @echo ImpH >> $@
    @echo ImpHide >> $@
    @echo ImpI >> $@
    @echo ImpItem >> $@
    @echo ImpJ >> $@
    @echo ImpL >> $@
    @echo ImpList >> $@
    @echo ImpM >> $@
    @echo ImpMulti >> $@
    @echo ImportInfo >> $@
    @echo ImpP >> $@
    @echo ImpPaint >> $@
    @echo ImpPast >> $@
    @echo ImpRecalc >> $@
    @echo ImpReset >> $@
    @echo ImpRemove >> $@
    @echo ImpRef >> $@
    @echo ImpReset >> $@
    @echo ImpS >> $@
    @echo ImpScroll >> $@
    @echo ImpSdrGDIMetaFileImport >> $@
    @echo ImpSet >> $@
    @echo ImpShow >> $@
    @echo ImpT >> $@
    @echo ImpX >> $@
    @echo OLUndoAttr >> $@
    @echo OLUndoDepth >> $@
    @echo OLUndoExpand >> $@
    @echo OLUndoHeight >> $@
    @echo OutlinerEditEng >> $@
    @echo SdrGraphicLink >> $@
    @echo SdrItemBrowser >> $@
    @echo SdrOleLink >> $@
    @echo SpellCache >> $@
    @echo SvFileObject >> $@
    @echo WrongList >> $@
    @echo WrongRanges >> $@
    @echo XIOCompat >> $@
    @echo _SdrItemBrowserControl >> $@
    @echo _SdrItemBrowserWindow >> $@
    @echo +FmXGridControl >> $@
    @echo +FmXGridPeer >> $@
    @echo FmX >> $@
    @echo FmExp >> $@
    @echo ODatabaseForm >> $@
.IF "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
    @echo WEP>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
.ENDIF
.IF "$(COM)"=="ICC"
    @echo _alloc >> $@
    @echo _lower_bound >> $@
    @echo _stl_prime >> $@
.ENDIF
.IF "$(COM)"=="MSC"
    @echo ??_5>>$@
    @echo ??_7>>$@
    @echo ??_8>>$@
    @echo ??_9>>$@
    @echo ??_C>>$@
    @echo ??_E>>$@
    @echo ??_F>>$@
    @echo ??_G>>$@
    @echo ??_H>>$@
    @echo ??_I>>$@
#   @echo ?CreateType@>>$@
#   @echo ?LinkStub>>$@
.ENDIF
    @echo _alloc>>$@
    @echo _CT>>$@
    @echo exception::exception>>$@
    @echo _TI2>>$@
    @echo ___CT>>$@

$(SRS)$/hidother.hid: hidother.src
.IF "$(GUI)$(CPU)"!="WNTI" || "$(product)"!="full"
    @echo nix
.ELSE
    @+echo NO HIDS!!!
    +-mhids hidother.src ..\$(INPATH)$/srs $(INCLUDE) svx hidother
.ENDIF


.ENDIF                  # "$(header)" == ""


.INCLUDE :  target.mk


implib1: $(MISC)\svx1.def
    implib /noi $(LB)\svx1.lib $(MISC)\svx1.def

implib2: $(MISC)\svx2.def
    implib /noi $(LB)\svx2.lib $(MISC)\svx2.def

implib3: $(MISC)\dl1.def
    implib /noi $(LB)\dl1.lib $(MISC)\dl1.def

implib4: $(MISC)\dl2.def
    implib /noi $(LB)\dl2.lib $(MISC)\dl2.def

implib_defs: $(SHL1DEF) $(SHL2DEF)
    +-$(RM) $(MISC)$/svx1.def
    +-$(RM) $(MISC)$/svx2.def
    +-$(RM) $(MISC)$/dl1.def
    +-$(RM) $(MISC)$/dl2.def
    splitdef $(SHL1DEF) $(MISC)$/svx1.def $(MISC)$/svx2.def
    splitdef $(SHL2DEF) $(MISC)$/dl1.def $(MISC)$/dl2.def


