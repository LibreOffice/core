#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: csaba $ $Date: 2000-10-16 14:50:27 $
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
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
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

PRJNAME=BASCTL
TARGET=basctl
#svx.hid generieren
GEN_HID=TRUE
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------

# --- Basctl - DLL ----------

.IF "$(header)" == ""

HELPIDFILES=    ..$/inc$/helpid.hrc

SHL1TARGET= basctl$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= basctl
SHL1BASE  = 0x1d800000
SHL1STDLIBS= \
            $(SVXLIB) \
            $(SALLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
             $(SVLLIB)	\
            $(VCLLIB) \
            $(SO2LIB) \
            $(SOTLIB) \
            $(SFX2LIB) \
            $(BASICLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
            $(SHELLLIB)
.ENDIF

.IF "$(TF_UCB)" == ""
SHL1STDLIBS+=\
        $(CHAOSLIB)
.ENDIF

.IF "$(SOLAR_JAVA)" != ""
SHL1STDLIBS+=\
        $(SJLIB)
.ENDIF


SHL1LIBS=       $(SLB)$/basctl.lib
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME        =$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME=basctl
DEF1DES		= Rtf, Edt, Outliner, SvDraw, Form, Fmcomp, Engine3D, MSFilter
DEF1EXPORTFILE	= basctl.dxp

LIB1TARGET      =$(SLB)$/basctl.lib
LIB1FILES       = \
            $(LIBPRE) $(SLB)$/vcdlged.lib   \
            $(LIBPRE) $(SLB)$/vctrls.lib    \
            $(LIBPRE) $(SLB)$/basicide.lib \
            $(LIBPRE) $(SLB)$/ibrw.lib

SRSFILELIST=\
                $(SRS)$/vctrls.srs \
                $(SRS)$/basicide.srs \
                $(SRS)$/ibrw.srs
                
SRSFILELIST+=   $(SOLARVERSION)$/$(INPATH)$/res$(UPDMINOREXT)$/sfx.srs

RESLIB1NAME=basctl
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk


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

$(SRS)$/hidother.hid: hidother.src
.IF "$(GUI)$(CPU)"!="WNTI" || "$(product)"!="full"
    @echo nix
.ELSE
.IF "$(BUILD_SOSL)" == ""
    @+echo NO HIDS!!!
    +-mhids hidother.src ..\$(INPATH)$/srs $(INCLUDE) hidother
.ENDIF
.ENDIF


.ENDIF                  # "$(header)" == ""

