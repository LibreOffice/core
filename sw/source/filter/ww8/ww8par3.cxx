/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <unotextrange.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <docsh.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <charatr.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <expfld.hxx>
#include <fmtfld.hxx>
#include <flddropdown.hxx>
#include <sprmids.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"

#include <IMark.hxx>
#include <unotools/fltrcfg.hxx>
#include <rtl/character.hxx>
#include <xmloff/odffields.hxx>

using namespace com::sun::star;
using namespace sw::util;
using namespace sw::types;
using namespace sw::mark;

//            UNO-Controls

// OCX i.e. word 97 form controls
eF_ResT SwWW8ImplReader::Read_F_OCX( WW8FieldDesc*, OUString& )
{
    if( m_bObj && m_nPicLocFc )
        m_nObjLocFc = m_nPicLocFc;
    m_bEmbeddObj = true;
    return FLD_TEXT;
}

eF_ResT SwWW8ImplReader::Read_F_FormTextBox( WW8FieldDesc* pF, OUString& rStr )
{
    WW8FormulaEditBox aFormula(*this);

    if (rStr[pF->nLCode-1]==0x01) {
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
    const bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields)
    {
        aFormula.msDefault = GetFieldResult(pF);

        SwInputField aField(
            static_cast<SwInputFieldType*>(m_rDoc.getIDocumentFieldsAccess().GetSysFieldType( RES_INPUTFLD )),
            aFormula.msDefault,
            aFormula.msTitle,
            INP_TXT,
            0 );
        aField.SetHelp(aFormula.msHelp);
        aField.SetToolTip(aFormula.msToolTip);

        m_rDoc.getIDocumentContentOperations().InsertPoolItem(*m_pPaM, SwFormatField(aField));
        return FLD_OK;
    }
    else
    {
        WW8PLCFx_Book* pB = m_pPlcxMan->GetBook();
        OUString aBookmarkName;
        if (pB!=nullptr) {
            WW8_CP currentCP=pF->nSCode;
            WW8_CP currentLen=pF->nLen;

            sal_uInt16 bkmFindIdx;
            OUString aBookmarkFind=pB->GetBookmark(currentCP-1, currentCP+currentLen-1, bkmFindIdx);

            if (!aBookmarkFind.isEmpty()) {
                pB->SetStatus(bkmFindIdx, BOOK_FIELD); // mark bookmark as consumed, such that tl'll not get inserted as a "normal" bookmark again
                if (!aBookmarkFind.isEmpty()) {
                    aBookmarkName=aBookmarkFind;
                }
            }
        }

        if (pB!=nullptr && aBookmarkName.isEmpty()) {
            aBookmarkName=pB->GetUniqueBookmarkName(aFormula.msTitle);
        }

        if (!aBookmarkName.isEmpty()) {
            m_aFieldStack.back().SetBookmarkName(aBookmarkName);
            m_aFieldStack.back().SetBookmarkType(ODF_FORMTEXT);
            m_aFieldStack.back().getParameters()["Description"] = uno::makeAny(OUString(aFormula.msToolTip));
            m_aFieldStack.back().getParameters()["Name"] = uno::makeAny(OUString(aFormula.msTitle));
            if (aFormula.mnMaxLen)
                m_aFieldStack.back().getParameters()["MaxLength"] = uno::makeAny(OUString::number(aFormula.mnMaxLen));
        }
        return FLD_TEXT;
    }
}

eF_ResT SwWW8ImplReader::Read_F_FormCheckBox( WW8FieldDesc* pF, OUString& rStr )
{
    WW8FormulaCheckBox aFormula(*this);

    if (!m_pFormImpl)
        m_pFormImpl = new SwMSConvertControls(m_pDocShell, m_pPaM);

    if (rStr[pF->nLCode-1]==0x01)
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_CHECKBOX);
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    const bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields)
    {
        m_pFormImpl->InsertFormula(aFormula);
        return FLD_OK;
    }

    OUString aBookmarkName;
    WW8PLCFx_Book* pB = m_pPlcxMan->GetBook();
    if (pB!=nullptr) {
        WW8_CP currentCP=pF->nSCode;
        WW8_CP currentLen=pF->nLen;

        sal_uInt16 bkmFindIdx;
        OUString aBookmarkFind=pB->GetBookmark(currentCP-1, currentCP+currentLen-1, bkmFindIdx);

        if (!aBookmarkFind.isEmpty()) {
            pB->SetStatus(bkmFindIdx, BOOK_FIELD); // mark as consumed by field
            if (!aBookmarkFind.isEmpty()) {
                aBookmarkName=aBookmarkFind;
            }
        }
    }

    if (pB!=nullptr && aBookmarkName.isEmpty()) {
        aBookmarkName=pB->GetUniqueBookmarkName(aFormula.msTitle);
    }

    if (!aBookmarkName.isEmpty())
    {
        IDocumentMarkAccess* pMarksAccess = m_rDoc.getIDocumentMarkAccess( );
        IFieldmark* pFieldmark = dynamic_cast<IFieldmark*>( pMarksAccess->makeNoTextFieldBookmark(
                *m_pPaM, aBookmarkName, ODF_FORMCHECKBOX ) );
        OSL_ENSURE(pFieldmark!=nullptr, "hmmm; why was the bookmark not created?");
        if (pFieldmark!=nullptr) {
            IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
            ICheckboxFieldmark* pCheckboxFm = dynamic_cast<ICheckboxFieldmark*>(pFieldmark);
            (*pParameters)[ODF_FORMCHECKBOX_NAME] = uno::makeAny(OUString(aFormula.msTitle));
            (*pParameters)[ODF_FORMCHECKBOX_HELPTEXT] = uno::makeAny(OUString(aFormula.msToolTip));

            if(pCheckboxFm)
                pCheckboxFm->SetChecked(aFormula.mnChecked);
            // set field data here...
        }
    }
    return FLD_OK;
}

eF_ResT SwWW8ImplReader::Read_F_FormListBox( WW8FieldDesc* pF, OUString& rStr)
{
    WW8FormulaListBox aFormula(*this);

    if (rStr[pF->nLCode-1]==0x01)
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_DROPDOWN);

    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields)
    {
        SwDropDownField aField(static_cast<SwDropDownFieldType*>(m_rDoc.getIDocumentFieldsAccess().GetSysFieldType(RES_DROPDOWN)));

        aField.SetName(aFormula.msTitle);
        aField.SetHelp(aFormula.msHelp);
        aField.SetToolTip(aFormula.msToolTip);

        if (!aFormula.maListEntries.empty())
        {
            aField.SetItems(aFormula.maListEntries);
            int nIndex = aFormula.mfDropdownIndex  < aFormula.maListEntries.size() ? aFormula.mfDropdownIndex : 0;
            aField.SetSelectedItem(aFormula.maListEntries[nIndex]);
        }

        m_rDoc.getIDocumentContentOperations().InsertPoolItem(*m_pPaM, SwFormatField(aField));
        return FLD_OK;
    }
    else
    {
        // TODO: review me
        OUString aBookmarkName;
        WW8PLCFx_Book* pB = m_pPlcxMan->GetBook();
        if (pB!=nullptr)
        {
            WW8_CP currentCP=pF->nSCode;
            WW8_CP currentLen=pF->nLen;

            sal_uInt16 bkmFindIdx;
            OUString aBookmarkFind=pB->GetBookmark(currentCP-1, currentCP+currentLen-1, bkmFindIdx);

            if (!aBookmarkFind.isEmpty())
            {
                pB->SetStatus(bkmFindIdx, BOOK_FIELD); // mark as consumed by field
                if (!aBookmarkFind.isEmpty())
                    aBookmarkName=aBookmarkFind;
            }
        }

        if (pB!=nullptr && aBookmarkName.isEmpty())
            aBookmarkName=pB->GetUniqueBookmarkName(aFormula.msTitle);

        if (!aBookmarkName.isEmpty())
        {
            IDocumentMarkAccess* pMarksAccess = m_rDoc.getIDocumentMarkAccess( );
            IFieldmark *pFieldmark = dynamic_cast<IFieldmark*>(
                    pMarksAccess->makeNoTextFieldBookmark( *m_pPaM, aBookmarkName, ODF_FORMDROPDOWN ) );
            OSL_ENSURE(pFieldmark!=nullptr, "hmmm; why was the bookmark not created?");
            if ( pFieldmark != nullptr )
            {
                uno::Sequence< OUString > vListEntries(aFormula.maListEntries.size());
                ::std::copy(aFormula.maListEntries.begin(), aFormula.maListEntries.end(), vListEntries.begin());
                (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] = uno::makeAny(vListEntries);
                sal_Int32 nIndex = aFormula.mfDropdownIndex  < aFormula.maListEntries.size() ? aFormula.mfDropdownIndex : 0;
                (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_RESULT] = uno::makeAny(nIndex);
                // set field data here...
            }
        }

        return FLD_OK;
    }
}

eF_ResT SwWW8ImplReader::Read_F_HTMLControl(WW8FieldDesc*, OUString&)
{
    if( m_bObj && m_nPicLocFc )
        m_nObjLocFc = m_nPicLocFc;
    m_bEmbeddObj = true;
    return FLD_TEXT;
}

void SwWW8ImplReader::DeleteFormImpl()
{
    delete m_pFormImpl, m_pFormImpl = nullptr;
}

// Hilfs-Deklarationen

