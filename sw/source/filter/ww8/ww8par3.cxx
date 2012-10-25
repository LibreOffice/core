/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <svl/itemiter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#include <com/sun/star/form/XFormController.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XConnectableShape.hpp>
#include <com/sun/star/drawing/XConnectorShape.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XShapeAligner.hpp>
#include <com/sun/star/drawing/XShapeGroup.hpp>
#include <com/sun/star/drawing/XUniversalShapeDescriptor.hpp>
#include <com/sun/star/drawing/XShapeMirror.hpp>
#include <com/sun/star/drawing/XShapeArranger.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/stlunosequence.hxx>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>
#include <functional>
#include <hintids.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <unotextrange.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <charatr.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <expfld.hxx>
#include <fmtfld.hxx>
#include <flddropdown.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"  // wg. Listen-Attributen in Styles

#include <IMark.hxx>
#include <unotools/fltrcfg.hxx>
#include <xmloff/odffields.hxx>

#include <stdio.h>

using namespace com::sun::star;
using namespace sw::util;
using namespace sw::types;
using namespace sw::mark;

//-----------------------------------------
//            UNO-Controls
//-----------------------------------------

//cmc, OCX i.e. word 97 form controls
eF_ResT SwWW8ImplReader::Read_F_OCX( WW8FieldDesc*, String& )
{
    if( bObj && nPicLocFc )
        nObjLocFc = nPicLocFc;
    bEmbeddObj = true;
    return FLD_TEXT;
}

eF_ResT SwWW8ImplReader::Read_F_FormTextBox( WW8FieldDesc* pF, String& rStr )
{
    WW8FormulaEditBox aFormula(*this);

    if (0x01 == rStr.GetChar(writer_cast<xub_StrLen>(pF->nLCode-1))) {
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_EDIT);
    }

    /*
    Here we have a small complication. This formula control contains
    the default text that is displayed if you edit the form field in
    the "default text" area. But MSOffice does not display that
    information, instead it display the result of the field,
    MSOffice just uses the default text of the control as its
    initial value for the displayed default text. So we will swap in
    the field result into the formula here in place of the default
    text.
    */

    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    sal_Bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields) {
    aFormula.sDefault = GetFieldResult(pF);

    SwInputField aFld((SwInputFieldType*)rDoc.GetSysFldType( RES_INPUTFLD ),
              aFormula.sDefault , aFormula.sTitle , INP_TXT, 0 );
    aFld.SetHelp(aFormula.sHelp);
    aFld.SetToolTip(aFormula.sToolTip);

    rDoc.InsertPoolItem(*pPaM, SwFmtFld(aFld), 0);
    return FLD_OK;
    } else {
    WW8PLCFx_Book* pB = pPlcxMan->GetBook();
    String aBookmarkName;
    if (pB!=NULL) {
        WW8_CP currentCP=pF->nSCode;
        WW8_CP currentLen=pF->nLen;

        sal_uInt16 bkmFindIdx;
        String aBookmarkFind=pB->GetBookmark(currentCP-1, currentCP+currentLen-1, bkmFindIdx);

        if (aBookmarkFind.Len()>0) {
            pB->SetStatus(bkmFindIdx, BOOK_FIELD); // mark bookmark as consumed, such that tl'll not get inserted as a "normal" bookmark again
            if (aBookmarkFind.Len()>0) {
                aBookmarkName=aBookmarkFind;
            }
        }
    }

    if (pB!=NULL && aBookmarkName.Len()==0) {
        aBookmarkName=pB->GetUniqueBookmarkName(aFormula.sTitle);
    }


    if (aBookmarkName.Len()>0) {
        maFieldStack.back().SetBookmarkName(aBookmarkName);
        maFieldStack.back().SetBookmarkType(ODF_FORMTEXT);
        maFieldStack.back().getParameters()["Description"] = uno::makeAny(::rtl::OUString(aFormula.sToolTip));
        maFieldStack.back().getParameters()["Name"] = uno::makeAny(::rtl::OUString(aFormula.sTitle));
    }
    return FLD_TEXT;
    }
}

eF_ResT SwWW8ImplReader::Read_F_FormCheckBox( WW8FieldDesc* pF, String& rStr )
{
    WW8FormulaCheckBox aFormula(*this);

    if (!pFormImpl)
        pFormImpl = new SwMSConvertControls(mpDocShell, pPaM);

    if (0x01 == rStr.GetChar(writer_cast<xub_StrLen>(pF->nLCode-1)))
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_CHECKBOX);
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    sal_Bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields) {
    pFormImpl->InsertFormula(aFormula);
    return FLD_OK;
    } else {
    String aBookmarkName;
    WW8PLCFx_Book* pB = pPlcxMan->GetBook();
    if (pB!=NULL) {
        WW8_CP currentCP=pF->nSCode;
        WW8_CP currentLen=pF->nLen;

        sal_uInt16 bkmFindIdx;
        String aBookmarkFind=pB->GetBookmark(currentCP-1, currentCP+currentLen-1, bkmFindIdx);

        if (aBookmarkFind.Len()>0) {
            pB->SetStatus(bkmFindIdx, BOOK_FIELD); // mark as consumed by field
            if (aBookmarkFind.Len()>0) {
                aBookmarkName=aBookmarkFind;
            }
        }
    }

    if (pB!=NULL && aBookmarkName.Len()==0) {
        aBookmarkName=pB->GetUniqueBookmarkName(aFormula.sTitle);
    }

    if (aBookmarkName.Len()>0)
    {
        IDocumentMarkAccess* pMarksAccess = rDoc.getIDocumentMarkAccess( );
        IFieldmark* pFieldmark = dynamic_cast<IFieldmark*>( pMarksAccess->makeNoTextFieldBookmark(
                *pPaM, aBookmarkName, ODF_FORMCHECKBOX ) );
        OSL_ENSURE(pFieldmark!=NULL, "hmmm; why was the bookmark not created?");
        if (pFieldmark!=NULL) {
            IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
            ICheckboxFieldmark* pCheckboxFm = dynamic_cast<ICheckboxFieldmark*>(pFieldmark);
            (*pParameters)[ODF_FORMCHECKBOX_NAME] = uno::makeAny(::rtl::OUString(aFormula.sTitle));
            (*pParameters)[ODF_FORMCHECKBOX_HELPTEXT] = uno::makeAny(::rtl::OUString(aFormula.sToolTip));

            if(pCheckboxFm)
                pCheckboxFm->SetChecked(aFormula.nChecked);
            // set field data here...
        }
    }
    return FLD_OK;
    }
}

eF_ResT SwWW8ImplReader::Read_F_FormListBox( WW8FieldDesc* pF, String& rStr)
{
    WW8FormulaListBox aFormula(*this);

    if (0x01 == rStr.GetChar(writer_cast<xub_StrLen>(pF->nLCode-1)))
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_DROPDOWN);

    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    sal_Bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields)
    {
        SwDropDownField aFld((SwDropDownFieldType*)rDoc.GetSysFldType(RES_DROPDOWN));

        aFld.SetName(aFormula.sTitle);
        aFld.SetHelp(aFormula.sHelp);
        aFld.SetToolTip(aFormula.sToolTip);

        if (!aFormula.maListEntries.empty())
        {
            aFld.SetItems(aFormula.maListEntries);
            int nIndex = aFormula.fDropdownIndex  < aFormula.maListEntries.size() ? aFormula.fDropdownIndex : 0;
            aFld.SetSelectedItem(aFormula.maListEntries[nIndex]);
        }

        rDoc.InsertPoolItem(*pPaM, SwFmtFld(aFld), 0);
        return FLD_OK;
    }
    else
    {
        // TODO: review me
        String aBookmarkName;
        WW8PLCFx_Book* pB = pPlcxMan->GetBook();
        if (pB!=NULL)
        {
            WW8_CP currentCP=pF->nSCode;
            WW8_CP currentLen=pF->nLen;

            sal_uInt16 bkmFindIdx;
            String aBookmarkFind=pB->GetBookmark(currentCP-1, currentCP+currentLen-1, bkmFindIdx);

            if (aBookmarkFind.Len()>0)
            {
                pB->SetStatus(bkmFindIdx, BOOK_FIELD); // mark as consumed by field
                if (aBookmarkFind.Len()>0)
                    aBookmarkName=aBookmarkFind;
            }
        }

        if (pB!=NULL && aBookmarkName.Len()==0)
            aBookmarkName=pB->GetUniqueBookmarkName(aFormula.sTitle);

        if (aBookmarkName.Len()>0)
        {
            IDocumentMarkAccess* pMarksAccess = rDoc.getIDocumentMarkAccess( );
            IFieldmark *pFieldmark = dynamic_cast<IFieldmark*>(
                    pMarksAccess->makeNoTextFieldBookmark( *pPaM, aBookmarkName, ODF_FORMDROPDOWN ) );
            OSL_ENSURE(pFieldmark!=NULL, "hmmm; why was the bookmark not created?");
            if ( pFieldmark != NULL )
            {
                uno::Sequence< ::rtl::OUString > vListEntries(aFormula.maListEntries.size());
                ::std::copy(aFormula.maListEntries.begin(), aFormula.maListEntries.end(), ::comphelper::stl_begin(vListEntries));
                (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] = uno::makeAny(vListEntries);
                sal_Int32 nIndex = aFormula.fDropdownIndex  < aFormula.maListEntries.size() ? aFormula.fDropdownIndex : 0;
                (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_RESULT] = uno::makeAny(nIndex);
                // set field data here...
            }
        }

        return FLD_OK;
    }
}

eF_ResT SwWW8ImplReader::Read_F_HTMLControl(WW8FieldDesc*, String&)
{
    if( bObj && nPicLocFc )
        nObjLocFc = nPicLocFc;
    bEmbeddObj = true;
    return FLD_TEXT;
}

void SwWW8ImplReader::DeleteFormImpl()
{
    delete pFormImpl, pFormImpl = 0;
}

//----------------------------------------------------------------------------
//          WW8ListManager          oeffentliche Methoden stehen ganz am Ende
//------------------------- ============ --------------- ============ --------



// Hilfs-Deklarationen ///////////////////////////////////////////////////////
//
// Style Id's for each level
typedef sal_uInt16 WW8aIdSty[WW8ListManager::nMaxLevel];
// Zeichenattribute aus GrpprlChpx
typedef SfxItemSet* WW8aISet[WW8ListManager::nMaxLevel];
// Zeichen Style Pointer
typedef SwCharFmt* WW8aCFmt[WW8ListManager::nMaxLevel];

struct WW8LST   // nur DIE Eintraege, die WIR benoetigen!
{
    WW8aIdSty aIdSty;     // Style Id's for each level,
                            //   nIStDNil if no style linked
    sal_uInt32 nIdLst;     // Unique List ID
    sal_uInt32 nTplC;      // Unique template code - Was ist das bloss?
    sal_uInt8 bSimpleList:1; // Flag: Liste hat nur EINEN Level
    sal_uInt8 bRestartHdn:1; // WW6-Kompatibilitaets-Flag:
                                                        //   true if the list should start numbering over
};                                                      //   at the beginning of each section

const sal_uInt32 cbLSTF=28;

struct WW8LFO   // nur DIE Eintraege, die WIR benoetigen!
{
    SwNumRule*      pNumRule;   // Parent NumRule
    sal_uInt32      nIdLst;     // Unique List ID
    sal_uInt8       nLfoLvl;    // count of levels whose format is overridden
    bool bSimpleList;
};

struct WW8LVL   // nur DIE Eintraege, die WIR benoetigen!
{
    sal_Int32 nStartAt;       // start at value for this value
    sal_Int32 nV6DxaSpace;// Ver6-Compatible: min Space between Num anf text::Paragraph
    sal_Int32 nV6Indent;  // Ver6-Compatible: Breite des Prefix Textes; ggfs. zur
                        // Definition d. Erstzl.einzug nutzen!
    // Absatzattribute aus GrpprlPapx
    sal_uInt16  nDxaLeft;               // linker Einzug
    short   nDxaLeft1;          // Erstzeilen-Einzug

