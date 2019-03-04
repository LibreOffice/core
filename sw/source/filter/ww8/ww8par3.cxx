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

#include <memory>
#include <svl/itemiter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <sal/log.hxx>

#include <vcl/unohelp.hxx>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>
#include <hintids.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <o3tl/safeint.hxx>
#include <unotextrange.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <docsh.hxx>
#include <numrule.hxx>
#include <paratr.hxx>
#include <charatr.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <expfld.hxx>
#include <fmtfld.hxx>
#include <flddropdown.hxx>
#include "sprmids.hxx"
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
    return eF_ResT::TEXT;
}

eF_ResT SwWW8ImplReader::Read_F_FormTextBox( WW8FieldDesc* pF, OUString& rStr )
{
    WW8FormulaEditBox aFormula(*this);

    if (pF->nLCode && rStr[pF->nLCode-1]==0x01) {
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
            static_cast<SwInputFieldType*>(m_rDoc.getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::Input )),
            aFormula.msDefault,
            aFormula.msTitle,
            INP_TXT,
            0 );
        aField.SetHelp(aFormula.msHelp);
        aField.SetToolTip(aFormula.msToolTip);

        m_rDoc.getIDocumentContentOperations().InsertPoolItem(*m_pPaM, SwFormatField(aField));
        return eF_ResT::OK;
    }
    else
    {
        WW8PLCFx_Book* pB = m_xPlcxMan->GetBook();
        OUString aBookmarkName;
        if (pB!=nullptr) {
            WW8_CP currentCP=pF->nSCode;
            WW8_CP currentLen=pF->nLen;

            WW8_CP nEnd;
            if (o3tl::checked_add(currentCP, currentLen-1, nEnd)) {
                SAL_WARN("sw.ww8", "broken offset, ignoring");
            }
            else
            {
                sal_uInt16 bkmFindIdx;
                OUString aBookmarkFind=pB->GetBookmark(currentCP-1, nEnd, bkmFindIdx);

                if (!aBookmarkFind.isEmpty()) {
                    pB->SetStatus(bkmFindIdx, BOOK_FIELD); // mark bookmark as consumed, such that tl'll not get inserted as a "normal" bookmark again
                    if (!aBookmarkFind.isEmpty()) {
                        aBookmarkName=aBookmarkFind;
                    }
                }
            }
        }

        if (pB!=nullptr && aBookmarkName.isEmpty()) {
            aBookmarkName=pB->GetUniqueBookmarkName(aFormula.msTitle);
        }

        if (!aBookmarkName.isEmpty()) {
            m_aFieldStack.back().SetBookmarkName(aBookmarkName);
            m_aFieldStack.back().SetBookmarkType(ODF_FORMTEXT);
            if ( aFormula.msToolTip.getLength() < 139 )
                m_aFieldStack.back().getParameters()["Description"] <<= aFormula.msToolTip;
            m_aFieldStack.back().getParameters()["Name"] <<= aFormula.msTitle;
            if (aFormula.mnMaxLen && aFormula.mnMaxLen < 32768 )
                m_aFieldStack.back().getParameters()["MaxLength"] <<= aFormula.mnMaxLen;

            if ( aFormula.mfType == 1 )
                m_aFieldStack.back().getParameters()["Type"] <<= OUString("number");
            else if ( aFormula.mfType == 2 )
                m_aFieldStack.back().getParameters()["Type"] <<= OUString("date");
            else if ( aFormula.mfType == 3 )
                m_aFieldStack.back().getParameters()["Type"] <<= OUString("currentTime");
            else if ( aFormula.mfType == 4 )
                m_aFieldStack.back().getParameters()["Type"] <<= OUString("currentDate");
            else if ( aFormula.mfType == 5 )
                m_aFieldStack.back().getParameters()["Type"] <<= OUString("calculated");
        }
        return eF_ResT::TEXT;
    }
}

eF_ResT SwWW8ImplReader::Read_F_FormCheckBox( WW8FieldDesc* pF, OUString& rStr )
{
    WW8FormulaCheckBox aFormula(*this);

    if (!m_xFormImpl)
        m_xFormImpl.reset(new SwMSConvertControls(m_pDocShell, m_pPaM));

    if (rStr[pF->nLCode-1]==0x01)
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_CHECKBOX);
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    const bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields)
    {
        m_xFormImpl->InsertFormula(aFormula);
        return eF_ResT::OK;
    }

    OUString aBookmarkName;
    WW8PLCFx_Book* pB = m_xPlcxMan->GetBook();
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
        IFieldmark* pFieldmark = pMarksAccess->makeNoTextFieldBookmark(
                *m_pPaM, aBookmarkName, ODF_FORMCHECKBOX );
        OSL_ENSURE(pFieldmark!=nullptr, "hmmm; why was the bookmark not created?");
        if (pFieldmark!=nullptr) {
            IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
            ICheckboxFieldmark* pCheckboxFm = dynamic_cast<ICheckboxFieldmark*>(pFieldmark);
            (*pParameters)[ODF_FORMCHECKBOX_NAME] <<= aFormula.msTitle;
            (*pParameters)[ODF_FORMCHECKBOX_HELPTEXT] <<= aFormula.msToolTip;

            if(pCheckboxFm)
                pCheckboxFm->SetChecked(aFormula.mnChecked != 0);
            // set field data here...
        }
    }
    return eF_ResT::OK;
}

eF_ResT SwWW8ImplReader::Read_F_FormListBox( WW8FieldDesc* pF, OUString& rStr)
{
    WW8FormulaListBox aFormula(*this);

    if (pF->nLCode > 0 && rStr.getLength() >= pF->nLCode && rStr[pF->nLCode-1] == 0x01)
        ImportFormulaControl(aFormula,pF->nSCode+pF->nLCode-1, WW8_CT_DROPDOWN);

    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    bool bUseEnhFields = rOpt.IsUseEnhancedFields();

    if (!bUseEnhFields)
    {
        SwDropDownField aField(static_cast<SwDropDownFieldType*>(m_rDoc.getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Dropdown)));

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
        return eF_ResT::OK;
    }
    else
    {
        // TODO: review me
        OUString aBookmarkName;
        WW8PLCFx_Book* pB = m_xPlcxMan->GetBook();
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
            IFieldmark *pFieldmark =
                    pMarksAccess->makeNoTextFieldBookmark( *m_pPaM, aBookmarkName, ODF_FORMDROPDOWN );
            OSL_ENSURE(pFieldmark!=nullptr, "hmmm; why was the bookmark not created?");
            if ( pFieldmark != nullptr )
            {
                uno::Sequence< OUString > vListEntries(aFormula.maListEntries.size());
                std::copy(aFormula.maListEntries.begin(), aFormula.maListEntries.end(), vListEntries.begin());
                (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] <<= vListEntries;
                sal_Int32 nIndex = aFormula.mfDropdownIndex  < aFormula.maListEntries.size() ? aFormula.mfDropdownIndex : 0;
                (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_RESULT] <<= nIndex;
                // set field data here...
            }
        }

        return eF_ResT::OK;
    }
}

eF_ResT SwWW8ImplReader::Read_F_HTMLControl(WW8FieldDesc*, OUString&)
{
    if( m_bObj && m_nPicLocFc )
        m_nObjLocFc = m_nPicLocFc;
    m_bEmbeddObj = true;
    return eF_ResT::TEXT;
}