// Style Id's for each level
typedef sal_uInt16 WW8aIdSty[WW8ListManager::nMaxLevel];
// Zeichenattribute aus GrpprlChpx
typedef SfxItemSet* WW8aISet[WW8ListManager::nMaxLevel];
// Zeichen Style Pointer
typedef SwCharFormat* WW8aCFormat[WW8ListManager::nMaxLevel];

struct WW8LST   // nur DIE Eintraege, die WIR benoetigen!
{
    WW8aIdSty aIdSty;     // Style Id's for each level,
                            //   nIStDNil if no style linked
    sal_uInt32 nIdLst;     // Unique List ID
    sal_uInt32 nTplC;      // Unique template code - Was ist das bloss?
    bool bSimpleList:1;    // Flag: Liste hat nur EINEN Level
    bool bRestartHdn:1;    // WW6-Kompatibilitaets-Flag:
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

    bool bStartAt :1;       // true if the start-at value is overridden
    bool bFormat :1;        // true if the formatting is overridden

    WW8LFOLVL() :
        nStartAt(1), nLevel(0), bStartAt(true), bFormat(false) {}
};

// in den ListenInfos zu speichernde Daten

struct WW8LSTInfo   // sortiert nach nIdLst (in WW8 verwendete Listen-Id)
{
    std::vector<ww::bytes> maParaSprms;
    WW8aIdSty   aIdSty;          // Style Id's for each level
    WW8aISet    aItemSet;        // Zeichenattribute aus GrpprlChpx
    WW8aCFormat    aCharFormat;        // Zeichen Style Pointer

    SwNumRule*  pNumRule;        // Zeiger auf entsprechende Listenvorlage im Writer
    sal_uInt32      nIdLst;          // WW8Id dieser Liste
    bool bSimpleList:1;// Flag, ob diese NumRule nur einen Level verwendet
    bool bUsedInDoc :1;// Flag, ob diese NumRule im Doc verwendet wird,
                                                     //   oder beim Reader-Ende geloescht werden sollte

    WW8LSTInfo(SwNumRule* pNumRule_, WW8LST& aLST)
        : pNumRule(pNumRule_), nIdLst(aLST.nIdLst),
        bSimpleList(aLST.bSimpleList), bUsedInDoc(false)
    {
        memcpy( aIdSty, aLST.aIdSty, sizeof( aIdSty   ));
        memset(&aItemSet, 0,  sizeof( aItemSet ));
        memset(&aCharFormat, 0,  sizeof( aCharFormat ));
    }

};

// in den ListenFormatOverrideInfos zu speichernde Daten

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

    bool bOverride  :1;// Flag, ob die NumRule nicht in maLSTInfos steht,
                        //   sondern fuer m_LFOInfos NEU angelegt wurde
    bool bUsedInDoc :1;// Flag, ob diese NumRule im Doc verwendet wird,
                                                     //   oder beim Reader-Ende geloescht werden sollte
    bool bLSTbUIDSet    :1;// Flag, ob bUsedInDoc in maLSTInfos gesetzt wurde,
                                                     //   und nicht nochmals gesetzt zu werden braucht
    explicit WW8LFOInfo(const WW8LFO& rLFO);
};

WW8LFOInfo::WW8LFOInfo(const WW8LFO& rLFO)
    : maParaSprms(WW8ListManager::nMaxLevel)
    , maOverrides(WW8ListManager::nMaxLevel)
    , pNumRule(rLFO.pNumRule)
    , nIdLst(rLFO.nIdLst)
    , nLfoLvl(rLFO.nLfoLvl)
    , bOverride(rLFO.nLfoLvl != 0)
    , bUsedInDoc(false)
    , bLSTbUIDSet(false)
{
}

// Hilfs-Methoden

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
        return nullptr;
    return *aResult;
}

static void lcl_CopyGreaterEight(OUString &rDest, OUString &rSrc,
    sal_Int32 nStart, sal_Int32 nLen = SAL_MAX_INT32)
{
    const sal_Int32 nMaxLen = std::min(rSrc.getLength(), nLen);
    for( sal_Int32 nI = nStart; nI < nMaxLen; ++nI)
    {
        sal_Unicode nChar = rSrc[nI];
        if (nChar > WW8ListManager::nMaxLevel)
            rDest += OUString(nChar);
    }
}

OUString sanitizeString(const OUString& rString)
{
    sal_Int32 i=0;
    while (i < rString.getLength())
    {
        sal_Unicode c = rString[i];
        if (rtl::isHighSurrogate(c))
        {
            if (i+1 == rString.getLength()
                || !rtl::isLowSurrogate(rString[i+1]))
            {
                SAL_WARN("sw.ww8", "Surrogate error: high without low");
                return rString.copy(0, i);
            }
            ++i;    //skip correct low
        }
        if (rtl::isLowSurrogate(c)) //bare low without preceding high
        {
            SAL_WARN("sw.ww8", "Surrogate error: low without high");
            return rString.copy(0, i);
        }
        ++i;
    }
    return rString;
}