    sal_uInt8   nNFC;               // number format code
    // Offset der Feldkodes im Num-X-String
    sal_uInt8   aOfsNumsXCH[WW8ListManager::nMaxLevel];
    sal_uInt8   nLenGrpprlChpx; // length, in bytes, of the LVL's grpprlChpx
    sal_uInt8   nLenGrpprlPapx; // length, in bytes, of the LVL's grpprlPapx
    sal_uInt8   nAlign: 2;  // alignment (left, right, centered) of the number
    sal_uInt8 bLegal:    1;  // egal
    sal_uInt8 bNoRest:1; // egal
    sal_uInt8 bV6Prev:1; // Ver6-Compatible: number will include previous levels
    sal_uInt8 bV6PrSp:1; // Ver6-Compatible: egal
    sal_uInt8 bV6:       1;  // falls true , beachte die V6-Compatible Eintraege!
    sal_uInt8   bDummy: 1;  // (macht das Byte voll)

};

struct WW8LFOLVL
{
    sal_Int32 nStartAt;          // start-at value if bFormat==false and bStartAt == true
                                            // (if bFormat==true, the start-at is stored in the LVL)
    sal_uInt8 nLevel;               // the level to be overridden
    // dieses Byte ist _absichtlich_ nicht in das folgende Byte hineingepackt   !!
    // (siehe Kommentar unten bei struct WW8LFOInfo)

    sal_uInt8 bStartAt :1;       // true if the start-at value is overridden
    sal_uInt8 bFormat :1;        // true if the formatting is overriden

    WW8LFOLVL() :
        nStartAt(1), nLevel(0), bStartAt(1), bFormat(0) {}
};

// in den ListenInfos zu speichernde Daten ///////////////////////////////////
//
struct WW8LSTInfo   // sortiert nach nIdLst (in WW8 verwendete Listen-Id)
{
    std::vector<ww::bytes> maParaSprms;
    WW8aIdSty   aIdSty;          // Style Id's for each level
    WW8aISet    aItemSet;        // Zeichenattribute aus GrpprlChpx
    WW8aCFmt    aCharFmt;        // Zeichen Style Pointer

    SwNumRule*  pNumRule;        // Zeiger auf entsprechende Listenvorlage im Writer
    sal_uInt32      nIdLst;          // WW8Id dieser Liste
    sal_uInt8 bSimpleList:1;// Flag, ob diese NumRule nur einen Level verwendet
    sal_uInt8 bUsedInDoc :1;// Flag, ob diese NumRule im Doc verwendet wird,
                                                     //   oder beim Reader-Ende geloescht werden sollte

    WW8LSTInfo(SwNumRule* pNumRule_, WW8LST& aLST)
        : pNumRule(pNumRule_), nIdLst(aLST.nIdLst),
        bSimpleList(aLST.bSimpleList), bUsedInDoc(0)
    {
        memcpy( aIdSty, aLST.aIdSty, sizeof( aIdSty   ));
        memset(&aItemSet, 0,  sizeof( aItemSet ));
        memset(&aCharFmt, 0,  sizeof( aCharFmt ));
    }

};

// in den ListenFormatOverrideInfos zu speichernde Daten /////////////////////
//
struct WW8LFOInfo   // unsortiert, d.h. Reihenfolge genau wie im WW8 Stream
{
    std::vector<ww::bytes> maParaSprms;
    std::vector<WW8LFOLVL> maOverrides;
    SwNumRule* pNumRule;         // Zeiger auf entsprechende Listenvorlage im Writer
                                                     // entweder: Liste in LSTInfos oder eigene Liste
                                                     // (im Ctor erstmal die aus den LSTInfos merken)

    sal_uInt32  nIdLst;          // WW8-Id der betreffenden Liste
    sal_uInt8   nLfoLvl;             // count of levels whose format is overridden
    // Ja, ich natuerlich koennten wir nLfoLvl (mittels :4) noch in das folgende
    // Byte mit hineinpacken, doch waere das eine ziemliche Fehlerquelle,
    // an dem Tag, wo MS ihr Listenformat auf mehr als 15 Level aufbohren.

    sal_uInt8 bOverride  :1;// Flag, ob die NumRule nicht in maLSTInfos steht,
                                                     //   sondern fuer pLFOInfos NEU angelegt wurde
    sal_uInt8 bSimpleList:1;// Flag, ob diese NumRule nur einen Level verwendet
    sal_uInt8 bUsedInDoc :1;// Flag, ob diese NumRule im Doc verwendet wird,
                                                     //   oder beim Reader-Ende geloescht werden sollte
    sal_uInt8 bLSTbUIDSet    :1;// Flag, ob bUsedInDoc in maLSTInfos gesetzt wurde,
                                                     //   und nicht nochmals gesetzt zu werden braucht
    WW8LFOInfo(const WW8LFO& rLFO);
};

WW8LFOInfo::WW8LFOInfo(const WW8LFO& rLFO)
    : maParaSprms(WW8ListManager::nMaxLevel),
    maOverrides(WW8ListManager::nMaxLevel), pNumRule(rLFO.pNumRule),
    nIdLst(rLFO.nIdLst), nLfoLvl(rLFO.nLfoLvl),
    bOverride(rLFO.nLfoLvl ? true : false), bSimpleList(rLFO.bSimpleList),
    bUsedInDoc(0), bLSTbUIDSet(0)
{
}



// Hilfs-Methoden ////////////////////////////////////////////////////////////
//

// finden der Sprm-Parameter-Daten, falls Sprm im Grpprl enthalten
sal_uInt8* WW8ListManager::GrpprlHasSprm(sal_uInt16 nId, sal_uInt8& rSprms,
    sal_uInt8 nLen)
{
    return maSprmParser.findSprmData(nId, &rSprms, nLen);
}

class ListWithId : public std::unary_function<const WW8LSTInfo *, bool>
{
private:
    sal_uInt32 mnIdLst;
public:
    explicit ListWithId(sal_uInt32 nIdLst) : mnIdLst(nIdLst) {}
    bool operator() (const WW8LSTInfo *pEntry) const
        { return (pEntry->nIdLst == mnIdLst); }
};

// Zugriff ueber die List-Id des LST Eintrags
WW8LSTInfo* WW8ListManager::GetLSTByListId( sal_uInt32 nIdLst ) const
{
    std::vector<WW8LSTInfo *>::const_iterator aResult =
        std::find_if(maLSTInfos.begin(),maLSTInfos.end(),ListWithId(nIdLst));
    if (aResult == maLSTInfos.end())
        return 0;
    return *aResult;
}

static void lcl_CopyGreaterEight(String &rDest, String &rSrc,
    xub_StrLen nStart, xub_StrLen nLen = STRING_LEN)
{
    if (nLen > rSrc.Len() || nLen == STRING_LEN)
        nLen = rSrc.Len();
    for (xub_StrLen nI = nStart; nI < nLen; ++nI)
    {
        sal_Unicode nChar = rSrc.GetChar(nI);
        if (nChar > WW8ListManager::nMaxLevel)
            rDest.Append(nChar);
    }
}