// Helper declarations

// Style Id's for each level
typedef sal_uInt16 WW8aIdSty[WW8ListManager::nMaxLevel];
// Character Style Pointer
typedef SwCharFormat* WW8aCFormat[WW8ListManager::nMaxLevel];

struct WW8LST   // only THOSE entries, WE need!
{
    WW8aIdSty aIdSty;     // Style Id's for each level,
                            //   nIStDNil if no style linked
    sal_uInt32 nIdLst;     // Unique List ID
    sal_uInt32 nTplC;      // Unique template code - What is this?
    bool bSimpleList:1;    // Flag: List only has ONE level
    bool bRestartHdn:1;    // WW6-Compatibility-Flag:
                                                        //   true if the list should start numbering over
};                                                      //   at the beginning of each section

const sal_uInt32 cbLSTF=28;

struct WW8LFO   // only THOSE entries, WE need!
{
    SwNumRule*      pNumRule;   // Parent NumRule
    sal_uInt32      nIdLst;     // Unique List ID
    sal_uInt8       nLfoLvl;    // count of levels whose format is overridden
    bool bSimpleList;
};

struct WW8LVL   // only THE entries, WE need!
{
    sal_Int32 nStartAt;       // start at value for this value
    sal_Int32 nV6DxaSpace;// Ver6-Compatible: min Space between Num and text::Paragraph
    sal_Int32 nV6Indent;  // Ver6-Compatible: Width of prefix text;
                        // Use definition of first line indent if appropriate!
    // Paragraph attributes from GrpprlPapx
    sal_uInt16  nDxaLeft;               // left indent
    short   nDxaLeft1;          // first line indent

    sal_uInt8   nNFC;               // number format code
    // Offset of fieldcodes in Num-X-String
    sal_uInt8   aOfsNumsXCH[WW8ListManager::nMaxLevel];
    sal_uInt8   nLenGrpprlChpx; // length, in bytes, of the LVL's grpprlChpx
    sal_uInt8   nLenGrpprlPapx; // length, in bytes, of the LVL's grpprlPapx
    sal_uInt8   nAlign: 2;  // alignment (left, right, centered) of the number
    sal_uInt8 bLegal:    1;  // doesn't matter
    sal_uInt8 bNoRest:1; // doesn't matter
    sal_uInt8 bV6Prev:1; // Ver6-Compatible: number will include previous levels
    sal_uInt8 bV6PrSp:1; // Ver6-Compatible: doesn't matter
    sal_uInt8 bV6:       1;  // if true, pay attention to the V6-Compatible Entries!
    sal_uInt8   bDummy: 1;  // (fills the byte)

};

struct WW8LFOLVL
{
    sal_Int32 nStartAt;          // start-at value if bFormat==false and bStartAt == true
                                            // (if bFormat==true, the start-at is stored in the LVL)
    sal_uInt8 nLevel;               // the level to be overridden
    // this byte has not been packed into the following byte on _purpose_ !!
    // (see comment of struct WW8LFOInfo)

    bool bStartAt :1;       // true if the start-at value is overridden
    bool bFormat :1;        // true if the formatting is overridden

    WW8LFOLVL() :
        nStartAt(1), nLevel(0), bStartAt(true), bFormat(false) {}
};

// Data to be saved in ListInfo

struct WW8LSTInfo   // sorted by nIdLst (in WW8 used list-Id)
{
    std::vector<ww::bytes> maParaSprms;
    WW8aIdSty   aIdSty;          // Style Id's for each level
    WW8aCFormat    aCharFormat;        // Character Style Pointer

    SwNumRule*  pNumRule;        // Pointer to list-template in Writer
    sal_uInt32      nIdLst;          // WW8Id of this list
    bool bSimpleList:1;// Flag, if this NumRule only uses one Level
    bool bUsedInDoc :1;// Flag, if this NumRule is used in the Doc,
                                                     //   or is supposed to be deleted on Reader-End

    WW8LSTInfo(SwNumRule* pNumRule_, const WW8LST& aLST)
        : pNumRule(pNumRule_), nIdLst(aLST.nIdLst),
        bSimpleList(aLST.bSimpleList), bUsedInDoc(false)
    {
        memcpy( aIdSty, aLST.aIdSty, sizeof( aIdSty   ));
        memset(&aCharFormat, 0,  sizeof( aCharFormat ));
    }

};

// Data to be saved in ListenFormatOverrideInfos

struct WW8LFOInfo   // unordered, means ordered like in WW8 Stream
{
    std::vector<ww::bytes> maParaSprms;
    std::vector<WW8LFOLVL> maOverrides;
    SwNumRule* pNumRule;         // Pointer to list template in Writer
                                                     // either List in LSTInfos or own List
                                                     // (in Ctor use the list from LSTInfos first)

    sal_uInt32  nIdLst;          // WW8-Id of the relevant list
    sal_uInt8   nLfoLvl;             // count of levels whose format is overridden
    // yes we could include nLfoLvl (via :4) into the following byte,
    // but it probably would be a source of error once MS increases their Listformat
    // to more than 15 levels