bool WW8ListManager::ReadLVL(SwNumFormat& rNumFormat, SfxItemSet*& rpItemSet,
    sal_uInt16 nLevelStyle, bool bSetStartNo,
    std::deque<bool> &rNotReallyThere, sal_uInt16 nLevel,
    ww::bytes &rParaSprms)
{
    sal_uInt8       aBits1(0);
    sal_uInt16      nStartNo(0);        // Start-Nr. fuer den Writer
    sal_uInt16      nType(style::NumberingType::ARABIC); // NumberintType
    SvxAdjust       eAdj;               // Ausrichtung (Links/rechts/zent.)
    sal_Unicode     cBullet(0x2190);    // default safe bullet

    sal_Unicode     cGrfBulletCP(USHRT_MAX);

    OUString        sPrefix;
    OUString        sPostfix;
    WW8LVL          aLVL;

    // 1. LVLF einlesen

    memset(&aLVL, 0, sizeof( aLVL ));
    rSt.ReadInt32( aLVL.nStartAt );
    rSt.ReadUChar( aLVL.nNFC );
    rSt.ReadUChar( aBits1 );
    if( 0 != rSt.GetError() ) return false;
    aLVL.nAlign = (aBits1 & 0x03);
    if( aBits1 & 0x10 ) aLVL.bV6Prev    = sal_uInt8(true);
    if( aBits1 & 0x20 ) aLVL.bV6PrSp    = sal_uInt8(true);
    if( aBits1 & 0x40 ) aLVL.bV6        = sal_uInt8(true);
    bool bLVLOkB = true;
    sal_uInt8 nLevelB = 0;
    for(nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
    {
        rSt.ReadUChar( aLVL.aOfsNumsXCH[ nLevelB ] );
        if( 0 != rSt.GetError() )
        {
            bLVLOkB = false;
            break;
        }
    }

    if( !bLVLOkB )
        return false;

    sal_uInt8 ixchFollow(0);
    rSt.ReadUChar( ixchFollow );
    rSt.ReadInt32( aLVL.nV6DxaSpace );
    rSt.ReadInt32( aLVL.nV6Indent );
    rSt.ReadUChar( aLVL.nLenGrpprlChpx );
    rSt.ReadUChar( aLVL.nLenGrpprlPapx );
    rSt.SeekRel( 2 );
    if( 0 != rSt.GetError()) return false;

    // 2. ggfs. PAPx einlesen und nach Einzug-Werten suchen

    short nTabPos = 0; // #i86652# - read tab setting
    if( aLVL.nLenGrpprlPapx )
    {
        sal_uInt8 aGrpprlPapx[ 255 ];
        if(aLVL.nLenGrpprlPapx != rSt.Read(&aGrpprlPapx,aLVL.nLenGrpprlPapx))
            return false;
        // "sprmPDxaLeft"  pap.dxaLeft;dxa;word;
        sal_uInt8* pSprm;
        if (
            (nullptr != (pSprm = GrpprlHasSprm(0x840F,aGrpprlPapx[0],aLVL.nLenGrpprlPapx))) ||
            (nullptr != (pSprm = GrpprlHasSprm(0x845E,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)))
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
            (nullptr != (pSprm = GrpprlHasSprm(0x8411,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) ) ||
            (nullptr != (pSprm = GrpprlHasSprm(0x8460,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) )
            )
        {
            sal_uInt8 *pBegin = pSprm-2;
            for(int i=0;i<4;++i)
                rParaSprms.push_back(*pBegin++);
            aLVL.nDxaLeft1 = SVBT16ToShort(  pSprm );
        }

        // #i86652# - read tab setting
        if(nullptr != (pSprm = GrpprlHasSprm(0xC615,aGrpprlPapx[0],aLVL.nLenGrpprlPapx)) )
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
        if ( rNumFormat.GetPositionAndSpaceMode() ==
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

    // 3. ggfs. CHPx einlesen und

    sal_uInt16 nWitchPicIsBullet = USHRT_MAX;
    bool bIsPicBullet = false;

    if( aLVL.nLenGrpprlChpx )
    {
        sal_uInt8 aGrpprlChpx[ 255 ];
        memset(&aGrpprlChpx, 0, sizeof( aGrpprlChpx ));
        if(aLVL.nLenGrpprlChpx != rSt.Read(&aGrpprlChpx, aLVL.nLenGrpprlChpx))
            return false;

        //For i120928,parse the graphic info of bullets
        sal_uInt8 *pSprmWhichPis = GrpprlHasSprm(NS_sprm::LN_CPbiIBullet, aGrpprlChpx[0],aLVL.nLenGrpprlChpx);
        sal_uInt8 *pSprmIsPicBullet = GrpprlHasSprm(NS_sprm::LN_CPbiGrf, aGrpprlChpx[0],aLVL.nLenGrpprlChpx);
        if (pSprmWhichPis)
        {
            nWitchPicIsBullet = *pSprmWhichPis;
        }
        if (pSprmIsPicBullet)
        {
            bIsPicBullet = (*pSprmIsPicBullet) & 0x0001;
        }

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
        // in WW8PAR6.CXX rufen ganz normal ihr NewAttr() oder GetFormatAttr()
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
        rReader.SetAktItemSet( nullptr );
        rReader.SetNAktColl( nOldColl );
        rReader.SetToggleAttrFlags(nOldFlags1);
        rReader.SetToggleBiDiAttrFlags(nOldFlags2);
    }

    // 4. den Nummerierungsstring einlesen: ergibt Prefix und Postfix

    OUString sNumString(sanitizeString(read_uInt16_PascalString(rSt)));

    // 5. gelesene Werte in Writer Syntax umwandeln

    if( 0 <= aLVL.nStartAt )
        nStartNo = (sal_uInt16)aLVL.nStartAt;

    switch( aLVL.nNFC )
    {
        case 0:
            nType = style::NumberingType::ARABIC;
            break;
        case 1:
            nType = style::NumberingType::ROMAN_UPPER;
            break;
        case 2:
            nType = style::NumberingType::ROMAN_LOWER;
            break;
        case 3:
            nType = style::NumberingType::CHARS_UPPER_LETTER_N;
            break;
        case 4:
            nType = style::NumberingType::CHARS_LOWER_LETTER_N;
            break;
        case 5:
            // eigentlich: ORDINAL
            nType = style::NumberingType::ARABIC;
            break;
        case 23:
            nType = style::NumberingType::CHAR_SPECIAL;
            //For i120928,type info
            if (bIsPicBullet)
            {
                nType = style::NumberingType::BITMAP;
            }

            break;
        case 255:
            nType = style::NumberingType::NUMBER_NONE;
            break;
        case 14:
        case 19:nType = style::NumberingType::FULLWIDTH_ARABIC;        break;
        case 30:nType = style::NumberingType::TIAN_GAN_ZH;        break;
        case 31:nType = style::NumberingType::DI_ZI_ZH;        break;
        case 35:
        case 36:
        case 37:
        case 39:
                nType = style::NumberingType::NUMBER_LOWER_ZH;        break;
        case 34:nType = style::NumberingType::NUMBER_UPPER_ZH_TW;break;
        case 38:nType = style::NumberingType::NUMBER_UPPER_ZH;        break;
        case 10:
        case 11:
                nType = style::NumberingType::NUMBER_TRADITIONAL_JA;break;
        case 20:nType = style::NumberingType::AIU_FULLWIDTH_JA;break;
        case 12:nType = style::NumberingType::AIU_HALFWIDTH_JA;break;
        case 21:nType = style::NumberingType::IROHA_FULLWIDTH_JA;break;
        case 13:nType = style::NumberingType::IROHA_HALFWIDTH_JA;break;
        case 24:nType = style::NumberingType::HANGUL_SYLLABLE_KO;break;
        case 25:nType = style::NumberingType::HANGUL_JAMO_KO;break;
        case 41:nType = style::NumberingType::NUMBER_HANGUL_KO;break;
        //case 42:
        //case 43:
        case 44:nType = style::NumberingType::NUMBER_UPPER_KO; break;
        default:
                nType= style::NumberingType::ARABIC;break;
    }

    //If a number level is not going to be used, then record this fact
    if (style::NumberingType::NUMBER_NONE == nType)
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
        if (nPos && nPos < sNumString.getLength()  && sNumString[nPos-1] < nMaxLevel)
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

    if (style::NumberingType::CHAR_SPECIAL == nType)
    {
        cBullet = !sNumString.isEmpty() ? sNumString[0] : 0x2190;

        if (!cBullet)  // unsave control code?
            cBullet = 0x2190;
    }
    else if (style::NumberingType::BITMAP == nType)   //For i120928,position index info of graphic
    {
        cGrfBulletCP = nWitchPicIsBullet;       // This is a bullet picture ID
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
        const sal_Int32 nFirstNoIndex =
            nOneBasedFirstNoIndex > 0 ? nOneBasedFirstNoIndex -1 : SAL_MAX_INT32;
        lcl_CopyGreaterEight(sPrefix, sNumString, 0, nFirstNoIndex);

        //Next number appears at
        if (nUpperLevel)
        {
            sal_uInt8 nOneBasedNextNoIndex = aOfsNumsXCH[nUpperLevel-1];
            const sal_Int32 nNextNoIndex =
                nOneBasedNextNoIndex > 0 ? nOneBasedNextNoIndex : SAL_MAX_INT32;
            if (sNumString.getLength() > nNextNoIndex)
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
            // undefined value
            OSL_ENSURE( false, "Value of aLVL.nAlign is not supported" );
            // take default
            eAdj = SVX_ADJUST_LEFT;
            break;
    }

    // 6. entsprechendes NumFormat konfigurieren
    if( bSetStartNo )
        rNumFormat.SetStart( nStartNo );
    rNumFormat.SetNumberingType( nType );
    rNumFormat.SetNumAdjust( eAdj );

    if( style::NumberingType::CHAR_SPECIAL == nType )
    {
        // first character of the Prefix-Text is the Bullet
        rNumFormat.SetBulletChar(cBullet);
        // Don't forget: unten, nach dem Bauen eventueller Styles auch noch
        // SetBulletFont() rufen !!!
    }
    //For i120928,position index info
    else if (style::NumberingType::BITMAP == nType)
    {
        rNumFormat.SetGrfBulletCP(cGrfBulletCP);
    }
    else
    {
        // reminder: Garnix is default Prefix
        if( !sPrefix.isEmpty() )
            rNumFormat.SetPrefix( sPrefix );
        // reminder: Point is default Postfix
        rNumFormat.SetSuffix( sPostfix );
        rNumFormat.SetIncludeUpperLevels( nUpperLevel );
    }

    // #i89181#
    if ( rNumFormat.GetPositionAndSpaceMode() ==
                              SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        if (eAdj == SVX_ADJUST_RIGHT)
        {
            rNumFormat.SetAbsLSpace(aLVL.nDxaLeft);
            rNumFormat.SetFirstLineOffset(-aLVL.nDxaLeft);
            rNumFormat.SetCharTextDistance(-aLVL.nDxaLeft1);
        }
        else
        {
            rNumFormat.SetAbsLSpace( aLVL.nDxaLeft );
            rNumFormat.SetFirstLineOffset(aLVL.nDxaLeft1);
        }
    }
    else
    {
        rNumFormat.SetIndentAt( aLVL.nDxaLeft );
        rNumFormat.SetFirstLineIndent(aLVL.nDxaLeft1);
        if ( !aLVL.bV6 )
            rNumFormat.SetListtabPos( nTabPos );
        else
            rNumFormat.SetListtabPos( aLVL.nV6Indent );
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
        rNumFormat.SetLabelFollowedBy( eNumLabelFollowedBy );
    }

    return true;
}

void WW8ListManager::AdjustLVL( sal_uInt8 nLevel, SwNumRule& rNumRule,
    WW8aISet& rListItemSet, WW8aCFormat& rCharFormat, bool& bNewCharFormatCreated,
    const OUString& sPrefix )
{
    bNewCharFormatCreated = false;
    SfxItemSet* pThisLevelItemSet;
    sal_uInt8 nIdenticalItemSetLevel;
    const SfxPoolItem* pItem;

    SwNumFormat aNumFormat  = rNumRule.Get( nLevel );

    pThisLevelItemSet = rListItemSet[ nLevel ];

    if( pThisLevelItemSet && pThisLevelItemSet->Count())
    {
        nIdenticalItemSetLevel = nMaxLevel;
        SfxItemIter aIter( *pThisLevelItemSet );
        for (sal_uInt8 nLowerLevel = 0; nLowerLevel < nLevel; ++nLowerLevel)
        {
            SfxItemSet* pLowerLevelItemSet = rListItemSet[ nLowerLevel ];
            if(     pLowerLevelItemSet
                && (pLowerLevelItemSet->Count() == pThisLevelItemSet->Count()) )
            {
                nIdenticalItemSetLevel = nLowerLevel;
                sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                while (true)
                {
                    if(  // ggfs. passenden pItem im pLowerLevelItemSet finden
                         (SfxItemState::SET != pLowerLevelItemSet->GetItemState(
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

        SwCharFormat* pFormat;
        if (nMaxLevel == nIdenticalItemSetLevel)
        {
            // Style definieren
            const OUString aName( (!sPrefix.isEmpty() ? sPrefix : rNumRule.GetName())
                                  + "z" + OUString::number( nLevel ) );

            // const Wegcasten
            pFormat = rDoc.MakeCharFormat(aName, rDoc.GetDfltCharFormat());
            bNewCharFormatCreated = true;
            // Attribute reinsetzen
            pFormat->SetFormatAttr( *pThisLevelItemSet );
        }
        else
        {
            // passenden Style hier anhaengen
            pFormat = rCharFormat[ nIdenticalItemSetLevel ];
        }

        // merken
        rCharFormat[ nLevel ] = pFormat;

        // Style an das NumFormat haengen

        aNumFormat.SetCharFormat( pFormat );
    }
    //Ensure the default char fmt is initialized for any level of num ruler if no customized attr
    else
    {
        SwCharFormat* pFormat = aNumFormat.GetCharFormat();
        if ( !pFormat)
        {
            const OUString aName( (!sPrefix.isEmpty() ? sPrefix : rNumRule.GetName())
                                  + "z" + OUString::number( nLevel ) );

            pFormat = rDoc.MakeCharFormat(aName, rDoc.GetDfltCharFormat());
            bNewCharFormatCreated = true;
            rCharFormat[ nLevel ] = pFormat;
            aNumFormat.SetCharFormat( pFormat );
        }
    }

    // ggfs. Bullet Font an das NumFormat haengen

    if( SVX_NUM_CHAR_SPECIAL == aNumFormat.GetNumberingType() )
    {
        SwCharFormat* pFormat = aNumFormat.GetCharFormat();
        vcl::Font aFont;
        if( !pFormat )
        {
            aFont = numfunc::GetDefBulletFont();
        }
        else
        {
            const SvxFontItem& rFontItem = pFormat->GetFont();
            aFont.SetFamily(    rFontItem.GetFamily()     );
            aFont.SetName(      rFontItem.GetFamilyName() );
            aFont.SetStyleName( rFontItem.GetStyleName()  );
            aFont.SetPitch(     rFontItem.GetPitch()      );
            aFont.SetCharSet(   rFontItem.GetCharSet()    );
        }
        aNumFormat.SetBulletFont( &aFont );
    }

    // und wieder rein in die NumRule

    rNumRule.Set(nLevel, aNumFormat);
}

SwNumRule* WW8ListManager::CreateNextRule(bool bSimple)
{
    // wird erstmal zur Bildung des Style Namens genommen
    const OUString sPrefix("WW8Num" + OUString::number(nUniqueList++));
    // #i86652#
    sal_uInt16 nRul =
            rDoc.MakeNumRule( rDoc.GetUniqueNumRuleName(&sPrefix), nullptr, false,
                              SvxNumberFormat::LABEL_ALIGNMENT );
    SwNumRule* pMyNumRule = rDoc.GetNumRuleTable()[nRul];
    pMyNumRule->SetAutoRule(false);
    pMyNumRule->SetContinusNum(bSimple);
    return pMyNumRule;
}

SwNumRule* WW8ListManager::GetNumRule(size_t i)
{
    if (i < maLSTInfos.size())
        return maLSTInfos[i]->pNumRule;
    else
        return nullptr;
}

// oeffentliche Methoden

WW8ListManager::WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_)
    : maSprmParser(rReader_.GetFib().GetFIBVersion()), rReader(rReader_)
    , rDoc(rReader.GetDoc())
    , rFib(rReader.GetFib()), rSt(rSt_)
    , nUniqueList(1)
    , nLastLFOPosition(USHRT_MAX)
{

    // LST und LFO gibts erst ab WW8
    if(    ( 8 > rFib.nVersion )
            || ( rFib.fcPlcfLst == rFib.fcPlfLfo )
            || ( rFib.lcbPlcfLst < 2 )
            || ( rFib.lcbPlfLfo < 2) ) return; // offensichtlich keine Listen da

    // Arrays anlegen
    bool bLVLOk = true;

    long nOriginalPos = rSt.Tell();

    // 1. PLCF LST auslesen und die Listen Vorlagen im Writer anlegen

    bool bOk = checkSeek(rSt, rFib.fcPlcfLst);

    if (!bOk)
        return;

    sal_uInt32 nRemainingPlcfLst = rFib.lcbPlcfLst;

    sal_uInt16 nListCount(0);
    rSt.ReadUInt16( nListCount );
    nRemainingPlcfLst -= 2;
    bOk = nListCount > 0;

    if (!bOk)
        return;

    // 1.1 alle LST einlesen
    const size_t nMinRecordSize = 10 + 2*nMaxLevel;
    const size_t nMaxRecords = rSt.remainingSize() / nMinRecordSize;
    if (nListCount > nMaxRecords)
    {
        SAL_WARN("sw.ww8", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nListCount << " claimed, truncating");
        nListCount = nMaxRecords;
    }
    for (sal_uInt16 nList=0; nList < nListCount; ++nList)
    {
        if (nRemainingPlcfLst < cbLSTF)
            break;

        WW8LST aLST;
        memset(&aLST, 0, sizeof( aLST ));

        // 1.1.1 Daten einlesen

        rSt.ReadUInt32( aLST.nIdLst );
        rSt.ReadUInt32( aLST.nTplC );
        for (sal_uInt16 nLevel = 0; nLevel < nMaxLevel; ++nLevel)
            rSt.ReadUInt16( aLST.aIdSty[ nLevel ] );

        sal_uInt8 aBits1(0);
        rSt.ReadUChar( aBits1 );

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
        continuous flag in these lists to store this information.
        */
        SwNumRule* pMyNumRule = CreateNextRule(
            aLST.bSimpleList || (aBits1 & 0x10));

        WW8LSTInfo* pLSTInfo = new WW8LSTInfo(pMyNumRule, aLST);
        maLSTInfos.push_back(pLSTInfo);

        nRemainingPlcfLst -= cbLSTF;
    }

    // 1.2 alle LVL aller aLST einlesen

    sal_uInt16 nLSTInfos = static_cast< sal_uInt16 >(maLSTInfos.size());
    for (sal_uInt16 nList = 0; nList < nLSTInfos; ++nList)
    {
        WW8LSTInfo* pListInfo = maLSTInfos[nList];
        if( !pListInfo || !pListInfo->pNumRule ) break;
        SwNumRule& rMyNumRule = *pListInfo->pNumRule;

        // 1.2.1 betreffende(n) LVL(s) fuer diese aLST einlesen

        sal_uInt16 nLvlCount = static_cast< sal_uInt16 >(pListInfo->bSimpleList ? nMinLevel : nMaxLevel);
        std::deque<bool> aNotReallyThere;
        aNotReallyThere.resize(nMaxLevel);
        pListInfo->maParaSprms.resize(nMaxLevel);
        for (sal_uInt16 nLevel = 0; nLevel < nLvlCount; ++nLevel)
        {
            SwNumFormat aNumFormat( rMyNumRule.Get( nLevel ) );
            // LVLF einlesen
            bLVLOk = ReadLVL( aNumFormat, pListInfo->aItemSet[nLevel],
                pListInfo->aIdSty[nLevel], true, aNotReallyThere, nLevel,
                pListInfo->maParaSprms[nLevel]);
            if( !bLVLOk )
                break;
            // und in die rMyNumRule aufnehmen
            rMyNumRule.Set( nLevel, aNumFormat );
        }
        if( !bLVLOk )
            break;

        // 1.2.2 die ItemPools mit den CHPx Einstellungen der verschiedenen
        //       Level miteinander vergleichen und ggfs. Style(s) erzeugen

        for (sal_uInt16 nLevel = 0; nLevel < nLvlCount; ++nLevel)
        {
            bool bDummy;
            AdjustLVL( nLevel, rMyNumRule, pListInfo->aItemSet,
                                           pListInfo->aCharFormat, bDummy );
        }

        // 1.2.3 ItemPools leeren und loeschen

        for (sal_uInt16 nLevel = 0; nLevel < nLvlCount; ++nLevel)
            delete pListInfo->aItemSet[ nLevel ];
    }

    // 2. PLF LFO auslesen und speichern

    bOk = checkSeek(rSt, rFib.fcPlfLfo);

    if (!bOk)
        return;

    sal_Int32 nLfoCount(0);
    rSt.ReadInt32( nLfoCount );
    bOk = nLfoCount > 0;

    if (!bOk)
        return;

    // 2.1 alle LFO einlesen

    for (sal_Int32 nLfo = 0; nLfo < nLfoCount; ++nLfo)
    {
        bOk = false;

        WW8LFO aLFO;
        memset(&aLFO, 0, sizeof( aLFO ));

        rSt.ReadUInt32( aLFO.nIdLst );
        rSt.SeekRel( 8 );
        rSt.ReadUChar( aLFO.nLfoLvl );
        if (!rSt.good())
            break;
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
        std::unique_ptr<WW8LFOInfo> pLFOInfo(new WW8LFOInfo(aLFO));
        if (pParentListInfo)
        {
            //Copy the basic paragraph properties for each level from the
            //original list into the list format override levels.
            int nMaxSize = pParentListInfo->maParaSprms.size();
            pLFOInfo->maParaSprms.resize(nMaxSize);
            for (int i = 0; i < nMaxSize; ++i)
                pLFOInfo->maParaSprms[i] = pParentListInfo->maParaSprms[i];
        }
        m_LFOInfos.push_back(std::move(pLFOInfo));
        bOk = true;
    }

    if( bOk )
    {

        // 2.2 fuer alle LFO die zugehoerigen LFOLVL einlesen

        size_t nLFOInfos = m_LFOInfos.size();
        for (size_t nLfo = 0; nLfo < nLFOInfos; ++nLfo)
        {
            WW8LFOInfo& rLFOInfo = *m_LFOInfos[nLfo];
            // stehen hierfuer ueberhaupt LFOLVL an ?
            if( rLFOInfo.bOverride )
            {
                WW8LSTInfo* pParentListInfo = GetLSTByListId(rLFOInfo.nIdLst);
                if (!pParentListInfo)
                    break;

                // 2.2.1 eine neue NumRule fuer diese Liste anlegen

                SwNumRule* pParentNumRule = rLFOInfo.pNumRule;
                OSL_ENSURE(pParentNumRule, "ww: Impossible lists, please report");
                if( !pParentNumRule )
                    break;
                // Nauemsprefix aufbauen: fuer NumRule-Name (eventuell)
                // und (falls vorhanden) fuer Style-Name (dann auf jeden Fall)
                const OUString sPrefix("WW8NumSt" + OUString::number( nLfo + 1 ));
                // jetzt dem pNumRule seinen RICHTIGEN Wert zuweisen !!!
                // (bis dahin war hier die Parent NumRule vermerkt )

                // Dazu erst mal nachsehen, ob ein Style diesen LFO
                // referenziert:
                if( USHRT_MAX > rReader.StyleUsingLFO( nLfo ) )
                {
                    sal_uInt16 nRul = rDoc.MakeNumRule(
                        rDoc.GetUniqueNumRuleName( &sPrefix ), pParentNumRule);
                    rLFOInfo.pNumRule = rDoc.GetNumRuleTable()[ nRul ];
                    rLFOInfo.pNumRule->SetAutoRule(false);
                }
                else
                {
                    sal_uInt16 nRul = rDoc.MakeNumRule(
                        rDoc.GetUniqueNumRuleName(), pParentNumRule);
                    rLFOInfo.pNumRule = rDoc.GetNumRuleTable()[ nRul ];
                    rLFOInfo.pNumRule->SetAutoRule(true);  // = default
                }

                // 2.2.2 alle LFOLVL (und ggfs. LVL) fuer die neue NumRule
                // einlesen

                WW8aISet aItemSet;       // Zeichenattribute aus GrpprlChpx
                WW8aCFormat aCharFormat;       // Zeichen Style Pointer
                memset(&aItemSet, 0,  sizeof( aItemSet ));
                memset(&aCharFormat, 0,  sizeof( aCharFormat ));

                //2.2.2.0 skip inter-group of override header ?
                //See #i25438# for why I moved this here, compare
                //that original bugdoc's binary to what it looks like
                //when resaved with word, i.e. there is always a
                //4 byte header, there might be more than one if
                //that header was 0xFFFFFFFF, e.g. #114412# ?
                sal_uInt32 nTest;
                rSt.ReadUInt32( nTest );
                do
                {
                    nTest = 0;
                    rSt.ReadUInt32( nTest );
                }
                while (nTest == 0xFFFFFFFF);
                rSt.SeekRel(-4);

                std::deque<bool> aNotReallyThere(WW8ListManager::nMaxLevel);
                for (sal_uInt8 nLevel = 0; nLevel < rLFOInfo.nLfoLvl; ++nLevel)
                {
                    WW8LFOLVL aLFOLVL;
                    bLVLOk = false;

                    // 2.2.2.1 den LFOLVL einlesen

                    rSt.ReadInt32( aLFOLVL.nStartAt );
                    sal_uInt8 aBits1(0);
                    rSt.ReadUChar( aBits1 );
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

                        // 2.2.2.2 eventuell auch den zugehoerigen LVL einlesen

                        SwNumFormat aNumFormat(
                            rLFOInfo.pNumRule->Get(aLFOLVL.nLevel));
                        if (aBits1 & 0x20)
                        {
                            aLFOLVL.bFormat = true;
                            // falls bStartup true, hier den Startup-Level
                            // durch den im LVL vermerkten ersetzen LVLF
                            // einlesen
                            bLVLOk = ReadLVL(aNumFormat, aItemSet[nLevel],
                                pParentListInfo->aIdSty[nLevel],
                                aLFOLVL.bStartAt, aNotReallyThere, nLevel,
                                rLFOInfo.maParaSprms[nLevel]);

                            if (!bLVLOk)
                                break;
                        }
                        else if (aLFOLVL.bStartAt)
                        {
                            aNumFormat.SetStart(
                                writer_cast<sal_uInt16>(aLFOLVL.nStartAt));
                        }

                        // 2.2.2.3 das NumFormat in die NumRule aufnehmen

                        rLFOInfo.pNumRule->Set(aLFOLVL.nLevel, aNumFormat);
                    }
                    bLVLOk = true;

                    if (nMaxLevel > aLFOLVL.nLevel)
                        rLFOInfo.maOverrides[aLFOLVL.nLevel] = aLFOLVL;
                }
                if( !bLVLOk )
                    break;

                // 2.2.3 die LVL der neuen NumRule anpassen

                sal_uInt16 aFlagsNewCharFormat = 0;
                bool bNewCharFormatCreated = false;
                for (sal_uInt8 nLevel = 0; nLevel < rLFOInfo.nLfoLvl; ++nLevel)
                {
                    AdjustLVL( nLevel, *rLFOInfo.pNumRule, aItemSet, aCharFormat,
                        bNewCharFormatCreated, sPrefix );
                    if( bNewCharFormatCreated )
                        aFlagsNewCharFormat += (1 << nLevel);
                }

                // 2.2.4 ItemPools leeren und loeschen

                for (sal_uInt8 nLevel = 0; nLevel < rLFOInfo.nLfoLvl; ++nLevel)
                    delete aItemSet[ nLevel ];
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
    for (auto aIter = m_LFOInfos.rbegin(); aIter != m_LFOInfos.rend(); ++aIter)
    {
        if ((*aIter)->bOverride
            && (*aIter)->pNumRule
            && !(*aIter)->bUsedInDoc
            && (*aIter)->pNumRule->IsAutoRule())
        {
            rDoc.DelNumRule( (*aIter)->pNumRule->GetName() );
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
          rOne.GetPoolFormatId() == rTwo.GetPoolFormatId() &&
          rOne.GetPoolHelpId() == rTwo.GetPoolHelpId() &&
          rOne.GetPoolHlpFileId() == rTwo.GetPoolHlpFileId()
        );

    if (bRet)
    {
        for (sal_uInt8 n = 0; n < MAXLEVEL; ++n )
        {
            //The SvxNumberFormat compare, not the SwNumFormat compare
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
    const sal_uInt8 nLevel, std::vector<sal_uInt8> &rParaSprms, SwTextNode *pNode)
{
    if (m_LFOInfos.size() <= nLFOPosition)
        return nullptr;

    WW8LFOInfo& rLFOInfo = *m_LFOInfos[nLFOPosition];

    bool bFirstUse = !rLFOInfo.bUsedInDoc;
    rLFOInfo.bUsedInDoc = true;

    if( !rLFOInfo.pNumRule )
        return nullptr;

    // #i25545#
    // #i100132# - a number format does not have to exist on given list level
    SwNumFormat aFormat(rLFOInfo.pNumRule->Get(nLevel));

    if (rReader.IsRightToLeft() && nLastLFOPosition != nLFOPosition) {
        if ( aFormat.GetNumAdjust() == SVX_ADJUST_RIGHT)
            aFormat.SetNumAdjust(SVX_ADJUST_LEFT);
        else if ( aFormat.GetNumAdjust() == SVX_ADJUST_LEFT)
            aFormat.SetNumAdjust(SVX_ADJUST_RIGHT);
        rLFOInfo.pNumRule->Set(nLevel, aFormat);
    }
    nLastLFOPosition = nLFOPosition;
    /*
    #i1869#
    If this list has had its bits set in word 2000 to pretend that it is a
    simple list from the point of view of the user, then it is almost
    certainly a simple continuous list, and we will try to keep it like that.
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
                        const SwNumFormat &rFormat =
                            pParentListInfo->pNumRule->Get(nLevel);
                        if (
                             rFormat.GetStart() ==
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

//          SwWW8ImplReader:  anhaengen einer Liste an einen Style oder Absatz

bool SwWW8ImplReader::SetTextFormatCollAndListLevel(const SwPaM& rRg,
    SwWW8StyInf& rStyleInfo)
{
    bool bRes = true;
    if( rStyleInfo.m_pFormat && rStyleInfo.m_bColl )
    {
        bRes = m_rDoc.SetTextFormatColl(rRg, static_cast<SwTextFormatColl*>(rStyleInfo.m_pFormat));
        SwTextNode* pTextNode = m_pPaM->GetNode().GetTextNode();
        OSL_ENSURE( pTextNode, "No Text-Node at PaM-Position" );
        if ( !pTextNode )
        {
            // make code robust
            return bRes;
        }

        const SwNumRule * pNumRule = pTextNode->GetNumRule(); // #i27610#

        if( !IsInvalidOrToBeMergedTabCell() &&
            ! (pNumRule && pNumRule->IsOutlineRule()) ) // #i27610#
        {
            pTextNode->ResetAttr( RES_PARATR_NUMRULE );
        }

        if (USHRT_MAX > rStyleInfo.m_nLFOIndex && WW8ListManager::nMaxLevel
                                                > rStyleInfo.m_nListLevel)
        {
            const bool bApplyListStyle = false;
            RegisterNumFormatOnTextNode(rStyleInfo.m_nLFOIndex, rStyleInfo.m_nListLevel,
                                    bApplyListStyle);
        }
    }
    return bRes;
}

void UseListIndent(SwWW8StyInf &rStyle, const SwNumFormat &rFormat)
{
    // #i86652#
    if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        const long nAbsLSpace = rFormat.GetAbsLSpace();
        const long nListFirstLineIndent = GetListFirstLineIndent(rFormat);
        SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*rStyle.m_pFormat, RES_LR_SPACE));
        aLR.SetTextLeft(nAbsLSpace);
        aLR.SetTextFirstLineOfst(writer_cast<short>(nListFirstLineIndent));
        rStyle.m_pFormat->SetFormatAttr(aLR);
        rStyle.m_bListReleventIndentSet = true;
    }
}

void SetStyleIndent(SwWW8StyInf &rStyle, const SwNumFormat &rFormat)
{
    if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION ) // #i86652#
    {
        SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*rStyle.m_pFormat, RES_LR_SPACE));
        if (rStyle.m_bListReleventIndentSet)
        {

            SyncIndentWithList( aLR, rFormat, false, false ); // #i103711#, #i105414#
        }
        else
        {
            aLR.SetTextLeft(0);
            aLR.SetTextFirstLineOfst(0);
        }
        rStyle.m_pFormat->SetFormatAttr(aLR);
    }
}

void SwWW8ImplReader::SetStylesList(sal_uInt16 nStyle, sal_uInt16 nActLFO,
    sal_uInt8 nActLevel)
{
    if (nStyle >= m_vColl.size())
        return;

    SwWW8StyInf &rStyleInf = m_vColl[nStyle];
    if (rStyleInf.m_bValid)
    {
        OSL_ENSURE(m_pAktColl, "Cannot be called outside of style import");
        // Phase 1: Nummerierungsattribute beim Einlesen einer StyleDef
        if( m_pAktColl )
        {
            // jetzt nur die Parameter vermerken: die tatsaechliche Liste wird
            // spaeter drangehaengt, wenn die Listendefinitionen gelesen sind...
            if (
                 (USHRT_MAX > nActLFO) &&
                 (WW8ListManager::nMaxLevel > nActLevel)
               )
            {
                rStyleInf.m_nLFOIndex  = nActLFO;
                rStyleInf.m_nListLevel = nActLevel;

                if (
                    (USHRT_MAX > nActLFO) &&
                    (WW8ListManager::nMaxLevel > nActLevel)
                   )
                {
                    std::vector<sal_uInt8> aParaSprms;
                    SwNumRule *pNmRule =
                        m_pLstManager->GetNumRuleForActivation(nActLFO,
                            nActLevel, aParaSprms);
                    if (pNmRule)
                        UseListIndent(rStyleInf, pNmRule->Get(nActLevel));
                }
            }
        }
    }
}

void SwWW8ImplReader::RegisterNumFormatOnStyle(sal_uInt16 nStyle)
{

    if (nStyle >= m_vColl.size())
        return;

    SwWW8StyInf &rStyleInf = m_vColl[nStyle];
    if (rStyleInf.m_bValid && rStyleInf.m_pFormat)
    {
        //Save old pre-list modified indent, which are the word indent values
        rStyleInf.maWordLR =
            ItemGet<SvxLRSpaceItem>(*rStyleInf.m_pFormat, RES_LR_SPACE);

        // Phase 2: aktualisieren der StyleDef nach einlesen aller Listen
        SwNumRule* pNmRule = nullptr;
        const sal_uInt16 nLFO = rStyleInf.m_nLFOIndex;
        const sal_uInt8  nLevel = rStyleInf.m_nListLevel;
        if (
             (USHRT_MAX > nLFO) &&
             (WW8ListManager::nMaxLevel > nLevel)
           )
        {
            std::vector<sal_uInt8> aParaSprms;
            pNmRule = m_pLstManager->GetNumRuleForActivation(nLFO, nLevel,
                aParaSprms);

            if (pNmRule != nullptr)
            {
                if (rStyleInf.IsWW8BuiltInHeadingStyle()
                    && rStyleInf.HasWW8OutlineLevel())
                {
                    rStyleInf.m_pOutlineNumrule = pNmRule;
                }
                else
                {
                    rStyleInf.m_pFormat->SetFormatAttr(
                        SwNumRuleItem(pNmRule->GetName()));
                    rStyleInf.m_bHasStyNumRule = true;
                }
            }
        }

        if (pNmRule)
            SetStyleIndent(rStyleInf, pNmRule->Get(nLevel));
    }
}

void SwWW8ImplReader::RegisterNumFormatOnTextNode(sal_uInt16 nActLFO,
                                              sal_uInt8 nActLevel,
                                              const bool bSetAttr)
{
    // beachte: die Methode haengt die NumRule an den Text Node, falls
    // bSetAttr (dann muessen natuerlich vorher die Listen gelesen sein)
    // stellt sie NUR den Level ein, im Vertrauen darauf, dass am STYLE eine
    // NumRule haengt - dies wird NICHT ueberprueft !!!

    if (m_pLstManager) // sind die Listendeklarationen gelesen?
    {
        SwTextNode* pTextNd = m_pPaM->GetNode().GetTextNode();
        OSL_ENSURE(pTextNd, "No Text-Node at PaM-Position");
        if (!pTextNd)
            return;

        std::vector<sal_uInt8> aParaSprms;
        const SwNumRule* pRule = bSetAttr ?
            m_pLstManager->GetNumRuleForActivation( nActLFO, nActLevel,
                aParaSprms, pTextNd) : nullptr;

        if (pRule != nullptr || !bSetAttr)
        {
            if (bSetAttr && pTextNd->GetNumRule() != pRule
                && pTextNd->GetNumRule() != m_rDoc.GetOutlineNumRule())
            {
                pTextNd->SetAttr(SwNumRuleItem(pRule->GetName()));
            }
            pTextNd->SetAttrListLevel(nActLevel);

            // <IsCounted()> state of text node has to be adjusted accordingly.
            if ( /*nActLevel >= 0 &&*/ nActLevel < MAXLEVEL )
            {
                pTextNd->SetCountedInList( true );
            }

            // #i99822#
            // Direct application of the list level formatting no longer
            // needed for list levels of mode LABEL_ALIGNMENT
            bool bApplyListLevelIndentDirectlyAtPara(true);
            {
                if (pTextNd->GetNumRule() && nActLevel < MAXLEVEL)
                {
                    const SwNumFormat& rFormat = pTextNd->GetNumRule()->Get(nActLevel);
                    if (rFormat.GetPositionAndSpaceMode()
                        == SvxNumberFormat::LABEL_ALIGNMENT)
                    {
                        bApplyListLevelIndentDirectlyAtPara = false;
                    }
                }
            }

            if (bApplyListLevelIndentDirectlyAtPara)
            {
                SfxItemSet aListIndent(m_rDoc.GetAttrPool(), RES_LR_SPACE,
                        RES_LR_SPACE);
                const SvxLRSpaceItem *pItem = static_cast<const SvxLRSpaceItem*>(
                    GetFormatAttr(RES_LR_SPACE));
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
                    SfxItemSet* pOldAktItemSet = m_pAktItemSet;
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

                if (const SvxLRSpaceItem *pLR = aListIndent.GetItem<SvxLRSpaceItem>(RES_LR_SPACE))
                {
                    m_pCtrlStck->NewAttr(*m_pPaM->GetPoint(), *pLR);
                    m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_LR_SPACE);
                }
            }
        }
    }
}

void SwWW8ImplReader::RegisterNumFormat(sal_uInt16 nActLFO, sal_uInt8 nActLevel)
{
    // sind wir erst beim Einlesen der StyleDef ?
    if (m_pAktColl)
        SetStylesList( m_nAktColl , nActLFO, nActLevel);
    else
        RegisterNumFormatOnTextNode(nActLFO, nActLevel);
}

void SwWW8ImplReader::Read_ListLevel(sal_uInt16, const sal_uInt8* pData,
    short nLen)
{
    if (m_pPlcxMan && m_pPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // the actual level is finished, what should we do ?
        m_nListLevel = WW8ListManager::nMaxLevel;
        if (m_pStyles && !m_bVer67)
            m_pStyles->nWwNumLevel = 0;
    }
    else
    {
        // security check
        if( !pData )
            return;

        // die Streamdaten sind hier Null basiert, so wie wir es brauchen
        m_nListLevel = *pData;

        if (m_pStyles && !m_bVer67)
        {
            /*
            if this is the case, then if the numbering is actually stored in
            winword 6 format, and its likely that sprmPIlvl has been abused
            to set the ww6 list level information which we will need when we
            reach the true ww6 list def.  So set it now
            */
            m_pStyles->nWwNumLevel = m_nListLevel;
        }

        if (WW8ListManager::nMaxLevel <= m_nListLevel )
            m_nListLevel = WW8ListManager::nMaxLevel;
        else if
           (
             (USHRT_MAX > m_nLFOPosition) &&
             (WW8ListManager::nMaxLevel > m_nListLevel)
           )
        {
            RegisterNumFormat(m_nLFOPosition, m_nListLevel);
            m_nLFOPosition = USHRT_MAX;
            m_nListLevel  = WW8ListManager::nMaxLevel;
        }
    }
}

void SwWW8ImplReader::Read_LFOPosition(sal_uInt16, const sal_uInt8* pData,
    short nLen)
{
    if (m_pPlcxMan && m_pPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // the actual level is finished, what should we do ?
        m_nLFOPosition = USHRT_MAX;
        m_nListLevel = WW8ListManager::nMaxLevel;
    }
    else
    {
        // security check
        if( !pData )
            return;

        short nData = SVBT16ToShort( pData );
        if( 0 >= nData )
        {
            // disable the numbering/list style apply to the paragraph or the style

            /*
            If you have a paragraph in word with left and/or hanging indent
            and remove its numbering, then the indentation appears to get
            reset, but not back to the base style, instead its goes to a blank
            setting.
            Unless it's a broken ww6 list in 97 in which case more hackery is
            required, some more details about broken ww6 list in
            ww8par6.cxx#SwWW8ImplReader::Read_LR
            */

            if (m_pAktColl)
            {
                // here a "named" style is being configured

                // disable the numbering/list in the style currently configured
                m_pAktColl->SetFormatAttr(*GetDfltAttr(RES_PARATR_NUMRULE));

                // reset/blank the indent
                m_pAktColl->SetFormatAttr(SvxLRSpaceItem(RES_LR_SPACE));
            }
            else if (SwTextNode* pTextNode = m_pPaM->GetNode().GetTextNode())
            {
                // here a paragraph is being directly formatted

                // empty the numbering/list style applied to the current paragraph
                SwNumRuleItem aEmptyRule( aEmptyOUStr );
                pTextNode->SetAttr( aEmptyRule );

                // create an empty SvxLRSpaceItem
                SvxLRSpaceItem aLR( RES_LR_SPACE );

                // replace it with the one of the current node if it exist
                const SfxPoolItem* pLR = GetFormatAttr(RES_LR_SPACE);
                if( pLR )
                    aLR = *static_cast<const SvxLRSpaceItem*>(pLR);

                // reset/blank the left indent (and only the left)
                aLR.SetTextLeft(0);
                aLR.SetTextFirstLineOfst(0);

                // apply the modified SvxLRSpaceItem to the current paragraph
                pTextNode->SetAttr( aLR );
            }

            m_nLFOPosition = USHRT_MAX;
        }
        else
        {
            m_nLFOPosition = (sal_uInt16)nData-1;
            /*
            If we are a ww8+ style with ww7- style lists then there is a
            bizarre broken word bug where when the list is removed from a para
            the ww6 list first line indent still affects the first line
            indentation.  Setting this flag will allow us to recover from this
            braindeadness
            */
            if (m_pAktColl && (m_nLFOPosition == 2047-1) && m_nAktColl < m_vColl.size())
                m_vColl[m_nAktColl].m_bHasBrokenWW6List = true;

            // die Streamdaten sind hier 1 basiert, wir ziehen EINS ab
            if (USHRT_MAX > m_nLFOPosition)
            {
                if (m_nLFOPosition != 2047-1) //Normal ww8+ list behaviour
                {
                    if (WW8ListManager::nMaxLevel == m_nListLevel)
                        m_nListLevel = 0;
                    if (WW8ListManager::nMaxLevel > m_nListLevel)
                    {
                        RegisterNumFormat(m_nLFOPosition, m_nListLevel);
                        m_nLFOPosition = USHRT_MAX;
                        m_nListLevel = WW8ListManager::nMaxLevel;
                    }
                }
                else if (m_pPlcxMan && m_pPlcxMan->HasParaSprm(0xC63E))
                {
                    /*
                     #i8114# Horrific backwards compatible ww7- lists in ww8+
                     docs
                    */
                    Read_ANLevelNo(13 /*equiv ww7- sprm no*/, &m_nListLevel, 1);
                }
            }
        }
    }
}

// Reading Controls

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
    nStart = m_pPlcxMan->Where();
    while(nStart <= nEndCp)
    {
        if ( m_pPlcxMan->Get(&aRes)
            && aRes.pMemPos && aRes.nSprmId )
        {
            //only interested in sprms which would set nPicLocFc
            if ( (68 == aRes.nSprmId) || (0x6A03 == aRes.nSprmId) )
            {
                Read_PicLoc( aRes.nSprmId, aRes.pMemPos +
                    m_pSprmParser->DistanceToData(aRes.nSprmId), 4);
                break;
            }
        }
        m_pPlcxMan->advance();
        nStart = m_pPlcxMan->Where();
    }
    sal_uLong nOffset = m_nPicLocFc;
    aSave.Restore(this);

    sal_uLong nOldPos = m_pDataStream->Tell();
    WW8_PIC aPic;
    m_pDataStream->Seek( nOffset);
    PicRead( m_pDataStream, &aPic, m_bVer67);

    if((aPic.lcb > 0x3A) && !m_pDataStream->GetError() )
    {
        aFormula.FormulaRead(nWhich,m_pDataStream);
        bRet = true;
    }

    /*
     There is a problem with aPic, the WW8_PIC is always used even though it
     is too big for the WW95 files, it needs to be modified to check the
     version C.
     */
    m_pDataStream->Seek( nOldPos );
    return bRet;
}

bool SwMSConvertControls::InsertFormula(WW8FormulaControl &rFormula)
{
    bool bRet = false;

    const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
        GetServiceFactory();

    if(!rServiceFactory.is())
        return false;

    awt::Size aSz;
    uno::Reference< form::XFormComponent> xFComp;

    if ((bRet = rFormula.Import(rServiceFactory, xFComp, aSz)))
    {
        uno::Reference <drawing::XShape> xShapeRef;
        if ((bRet = InsertControl(xFComp, aSz, &xShapeRef, false)))
            GetShapes()->add(xShapeRef);
    }
    return bRet;
}

void WW8FormulaControl::FormulaRead(SwWw8ControlType nWhich,
    SvStream *pDataStream)
{
    sal_uInt8 nField;
    // nHeaderBype == version
    sal_uInt32 nHeaderByte = 0;

    // The following is a FFData structure as described in
    // Microsoft's DOC specification (chapter 2.9.78)

    pDataStream->ReadUInt32( nHeaderByte );

    // might be better to read the bits as a 16 bit word
    // ( like it is in the spec. )
    sal_uInt8 bits1 = 0;
    pDataStream->ReadUChar( bits1 );
    sal_uInt8 bits2 = 0;
    pDataStream->ReadUChar( bits2 );

    sal_uInt8 iType = ( bits1 & 0x3 );

    // we should verify that bits.iType & nWhich concur
    OSL_ENSURE( iType == nWhich, "something wrong, expect control type read from stream doesn't match nWhich passed in");
    if ( !( iType == nWhich ) )
        return; // bail out

    sal_uInt8 iRes = (bits1 & 0x7C) >> 2;

    pDataStream->ReadUInt16( mnMaxLen );

    sal_uInt16 hps = 0;
    pDataStream->ReadUInt16( hps );

    // xstzName
    msTitle = read_uInt16_BeltAndBracesString(*pDataStream);

    if (nWhich == WW8_CT_EDIT)
    {   // Field is a textbox
        // Default text
        // xstzTextDef
        msDefault = read_uInt16_BeltAndBracesString(*pDataStream);
    }
    else
    {
        // CheckBox or ComboBox
        sal_uInt16 wDef = 0;
        pDataStream->ReadUInt16( wDef );
        mnChecked = wDef; // default
        if (nWhich == WW8_CT_CHECKBOX)
        {
            if ( iRes != 25 )
                mnChecked = iRes;
            msDefault = ( wDef == 0 ) ? OUString( "0" ) :  OUString( "1" );
        }
    }
    // xstzTextFormat
    msFormatting = read_uInt16_BeltAndBracesString(*pDataStream);
    // xstzHelpText
    msHelp = read_uInt16_BeltAndBracesString(*pDataStream);
    // xstzStatText
    msToolTip = read_uInt16_BeltAndBracesString(*pDataStream);

    /*String sEntryMacro =*/ read_uInt16_BeltAndBracesString(*pDataStream);
    /*String sExitMcr =*/ read_uInt16_BeltAndBracesString(*pDataStream);

    if (nWhich == WW8_CT_DROPDOWN)
    {
        bool bAllOk = true;
        // SSTB (see Spec. 2.2.4)
        sal_uInt16 fExtend = 0;
        pDataStream->ReadUInt16( fExtend );
        sal_uInt16 nNoStrings = 0;

        // Isn't it that if fExtend isn't 0xFFFF then fExtend actually
        // doesn't exist and we really have just read nNoStrings ( or cData )?
        if (fExtend != 0xFFFF)
            bAllOk = false;
        pDataStream->ReadUInt16( nNoStrings );

        // I guess this should be zero ( and we should ensure that )
        sal_uInt16 cbExtra = 0;
        pDataStream->ReadUInt16( cbExtra );

        OSL_ENSURE(bAllOk, "Unknown formfield dropdown list structure");
        if (!bAllOk)    //Not as expected, don't risk it at all.
            nNoStrings = 0;
        const size_t nMinRecordSize = sizeof(sal_uInt16);
        const size_t nMaxRecords = pDataStream->remainingSize() / nMinRecordSize;
        if (nNoStrings > nMaxRecords)
        {
            SAL_WARN("sw.ww8", "Parsing error: " << nMaxRecords <<
                     " max possible entries, but " << nNoStrings << " claimed, truncating");
            nNoStrings = nMaxRecords;
        }
        maListEntries.reserve(nNoStrings);
        for (sal_uInt32 nI = 0; nI < nNoStrings; ++nI)
        {
            OUString sEntry =  read_uInt16_PascalString(*pDataStream);
            maListEntries.push_back(sEntry);
        }
    }
    mfDropdownIndex = iRes;

    nField = bits2;
    mfToolTip = nField & 0x01;
    mfNoMark = (nField & 0x02)>>1;
    mfUseSize = (nField & 0x04)>>2;
    mfNumbersOnly= (nField & 0x08)>>3;
    mfDateOnly = (nField & 0x10)>>4;
    mfUnused = (nField & 0xE0)>>5;
}

WW8FormulaListBox::WW8FormulaListBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(OUString(SL::aListBox), rR)
{
}

//Miserable hack to get a hardcoded guesstimate of the size of a list dropdown
//box's first entry to set as the lists default size
awt::Size SwWW8ImplReader::MiserableDropDownFormHack(const OUString &rString,
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
        { 0,                          nullptr }
    };

    vcl::Font aFont;
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
        rPropSet->getPropertySetInfo();

    uno::Any aTmp;
    for (const CtrlFontMapEntry* pMap = aMapTable; pMap->nWhichId; ++pMap)
    {
        bool bSet = true;
        const SfxPoolItem* pItem = GetFormatAttr( pMap->nWhichId );
        OSL_ENSURE(pItem, "Impossible");
        if (!pItem)
            continue;

        switch ( pMap->nWhichId )
        {
        case RES_CHRATR_COLOR:
            {
                OUString aNm;
                if (xPropSetInfo->hasPropertyByName(aNm = "TextColor"))
                {
                    aTmp <<= (sal_Int32)static_cast<const SvxColorItem*>(pItem)->GetValue().GetColor();
                    rPropSet->setPropertyValue(aNm, aTmp);
                }
            }
            aFont.SetColor(static_cast<const SvxColorItem*>(pItem)->GetValue());
            break;
        case RES_CHRATR_FONT:
            {
                const SvxFontItem *pFontItem = static_cast<const SvxFontItem *>(pItem);
                OUString aNm;
                if (xPropSetInfo->hasPropertyByName(aNm = "FontStyleName"))
                {
                    aTmp <<= OUString( pFontItem->GetStyleName());
                    rPropSet->setPropertyValue( aNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(aNm = "FontFamily"))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetFamily();
                    rPropSet->setPropertyValue( aNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(aNm = "FontCharset"))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetCharSet();
                    rPropSet->setPropertyValue( aNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(aNm = "FontPitch"))
                {
                    aTmp <<= (sal_Int16)pFontItem->GetPitch();
                    rPropSet->setPropertyValue( aNm, aTmp );
                }

                aTmp <<= OUString( pFontItem->GetFamilyName());
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
                            static_cast<const SvxFontHeightItem*>(pItem)->GetHeight() );
                aTmp <<= ((float)aSize.Height()) / 20.0;

                aFont.SetSize(OutputDevice::LogicToLogic(aSize, MAP_TWIP,
                    MAP_100TH_MM));
            }
            break;

        case RES_CHRATR_WEIGHT:
            aTmp <<= (float)VCLUnoHelper::ConvertFontWeight(
                                        static_cast<const SvxWeightItem*>(pItem)->GetWeight() );
            aFont.SetWeight( static_cast<const SvxWeightItem*>(pItem)->GetWeight() );
            break;

        case RES_CHRATR_UNDERLINE:
            aTmp <<= (sal_Int16)(static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle());
            aFont.SetUnderline(static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle());
            break;

        case RES_CHRATR_CROSSEDOUT:
            aTmp <<= (sal_Int16)( static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout() );
            aFont.SetStrikeout( static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout() );
            break;

        case RES_CHRATR_POSTURE:
            aTmp <<= (sal_Int16)( static_cast<const SvxPostureItem*>(pItem)->GetPosture() );
            aFont.SetItalic( static_cast<const SvxPostureItem*>(pItem)->GetPosture() );
            break;

        default:
            bSet = false;
            break;
        }

        if (bSet && xPropSetInfo->hasPropertyByName(OUString::createFromAscii(pMap->pPropNm)))
            rPropSet->setPropertyValue(OUString::createFromAscii(pMap->pPropNm), aTmp);
    }
    // now calculate the size of the control
    OutputDevice* pOut = Application::GetDefaultDevice();
    OSL_ENSURE(pOut, "Impossible");
    if (pOut)
    {
        pOut->Push( PushFlags::FONT | PushFlags::MAPMODE );
        pOut->SetMapMode( MapMode( MAP_100TH_MM ));
        pOut->SetFont( aFont );
        aRet.Width  = pOut->GetTextWidth(rString);
        aRet.Width += 500; //plus size of button, total hack territory
        aRet.Height = pOut->GetTextHeight();
        pOut->Pop();
    }
    return aRet;
}

bool WW8FormulaListBox::Import(const uno::Reference <
    lang::XMultiServiceFactory> &rServiceFactory,
    uno::Reference <form::XFormComponent> &rFComp,awt::Size &rSz )
{
    uno::Reference<uno::XInterface> xCreate = rServiceFactory->createInstance("com.sun.star.form.component.ComboBox");
    if( !xCreate.is() )
        return false;

    rFComp.set(xCreate, uno::UNO_QUERY);
    if( !rFComp.is() )
        return false;

    uno::Reference<beans::XPropertySet> xPropSet(xCreate, uno::UNO_QUERY);

    uno::Any aTmp;
    if (!msTitle.isEmpty())
        aTmp <<= msTitle;
    else
        aTmp <<= msName;
    xPropSet->setPropertyValue("Name", aTmp );

    if (!msToolTip.isEmpty())
    {
        aTmp <<= msToolTip;
        xPropSet->setPropertyValue("HelpText", aTmp );
    }

    bool bDropDown(true);
    xPropSet->setPropertyValue("Dropdown", css::uno::makeAny(bDropDown));

    if (!maListEntries.empty())
    {
        sal_uInt32 nLen = maListEntries.size();
        uno::Sequence< OUString > aListSource(nLen);
        for (sal_uInt32 nI = 0; nI < nLen; ++nI)
            aListSource[nI] = maListEntries[nI];
        aTmp <<= aListSource;
        xPropSet->setPropertyValue("StringItemList", aTmp );

        if (mfDropdownIndex < nLen)
        {
            aTmp <<= aListSource[mfDropdownIndex];
        }
        else
        {
            aTmp <<= aListSource[0];
        }

        xPropSet->setPropertyValue("DefaultText", aTmp );

        rSz = mrRdr.MiserableDropDownFormHack(maListEntries[0], xPropSet);
    }
    else
    {
        static const sal_Unicode aBlank[] =
        {
            0x2002,0x2002,0x2002,0x2002,0x2002
        };
        rSz = mrRdr.MiserableDropDownFormHack(OUString(aBlank, SAL_N_ELEMENTS(aBlank)), xPropSet);
    }

    return true;
}

WW8FormulaCheckBox::WW8FormulaCheckBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(OUString(SL::aCheckBox), rR)
{
}

static void lcl_AddToPropertyContainer
(uno::Reference<beans::XPropertySet> xPropSet,
 const OUString & rPropertyName, const OUString & rValue)
{
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if (xPropSetInfo.is() &&
        ! xPropSetInfo->hasPropertyByName(rPropertyName))
    {
        uno::Reference<beans::XPropertyContainer>
            xPropContainer(xPropSet, uno::UNO_QUERY);
        uno::Any aAny(OUString(""));
        xPropContainer->addProperty
            (rPropertyName,
             static_cast<sal_Int16>(beans::PropertyAttribute::BOUND |
                                    beans::PropertyAttribute::REMOVABLE),
             aAny);
    }

    uno::Any aAnyValue(rValue);
    xPropSet->setPropertyValue(rPropertyName, aAnyValue );
}

bool WW8FormulaCheckBox::Import(const uno::Reference <
    lang::XMultiServiceFactory> &rServiceFactory,
    uno::Reference <form::XFormComponent> &rFComp,awt::Size &rSz )
{
    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance("com.sun.star.form.component.CheckBox");
    if( !xCreate.is() )
        return false;

    rFComp.set( xCreate, uno::UNO_QUERY );
    if( !rFComp.is() )
        return false;

    uno::Reference< beans::XPropertySet > xPropSet( xCreate, uno::UNO_QUERY );

    rSz.Width = 16 * mhpsCheckBox;
    rSz.Height = 16 * mhpsCheckBox;

    uno::Any aTmp;
    if (!msTitle.isEmpty())
        aTmp <<= msTitle;
    else
        aTmp <<= msName;
    xPropSet->setPropertyValue("Name", aTmp );

    aTmp <<= (sal_Int16)mnChecked;
    xPropSet->setPropertyValue("DefaultState", aTmp);

    if (!msToolTip.isEmpty())
        lcl_AddToPropertyContainer(xPropSet, "HelpText", msToolTip);

    if (!msHelp.isEmpty())
        lcl_AddToPropertyContainer(xPropSet, "HelpF1Text", msHelp);

    return true;

}

WW8FormulaEditBox::WW8FormulaEditBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(OUString(SL::aTextField) ,rR)
{
}

bool SwMSConvertControls::InsertControl(
    const uno::Reference< form::XFormComponent > & rFComp,
    const awt::Size& rSize, uno::Reference< drawing::XShape > *pShape,
    bool bFloatingCtrl)
{
    const uno::Reference< container::XIndexContainer > &rComps = GetFormComps();
    uno::Any aTmp( &rFComp, cppu::UnoType<form::XFormComponent>::get());
    rComps->insertByIndex( rComps->getCount(), aTmp );

    const uno::Reference< lang::XMultiServiceFactory > &rServiceFactory =
        GetServiceFactory();
    if( !rServiceFactory.is() )
        return false;

    uno::Reference< uno::XInterface > xCreate = rServiceFactory->createInstance(
        "com.sun.star.drawing.ControlShape");
    if( !xCreate.is() )
        return false;

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
    xShapePropSet->setPropertyValue("AnchorType", aTmp );

    nTemp= text::VertOrientation::TOP;
    aTmp <<= nTemp;
    xShapePropSet->setPropertyValue("VertOrient", aTmp );

    uno::Reference< text::XText >  xDummyTextRef;
    uno::Reference< text::XTextRange >  xTextRg =
        new SwXTextRange( *pPaM, xDummyTextRef );

    aTmp.setValue(&xTextRg, cppu::UnoType<text::XTextRange>::get());
    xShapePropSet->setPropertyValue("TextRange", aTmp );

    // Das Control-Model am Control-Shape setzen
    uno::Reference< drawing::XControlShape >  xControlShape( xShape,
        uno::UNO_QUERY );
    uno::Reference< awt::XControlModel >  xControlModel( rFComp,
        uno::UNO_QUERY );
    xControlShape->setControl( xControlModel );

    if (pShape)
        *pShape = xShape;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