bool WW8ListManager::ReadLVL(SwNumFmt& rNumFmt, SfxItemSet*& rpItemSet,
    sal_uInt16 nLevelStyle, bool bSetStartNo,
    std::deque<bool> &rNotReallyThere, sal_uInt16 nLevel,
    ww::bytes &rParaSprms)
{
    sal_uInt8       aBits1(0);
    sal_uInt16      nStartNo(0);        // Start-Nr. fuer den Writer
    SvxExtNumType   eType;              // Writer-Num-Typ
    SvxAdjust       eAdj;               // Ausrichtung (Links/rechts/zent.)
    sal_Unicode     cBullet(0x2190);    // default safe bullet
    String          sPrefix;
    String          sPostfix;
    WW8LVL          aLVL;
    //
    // 1. LVLF einlesen
    //
    memset(&aLVL, 0, sizeof( aLVL ));
    rSt >> aLVL.nStartAt;
    rSt >> aLVL.nNFC;
    rSt >> aBits1;
    if( 0 != rSt.GetError() ) return false;
    aLVL.nAlign = (aBits1 & 0x03);
    if( aBits1 & 0x10 ) aLVL.bV6Prev    = true;
    if( aBits1 & 0x20 ) aLVL.bV6PrSp    = true;
    if( aBits1 & 0x40 ) aLVL.bV6        = true;
    bool bLVLOkB = true;
    sal_uInt8 nLevelB = 0;
    for(nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
    {
        rSt >> aLVL.aOfsNumsXCH[ nLevelB ];
        if( 0 != rSt.GetError() )
        {
            bLVLOkB = false;
            break;
        }
    }

    if( !bLVLOkB )
        return false;

    sal_uInt8 ixchFollow(0);
    rSt >> ixchFollow;
    rSt >> aLVL.nV6DxaSpace;
    rSt >> aLVL.nV6Indent;
    rSt >> aLVL.nLenGrpprlChpx;
    rSt >> aLVL.nLenGrpprlPapx;
    rSt.SeekRel( 2 );
    if( 0 != rSt.GetError()) return false;

    //
    // 2. ggfs. PAPx einlesen und nach Einzug-Werten suchen
    //
    short nTabPos = 0; // #i86652# - read tab setting
    if( aLVL.nLenGrpprlPapx )
    {
        sal_uInt8 aGrpprlPapx[ 255 ];
        if(aLVL.nLenGrpprlPapx != rSt.Read(&aGrpprlPapx,aLVL.nLenGrpprlPapx))
            return false;
        // "sprmPDxaLeft"  pap.dxaLeft;dxa;word;
        sal_uInt8* pSprm;
        if (
            (0 != (pSprm = GrpprlHasSprm(0x840F,aGrpprlPapx[0],aLVL.nLenGrpprlPapx))) ||
            (0 != (pSprm = GrpprlHasSprm(0x845E,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)))
            )
        {
            sal_uInt8 *pBegin = pSprm-2;
            for(int i=0;i<4;++i)
                rParaSprms.push_back(*pBegin++);
            short nDxaLeft = SVBT16ToShort( pSprm );
            aLVL.nDxaLeft = (0 < nDxaLeft) ? (sal_uInt16)nDxaLeft
                            : (sal_uInt16)(-nDxaLeft);
        }

        // "sprmPDxaLeft1" pap.dxaLeft1;dxa;word;
        if (
            (0 != (pSprm = GrpprlHasSprm(0x8411,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) ) ||
            (0 != (pSprm = GrpprlHasSprm(0x8460,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) )
            )
        {
            sal_uInt8 *pBegin = pSprm-2;
            for(int i=0;i<4;++i)
                rParaSprms.push_back(*pBegin++);
            aLVL.nDxaLeft1 = SVBT16ToShort(  pSprm );
        }

        // #i86652# - read tab setting
        if(0 != (pSprm = GrpprlHasSprm(0xC615,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) )
        {
            bool bDone = false;
            if (*(pSprm-1) == 5)
            {
                if (*pSprm++ == 0) //nDel
                {
                    if (*pSprm++ == 1) //nIns
                    {
                        nTabPos = SVBT16ToShort(pSprm);
                        pSprm+=2;
                        if (*pSprm == 6) //type
                        {
                            bDone = true;
                        }
                    }
                }
            }
            OSL_ENSURE(bDone, "tab setting in numbering is "
                "of unexpected configuration");
            (void)bDone;
        }
        if ( rNumFmt.GetPositionAndSpaceMode() ==
                                  SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
        {
            // If there is a tab setting with a larger value, then use that.
            // Ideally we would allow tabs to be used in numbering fields and set
            // this on the containing paragraph which would make it actually work
            // most of the time.
            if ( nTabPos != 0 )
            {
                const sal_uInt16 nDesired = aLVL.nDxaLeft + aLVL.nDxaLeft1;

                bool bDoAdjust = false;
                if ( nDesired < aLVL.nDxaLeft )
                {
                    if ( nDesired < nTabPos && nTabPos < aLVL.nDxaLeft )
                    {
                        bDoAdjust = true;
                    }
                }
                else
                {
                    if ( aLVL.nDxaLeft < nTabPos && nTabPos < nDesired )
                    {
                        bDoAdjust = true;
                    }
                }

                if (bDoAdjust)
                {
                    aLVL.nDxaLeft = (0 < nTabPos)
                                    ? (sal_uInt16)nTabPos
                                    : (sal_uInt16)(-nTabPos);

                    aLVL.nDxaLeft1 = nDesired - aLVL.nDxaLeft;
                }
            }
        }
    }
    //
    // 3. ggfs. CHPx einlesen und
    //
    if( aLVL.nLenGrpprlChpx )
    {
        sal_uInt8 aGrpprlChpx[ 255 ];
        memset(&aGrpprlChpx, 0, sizeof( aGrpprlChpx ));
        if(aLVL.nLenGrpprlChpx != rSt.Read(&aGrpprlChpx, aLVL.nLenGrpprlChpx))
            return false;
        // neues ItemSet fuer die Zeichenattribute anlegen
        rpItemSet = new SfxItemSet( rDoc.GetAttrPool(), RES_CHRATR_BEGIN,
            RES_CHRATR_END - 1 );

        // Reader-ItemSet-Pointer darauf zeigen lassen
        rReader.SetAktItemSet( rpItemSet );
        // Reader-Style auf den Style dieses Levels setzen
        sal_uInt16 nOldColl = rReader.GetNAktColl();
        sal_uInt16 nNewColl = nLevelStyle;
        if (ww::stiNil == nNewColl)
            nNewColl = 0;
        rReader.SetNAktColl( nNewColl );

        // Nun den GrpprlChpx einfach durchnudeln: die Read_xy() Methoden
        // in WW8PAR6.CXX rufen ganz normal ihr NewAttr() oder GetFmtAttr()
        // und diese merken am besetzten Reader-ItemSet-Pointer, dass dieser
        // spezielle ItemSet relevant ist - und nicht ein Stack oder Style!
        sal_uInt16 nOldFlags1 = rReader.GetToggleAttrFlags();
        sal_uInt16 nOldFlags2 = rReader.GetToggleBiDiAttrFlags();

        WW8SprmIter aSprmIter(&aGrpprlChpx[0], aLVL.nLenGrpprlChpx,
            maSprmParser);
        while (const sal_uInt8* pSprm = aSprmIter.GetSprms())
        {
            rReader.ImportSprm(pSprm);
            aSprmIter.advance();
        }

        // Reader-ItemSet-Pointer und Reader-Style zuruecksetzen
        rReader.SetAktItemSet( 0 );
        rReader.SetNAktColl( nOldColl );
        rReader.SetToggleAttrFlags(nOldFlags1);
        rReader.SetToggleBiDiAttrFlags(nOldFlags2);
    }
    //
    // 4. den Nummerierungsstring einlesen: ergibt Prefix und Postfix
    //
    String sNumString(read_uInt16_PascalString(rSt));

    //
    // 5. gelesene Werte in Writer Syntax umwandeln
    //
    if( 0 <= aLVL.nStartAt )
        nStartNo = (sal_uInt16)aLVL.nStartAt;

    switch( aLVL.nNFC )
    {
        case 0:
            eType = SVX_NUM_ARABIC;
            break;
        case 1:
            eType = SVX_NUM_ROMAN_UPPER;
            break;
        case 2:
            eType = SVX_NUM_ROMAN_LOWER;
            break;
        case 3:
            eType = SVX_NUM_CHARS_UPPER_LETTER_N;
            break;
        case 4:
            eType = SVX_NUM_CHARS_LOWER_LETTER_N;
            break;
        case 5:
            // eigentlich: ORDINAL
            eType = SVX_NUM_ARABIC;
            break;
        case 23:
        case 25:
            eType = SVX_NUM_CHAR_SPECIAL;
            break;
        case 255:
            eType = SVX_NUM_NUMBER_NONE;
            break;
         default:
            // take default
            eType = SVX_NUM_ARABIC;
            break;
    }

    //If a number level is not going to be used, then record this fact
    if (SVX_NUM_NUMBER_NONE == eType)
        rNotReallyThere[nLevel] = true;

    /*
     If a number level was not used (i.e. is in NotReallyThere), and that
     number level appears at one of the positions in the display string of the
     list, then it effectively is not there at all. So remove that level entry
     from a copy of the aOfsNumsXCH.
    */
    std::vector<sal_uInt8> aOfsNumsXCH;
    typedef std::vector<sal_uInt8>::iterator myIter;
    aOfsNumsXCH.reserve(nMaxLevel);

    for(nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
        aOfsNumsXCH.push_back(aLVL.aOfsNumsXCH[nLevelB]);

    for(nLevelB = 0; nLevelB <= nLevel; ++nLevelB)
    {
        sal_uInt8 nPos = aOfsNumsXCH[nLevelB];
        if (nPos && nPos < sNumString.Len()  && sNumString.GetChar(nPos-1) < nMaxLevel)
        {
            if (rNotReallyThere[nLevelB])
                aOfsNumsXCH[nLevelB] = 0;
        }
    }
    myIter aIter = std::remove(aOfsNumsXCH.begin(), aOfsNumsXCH.end(), 0);
    myIter aEnd = aOfsNumsXCH.end();
    // #i60633# - suppress access on <aOfsNumsXCH.end()>
    if ( aIter != aEnd )
    {
        // Somehow the first removed vector element, at which <aIter>
        // points to, isn't reset to zero.
        // Investigation is needed to clarify why. It seems that only
        // special arrays are handled correctly by this code.
        ++aIter;
        while (aIter != aEnd)
        {
            (*aIter) = 0;
            ++aIter;
        }
    }

    sal_uInt8 nUpperLevel = 0;  // akt. Anzeigetiefe fuer den Writer
    for(nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
    {
        if (!nUpperLevel && !aOfsNumsXCH[nLevelB])
            nUpperLevel = nLevelB;
    }

    // falls kein NULL als Terminierungs-Char kam,
    // ist die Liste voller Indices, d.h. alle Plaetze sind besetzt,
    // also sind alle Level anzuzeigen
    if (!nUpperLevel)
        nUpperLevel = nMaxLevel;

    if (SVX_NUM_CHAR_SPECIAL == eType)
    {
        cBullet = sNumString.Len() ? sNumString.GetChar(0) : 0x2190;

        if (!cBullet)  // unsave control code?
            cBullet = 0x2190;
    }
    else
    {
        /*
        #i173#
        Our aOfsNumsXCH seems generally to be an array that contains the
        offset into sNumString of locations where the numbers should be
        filled in, so if the first "fill in a number" slot is greater than
        1 there is a "prefix" before the number
        */
        //First number appears at
        sal_uInt8 nOneBasedFirstNoIndex = aOfsNumsXCH[0];
        xub_StrLen nFirstNoIndex =
            nOneBasedFirstNoIndex > 0 ? nOneBasedFirstNoIndex -1 : STRING_LEN;
        lcl_CopyGreaterEight(sPrefix, sNumString, 0, nFirstNoIndex);

        //Next number appears at
        if (nUpperLevel)
        {
            sal_uInt8 nOneBasedNextNoIndex = aOfsNumsXCH[nUpperLevel-1];
            xub_StrLen nNextNoIndex =
                nOneBasedNextNoIndex > 0 ? nOneBasedNextNoIndex -1 : STRING_LEN;
            if (nNextNoIndex != STRING_LEN)
                ++nNextNoIndex;
            if (sNumString.Len() > nNextNoIndex)
                lcl_CopyGreaterEight(sPostfix, sNumString, nNextNoIndex);
        }

        /*
         We use lcl_CopyGreaterEight because once if we have removed unused
         number indexes from the aOfsNumsXCH then placeholders remain in
         sNumString which must not be copied into the final numbering strings
        */
    }

    switch( aLVL.nAlign )
    {
        case 0:
            eAdj = SVX_ADJUST_LEFT;
            break;
        case 1:
            eAdj = SVX_ADJUST_CENTER;
            break;
        case 2:
            eAdj = SVX_ADJUST_RIGHT;
            break;
        case 3:
            // Writer here cannot do block justification
            eAdj = SVX_ADJUST_LEFT;
            break;
         default:
            // undefied value
            OSL_ENSURE( !this, "Value of aLVL.nAlign is not supported" );
            // take default
            eAdj = SVX_ADJUST_LEFT;
            break;
    }

    // 6. entsprechendes NumFmt konfigurieren
    if( bSetStartNo )
        rNumFmt.SetStart( nStartNo );
    rNumFmt.SetNumberingType( static_cast< sal_Int16 >(eType) );
    rNumFmt.SetNumAdjust( eAdj );

    if( SVX_NUM_CHAR_SPECIAL == eType )
    {
        // first character of the Prefix-Text is the Bullet
        rNumFmt.SetBulletChar(cBullet);
        // Don't forget: unten, nach dem Bauen eventueller Styles auch noch
        // SetBulletFont() rufen !!!
    }
    else
    {
        // reminder: Garnix ist default Prefix
        if( sPrefix.Len() )
            rNumFmt.SetPrefix( sPrefix );
        // reminder: Point is default Postfix
        rNumFmt.SetSuffix( sPostfix );
        rNumFmt.SetIncludeUpperLevels( nUpperLevel );
    }

    // #i89181#
    if ( rNumFmt.GetPositionAndSpaceMode() ==
                              SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        if (eAdj == SVX_ADJUST_RIGHT)
        {
            rNumFmt.SetAbsLSpace(aLVL.nDxaLeft);
            rNumFmt.SetFirstLineOffset(-aLVL.nDxaLeft);
            rNumFmt.SetCharTextDistance(-aLVL.nDxaLeft1);
        }
        else
        {
            rNumFmt.SetAbsLSpace( aLVL.nDxaLeft );
            rNumFmt.SetFirstLineOffset(aLVL.nDxaLeft1);
        }
    }
    else
    {
        rNumFmt.SetIndentAt( aLVL.nDxaLeft );
        rNumFmt.SetFirstLineIndent(aLVL.nDxaLeft1);
        if ( !aLVL.bV6 )
            rNumFmt.SetListtabPos( nTabPos );
        else
            rNumFmt.SetListtabPos( aLVL.nV6Indent );
        SvxNumberFormat::LabelFollowedBy eNumLabelFollowedBy = SvxNumberFormat::LISTTAB;
        switch ( ixchFollow )
        {
            case 0:
            {
                eNumLabelFollowedBy = SvxNumberFormat::LISTTAB;
            }
            break;
            case 1:
            {
                eNumLabelFollowedBy = SvxNumberFormat::SPACE;
            }
            break;
            case 2:
            {
                eNumLabelFollowedBy = SvxNumberFormat::NOTHING;
            }
            break;
        }
        rNumFmt.SetLabelFollowedBy( eNumLabelFollowedBy );
    }

    return true;
}

void WW8ListManager::AdjustLVL( sal_uInt8 nLevel, SwNumRule& rNumRule,
    WW8aISet& rListItemSet, WW8aCFmt& rCharFmt, bool& bNewCharFmtCreated,
    String sPrefix )
{
    bNewCharFmtCreated = false;
    SfxItemSet* pThisLevelItemSet;
    SfxItemSet* pLowerLevelItemSet;
    sal_uInt8        nIdenticalItemSetLevel;
    const SfxPoolItem* pItem;

    SwNumFmt aNumFmt  = rNumRule.Get( nLevel );

    pThisLevelItemSet = rListItemSet[ nLevel ];

    if( pThisLevelItemSet && pThisLevelItemSet->Count())
    {
        nIdenticalItemSetLevel = nMaxLevel;
        SfxItemIter aIter( *pThisLevelItemSet );
        for (sal_uInt8 nLowerLevel = 0; nLowerLevel < nLevel; ++nLowerLevel)
        {
            pLowerLevelItemSet = rListItemSet[ nLowerLevel ];
            if(     pLowerLevelItemSet
                && (pLowerLevelItemSet->Count() == pThisLevelItemSet->Count()) )
            {
                nIdenticalItemSetLevel = nLowerLevel;
                sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                while (true)
                {
                    if(  // ggfs. passenden pItem im pLowerLevelItemSet finden
                         (SFX_ITEM_SET != pLowerLevelItemSet->GetItemState(
                                            nWhich, false, &pItem ) )
                        || // virtuellen "!=" Operator anwenden
                         (*pItem != *aIter.GetCurItem() ) )
                    // falls kein Item mit gleicher nWhich gefunden oder Werte
                    // der Items ungleich, Ungleichheit merken und abbrechen!
                    {
                        nIdenticalItemSetLevel = nMaxLevel;
                        break;
                    }
                    if( aIter.IsAtEnd() )
                        break;
                    nWhich = aIter.NextItem()->Which();
                }

                if( nIdenticalItemSetLevel != nMaxLevel )
                    break;
            }
        }

        SwCharFmt* pFmt;
        if (nMaxLevel == nIdenticalItemSetLevel)
        {
            // Style definieren
            String aName( sPrefix.Len() ? sPrefix : rNumRule.GetName() );
            (aName += 'z') += String::CreateFromInt32( nLevel );

            // const Wegcasten
            pFmt = rDoc.MakeCharFmt(aName, (SwCharFmt*)rDoc.GetDfltCharFmt());
            bNewCharFmtCreated = true;
            // Attribute reinsetzen
            pFmt->SetFmtAttr( *pThisLevelItemSet );
        }
        else
        {
            // passenden Style hier anhaengen
            pFmt = rCharFmt[ nIdenticalItemSetLevel ];
        }

        // merken
        rCharFmt[ nLevel ] = pFmt;

        //
        // Style an das NumFormat haengen
        //
        aNumFmt.SetCharFmt( pFmt );
    }
    //
    // ggfs. Bullet Font an das NumFormat haengen
    //
    if( SVX_NUM_CHAR_SPECIAL == aNumFmt.GetNumberingType() )
    {
        SwCharFmt* pFmt = aNumFmt.GetCharFmt();
        Font aFont;
        if( !pFmt )
        {
            aFont = numfunc::GetDefBulletFont();
        }
        else
        {
            const SvxFontItem& rFontItem = pFmt->GetFont();
            aFont.SetFamily(    rFontItem.GetFamily()     );
            aFont.SetName(      rFontItem.GetFamilyName() );
            aFont.SetStyleName( rFontItem.GetStyleName()  );
            aFont.SetPitch(     rFontItem.GetPitch()      );
            aFont.SetCharSet(   rFontItem.GetCharSet()    );
        }
        aNumFmt.SetBulletFont( &aFont );
    }
    //
    // und wieder rein in die NumRule
    //
    rNumRule.Set(nLevel, aNumFmt);
}

SwNumRule* WW8ListManager::CreateNextRule(bool bSimple)
{
    // wird erstmal zur Bildung des Style Namens genommen
    String sPrefix(rtl::OUString("WW8Num"));
    sPrefix += String::CreateFromInt32(nUniqueList++);
    // #i86652#
    sal_uInt16 nRul =
            rDoc.MakeNumRule( rDoc.GetUniqueNumRuleName(&sPrefix), 0, sal_False,
                              SvxNumberFormat::LABEL_ALIGNMENT );
    SwNumRule* pMyNumRule = rDoc.GetNumRuleTbl()[nRul];
    pMyNumRule->SetAutoRule(false);
    pMyNumRule->SetContinusNum(bSimple);
    return pMyNumRule;
}

// oeffentliche Methoden /////////////////////////////////////////////////////
//
WW8ListManager::WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_)
    : maSprmParser(rReader_.GetFib().GetFIBVersion()), rReader(rReader_),
    rDoc(rReader.GetDoc()), rFib(rReader.GetFib()), rSt(rSt_),
    nUniqueList(1)
{
    // LST und LFO gibts erst ab WW8
    if(    ( 8 > rFib.nVersion )
            || ( rFib.fcPlcfLst == rFib.fcPlfLfo )
            || ( rFib.lcbPlcfLst < 2 )
            || ( rFib.lcbPlfLfo < 2) ) return; // offensichtlich keine Listen da

    // Arrays anlegen
    bool bLVLOk = true;

    nLastLFOPosition = USHRT_MAX;
    long nOriginalPos = rSt.Tell();
    //
    // 1. PLCF LST auslesen und die Listen Vorlagen im Writer anlegen
    //
    bool bOk = checkSeek(rSt, rFib.fcPlcfLst);

    if (!bOk)
        return;

    sal_uInt32 nRemainingPlcfLst = rFib.lcbPlcfLst;

    sal_uInt16 nListCount(0);
    rSt >> nListCount;
    nRemainingPlcfLst -= 2;
    bOk = nListCount > 0;

    if (!bOk)
        return;

    //
    // 1.1 alle LST einlesen
    //
    for (sal_uInt16 nList=0; nList < nListCount; ++nList)
    {
        if (nRemainingPlcfLst < cbLSTF)
            break;

        WW8LST aLST;
        memset(&aLST, 0, sizeof( aLST ));

        //
        // 1.1.1 Daten einlesen
        //
        rSt >> aLST.nIdLst;
        rSt >> aLST.nTplC;
        for (sal_uInt16 nLevel = 0; nLevel < nMaxLevel; ++nLevel)
            rSt >> aLST.aIdSty[ nLevel ];

        sal_uInt8 aBits1(0);
        rSt >> aBits1;

        rSt.SeekRel( 1 );

        if( aBits1 & 0x01 )
            aLST.bSimpleList = true;
        if( aBits1 & 0x02 )
            aLST.bRestartHdn = true;

        // 1.1.2 new NumRule inserted in Doc and  WW8LSTInfo marked

        /*
        #i1869#
        In word 2000 microsoft got rid of creating new "simple lists" with
        only 1 level, all new lists are created with 9 levels. To hack it
        so that the list types formerly known as simple lists still have
        their own tab page to themselves one of the reserved bits is used
        to show that a given list is to be in the simple list tabpage.
        This has now nothing to do with the actual number of list level a
        list has, only how many will be shown in the user interface.

        i.e. create a simple list in 2000 and open it in 97 and 97 will
        claim (correctly) that it is an outline list. We can set our
        continous flag in these lists to store this information.
        */
        SwNumRule* pMyNumRule = CreateNextRule(
            aLST.bSimpleList || (aBits1 & 0x10));

        WW8LSTInfo* pLSTInfo = new WW8LSTInfo(pMyNumRule, aLST);
        maLSTInfos.push_back(pLSTInfo);

        nRemainingPlcfLst -= cbLSTF;
    }

    //
    // 1.2 alle LVL aller aLST einlesen
    //
    sal_uInt16 nLSTInfos = static_cast< sal_uInt16 >(maLSTInfos.size());
    for (sal_uInt16 nList = 0; nList < nLSTInfos; ++nList)
    {
        WW8LSTInfo* pListInfo = maLSTInfos[nList];
        if( !pListInfo || !pListInfo->pNumRule ) break;
        SwNumRule& rMyNumRule = *pListInfo->pNumRule;
        //
        // 1.2.1 betreffende(n) LVL(s) fuer diese aLST einlesen
        //
        sal_uInt16 nLvlCount = static_cast< sal_uInt16 >(pListInfo->bSimpleList ? nMinLevel : nMaxLevel);
        std::deque<bool> aNotReallyThere;
        aNotReallyThere.resize(nMaxLevel);
        pListInfo->maParaSprms.resize(nMaxLevel);
        for (sal_uInt8 nLevel = 0; nLevel < nLvlCount; ++nLevel)
        {
            SwNumFmt aNumFmt( rMyNumRule.Get( nLevel ) );
            // LVLF einlesen
            bLVLOk = ReadLVL( aNumFmt, pListInfo->aItemSet[nLevel],
                pListInfo->aIdSty[nLevel], true, aNotReallyThere, nLevel,
                pListInfo->maParaSprms[nLevel]);
            if( !bLVLOk )
                break;
            // und in die rMyNumRule aufnehmen
            rMyNumRule.Set( nLevel, aNumFmt );
        }
        if( !bLVLOk )
            break;
        //
        // 1.2.2 die ItemPools mit den CHPx Einstellungen der verschiedenen
        //       Level miteinander vergleichen und ggfs. Style(s) erzeugen
        //
        for (sal_uInt8 nLevel = 0; nLevel < nLvlCount; ++nLevel)
        {
            bool bDummy;
            AdjustLVL( nLevel, rMyNumRule, pListInfo->aItemSet,
                                           pListInfo->aCharFmt, bDummy );
        }
        //
        // 1.2.3 ItemPools leeren und loeschen
        //
        for (sal_uInt8 nLevel = 0; nLevel < nLvlCount; ++nLevel)
            delete pListInfo->aItemSet[ nLevel ];
    }

    //
    // 2. PLF LFO auslesen und speichern
    //
    bOk = checkSeek(rSt, rFib.fcPlfLfo);

    if (!bOk)
        return;

    sal_Int32 nLfoCount(0);
    rSt >> nLfoCount;
    bOk = nLfoCount > 0;

    if (!bOk)
        return;

    //
    // 2.1 alle LFO einlesen
    //
    for (sal_uInt16 nLfo = 0; nLfo < nLfoCount; ++nLfo)
    {
        bOk = false;

        if (rSt.IsEof())
            break;

        WW8LFO aLFO;
        memset(&aLFO, 0, sizeof( aLFO ));

        rSt >> aLFO.nIdLst;
        rSt.SeekRel( 8 );
        rSt >> aLFO.nLfoLvl;
        rSt.SeekRel( 3 );
        // soviele Overrides existieren
        if ((nMaxLevel < aLFO.nLfoLvl) || rSt.GetError())
            break;

        // die Parent NumRule der entsprechenden Liste ermitteln
        WW8LSTInfo* pParentListInfo = GetLSTByListId(aLFO.nIdLst);
        if (pParentListInfo)
        {
            // hier, im ersten Schritt, erst mal diese NumRule festhalten
            aLFO.pNumRule = pParentListInfo->pNumRule;

            // hat die Liste mehrere Level ?
            aLFO.bSimpleList = pParentListInfo->bSimpleList;
        }
        // und rein ins Merk-Array mit dem Teil
        WW8LFOInfo* pLFOInfo = new WW8LFOInfo(aLFO);
        if (pParentListInfo)
        {
            //Copy the basic paragraph properties for each level from the
            //original list into the list format override levels.
            int nMaxSize = pParentListInfo->maParaSprms.size();
            pLFOInfo->maParaSprms.resize(nMaxSize);
            for (int i = 0; i < nMaxSize; ++i)
                pLFOInfo->maParaSprms[i] = pParentListInfo->maParaSprms[i];
        }
        pLFOInfos.push_back(pLFOInfo);
        bOk = true;
    }

    if( bOk )
    {
        //
        // 2.2 fuer alle LFO die zugehoerigen LFOLVL einlesen
        //
        size_t nLFOInfos = pLFOInfos.size();
        for (size_t nLfo = 0; nLfo < nLFOInfos; ++nLfo)
        {
            bOk = false;
            WW8LFOInfo& rLFOInfo = pLFOInfos[nLfo];
            // stehen hierfuer ueberhaupt LFOLVL an ?
            if( rLFOInfo.bOverride )
            {
                WW8LSTInfo* pParentListInfo = GetLSTByListId(rLFOInfo.nIdLst);
                if (!pParentListInfo)
                    break;
                //
                // 2.2.1 eine neue NumRule fuer diese Liste anlegen
                //
                SwNumRule* pParentNumRule = rLFOInfo.pNumRule;
                OSL_ENSURE(pParentNumRule, "ww: Impossible lists, please report");
                if( !pParentNumRule )
                    break;
                // Nauemsprefix aufbauen: fuer NumRule-Name (eventuell)
                // und (falls vorhanden) fuer Style-Name (dann auf jeden Fall)
                String sPrefix(rtl::OUString("WW8NumSt"));
                sPrefix += String::CreateFromInt32( nLfo + 1 );
                // jetzt dem pNumRule seinen RICHTIGEN Wert zuweisen !!!
                // (bis dahin war hier die Parent NumRule vermerkt )
                //
                // Dazu erst mal nachsehen, ob ein Style diesen LFO
                // referenziert:
                if( USHRT_MAX > rReader.StyleUsingLFO( nLfo ) )
                {
                    sal_uInt16 nRul = rDoc.MakeNumRule(
                        rDoc.GetUniqueNumRuleName( &sPrefix ), pParentNumRule);
                    rLFOInfo.pNumRule = rDoc.GetNumRuleTbl()[ nRul ];
                    rLFOInfo.pNumRule->SetAutoRule(false);
                }
                else
                {
                    sal_uInt16 nRul = rDoc.MakeNumRule(
                        rDoc.GetUniqueNumRuleName(), pParentNumRule);
                    rLFOInfo.pNumRule = rDoc.GetNumRuleTbl()[ nRul ];
                    rLFOInfo.pNumRule->SetAutoRule(true);  // = default
                }
                //
                // 2.2.2 alle LFOLVL (und ggfs. LVL) fuer die neue NumRule
                // einlesen
                //
                WW8aISet aItemSet;       // Zeichenattribute aus GrpprlChpx
                WW8aCFmt aCharFmt;       // Zeichen Style Pointer
                memset(&aItemSet, 0,  sizeof( aItemSet ));
                memset(&aCharFmt, 0,  sizeof( aCharFmt ));

                //2.2.2.0 skip inter-group of override header ?
                //See #i25438# for why I moved this here, compare
                //that original bugdoc's binary to what it looks like
                //when resaved with word, i.e. there is always a
                //4 byte header, there might be more than one if
                //that header was 0xFFFFFFFF, e.g. #114412# ?
                sal_uInt32 nTest;
                rSt >> nTest;
                do
                {
                    nTest = 0;
                    rSt >> nTest;
                }
                while (nTest == 0xFFFFFFFF);
                rSt.SeekRel(-4);

                std::deque<bool> aNotReallyThere(WW8ListManager::nMaxLevel);
                for (sal_uInt8 nLevel = 0; nLevel < rLFOInfo.nLfoLvl; ++nLevel)
                {
                    WW8LFOLVL aLFOLVL;
                    bLVLOk = false;

                    //
                    // 2.2.2.1 den LFOLVL einlesen
                    //
                    rSt >> aLFOLVL.nStartAt;
                    sal_uInt8 aBits1(0);
                    rSt >> aBits1;
                    rSt.SeekRel( 3 );
                    if (rSt.GetError())
                        break;

                    // beachte: Die Witzbolde bei MS quetschen die
                    // Override-Level-Nummer in vier Bits hinein, damit sie
                    // wieder einen Grund haben, ihr Dateiformat zu aendern,
                    // falls ihnen einfaellt, dass sie eigentlich doch gerne
                    // bis zu 16 Listen-Level haetten.  Wir tun das *nicht*
                    // (siehe Kommentar oben bei "struct
                    // WW8LFOInfo")
                    aLFOLVL.nLevel = aBits1 & 0x0F;
                    if( (0xFF > aBits1) &&
                        (nMaxLevel > aLFOLVL.nLevel) )
                    {
                        if (aBits1 & 0x10)
                            aLFOLVL.bStartAt = true;
                        else
                            aLFOLVL.bStartAt = false;
                        //
                        // 2.2.2.2 eventuell auch den zugehoerigen LVL einlesen
                        //
                        SwNumFmt aNumFmt(
                            rLFOInfo.pNumRule->Get(aLFOLVL.nLevel));
                        if (aBits1 & 0x20)
                        {
                            aLFOLVL.bFormat = true;
                            // falls bStartup true, hier den Startup-Level
                            // durch den im LVL vermerkten ersetzen LVLF
                            // einlesen
                            bLVLOk = ReadLVL(aNumFmt, aItemSet[nLevel],
                                pParentListInfo->aIdSty[nLevel],
                                aLFOLVL.bStartAt, aNotReallyThere, nLevel,
                                rLFOInfo.maParaSprms[nLevel]);

                            if (!bLVLOk)
                                break;
                        }
                        else if (aLFOLVL.bStartAt)
                        {
                            aNumFmt.SetStart(
                                writer_cast<sal_uInt16>(aLFOLVL.nStartAt));
                        }
                        //
                        // 2.2.2.3 das NumFmt in die NumRule aufnehmen
                        //
                        rLFOInfo.pNumRule->Set(aLFOLVL.nLevel, aNumFmt);
                    }
                    bLVLOk = true;

                    if (nMaxLevel > aLFOLVL.nLevel)
                        rLFOInfo.maOverrides[aLFOLVL.nLevel] = aLFOLVL;
                }
                if( !bLVLOk )
                    break;
                //
                // 2.2.3 die LVL der neuen NumRule anpassen
                //
                sal_uInt16 aFlagsNewCharFmt = 0;
                bool bNewCharFmtCreated = false;
                for (sal_uInt8 nLevel = 0; nLevel < rLFOInfo.nLfoLvl; ++nLevel)
                {
                    AdjustLVL( nLevel, *rLFOInfo.pNumRule, aItemSet, aCharFmt,
                        bNewCharFmtCreated, sPrefix );
                    if( bNewCharFmtCreated )
                        aFlagsNewCharFmt += (1 << nLevel);
                }
                //
                // 2.2.4 ItemPools leeren und loeschen
                //
                for (sal_uInt8 nLevel = 0; nLevel < rLFOInfo.nLfoLvl; ++nLevel)
                    delete aItemSet[ nLevel ];
                bOk = true;
            }
        }
    }
    // und schon sind wir fertig!
    rSt.Seek( nOriginalPos );
}

WW8ListManager::~WW8ListManager()
{
    /*
     named lists remain in document
     unused automatic lists are removed from document (DelNumRule)
    */
    for(std::vector<WW8LSTInfo *>::iterator aIter = maLSTInfos.begin();
        aIter != maLSTInfos.end(); ++aIter)
    {
        if ((*aIter)->pNumRule && !(*aIter)->bUsedInDoc &&
            (*aIter)->pNumRule->IsAutoRule())
        {
            rDoc.DelNumRule((*aIter)->pNumRule->GetName());
        }
        delete *aIter;
    }
    boost::ptr_vector<WW8LFOInfo >::reverse_iterator aIter;
    for (aIter = pLFOInfos.rbegin() ;
        aIter < pLFOInfos.rend();
        ++aIter )
    {
        if (aIter->bOverride
            && aIter->pNumRule
            && !aIter->bUsedInDoc
            && aIter->pNumRule->IsAutoRule())
        {
            rDoc.DelNumRule( aIter->pNumRule->GetName() );
        }
    }
}

bool IsEqualFormatting(const SwNumRule &rOne, const SwNumRule &rTwo)
{
    bool bRet =
        (
          rOne.GetRuleType() == rTwo.GetRuleType() &&
          rOne.IsContinusNum() == rTwo.IsContinusNum() &&
          rOne.IsAbsSpaces() == rTwo.IsAbsSpaces() &&
          rOne.GetPoolFmtId() == rTwo.GetPoolFmtId() &&
          rOne.GetPoolHelpId() == rTwo.GetPoolHelpId() &&
          rTwo.GetPoolHlpFileId() == rTwo.GetPoolHlpFileId()
        );

    if (bRet)
    {
        for (sal_uInt8 n = 0; n < MAXLEVEL; ++n )
        {
            //The SvxNumberFormat compare, not the SwNumFmt compare
            const SvxNumberFormat &rO = rOne.Get(n);
            const SvxNumberFormat &rT = rTwo.Get(n);
            if (!(rO == rT))
            {
                bRet = false;
                break;
            }
        }
    }
    return bRet;
}

SwNumRule* WW8ListManager::GetNumRuleForActivation(sal_uInt16 nLFOPosition,
    const sal_uInt8 nLevel, std::vector<sal_uInt8> &rParaSprms, SwTxtNode *pNode)
{
    if (pLFOInfos.size() <= nLFOPosition)
        return 0;

    WW8LFOInfo& rLFOInfo = pLFOInfos[nLFOPosition];

    bool bFirstUse = !rLFOInfo.bUsedInDoc;
    rLFOInfo.bUsedInDoc = true;

    if( !rLFOInfo.pNumRule )
        return 0;

    // #i25545#
    // #i100132# - a number format does not have to exist on given list level
    SwNumFmt pFmt(rLFOInfo.pNumRule->Get(nLevel));

    if (rReader.IsRightToLeft() && nLastLFOPosition != nLFOPosition) {
        if ( pFmt.GetNumAdjust() == SVX_ADJUST_RIGHT)
            pFmt.SetNumAdjust(SVX_ADJUST_LEFT);
        else if ( pFmt.GetNumAdjust() == SVX_ADJUST_LEFT)
            pFmt.SetNumAdjust(SVX_ADJUST_RIGHT);
        rLFOInfo.pNumRule->Set(nLevel, pFmt);
    }
    nLastLFOPosition = nLFOPosition;
    /*
    #i1869#
    If this list has had its bits set in word 2000 to pretend that it is a
    simple list from the point of view of the user, then it is almost
    certainly a simple continous list, and we will try to keep it like that.
    Otherwise when we save again it will be shown as the true outline list
    that it is, confusing the user that just wanted what they thought was a
    simple list. On the otherhand it is possible that some of the other levels
    were used by the user, in which case we will not pretend anymore that it
    is a simple list. Something that word 2000 does anyway, that 97 didn't, to
    my bewilderment.
    */
    if (nLevel && rLFOInfo.pNumRule->IsContinusNum())
        rLFOInfo.pNumRule->SetContinusNum(false);

    if( (!rLFOInfo.bOverride) && (!rLFOInfo.bLSTbUIDSet) )
    {
        WW8LSTInfo* pParentListInfo = GetLSTByListId( rLFOInfo.nIdLst );
        if( pParentListInfo )
            pParentListInfo->bUsedInDoc = true;
        rLFOInfo.bLSTbUIDSet = true;
    }

    if (rLFOInfo.maParaSprms.size() > nLevel)
        rParaSprms = rLFOInfo.maParaSprms[nLevel];

    SwNumRule *pRet = rLFOInfo.pNumRule;

    bool bRestart(false);
    sal_uInt16 nStart(0);
    bool bNewstart(false);
    /*
      Note: If you fiddle with this then you have to make sure that #i18322#
      #i13833#, #i20095# and #112466# continue to work

      Check if there were overrides for this level
    */
    if (rLFOInfo.bOverride && nLevel < rLFOInfo.nLfoLvl)
    {
        WW8LSTInfo* pParentListInfo = GetLSTByListId(rLFOInfo.nIdLst);
        OSL_ENSURE(pParentListInfo, "ww: Impossible lists, please report");
        if (pParentListInfo && pParentListInfo->pNumRule)
        {
            const WW8LFOLVL &rOverride = rLFOInfo.maOverrides[nLevel];
            bool bNoChangeFromParent =
                IsEqualFormatting(*pRet, *(pParentListInfo->pNumRule));

            //If so then I think word still uses the parent (maybe)
            if (bNoChangeFromParent)
            {
                pRet = pParentListInfo->pNumRule;

                //did it not affect start at value ?
                if (bFirstUse)
                {
                    if (rOverride.bStartAt)
                    {
                        const SwNumFmt &rFmt =
                            pParentListInfo->pNumRule->Get(nLevel);
                        if (
                             rFmt.GetStart() ==
                             rLFOInfo.maOverrides[nLevel].nStartAt
                           )
                        {
                            bRestart = true;
                        }
                        else
                        {
                            bNewstart = true;
                            nStart = writer_cast<sal_uInt16>
                                (rLFOInfo.maOverrides[nLevel].nStartAt);
                        }
                    }
                }

                pParentListInfo->bUsedInDoc = true;
            }
        }
    }

    if (pNode)
    {
        pNode->SetAttrListLevel(nLevel);

        if (bRestart || bNewstart)
            pNode->SetListRestart(true);
        if (bNewstart)
            pNode->SetAttrListRestartValue(nStart);
    }
    return pRet;
}

//----------------------------------------------------------------------------
//          SwWW8ImplReader:  anhaengen einer Liste an einen Style oder Absatz
//----------------------------------------------------------------------------
bool SwWW8ImplReader::SetTxtFmtCollAndListLevel(const SwPaM& rRg,
    SwWW8StyInf& rStyleInfo)
{
    bool bRes = true;
    if( rStyleInfo.pFmt && rStyleInfo.bColl )
    {
        bRes = rDoc.SetTxtFmtColl(rRg, (SwTxtFmtColl*)rStyleInfo.pFmt);
        SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode();
        OSL_ENSURE( pTxtNode, "No Text-Node at PaM-Position" );
        // make code robust
        if ( !pTxtNode )
        {
            return bRes;
        }

        SwNumRule * pNumRule = pTxtNode->GetNumRule(); // #i27610#

        if( !IsInvalidOrToBeMergedTabCell() &&
            ! (pNumRule && pNumRule->IsOutlineRule()) ) // #i27610#
            pTxtNode->ResetAttr( RES_PARATR_NUMRULE );

        if( !rStyleInfo.pOutlineNumrule )
        {
            if (
                 (USHRT_MAX > rStyleInfo.nLFOIndex) &&
                 (WW8ListManager::nMaxLevel > rStyleInfo.nListLevel)
               )
            {
                RegisterNumFmtOnTxtNode(rStyleInfo.nLFOIndex,
                    rStyleInfo.nListLevel, false);
            }
        }
        else
        {
            // Use outline level set at the style info <rStyleInfo> instead of
            // the outline level at the text format, because the WW8 document
            // could contain more than one outline numbering rule and the one
            // of the text format isn't the one, which a chosen as the Writer
            // outline rule.
            pTxtNode->SetAttrListLevel( rStyleInfo.nOutlineLevel );
        }
    }
    return bRes;
}

void UseListIndent(SwWW8StyInf &rStyle, const SwNumFmt &rFmt)
{
    // #i86652#
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        const long nAbsLSpace = rFmt.GetAbsLSpace();
        const long nListFirstLineIndent = GetListFirstLineIndent(rFmt);
        SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*rStyle.pFmt, RES_LR_SPACE));
        aLR.SetTxtLeft(nAbsLSpace);
        aLR.SetTxtFirstLineOfst(writer_cast<short>(nListFirstLineIndent));
        rStyle.pFmt->SetFmtAttr(aLR);
        rStyle.bListReleventIndentSet = true;
    }
}

void SetStyleIndent(SwWW8StyInf &rStyle, const SwNumFmt &rFmt)
{
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION ) // #i86652#
    {
        SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*rStyle.pFmt, RES_LR_SPACE));
        if (rStyle.bListReleventIndentSet)
        {

            SyncIndentWithList( aLR, rFmt, false, false ); // #i103711#, #i105414#
        }
        else
        {
            aLR.SetTxtLeft(0);
            aLR.SetTxtFirstLineOfst(0);
        }
        rStyle.pFmt->SetFmtAttr(aLR);
    }
}