    bool bOverride  :1;// Flag if NumRule is not included in maLSTInfos,
                        //   but was created for m_LFOInfos
    bool bUsedInDoc :1;// Flag if NumRule is used in Doc,
                                                     //  or should be deleted on Reader-End
    bool bLSTbUIDSet    :1;// Flag, if bUsedInDoc is set in maLSTInfos

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

// Helper methods

// find Sprm-Parameter-Data, if Sprm is included in Grpprl
SprmResult WW8ListManager::GrpprlHasSprm(sal_uInt16 nId, sal_uInt8& rSprms,
    sal_uInt8 nLen)
{
    return maSprmParser.findSprmData(nId, &rSprms, nLen);
}

class ListWithId
{
private:
    sal_uInt32 mnIdLst;
public:
    explicit ListWithId(sal_uInt32 nIdLst) : mnIdLst(nIdLst) {}
    bool operator() (const std::unique_ptr<WW8LSTInfo>& pEntry) const
        { return (pEntry->nIdLst == mnIdLst); }
};

// Access via List-Id of LST Entry
WW8LSTInfo* WW8ListManager::GetLSTByListId( sal_uInt32 nIdLst ) const
{
    auto aResult =
        std::find_if(maLSTInfos.begin(),maLSTInfos.end(),ListWithId(nIdLst));
    if (aResult == maLSTInfos.end())
        return nullptr;
    return aResult->get();
}

static void lcl_CopyGreaterEight(OUString &rDest, OUString const &rSrc,
    sal_Int32 nStart, sal_Int32 nLen = SAL_MAX_INT32)
{
    const sal_Int32 nMaxLen = std::min(rSrc.getLength(), nLen);
    for( sal_Int32 nI = nStart; nI < nMaxLen; ++nI)
    {
        sal_Unicode nChar = rSrc[nI];
        if (nChar > WW8ListManager::nMaxLevel)
            rDest += OUStringLiteral1(nChar);
    }
}

static OUString sanitizeString(const OUString& rString)
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

bool WW8ListManager::ReadLVL(SwNumFormat& rNumFormat, std::unique_ptr<SfxItemSet>& rpItemSet,
    sal_uInt16 nLevelStyle, bool bSetStartNo,
    std::deque<bool> &rNotReallyThere, sal_uInt16 nLevel,
    ww::bytes &rParaSprms)
{
    sal_uInt8       aBits1(0);
    sal_uInt16      nStartNo(0);        // Start-No. for Writer
    SvxNumType      nType(SVX_NUM_ARABIC);
    SvxAdjust       eAdj;               // Alignment (Left/right/centered)
    sal_Unicode     cBullet(0x2190);    // default safe bullet

    sal_Unicode     cGrfBulletCP(USHRT_MAX);

    OUString        sPrefix;
    OUString        sPostfix;
    WW8LVL          aLVL;

    // 1. read LVLF

    memset(&aLVL, 0, sizeof( aLVL ));
    rSt.ReadInt32( aLVL.nStartAt );
    rSt.ReadUChar( aLVL.nNFC );
    rSt.ReadUChar( aBits1 );
    if( ERRCODE_NONE != rSt.GetError() ) return false;
    aLVL.nAlign = (aBits1 & 0x03);
    if( aBits1 & 0x10 ) aLVL.bV6Prev    = sal_uInt8(true);
    if( aBits1 & 0x20 ) aLVL.bV6PrSp    = sal_uInt8(true);
    if( aBits1 & 0x40 ) aLVL.bV6        = sal_uInt8(true);
    bool bLVLOkB = true;
    for(sal_uInt8 nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
    {
        rSt.ReadUChar( aLVL.aOfsNumsXCH[ nLevelB ] );
        if( ERRCODE_NONE != rSt.GetError() )
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
    if( ERRCODE_NONE != rSt.GetError()) return false;

    // 2. read PAPx if needed and search for indent values

    short nTabPos = 0; // #i86652# - read tab setting
    if( aLVL.nLenGrpprlPapx )
    {
        sal_uInt8 aGrpprlPapx[ 255 ];
        if (aLVL.nLenGrpprlPapx != rSt.ReadBytes(&aGrpprlPapx, aLVL.nLenGrpprlPapx))
            return false;
        // "sprmPDxaLeft"  pap.dxaLeft;dxa;word;
        SprmResult aSprm;

        aSprm = GrpprlHasSprm(0x840F,aGrpprlPapx[0],aLVL.nLenGrpprlPapx);
        if (!aSprm.pSprm)
            aSprm = GrpprlHasSprm(0x845E,aGrpprlPapx[0],aLVL.nLenGrpprlPapx);

        if (aSprm.pSprm && aSprm.nRemainingData >= 2)
        {
            const sal_uInt8 *pBegin = aSprm.pSprm - 2;
            for(int i=0;i<4;++i)
                rParaSprms.push_back(*pBegin++);
            short nDxaLeft = SVBT16ToUInt16(aSprm.pSprm);
            aLVL.nDxaLeft = (0 < nDxaLeft) ? static_cast<sal_uInt16>(nDxaLeft)
                            : static_cast<sal_uInt16>(-nDxaLeft);
        }

        // "sprmPDxaLeft1" pap.dxaLeft1;dxa;word;
        aSprm = GrpprlHasSprm(0x8411,aGrpprlPapx[0],aLVL.nLenGrpprlPapx);
        if (!aSprm.pSprm)
            aSprm = GrpprlHasSprm(0x8460,aGrpprlPapx[0],aLVL.nLenGrpprlPapx);

        if (aSprm.pSprm && aSprm.nRemainingData >= 2)
        {
            const sal_uInt8 *pBegin = aSprm.pSprm - 2;
            for(int i=0;i<4;++i)
                rParaSprms.push_back(*pBegin++);
            aLVL.nDxaLeft1 = SVBT16ToUInt16(aSprm.pSprm);
        }

        // #i86652# - read tab setting
        aSprm = GrpprlHasSprm(0xC615,aGrpprlPapx[0],aLVL.nLenGrpprlPapx);
        const sal_uInt8* pSprm = aSprm.pSprm;
        if (pSprm && aSprm.nRemainingData >= 5)
        {
            bool bDone = false;
            if (*(pSprm-1) == 5)
            {
                if (*pSprm++ == 0) //nDel
                {
                    if (*pSprm++ == 1) //nIns
                    {
                        nTabPos = SVBT16ToUInt16(pSprm);
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
                                    ? static_cast<sal_uInt16>(nTabPos)
                                    : static_cast<sal_uInt16>(-nTabPos);

                    aLVL.nDxaLeft1 = nDesired - aLVL.nDxaLeft;
                }
            }
        }
    }

    // 3. read CHPx if needed

    sal_uInt16 nWitchPicIsBullet = USHRT_MAX;
    bool bIsPicBullet = false;

    if( aLVL.nLenGrpprlChpx )
    {
        sal_uInt8 aGrpprlChpx[ 255 ];
        memset(&aGrpprlChpx, 0, sizeof( aGrpprlChpx ));
        if (aLVL.nLenGrpprlChpx != rSt.ReadBytes(&aGrpprlChpx, aLVL.nLenGrpprlChpx))
            return false;

        //For i120928,parse the graphic info of bullets
        SprmResult aSprmWhichPis = GrpprlHasSprm(NS_sprm::sprmCPbiIBullet, aGrpprlChpx[0],aLVL.nLenGrpprlChpx);
        SprmResult aSprmIsPicBullet = GrpprlHasSprm(NS_sprm::sprmCPbiGrf, aGrpprlChpx[0],aLVL.nLenGrpprlChpx);
        if (aSprmWhichPis.pSprm && aSprmWhichPis.nRemainingData >= 1)
        {
            nWitchPicIsBullet = *aSprmWhichPis.pSprm;
        }
        if (aSprmIsPicBullet.pSprm && aSprmIsPicBullet.nRemainingData >= 1)
        {
            bIsPicBullet = (*aSprmIsPicBullet.pSprm) & 0x0001;
        }

        // create new Itemset for character attributes
        rpItemSet.reset(new SfxItemSet( rDoc.GetAttrPool(), svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END - 1>{}));

        // Set Reader-ItemSet-Pointer to the newly created set
        rReader.SetCurrentItemSet(std::move(rpItemSet));
        // Set Reader-Style to Style of this Level
        sal_uInt16 nOldColl = rReader.GetCurrentColl();
        sal_uInt16 nNewColl = nLevelStyle;
        if (ww::stiNil == nNewColl)
            nNewColl = 0;
        rReader.SetNCurrentColl( nNewColl );

        // The Read_xy() methods in WW8PAR6.cxx are calling their respective
        // NewAttr() or GetFormatAttr() which can determine, by using the assigned
        // Reader-ItemSet-Pointer, whether this specific ItemSet is relevant
        // and not a Stack or Style!
        sal_uInt16 nOldFlags1 = rReader.GetToggleAttrFlags();
        sal_uInt16 nOldFlags2 = rReader.GetToggleBiDiAttrFlags();

        WW8SprmIter aSprmIter(&aGrpprlChpx[0], aLVL.nLenGrpprlChpx,
            maSprmParser);
        while (const sal_uInt8* pSprm = aSprmIter.GetSprms())
        {
            rReader.ImportSprm(pSprm, aSprmIter.GetRemLen(), aSprmIter.GetCurrentId());
            aSprmIter.advance();
        }

        // Reset Reader-ItemSet-Pointer and Reader-Style
        rpItemSet = rReader.SetCurrentItemSet(nullptr);
        rReader.SetNCurrentColl( nOldColl );
        rReader.SetToggleAttrFlags(nOldFlags1);
        rReader.SetToggleBiDiAttrFlags(nOldFlags2);
    }

    // 4. Read numbering String. Results in prefix and postfix

    OUString sNumString(sanitizeString(read_uInt16_PascalString(rSt)));

    // 5. convert read values into Writer syntax

    if( 0 <= aLVL.nStartAt )
        nStartNo = static_cast<sal_uInt16>(aLVL.nStartAt);

    switch( aLVL.nNFC )
    {
        case 0:
            nType = SVX_NUM_ARABIC;
            break;
        case 1:
            nType = SVX_NUM_ROMAN_UPPER;
            break;
        case 2:
            nType = SVX_NUM_ROMAN_LOWER;
            break;
        case 3:
            nType = SVX_NUM_CHARS_UPPER_LETTER_N;
            break;
        case 4:
            nType = SVX_NUM_CHARS_LOWER_LETTER_N;
            break;
        case 5:
            // actually: ORDINAL
            nType = SVX_NUM_ARABIC;
            break;
        case 23:
            nType = SVX_NUM_CHAR_SPECIAL;
            //For i120928,type info
            if (bIsPicBullet)
            {
                nType = SVX_NUM_BITMAP;
            }

            break;
        case 255:
            nType = SVX_NUM_NUMBER_NONE;
            break;
        case 14:
        case 19:nType = SVX_NUM_FULL_WIDTH_ARABIC; break;
        case 30:nType = SVX_NUM_TIAN_GAN_ZH; break;
        case 31:nType = SVX_NUM_DI_ZI_ZH; break;
        case 35:
        case 36:
        case 37:
        case 11:
        case 39:nType = SVX_NUM_NUMBER_LOWER_ZH; break;
        case 34:nType = SVX_NUM_NUMBER_UPPER_ZH_TW; break;
        case 38:nType = SVX_NUM_NUMBER_UPPER_ZH; break;
        case 10:nType = SVX_NUM_NUMBER_TRADITIONAL_JA; break;
        case 20:nType = SVX_NUM_AIU_FULLWIDTH_JA; break;
        case 12:nType = SVX_NUM_AIU_HALFWIDTH_JA; break;
        case 21:nType = SVX_NUM_IROHA_FULLWIDTH_JA; break;
        case 13:nType = SVX_NUM_IROHA_HALFWIDTH_JA; break;
        case 24:nType = SVX_NUM_HANGUL_SYLLABLE_KO; break;
        case 25:nType = SVX_NUM_HANGUL_JAMO_KO; break;
        case 41:nType = SVX_NUM_NUMBER_HANGUL_KO; break;
        //case 42:
        //case 43:
        case 44:nType = SVX_NUM_NUMBER_UPPER_KO; break;
        default:
                nType= SVX_NUM_ARABIC; break;
    }

    //If a number level is not going to be used, then record this fact
    if (SVX_NUM_NUMBER_NONE == nType)
        rNotReallyThere[nLevel] = true;

    /*
     If a number level was not used (i.e. is in NotReallyThere), and that
     number level appears at one of the positions in the display string of the
     list, then it effectively is not there at all. So remove that level entry
     from a copy of the aOfsNumsXCH.
    */
    std::vector<sal_uInt8> aOfsNumsXCH;
    aOfsNumsXCH.reserve(nMaxLevel);

    for(sal_uInt8 nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
        aOfsNumsXCH.push_back(aLVL.aOfsNumsXCH[nLevelB]);

    // nLevelB is an index in the aOfsNumsXCH array.
    for(sal_uInt16 nLevelB = 0; nLevelB <= nLevel; ++nLevelB)
    {
        // nPos is a one-based character offset to a level placeholder in
        // sNumString.
        sal_uInt8 nPos = aOfsNumsXCH[nLevelB];
        if (nPos && nPos < sNumString.getLength())
        {
            // nPosValue is the actual numbering level.
            sal_Unicode nPosValue = sNumString[nPos-1];
            if (nPosValue < nMaxLevel)
            {
                if (rNotReallyThere[nPosValue])
                    aOfsNumsXCH[nLevelB] = 0;
            }
        }
    }
    auto aIter = std::remove(aOfsNumsXCH.begin(), aOfsNumsXCH.end(), 0);
    auto aEnd = aOfsNumsXCH.end();
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

    sal_uInt8 nUpperLevel = 0;  // current displaydepth for Writer
    for(sal_uInt8 nLevelB = 0; nLevelB < nMaxLevel; ++nLevelB)
    {
        if (!nUpperLevel && !aOfsNumsXCH[nLevelB])
            nUpperLevel = nLevelB;
    }

    // If the terminating char was not NULL, all indices of the list are
    // filled, so the levels have to be displayed.
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
        assert(nUpperLevel > 0);
        sal_uInt8 nOneBasedNextNoIndex = aOfsNumsXCH[nUpperLevel-1];
        const sal_Int32 nNextNoIndex =
            nOneBasedNextNoIndex > 0 ? nOneBasedNextNoIndex : SAL_MAX_INT32;
        if (sNumString.getLength() > nNextNoIndex)
            lcl_CopyGreaterEight(sPostfix, sNumString, nNextNoIndex);

        /*
         We use lcl_CopyGreaterEight because once if we have removed unused
         number indexes from the aOfsNumsXCH then placeholders remain in
         sNumString which must not be copied into the final numbering strings
        */
    }

    switch( aLVL.nAlign )
    {
        case 0:
            eAdj = SvxAdjust::Left;
            break;
        case 1:
            eAdj = SvxAdjust::Center;
            break;
        case 2:
            eAdj = SvxAdjust::Right;
            break;
        case 3:
            // Writer here cannot do block justification
            eAdj = SvxAdjust::Left;
            break;
         default:
            // undefined value
            OSL_ENSURE( false, "Value of aLVL.nAlign is not supported" );
            // take default
            eAdj = SvxAdjust::Left;
            break;
    }

    // 6. Configure NumFormat
    if( bSetStartNo )
        rNumFormat.SetStart( nStartNo );
    rNumFormat.SetNumberingType( nType );
    rNumFormat.SetNumAdjust( eAdj );

    if( style::NumberingType::CHAR_SPECIAL == nType )
    {
        // first character of the Prefix-Text is the Bullet
        rNumFormat.SetBulletChar(cBullet);
        // Don't forget: further below, after building styles
        // Call SetBulletFont() !!!
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
        if (eAdj == SvxAdjust::Right)
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
    WW8aISet const & rListItemSet, WW8aCFormat& rCharFormat, bool& bNewCharFormatCreated,
    const OUString& sPrefix )
{
    bNewCharFormatCreated = false;
    sal_uInt8 nIdenticalItemSetLevel;
    const SfxPoolItem* pItem;

    SwNumFormat aNumFormat  = rNumRule.Get( nLevel );

    SfxItemSet* pThisLevelItemSet = rListItemSet[nLevel].get();

    if( pThisLevelItemSet && pThisLevelItemSet->Count())
    {
        nIdenticalItemSetLevel = nMaxLevel;
        SfxItemIter aIter( *pThisLevelItemSet );
        for (sal_uInt8 nLowerLevel = 0; nLowerLevel < nLevel; ++nLowerLevel)
        {
            SfxItemSet* pLowerLevelItemSet = rListItemSet[nLowerLevel].get();
            if(     pLowerLevelItemSet
                && (pLowerLevelItemSet->Count() == pThisLevelItemSet->Count()) )
            {
                nIdenticalItemSetLevel = nLowerLevel;
                sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                while (true)
                {
                    if(  // search for appropriate pItem in pLowerLevelItemSet
                         (SfxItemState::SET != pLowerLevelItemSet->GetItemState(
                                            nWhich, false, &pItem ) )
                        || // use virtual "!=" Operator
                         (*pItem != *aIter.GetCurItem() ) )
                    // if no Item with equal nWhich was found or Item value was not equal
                    // store inequality and break!
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
            // Define Style
            const OUString aName( (!sPrefix.isEmpty() ? sPrefix : rNumRule.GetName())
                                  + "z" + OUString::number( nLevel ) );

            // remove const by casting
            pFormat = rDoc.MakeCharFormat(aName, rDoc.GetDfltCharFormat());
            bNewCharFormatCreated = true;
            // Set Attributes
            pFormat->SetFormatAttr( *pThisLevelItemSet );
        }
        else
        {
            // append Style
            pFormat = rCharFormat[ nIdenticalItemSetLevel ];
        }

        // store
        rCharFormat[ nLevel ] = pFormat;

        // Append Style to NumFormat

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

    // if necessary: Append Bullet Font to NumFormat

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
            aFont.SetFamily(        rFontItem.GetFamily()     );
            aFont.SetFamilyName(    rFontItem.GetFamilyName() );
            aFont.SetStyleName(     rFontItem.GetStyleName()  );
            aFont.SetPitch(         rFontItem.GetPitch()      );
            aFont.SetCharSet(       rFontItem.GetCharSet()    );
        }
        aNumFormat.SetBulletFont( &aFont );
    }

    // Set NumFormat in NumRule

    rNumRule.Set(nLevel, aNumFormat);
}

SwNumRule* WW8ListManager::CreateNextRule(bool bSimple)
{
    // Used to build the Style Name
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

// public methods

WW8ListManager::WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_)
    : maSprmParser(rReader_.GetFib()), rReader(rReader_)
    , rDoc(rReader.GetDoc())
    , rFib(rReader.GetFib()), rSt(rSt_)
    , nUniqueList(1)
    , nLastLFOPosition(USHRT_MAX)
{

    // LST and LFO only since WW8
    if(    ( 8 > rFib.m_nVersion )
            || ( rFib.m_fcPlcfLst == rFib.m_fcPlfLfo )
            || ( rFib.m_lcbPlcfLst < 2 )
            || ( rFib.m_lcbPlfLfo < 2) ) return; // no public lists

    // create Arrays
    bool bLVLOk = true;

    long nOriginalPos = rSt.Tell();

    // 1. read PLCF LST and create list templates in Writer

    bool bOk = checkSeek(rSt, rFib.m_fcPlcfLst);

    if (!bOk)
        return;

    sal_uInt32 nRemainingPlcfLst = rFib.m_lcbPlcfLst;

    sal_uInt16 nListCount(0);
    rSt.ReadUInt16( nListCount );
    nRemainingPlcfLst -= 2;
    bOk = nListCount > 0;

    if (!bOk)
        return;

    // 1.1 read all LST
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

        // 1.1.1 read Data

        rSt.ReadUInt32( aLST.nIdLst );
        rSt.ReadUInt32( aLST.nTplC );
        for (sal_uInt16 & nLevel : aLST.aIdSty)
            rSt.ReadUInt16( nLevel );

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
        maLSTInfos.emplace_back(pLSTInfo);

        nRemainingPlcfLst -= cbLSTF;
    }

    // 1.2 read all LVL of all aLST

    sal_uInt16 nLSTInfos = static_cast< sal_uInt16 >(maLSTInfos.size());
    for (sal_uInt16 nList = 0; nList < nLSTInfos; ++nList)
    {
        WW8aISet aItemSet;        // Character attributes from GrpprlChpx

        WW8LSTInfo* pListInfo = maLSTInfos[nList].get();
        if( !pListInfo || !pListInfo->pNumRule ) break;
        SwNumRule& rMyNumRule = *pListInfo->pNumRule;

        // 1.2.1 read specific LVL(s) for this aLST

        sal_uInt16 nLvlCount = static_cast< sal_uInt16 >(pListInfo->bSimpleList ? nMinLevel : nMaxLevel);
        std::deque<bool> aNotReallyThere;
        aNotReallyThere.resize(nMaxLevel);
        pListInfo->maParaSprms.resize(nMaxLevel);
        for (sal_uInt16 nLevel = 0; nLevel < nLvlCount; ++nLevel)
        {
            SwNumFormat aNumFormat( rMyNumRule.Get( nLevel ) );
            // read LVLF
            bLVLOk = ReadLVL( aNumFormat, aItemSet[nLevel],
                pListInfo->aIdSty[nLevel], true, aNotReallyThere, nLevel,
                pListInfo->maParaSprms[nLevel]);
            if( !bLVLOk )
                break;
            // and set in rMyNumRule
            rMyNumRule.Set( nLevel, aNumFormat );
        }
        if( !bLVLOk )
            break;

        // 1.2.2 compare ItemPools and CHPx Settings of different Levels
        //       and create Style(s) if necessary

        for (sal_uInt16 nLevel = 0; nLevel < nLvlCount; ++nLevel)
        {
            bool bDummy;
            AdjustLVL( nLevel, rMyNumRule, aItemSet,
                                           pListInfo->aCharFormat, bDummy );
        }
    }

    // 2. read and save PLF LFO

    bOk = checkSeek(rSt, rFib.m_fcPlfLfo);

    if (!bOk)
        return;

    sal_Int32 nLfoCount(0);
    rSt.ReadInt32( nLfoCount );
    bOk = nLfoCount > 0;

    if (!bOk)
        return;

    // 2.1 read all LFO

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
        // as many Overrides as there are
        if ((nMaxLevel < aLFO.nLfoLvl) || rSt.GetError())
            break;

        // get the Parent NumRule of the current List
        WW8LSTInfo* pParentListInfo = GetLSTByListId(aLFO.nIdLst);
        if (pParentListInfo)
        {
            // Save the NumRule in this first step
            aLFO.pNumRule = pParentListInfo->pNumRule;

            // are there multiple Levels in the List?
            aLFO.bSimpleList = pParentListInfo->bSimpleList;
        }
        // store in Array
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

        // 2.2 read specific LFOLVL for all LFO

        size_t nLFOInfos = m_LFOInfos.size();
        for (size_t nLfo = 0; nLfo < nLFOInfos; ++nLfo)
        {
            WW8LFOInfo& rLFOInfo = *m_LFOInfos[nLfo];
            // Do LFOLVL exist?
            if( rLFOInfo.bOverride )
            {
                WW8LSTInfo* pParentListInfo = GetLSTByListId(rLFOInfo.nIdLst);
                if (!pParentListInfo)
                    break;

                // 2.2.1 create new NumRule for this List

                SwNumRule* pParentNumRule = rLFOInfo.pNumRule;
                OSL_ENSURE(pParentNumRule, "ww: Impossible lists, please report");
                if( !pParentNumRule )
                    break;
                // create name-prefix for NumRule-Name
                // and (if necessary) for Style-Name
                const OUString sPrefix("WW8NumSt" + OUString::number( nLfo + 1 ));
                // Now assign pNumRule its actual value!!!
                // (it contained the parent NumRule up to this point)

                // check if a Style is referencing this LFO
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

                // 2.2.2 read all LFOLVL (and LVL) for the new NumRule

                WW8aISet aItemSet;       // Character attributes from GrpprlChpx
                WW8aCFormat aCharFormat;       // Character Style Pointer
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

                    // 2.2.2.1 read LFOLVL

                    rSt.ReadInt32( aLFOLVL.nStartAt );
                    sal_uInt8 aBits1(0);
                    rSt.ReadUChar( aBits1 );
                    rSt.SeekRel( 3 );
                    if (rSt.GetError())
                        break;

                    // Note: MS writes the Override-Level-Number into 4 bit.
                    // We do not! (See comment at "struct WW8LFOInfo")
                    aLFOLVL.nLevel = aBits1 & 0x0F;
                    if( (0xFF > aBits1) &&
                        (nMaxLevel > aLFOLVL.nLevel) )
                    {
                        if (aBits1 & 0x10)
                            aLFOLVL.bStartAt = true;
                        else
                            aLFOLVL.bStartAt = false;

                        // 2.2.2.2 load dedicated LVL if necessary

                        SwNumFormat aNumFormat(
                            rLFOInfo.pNumRule->Get(aLFOLVL.nLevel));
                        if (aBits1 & 0x20)
                        {
                            aLFOLVL.bFormat = true;
                            // if bStartup is true, replace Startup-Level
                            // with the LVLF that is saved in the LVL
                            bLVLOk = nLevel < rLFOInfo.maParaSprms.size() &&
                                ReadLVL(aNumFormat, aItemSet[nLevel],
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

                        // 2.2.2.3 Set NumFormat in NumRule

                        rLFOInfo.pNumRule->Set(aLFOLVL.nLevel, aNumFormat);
                    }
                    bLVLOk = true;

                    if (nMaxLevel > aLFOLVL.nLevel)
                        rLFOInfo.maOverrides[aLFOLVL.nLevel] = aLFOLVL;
                }
                if( !bLVLOk )
                    break;

                // 2.2.3 adjust LVL of the new NumRule

                bool bNewCharFormatCreated = false;
                for (sal_uInt8 nLevel = 0; nLevel < rLFOInfo.nLfoLvl; ++nLevel)
                {
                    AdjustLVL( nLevel, *rLFOInfo.pNumRule, aItemSet, aCharFormat,
                        bNewCharFormatCreated, sPrefix );
                }
            }
        }
    }
    // and we're done!
    rSt.Seek( nOriginalPos );
}

WW8ListManager::~WW8ListManager() COVERITY_NOEXCEPT_FALSE
{
    /*
     named lists remain in document
     unused automatic lists are removed from document (DelNumRule)
    */
    for(auto & rpInfo : maLSTInfos)
    {
        if (rpInfo->pNumRule && !rpInfo->bUsedInDoc &&
            rpInfo->pNumRule->IsAutoRule())
        {
            rDoc.DelNumRule(rpInfo->pNumRule->GetName());
        }
        rpInfo.reset();
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

static bool IsEqualFormatting(const SwNumRule &rOne, const SwNumRule &rTwo)
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
            if (rO != rT)
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
        if ( aFormat.GetNumAdjust() == SvxAdjust::Right)
            aFormat.SetNumAdjust(SvxAdjust::Left);
        else if ( aFormat.GetNumAdjust() == SvxAdjust::Left)
            aFormat.SetNumAdjust(SvxAdjust::Right);
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
    simple list. On the other hand it is possible that some of the other levels
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
                if (bFirstUse && rOverride.bStartAt)
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

// SwWW8ImplReader:  append a List to a Style or Paragraph

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
        const auto nAbsLSpace = rFormat.GetAbsLSpace();
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

void SwWW8ImplReader::SetStylesList(sal_uInt16 nStyle, sal_uInt16 nCurrentLFO,
    sal_uInt8 nCurrentLevel)
{
    if (nStyle >= m_vColl.size())
        return;

    SwWW8StyInf &rStyleInf = m_vColl[nStyle];
    if (rStyleInf.m_bValid)
    {
        OSL_ENSURE(m_pCurrentColl, "Cannot be called outside of style import");
        // Phase 1: Numbering attributes when reading a StyleDef
        if( m_pCurrentColl )
        {
            // only save the Parameters for now. The actual List will be appended
            // at a later point, when the Listdefinitions is read...
            if (
                 (USHRT_MAX > nCurrentLFO) &&
                 (WW8ListManager::nMaxLevel > nCurrentLevel)
               )
            {
                rStyleInf.m_nLFOIndex  = nCurrentLFO;
                rStyleInf.m_nListLevel = nCurrentLevel;

                std::vector<sal_uInt8> aParaSprms;
                SwNumRule* pNmRule = m_xLstManager->GetNumRuleForActivation(
                    nCurrentLFO, nCurrentLevel, aParaSprms);
                if (pNmRule)
                    UseListIndent(rStyleInf, pNmRule->Get(nCurrentLevel));
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

        // Phase 2: refresh StyleDef after reading all Lists
        SwNumRule* pNmRule = nullptr;
        const sal_uInt16 nLFO = rStyleInf.m_nLFOIndex;
        const sal_uInt8  nLevel = rStyleInf.m_nListLevel;
        if (
             (USHRT_MAX > nLFO) &&
             (WW8ListManager::nMaxLevel > nLevel)
           )
        {
            std::vector<sal_uInt8> aParaSprms;
            pNmRule = m_xLstManager->GetNumRuleForActivation(nLFO, nLevel,
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

void SwWW8ImplReader::RegisterNumFormatOnTextNode(sal_uInt16 nCurrentLFO,
                                              sal_uInt8 nCurrentLevel,
                                              const bool bSetAttr)
{
    // Note: the method appends NumRule to the Text Node if
    // bSetAttr (of course the lists have to be read before)
    // and only sets the Level. It does not check if there is a NumRule
    // attached to the STYLE !!!

    if (m_xLstManager) // are all list declarations read?
    {
        SwTextNode* pTextNd = m_pPaM->GetNode().GetTextNode();
        OSL_ENSURE(pTextNd, "No Text-Node at PaM-Position");
        if (!pTextNd)
            return;

        std::vector<sal_uInt8> aParaSprms;
        const SwNumRule* pRule = bSetAttr ?
            m_xLstManager->GetNumRuleForActivation( nCurrentLFO, nCurrentLevel,
                aParaSprms, pTextNd) : nullptr;

        if (pRule != nullptr || !bSetAttr)
        {
            if (bSetAttr && pTextNd->GetNumRule() != pRule
                && pTextNd->GetNumRule() != m_rDoc.GetOutlineNumRule())
            {
                pTextNd->SetAttr(SwNumRuleItem(pRule->GetName()));
            }
            pTextNd->SetAttrListLevel(nCurrentLevel);

            // <IsCounted()> state of text node has to be adjusted accordingly.
            if ( /*nCurrentLevel >= 0 &&*/ nCurrentLevel < MAXLEVEL )
            {
                pTextNd->SetCountedInList( true );
            }

            // #i99822#
            // Direct application of the list level formatting no longer
            // needed for list levels of mode LABEL_ALIGNMENT
            bool bApplyListLevelIndentDirectlyAtPara(true);
            {
                if (pTextNd->GetNumRule() && nCurrentLevel < MAXLEVEL)
                {
                    const SwNumFormat& rFormat = pTextNd->GetNumRule()->Get(nCurrentLevel);
                    if (rFormat.GetPositionAndSpaceMode()
                        == SvxNumberFormat::LABEL_ALIGNMENT)
                    {
                        bApplyListLevelIndentDirectlyAtPara = false;
                    }
                }
            }

            if (bApplyListLevelIndentDirectlyAtPara)
            {
                std::unique_ptr<SfxItemSet> xListIndent(new SfxItemSet(m_rDoc.GetAttrPool(), svl::Items<RES_LR_SPACE,
                                                                                                        RES_LR_SPACE>{}));
                const SvxLRSpaceItem *pItem = static_cast<const SvxLRSpaceItem*>(
                    GetFormatAttr(RES_LR_SPACE));
                OSL_ENSURE(pItem, "impossible");
                if (pItem)
                    xListIndent->Put(*pItem);

                /*
                 Take the original paragraph sprms attached to this list level
                 formatting and apply them to the paragraph. I'm convinced that
                 this is exactly what word does.
                */
                if (short nLen = static_cast< short >(aParaSprms.size()))
                {
                    std::unique_ptr<SfxItemSet> xOldCurrentItemSet(SetCurrentItemSet(std::move(xListIndent)));

                    sal_uInt8* pSprms1  = &aParaSprms[0];
                    while (0 < nLen)
                    {
                        sal_uInt16 nL1 = ImportSprm(pSprms1, nLen);
                        nLen = nLen - nL1;
                        pSprms1 += nL1;
                    }

                    xListIndent = SetCurrentItemSet(std::move(xOldCurrentItemSet));
                }

                if (const SvxLRSpaceItem *pLR = xListIndent->GetItem<SvxLRSpaceItem>(RES_LR_SPACE))
                {
                    m_xCtrlStck->NewAttr(*m_pPaM->GetPoint(), *pLR);
                    m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_LR_SPACE);
                }
            }
        }
    }
}

void SwWW8ImplReader::RegisterNumFormat(sal_uInt16 nCurrentLFO, sal_uInt8 nCurrentLevel)
{
    // Are we reading the StyleDef ?
    if (m_pCurrentColl)
        SetStylesList( m_nCurrentColl , nCurrentLFO, nCurrentLevel);
    else
        RegisterNumFormatOnTextNode(nCurrentLFO, nCurrentLevel);
}

void SwWW8ImplReader::Read_ListLevel(sal_uInt16, const sal_uInt8* pData,
    short nLen)
{
    if (m_xPlcxMan && m_xPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // the current level is finished, what should we do ?
        m_nListLevel = WW8ListManager::nMaxLevel;
        if (m_xStyles && !m_bVer67)
            m_xStyles->mnWwNumLevel = 0;
    }
    else
    {
        // security check
        if( !pData )
            return;

        // the Streamdata is zero based
        m_nListLevel = *pData;

        if (m_xStyles && !m_bVer67)
        {
            /*
            if this is the case, then if the numbering is actually stored in
            winword 6 format, and its likely that sprmPIlvl has been abused
            to set the ww6 list level information which we will need when we
            reach the true ww6 list def.  So set it now
            */
            m_xStyles->mnWwNumLevel = m_nListLevel;
        }

        if (WW8ListManager::nMaxLevel <= m_nListLevel )
            m_nListLevel = WW8ListManager::nMaxLevel;
        else if (USHRT_MAX > m_nLFOPosition)
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
    if (m_xPlcxMan && m_xPlcxMan->GetDoingDrawTextBox())
        return;

    if( nLen < 0 )
    {
        // the current level is finished, what should we do ?
        m_nLFOPosition = USHRT_MAX;
        m_nListLevel = WW8ListManager::nMaxLevel;
    }
    else
    {
        // security check
        if( !pData )
            return;

        short nData = SVBT16ToUInt16( pData );
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

            if (m_pCurrentColl)
            {
                // here a "named" style is being configured

                // disable the numbering/list in the style currently configured
                m_pCurrentColl->SetFormatAttr(*GetDfltAttr(RES_PARATR_NUMRULE));

                // reset/blank the indent
                m_pCurrentColl->SetFormatAttr(SvxLRSpaceItem(RES_LR_SPACE));
            }
            else if (SwTextNode* pTextNode = m_pPaM->GetNode().GetTextNode())
            {
                // here a paragraph is being directly formatted

                // empty the numbering/list style applied to the current paragraph
                SwNumRuleItem aEmptyRule;
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
        else // nData in (0..0x7FFF]
        {
            m_nLFOPosition = static_cast<sal_uInt16>(nData)-1; // m_nLFOPosition in [0..0x7FFF)
            /*
            If we are a ww8+ style with ww7- style lists then there is a
            bizarre broken word bug where when the list is removed from a para
            the ww6 list first line indent still affects the first line
            indentation.  Setting this flag will allow us to recover from this
            braindeadness
            */
            if (m_pCurrentColl && (m_nLFOPosition == 2047-1) && m_nCurrentColl < m_vColl.size())
                m_vColl[m_nCurrentColl].m_bHasBrokenWW6List = true;

            // here the stream data is 1-based, we subtract ONE
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
            else if (m_xPlcxMan && m_xPlcxMan->HasParaSprm(NS_sprm::LN_PAnld).pSprm)
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
    nStart = m_xPlcxMan->Where();
    while(nStart <= nEndCp)
    {
        if ( m_xPlcxMan->Get(&aRes)
            && aRes.pMemPos && aRes.nSprmId )
        {
            //only interested in sprms which would set nPicLocFc
            if ( (68 == aRes.nSprmId) || (0x6A03 == aRes.nSprmId) )
            {
                Read_PicLoc( aRes.nSprmId, aRes.pMemPos +
                    m_xSprmParser->DistanceToData(aRes.nSprmId), 4);
                break;
            }
        }
        m_xPlcxMan->advance();
        nStart = m_xPlcxMan->Where();
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

void SwMSConvertControls::InsertFormula(WW8FormulaControl &rFormula)
{
    const uno::Reference< lang::XMultiServiceFactory > & rServiceFactory =
        GetServiceFactory();

    if(!rServiceFactory.is())
        return;

    awt::Size aSz;
    uno::Reference< form::XFormComponent> xFComp;

    if (rFormula.Import(rServiceFactory, xFComp, aSz))
    {
        uno::Reference <drawing::XShape> xShapeRef;
        if (InsertControl(xFComp, aSz, &xShapeRef, false))
            GetShapes()->add(xShapeRef);
    }
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
    if ( iType != nWhich )
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

    // xstzEntryMcr
    msEntryMcr = read_uInt16_BeltAndBracesString(*pDataStream);
    //xstzExitMcr
    msExitMcr = read_uInt16_BeltAndBracesString(*pDataStream);

    if (nWhich == WW8_CT_DROPDOWN)
    {
        bool bAllOk = true;
        // SSTB (see Spec. 2.2.4)
        sal_uInt16 fExtend = 0;
        pDataStream->ReadUInt16( fExtend );
        sal_uInt16 nStringsCnt = 0;

        // Isn't it that if fExtend isn't 0xFFFF then fExtend actually
        // doesn't exist and we really have just read nStringsCnt ( or cData )?
        if (fExtend != 0xFFFF)
            bAllOk = false;
        pDataStream->ReadUInt16( nStringsCnt );

        // I guess this should be zero ( and we should ensure that )
        sal_uInt16 cbExtra = 0;
        pDataStream->ReadUInt16( cbExtra );

        OSL_ENSURE(bAllOk, "Unknown formfield dropdown list structure");
        if (!bAllOk)    //Not as expected, don't risk it at all.
            nStringsCnt = 0;
        const size_t nMinRecordSize = sizeof(sal_uInt16);
        const size_t nMaxRecords = pDataStream->remainingSize() / nMinRecordSize;
        if (nStringsCnt > nMaxRecords)
        {
            SAL_WARN("sw.ww8", "Parsing error: " << nMaxRecords <<
                     " max possible entries, but " << nStringsCnt << " claimed, truncating");
            nStringsCnt = nMaxRecords;
        }
        maListEntries.reserve(nStringsCnt);
        for (sal_uInt32 nI = 0; nI < nStringsCnt; ++nI)
        {
            OUString sEntry =  read_uInt16_PascalString(*pDataStream);
            maListEntries.push_back(sEntry);
        }
    }
    mfDropdownIndex = iRes;

    mbHelp = bits1 & 0x80;

    nField = bits2;
    mfToolTip = nField & 0x01;
    mfNoMark = (nField & 0x02)>>1;
    mfType = (nField & 0x38)>>3;
    mfUnused = (nField & 0xE0)>>5;
}

WW8FormulaListBox::WW8FormulaListBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(SL::aListBox, rR)
{
}

//Miserable hack to get a hardcoded guesstimate of the size of a list dropdown
//box's first entry to set as the lists default size
awt::Size SwWW8ImplReader::MiserableDropDownFormHack(const OUString &rString,
    uno::Reference<beans::XPropertySet> const & rPropSet)
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
                    aTmp <<= static_cast<sal_Int32>(static_cast<const SvxColorItem*>(pItem)->GetValue());
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
                    aTmp <<= pFontItem->GetStyleName();
                    rPropSet->setPropertyValue( aNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(aNm = "FontFamily"))
                {
                    aTmp <<= static_cast<sal_Int16>(pFontItem->GetFamily());
                    rPropSet->setPropertyValue( aNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(aNm = "FontCharset"))
                {
                    aTmp <<= static_cast<sal_Int16>(pFontItem->GetCharSet());
                    rPropSet->setPropertyValue( aNm, aTmp );
                }
                if (xPropSetInfo->hasPropertyByName(aNm = "FontPitch"))
                {
                    aTmp <<= static_cast<sal_Int16>(pFontItem->GetPitch());
                    rPropSet->setPropertyValue( aNm, aTmp );
                }

                aTmp <<= pFontItem->GetFamilyName();
                aFont.SetFamilyName( pFontItem->GetFamilyName() );
                aFont.SetStyleName( pFontItem->GetStyleName() );
                aFont.SetFamily( pFontItem->GetFamily() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
            }
            break;

        case RES_CHRATR_FONTSIZE:
            {
                Size aSize( aFont.GetFontSize().Width(),
                            static_cast<const SvxFontHeightItem*>(pItem)->GetHeight() );
                aTmp <<= static_cast<float>(aSize.Height()) / 20.0;

                aFont.SetFontSize(OutputDevice::LogicToLogic(aSize,
                    MapMode(MapUnit::MapTwip), MapMode(MapUnit::Map100thMM)));
            }
            break;

        case RES_CHRATR_WEIGHT:
            aTmp <<= vcl::unohelper::ConvertFontWeight(
                                        static_cast<const SvxWeightItem*>(pItem)->GetWeight() );
            aFont.SetWeight( static_cast<const SvxWeightItem*>(pItem)->GetWeight() );
            break;

        case RES_CHRATR_UNDERLINE:
            aTmp <<= static_cast<sal_Int16>(static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle());
            aFont.SetUnderline(static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle());
            break;

        case RES_CHRATR_CROSSEDOUT:
            aTmp <<= static_cast<sal_Int16>( static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout() );
            aFont.SetStrikeout( static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout() );
            break;

        case RES_CHRATR_POSTURE:
            aTmp <<= static_cast<sal_Int16>( static_cast<const SvxPostureItem*>(pItem)->GetPosture() );
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
        pOut->SetMapMode( MapMode( MapUnit::Map100thMM ));
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

    xPropSet->setPropertyValue("Dropdown", css::uno::makeAny(true));

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
    : WW8FormulaControl(SL::aCheckBox, rR)
{
}

static void lcl_AddToPropertyContainer
(uno::Reference<beans::XPropertySet> const & xPropSet,
 const OUString & rPropertyName, const OUString & rValue)
{
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if (xPropSetInfo.is() &&
        ! xPropSetInfo->hasPropertyByName(rPropertyName))
    {
        uno::Reference<beans::XPropertyContainer>
            xPropContainer(xPropSet, uno::UNO_QUERY);
        uno::Any aAny((OUString()));
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

    aTmp <<= static_cast<sal_Int16>(mnChecked);
    xPropSet->setPropertyValue("DefaultState", aTmp);

    if (!msToolTip.isEmpty())
        lcl_AddToPropertyContainer(xPropSet, "HelpText", msToolTip);

    if (!msHelp.isEmpty())
        lcl_AddToPropertyContainer(xPropSet, "HelpF1Text", msHelp);

    return true;

}

WW8FormulaEditBox::WW8FormulaEditBox(SwWW8ImplReader &rR)
    : WW8FormulaControl(SL::aTextField ,rR)
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

    OSL_ENSURE(xShape.is(), "Did not get XShape");
    xShape->setSize(rSize);

    uno::Reference< beans::XPropertySet > xShapePropSet(
        xCreate, uno::UNO_QUERY );

    //I lay a small bet that this will change to
    //sal_Int16 nTemp=TextContentAnchorType::AS_CHARACTER;
    text::TextContentAnchorType nTemp;
    if (bFloatingCtrl)
        nTemp = text::TextContentAnchorType_AT_PARAGRAPH;
    else
        nTemp = text::TextContentAnchorType_AS_CHARACTER;

    xShapePropSet->setPropertyValue("AnchorType", uno::Any(static_cast<sal_Int16>(nTemp)) );

    xShapePropSet->setPropertyValue("VertOrient", uno::Any(sal_Int16(text::VertOrientation::TOP)) );

    uno::Reference< text::XText >  xDummyTextRef;
    uno::Reference< text::XTextRange >  xTextRg =
        new SwXTextRange( *pPaM, xDummyTextRef );

    aTmp <<= xTextRg;
    xShapePropSet->setPropertyValue("TextRange", aTmp );

    // Set the Control-Model for the Control-Shape
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