void SwWW8ImplReader::SetStylesList(sal_uInt16 nStyle, sal_uInt16 nActLFO,
    sal_uInt8 nActLevel)
{
    if (nStyle >= vColl.size())
        return;

    SwWW8StyInf &rStyleInf = vColl[nStyle];
    if (rStyleInf.bValid)
    {
        OSL_ENSURE(pAktColl, "Cannot be called outside of style import");
        // Phase 1: Nummerierungsattribute beim Einlesen einer StyleDef
        if( pAktColl )
        {
            // jetzt nur die Parameter vermerken: die tatsaechliche Liste wird
            // spaeter drangehaengt, wenn die Listendefinitionen gelesen sind...
            if (
                 (USHRT_MAX > nActLFO) &&
                 (WW8ListManager::nMaxLevel > nActLevel)
               )
            {
                rStyleInf.nLFOIndex  = nActLFO;
                rStyleInf.nListLevel = nActLevel;
                if (nActLevel > 0) // it must be an outline list
                    rStyleInf.nOutlineLevel = nActLevel;

                if (
                    (USHRT_MAX > nActLFO) &&
                    (WW8ListManager::nMaxLevel > nActLevel)
                   )
                {
                    std::vector<sal_uInt8> aParaSprms;
                    SwNumRule *pNmRule =
                        pLstManager->GetNumRuleForActivation(nActLFO,
                            nActLevel, aParaSprms);
                    if (pNmRule)
                        UseListIndent(rStyleInf, pNmRule->Get(nActLevel));
                }
            }
        }
    }
}

void SwWW8ImplReader::RegisterNumFmtOnStyle(sal_uInt16 nStyle)
{

    if (nStyle >= vColl.size())
        return;

    SwWW8StyInf &rStyleInf = vColl[nStyle];
    if (rStyleInf.bValid && rStyleInf.pFmt)
    {
        //Save old pre-list modified indent, which are the word indent values
        rStyleInf.maWordLR =
            ItemGet<SvxLRSpaceItem>(*rStyleInf.pFmt, RES_LR_SPACE);

        // Phase 2: aktualisieren der StyleDef nach einlesen aller Listen
        SwNumRule* pNmRule = 0;
        sal_uInt16 nLFO = rStyleInf.nLFOIndex;
        sal_uInt8  nLevel = rStyleInf.nListLevel;
        if (
             (USHRT_MAX > nLFO) &&
             (WW8ListManager::nMaxLevel > nLevel)
           )
        {
            std::vector<sal_uInt8> aParaSprms;
            pNmRule = pLstManager->GetNumRuleForActivation(nLFO, nLevel,
                aParaSprms);

            if (pNmRule)
            {
                if( MAXLEVEL > rStyleInf.nOutlineLevel )
                    rStyleInf.pOutlineNumrule = pNmRule;
                else
                {
                    rStyleInf.pFmt->SetFmtAttr(
                        SwNumRuleItem( pNmRule->GetName() ) );
                    rStyleInf.bHasStyNumRule = true;
                }
            }
        }

        if (pNmRule)
            SetStyleIndent(rStyleInf, pNmRule->Get(nLevel));
    }
}

void SwWW8ImplReader::RegisterNumFmtOnTxtNode(sal_uInt16 nActLFO,
    sal_uInt8 nActLevel, bool bSetAttr)
{
    // beachte: die Methode haengt die NumRule an den Text Node, falls
    // bSetAttr (dann muessen natuerlich vorher die Listen gelesen sein)
    // stellt sie NUR den Level ein, im Vertrauen darauf, dass am STYLE eine
    // NumRule haengt - dies wird NICHT ueberprueft !!!

    if (pLstManager) // sind die Listendeklarationen gelesen?
    {
        SwTxtNode* pTxtNd = pPaM->GetNode()->GetTxtNode();
        OSL_ENSURE(pTxtNd, "No Text-Node at PaM-Position");
        if (!pTxtNd)
            return;

        std::vector<sal_uInt8> aParaSprms;
        const SwNumRule* pRule = bSetAttr ?
            pLstManager->GetNumRuleForActivation( nActLFO, nActLevel,
                aParaSprms, pTxtNd) : 0;

        if (pRule || !bSetAttr)
        {
            //#i24136# old is the same as new, and its the outline numbering,
            //then we don't set the numrule again, and we just take the num node
            //(the actual outline numbering gets set in SetOutlineNum)
            using namespace sw::util;
            bool bUnchangedOutlineNumbering = false;
            /*
             If the node is outline numbered, and the new numbering to apply
             is the one that was chosen to be the outline numbering then all
             is unchanged
            */
            // correct condition according to the above given comment.
            if ( pTxtNd->GetNumRule() == rDoc.GetOutlineNumRule() &&
                 pRule == mpChosenOutlineNumRule )
            {
                bUnchangedOutlineNumbering = true;
            }
            if (!bUnchangedOutlineNumbering)
            {
                //If its normal numbering, see if its the same as it already
                //was, if its not, and we have been asked to set it, then set
                //it to the new one
                if (bSetAttr)
                {
                    const SwNumRule *pNormal = pTxtNd->GetNumRule();
                    if (pNormal != pRule)
                    {
                        pTxtNd->SetAttr
                            (SwNumRuleItem(pRule->GetName()));
                    }
                }
            }
            pTxtNd->SetAttrListLevel(nActLevel);

            // <IsCounted()> state of text node has to be adjusted accordingly.
            if ( /*nActLevel >= 0 &&*/ nActLevel < MAXLEVEL )
            {
                pTxtNd->SetCountedInList( true );
            }

            // #i99822#
            // Direct application of the list level formatting no longer
            // needed for list levels of mode LABEL_ALIGNMENT
            bool bApplyListLevelIndentDirectlyAtPara( true );
            if ( pTxtNd->GetNumRule() && nActLevel < MAXLEVEL )
            {
                const SwNumFmt& rFmt = pTxtNd->GetNumRule()->Get( nActLevel );
                if ( rFmt.GetPositionAndSpaceMode() ==
                                            SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    bApplyListLevelIndentDirectlyAtPara = false;
                }
            }

            if ( bApplyListLevelIndentDirectlyAtPara )
            {
                SfxItemSet aListIndent(rDoc.GetAttrPool(), RES_LR_SPACE,
                        RES_LR_SPACE);
                const SvxLRSpaceItem *pItem = (const SvxLRSpaceItem*)(
                    GetFmtAttr(RES_LR_SPACE));
                OSL_ENSURE(pItem, "impossible");
                if (pItem)
                    aListIndent.Put(*pItem);

                /*
                 Take the original paragraph sprms attached to this list level
                 formatting and apply them to the paragraph. I'm convinced that
                 this is exactly what word does.
                */
                if (short nLen = static_cast< short >(aParaSprms.size()))
                {
                    SfxItemSet* pOldAktItemSet = pAktItemSet;
                    SetAktItemSet(&aListIndent);

                    sal_uInt8* pSprms1  = &aParaSprms[0];
                    while (0 < nLen)
                    {
                        sal_uInt16 nL1 = ImportSprm(pSprms1);
                        nLen = nLen - nL1;
                        pSprms1 += nL1;
                    }

                    SetAktItemSet(pOldAktItemSet);
                }

                const SvxLRSpaceItem *pLR =
                    HasItem<SvxLRSpaceItem>(aListIndent, RES_LR_SPACE);
                OSL_ENSURE(pLR, "Impossible");
                if (pLR)
                {
                    pCtrlStck->NewAttr(*pPaM->GetPoint(), *pLR);
                    pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_LR_SPACE);
                }
            }
        }
    }
}

void SwWW8ImplReader::RegisterNumFmt(sal_uInt16 nActLFO, sal_uInt8 nActLevel)
{
    // sind wir erst beim Einlesen der StyleDef ?
    if (pAktColl)
        SetStylesList( nAktColl , nActLFO, nActLevel);
    else
        RegisterNumFmtOnTxtNode(nActLFO, nActLevel);
}

void SwWW8ImplReader::Read_ListLevel(sal_uInt16, const sal_uInt8* pData,
    short nLen)
{
    if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // aktuelle Liste ist hier zu Ende, was ist zu tun ???
        nListLevel = WW8ListManager::nMaxLevel;
        if (pStyles && !bVer67)
            pStyles->nWwNumLevel = 0;
    }
    else
    {
        // Sicherheitspruefung auf NIL Pointer
        if( !pData ) return;
        // die Streamdaten sind hier Null basiert, so wie wir es brauchen
        nListLevel = *pData;

        if (pStyles && !bVer67)
        {
            /*
            if this is the case, then if the numbering is actually stored in
            winword 6 format, and its likely that sprmPIlvl has been abused
            to set the ww6 list level information which we will need when we
            reach the true ww6 list def.  So set it now
            */
            pStyles->nWwNumLevel = nListLevel;
        }

        if (WW8ListManager::nMaxLevel <= nListLevel )
            nListLevel = WW8ListManager::nMaxLevel;
        else if
           (
             (USHRT_MAX > nLFOPosition) &&
             (WW8ListManager::nMaxLevel > nListLevel)
           )
        {
            RegisterNumFmt(nLFOPosition, nListLevel);
            nLFOPosition = USHRT_MAX;
            nListLevel  = WW8ListManager::nMaxLevel;
        }
    }
}

void SwWW8ImplReader::Read_LFOPosition(sal_uInt16, const sal_uInt8* pData,
    short nLen)
{
    if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // aktueller Level ist hier zu Ende, was ist zu tun ???
        nLFOPosition = USHRT_MAX;
        nListLevel = WW8ListManager::nMaxLevel;
    }
    else
    {
        // Sicherheitspruefung auf NIL Pointer
        if( !pData )
            return;
        short nData = SVBT16ToShort( pData );
        if( 0 >= nData )
        {
            /*
            If you have a paragraph in word with left and/or hanging indent
            and remove its numbering, then the indentation appears to get
            reset, but not back to the base style, instead its goes to a blank
            setting.
            Unless its a broken ww6 list in 97 in which case more hackery is
            required, some more details about that in
            ww8par6.cxx#SwWW8ImplReader::Read_LR
            */

            if (pAktColl)
            {
                pAktColl->SetFmtAttr(*GetDfltAttr( RES_PARATR_NUMRULE));
                pAktColl->SetFmtAttr(SvxLRSpaceItem(RES_LR_SPACE));
            }
            else if (SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode())
            {
                pTxtNode->ResetAttr( RES_PARATR_NUMRULE ); // #i54393#
                pTxtNode->SetCountedInList(false);

                /*
                #i24553#
                Hmm, I can't remove outline numbering on a per txtnode basis,
                but I can set some normal numbering, and that overrides outline
                numbering, and then I can say when I come to say that I want no
                number on the normal numbering rule, that should all work out

                No special outline number in textnode any more
                */
                if (pTxtNode->IsOutline())
                {
                    // #i54393#
                    // It's not needed to call <SetCounted( false )> again - see above.
                    // #i54393#
                    // Assure that the numbering rule, which is retrieved at
                    // the paragraph is the outline numbering rule, instead of
                    // incorrectly setting the chosen outline rule.
                    // Note: The chosen outline rule doesn't have to correspond
                    //       to the outline rule
                    if ( pTxtNode->GetNumRule() != rDoc.GetOutlineNumRule() )
                    {
                        pTxtNode->SetAttr(
                            SwNumRuleItem( rDoc.GetOutlineNumRule()->GetName() ) );
                    }
                }

                pCtrlStck->NewAttr(*pPaM->GetPoint(), SvxLRSpaceItem(RES_LR_SPACE));
                pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_LR_SPACE);
            }
            nLFOPosition = USHRT_MAX;
        }
        else
        {
            nLFOPosition = (sal_uInt16)nData-1;
            /*
            If we are a ww8+ style with ww7- style lists then there is a
            bizarre broken word bug where when the list is removed from a para
            the ww6 list first line indent still affects the first line
            indentation.  Setting this flag will allow us to recover from this
            braindeadness
            */
            if (pAktColl && (nLFOPosition == 2047-1) && nAktColl < vColl.size())
                vColl[nAktColl].bHasBrokenWW6List = true;

            // die Streamdaten sind hier 1 basiert, wir ziehen EINS ab
            if (USHRT_MAX > nLFOPosition)
            {
                if (nLFOPosition != 2047-1) //Normal ww8+ list behaviour
                {
                    if (WW8ListManager::nMaxLevel == nListLevel)
                        nListLevel = 0;
                    else if (WW8ListManager::nMaxLevel > nListLevel)
                    {
                        RegisterNumFmt(nLFOPosition, nListLevel);
                        nLFOPosition = USHRT_MAX;
                        nListLevel = WW8ListManager::nMaxLevel;
                    }
                }
                else if (pPlcxMan && pPlcxMan->HasParaSprm(0xC63E))
                {
                    /*
                     #i8114# Horrific backwards compatible ww7- lists in ww8+
                     docs
                    */
                    Read_ANLevelNo(13 /*equiv ww7- sprm no*/, &nListLevel, 1);
                }
            }
        }
    }
}

// -------------------------------------------------------------------
// ------------------------- Reading Controls ------------------------
// -------------------------------------------------------------------

bool SwWW8ImplReader::ImportFormulaControl(WW8FormulaControl &aFormula,
    WW8_CP nStart, SwWw8ControlType nWhich )
{
    bool bRet=false;
    /*
     * Save the reader state and process the sprms for this anchor cp.
     * Doing so will set the nPicLocFc to the offset to find the hypertext
     * data in the data stream.
     */
    WW8_CP nEndCp = nStart+1; //Only interested in the single 0x01 character

    WW8ReaderSave aSave(this,nStart);

    WW8PLCFManResult aRes;
    nStart = pPlcxMan->Where();
    while(nStart <= nEndCp)
    {
        if ( pPlcxMan->Get(&aRes)
            && aRes.pMemPos && aRes.nSprmId )
        {
            //only interested in sprms which would set nPicLocFc
            if ( (68 == aRes.nSprmId) || (0x6A03 == aRes.nSprmId) )
            {
                Read_PicLoc( aRes.nSprmId, aRes.pMemPos +
                    mpSprmParser->DistanceToData(aRes.nSprmId), 4);
                break;
            }
        }
        pPlcxMan->advance();
        nStart = pPlcxMan->Where();
    }
    sal_uLong nOffset = nPicLocFc;
    aSave.Restore(this);

    sal_uLong nOldPos = pDataStream->Tell();
    WW8_PIC aPic;
    pDataStream->Seek( nOffset);
    PicRead( pDataStream, &aPic, bVer67);

    if((aPic.lcb > 0x3A) && !pDataStream->GetError() )
    {
        aFormula.FormulaRead(nWhich,pDataStream);
        bRet = true;
    }

    /*
     There is a problem with aPic, the WW8_PIC is always used even though it
     is too big for the WW95 files, it needs to be modified to check the
     version C.
     */
    pDataStream->Seek( nOldPos );
    return(bRet);
}

sal_Bool SwMSConvertControls::InsertFormula(WW8FormulaControl &rFormula)
{
    sal_Bool bRet = sal_False;

    const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
        GetServiceFactory();

    if(!rServiceFactory.is())
        return sal_False;

    awt::Size aSz;
    uno::Reference< form::XFormComponent> xFComp;

    if (sal_True == (bRet = rFormula.Import(rServiceFactory, xFComp, aSz)))
    {
        uno::Reference <drawing::XShape> xShapeRef;
        if (sal_True == (bRet = InsertControl(xFComp, aSz, &xShapeRef, false)))
            GetShapes()->add(xShapeRef);
    }
    return bRet;
}

void WW8FormulaControl::FormulaRead(SwWw8ControlType nWhich,
    SvStream *pDataStream)
{
    sal_uInt8 nField;
    // nHeaderBype == version
    sal_uInt32 nHeaderByte;

    // The following is a FFData structure as described in
    // Microsoft's DOC specification (chapter 2.9.78)

    *pDataStream >> nHeaderByte;

    // might be better to read the bits as a 16 bit word 
    // ( like it is in the spec. )
    sal_uInt8 bits1;
    *pDataStream >> bits1;
    sal_uInt8 bits2;
    *pDataStream >> bits2;

    sal_uInt8 iType = ( bits1 & 0x3 );

    // we should verify that bits.iType & nWhich concur
    OSL_ENSURE( iType == nWhich, "something wrong, expect control type read from stream doesn't match nWhich passed in");
    if ( !( iType == nWhich ) )
        return; // bail out

    sal_uInt8 iRes = (bits1 & 0x7C) >> 2;

    sal_uInt16 cch;
    *pDataStream >> cch;

    sal_uInt16 hps;
    *pDataStream >> hps;

    // xstzName
    sTitle = read_uInt16_BeltAndBracesString(*pDataStream);

    if (nWhich == WW8_CT_EDIT)
    {   // Field is a textbox
        // Default text
        // xstzTextDef
        sDefault = read_uInt16_BeltAndBracesString(*pDataStream);
    }
    else
    {
        // CheckBox or ComboBox
        sal_uInt16 wDef = 0;
        *pDataStream >> wDef;
        nChecked = wDef; // default
        if (nWhich == WW8_CT_CHECKBOX)
        {
            if ( iRes != 25 )
                nChecked = iRes;
            sDefault = ( wDef == 0 ) ? rtl::OUString( "0" ) :  rtl::OUString( "1" );
        }
    }
    // xstzTextFormat
    sFormatting = read_uInt16_BeltAndBracesString(*pDataStream);
    // xstzHelpText
    sHelp = read_uInt16_BeltAndBracesString(*pDataStream);
    // xstzStatText
    sToolTip = read_uInt16_BeltAndBracesString(*pDataStream);

    /*String sEntryMacro =*/ read_uInt16_BeltAndBracesString(*pDataStream);
    /*String sExitMcr =*/ read_uInt16_BeltAndBracesString(*pDataStream);

    if (nWhich == WW8_CT_DROPDOWN)
    {
        bool bAllOk = true;
        // SSTB (see Spec. 2.2.4)
        sal_uInt16 fExtend;
        *pDataStream >> fExtend;
        sal_uInt16 nNoStrings;

        // Isn't it that if fExtend isn't 0xFFFF then fExtend actually
        // doesn't exist and we really have just read nNoStrings ( or cData )?
        if (fExtend != 0xFFFF)
            bAllOk = false;
        *pDataStream >> nNoStrings;

        // I guess this should be zero ( and we should ensure that )
        sal_uInt16 cbExtra;
        *pDataStream >> cbExtra;

        OSL_ENSURE(bAllOk,
            "Unknown formfield dropdown list structure. Report to cmc");
        if (!bAllOk)    //Not as expected, don't risk it at all.
            nNoStrings = 0;
        maListEntries.reserve(nNoStrings);
        for (sal_uInt32 nI = 0; nI < nNoStrings; ++nI)
        {
            String sEntry =  read_uInt16_PascalString(*pDataStream);
            maListEntries.push_back(sEntry);
        }
    }
    fDropdownIndex = iRes;

    nField = bits2;
    fToolTip = nField & 0x01;
    fNoMark = (nField & 0x02)>>1;
    fUseSize = (nField & 0x04)>>2;
    fNumbersOnly= (nField & 0x08)>>3;
    fDateOnly = (nField & 0x10)>>4;
    fUnused = (nField & 0xE0)>>5;
}

WW8FormulaListBox::WW8FormulaListBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(rtl::OUString(SL::aListBox), rR)
{
}

//Miserable hack to get a hardcoded guesstimate of the size of a list dropdown
//box's first entry to set as the lists default size
awt::Size SwWW8ImplReader::MiserableDropDownFormHack(const String &rString,
    uno::Reference<beans::XPropertySet>& rPropSet)
{
    awt::Size aRet;
    struct CtrlFontMapEntry
    {
        sal_uInt16 nWhichId;
        const sal_Char* pPropNm;
    };
    const CtrlFontMapEntry aMapTable[] =
    {
        { RES_CHRATR_COLOR,           "TextColor" },
        { RES_CHRATR_FONT,            "FontName" },
        { RES_CHRATR_FONTSIZE,        "FontHeight" },
        { RES_CHRATR_WEIGHT,          "FontWeight" },
        { RES_CHRATR_UNDERLINE,       "FontUnderline" },
        { RES_CHRATR_CROSSEDOUT,      "FontStrikeout" },
        { RES_CHRATR_POSTURE,         "FontSlant" },
        { 0,                          0 }
    };

    Font aFont;
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
        rPropSet->getPropertySetInfo();

    uno::Any aTmp;
    for (const CtrlFontMapEntry* pMap = aMapTable; pMap->nWhichId; ++pMap)
    {
        bool bSet = true;
        const SfxPoolItem* pItem = GetFmtAttr( pMap->nWhichId );
        OSL_ENSURE(pItem, "Impossible");
        if (!pItem)
            continue;

        switch ( pMap->nWhichId )
        {
        case RES_CHRATR_COLOR:
            {
                String pNm;
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("TextColor")))
                {
                    aTmp <<= (sal_Int32)((SvxColorItem*)pItem)->GetValue().GetColor();
                    rPropSet->setPropertyValue(pNm, aTmp);
                }
            }
            aFont.SetColor(((SvxColorItem*)pItem)->GetValue());
            break;
        case RES_CHRATR_FONT:
            {
                const SvxFontItem *pFontItem = (SvxFontItem *)pItem;
                String pNm;
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontStyleName")))
                {
                    aTmp <<= rtl::OUString( pFontItem->GetStyleName());
                    rPropSet->setPropertyValue( pNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontFamily")))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetFamily();
                    rPropSet->setPropertyValue( pNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontCharset")))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetCharSet();
                    rPropSet->setPropertyValue( pNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(pNm = C2U("FontPitch")))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetPitch();
                    rPropSet->setPropertyValue( pNm, aTmp );
                }

                aTmp <<= rtl::OUString( pFontItem->GetFamilyName());
                aFont.SetName( pFontItem->GetFamilyName() );
                aFont.SetStyleName( pFontItem->GetStyleName() );
                aFont.SetFamily( pFontItem->GetFamily() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
            }
            break;

        case RES_CHRATR_FONTSIZE:
            {
                Size aSize( aFont.GetSize().Width(),
                            ((SvxFontHeightItem*)pItem)->GetHeight() );
                aTmp <<= ((float)aSize.Height()) / 20.0;

                aFont.SetSize(OutputDevice::LogicToLogic(aSize, MAP_TWIP,
                    MAP_100TH_MM));
            }
            break;

        case RES_CHRATR_WEIGHT:
            aTmp <<= (float)VCLUnoHelper::ConvertFontWeight(
                                        ((SvxWeightItem*)pItem)->GetWeight() );
            aFont.SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
            break;

        case RES_CHRATR_UNDERLINE:
            aTmp <<= (sal_Int16)(((SvxUnderlineItem*)pItem)->GetLineStyle());
            aFont.SetUnderline(((SvxUnderlineItem*)pItem)->GetLineStyle());
            break;

        case RES_CHRATR_CROSSEDOUT:
            aTmp <<= (sal_Int16)( ((SvxCrossedOutItem*)pItem)->GetStrikeout() );
            aFont.SetStrikeout( ((SvxCrossedOutItem*)pItem)->GetStrikeout() );
            break;

        case RES_CHRATR_POSTURE:
            aTmp <<= (sal_Int16)( ((SvxPostureItem*)pItem)->GetPosture() );
            aFont.SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
            break;

        default:
            bSet = false;
            break;
        }

        if (bSet && xPropSetInfo->hasPropertyByName(rtl::OUString::createFromAscii(pMap->pPropNm)))
            rPropSet->setPropertyValue(rtl::OUString::createFromAscii(pMap->pPropNm), aTmp);
    }
    // now calculate the size of the control
    OutputDevice* pOut = Application::GetDefaultDevice();
    OSL_ENSURE(pOut, "Impossible");
    if (pOut)
    {
        pOut->Push( PUSH_FONT | PUSH_MAPMODE );
        pOut->SetMapMode( MapMode( MAP_100TH_MM ));
        pOut->SetFont( aFont );
        aRet.Width  = pOut->GetTextWidth(rString);
        aRet.Width += 500; //plus size of button, total hack territory
        aRet.Height = pOut->GetTextHeight();
        pOut->Pop();
    }
    return aRet;
}

sal_Bool WW8FormulaListBox::Import(const uno::Reference <
    lang::XMultiServiceFactory> &rServiceFactory,
    uno::Reference <form::XFormComponent> &rFComp,awt::Size &rSz )
{
    uno::Reference<uno::XInterface> xCreate = rServiceFactory->createInstance(
        C2U("com.sun.star.form.component.ComboBox"));
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference<form::XFormComponent>(xCreate, uno::UNO_QUERY);
    if( !rFComp.is() )
        return sal_False;

    uno::Reference<beans::XPropertySet> xPropSet(xCreate, uno::UNO_QUERY);

    uno::Any aTmp;
    if (!sTitle.isEmpty())
        aTmp <<= sTitle;
    else
        aTmp <<= sName;
    xPropSet->setPropertyValue(C2U("Name"), aTmp );

    if (!sToolTip.isEmpty())
    {
        aTmp <<= sToolTip;
        xPropSet->setPropertyValue(C2U("HelpText"), aTmp );
    }

    sal_Bool bDropDown(sal_True);
    xPropSet->setPropertyValue(C2U("Dropdown"), cppu::bool2any(bDropDown));

    if (!maListEntries.empty())
    {
        sal_uInt32 nLen = maListEntries.size();
        uno::Sequence< ::rtl::OUString > aListSource(nLen);
        for (sal_uInt32 nI = 0; nI < nLen; ++nI)
            aListSource[nI] = rtl::OUString(maListEntries[nI]);
        aTmp <<= aListSource;
        xPropSet->setPropertyValue(C2U("StringItemList"), aTmp );

        if (fDropdownIndex < nLen)
        {
            aTmp <<= aListSource[fDropdownIndex];
        }
        else
        {
            aTmp <<= aListSource[0];
        }

        xPropSet->setPropertyValue(C2U("DefaultText"), aTmp );

        rSz = rRdr.MiserableDropDownFormHack(maListEntries[0], xPropSet);
    }
    else
    {
        static const sal_Unicode aBlank[] =
        {
            0x2002,0x2002,0x2002,0x2002,0x2002
        };
        rSz = rRdr.MiserableDropDownFormHack(rtl::OUString(aBlank, SAL_N_ELEMENTS(aBlank)), xPropSet);
    }

    return sal_True;
}

WW8FormulaCheckBox::WW8FormulaCheckBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(rtl::OUString(SL::aCheckBox), rR)
{
}

static void lcl_AddToPropertyContainer
(uno::Reference<beans::XPropertySet> xPropSet,
 const rtl::OUString & rPropertyName, const rtl::OUString & rValue)
{
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if (xPropSetInfo.is() &&
        ! xPropSetInfo->hasPropertyByName(rPropertyName))
    {
        uno::Reference<beans::XPropertyContainer>
            xPropContainer(xPropSet, uno::UNO_QUERY);
        uno::Any aAny(C2U(""));
        xPropContainer->addProperty
            (rPropertyName,
             static_cast<sal_Int16>(beans::PropertyAttribute::BOUND |
                                    beans::PropertyAttribute::REMOVABLE),
             aAny);
    }

    uno::Any aAnyValue(rValue);
    xPropSet->setPropertyValue(rPropertyName, aAnyValue );
}

sal_Bool WW8FormulaCheckBox::Import(const uno::Reference <
    lang::XMultiServiceFactory> &rServiceFactory,
    uno::Reference <form::XFormComponent> &rFComp,awt::Size &rSz )
{
    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance(
        C2U("com.sun.star.form.component.CheckBox"));
    if( !xCreate.is() )
        return sal_False;

    rFComp = uno::Reference< form::XFormComponent >( xCreate, uno::UNO_QUERY );
    if( !rFComp.is() )
        return sal_False;

    uno::Reference< beans::XPropertySet > xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = 16 * hpsCheckBox;
    rSz.Height = 16 * hpsCheckBox;

    uno::Any aTmp;
    if (!sTitle.isEmpty())
        aTmp <<= sTitle;
    else
        aTmp <<= sName;
    xPropSet->setPropertyValue(C2U("Name"), aTmp );

    aTmp <<= (sal_Int16)nChecked;
    xPropSet->setPropertyValue(C2U("DefaultState"), aTmp);

    if (!sToolTip.isEmpty())
        lcl_AddToPropertyContainer(xPropSet, C2U("HelpText"), sToolTip);

    if (!sHelp.isEmpty())
        lcl_AddToPropertyContainer(xPropSet, C2U("HelpF1Text"), sHelp);

    return sal_True;

}

WW8FormulaEditBox::WW8FormulaEditBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(rtl::OUString(SL::aTextField) ,rR)
{
}

sal_Bool SwMSConvertControls::InsertControl(
    const uno::Reference< form::XFormComponent > & rFComp,
    const awt::Size& rSize, uno::Reference< drawing::XShape > *pShape,
    sal_Bool bFloatingCtrl)
{
    const uno::Reference< container::XIndexContainer > &rComps = GetFormComps();
    uno::Any aTmp( &rFComp, ::getCppuType((const uno::Reference<
        form::XFormComponent >*)0) );
    rComps->insertByIndex( rComps->getCount(), aTmp );

    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory =
        GetServiceFactory();
    if( !rServiceFactory.is() )
        return sal_False;

    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance(
        C2U("com.sun.star.drawing.ControlShape"));
    if( !xCreate.is() )
        return sal_False;

    uno::Reference< drawing::XShape > xShape =
        uno::Reference< drawing::XShape >(xCreate, uno::UNO_QUERY);

    OSL_ENSURE(xShape.is(), "XShape nicht erhalten");
    xShape->setSize(rSize);

    uno::Reference< beans::XPropertySet > xShapePropSet(
        xCreate, uno::UNO_QUERY );

    //I lay a small bet that this will change to
    //sal_Int16 nTemp=TextContentAnchorType::AS_CHARACTER;
    sal_Int16 nTemp;
    if (bFloatingCtrl)
        nTemp= text::TextContentAnchorType_AT_PARAGRAPH;
    else
        nTemp= text::TextContentAnchorType_AS_CHARACTER;

    aTmp <<= nTemp;
    xShapePropSet->setPropertyValue(C2U("AnchorType"), aTmp );

    nTemp= text::VertOrientation::TOP;
    aTmp <<= nTemp;
    xShapePropSet->setPropertyValue(C2U("VertOrient"), aTmp );

    uno::Reference< text::XText >  xDummyTxtRef;
    uno::Reference< text::XTextRange >  xTxtRg =
        new SwXTextRange( *pPaM, xDummyTxtRef );

    aTmp.setValue(&xTxtRg,::getCppuType((
        uno::Reference< text::XTextRange >*)0));
    xShapePropSet->setPropertyValue(C2U("TextRange"), aTmp );

    // Das Control-Model am Control-Shape setzen
    uno::Reference< drawing::XControlShape >  xControlShape( xShape,
        uno::UNO_QUERY );
    uno::Reference< awt::XControlModel >  xControlModel( rFComp,
        uno::UNO_QUERY );
    xControlShape->setControl( xControlModel );

    if (pShape)
        *pShape = xShape;

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
