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

#include <sal/config.h>

#include <comphelper/string.hxx>
#include <tools/solar.h>
#include <vcl/vclenum.hxx>
#include <vcl/font.hxx>
#include <hintids.hxx>
#include <editeng/colritem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/pgrditem.hxx>
#include <msfilter.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <poolfmt.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <mdiexp.hxx>
#include <fmtpdsc.hxx>
#include <txtftn.hxx>
#include <frmfmt.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <fmtlsplt.hxx>
#include <charfmt.hxx>
#include <SwStyleNameMapper.hxx>
#include <fltshell.hxx>
#include <fmtanchr.hxx>
#include <fmtrowsplt.hxx>
#include <fmtfollowtextflow.hxx>
#include <numrule.hxx>
#include "sprmids.hxx"
#include <wwstyles.hxx>
#include "writerhelper.hxx"
#include "ww8struc.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"

#include <frmatr.hxx>
#include <itabenum.hxx>

#include <iostream>
#include <memory>

using namespace ::com::sun::star;

WW8TabBandDesc::WW8TabBandDesc()
{
    memset(this, 0, sizeof(*this));
    for (sal_uInt16 & rn : maDirections)
        rn = 4;
}

WW8TabBandDesc::~WW8TabBandDesc()
{
    delete[] pTCs;
    delete[] pSHDs;
    delete[] pNewSHDs;
}

void sw::util::RedlineStack::close( const SwPosition& rPos,
    RedlineType_t eType, WW8TabDesc* pTabDesc )
{
    // If the redline type is not found in the redline stack, we have to check if there has been
    // a tabledesc and to check its saved redline stack, too. (#136939, #i68139)
    if( !close( rPos, eType ) )
    {
        if( pTabDesc && pTabDesc->getOldRedlineStack() )
        {
            bool const bResult =
                pTabDesc->getOldRedlineStack()->close(rPos, eType);
            OSL_ENSURE( bResult, "close without open!");
        }
    }
}

void wwSectionManager::SetCurrentSectionHasFootnote()
{
    OSL_ENSURE(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if (!maSegments.empty())
        maSegments.back().mbHasFootnote = true;
}

void wwSectionManager::SetCurrentSectionVerticalAdjustment(const drawing::TextVerticalAdjust nVA)
{
    OSL_ENSURE(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if ( !maSegments.empty() )
        maSegments.back().mnVerticalAdjustment = nVA;
}

bool wwSectionManager::CurrentSectionIsVertical() const
{
    OSL_ENSURE(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if (!maSegments.empty())
        return maSegments.back().IsVertical();
    return false;
}

bool wwSectionManager::CurrentSectionIsProtected() const
{
    OSL_ENSURE(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if (!maSegments.empty())
        return SectionIsProtected(maSegments.back());
    return false;
}

sal_uInt32 wwSectionManager::GetPageLeft() const
{
    return !maSegments.empty() ? maSegments.back().nPgLeft : 0;
}

sal_uInt32 wwSectionManager::GetPageRight() const
{
    return !maSegments.empty() ? maSegments.back().nPgRight : 0;
}

sal_uInt32 wwSectionManager::GetPageWidth() const
{
    return !maSegments.empty() ? maSegments.back().GetPageWidth() : 0;
}

sal_uInt32 wwSectionManager::GetTextAreaWidth() const
{
    return !maSegments.empty() ? maSegments.back().GetTextAreaWidth() : 0;
}

sal_uInt32 wwSectionManager::GetWWPageTopMargin() const
{
    return !maSegments.empty() ? maSegments.back().maSep.dyaTop : 0;
}

sal_uInt16 SwWW8ImplReader::End_Footnote()
{
    /*
    Ignoring Footnote outside of the normal Text. People will put footnotes
    into field results and field commands.
    */
    if (m_bIgnoreText ||
        m_pPaM->GetPoint()->nNode < m_rDoc.GetNodes().GetEndOfExtras().GetIndex())
    {
        return 0;
    }

    OSL_ENSURE(!m_aFootnoteStack.empty(), "footnote end without start");
    if (m_aFootnoteStack.empty())
        return 0;

    bool bFtEdOk = false;
    const FootnoteDescriptor &rDesc = m_aFootnoteStack.back();

    //Get the footnote character and remove it from the txtnode. We'll
    //replace it with the footnote
    SwTextNode* pText = m_pPaM->GetNode().GetTextNode();
    sal_Int32 nPos = m_pPaM->GetPoint()->nContent.GetIndex();

    OUString sChar;
    SwTextAttr* pFN = nullptr;
    //There should have been a footnote char, we will replace this.
    if (pText && nPos)
    {
        sChar += OUStringLiteral1(pText->GetText()[--nPos]);
        m_pPaM->SetMark();
        --m_pPaM->GetMark()->nContent;
        m_rDoc.getIDocumentContentOperations().DeleteRange( *m_pPaM );
        m_pPaM->DeleteMark();
        SwFormatFootnote aFootnote(rDesc.meType == MAN_EDN);
        pFN = pText->InsertItem(aFootnote, nPos, nPos);
    }
    OSL_ENSURE(pFN, "Problems creating the footnote text");
    if (pFN)
    {

        SwPosition aTmpPos( *m_pPaM->GetPoint() );    // remember old cursor position
        WW8PLCFxSaveAll aSave;
        m_xPlcxMan->SaveAllPLCFx( aSave );
        std::shared_ptr<WW8PLCFMan> xOldPlcxMan = m_xPlcxMan;

        const SwNodeIndex* pSttIdx = static_cast<SwTextFootnote*>(pFN)->GetStartNode();
        OSL_ENSURE(pSttIdx, "Problems creating footnote text");

        static_cast<SwTextFootnote*>(pFN)->SetSeqNo( m_rDoc.GetFootnoteIdxs().size() );

        bool bOld = m_bFootnoteEdn;
        m_bFootnoteEdn = true;

        // read content of Ft-/End-Note
        Read_HdFtFootnoteText( pSttIdx, rDesc.mnStartCp, rDesc.mnLen, rDesc.meType);
        bFtEdOk = true;
        m_bFootnoteEdn = bOld;

        OSL_ENSURE(sChar.getLength()==1 && ((rDesc.mbAutoNum == (sChar[0] == 2))),
         "footnote autonumbering must be 0x02, and everything else must not be");

        // If no automatic numbering use the following char from the main text
        // as the footnote number
        if (!rDesc.mbAutoNum)
            static_cast<SwTextFootnote*>(pFN)->SetNumber(0, sChar);

        /*
            Delete the footnote char from the footnote if its at the beginning
            as usual. Might not be if the user has already deleted it, e.g.
            #i14737#
        */
        SwNodeIndex& rNIdx = m_pPaM->GetPoint()->nNode;
        rNIdx = pSttIdx->GetIndex() + 1;
        SwTextNode* pTNd = rNIdx.GetNode().GetTextNode();
        if (pTNd && !pTNd->GetText().isEmpty() && !sChar.isEmpty())
        {
            const OUString &rText = pTNd->GetText();
            if (rText[0] == sChar[0])
            {
                // Allow MSO to emulate LO footnote text starting at left margin - only meaningful with hanging indent
                sal_Int32 nFirstLineIndent=0;
                SfxItemSet aSet( m_rDoc.GetAttrPool(), svl::Items<RES_LR_SPACE, RES_LR_SPACE>{} );
                if ( pTNd->GetAttr(aSet) )
                {
                    const SvxLRSpaceItem* pLRSpace = aSet.GetItem<SvxLRSpaceItem>(RES_LR_SPACE);
                    if ( pLRSpace )
                        nFirstLineIndent = pLRSpace->GetTextFirstLineOfst();
                }

                m_pPaM->GetPoint()->nContent.Assign( pTNd, 0 );
                m_pPaM->SetMark();
                // Strip out aesthetic tabs we may have inserted on export #i24762#
                if (nFirstLineIndent < 0 && rText.getLength() > 1 && rText[1] == 0x09)
                    ++m_pPaM->GetMark()->nContent;
                ++m_pPaM->GetMark()->nContent;
                m_xReffingStck->Delete(*m_pPaM);
                m_rDoc.getIDocumentContentOperations().DeleteRange( *m_pPaM );
                m_pPaM->DeleteMark();
            }
        }

        *m_pPaM->GetPoint() = aTmpPos;        // restore Cursor

        m_xPlcxMan = xOldPlcxMan;             // Restore attributes
        m_xPlcxMan->RestoreAllPLCFx( aSave );
    }

    if (bFtEdOk)
        m_aSectionManager.SetCurrentSectionHasFootnote();

    m_aFootnoteStack.pop_back();
    return 0;
}

long SwWW8ImplReader::Read_Footnote(WW8PLCFManResult* pRes)
{
    /*
    Ignoring Footnote outside of the normal Text. People will put footnotes
    into field results and field commands.
    */
    if (m_bIgnoreText ||
        m_pPaM->GetPoint()->nNode < m_rDoc.GetNodes().GetEndOfExtras().GetIndex())
    {
        return 0;
    }

    FootnoteDescriptor aDesc;
    aDesc.mbAutoNum = true;
    if (eEDN == pRes->nSprmId)
    {
        aDesc.meType = MAN_EDN;
        WW8PLCFx_SubDoc* pEndNote = m_xPlcxMan->GetEdn();
        if (const void* pData = pEndNote ? pEndNote->GetData() : nullptr)
            aDesc.mbAutoNum = 0 != *static_cast<short const*>(pData);
    }
    else
    {
        aDesc.meType = MAN_FTN;
        WW8PLCFx_SubDoc* pFootNote = m_xPlcxMan->GetFootnote();
        if (const void* pData = pFootNote ? pFootNote->GetData() : nullptr)
            aDesc.mbAutoNum = 0 != *static_cast<short const*>(pData);
    }

    aDesc.mnStartCp = pRes->nCp2OrIdx;
    aDesc.mnLen = pRes->nMemLen;

    m_aFootnoteStack.push_back(aDesc);

    return 0;
}

bool SwWW8ImplReader::SearchRowEnd(WW8PLCFx_Cp_FKP* pPap, WW8_CP &rStartCp,
    int nLevel) const
{
    WW8PLCFxDesc aRes;
    aRes.pMemPos = nullptr;
    aRes.nEndPos = rStartCp;
    std::set<std::pair<WW8_CP, WW8_CP>> aPrevRes;

    while (pPap->HasFkp() && rStartCp != WW8_CP_MAX)
    {
        if (pPap->Where() != WW8_CP_MAX)
        {
            SprmResult aSprmRes = pPap->HasSprm(TabRowSprm(nLevel));
            const sal_uInt8* pB = aSprmRes.pSprm;
            if (pB && aSprmRes.nRemainingData >= 1 && *pB == 1)
            {
                aSprmRes = pPap->HasSprm(0x6649);
                const sal_uInt8 *pLevel = aSprmRes.pSprm;
                if (pLevel && aSprmRes.nRemainingData >= 1)
                {
                    if (nLevel + 1 == *pLevel)
                        return true;
                }
                else
                {
                    OSL_ENSURE(!nLevel || pLevel, "sublevel without level sprm");
                    return true;    // RowEnd found
                }
            }
        }

        aRes.nStartPos = aRes.nEndPos;
        aRes.pMemPos = nullptr;
        //Seek to our next block of properties
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);
        auto aBounds(std::make_pair(aRes.nStartPos, aRes.nEndPos));
        if (!aPrevRes.insert(aBounds).second) //already seen these bounds, infinite loop
        {
            SAL_WARN("sw.ww8", "SearchRowEnd, loop in paragraph property chain");
            break;
        }
        //Update our aRes to get the new starting point of the next properties
        rStartCp = aRes.nEndPos;
    }

    return false;
}

bool SwWW8ImplReader::SearchTableEnd(WW8PLCFx_Cp_FKP* pPap) const
{
    if (m_bVer67)
        // The below SPRM is for WW8 only.
        return false;

    WW8PLCFxDesc aRes;
    aRes.pMemPos = nullptr;
    aRes.nEndPos = pPap->Where();
    std::set<std::pair<WW8_CP, WW8_CP>> aPrevRes;

    while (pPap->HasFkp() && pPap->Where() != WW8_CP_MAX)
    {
        // See if the current pap is outside the table.
        SprmResult aSprmRes = pPap->HasSprm(NS_sprm::sprmPFInTable);
        const sal_uInt8* pB = aSprmRes.pSprm;
        if (!pB || aSprmRes.nRemainingData < 1 || *pB != 1)
            // Yes, this is the position after the end of the table.
            return true;

        // It is, so seek to the next pap.
        aRes.nStartPos = aRes.nEndPos;
        aRes.pMemPos = nullptr;
        if (!pPap->SeekPos(aRes.nStartPos))
            return false;

        // Read the sprms and make sure we moved forward to avoid infinite loops.
        pPap->GetSprms(&aRes);
        auto aBounds(std::make_pair(aRes.nStartPos, aRes.nEndPos));
        if (!aPrevRes.insert(aBounds).second) //already seen these bounds, infinite loop
        {
            SAL_WARN("sw.ww8", "SearchTableEnd, loop in paragraph property chain");
            break;
        }
    }

    return false;
}

ApoTestResults SwWW8ImplReader::TestApo(int nCellLevel, bool bTableRowEnd,
    const WW8_TablePos *pTabPos)
{
    const WW8_TablePos *pTopLevelTable = nCellLevel <= 1 ? pTabPos : nullptr;
    ApoTestResults aRet;
    // Frame in Style Definition (word appears to ignore them if inside an
    // text autoshape)
    sal_uInt16 const nStyle(m_xPlcxMan->GetColl());
    if (!m_bTxbxFlySection && nStyle < m_vColl.size())
        aRet.mpStyleApo = StyleExists(nStyle) ? m_vColl[nStyle].m_xWWFly.get() : nullptr;

    /*
    #i1140#
    If I have a table and apply a style to one of its frames that should cause
    a paragraph that it is applied to it to only exist as a separate floating
    frame, then the behaviour depends on which cell that it has been applied
    to. If it is the first cell of a row then the whole table row jumps into the
    new frame, if it isn't then the paragraph attributes are applied
    "except" for the floating frame stuff. i.e. it's ignored. So if there's a
    table, and we're not in the first cell then we ignore the fact that the
    paragraph style wants to be in a different frame.

    This sort of mindbending inconsistency is surely why frames are deprecated
    in word 97 onwards and hidden away from the user

    #i1532# & #i5379#
    If we are already a table in a frame then we must grab the para properties
    to see if we are still in that frame.
    */

    aRet.m_bHasSprm37 = m_xPlcxMan->HasParaSprm(m_bVer67 ? 37 : 0x2423).pSprm != nullptr;
    SprmResult aSrpm29 = m_xPlcxMan->HasParaSprm(m_bVer67 ? 29 : 0x261B);
    const sal_uInt8 *pSrpm29 = aSrpm29.pSprm;
    aRet.m_bHasSprm29 = pSrpm29 != nullptr;
    aRet.m_nSprm29 = (pSrpm29 && aSrpm29.nRemainingData >= 1) ? *pSrpm29 : 0;

    // Is there some frame data here
    bool bNowApo = aRet.HasFrame() || pTopLevelTable;
    if (bNowApo)
    {
        if (WW8FlyPara *pTest = ConstructApo(aRet, pTabPos))
            delete pTest;
        else
            bNowApo = false;
    }

    bool bTestAllowed = !m_bTxbxFlySection && !bTableRowEnd;
    if (bTestAllowed)
    {
        //Test is allowed if there is no table.
        //Otherwise only allowed if we are in the
        //first paragraph of the first cell of a row.
        //(And only if the row we are inside is at the
        //same level as the previous row, think tables
        //in tables)
        if (nCellLevel == m_nInTable)
        {

            if (!m_nInTable)
                bTestAllowed = true;
            else
            {
                if (!m_xTableDesc)
                {
                    OSL_ENSURE(m_xTableDesc.get(), "What!");
                    bTestAllowed = false;
                }
                else
                {
                    // #i39468#
                    // If current cell isn't valid, the test is allowed.
                    // The cell isn't valid, if e.g. there is a new row
                    // <pTableDesc->nCurrentRow> >= <pTableDesc->pTabLines->Count()>
                    bTestAllowed =
                        m_xTableDesc->GetCurrentCol() == 0 &&
                        ( !m_xTableDesc->IsValidCell( m_xTableDesc->GetCurrentCol() ) ||
                          m_xTableDesc->InFirstParaInCell() );
                }
            }
        }
    }

    if (!bTestAllowed)
        return aRet;

    aRet.mbStartApo = bNowApo && !InEqualOrHigherApo(1); // APO-start
    aRet.mbStopApo = InEqualOrHigherApo(nCellLevel) && !bNowApo;  // APO-end

    //If it happens that we are in a table, then if it's not the first cell
    //then any attributes that might otherwise cause the contents to jump
    //into another frame don't matter, a table row sticks together as one
    //unit no matter what else happens. So if we are not in a table at
    //all, or if we are in the first cell then test that the last frame
    //data is the same as the current one
    if (bNowApo && InEqualApo(nCellLevel))
    {
        // two bordering each other
        if (!TestSameApo(aRet, pTabPos))
            aRet.mbStopApo = aRet.mbStartApo = true;
    }

    return aRet;
}

// helper methods for outline, numbering and bullets

static void SetBaseAnlv(SwNumFormat &rNum, WW8_ANLV const &rAV, sal_uInt8 nSwLevel )
{
    static const SvxNumType eNumA[8] = { SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC };

    static const SvxAdjust eAdjA[4] = { SvxAdjust::Left,
        SvxAdjust::Right, SvxAdjust::Left, SvxAdjust::Left };
    if (rAV.nfc < 8) {
        rNum.SetNumberingType( eNumA[ rAV.nfc ] );
    } else {
        SvxNumType nType = SVX_NUM_ARABIC;
        switch( rAV.nfc ) {
        case 14:
        case 19:nType = SVX_NUM_FULL_WIDTH_ARABIC; break;
        case 30:nType = SVX_NUM_TIAN_GAN_ZH; break;
        case 31:nType = SVX_NUM_DI_ZI_ZH; break;
        case 35:
        case 36:
        case 37:
        case 39:nType = SVX_NUM_NUMBER_LOWER_ZH; break;
        case 34:nType = SVX_NUM_NUMBER_UPPER_ZH_TW; break;
        case 38:nType = SVX_NUM_NUMBER_UPPER_ZH; break;
        case 10:
        case 11:nType = SVX_NUM_NUMBER_TRADITIONAL_JA; break;
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
            nType= SVX_NUM_ARABIC;break;
        }
        rNum.SetNumberingType( nType );
    }

    if ((rAV.aBits1 & 0x4) >> 2)
    {
        rNum.SetIncludeUpperLevels(nSwLevel + 1);
    }
    rNum.SetStart( SVBT16ToShort( rAV.iStartAt ) );
    rNum.SetNumAdjust( eAdjA[ rAV.aBits1 & 0x3] );

    rNum.SetCharTextDistance( SVBT16ToShort( rAV.dxaSpace ) );
    sal_Int16 nIndent = std::abs(static_cast<sal_Int16>(SVBT16ToShort( rAV.dxaIndent )));
    if( rAV.aBits1 & 0x08 )      //fHang
    {
        rNum.SetFirstLineOffset( -nIndent );
        rNum.SetAbsLSpace( nIndent );
    }
    else
        rNum.SetCharTextDistance( nIndent );        // width of number is missing

    if( rAV.nfc == 5 || rAV.nfc == 7 )
    {
        OUString sP = "." + rNum.GetSuffix();
        rNum.SetSuffix( sP );   // ordinal number
    }
}

void SwWW8ImplReader::SetAnlvStrings(SwNumFormat &rNum, WW8_ANLV const &rAV,
    const sal_uInt8* pText, size_t nStart, size_t nElements, bool bOutline)
{
    if (nStart > nElements)
        return;

    pText += nStart;
    nElements -= nStart;

    bool bInsert = false;                       // Default
    rtl_TextEncoding eCharSet = m_eStructCharSet;

    const WW8_FFN* pF = m_xFonts->GetFont(SVBT16ToShort(rAV.ftc)); // FontInfo
    bool bListSymbol = pF && ( pF->chs == 2 );      // Symbol/WingDings/...

    OUString sText;
    sal_uInt32 nLen = rAV.cbTextBefore + rAV.cbTextAfter;
    if (m_bVer67)
    {
        if (nLen > nElements)
        {
            SAL_WARN("sw.ww8", "SetAnlvStrings: ignoring out of range "
                << nLen << " vs " << nElements << " max");
            return;
        }
        sText = OUString(reinterpret_cast<char const *>(pText), nLen, eCharSet);
    }
    else
    {
        if (nLen > nElements / 2)
        {
            SAL_WARN("sw.ww8", "SetAnlvStrings: ignoring out of range "
                << nLen << " vs " << nElements / 2 << " max");
            return;
        }
        for(sal_uInt32 i = 0; i < nLen; ++i, pText += 2)
        {
            sText += OUStringLiteral1(SVBT16ToShort(*reinterpret_cast<SVBT16 const *>(pText)));
        }
    }

    if( bOutline )
    {                             // outline
        if( !rNum.GetIncludeUpperLevels()                          // there are  <= 1 number to show
            || rNum.GetNumberingType() == SVX_NUM_NUMBER_NONE )    // or this level has none
        {
                                                // if self defined digits
            bInsert = true;                     // then apply character

            // replace by simple Bullet ?
            if( bListSymbol )
            {
                // use cBulletChar for correct mapping on MAC
                OUStringBuffer aBuf;
                comphelper::string::padToLength(aBuf, rAV.cbTextBefore
                    + rAV.cbTextAfter, cBulletChar);
                sText = aBuf.makeStringAndClear();
            }
        }
    }
    else
    {                                       // numbering / bullets
        bInsert = true;
        if( bListSymbol )
        {
            FontFamily eFamily;
            OUString aName;
            FontPitch ePitch;

            if( GetFontParams( SVBT16ToShort( rAV.ftc ), eFamily, aName,
                                ePitch, eCharSet ) ){

                vcl::Font aFont;
                aFont.SetFamilyName( aName );
                aFont.SetFamily( eFamily );

                aFont.SetCharSet( eCharSet );
                rNum.SetNumberingType(SVX_NUM_CHAR_SPECIAL);

                rNum.SetBulletFont( &aFont );

                // take only the very first character
                if (rAV.cbTextBefore || rAV.cbTextAfter)
                    rNum.SetBulletChar( sText[ 0 ] );
                else
                    rNum.SetBulletChar( 0x2190 );
            }
        }
    }
    if( bInsert )
    {
        if (rAV.cbTextBefore)
        {
            OUString sP( sText.copy( 0, rAV.cbTextBefore ) );
            rNum.SetPrefix( sP );
        }
        if( rAV.cbTextAfter )
        {
            OUString sP( rNum.GetSuffix() );
            sP += sText.copy( rAV.cbTextBefore, rAV.cbTextAfter);
            rNum.SetSuffix( sP );
        }
// The characters before and after multiple digits do not apply because
// those are handled differently by the writer and the result is in most
// cases worse than without.
    }
}

// SetAnld gets a WW-ANLD-Descriptor and a Level and modifies the NumRules
// which are provided by pNumR. This is used for everything beside
// outline inside the text.
void SwWW8ImplReader::SetAnld(SwNumRule* pNumR, WW8_ANLD const * pAD, sal_uInt8 nSwLevel,
    bool bOutLine)
{
    SwNumFormat aNF;
    if (pAD)
    {                                                       // there is a Anld-Sprm
        m_bCurrentAND_fNumberAcross = 0 != pAD->fNumberAcross;
        WW8_ANLV const &rAV = pAD->eAnlv;
        SetBaseAnlv(aNF, rAV, nSwLevel);                    // set the base format
        SetAnlvStrings(aNF, rAV, pAD->rgchAnld, 0, SAL_N_ELEMENTS(pAD->rgchAnld), bOutLine); // set the rest
    }
    pNumR->Set(nSwLevel, aNF);
}

// chapter numbering and bullets

// Chapter numbering happens in the style definition.
// Sprm 13 provides the level, Sprm 12 the content.

SwNumRule* SwWW8ImplReader::GetStyRule()
{
    if( m_xStyles->mpStyRule )         // Bullet-Style already present
        return m_xStyles->mpStyRule;

    const OUString aBaseName("WW8StyleNum");
    const OUString aName( m_rDoc.GetUniqueNumRuleName( &aBaseName, false) );

    // #i86652#
    sal_uInt16 nRul = m_rDoc.MakeNumRule( aName, nullptr, false,
                                    SvxNumberFormat::LABEL_ALIGNMENT );
    m_xStyles->mpStyRule = m_rDoc.GetNumRuleTable()[nRul];
    // Auto == false-> numbering style
    m_xStyles->mpStyRule->SetAutoRule(false);

    return m_xStyles->mpStyRule;
}

// Sprm 13
void SwWW8ImplReader::Read_ANLevelNo( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    m_nSwNumLevel = 0xff; // Default: invalid

    if( nLen <= 0 )
        return;

    // StyleDef ?
    if( m_pCurrentColl )
    {
        // only for SwTextFormatColl, not CharFormat
        // WW: 0 = no Numbering
        SwWW8StyInf * pColl = GetStyle(m_nCurrentColl);
        if (pColl != nullptr && pColl->m_bColl && *pData)
        {
            // Range WW:1..9 -> SW:0..8 no bullets / numbering

            if (*pData <= MAXLEVEL && *pData <= 9)
            {
                m_nSwNumLevel = *pData - 1;
                if (!m_bNoAttrImport)
                    static_cast<SwTextFormatColl*>(m_pCurrentColl)->AssignToListLevelOfOutlineStyle( m_nSwNumLevel );
                    // For WW-NoNumbering also NO_NUMBERING could be used.
                    // ( For normal numberierung NO_NUM has to be used:
                    //   NO_NUM : pauses numbering,
                    //   NO_NUMBERING : no numbering at all )

            }
            else if( *pData == 10 || *pData == 11 )
            {
                // remember type, the rest happens at Sprm 12
                m_xStyles->mnWwNumLevel = *pData;
            }
        }
    }
    else
    {
        //Not StyleDef
        if (!m_bAnl)
            StartAnl(pData);        // begin of outline / bullets
        NextAnlLine(pData);
    }
}

void SwWW8ImplReader::Read_ANLevelDesc( sal_uInt16, const sal_uInt8* pData, short nLen ) // Sprm 12
{
    SwWW8StyInf * pStyInf = GetStyle(m_nCurrentColl);
    if( !m_pCurrentColl || nLen <= 0                       // only for Styledef
        || (pStyInf && !pStyInf->m_bColl)              // ignore  CharFormat ->
        || ( m_nIniFlags & WW8FL_NO_OUTLINE ) )
    {
        m_nSwNumLevel = 0xff;
        return;
    }

    if (static_cast<size_t>(nLen) < sizeof(WW8_ANLD))
    {
        SAL_WARN("sw.ww8", "ANLevelDesc property is " << nLen << " long, needs to be at least " << sizeof(WW8_ANLD));
        m_nSwNumLevel = 0xff;
        return;
    }

    if( m_nSwNumLevel <= MAXLEVEL         // Value range mapping WW:1..9 -> SW:0..8
        && m_nSwNumLevel <= 9 ){          // No Bullets or Numbering

        // If NumRuleItems were set, either directly or through inheritance, disable them now
        m_pCurrentColl->SetFormatAttr( SwNumRuleItem() );

        const OUString aName("Outline");
        SwNumRule aNR( m_rDoc.GetUniqueNumRuleName( &aName ),
                       SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
                       OUTLINE_RULE );
        aNR = *m_rDoc.GetOutlineNumRule();

        SetAnld(&aNR, reinterpret_cast<WW8_ANLD const *>(pData), m_nSwNumLevel, true);

        // Missing Levels need not be replenished
        m_rDoc.SetOutlineNumRule( aNR );
    }else if( m_xStyles->mnWwNumLevel == 10 || m_xStyles->mnWwNumLevel == 11 ){
        SwNumRule* pNR = GetStyRule();
        SetAnld(pNR, reinterpret_cast<WW8_ANLD const *>(pData), 0, false);
        m_pCurrentColl->SetFormatAttr( SwNumRuleItem( pNR->GetName() ) );

        pStyInf = GetStyle(m_nCurrentColl);
        if (pStyInf != nullptr)
            pStyInf->m_bHasStyNumRule = true;
    }
}

// Numbering / Bullets

// SetNumOlst() carries the Numrules for this cell to SwNumFormat.
// For this the info is fetched from OLST and not from ANLD ( see later )
// ( only for outline inside text; Bullets / numbering use ANLDs )
void SwWW8ImplReader::SetNumOlst(SwNumRule* pNumR, WW8_OLST* pO, sal_uInt8 nSwLevel)
{
    SwNumFormat aNF;
    WW8_ANLV &rAV = pO->rganlv[nSwLevel];
    SetBaseAnlv(aNF, rAV, nSwLevel);
                                            // ... and then the Strings
    int nTextOfs = 0;
    sal_uInt8 i;
    WW8_ANLV* pAV1;                 // search String-Positions
    for (i = 0, pAV1 = pO->rganlv; i < nSwLevel; ++i, ++pAV1)
        nTextOfs += pAV1->cbTextBefore + pAV1->cbTextAfter;

    if (!m_bVer67)
        nTextOfs *= 2;
    SetAnlvStrings(aNF, rAV, pO->rgch, nTextOfs, SAL_N_ELEMENTS(pO->rgch), true); // and apply
    pNumR->Set(nSwLevel, aNF);
}

// The OLST is at the beginning of each section that contains outlines.
// The ANLDs that are connected to each outline-line contain only nonsense,
// so the OLSTs are remembered for the section to have usable information
// when outline-paragraphs occur.
void SwWW8ImplReader::Read_OLST( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    m_xNumOlst.reset();
    if (nLen <= 0)
        return;

    if (static_cast<size_t>(nLen) < sizeof(WW8_OLST))
    {
        SAL_WARN("sw.ww8", "WW8_OLST property is " << nLen << " long, needs to be at least " << sizeof(WW8_OLST));
        return;
    }

    m_xNumOlst.reset(new WW8_OLST);
    *m_xNumOlst = *reinterpret_cast<WW8_OLST const *>(pData);
}

WW8LvlType GetNumType(sal_uInt8 nWwLevelNo)
{
    WW8LvlType nRet = WW8_None;
    if( nWwLevelNo == 12 )
       nRet = WW8_Pause;
    else if( nWwLevelNo == 10 )
       nRet = WW8_Numbering;
    else if( nWwLevelNo == 11 )
       nRet = WW8_Sequence;
    else if( nWwLevelNo > 0 && nWwLevelNo <= 9 )
       nRet = WW8_Outline;
    return nRet;
}

SwNumRule *ANLDRuleMap::GetNumRule(const SwDoc& rDoc, sal_uInt8 nNumType)
{
    const OUString& rNumRule = WW8_Numbering == nNumType ? msNumberingNumRule : msOutlineNumRule;
    if (rNumRule.isEmpty())
        return nullptr;
    return rDoc.FindNumRulePtr(rNumRule);
}

void ANLDRuleMap::SetNumRule(const OUString& rNumRule, sal_uInt8 nNumType)
{
    if (WW8_Numbering == nNumType)
        msNumberingNumRule = rNumRule;
    else
        msOutlineNumRule = rNumRule;
}

// StartAnl is called at the beginning of a row area that contains
// outline / numbering / bullets
void SwWW8ImplReader::StartAnl(const sal_uInt8* pSprm13)
{
    m_bCurrentAND_fNumberAcross = false;

    sal_uInt8 nT = static_cast< sal_uInt8 >(GetNumType(*pSprm13));
    if (nT == WW8_Pause || nT == WW8_None)
        return;

    m_nWwNumType = nT;
    SwNumRule *pNumRule = m_aANLDRules.GetNumRule(m_rDoc, m_nWwNumType);

    // check for COL numbering:
    SprmResult aS12; // sprmAnld
    OUString sNumRule;

    if (m_xTableDesc)
    {
        sNumRule = m_xTableDesc->GetNumRuleName();
        if (!sNumRule.isEmpty())
        {
            pNumRule = m_rDoc.FindNumRulePtr(sNumRule);
            if (!pNumRule)
                sNumRule.clear();
            else
            {
                // this is ROW numbering ?
                aS12 = m_xPlcxMan->HasParaSprm(m_bVer67 ? 12 : NS_sprm::LN_PAnld); // sprmAnld
                if (aS12.pSprm && aS12.nRemainingData >= sal_Int32(sizeof(WW8_ANLD)) && 0 != reinterpret_cast<WW8_ANLD const *>(aS12.pSprm)->fNumberAcross)
                    sNumRule.clear();
            }
        }
    }

    SwWW8StyInf * pStyInf = GetStyle(m_nCurrentColl);
    if (sNumRule.isEmpty() && pStyInf != nullptr &&  pStyInf->m_bHasStyNumRule)
    {
        sNumRule = pStyInf->m_pFormat->GetNumRule().GetValue();
        pNumRule = m_rDoc.FindNumRulePtr(sNumRule);
        if (!pNumRule)
            sNumRule.clear();
    }

    if (sNumRule.isEmpty())
    {
        if (!pNumRule)
        {
            // #i86652#
            pNumRule = m_rDoc.GetNumRuleTable()[
                            m_rDoc.MakeNumRule( sNumRule, nullptr, false,
                                              SvxNumberFormat::LABEL_ALIGNMENT ) ];
        }
        if (m_xTableDesc)
        {
            if (!aS12.pSprm)
                aS12 = m_xPlcxMan->HasParaSprm(m_bVer67 ? 12 : NS_sprm::LN_PAnld); // sprmAnld
            if (!aS12.pSprm || aS12.nRemainingData < sal_Int32(sizeof(WW8_ANLD)) || !reinterpret_cast<WW8_ANLD const *>(aS12.pSprm)->fNumberAcross)
                m_xTableDesc->SetNumRuleName(pNumRule->GetName());
        }
    }

    m_bAnl = true;

    sNumRule = pNumRule ? pNumRule->GetName() : OUString();
    // set NumRules via stack
    m_xCtrlStck->NewAttr(*m_pPaM->GetPoint(),
        SfxStringItem(RES_FLTR_NUMRULE, sNumRule));

    m_aANLDRules.SetNumRule(sNumRule, m_nWwNumType);
}

// NextAnlLine() is called once for every row of a
// outline / numbering / bullet
void SwWW8ImplReader::NextAnlLine(const sal_uInt8* pSprm13)
{
    if (!m_bAnl)
        return;

    SwNumRule *pNumRule = m_aANLDRules.GetNumRule(m_rDoc, m_nWwNumType);

    // pNd->UpdateNum without a set of rules crashes at the latest whilst storing as sdw3

    // WW:10 = numbering -> SW:0 & WW:11 = bullets -> SW:0
    if (*pSprm13 == 10 || *pSprm13 == 11)
    {
        m_nSwNumLevel = 0;
        if (pNumRule && !pNumRule->GetNumFormat(m_nSwNumLevel))
        {
            // not defined yet
            // sprmAnld o. 0
            SprmResult aS12 = m_xPlcxMan->HasParaSprm(m_bVer67 ? 12 : NS_sprm::LN_PAnld);
            if (aS12.nRemainingData >= sal_Int32(sizeof(WW8_ANLD)))
                SetAnld(pNumRule, reinterpret_cast<WW8_ANLD const *>(aS12.pSprm), m_nSwNumLevel, false);
        }
    }
    else if( *pSprm13 > 0 && *pSprm13 <= MAXLEVEL )          // range WW:1..9 -> SW:0..8
    {
        m_nSwNumLevel = *pSprm13 - 1;             // outline
        // undefined
        if (pNumRule && !pNumRule->GetNumFormat(m_nSwNumLevel))
        {
            if (m_xNumOlst)                       // there was a OLST
            {
                //Assure upper levels are set, #i9556#
                for (sal_uInt8 nI = 0; nI < m_nSwNumLevel; ++nI)
                {
                    if (!pNumRule->GetNumFormat(nI))
                        SetNumOlst(pNumRule, m_xNumOlst.get(), nI);
                }

                SetNumOlst(pNumRule, m_xNumOlst.get(), m_nSwNumLevel);
            }
            else                                // no Olst -> use Anld
            {
                // sprmAnld
                SprmResult aS12 = m_xPlcxMan->HasParaSprm(m_bVer67 ? 12 : NS_sprm::LN_PAnld);
                if (aS12.nRemainingData >= sal_Int32(sizeof(WW8_ANLD)))
                    SetAnld(pNumRule, reinterpret_cast<WW8_ANLD const *>(aS12.pSprm), m_nSwNumLevel, false);
            }
        }
    }
    else
        m_nSwNumLevel = 0xff;                 // no number

    SwTextNode* pNd = m_pPaM->GetNode().GetTextNode();
    if (!pNd)
        return;

    if (m_nSwNumLevel < MAXLEVEL)
        pNd->SetAttrListLevel( m_nSwNumLevel );
    else
    {
        pNd->SetAttrListLevel(0);
        pNd->SetCountedInList( false );
    }
}

void SwWW8ImplReader::StopAllAnl(bool bGoBack)
{
    //Of course we're not restarting, but we'll make use of our knowledge
    //of the implementation to do it.
    StopAnlToRestart(WW8_None, bGoBack);
}

void SwWW8ImplReader::StopAnlToRestart(sal_uInt8 nNewType, bool bGoBack)
{
    if (bGoBack)
    {
        SwPosition aTmpPos(*m_pPaM->GetPoint());
        m_pPaM->Move(fnMoveBackward, GoInContent);
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_FLTR_NUMRULE);
        *m_pPaM->GetPoint() = aTmpPos;
    }
    else
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_FLTR_NUMRULE);

    m_aANLDRules.msNumberingNumRule.clear();
    /*
     #i18816#
     my take on this problem is that moving either way from an outline to a
     numbering doesn't halt the outline, while the numbering is always halted
    */
    bool bNumberingNotStopOutline =
        (((m_nWwNumType == WW8_Outline) && (nNewType == WW8_Numbering)) ||
        ((m_nWwNumType == WW8_Numbering) && (nNewType == WW8_Outline)));
    if (!bNumberingNotStopOutline)
        m_aANLDRules.msOutlineNumRule.clear();

    m_nSwNumLevel = 0xff;
    m_nWwNumType = WW8_None;
    m_bAnl = false;
}

WW8TabBandDesc::WW8TabBandDesc( WW8TabBandDesc const & rBand )
{
    *this = rBand;
    if( rBand.pTCs )
    {
        pTCs = new WW8_TCell[nWwCols];
        memcpy( pTCs, rBand.pTCs, nWwCols * sizeof( WW8_TCell ) );
    }
    if( rBand.pSHDs )
    {
        pSHDs = new WW8_SHD[nWwCols];
        memcpy( pSHDs, rBand.pSHDs, nWwCols * sizeof( WW8_SHD ) );
    }
    if( rBand.pNewSHDs )
    {
        pNewSHDs = new Color[nWwCols];
        memcpy(pNewSHDs, rBand.pNewSHDs, nWwCols * sizeof(Color));
    }
    memcpy(aDefBrcs, rBand.aDefBrcs, sizeof(aDefBrcs));
}

// ReadDef reads the cell position and the borders of a band
void WW8TabBandDesc::ReadDef(bool bVer67, const sal_uInt8* pS, short nLen)
{
    if (!bVer67)
    {
        //the ww8 version of this is unusual in masquerading as a a srpm with a
        //single byte len arg while it really has a word len arg, after this
        //increment nLen is correct to describe the remaining amount of data
        pS++;
    }

    --nLen; //reduce len by expected nCols arg
    if (nLen < 0)
        return;
    sal_uInt8 nCols = *pS;                       // number of cells

    if (nCols > MAX_COL)
        return;

    nLen -= 2 * (nCols + 1); //reduce len by claimed amount of next x-borders arguments
    if (nLen < 0)
        return;

    short nOldCols = nWwCols;
    nWwCols = nCols;

    const sal_uInt8* pT = &pS[1];
    for (int i = 0; i <= nCols; i++, pT+=2)
        nCenter[i] = static_cast<sal_Int16>(SVBT16ToShort( pT ));    // X-borders

    if( nCols != nOldCols ) // different column count
    {
        delete[] pTCs;
        pTCs = nullptr;
        delete[] pSHDs;
        pSHDs = nullptr;
        delete[] pNewSHDs;
        pNewSHDs = nullptr;
    }

    short nFileCols = nLen / ( bVer67 ? 10 : 20 );  // really saved

    if (!pTCs && nCols)
    {
        // create empty TCs
        pTCs = new WW8_TCell[nCols];
        setcelldefaults(pTCs,nCols);
    }

    short nColsToRead = std::min<short>(nFileCols, nCols);

    if (nColsToRead > 0)
    {
        // read TCs

        /*
            Attention: Beginning with Ver8 there is an extra ushort per TC
                       added and the size of the border code is doubled.
                       Because of this a simple copy (pTCs[i] = *pTc;)
                       is not possible.
            ---
            Advantage: The work structure suits better.
        */
        WW8_TCell* pCurrentTC  = pTCs;
        if( bVer67 )
        {
            WW8_TCellVer6 const * pTc = reinterpret_cast<WW8_TCellVer6 const *>(pT);
            for (int i = 0; i < nColsToRead; i++, ++pCurrentTC,++pTc)
            {
                // TC from file ?
                sal_uInt8 aBits1 = pTc->aBits1Ver6;
                pCurrentTC->bFirstMerged = sal_uInt8( ( aBits1 & 0x01 ) != 0 );
                pCurrentTC->bMerged = sal_uInt8( ( aBits1 & 0x02 ) != 0 );
                pCurrentTC->rgbrc[ WW8_TOP ]
                    = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_TOP ] ));
                pCurrentTC->rgbrc[ WW8_LEFT ]
                    = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_LEFT ] ));
                pCurrentTC->rgbrc[ WW8_BOT ]
                    = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_BOT ] ));
                pCurrentTC->rgbrc[ WW8_RIGHT ]
                    = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_RIGHT ] ));
                if(    ( pCurrentTC->bMerged )
                        && ( i > 0             ) )
                {
                    // Cell merged -> remember
                    //bWWMergedVer6[i] = true;
                    pTCs[i-1].rgbrc[ WW8_RIGHT ]
                        = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_RIGHT ] ));
                        // apply right border to previous cell
                        // bExist must not be set to false, because WW
                        // does not count this cells in text boxes....
                }
            }
        }
        else
        {
            WW8_TCellVer8 const * pTc = reinterpret_cast<WW8_TCellVer8 const *>(pT);
            for (int k = 0; k < nColsToRead; ++k, ++pCurrentTC, ++pTc )
            {
                sal_uInt16 aBits1 = SVBT16ToShort( pTc->aBits1Ver8 );
                pCurrentTC->bFirstMerged    = sal_uInt8( ( aBits1 & 0x0001 ) != 0 );
                pCurrentTC->bMerged         = sal_uInt8( ( aBits1 & 0x0002 ) != 0 );
                pCurrentTC->bVertical       = sal_uInt8( ( aBits1 & 0x0004 ) != 0 );
                pCurrentTC->bBackward       = sal_uInt8( ( aBits1 & 0x0008 ) != 0 );
                pCurrentTC->bRotateFont     = sal_uInt8( ( aBits1 & 0x0010 ) != 0 );
                pCurrentTC->bVertMerge      = sal_uInt8( ( aBits1 & 0x0020 ) != 0 );
                pCurrentTC->bVertRestart    = sal_uInt8( ( aBits1 & 0x0040 ) != 0 );
                pCurrentTC->nVertAlign      = ( ( aBits1 & 0x0180 ) >> 7 );
                // note: in aBits1 there are 7 bits unused,
                //       followed by another 16 unused bits

                pCurrentTC->rgbrc[ WW8_TOP   ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_TOP   ]);
                pCurrentTC->rgbrc[ WW8_LEFT  ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_LEFT  ]);
                pCurrentTC->rgbrc[ WW8_BOT   ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_BOT   ]);
                pCurrentTC->rgbrc[ WW8_RIGHT ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_RIGHT ]);
            }
        }

        // #i25071 In '97 text direction appears to be only set using TC properties
        // not with sprmTTextFlow so we need to cycle through the maDirections and
        // double check any non-default directions
        for (int k = 0; k < nCols; ++k)
        {
            if(maDirections[k] == 4)
            {
                if(pTCs[k].bVertical)
                {
                    if(pTCs[k].bBackward)
                        maDirections[k] = 3;
                    else
                        maDirections[k] = 1;
                }
            }
        }
    }
}

void WW8TabBandDesc::ProcessSprmTSetBRC(int nBrcVer, const sal_uInt8* pParamsTSetBRC)
{
    if( !pParamsTSetBRC || !pTCs ) // set one or more cell border(s)
        return;

    sal_uInt8 nitcFirst= pParamsTSetBRC[0];// first col to be changed
    sal_uInt8 nitcLim  = pParamsTSetBRC[1];// (last col to be changed)+1
    sal_uInt8 nFlag    = *(pParamsTSetBRC+2);

    if (nitcFirst >= nWwCols)
        return;

    if (nitcLim > nWwCols)
        nitcLim = nWwCols;

    bool bChangeRight  = (nFlag & 0x08) != 0;
    bool bChangeBottom = (nFlag & 0x04) != 0;
    bool bChangeLeft   = (nFlag & 0x02) != 0;
    bool bChangeTop    = (nFlag & 0x01) != 0;

    WW8_TCell* pCurrentTC  = pTCs + nitcFirst;
    WW8_BRCVer9 brcVer9;
    if( nBrcVer == 6 )
        brcVer9 = WW8_BRCVer9(WW8_BRC(*reinterpret_cast<WW8_BRCVer6 const *>(pParamsTSetBRC+3)));
    else if( nBrcVer == 8 )
        brcVer9 = WW8_BRCVer9(*reinterpret_cast<WW8_BRC const *>(pParamsTSetBRC+3));
    else
        brcVer9 = *reinterpret_cast<WW8_BRCVer9 const *>(pParamsTSetBRC+3);

    for( int i = nitcFirst; i < nitcLim; ++i, ++pCurrentTC )
    {
        if( bChangeTop )
            pCurrentTC->rgbrc[ WW8_TOP   ] = brcVer9;
        if( bChangeLeft )
            pCurrentTC->rgbrc[ WW8_LEFT  ] = brcVer9;
        if( bChangeBottom )
            pCurrentTC->rgbrc[ WW8_BOT   ] = brcVer9;
        if( bChangeRight )
            pCurrentTC->rgbrc[ WW8_RIGHT ] = brcVer9;
    }

}

void WW8TabBandDesc::ProcessSprmTTableBorders(int nBrcVer, const sal_uInt8* pParams, sal_uInt16 nParamsLen)
{
    // sprmTTableBorders
    if( nBrcVer == 6 )
    {
        if (nParamsLen < sizeof(WW8_BRCVer6) * 6)
        {
            SAL_WARN("sw.ww8", "table border property is too short");
            return;
        }
        WW8_BRCVer6 const *pVer6 = reinterpret_cast<WW8_BRCVer6 const *>(pParams);
        for (int i = 0; i < 6; ++i)
            aDefBrcs[i] = WW8_BRCVer9(WW8_BRC(pVer6[i]));
    }
    else if ( nBrcVer == 8 )
    {
        static_assert(sizeof (WW8_BRC) == 4, "this has to match the msword size");
        if (nParamsLen < sizeof(WW8_BRC) * 6)
        {
            SAL_WARN("sw.ww8", "table border property is too short");
            return;
        }
        for( int i = 0; i < 6; ++i )
            aDefBrcs[i] = WW8_BRCVer9(reinterpret_cast<WW8_BRC const *>(pParams)[i]);
    }
    else
    {
        if (nParamsLen < sizeof( aDefBrcs ))
        {
            SAL_WARN("sw.ww8", "table border property is too short");
            return;
        }
        memcpy( aDefBrcs, pParams, sizeof( aDefBrcs ) );
    }
}

void WW8TabBandDesc::ProcessSprmTDxaCol(const sal_uInt8* pParamsTDxaCol)
{
    // sprmTDxaCol (opcode 0x7623) changes the width of cells
    // whose index is within a certain range to be a certain value.

    if( nWwCols && pParamsTDxaCol ) // set one or more cell length(s)
    {
        sal_uInt8 nitcFirst= pParamsTDxaCol[0]; // first col to be changed
        sal_uInt8 nitcLim  = pParamsTDxaCol[1]; // (last col to be changed)+1
        short nDxaCol = static_cast<sal_Int16>(SVBT16ToShort( pParamsTDxaCol + 2 ));

        for( int i = nitcFirst; (i < nitcLim) && (i < nWwCols); i++ )
        {
            const short nOrgWidth  = nCenter[i+1] - nCenter[i];
            const short nDelta = nDxaCol - nOrgWidth;
            for( int j = i+1; j <= nWwCols; j++ )
            {
                nCenter[j] = nCenter[j] + nDelta;
            }
        }
    }
}

void WW8TabBandDesc::ProcessSprmTInsert(const sal_uInt8* pParamsTInsert)
{
    if( !nWwCols || !pParamsTInsert )        // set one or more cell length(s)
        return;

    sal_uInt8 nitcInsert = pParamsTInsert[0]; // position at which to insert
    if (nitcInsert >= MAX_COL)  // cannot insert into cell outside max possible index
        return;
    sal_uInt8 nctc  = pParamsTInsert[1];      // number of cells
    sal_uInt16 ndxaCol = SVBT16ToShort( pParamsTInsert+2 );

    short nNewWwCols;
    if (nitcInsert > nWwCols)
    {
        nNewWwCols = nitcInsert+nctc;
        //if new count would be outside max possible count, clip it, and calc a new replacement
        //legal nctc
        if (nNewWwCols > MAX_COL)
        {
            nNewWwCols = MAX_COL;
            nctc = ::sal::static_int_cast<sal_uInt8>(nNewWwCols-nitcInsert);
        }
    }
    else
    {
        nNewWwCols = nWwCols+nctc;
        //if new count would be outside max possible count, clip it, and calc a new replacement
        //legal nctc
        if (nNewWwCols > MAX_COL)
        {
            nNewWwCols = MAX_COL;
            nctc = ::sal::static_int_cast<sal_uInt8>(nNewWwCols-nWwCols);
        }
    }

    WW8_TCell *pTC2s = new WW8_TCell[nNewWwCols];
    setcelldefaults(pTC2s, nNewWwCols);

    if (pTCs)
    {
        memcpy( pTC2s, pTCs, nWwCols * sizeof( WW8_TCell ) );
        delete[] pTCs;
    }
    pTCs = pTC2s;

    //If we have to move some cells
    if (nitcInsert <= nWwCols)
    {
        // adjust the left x-position of the dummy at the very end
        nCenter[nWwCols + nctc] = nCenter[nWwCols]+nctc*ndxaCol;
        for( int i = nWwCols-1; i >= nitcInsert; i--)
        {
            // adjust the left x-position
            nCenter[i + nctc] = nCenter[i]+nctc*ndxaCol;

            // adjust the cell's borders
            pTCs[i + nctc] = pTCs[i];
        }
    }

    //if itcMac is larger than full size, fill in missing ones first
    for( int i = nWwCols; i > nitcInsert+nWwCols; i--)
        nCenter[i] = i ? (nCenter[i - 1]+ndxaCol) : 0;

    //now add in our new cells
    for( int j = 0;j < nctc; j++)
        nCenter[j + nitcInsert] = (j + nitcInsert) ? (nCenter[j + nitcInsert -1]+ndxaCol) : 0;

    nWwCols = nNewWwCols;

}

void WW8TabBandDesc::ProcessDirection(const sal_uInt8* pParams)
{
    sal_uInt8 nStartCell = *pParams++;
    sal_uInt8 nEndCell = *pParams++;
    sal_uInt16 nCode = SVBT16ToShort(pParams);

    OSL_ENSURE(nStartCell < nEndCell, "not as I thought");
    OSL_ENSURE(nEndCell < MAX_COL + 1, "not as I thought");
    if (nStartCell > MAX_COL)
        return;
    if (nEndCell > MAX_COL + 1)
        nEndCell = MAX_COL + 1;

    for (;nStartCell < nEndCell; ++nStartCell)
        maDirections[nStartCell] = nCode;
}

void WW8TabBandDesc::ProcessSpacing(const sal_uInt8* pParams)
{
    sal_uInt8 nLen = pParams ? *(pParams - 1) : 0;
    OSL_ENSURE(nLen == 6, "Unexpected spacing len");
    if (nLen != 6)
        return;
    mbHasSpacing=true;
#if OSL_DEBUG_LEVEL > 0
    sal_uInt8 nWhichCell = *pParams;
    OSL_ENSURE(nWhichCell == 0, "Expected cell to be 0!");
#endif
    ++pParams; //Skip which cell
    ++pParams; //unknown byte

    sal_uInt8 nSideBits = *pParams++;
    OSL_ENSURE(nSideBits < 0x10, "Unexpected value for nSideBits");
    ++pParams; //unknown byte
    sal_uInt16 nValue =  SVBT16ToShort( pParams );
    for (int i = wwTOP; i <= wwRIGHT; i++)
    {
        switch (nSideBits & (1 << i))
        {
            case 1 << wwTOP:
                mnDefaultTop = nValue;
                break;
            case 1 << wwLEFT:
                mnDefaultLeft = nValue;
                break;
            case 1 << wwBOTTOM:
                mnDefaultBottom = nValue;
                break;
            case 1 << wwRIGHT:
                mnDefaultRight = nValue;
                break;
            case 0:
                break;
            default:
                OSL_ENSURE(false, "Impossible");
                break;
        }
    }
}

void WW8TabBandDesc::ProcessSpecificSpacing(const sal_uInt8* pParams)
{
    sal_uInt8 nLen = pParams ? *(pParams - 1) : 0;
    OSL_ENSURE(nLen == 6, "Unexpected spacing len");
    if (nLen != 6)
        return;
    sal_uInt8 nWhichCell = *pParams++;
    OSL_ENSURE(nWhichCell < MAX_COL + 1, "Cell out of range in spacings");
    if (nWhichCell >= MAX_COL + 1)
        return;

    ++pParams; //unknown byte
    sal_uInt8 nSideBits = *pParams++;
    OSL_ENSURE(nSideBits < 0x10, "Unexpected value for nSideBits");
    nOverrideSpacing[nWhichCell] |= nSideBits;

    OSL_ENSURE(nOverrideSpacing[nWhichCell] < 0x10,
        "Unexpected value for nSideBits");
#if OSL_DEBUG_LEVEL > 0
    sal_uInt8 nUnknown2 = *pParams;
    OSL_ENSURE(nUnknown2 == 0x3, "Unexpected value for spacing2");
#endif
    ++pParams;
    sal_uInt16 nValue =  SVBT16ToShort( pParams );

    for (int i=0; i < 4; i++)
    {
        if (nSideBits & (1 << i))
            nOverrideValues[nWhichCell][i] = nValue;
    }
}

void WW8TabBandDesc::ProcessSprmTDelete(const sal_uInt8* pParamsTDelete)
{
    if( nWwCols && pParamsTDelete )        // set one or more cell length(s)
    {
        sal_uInt8 nitcFirst= pParamsTDelete[0]; // first col to be deleted
        if (nitcFirst >= nWwCols) // first index to delete from doesn't exist
            return;
        sal_uInt8 nitcLim  = pParamsTDelete[1]; // (last col to be deleted)+1
        if (nitcLim <= nitcFirst) // second index to delete to is not greater than first index
            return;

        /*
         * sprmTDelete causes any rgdxaCenter and rgtc entries whose index is
         * greater than or equal to itcLim to be moved
         */
        int nShlCnt  = nWwCols - nitcLim; // count of cells to be shifted

        if (nShlCnt >= 0) //There exist entries whose index is greater than or equal to itcLim
        {
            WW8_TCell* pCurrentTC  = pTCs + nitcFirst;
            int i = 0;
            while( i < nShlCnt )
            {
                // adjust the left x-position
                nCenter[nitcFirst + i] = nCenter[nitcLim + i];

                // adjust the cell's borders
                *pCurrentTC = pTCs[ nitcLim + i];

                ++i;
                ++pCurrentTC;
            }
            // adjust the left x-position of the dummy at the very end
            nCenter[nitcFirst + i] = nCenter[nitcLim + i];
        }

        short nCellsDeleted = nitcLim - nitcFirst;
        //clip delete request to available number of cells
        if (nCellsDeleted > nWwCols)
            nCellsDeleted = nWwCols;
        nWwCols -= nCellsDeleted;
    }
}

// ReadShd reads the background color of a cell
// ReadDef must be called before
void WW8TabBandDesc::ReadShd(const sal_uInt8* pS )
{
    sal_uInt8 nLen = pS ? *(pS - 1) : 0;
    if( !nLen )
        return;

    if( !pSHDs )
    {
        pSHDs = new WW8_SHD[nWwCols];
    }

    short nCount = nLen >> 1;
    if (nCount > nWwCols)
        nCount = nWwCols;

    SVBT16 const * pShd;
    int i;
    for(i=0, pShd = reinterpret_cast<SVBT16 const *>(pS); i<nCount; i++, pShd++ )
        pSHDs[i].SetWWValue( *pShd );
}

void WW8TabBandDesc::ReadNewShd(const sal_uInt8* pS, bool bVer67)
{
    sal_uInt8 nLen = pS ? *(pS - 1) : 0;
    if (!nLen)
        return;

    if (!pNewSHDs)
        pNewSHDs = new Color[nWwCols];

    short nCount = nLen / 10; //10 bytes each
    if (nCount > nWwCols)
        nCount = nWwCols;

    int i=0;
    while (i < nCount)
        pNewSHDs[i++] = SwWW8ImplReader::ExtractColour(pS, bVer67);

    while (i < nWwCols)
        pNewSHDs[i++] = COL_AUTO;
}

void WW8TabBandDesc::setcelldefaults(WW8_TCell *pCells, short nCols)
{
    memset(static_cast<void*>(pCells), 0, nCols * sizeof(WW8_TCell));
}

namespace
{
    SprmResult HasTabCellSprm(WW8PLCFx_Cp_FKP* pPap, bool bVer67)
    {
        if (bVer67)
            return pPap->HasSprm(24);
        SprmResult aRes = pPap->HasSprm(0x244B);
        if (aRes.pSprm == nullptr)
            aRes = pPap->HasSprm(0x2416);
        return aRes;
    }
}

enum wwTableSprm
{
    sprmNil,

    sprmTTableWidth, sprmTTextFlow, sprmTFCantSplit, sprmTJc, sprmTFBiDi,
    sprmTDefTable, sprmTDyaRowHeight, sprmTDefTableShd, sprmTDxaLeft,
    sprmTSetBrc, sprmTSetBrc90, sprmTDxaCol, sprmTInsert, sprmTDelete,
    sprmTTableHeader, sprmTDxaGapHalf, sprmTTableBorders, sprmTTableBorders90,
    sprmTDefTableNewShd, sprmTCellPadding, sprmTCellPaddingDefault
};

wwTableSprm GetTableSprm(sal_uInt16 nId, ww::WordVersion eVer)
{
    switch (eVer)
    {
        case ww::eWW8:
            switch (nId)
            {
                case NS_sprm::sprmTTableWidth:
                    return sprmTTableWidth;
                case NS_sprm::sprmTTextFlow:
                    return sprmTTextFlow;
                case NS_sprm::sprmTTableHeader:
                    return sprmTTableHeader;
                case NS_sprm::sprmTFCantSplit:
                    return sprmTFCantSplit;
                case NS_sprm::sprmTJc90:
                    return sprmTJc;
                case NS_sprm::sprmTFBiDi:
                    return sprmTFBiDi;
                case NS_sprm::sprmTDelete:
                    return sprmTDelete;
                case NS_sprm::sprmTInsert:
                    return sprmTInsert;
                case NS_sprm::sprmTDxaCol:
                    return sprmTDxaCol;
                case NS_sprm::sprmTDyaRowHeight:
                    return sprmTDyaRowHeight;
                case NS_sprm::sprmTDxaLeft:
                    return sprmTDxaLeft;
                case NS_sprm::sprmTDxaGapHalf:
                    return sprmTDxaGapHalf;
                case NS_sprm::sprmTTableBorders80:
                    return sprmTTableBorders;
                case NS_sprm::sprmTDefTable:
                    return sprmTDefTable;
                case NS_sprm::sprmTDefTableShd80:
                    return sprmTDefTableShd;
                case NS_sprm::sprmTDefTableShd:
                    return sprmTDefTableNewShd;
                case NS_sprm::sprmTTableBorders:
                    return sprmTTableBorders90;
                case NS_sprm::sprmTSetBrc80:
                    return sprmTSetBrc;
                case NS_sprm::sprmTSetBrc:
                    return sprmTSetBrc90;
                case NS_sprm::sprmTCellPadding:
                    return sprmTCellPadding;
                case NS_sprm::sprmTCellPaddingDefault:
                    return sprmTCellPaddingDefault;
            }
            break;
        case ww::eWW7:
        case ww::eWW6:
            switch (nId)
            {
                case 182:
                    return sprmTJc;
                case 183:
                    return sprmTDxaLeft;
                case 184:
                    return sprmTDxaGapHalf;
                case 186:
                    return sprmTTableHeader;
                case 187:
                    return sprmTTableBorders;
                case 189:
                    return sprmTDyaRowHeight;
                case 190:
                    return sprmTDefTable;
                case 191:
                    return sprmTDefTableShd;
                case 193:
                    return sprmTSetBrc;
                case 194:
                    return sprmTInsert;
                case 195:
                    return sprmTDelete;
                case 196:
                    return sprmTDxaCol;
            }
            break;
        case ww::eWW1:
        case ww::eWW2:
            switch (nId)
            {
                case 146:
                    return sprmTJc;
                case 147:
                    return sprmTDxaLeft;
                case 148:
                    return sprmTDxaGapHalf;
                case 153:
                    return sprmTDyaRowHeight;
                case 154:
                    return sprmTDefTable;
                case 155:
                    return sprmTDefTableShd;
                case 157:
                    return sprmTSetBrc;
                case 158:
                    return sprmTInsert;
                case 159:
                    return sprmTDelete;
                case 160:
                    return sprmTDxaCol;
            }
            break;
    }
    return sprmNil;
}

WW8TabDesc::WW8TabDesc(SwWW8ImplReader* pIoClass, WW8_CP nStartCp) :
    m_pIo(pIoClass),
    m_pFirstBand(nullptr),
    m_pActBand(nullptr),
    m_pTableNd(nullptr),
    m_pTabLines(nullptr),
    m_pTabLine(nullptr),
    m_pTabBoxes(nullptr),
    m_pTabBox(nullptr),
    m_pCurrentWWCell(nullptr),
    m_nRows(0),
    m_nDefaultSwCols(0),
    m_nBands(0),
    m_nMinLeft(0),
    m_nConvertedLeft(0),
    m_nMaxRight(0),
    m_nSwWidth(0),
    m_nPreferredWidth(0),
    m_nPercentWidth(0),
    m_bOk(true),
    m_bClaimLineFormat(false),
    m_eOri(text::HoriOrientation::NONE),
    m_bIsBiDi(false),
    m_nCurrentRow(0),
    m_nCurrentBandRow(0),
    m_nCurrentCol(0),
    m_nRowsToRepeat(0),
    m_pTable(nullptr),
    m_pParentPos(nullptr),
    m_pFlyFormat(nullptr),
    m_aItemSet(m_pIo->m_rDoc.GetAttrPool(),svl::Items<RES_FRMATR_BEGIN,RES_FRMATR_END-1>{})
{
    m_pIo->m_bCurrentAND_fNumberAcross = false;

    static const sal_Int16 aOriArr[] =
    {
        text::HoriOrientation::LEFT, text::HoriOrientation::CENTER, text::HoriOrientation::RIGHT, text::HoriOrientation::CENTER
    };

    bool bOldVer = ww::IsSevenMinus(m_pIo->GetFib().GetFIBVersion());
    WW8_TablePos aTabPos;

    WW8PLCFxSave1 aSave;
    m_pIo->m_xPlcxMan->GetPap()->Save( aSave );

    WW8PLCFx_Cp_FKP* pPap = m_pIo->m_xPlcxMan->GetPapPLCF();

    m_eOri = text::HoriOrientation::LEFT;

    WW8TabBandDesc* pNewBand = new WW8TabBandDesc;

    wwSprmParser aSprmParser(m_pIo->GetFib());

    // process pPap until end of table found
    do
    {
        short nTabeDxaNew      = SHRT_MAX;
        bool bTabRowJustRead   = false;
        const sal_uInt8* pShadeSprm = nullptr;
        const sal_uInt8* pNewShadeSprm = nullptr;
        const sal_uInt8* pTableBorders = nullptr;
        sal_uInt16 nTableBordersLen = 0;
        const sal_uInt8* pTableBorders90 = nullptr;
        sal_uInt16 nTableBorders90Len = 0;
        std::vector<const sal_uInt8*> aTSetBrcs, aTSetBrc90s;
        WW8_TablePos *pTabPos  = nullptr;

        // search end of a tab row
        if(!(m_pIo->SearchRowEnd(pPap, nStartCp, m_pIo->m_nInTable)))
        {
            m_bOk = false;
            break;
        }

        // Get the SPRM chains:
        // first from PAP and then from PCD (of the Piece Table)
        WW8PLCFxDesc aDesc;
        pPap->GetSprms( &aDesc );
        WW8SprmIter aSprmIter(aDesc.pMemPos, aDesc.nSprmsLen, aSprmParser);

        for (int nLoop = 0; nLoop < 2; ++nLoop)
        {
            const sal_uInt8* pParams;
            while (aSprmIter.GetSprms() && nullptr != (pParams = aSprmIter.GetCurrentParams()))
            {
                sal_uInt16 nId = aSprmIter.GetCurrentId();
                sal_uInt16 nFixedLen = aSprmParser.DistanceToData(nId);
                sal_uInt16 nL = aSprmParser.GetSprmSize(nId, aSprmIter.GetSprms(), aSprmIter.GetRemLen());
                sal_uInt16 nLen = nL - nFixedLen;
                wwTableSprm eSprm = GetTableSprm(nId, m_pIo->GetFib().GetFIBVersion());
                switch (eSprm)
                {
                    case sprmTTableWidth:
                        {
                        const sal_uInt8 b0 = pParams[0];
                        const sal_uInt8 b1 = pParams[1];
                        const sal_uInt8 b2 = pParams[2];
                        if (b0 == 3) // Twips
                            m_nPreferredWidth = b2 * 0x100 + b1;
                        else if (b0 == 2) // percent in fiftieths of a percent
                        {
                            m_nPercentWidth = (b2 * 0x100 + b1);
                            // MS documentation: non-negative, and 600% max
                            if ( m_nPercentWidth >= 0 && m_nPercentWidth <= 30000 )
                                m_nPercentWidth *= .02;
                            else
                                m_nPercentWidth = 100;
                        }
                        }
                        break;
                    case sprmTTextFlow:
                        pNewBand->ProcessDirection(pParams);
                        break;
                    case sprmTFCantSplit:
                        pNewBand->bCantSplit = *pParams;
                        m_bClaimLineFormat = true;
                        break;
                    case sprmTTableBorders:
                        pTableBorders = pParams; // process at end
                        nTableBordersLen = nLen;
                        break;
                    case sprmTTableBorders90:
                        pTableBorders90 = pParams; // process at end
                        nTableBorders90Len = nLen;
                        break;
                    case sprmTTableHeader:
                        // tdf#105570
                        if ( m_nRowsToRepeat == m_nRows )
                            m_nRowsToRepeat = (m_nRows + 1);
                        break;
                    case sprmTJc:
                        // sprmTJc  -  Justification Code
                        if (m_nRows == 0)
                            m_eOri = aOriArr[*pParams & 0x3];
                        break;
                    case sprmTFBiDi:
                        m_bIsBiDi = SVBT16ToShort(pParams) != 0;
                        break;
                    case sprmTDxaGapHalf:
                        pNewBand->nGapHalf = static_cast<sal_Int16>(SVBT16ToShort( pParams ));
                        break;
                    case sprmTDyaRowHeight:
                        pNewBand->nLineHeight = static_cast<sal_Int16>(SVBT16ToShort( pParams ));
                        m_bClaimLineFormat = true;
                        break;
                    case sprmTDefTable:
                        pNewBand->ReadDef(bOldVer, pParams, nLen);
                        bTabRowJustRead = true;
                        break;
                    case sprmTDefTableShd:
                        pShadeSprm = pParams;
                        break;
                    case sprmTDefTableNewShd:
                        pNewShadeSprm = pParams;
                        break;
                    case sprmTDxaLeft:
                        // our Writer cannot shift single table lines
                        // horizontally so we have to find the smallest
                        // parameter (meaning the left-most position) and then
                        // shift the whole table to that margin (see below)
                        {
                            short nDxaNew = static_cast<sal_Int16>(SVBT16ToShort( pParams ));
                            if( nDxaNew < nTabeDxaNew )
                                nTabeDxaNew = nDxaNew;
                        }
                        break;
                    case sprmTSetBrc:
                        aTSetBrcs.push_back(pParams); // process at end
                        break;
                    case sprmTSetBrc90:
                        aTSetBrc90s.push_back(pParams); // process at end
                        break;
                    case sprmTDxaCol:
                        pNewBand->ProcessSprmTDxaCol(pParams);
                        break;
                    case sprmTInsert:
                        pNewBand->ProcessSprmTInsert(pParams);
                        break;
                    case sprmTDelete:
                        pNewBand->ProcessSprmTDelete(pParams);
                        break;
                    case sprmTCellPaddingDefault:
                        pNewBand->ProcessSpacing(pParams);
                        break;
                    case sprmTCellPadding:
                        pNewBand->ProcessSpecificSpacing(pParams);
                        break;
                    default:
                        ;
                }
                aSprmIter.advance();
            }

            if( !nLoop )
            {
                pPap->GetPCDSprms(  aDesc );
                aSprmIter.SetSprms( aDesc.pMemPos, aDesc.nSprmsLen );
            }
        }

        // WW-Tables can contain Fly-changes. For this abort tables here
        // and start again. *pPap is still before TabRowEnd, so TestApo()
        // can be called with the last parameter set to false and therefore
        // take effect.

        if (bTabRowJustRead)
        {
            // Some SPRMs need to be processed *after* ReadDef is called
            // so they were saved up until here
            if (pShadeSprm)
                pNewBand->ReadShd(pShadeSprm);
            if (pNewShadeSprm)
                pNewBand->ReadNewShd(pNewShadeSprm, bOldVer);
            if (pTableBorders90)
                pNewBand->ProcessSprmTTableBorders(9, pTableBorders90, nTableBorders90Len);
            else if (pTableBorders)
                pNewBand->ProcessSprmTTableBorders(bOldVer ? 6 : 8,
                    pTableBorders, nTableBordersLen);
            std::vector<const sal_uInt8*>::const_iterator iter;
            for (iter = aTSetBrcs.begin(); iter != aTSetBrcs.end(); ++iter)
                pNewBand->ProcessSprmTSetBRC(bOldVer ? 6 : 8, *iter);
            for (iter = aTSetBrc90s.begin(); iter != aTSetBrc90s.end(); ++iter)
                pNewBand->ProcessSprmTSetBRC(9, *iter);
        }

        if( nTabeDxaNew < SHRT_MAX )
        {
            short* pCenter  = pNewBand->nCenter;
            short firstDxaCenter = *pCenter;
            for( int i = 0; i < pNewBand->nWwCols; i++, ++pCenter )
            {
                // #i30298# Use sprmTDxaLeft to adjust the left indent
                // #i40461# Use dxaGapHalf during calculation
                *pCenter +=
                    (nTabeDxaNew - (firstDxaCenter + pNewBand->nGapHalf));
            }
        }

        if (!m_pActBand)
            m_pActBand = m_pFirstBand = pNewBand;
        else
        {
            m_pActBand->pNextBand = pNewBand;
            m_pActBand = pNewBand;
        }
        m_nBands++;

        pNewBand = new WW8TabBandDesc;

        m_nRows++;
        m_pActBand->nRows++;

        //Seek our pap to its next block of properties
        WW8PLCFxDesc aRes;
        aRes.pMemPos = nullptr;
        aRes.nStartPos = nStartCp;

        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);

        //Are we at the end of available properties
        if (
             !pPap->HasFkp() || pPap->Where() == WW8_CP_MAX ||
             aRes.nStartPos == WW8_CP_MAX
           )
        {
            m_bOk = false;
            break;
        }

        //Are we still in a table cell
        SprmResult aParamsRes = HasTabCellSprm(pPap, bOldVer);
        const sal_uInt8* pParams = aParamsRes.pSprm;
        SprmResult aLevelRes = pPap->HasSprm(0x6649);
        const sal_uInt8 *pLevel = aLevelRes.pSprm;
        // InTable
        if (!pParams || aParamsRes.nRemainingData < 1 || (1 != *pParams) ||
            (pLevel && aLevelRes.nRemainingData >= 1 && (*pLevel <= m_pIo->m_nInTable)))
        {
            break;
        }

        //Get the end of row new table positioning data
        WW8_CP nMyStartCp=nStartCp;
        if (m_pIo->SearchRowEnd(pPap, nMyStartCp, m_pIo->m_nInTable))
            if (m_pIo->ParseTabPos(&aTabPos, pPap))
                pTabPos = &aTabPos;

        //Move back to this cell
        aRes.pMemPos = nullptr;
        aRes.nStartPos = nStartCp;

        // PlcxMan currently points too far ahead so we need to bring
        // it back to where we are trying to make a table
        m_pIo->m_xPlcxMan->GetPap()->nOrigStartPos = aRes.nStartPos;
        m_pIo->m_xPlcxMan->GetPap()->nCpOfs = aRes.nCpOfs;
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);

        //Does this row match up with the last row closely enough to be
        //considered part of the same table
        ApoTestResults aApo = m_pIo->TestApo(m_pIo->m_nInTable + 1, false, pTabPos);

        /*
        ##513##, #79474# If this is not sufficient, then we should look at
        sprmPD{y|x}aAbs as our indicator that the following set of rows is not
        part of this table, but instead is an absolutely positioned table
        outside of this one
        */
        if (aApo.mbStopApo)
            break;
        if (aApo.mbStartApo)
        {
            //if there really is a fly here, and not a "null" fly then break.
            WW8FlyPara *pNewFly = m_pIo->ConstructApo(aApo, pTabPos);
            if (pNewFly)
                delete pNewFly;
            else
                break;
        }

        nStartCp = aRes.nEndPos;
    }
    while(true);

    if( m_bOk )
    {
        if( m_pActBand->nRows > 1 )
        {
            // last band has more than 1 cell
            delete pNewBand;
            pNewBand = new WW8TabBandDesc( *m_pActBand ); // create new
            m_pActBand->nRows--;      // because of special treatment of border defaults
            pNewBand->nRows = 1;
            m_pActBand->pNextBand = pNewBand; // append at the end
            m_nBands++;
            pNewBand = nullptr;               // do not delete
        }
        CalcDefaults();
    }
    delete pNewBand;

    m_pIo->m_xPlcxMan->GetPap()->Restore( aSave );
}

WW8TabDesc::~WW8TabDesc()
{
    WW8TabBandDesc* pR = m_pFirstBand;
    while(pR)
    {
        WW8TabBandDesc* pR2 = pR->pNextBand;
        delete pR;
        pR = pR2;
    }

    delete m_pParentPos;
}

void WW8TabDesc::CalcDefaults()
{
    short nMinCols = SHRT_MAX;
    WW8TabBandDesc* pR;

    m_nMinLeft = SHRT_MAX;
    m_nMaxRight = SHRT_MIN;

    /*
    If we are an honestly inline centered table, then the normal rules of
    engagement for left and right margins do not apply. The multiple rows are
    centered regardless of the actual placement of rows, so we cannot have
    mismatched rows as is possible in other configurations.

    e.g. change the example bugdoc in word from text wrapping of none (inline)
    to around (in frame (bApo)) and the table splits into two very disjoint
    rows as the beginning point of each row are very different
    */
    if ((!m_pIo->InLocalApo()) && (m_eOri == text::HoriOrientation::CENTER))
    {
        for (pR = m_pFirstBand; pR; pR = pR->pNextBand)
            for( short i = pR->nWwCols; i >= 0; --i)
                pR->nCenter[i] = pR->nCenter[i] -  pR->nCenter[0];
    }

    // First loop: find outermost L and R borders
    for( pR = m_pFirstBand; pR; pR = pR->pNextBand )
    {
        if( pR->nCenter[0] < m_nMinLeft )
            m_nMinLeft = pR->nCenter[0];

        // Following adjustment moves a border and then uses it to find width
        // of next cell, so collect current widths, to avoid situation when width
        // adjustment to too narrow cell makes next cell have negative width
        short nOrigWidth[MAX_COL + 1];
        for( short i = 0; i < pR->nWwCols; i++ )
        {
            nOrigWidth[i] = pR->nCenter[i+1] - pR->nCenter[i];
        }

        for( short i = 0; i < pR->nWwCols; i++ )
        {
           /*
            If the margins are so large as to make the displayable
            area inside them smaller than the minimum allowed then adjust the
            width to fit. But only do it if the two cells are not the exact
            same value, if they are then the cell does not really exist and will
            be blended together into the same cell through the use of the
            nTrans(late) array.
            #i28333# If the nGapHalf is greater than the cell width best to ignore it
            */
            int nCellWidth = pR->nCenter[i+1] - pR->nCenter[i];
            if (nCellWidth != nOrigWidth[i])
            {
                if (nOrigWidth[i] == 0)
                    nCellWidth = 0; // restore zero-width "cell"
                else if ((pR->nGapHalf >= nCellWidth) && (pR->nGapHalf < nOrigWidth[i]))
                    nCellWidth = pR->nGapHalf + 1; // avoid false ignore
                else if ((nCellWidth <= 0) && (nOrigWidth[i] > 0))
                    nCellWidth = 1; // minimal non-zero width to minimize distortion
            }
            if (nCellWidth && ((nCellWidth - pR->nGapHalf*2) < MINLAY) && pR->nGapHalf < nCellWidth)
            {
                nCellWidth = MINLAY + pR->nGapHalf * 2;
            }
            pR->nCenter[i + 1] = pR->nCenter[i] + nCellWidth;
        }

        if( pR->nCenter[pR->nWwCols] > m_nMaxRight )
            m_nMaxRight = pR->nCenter[pR->nWwCols];
    }
    m_nSwWidth = m_nMaxRight - m_nMinLeft;

    // If the table is right aligned we need to align all rows to the
    // row that has the furthest right point

    if(m_eOri == text::HoriOrientation::RIGHT)
    {
        for( pR = m_pFirstBand; pR; pR = pR->pNextBand )
        {
            int adjust = m_nMaxRight - pR->nCenter[pR->nWwCols];
            for( short i = 0; i < pR->nWwCols + 1; i++ )
            {
                pR->nCenter[i] = static_cast< short >(pR->nCenter[i] + adjust);
            }

        }
    }

    // 2. pass: Detect number of writer columns. This can exceed the count
    // of columns in WW by 2, because SW in contrast to WW does not provide
    // fringed left and right borders and has to fill with empty boxes.
    // Non existent cells can reduce the number of columns.

    // 3. pass: Replace border with defaults if needed
    m_nConvertedLeft = m_nMinLeft;

    short nLeftMaxThickness = 0, nRightMaxThickness=0;
    for( pR = m_pFirstBand ; pR; pR = pR->pNextBand )
    {
        if( !pR->pTCs )
        {
            pR->pTCs = new WW8_TCell[ pR->nWwCols ];
            WW8TabBandDesc::setcelldefaults(pR->pTCs, pR->nWwCols);
        }
        for (int k = 0; k < pR->nWwCols; ++k)
        {
            WW8_TCell& rT = pR->pTCs[k];
            for (int i = 0; i < 4; ++i)
            {
                if (rT.rgbrc[i].brcType()==0)
                {
                    // if shadow is set, its invalid
                    int j = i;
                    switch( i )
                    {
                    case 0:
                        // outer top / horizontally inside
                        j = (pR == m_pFirstBand) ? 0 : 4;
                        break;
                    case 1:
                        // outer left / vertically inside
                        j = k ? 5 : 1;
                        break;
                    case 2:
                        // outer bottom  / horizontally inside
                        j = pR->pNextBand ? 4 : 2;
                        break;
                    case 3:
                        // outer right / vertically inside
                        j = (k == pR->nWwCols - 1) ? 3 : 5;
                        break;
                    }
                    // merge with above defaults
                    rT.rgbrc[i] = pR->aDefBrcs[j];
                }
            }
        }
        if (pR->nWwCols)
        {
            /*
            Similar to graphics and other elements word does not totally
            factor the width of the border into its calculations of size, we
            do so we must adjust out widths and other dimensions to fit.  It
            appears that what occurs is that the last cell's right margin if
            the margin width that is not calculated into winwords table
            dimensions, so in that case increase the table to include the
            extra width of the right margin.
            */
            if ( ! pR->pTCs[pR->nWwCols-1].rgbrc[3].fShadow() )
            {
                short nThickness = pR->pTCs[pR->nWwCols-1].rgbrc[3].
                    DetermineBorderProperties();
                pR->nCenter[pR->nWwCols] = pR->nCenter[pR->nWwCols] + nThickness;
                if (nThickness > nRightMaxThickness)
                    nRightMaxThickness = nThickness;
            }

            /*
            The left space of the table is in nMinLeft, but again this
            does not consider the margin thickness to its left in the
            placement value, so get the thickness of the left border,
            half is placed to the left of the nominal left side, and
            half to the right.
            */
            if ( ! pR->pTCs[0].rgbrc[1].fShadow() )
            {
                short nThickness = pR->pTCs[0].rgbrc[1].
                    DetermineBorderProperties();
                if (nThickness > nLeftMaxThickness)
                    nLeftMaxThickness = nThickness;
            }
        }
    }
    m_nSwWidth = m_nSwWidth + nRightMaxThickness;
    m_nMaxRight = m_nMaxRight + nRightMaxThickness;
    m_nConvertedLeft = m_nMinLeft-(nLeftMaxThickness/2);

    for( pR = m_pFirstBand; pR; pR = pR->pNextBand )
    {
        pR->nSwCols = pR->nWwCols;
        pR->bLEmptyCol = pR->nCenter[0] - m_nMinLeft >= MINLAY;
        pR->bREmptyCol = (m_nMaxRight - pR->nCenter[pR->nWwCols] - nRightMaxThickness) >= MINLAY;

        short nAddCols = short(pR->bLEmptyCol) + short(pR->bREmptyCol);
        sal_uInt16 i;
        sal_uInt16 j = ( pR->bLEmptyCol ) ? 1 : 0;
        for (i = 0; i < pR->nWwCols; ++i)
        {
            pR->nTransCell[i] = static_cast<sal_Int8>(j);
            if ( pR->nCenter[i] < pR->nCenter[i+1] )
            {
                pR->bExist[i] = true;
                j++;
            }
            else
            {
                pR->bExist[i] = false;
                nAddCols--;
            }
        }

        OSL_ENSURE(i,"no columns in row ?");

        /*
        If the last cell was "false" then there is no valid cell following it,
        so the default mapping forward won't work. So map it (and
        contiguous invalid cells backwards to the last valid cell instead.)
        */
        if (i && !pR->bExist[i-1])
        {
            sal_uInt16 k=i-1;
            while (k && !pR->bExist[k])
                k--;
            for (sal_uInt16 n=k+1;n<i;n++)
                pR->nTransCell[n] = pR->nTransCell[k];
        }

        pR->nTransCell[i++] = static_cast<sal_Int8>(j++);  // Can exceed by 2 among other
        pR->nTransCell[i] = static_cast<sal_Int8>(j);        // things because of bREmptyCol

        pR->nSwCols = pR->nSwCols + nAddCols;
        if( pR->nSwCols < nMinCols )
            nMinCols = pR->nSwCols;
    }

    if ((m_nMinLeft && !m_bIsBiDi && text::HoriOrientation::LEFT == m_eOri) ||
        (m_nMinLeft != -108 && m_bIsBiDi && text::HoriOrientation::RIGHT == m_eOri)) // Word sets the first nCenter value to -108 when no indent is used
        m_eOri = text::HoriOrientation::LEFT_AND_WIDTH; //  absolutely positioned

    m_nDefaultSwCols = nMinCols;  // because inserting cells is cheaper than merging
    if( m_nDefaultSwCols == 0 )
        m_bOk = false;
    m_pActBand = m_pFirstBand;
    m_nCurrentBandRow = 0;
    OSL_ENSURE( m_pActBand, "pActBand is 0" );
}

void WW8TabDesc::SetSizePosition(SwFrameFormat* pFrameFormat)
{
    SwFrameFormat* pApply = pFrameFormat;
    if (!pApply )
        pApply = m_pTable->GetFrameFormat();
    OSL_ENSURE(pApply,"No frame");
    pApply->SetFormatAttr(m_aItemSet);
    if (pFrameFormat)
    {
        SwFormatFrameSize aSize = pFrameFormat->GetFrameSize();
        aSize.SetHeightSizeType(ATT_MIN_SIZE);
        aSize.SetHeight(MINLAY);
        pFrameFormat->SetFormatAttr(aSize);
        m_pTable->GetFrameFormat()->SetFormatAttr(SwFormatHoriOrient(0,text::HoriOrientation::FULL));
    }
}

void wwSectionManager::PrependedInlineNode(const SwPosition &rPos,
    const SwNode &rNode)
{
    OSL_ENSURE(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if ((!maSegments.empty()) && (maSegments.back().maStart == rPos.nNode))
        maSegments.back().maStart.Assign(rNode);
}

void WW8TabDesc::CreateSwTable()
{
    ::SetProgressState(m_pIo->m_nProgress, m_pIo->m_pDocShell);   // Update

    // if there is already some content on the Node append new node to ensure
    // that this content remains ABOVE the table
    SwPosition* pPoint = m_pIo->m_pPaM->GetPoint();
    bool bInsNode = pPoint->nContent.GetIndex() != 0;
    bool bSetMinHeight = false;

    /*
     #i8062#
     Set fly anchor to its anchor pos, so that if a table starts immediately
     at this position a new node will be inserted before inserting the table.
    */
    if (!bInsNode && m_pIo->m_pFormatOfJustInsertedApo)
    {
        const SwPosition* pAPos =
            m_pIo->m_pFormatOfJustInsertedApo->GetAnchor().GetContentAnchor();
        if (pAPos && &pAPos->nNode.GetNode() == &pPoint->nNode.GetNode())
        {
            bInsNode = true;
            bSetMinHeight = true;

            SwFormatSurround aSur(m_pIo->m_pFormatOfJustInsertedApo->GetSurround());
            aSur.SetAnchorOnly(true);
            m_pIo->m_pFormatOfJustInsertedApo->SetFormatAttr(aSur);
        }
    }

    if (bSetMinHeight)
    {
        // minimize Fontsize to minimize height growth of the header/footer
        // set font size to 1 point to minimize y-growth of Hd/Ft
        SvxFontHeightItem aSz(20, 100, RES_CHRATR_FONTSIZE);
        m_pIo->NewAttr( aSz );
        m_pIo->m_xCtrlStck->SetAttr(*pPoint, RES_CHRATR_FONTSIZE);
    }

    if (bInsNode)
        m_pIo->AppendTextNode(*pPoint);

    m_xTmpPos.reset(new SwPosition(*m_pIo->m_pPaM->GetPoint()));

    // The table is small: The number of columns is the lowest count of
    // columns of the origin, because inserting is faster than deleting.
    // The number of rows is the count of bands because (identically)
    // rows of a band can be duplicated easy.
    m_pTable = m_pIo->m_rDoc.InsertTable(
            SwInsertTableOptions( SwInsertTableFlags::HeadlineNoBorder, 0 ),
            *m_xTmpPos, m_nBands, m_nDefaultSwCols, m_eOri );

    OSL_ENSURE(m_pTable && m_pTable->GetFrameFormat(), "insert table failed");
    if (!m_pTable || !m_pTable->GetFrameFormat())
        return;

    SwTableNode* pTableNode = m_pTable->GetTableNode();
    OSL_ENSURE(pTableNode, "no table node!");
    if (pTableNode)
    {
        m_pIo->m_aSectionManager.PrependedInlineNode(*m_pIo->m_pPaM->GetPoint(),
            *pTableNode);
    }

    // Check if the node into which the table should be inserted already
    // contains a Pagedesc. If so that Pagedesc would be moved to the
    // row after the table, that would be wrong. So delete and
    // set later to the table format.
    if (SwTextNode *const pNd = m_xTmpPos->nNode.GetNode().GetTextNode())
    {
        if (const SfxItemSet* pSet = pNd->GetpSwAttrSet())
        {
            SfxPoolItem *pSetAttr = nullptr;
            const SfxPoolItem* pItem;
            if (SfxItemState::SET == pSet->GetItemState(RES_BREAK, false, &pItem))
            {
                pSetAttr = new SvxFormatBreakItem( *static_cast<const SvxFormatBreakItem*>(pItem) );
                pNd->ResetAttr( RES_BREAK );
            }

            // eventually set the PageDesc/Break now to the table
            if (pSetAttr)
            {
                m_aItemSet.Put(*pSetAttr);
                delete pSetAttr;
            }
        }
    }

    // total width of table
    if( m_nMaxRight - m_nMinLeft > MINLAY * m_nDefaultSwCols )
    {
        SwFormatFrameSize aFrameSize(ATT_FIX_SIZE, m_nSwWidth);
        if( m_nPercentWidth )
            aFrameSize.SetWidthPercent(m_nPercentWidth);
        m_pTable->GetFrameFormat()->SetFormatAttr(aFrameSize);
        m_aItemSet.Put(aFrameSize);
    }

    SvxFrameDirectionItem aDirection(
        m_bIsBiDi ? SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR );
    m_pTable->GetFrameFormat()->SetFormatAttr(aDirection);

    if (text::HoriOrientation::LEFT_AND_WIDTH == m_eOri)
    {
        if (!m_pIo->m_nInTable && m_pIo->InLocalApo() && m_pIo->m_xSFlyPara &&
            m_pIo->m_xSFlyPara->pFlyFormat && GetMinLeft())
        {
            //If we are inside a frame and we have a border, the frames
            //placement does not consider the tables border, which word
            //displays outside the frame, so adjust here.
            SwFormatHoriOrient aHori(m_pIo->m_xSFlyPara->pFlyFormat->GetHoriOrient());
            sal_Int16 eHori = aHori.GetHoriOrient();
            if ((eHori == text::HoriOrientation::NONE) || (eHori == text::HoriOrientation::LEFT) ||
                (eHori == text::HoriOrientation::LEFT_AND_WIDTH))
            {
                //With multiple table, use last table settings. Perhaps
                //the maximum is what word does ?
                aHori.SetPos(m_pIo->m_xSFlyPara->nXPos + GetMinLeft());
                aHori.SetHoriOrient(text::HoriOrientation::NONE);
                m_pIo->m_xSFlyPara->pFlyFormat->SetFormatAttr(aHori);
            }
        }
        else
        {
            //If bApo is set, then this table is being placed in a floating
            //frame, and the frame matches the left and right *lines* of the
            //table, so the space to the left of the table isn't to be used
            //inside the frame, in word the dialog involved greys out the
            //ability to set the margin.
            SvxLRSpaceItem aL( RES_LR_SPACE );

            long nLeft = 0;
            if (!m_bIsBiDi)
                nLeft = GetMinLeft();
            else
            {
                const short nTableWidth = m_nPreferredWidth ? m_nPreferredWidth : m_nSwWidth;
                nLeft = m_pIo->m_aSectionManager.GetTextAreaWidth();
                nLeft = nLeft - nTableWidth - GetMinLeft();
            }
            aL.SetLeft(nLeft);

            m_aItemSet.Put(aL);
        }
    }

    mxOldRedlineStack = std::move(m_pIo->m_xRedlineStack);
    m_pIo->m_xRedlineStack.reset(new sw::util::RedlineStack(m_pIo->m_rDoc));
}

void WW8TabDesc::UseSwTable()
{
    // init global Vars
    m_pTabLines = &m_pTable->GetTabLines();
    m_nCurrentRow = m_nCurrentCol = m_nCurrentBandRow = 0;

    m_pTableNd  = const_cast<SwTableNode*>((*m_pTabLines)[0]->GetTabBoxes()[0]->
        GetSttNd()->FindTableNode());
    OSL_ENSURE( m_pTableNd, "Where is my table node" );

    // #i69519# - Restrict rows to repeat to a decent value
    if ( m_nRowsToRepeat == static_cast<sal_uInt16>(m_nRows) )
        m_nRowsToRepeat = 1;

    m_pTableNd->GetTable().SetRowsToRepeat( m_nRowsToRepeat );
    // insert extra cells if needed and something like this
    AdjustNewBand();

    WW8DupProperties aDup(m_pIo->m_rDoc, m_pIo->m_xCtrlStck.get());
    m_pIo->m_xCtrlStck->SetAttr(*m_pIo->m_pPaM->GetPoint(), 0, false);

    // now set the correct PaM and prepare first merger group if any
    SetPamInCell(m_nCurrentCol, true);
    aDup.Insert(*m_pIo->m_pPaM->GetPoint());

    m_pIo->m_bWasTabRowEnd = false;
    m_pIo->m_bWasTabCellEnd = false;
}

void WW8TabDesc::MergeCells()
{
    short nRow;

    for (m_pActBand=m_pFirstBand, nRow=0; m_pActBand; m_pActBand=m_pActBand->pNextBand)
    {
        // insert current box into merge group if appropriate.
        // The algorithm must ensure proper row and column order in WW8SelBoxInfo!
        if( m_pActBand->pTCs )
        {
            for( short j = 0; j < m_pActBand->nRows; j++, nRow++ )
                for( short i = 0; i < m_pActBand->nWwCols; i++ )
                {
                    WW8SelBoxInfo* pActMGroup = nullptr;

                    // start a new merge group if appropriate

                    OSL_ENSURE(nRow < static_cast<sal_uInt16>(m_pTabLines->size()),
                        "Too few lines, table ended early");
                    if (nRow >= static_cast<sal_uInt16>(m_pTabLines->size()))
                        return;
                    m_pTabLine = (*m_pTabLines)[ nRow ];
                    m_pTabBoxes = &m_pTabLine->GetTabBoxes();

                    sal_uInt16 nCol = m_pActBand->nTransCell[ i ];
                    if (!m_pActBand->bExist[i])
                        continue;
                    OSL_ENSURE(nCol < m_pTabBoxes->size(),
                        "Too few columns, table ended early");
                    if (nCol >= m_pTabBoxes->size())
                        return;
                    m_pTabBox = (*m_pTabBoxes)[nCol].get();
                    WW8_TCell& rCell = m_pActBand->pTCs[ i ];
                    // is this the left upper cell of a merge group ?

                    bool bMerge = false;
                    if ( rCell.bVertRestart && !rCell.bMerged )
                        bMerge = true;
                    else if (rCell.bFirstMerged && m_pActBand->bExist[i])
                    {
                        // Some tests to avoid merging cells which previously were
                        // declared invalid because of sharing the exact same dimensions
                        // as their previous cell

                        //If there's anything underneath/above we're ok.
                        if (rCell.bVertMerge || rCell.bVertRestart)
                            bMerge = true;
                        else
                        {
                            //If it's a hori merge only, and the only things in
                            //it are invalid cells then it's already taken care
                            //of, so don't merge.
                            for (sal_uInt16 i2 = i+1; i2 < m_pActBand->nWwCols; i2++ )
                                if (m_pActBand->pTCs[ i2 ].bMerged &&
                                    !m_pActBand->pTCs[ i2 ].bFirstMerged  )
                                {
                                    if (m_pActBand->bExist[i2])
                                    {
                                        bMerge = true;
                                        break;
                                    }
                                }
                                else
                                    break;
                        }
                    }

                    // remove numbering from cells that will be disabled in the merge
                    if( rCell.bVertMerge && !rCell.bVertRestart )
                    {
                        SwPaM aPam( *m_pTabBox->GetSttNd(), 0 );
                        aPam.GetPoint()->nNode++;
                        SwTextNode* pNd = aPam.GetNode().GetTextNode();
                        while( pNd )
                        {
                            pNd->SetCountedInList( false );

                            aPam.GetPoint()->nNode++;
                            pNd = aPam.GetNode().GetTextNode();
                        }
                    }

                    if (bMerge)
                    {
                        short nX1    = m_pActBand->nCenter[ i ];
                        short nWidth = m_pActBand->nWidth[ i ];

                        // 2. create current merge group
                        pActMGroup = new WW8SelBoxInfo( nX1, nWidth );

                        // determine size of new merge group
                        // before inserted the new merge group.
                        // Needed to correctly locked previously created merge groups.
                        // Calculate total width and set
                        short nSizCell = m_pActBand->nWidth[ i ];
                        for (sal_uInt16 i2 = i+1; i2 < m_pActBand->nWwCols; i2++ )
                            if (m_pActBand->pTCs[ i2 ].bMerged &&
                                !m_pActBand->pTCs[ i2 ].bFirstMerged  )
                            {
                                nSizCell = nSizCell + m_pActBand->nWidth[ i2 ];
                            }
                            else
                                break;
                        pActMGroup->nGroupWidth = nSizCell;

                        // locked previously created merge groups,
                        // after determining the size for the new merge group.
                        // 1. If necessary close old merge group(s) that overlap
                        // the X-area of the new group
                        for (;;)
                        {
                            WW8SelBoxInfo* p = FindMergeGroup(
                                nX1, pActMGroup->nGroupWidth, false );
                            if (p == nullptr)
                            {
                                break;
                            }
                            p->bGroupLocked = true;
                        }

                        // 3. push to group array
                        m_MergeGroups.push_back(std::unique_ptr<WW8SelBoxInfo>(pActMGroup));
                    }

                    // if necessary add the current box to a merge group
                    // (that can be a newly created or another group)
                    UpdateTableMergeGroup( rCell, pActMGroup, m_pTabBox, i );
                }
            }
    }
}

//There is a limbo area in word at the end of the row marker
//where properties can live in word, there is no location in
//writer equivalent, so try and park the cursor in the best
//match, see #i23022#/#i18644#
void WW8TabDesc::ParkPaM()
{
    SwTableBox *pTabBox2 = nullptr;
    short nRow = m_nCurrentRow + 1;
    if (nRow < static_cast<sal_uInt16>(m_pTabLines->size()))
    {
        if (SwTableLine *pLine = (*m_pTabLines)[nRow])
        {
            SwTableBoxes &rBoxes = pLine->GetTabBoxes();
            pTabBox2 = rBoxes.empty() ? nullptr : rBoxes.front().get();
        }
    }

    if (!pTabBox2 || !pTabBox2->GetSttNd())
    {
        MoveOutsideTable();
        return;
    }

    sal_uLong nSttNd = pTabBox2->GetSttIdx() + 1,
              nEndNd = pTabBox2->GetSttNd()->EndOfSectionIndex();

    if (m_pIo->m_pPaM->GetPoint()->nNode != nSttNd)
    {
        do
        {
            m_pIo->m_pPaM->GetPoint()->nNode = nSttNd;
        }
        while (m_pIo->m_pPaM->GetNode().GetNodeType() != SwNodeType::Text && ++nSttNd < nEndNd);

        m_pIo->m_pPaM->GetPoint()->nContent.Assign(m_pIo->m_pPaM->GetContentNode(), 0);
        m_pIo->m_rDoc.SetTextFormatColl(*m_pIo->m_pPaM, const_cast<SwTextFormatColl*>(m_pIo->m_pDfltTextFormatColl));
    }
}

void WW8TabDesc::MoveOutsideTable()
{
    OSL_ENSURE(m_xTmpPos.get() && m_pIo, "I've forgotten where the table is anchored");
    if (m_xTmpPos && m_pIo)
        *m_pIo->m_pPaM->GetPoint() = *m_xTmpPos;
}

void WW8TabDesc::FinishSwTable()
{
    m_pIo->m_xRedlineStack->closeall(*m_pIo->m_pPaM->GetPoint());
    m_pIo->m_aFrameRedlines.emplace(std::move(m_pIo->m_xRedlineStack));
    m_pIo->m_xRedlineStack = std::move(mxOldRedlineStack);

    WW8DupProperties aDup(m_pIo->m_rDoc,m_pIo->m_xCtrlStck.get());
    m_pIo->m_xCtrlStck->SetAttr( *m_pIo->m_pPaM->GetPoint(), 0, false);

    MoveOutsideTable();
    m_xTmpPos.reset();

    aDup.Insert(*m_pIo->m_pPaM->GetPoint());

    m_pIo->m_bWasTabRowEnd = false;
    m_pIo->m_bWasTabCellEnd = false;

    m_pIo->m_aInsertedTables.InsertTable(*m_pTableNd, *m_pIo->m_pPaM);

    MergeCells();

    // if needed group cells together that should be merged
    if (!m_MergeGroups.empty())
    {
        // process all merge groups one by one
        for (auto const& groupIt : m_MergeGroups)
        {
            if((1 < groupIt->size()) && groupIt->row(0)[0])
            {
                SwFrameFormat* pNewFormat = groupIt->row(0)[0]->ClaimFrameFormat();
                pNewFormat->SetFormatAttr(SwFormatFrameSize(ATT_VAR_SIZE, groupIt->nGroupWidth, 0));
                const sal_uInt16 nRowSpan = groupIt->rowsCount();
                for (sal_uInt16 n = 0; n < nRowSpan; ++n)
                {
                    auto& rRow = groupIt->row(n);
                    for (size_t i = 0; i<rRow.size(); ++i)
                    {
                        const long nRowSpanSet = (n == 0) && (i == 0) ?
                            nRowSpan :
                            (-1 * (nRowSpan - n));
                        SwTableBox* pCurrentBox = rRow[i];
                        pCurrentBox->setRowSpan(nRowSpanSet);

                        if (i == 0)
                            pCurrentBox->ChgFrameFormat(static_cast<SwTableBoxFormat*>(pNewFormat));
                        else
                        {
                            SwFrameFormat* pFormat = pCurrentBox->ClaimFrameFormat();
                            pFormat->SetFormatAttr(SwFormatFrameSize(ATT_VAR_SIZE, 0, 0));
                        }
                    }
                }
            }
        }
        m_pIo->m_pFormatOfJustInsertedApo = nullptr;
        m_MergeGroups.clear();
    }
}

// browse m_MergeGroups, detect the index of the first fitting group or -1 otherwise

// Parameter: nXcenter  = center position of asking box
//            nWidth    = width of asking box
//            bExact    = flag, if box has to fit into group
//                              or only has to touch

WW8SelBoxInfo* WW8TabDesc::FindMergeGroup(short nX1, short nWidth, bool bExact)
{
    if (!m_MergeGroups.empty())
    {
        // still valid area near the boundery
        const short nTolerance = 4;
        // box boundery
        short nX2 = nX1 + nWidth;
        // approximate group boundery
        short nGrX1;
        short nGrX2;

        // improvement: search backwards
        for (short iGr = m_MergeGroups.size() - 1; iGr >= 0; --iGr)
        {
            // the currently inspected group
            WW8SelBoxInfo& rActGroup = *m_MergeGroups[ iGr ];
            if (!rActGroup.bGroupLocked)
            {
                // approximate group boundery with room (tolerance) to the *outside*
                nGrX1 = rActGroup.nGroupXStart - nTolerance;
                nGrX2 = rActGroup.nGroupXStart
                        + rActGroup.nGroupWidth + nTolerance;

                // If box fits report success

                if( ( nX1 > nGrX1 ) && ( nX2 < nGrX2 ) )
                {
                    return &rActGroup;
                }

                // does the box share areas with the group?

                if( !bExact )
                {
                    // successful if nX1 *or* nX2 are inside the group
                    if(    (     ( nX1 > nGrX1 )
                                        && ( nX1 < nGrX2 - 2*nTolerance ) )
                            || (     ( nX2 > nGrX1 + 2*nTolerance )
                                        && ( nX2 < nGrX2 ) )
                            // or nX1 and nX2 surround the group
                            || (     ( nX1 <=nGrX1 )
                                        && ( nX2 >=nGrX2 ) ) )
                    {
                        return &rActGroup;
                    }
                }
            }
        }
    }
    return nullptr;
}

bool WW8TabDesc::IsValidCell(short nCol) const
{
    return (static_cast<size_t>(nCol) < SAL_N_ELEMENTS(m_pActBand->bExist)) &&
           m_pActBand->bExist[nCol] &&
           static_cast<sal_uInt16>(m_nCurrentRow) < m_pTabLines->size();
}

bool WW8TabDesc::InFirstParaInCell() const
{
    //e.g. #i19718#
    if (!m_pTabBox || !m_pTabBox->GetSttNd())
    {
        OSL_FAIL("Problem with table");
        return false;
    }

    if (!IsValidCell(GetCurrentCol()))
        return false;

    return m_pIo->m_pPaM->GetPoint()->nNode == m_pTabBox->GetSttIdx() + 1;
}

void WW8TabDesc::StartMiserableHackForUnsupportedDirection(short nWwCol)
{
    OSL_ENSURE(m_pActBand, "Impossible");
    if (m_pActBand && nWwCol <= MAX_COL && m_pActBand->maDirections[nWwCol] == 3)
    {
        m_pIo->m_xCtrlStck->NewAttr(*m_pIo->m_pPaM->GetPoint(),
            SvxCharRotateItem(900, false, RES_CHRATR_ROTATE));
    }
}

void WW8TabDesc::EndMiserableHackForUnsupportedDirection(short nWwCol)
{
    OSL_ENSURE(m_pActBand, "Impossible");
    if (m_pActBand && nWwCol <= MAX_COL && m_pActBand->maDirections[nWwCol] == 3)
        m_pIo->m_xCtrlStck->SetAttr(*m_pIo->m_pPaM->GetPoint(), RES_CHRATR_ROTATE);
}

void WW8TabDesc::SetPamInCell(short nWwCol, bool bPam)
{
    OSL_ENSURE( m_pActBand, "pActBand is 0" );
    if (!m_pActBand)
        return;

    sal_uInt16 nCol = m_pActBand->transCell(nWwCol);

    if (static_cast<sal_uInt16>(m_nCurrentRow) >= m_pTabLines->size())
    {
        OSL_ENSURE(false, "Actual row bigger than expected." );
        if (bPam)
            MoveOutsideTable();
        return;
    }

    m_pTabLine = (*m_pTabLines)[m_nCurrentRow];
    m_pTabBoxes = &m_pTabLine->GetTabBoxes();

    if (nCol >= m_pTabBoxes->size())
    {
        if (bPam)
        {
            // The first paragraph in a cell with upper autospacing has upper
            // spacing set to 0
            if (
                 m_pIo->m_bParaAutoBefore && m_pIo->m_bFirstPara &&
                 !m_pIo->m_xWDop->fDontUseHTMLAutoSpacing
               )
            {
                m_pIo->SetUpperSpacing(*m_pIo->m_pPaM, 0);
            }

            // The last paragraph in a cell with lower autospacing has lower
            // spacing set to 0
            if (m_pIo->m_bParaAutoAfter && !m_pIo->m_xWDop->fDontUseHTMLAutoSpacing)
                m_pIo->SetLowerSpacing(*m_pIo->m_pPaM, 0);

            ParkPaM();
        }
        return;
    }
    m_pTabBox = (*m_pTabBoxes)[nCol].get();
    if( !m_pTabBox->GetSttNd() )
    {
        OSL_ENSURE(m_pTabBox->GetSttNd(), "Problems building the table");
        if (bPam)
            MoveOutsideTable();
        return;
    }
    if (bPam)
    {
        m_pCurrentWWCell = &m_pActBand->pTCs[ nWwCol ];

       // The first paragraph in a cell with upper autospacing has upper spacing set to 0
        if(m_pIo->m_bParaAutoBefore && m_pIo->m_bFirstPara && !m_pIo->m_xWDop->fDontUseHTMLAutoSpacing)
            m_pIo->SetUpperSpacing(*m_pIo->m_pPaM, 0);

        // The last paragraph in a cell with lower autospacing has lower spacing set to 0
        if(m_pIo->m_bParaAutoAfter && !m_pIo->m_xWDop->fDontUseHTMLAutoSpacing)
            m_pIo->SetLowerSpacing(*m_pIo->m_pPaM, 0);

        //We need to set the pPaM on the first cell, invalid
        //or not so that we can collect paragraph properties over
        //all the cells, but in that case on the valid cell we do not
        //want to reset the fmt properties
        sal_uLong nSttNd = m_pTabBox->GetSttIdx() + 1,
                  nEndNd = m_pTabBox->GetSttNd()->EndOfSectionIndex();
        if (m_pIo->m_pPaM->GetPoint()->nNode != nSttNd)
        {
            do
            {
                m_pIo->m_pPaM->GetPoint()->nNode = nSttNd;
            }
            while (m_pIo->m_pPaM->GetNode().GetNodeType() != SwNodeType::Text && ++nSttNd < nEndNd);
            m_pIo->m_pPaM->GetPoint()->nContent.Assign(m_pIo->m_pPaM->GetContentNode(), 0);
            // Precautionally set now, otherwise the style is not set for cells
            // that are inserted for margin balancing.
            m_pIo->m_rDoc.SetTextFormatColl(*m_pIo->m_pPaM, const_cast<SwTextFormatColl*>(m_pIo->m_pDfltTextFormatColl));
            // because this cells are invisible helper constructions only to simulate
            // the frayed view of WW-tables we do NOT need SetTextFormatCollAndListLevel()
        }

        // Better to turn Snap to Grid off for all paragraphs in tables
        if(SwTextNode *pNd = m_pIo->m_pPaM->GetNode().GetTextNode())
        {
            const SfxPoolItem &rItm = pNd->SwContentNode::GetAttr(RES_PARATR_SNAPTOGRID);
            const SvxParaGridItem &rSnapToGrid = static_cast<const SvxParaGridItem&>(rItm);

            if(rSnapToGrid.GetValue())
            {
                SvxParaGridItem aGridItem( rSnapToGrid );
                aGridItem.SetValue(false);

                SwPosition* pGridPos = m_pIo->m_pPaM->GetPoint();

                const sal_Int32 nEnd = pGridPos->nContent.GetIndex();
                pGridPos->nContent.Assign(m_pIo->m_pPaM->GetContentNode(), 0);
                m_pIo->m_xCtrlStck->NewAttr(*pGridPos, aGridItem);
                pGridPos->nContent.Assign(m_pIo->m_pPaM->GetContentNode(), nEnd);
                m_pIo->m_xCtrlStck->SetAttr(*pGridPos, RES_PARATR_SNAPTOGRID);
            }
        }

        StartMiserableHackForUnsupportedDirection(nWwCol);
    }
}

void WW8TabDesc::InsertCells( short nIns )
{
    m_pTabLine = (*m_pTabLines)[m_nCurrentRow];
    m_pTabBoxes = &m_pTabLine->GetTabBoxes();
    m_pTabBox = (*m_pTabBoxes)[0].get();

    m_pIo->m_rDoc.GetNodes().InsBoxen( m_pTableNd, m_pTabLine, static_cast<SwTableBoxFormat*>(m_pTabBox->GetFrameFormat()),
                            const_cast<SwTextFormatColl*>(m_pIo->m_pDfltTextFormatColl), nullptr, m_pTabBoxes->size(), nIns );
    // The third parameter contains the FrameFormat of the boxes.
    // Here it is possible to optimize to save (reduce) FrameFormats.
}

void WW8TabDesc::SetTabBorders(SwTableBox* pBox, short nWwIdx)
{
    if( nWwIdx < 0 || nWwIdx >= m_pActBand->nWwCols )
        return;                 // faked cells -> no border

    SvxBoxItem aFormatBox( RES_BOX );
    if (m_pActBand->pTCs)     // neither Cell Border nor Default Border defined ?
    {
        WW8_TCell* pT = &m_pActBand->pTCs[nWwIdx];
        if (SwWW8ImplReader::IsBorder(pT->rgbrc))
            SwWW8ImplReader::SetBorder(aFormatBox, pT->rgbrc);
    }

    if (m_pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwTOP))
    {
        aFormatBox.SetDistance(
            m_pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwTOP],
            SvxBoxItemLine::TOP);
    }
    else
        aFormatBox.SetDistance(m_pActBand->mnDefaultTop, SvxBoxItemLine::TOP);
    if (m_pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwBOTTOM))
    {
        aFormatBox.SetDistance(
            m_pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwBOTTOM],
            SvxBoxItemLine::BOTTOM);
    }
    else
        aFormatBox.SetDistance(m_pActBand->mnDefaultBottom,SvxBoxItemLine::BOTTOM);

    // nGapHalf for WW is a *horizontal* gap between table cell and content.
    short nLeftDist =
        m_pActBand->mbHasSpacing ? m_pActBand->mnDefaultLeft : m_pActBand->nGapHalf;
    short nRightDist =
        m_pActBand->mbHasSpacing ? m_pActBand->mnDefaultRight : m_pActBand->nGapHalf;
    if (m_pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwLEFT))
    {
        aFormatBox.SetDistance(
            m_pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwLEFT],
            SvxBoxItemLine::LEFT);
    }
    else
        aFormatBox.SetDistance(nLeftDist, SvxBoxItemLine::LEFT);
    if (m_pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwRIGHT))
    {
        aFormatBox.SetDistance(
            m_pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwRIGHT],
            SvxBoxItemLine::RIGHT);
    }
    else
        aFormatBox.SetDistance(nRightDist,SvxBoxItemLine::RIGHT);

    pBox->GetFrameFormat()->SetFormatAttr(aFormatBox);
}

void WW8TabDesc::SetTabShades( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= m_pActBand->nWwCols )
        return;                 // faked cells -> no color

    bool bFound=false;
    if (m_pActBand->pNewSHDs && m_pActBand->pNewSHDs[nWwIdx] != COL_AUTO)
    {
        Color aColor(m_pActBand->pNewSHDs[nWwIdx]);
        pBox->GetFrameFormat()->SetFormatAttr(SvxBrushItem(aColor, RES_BACKGROUND));
        bFound = true;
    }

    //If there was no new shades, or no new shade setting
    if (m_pActBand->pSHDs && !bFound)
    {
        WW8_SHD& rSHD = m_pActBand->pSHDs[nWwIdx];
        if (!rSHD.GetValue())       // auto
            return;

        SwWW8Shade aSh( m_pIo->m_bVer67, rSHD );
        pBox->GetFrameFormat()->SetFormatAttr(SvxBrushItem(aSh.aColor, RES_BACKGROUND));
    }
}

SvxFrameDirection MakeDirection(sal_uInt16 nCode, bool bIsBiDi)
{
    SvxFrameDirection eDir = SvxFrameDirection::Environment;
    // 1: Asian layout with rotated CJK characters
    // 5: Asian layout
    // 3: Western layout rotated by 90 degrees
    // 4: Western layout
    switch (nCode)
    {
        default:
            OSL_ENSURE(eDir == SvxFrameDirection::Environment, "unknown direction code, maybe it's a bitfield");
            SAL_FALLTHROUGH;
        case 3:
            eDir = bIsBiDi ? SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB; // #i38158# - Consider RTL tables
            break;
        case 5:
            eDir = SvxFrameDirection::Vertical_RL_TB;
            break;
        case 1:
            eDir = SvxFrameDirection::Vertical_RL_TB;
            break;
        case 4:
            eDir = bIsBiDi ? SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB; // #i38158# - Consider RTL tables
            break;
    }
    return eDir;
}

void WW8TabDesc::SetTabDirection(SwTableBox* pBox, short nWwIdx)
{
    if (nWwIdx < 0 || nWwIdx >= m_pActBand->nWwCols)
        return;
    SvxFrameDirectionItem aItem(MakeDirection(m_pActBand->maDirections[nWwIdx], m_bIsBiDi), RES_FRAMEDIR);
    pBox->GetFrameFormat()->SetFormatAttr(aItem);
}

void WW8TabDesc::SetTabVertAlign( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= m_pActBand->nWwCols )
        return;

    sal_Int16 eVertOri=text::VertOrientation::TOP;

    if( m_pActBand->pTCs )
    {
        WW8_TCell* pT = &m_pActBand->pTCs[nWwIdx];
        switch (pT->nVertAlign)
        {
            case 0:
            default:
                eVertOri = text::VertOrientation::TOP;
                break;
            case 1:
                eVertOri = text::VertOrientation::CENTER;
                break;
            case 2:
                eVertOri = text::VertOrientation::BOTTOM;
                break;
        }
    }

    pBox->GetFrameFormat()->SetFormatAttr( SwFormatVertOrient(0,eVertOri) );
}

void WW8TabDesc::AdjustNewBand()
{
    if( m_pActBand->nSwCols > m_nDefaultSwCols )        // split cells
        InsertCells( m_pActBand->nSwCols - m_nDefaultSwCols );

    SetPamInCell( 0, false);
    OSL_ENSURE( m_pTabBoxes && m_pTabBoxes->size() == static_cast<sal_uInt16>(m_pActBand->nSwCols),
        "Wrong column count in table" );

    if( m_bClaimLineFormat )
    {
        m_pTabLine->ClaimFrameFormat();            // necessary because of cell height
        SwFormatFrameSize aF( ATT_MIN_SIZE, 0, 0 );  // default

        if (m_pActBand->nLineHeight == 0)    // 0 = Auto
            aF.SetHeightSizeType( ATT_VAR_SIZE );
        else
        {
            if (m_pActBand->nLineHeight < 0) // positive = min, negative = exact
            {
                aF.SetHeightSizeType(ATT_FIX_SIZE);
                m_pActBand->nLineHeight = -m_pActBand->nLineHeight;
            }
            if (m_pActBand->nLineHeight < MINLAY) // invalid cell height
                m_pActBand->nLineHeight = MINLAY;

            aF.SetHeight(m_pActBand->nLineHeight);// set min/exact height
        }
        m_pTabLine->GetFrameFormat()->SetFormatAttr(aF);
    }

    //Word stores 1 for bCantSplit if the row cannot be split, we set true if
    //we can split the row
    bool bSetCantSplit = m_pActBand->bCantSplit;
    m_pTabLine->GetFrameFormat()->SetFormatAttr(SwFormatRowSplit(!bSetCantSplit));

    //  if table is only a single row, and row is set as don't split, set the same value for the whole table.
    if( bSetCantSplit && m_pTabLines->size() == 1 )
        m_pTable->GetFrameFormat()->SetFormatAttr(SwFormatLayoutSplit( !bSetCantSplit ));

    short i;    // SW-Index
    short j;    // WW-Index
    short nW;   // Width
    SwFormatFrameSize aFS( ATT_FIX_SIZE );
    j = m_pActBand->bLEmptyCol ? -1 : 0;

    for( i = 0; i < m_pActBand->nSwCols; i++ )
    {
        // set cell width
        if( j < 0 )
            nW = m_pActBand->nCenter[0] - m_nMinLeft;
        else
        {
            //Set j to first non invalid cell
            while ((j < m_pActBand->nWwCols) && (!m_pActBand->bExist[j]))
                j++;

            if( j < m_pActBand->nWwCols )
                nW = m_pActBand->nCenter[j+1] - m_pActBand->nCenter[j];
            else
                nW = m_nMaxRight - m_pActBand->nCenter[j];
            m_pActBand->nWidth[ j ] = nW;
        }

        SwTableBox* pBox = (*m_pTabBoxes)[i].get();
        // could be reduced further by intelligent moving of FrameFormats
        pBox->ClaimFrameFormat();

        SetTabBorders(pBox, j);

        // #i18128# word has only one line between adjoining vertical cells
        // we have to mimic this in the filter by picking the larger of the
        // sides and using that one on one side of the line (right)
        SvxBoxItem aCurrentBox(sw::util::ItemGet<SvxBoxItem>(*(pBox->GetFrameFormat()), RES_BOX));
        if (i != 0)
        {
            SwTableBox* pBox2 = (*m_pTabBoxes)[i-1].get();
            SvxBoxItem aOldBox(sw::util::ItemGet<SvxBoxItem>(*(pBox2->GetFrameFormat()), RES_BOX));
            if( aOldBox.CalcLineWidth(SvxBoxItemLine::RIGHT) > aCurrentBox.CalcLineWidth(SvxBoxItemLine::LEFT) )
                aCurrentBox.SetLine(aOldBox.GetLine(SvxBoxItemLine::RIGHT), SvxBoxItemLine::LEFT);

            aOldBox.SetLine(nullptr, SvxBoxItemLine::RIGHT);
            pBox2->GetFrameFormat()->SetFormatAttr(aOldBox);
        }

        pBox->GetFrameFormat()->SetFormatAttr(aCurrentBox);

        SetTabVertAlign(pBox, j);
        SetTabDirection(pBox, j);
        if( m_pActBand->pSHDs || m_pActBand->pNewSHDs)
            SetTabShades(pBox, j);
        j++;

        aFS.SetWidth( nW );
        pBox->GetFrameFormat()->SetFormatAttr( aFS );

        // skip non existing cells
        while( ( j < m_pActBand->nWwCols ) && !m_pActBand->bExist[j] )
        {
            m_pActBand->nWidth[j] = m_pActBand->nCenter[j+1] - m_pActBand->nCenter[j];
            j++;
        }
    }
}

void WW8TabDesc::TableCellEnd()
{
    ::SetProgressState(m_pIo->m_nProgress, m_pIo->m_pDocShell);   // Update

    EndMiserableHackForUnsupportedDirection(m_nCurrentCol);

    // new line/row
    if( m_pIo->m_bWasTabRowEnd )
    {
        // bWasTabRowEnd will be deactivated in
        // SwWW8ImplReader::ProcessSpecial()

        sal_uInt16 iCol = GetLogicalWWCol();
        if (iCol < m_aNumRuleNames.size())
        {
            m_aNumRuleNames.erase(m_aNumRuleNames.begin() + iCol,
                m_aNumRuleNames.end());
        }

        m_nCurrentCol = 0;
        m_nCurrentRow++;
        m_nCurrentBandRow++;
        OSL_ENSURE( m_pActBand , "pActBand is 0" );
        if( m_pActBand )
        {
            if( m_nCurrentRow >= m_nRows )  // nothing to at end of table
                return;

            bool bNewBand = m_nCurrentBandRow >= m_pActBand->nRows;
            if( bNewBand )
            {                       // new band needed ?
                m_pActBand = m_pActBand->pNextBand;
                m_nCurrentBandRow = 0;
                OSL_ENSURE( m_pActBand, "pActBand is 0" );
                AdjustNewBand();
            }
            else
            {
                SwTableBox* pBox = (*m_pTabBoxes)[0].get();
                SwSelBoxes aBoxes;
                m_pIo->m_rDoc.InsertRow( SwTable::SelLineFromBox( pBox, aBoxes ) );
            }
        }
    }
    else
    {                       // new column ( cell )
        m_nCurrentCol++;
    }
    SetPamInCell(m_nCurrentCol, true);

    // finish Annotated Level Numbering ?
    if (m_pIo->m_bAnl && !m_pIo->m_bCurrentAND_fNumberAcross && m_pActBand)
        m_pIo->StopAllAnl(IsValidCell(m_nCurrentCol));
}

// if necessary register the box for the merge group for this column
void WW8TabDesc::UpdateTableMergeGroup(  WW8_TCell const &     rCell,
                                                WW8SelBoxInfo* pActGroup,
                                                SwTableBox*    pActBox,
                                                sal_uInt16         nCol )
{
    // check if the box has to be merged
    // If cell is the first one to be merged, a new merge group has to be provided.
    // E.g., it could be that a cell is the first one to be merged, but no
    // new merge group is provided, because the potential other cell to be merged
    // doesn't exist - see method <WW8TabDesc::MergeCells>.
    if ( m_pActBand->bExist[ nCol ] &&
         ( ( rCell.bFirstMerged && pActGroup ) ||
           rCell.bMerged ||
           rCell.bVertMerge ||
           rCell.bVertRestart ) )
    {
        // detect appropriate merge group
        WW8SelBoxInfo* pTheMergeGroup = nullptr;
        if( pActGroup )
            // assign group
            pTheMergeGroup = pActGroup;
        else
        {
            // find group
            pTheMergeGroup = FindMergeGroup(
                m_pActBand->nCenter[ nCol ], m_pActBand->nWidth[  nCol ], true );
        }
        if( pTheMergeGroup )
        {
            // add current box to merge group
            pTheMergeGroup->push_back(pActBox);
        }
    }
}

sal_uInt16 WW8TabDesc::GetLogicalWWCol() const // returns number of col as INDICATED within WW6 UI status line -1
{
    sal_uInt16 nCol = 0;
    if( m_pActBand && m_pActBand->pTCs)
    {
        for( sal_uInt16 iCol = 1; iCol <= m_nCurrentCol && iCol <= m_pActBand->nWwCols; ++iCol )
        {
            if( !m_pActBand->pTCs[ iCol-1 ].bMerged )
                ++nCol;
        }
    }
    return nCol;
}

// find name of numrule valid for current WW-COL
OUString WW8TabDesc::GetNumRuleName() const
{
    sal_uInt16 nCol = GetLogicalWWCol();
    if (nCol < m_aNumRuleNames.size())
        return m_aNumRuleNames[nCol];
    return OUString();
}

void WW8TabDesc::SetNumRuleName( const OUString& rName )
{
    sal_uInt16 nCol = GetLogicalWWCol();
    for (sal_uInt16 nSize = static_cast< sal_uInt16 >(m_aNumRuleNames.size()); nSize <= nCol; ++nSize)
        m_aNumRuleNames.emplace_back();
    m_aNumRuleNames[nCol] = rName;
}

bool SwWW8ImplReader::StartTable(WW8_CP nStartCp)
{
    // Entering a table so make sure the FirstPara flag gets set
    m_bFirstPara = true;
    // no recursive table, not with InsertFile in table or foot note
    if (m_bReadNoTable)
        return false;

    if (m_xTableDesc)
        m_aTableStack.push(std::move(m_xTableDesc));

    // #i33818# - determine absolute position object attributes,
    // if possible. It's needed for nested tables.
    WW8FlyPara* pTableWFlyPara( nullptr );
    WW8SwFlyPara* pTableSFlyPara( nullptr );
    // #i45301# - anchor nested table inside Writer fly frame
    // only at-character, if absolute position object attributes are available.
    // Thus, default anchor type is as-character anchored.
    RndStdIds eAnchor( RndStdIds::FLY_AS_CHAR );
    if ( m_nInTable )
    {
        WW8_TablePos* pNestedTabPos( nullptr );
        WW8_TablePos aNestedTabPos;
        WW8PLCFxSave1 aSave;
        m_xPlcxMan->GetPap()->Save( aSave );
        WW8PLCFx_Cp_FKP* pPap = m_xPlcxMan->GetPapPLCF();
        WW8_CP nMyStartCp = nStartCp;
        if ( SearchRowEnd( pPap, nMyStartCp, m_nInTable ) &&
             ParseTabPos( &aNestedTabPos, pPap ) )
        {
            pNestedTabPos = &aNestedTabPos;
        }
        m_xPlcxMan->GetPap()->Restore( aSave );
        if ( pNestedTabPos )
        {
            ApoTestResults aApo = TestApo( m_nInTable + 1, false, pNestedTabPos );
            pTableWFlyPara = ConstructApo( aApo, pNestedTabPos );
            if ( pTableWFlyPara )
            {
                // <WW8SwFlyPara> constructor has changed - new 4th parameter
                // containing WW8 page top margin.
                pTableSFlyPara = new WW8SwFlyPara(*m_pPaM, *this, *pTableWFlyPara,
                    m_aSectionManager.GetWWPageTopMargin(),
                    m_aSectionManager.GetTextAreaWidth(),
                    m_nIniFlyDx, m_nIniFlyDy);

                // #i45301# - anchor nested table Writer fly frame at-character
                eAnchor = RndStdIds::FLY_AT_CHAR;
            }
        }
    }
    // if first paragraph in table has break-before-page, transfer that setting to the table itself.
    else if( StyleExists(m_nCurrentColl) )
    {
        const SwFormat* pStyleFormat = m_vColl[m_nCurrentColl].m_pFormat;
        if( pStyleFormat && pStyleFormat->GetBreak().GetBreak() == SvxBreak::PageBefore )
            NewAttr( pStyleFormat->GetBreak() );
    }

    m_xTableDesc.reset(new WW8TabDesc(this, nStartCp));

    if( m_xTableDesc->Ok() )
    {
        int nNewInTable = m_nInTable + 1;

        if ((eAnchor == RndStdIds::FLY_AT_CHAR)
            && !m_aTableStack.empty() && !InEqualApo(nNewInTable) )
        {
            m_xTableDesc->m_pParentPos = new SwPosition(*m_pPaM->GetPoint());
            SfxItemSet aItemSet(m_rDoc.GetAttrPool(),
                                svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{});
            // #i33818# - anchor the Writer fly frame for the nested table at-character.
            // #i45301#
            SwFormatAnchor aAnchor( eAnchor );
            aAnchor.SetAnchor( m_xTableDesc->m_pParentPos );
            aItemSet.Put( aAnchor );
            m_xTableDesc->m_pFlyFormat = m_rDoc.MakeFlySection( eAnchor,
                                                      m_xTableDesc->m_pParentPos, &aItemSet);
            OSL_ENSURE( m_xTableDesc->m_pFlyFormat->GetAnchor().GetAnchorId() == eAnchor,
                   "Not the anchor type requested!" );
            MoveInsideFly(m_xTableDesc->m_pFlyFormat);
        }
        m_xTableDesc->CreateSwTable();
        if (m_xTableDesc->m_pFlyFormat)
        {
            m_xTableDesc->SetSizePosition(m_xTableDesc->m_pFlyFormat);
            // #i33818# - Use absolute position object attributes,
            // if existing, and apply them to the created Writer fly frame.
            if ( pTableWFlyPara && pTableSFlyPara )
            {
                WW8FlySet aFlySet( *this, pTableWFlyPara, pTableSFlyPara, false );
                SwFormatAnchor aAnchor( RndStdIds::FLY_AT_CHAR );
                aAnchor.SetAnchor( m_xTableDesc->m_pParentPos );
                aFlySet.Put( aAnchor );
                m_xTableDesc->m_pFlyFormat->SetFormatAttr( aFlySet );
            }
            else
            {
                SwFormatHoriOrient aHori =
                            m_xTableDesc->m_pTable->GetFrameFormat()->GetHoriOrient();
                m_xTableDesc->m_pFlyFormat->SetFormatAttr(aHori);
                m_xTableDesc->m_pFlyFormat->SetFormatAttr( SwFormatSurround( css::text::WrapTextMode_NONE ) );
            }
            // #i33818# - The nested table doesn't have to leave
            // the table cell. Thus, the Writer fly frame has to follow the text flow.
            m_xTableDesc->m_pFlyFormat->SetFormatAttr( SwFormatFollowTextFlow( true ) );
        }
        else
            m_xTableDesc->SetSizePosition(nullptr);
        m_xTableDesc->UseSwTable();
    }
    else
        PopTableDesc();

    // #i33818#
    delete pTableWFlyPara;
    delete pTableSFlyPara;

    return m_xTableDesc.get() != nullptr;
}

void SwWW8ImplReader::TabCellEnd()
{
    if (m_nInTable && m_xTableDesc)
        m_xTableDesc->TableCellEnd();

    m_bFirstPara = true;    // We have come to the end of a cell so FirstPara flag
    m_bReadTable = false;
}

void SwWW8ImplReader::Read_TabCellEnd( sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( ( nLen > 0 ) && ( *pData == 1 ) )
        m_bWasTabCellEnd = true;
}

void SwWW8ImplReader::Read_TabRowEnd( sal_uInt16, const sal_uInt8* pData, short nLen )   // Sprm25
{
    if( ( nLen > 0 ) && ( *pData == 1 ) )
        m_bWasTabRowEnd = true;
}

void SwWW8ImplReader::PopTableDesc()
{
    if (m_xTableDesc && m_xTableDesc->m_pFlyFormat)
    {
        MoveOutsideFly(m_xTableDesc->m_pFlyFormat, *m_xTableDesc->m_pParentPos);
    }

    m_xTableDesc.reset();
    if (!m_aTableStack.empty())
    {
       m_xTableDesc = std::move(m_aTableStack.top());
       m_aTableStack.pop();
    }
}

void SwWW8ImplReader::StopTable()
{
    OSL_ENSURE(m_xTableDesc.get(), "Panic, stop table with no table!");
    if (!m_xTableDesc)
        return;

    // We are leaving a table so make sure the next paragraph doesn't think
    // it's the first paragraph
    m_bFirstPara = false;

    m_xTableDesc->FinishSwTable();
    PopTableDesc();

    m_bReadTable = true;
}

bool SwWW8ImplReader::IsInvalidOrToBeMergedTabCell() const
{
    if( !m_xTableDesc )
        return false;

    const WW8_TCell* pCell = m_xTableDesc->GetCurrentWWCell();

    return     !m_xTableDesc->IsValidCell( m_xTableDesc->GetCurrentCol() )
            || (    pCell
                 && (    !pCell->bFirstMerged
                      && (    pCell->bMerged
                           || (    pCell->bVertMerge
                                && !pCell->bVertRestart
                              )
                         )
                    )
                );
}

sal_uInt16 SwWW8ImplReader::StyleUsingLFO( sal_uInt16 nLFOIndex ) const
{
    sal_uInt16 nRes = USHRT_MAX;
    if( !m_vColl.empty() )
    {
        for(sal_uInt16 nI = 0; nI < m_xStyles->GetCount(); nI++ )
            if(    m_vColl[ nI ].m_bValid
                && (nLFOIndex == m_vColl[ nI ].m_nLFOIndex) )
                nRes = nI;
    }
    return nRes;
}

const SwFormat* SwWW8ImplReader::GetStyleWithOrgWWName( OUString const & rName ) const
{
    SwFormat* pRet = nullptr;
    if( !m_vColl.empty() )
    {
        for(sal_uInt16 nI = 0; nI < m_xStyles->GetCount(); nI++ )
            if(    m_vColl[ nI ].m_bValid
                && (rName == m_vColl[ nI ].GetOrgWWName()) )
            {
                pRet = m_vColl[ nI ].m_pFormat;
                break;
            }
    }
    return pRet;
}

//          class WW8RStyle

SprmResult WW8RStyle::HasParaSprm(sal_uInt16 nId) const
{
    if( !mpParaSprms || !mnSprmsLen )
        return SprmResult();

    return maSprmParser.findSprmData(nId, mpParaSprms, mnSprmsLen);
}

void WW8RStyle::ImportSprms(sal_uInt8 *pSprms, short nLen, bool bPap)
{
    if (!nLen)
        return;

    if( bPap )
    {
        mpParaSprms = pSprms;   // for HasParaSprms()
        mnSprmsLen = nLen;
    }

    WW8SprmIter aSprmIter(pSprms, nLen, maSprmParser);
    while (const sal_uInt8* pSprm = aSprmIter.GetSprms())
    {
#ifdef DEBUGSPRMREADER
        fprintf(stderr, "id is %x\n", aIter.GetCurrentId());
#endif
        mpIo->ImportSprm(pSprm, aSprmIter.GetRemLen(), aSprmIter.GetCurrentId());
        aSprmIter.advance();
    }

    mpParaSprms = nullptr;
    mnSprmsLen = 0;
}

void WW8RStyle::ImportSprms(std::size_t nPosFc, short nLen, bool bPap)
{
    if (!nLen)
        return;

    if (checkSeek(*mpStStrm, nPosFc))
    {
        std::unique_ptr<sal_uInt8[]> pSprms( new sal_uInt8[nLen] );
        nLen = mpStStrm->ReadBytes(pSprms.get(), nLen);
        ImportSprms(pSprms.get(), nLen, bPap);
    }
}

static inline short WW8SkipOdd(SvStream* pSt )
{
    if ( pSt->Tell() & 0x1 )
    {
        sal_uInt8 c;
        return pSt->ReadBytes( &c, 1 );
    }
    return 0;
}

static inline short WW8SkipEven(SvStream* pSt )
{
    if (!(pSt->Tell() & 0x1))
    {
        sal_uInt8 c;
        return pSt->ReadBytes( &c, 1 );
    }
    return 0;
}

short WW8RStyle::ImportUPX(short nLen, bool bPAP, bool bOdd)
{
    if( 0 < nLen ) // Empty ?
    {
        if (bOdd)
            nLen = nLen - WW8SkipEven( mpStStrm );
        else
            nLen = nLen - WW8SkipOdd( mpStStrm );

        sal_Int16 cbUPX(0);
        mpStStrm->ReadInt16( cbUPX );

        nLen-=2;

        if ( cbUPX > nLen )
            cbUPX = nLen;       // shrink cbUPX to nLen

        if( (1 < cbUPX) || ( (0 < cbUPX) && !bPAP ) )
        {
            if( bPAP )
            {
                sal_uInt16 id;
                mpStStrm->ReadUInt16( id );

                cbUPX-=  2;
                nLen-=  2;
            }

            if( 0 < cbUPX )
            {
                sal_uInt64 const nPos = mpStStrm->Tell(); // if something is interpreted wrong,
                                                 // this should make it work again
                ImportSprms( nPos, cbUPX, bPAP );

                if ( mpStStrm->Tell() != nPos + cbUPX )
                    mpStStrm->Seek( nPos+cbUPX );

                nLen = nLen - cbUPX;
            }
        }
    }
    return nLen;
}

void WW8RStyle::ImportGrupx(short nLen, bool bPara, bool bOdd)
{
    if( nLen <= 0 )
        return;
    if (bOdd)
        nLen = nLen - WW8SkipEven( mpStStrm );
    else
        nLen = nLen - WW8SkipOdd( mpStStrm );

    if( bPara ) // Grupx.Papx
        nLen = ImportUPX(nLen, true, bOdd);
    ImportUPX(nLen, false, bOdd);                   // Grupx.Chpx
}

WW8RStyle::WW8RStyle(WW8Fib& _rFib, SwWW8ImplReader* pI)
    : WW8Style(*pI->m_pTableStream, _rFib)
    , maSprmParser(_rFib)
    , mpIo(pI)
    , mpStStrm(pI->m_pTableStream)
    , mpStyRule(nullptr)
    , mpParaSprms(nullptr)
    , mnSprmsLen(0)
    , mnWwNumLevel(0)
    , mbTextColChanged(false)
    , mbFontChanged(false)
    , mbCJKFontChanged(false)
    , mbCTLFontChanged(false)
    , mbFSizeChanged(false)
    , mbFCTLSizeChanged(false)
    , mbWidowsChanged(false)
{
    mpIo->m_vColl.resize(m_cstd);
}

void WW8RStyle::Set1StyleDefaults()
{
    // see #i25247#, #i25561#, #i48064#, #i92341# for default font
    if (!mbCJKFontChanged)   // Style no CJK Font? set the default
        mpIo->SetNewFontAttr(m_ftcFE, true, RES_CHRATR_CJK_FONT);

    if (!mbCTLFontChanged)   // Style no CTL Font? set the default
        mpIo->SetNewFontAttr(m_ftcBi, true, RES_CHRATR_CTL_FONT);

    // western 2nd to make western charset conversion the default
    if (!mbFontChanged)      // Style has no Font? set the default,
        mpIo->SetNewFontAttr(m_ftcAsci, true, RES_CHRATR_FONT);

    if( !mpIo->m_bNoAttrImport )
    {
        // Style has no text color set, winword default is auto
        if ( !mbTextColChanged )
            mpIo->m_pCurrentColl->SetFormatAttr(SvxColorItem(COL_AUTO, RES_CHRATR_COLOR));

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !mbFSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            mpIo->m_pCurrentColl->SetFormatAttr(aAttr);
            aAttr.SetWhich(RES_CHRATR_CJK_FONTSIZE);
            mpIo->m_pCurrentColl->SetFormatAttr(aAttr);
        }

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !mbFCTLSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            aAttr.SetWhich(RES_CHRATR_CTL_FONTSIZE);
            mpIo->m_pCurrentColl->SetFormatAttr(aAttr);
        }

        if( !mbWidowsChanged )  // Widows ?
        {
            mpIo->m_pCurrentColl->SetFormatAttr( SvxWidowsItem( 2, RES_PARATR_WIDOWS ) );
            mpIo->m_pCurrentColl->SetFormatAttr( SvxOrphansItem( 2, RES_PARATR_ORPHANS ) );
        }
    }
}

bool WW8RStyle::PrepareStyle(SwWW8StyInf &rSI, ww::sti eSti, sal_uInt16 nThisStyle, sal_uInt16 nNextStyle)
{
    SwFormat* pColl;
    bool bStyExist;

    if (rSI.m_bColl)
    {
        // Para-Style
        sw::util::ParaStyleMapper::StyleResult aResult =
            mpIo->m_aParaStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }
    else
    {
        // Char-Style
        sw::util::CharStyleMapper::StyleResult aResult =
            mpIo->m_aCharStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }

    bool bImport = !bStyExist || mpIo->m_bNewDoc; // import content ?

    // Do not override character styles the list import code created earlier.
    if (bImport && bStyExist && rSI.GetOrgWWName().startsWith("WW8Num"))
        bImport = false;

    bool bOldNoImp = mpIo->m_bNoAttrImport;
    rSI.m_bImportSkipped = !bImport;

    if( !bImport )
        mpIo->m_bNoAttrImport = true;
    else
    {
        if (bStyExist)
        {
            pColl->ResetAllFormatAttr(); // #i73790# - method renamed
        }
        pColl->SetAuto(false);          // suggested by JP
    }                                   // but changes the UI
    mpIo->m_pCurrentColl = pColl;
    rSI.m_pFormat = pColl;                  // remember translation WW->SW
    rSI.m_bImportSkipped = !bImport;

    // Set Based on style
    sal_uInt16 j = rSI.m_nBase;
    if (j != nThisStyle && j < m_cstd )
    {
        SwWW8StyInf* pj = &mpIo->m_vColl[j];
        if (rSI.m_pFormat && pj->m_pFormat && rSI.m_bColl == pj->m_bColl)
        {
            rSI.m_pFormat->SetDerivedFrom( pj->m_pFormat );  // ok, set Based on
            rSI.m_eLTRFontSrcCharSet = pj->m_eLTRFontSrcCharSet;
            rSI.m_eRTLFontSrcCharSet = pj->m_eRTLFontSrcCharSet;
            rSI.m_eCJKFontSrcCharSet = pj->m_eCJKFontSrcCharSet;
            rSI.m_n81Flags = pj->m_n81Flags;
            rSI.m_n81BiDiFlags = pj->m_n81BiDiFlags;
            if (!rSI.IsWW8BuiltInHeadingStyle())
            {
                rSI.mnWW8OutlineLevel = pj->mnWW8OutlineLevel;
            }
            rSI.m_bParaAutoBefore = pj->m_bParaAutoBefore;
            rSI.m_bParaAutoAfter = pj->m_bParaAutoAfter;

            if (pj->m_xWWFly)
                rSI.m_xWWFly.reset(new WW8FlyPara(mpIo->m_bVer67, pj->m_xWWFly.get()));
        }
    }
    else if( mpIo->m_bNewDoc && bStyExist )
        rSI.m_pFormat->SetDerivedFrom();

    rSI.m_nFollow = nNextStyle;       // remember Follow

    mpStyRule = nullptr;                   // recreate if necessary
    mbTextColChanged = mbFontChanged = mbCJKFontChanged = mbCTLFontChanged =
        mbFSizeChanged = mbFCTLSizeChanged = mbWidowsChanged = false;
    mpIo->SetNCurrentColl( nThisStyle );
    mpIo->m_bStyNormal = nThisStyle == 0;
    return bOldNoImp;
}

void WW8RStyle::PostStyle(SwWW8StyInf const &rSI, bool bOldNoImp)
{
    // Reset attribute flags, because there are no style-ends.

    mpIo->m_bHasBorder = mpIo->m_bSpec = mpIo->m_bObj = mpIo->m_bSymbol = false;
    mpIo->m_nCharFormat = -1;

    // if style is based on nothing or base ignored
    if ((rSI.m_nBase >= m_cstd || mpIo->m_vColl[rSI.m_nBase].m_bImportSkipped) && rSI.m_bColl)
    {
        // If Char-Styles does not work
        // -> set hard WW-Defaults
        Set1StyleDefaults();
    }

    mpStyRule = nullptr;                   // to be on the safe side
    mpIo->m_bStyNormal = false;
    mpIo->SetNCurrentColl( 0 );
    mpIo->m_bNoAttrImport = bOldNoImp;
    // reset the list-remember-fields, if used when reading styles
    mpIo->m_nLFOPosition = USHRT_MAX;
    mpIo->m_nListLevel = WW8ListManager::nMaxLevel;
}

void WW8RStyle::Import1Style( sal_uInt16 nNr )
{
    if (nNr >= mpIo->m_vColl.size())
        return;

    SwWW8StyInf &rSI = mpIo->m_vColl[nNr];

    if( rSI.m_bImported || !rSI.m_bValid )
        return;

    rSI.m_bImported = true;                      // set flag now to avoid endless loops

    // valid and not NUL and not yet imported

    if( rSI.m_nBase < m_cstd && !mpIo->m_vColl[rSI.m_nBase].m_bImported )
        Import1Style( rSI.m_nBase );

    mpStStrm->Seek( rSI.m_nFilePos );

    sal_uInt16 nSkip;
    OUString sName;

    std::unique_ptr<WW8_STD> xStd(Read1Style(nSkip, &sName));// read Style

    if (xStd)
        rSI.SetOrgWWIdent( sName, xStd->sti );

    // either no Name or unused Slot or unknown Style

    if ( !xStd || sName.isEmpty() || ((1 != xStd->sgc) && (2 != xStd->sgc)) )
    {
        nSkip = std::min<sal_uInt64>(nSkip, mpStStrm->remainingSize());
        mpStStrm->Seek(mpStStrm->Tell() + nSkip);
        return;
    }

    bool bOldNoImp = PrepareStyle(rSI, static_cast<ww::sti>(xStd->sti), nNr, xStd->istdNext);

    // if something is interpreted wrong, this should make it work again
    long nPos = mpStStrm->Tell();

    //Variable parts of the STD start at even byte offsets, but "inside
    //the STD", which I take to meaning even in relation to the starting
    //position of the STD, which matches findings in #89439#, generally it
    //doesn't matter as the STSHI starts off nearly always on an even
    //offset

    //Import of the Style Contents
    ImportGrupx(nSkip, xStd->sgc == 1, rSI.m_nFilePos & 1);

    PostStyle(rSI, bOldNoImp);

    mpStStrm->Seek( nPos+nSkip );
}

void WW8RStyle::RecursiveReg(sal_uInt16 nNr)
{
    if (nNr >= mpIo->m_vColl.size())
        return;

    SwWW8StyInf &rSI = mpIo->m_vColl[nNr];
    if( rSI.m_bImported || !rSI.m_bValid )
        return;

    rSI.m_bImported = true;

    if( rSI.m_nBase < m_cstd && !mpIo->m_vColl[rSI.m_nBase].m_bImported )
        RecursiveReg(rSI.m_nBase);

    mpIo->RegisterNumFormatOnStyle(nNr);

}

/*
 After all styles are imported then we can recursively apply numbering
 styles to them, and change their tab stop settings if they turned out
 to have special first line indentation.
*/
void WW8RStyle::PostProcessStyles()
{
    sal_uInt16 i;
    /*
     Clear all imported flags so that we can recursively apply numbering
     formats and use it to mark handled ones
    */
    for (i=0; i < m_cstd; ++i)
        mpIo->m_vColl[i].m_bImported = false;

    /*
     Register the num formats and tabstop changes on the styles recursively.
    */

    /*
     In the same loop apply the tabstop changes required because we need to
     change their location if there's a special indentation for the first line,
     By avoiding making use of each styles margins during reading of their
     tabstops we don't get problems with doubly adjusting tabstops that
     are inheritied.
    */
    for (i=0; i < m_cstd; ++i)
    {
        if (mpIo->m_vColl[i].m_bValid)
        {
            RecursiveReg(i);
        }
    }
}

void WW8RStyle::ScanStyles()        // investigate style dependencies
{                                   // and detect Filepos for each Style
    for (sal_uInt16 i = 0; i < m_cstd; ++i)
    {
        SwWW8StyInf &rSI = mpIo->m_vColl[i];

        rSI.m_nFilePos = mpStStrm->Tell();        // remember FilePos
        sal_uInt16 nSkip;
        std::unique_ptr<WW8_STD> xStd(Read1Style(nSkip, nullptr));  // read STD
        rSI.m_bValid = xStd.get() != nullptr;
        if (rSI.m_bValid)
        {
            rSI.m_nBase = xStd->istdBase; // remember Basis
            rSI.m_bColl = xStd->sgc == 1; // Para-Style
        }
        else
            rSI = SwWW8StyInf();

        xStd.reset();
        nSkip = std::min<sal_uInt64>(nSkip, mpStStrm->remainingSize());
        mpStStrm->Seek(mpStStrm->Tell() + nSkip);              // skip Names and Sprms
    }
}

std::vector<sal_uInt8> ChpxToSprms(const Word2CHPX &rChpx)
{
    std::vector<sal_uInt8> aRet;

    aRet.push_back(60);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fBold) );

    aRet.push_back(61);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fItalic) );

    aRet.push_back(62);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fStrike) );

    aRet.push_back(63);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fOutline) );

    aRet.push_back(65);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fSmallCaps) );

    aRet.push_back(66);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fCaps) );

    aRet.push_back(67);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fVanish) );

    if (rChpx.fsFtc)
    {
        aRet.push_back(68);
        SVBT16 a;
        ShortToSVBT16(rChpx.ftc, a);
        aRet.push_back(a[1]);
        aRet.push_back(a[0]);
    }

    if (rChpx.fsKul)
    {
        aRet.push_back(69);
        aRet.push_back(rChpx.kul);
    }

    if (rChpx.fsLid)
    {
        aRet.push_back(72);
        SVBT16 a;
        ShortToSVBT16(rChpx.lid, a);
        aRet.push_back(a[1]);
        aRet.push_back(a[0]);
    }

    if (rChpx.fsIco)
    {
        aRet.push_back(73);
        aRet.push_back(rChpx.ico);
    }

    if (rChpx.fsHps)
    {
        aRet.push_back(74);

        SVBT16 a;
        ShortToSVBT16(rChpx.hps, a);
        aRet.push_back(a[0]);
    }

    if (rChpx.fsPos)
    {
        aRet.push_back(76);
        aRet.push_back(rChpx.hpsPos);
    }

    aRet.push_back(80);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fBoldBi) );

    aRet.push_back(81);
    aRet.push_back( static_cast< sal_uInt8 >(128 + rChpx.fItalicBi) );

    if (rChpx.fsFtcBi)
    {
        aRet.push_back(82);
        SVBT16 a;
        ShortToSVBT16(rChpx.fsFtcBi, a);
        aRet.push_back(a[1]);
        aRet.push_back(a[0]);
    }

    if (rChpx.fsLidBi)
    {
        aRet.push_back(83);
        SVBT16 a;
        ShortToSVBT16(rChpx.lidBi, a);
        aRet.push_back(a[1]);
        aRet.push_back(a[0]);
    }

    if (rChpx.fsIcoBi)
    {
        aRet.push_back(84);
        aRet.push_back(rChpx.icoBi);
    }

    if (rChpx.fsHpsBi)
    {
        aRet.push_back(85);
        SVBT16 a;
        ShortToSVBT16(rChpx.hpsBi, a);
        aRet.push_back(a[1]);
        aRet.push_back(a[0]);
    }

    return aRet;
}

Word2CHPX ReadWord2Chpx(SvStream &rSt, std::size_t nOffset, sal_uInt8 nSize)
{
    Word2CHPX aChpx;

    if (!nSize)
        return aChpx;

    rSt.Seek(nOffset);

    sal_uInt8 nCount=0;

    while (true)
    {
        sal_uInt8 nFlags8;
        rSt.ReadUChar( nFlags8 );
        nCount++;

        aChpx.fBold = nFlags8 & 0x01;
        aChpx.fItalic = (nFlags8 & 0x02) >> 1;
        aChpx.fRMarkDel = (nFlags8 & 0x04) >> 2;
        aChpx.fOutline = (nFlags8 & 0x08) >> 3;
        aChpx.fFieldVanish = (nFlags8 & 0x10) >> 4;
        aChpx.fSmallCaps = (nFlags8 & 0x20) >> 5;
        aChpx.fCaps = (nFlags8 & 0x40) >> 6;
        aChpx.fVanish = (nFlags8 & 0x80) >> 7;

        if (nCount >= nSize) break;
        rSt.ReadUChar( nFlags8 );
        nCount++;

        aChpx.fRMark = nFlags8 & 0x01;
        aChpx.fSpec = (nFlags8 & 0x02) >> 1;
        aChpx.fStrike = (nFlags8 & 0x04) >> 2;
        aChpx.fObj = (nFlags8 & 0x08) >> 3;
        aChpx.fBoldBi = (nFlags8 & 0x10) >> 4;
        aChpx.fItalicBi = (nFlags8 & 0x20) >> 5;
        aChpx.fBiDi = (nFlags8 & 0x40) >> 6;
        aChpx.fDiacUSico = (nFlags8 & 0x80) >> 7;

        if (nCount >= nSize) break;
        rSt.ReadUChar( nFlags8 );
        nCount++;

        aChpx.fsIco = nFlags8 & 0x01;
        aChpx.fsFtc = (nFlags8 & 0x02) >> 1;
        aChpx.fsHps = (nFlags8 & 0x04) >> 2;
        aChpx.fsKul = (nFlags8 & 0x08) >> 3;
        aChpx.fsPos = (nFlags8 & 0x10) >> 4;
        aChpx.fsSpace = (nFlags8 & 0x20) >> 5;
        aChpx.fsLid = (nFlags8 & 0x40) >> 6;
        aChpx.fsIcoBi = (nFlags8 & 0x80) >> 7;

        if (nCount >= nSize) break;
        rSt.ReadUChar( nFlags8 );
        nCount++;

        aChpx.fsFtcBi = nFlags8 & 0x01;
        aChpx.fsHpsBi = (nFlags8 & 0x02) >> 1;
        aChpx.fsLidBi = (nFlags8 & 0x04) >> 2;

        if (nCount >= nSize) break;
        rSt.ReadUInt16( aChpx.ftc );
        nCount+=2;

        if (nCount >= nSize) break;
        rSt.ReadUInt16( aChpx.hps );
        nCount+=2;

        if (nCount >= nSize) break;
        rSt.ReadUChar( nFlags8 );
        nCount++;

        aChpx.qpsSpace = nFlags8 & 0x3F;
        aChpx.fSysVanish = (nFlags8 & 0x40) >> 6;
        aChpx.fNumRun = (nFlags8 & 0x80) >> 7;

        if (nCount >= nSize) break;
        rSt.ReadUChar( nFlags8 );
        nCount++;

        aChpx.ico = nFlags8 & 0x1F;
        aChpx.kul = (nFlags8 & 0xE0) >> 5;

        if (nCount >= nSize) break;
        rSt.ReadUChar( aChpx.hpsPos );
        nCount++;

        if (nCount >= nSize) break;
        rSt.ReadUChar( aChpx.icoBi );
        nCount++;

        if (nCount >= nSize) break;
        rSt.ReadUInt16( aChpx.lid );
        nCount+=2;

        if (nCount >= nSize) break;
        rSt.ReadUInt16( aChpx.ftcBi );
        nCount+=2;

        if (nCount >= nSize) break;
        rSt.ReadUInt16( aChpx.hpsBi );
        nCount+=2;

        if (nCount >= nSize) break;
        rSt.ReadUInt16( aChpx.lidBi );
        nCount+=2;

        if (nCount >= nSize) break;
        rSt.ReadUInt32( aChpx.fcPic );
        nCount+=4;

        break;
    }

    rSt.SeekRel(nSize-nCount);
    return aChpx;
}

namespace
{
    struct pxoffset { std::size_t mnOffset; sal_uInt8 mnSize; };
}

void WW8RStyle::ImportOldFormatStyles()
{
    for (sal_uInt16 i=0; i < m_cstd; ++i)
    {
        mpIo->m_vColl[i].m_bColl = true;
        //every chain must end eventually at the null style (style code 222)
        mpIo->m_vColl[i].m_nBase = 222;
    }

    rtl_TextEncoding eStructChrSet = WW8Fib::GetFIBCharset(
        mpIo->m_xWwFib->m_chseTables, mpIo->m_xWwFib->m_lid);

    sal_uInt16 cstcStd(0);
    m_rStream.ReadUInt16( cstcStd );

    size_t nMaxByteCount = m_rStream.remainingSize();
    sal_uInt16 cbName(0);
    m_rStream.ReadUInt16(cbName);
    if (cbName > nMaxByteCount)
    {
        SAL_WARN("sw.ww8", "WW8RStyle::ImportOldFormatStyles: truncating out of range "
            << cbName << " to " << nMaxByteCount);
        cbName = nMaxByteCount;
    }
    sal_uInt16 nByteCount = 2;
    sal_uInt16 stcp=0;
    while (nByteCount < cbName)
    {
        sal_uInt8 nCount(0);
        m_rStream.ReadUChar( nCount );
        nByteCount++;

        sal_uInt8 stc = static_cast< sal_uInt8 >((stcp - cstcStd) & 255);
        if (stc >=mpIo->m_vColl.size())
            continue;

        SwWW8StyInf &rSI = mpIo->m_vColl[stc];
        OUString sName;

        if (nCount != 0xFF)    // undefined style
        {
            if (nCount != 0)   // user style
            {
                OString aTmp = read_uInt8s_ToOString(m_rStream, nCount);
                nByteCount += aTmp.getLength();
                sName = OStringToOUString(aTmp, eStructChrSet);
            }
            rSI.m_bImported = true;
        }

        if (sName.isEmpty())
        {
            ww::sti eSti = ww::GetCanonicalStiFromStc(stc);
            if (const sal_Char *pStr = GetEnglishNameFromSti(eSti))
                sName = OUString(pStr, strlen(pStr), RTL_TEXTENCODING_ASCII_US);
        }

        if (sName.isEmpty())
            sName = "Unknown Style: " + OUString::number(stc);

        rSI.SetOrgWWIdent(sName, stc);
        stcp++;
    }

    sal_uInt16 nStyles=stcp;

    std::vector<pxoffset> aCHPXOffsets(stcp);
    nMaxByteCount = m_rStream.remainingSize();
    sal_uInt16 cbChpx(0);
    m_rStream.ReadUInt16(cbChpx);
    if (cbChpx > nMaxByteCount)
    {
        SAL_WARN("sw.ww8", "WW8RStyle::ImportOldFormatStyles: truncating out of range "
            << cbChpx << " to " << nMaxByteCount);
        cbChpx = nMaxByteCount;
    }
    nByteCount = 2;
    stcp=0;
    std::vector< std::vector<sal_uInt8> > aConvertedChpx;
    while (nByteCount < cbChpx)
    {
        if (stcp == aCHPXOffsets.size())
        {
            //more data than style slots, skip remainder
            m_rStream.SeekRel(cbChpx-nByteCount);
            break;
        }

        sal_uInt8 cb(0);
        m_rStream.ReadUChar( cb );
        nByteCount++;

        aCHPXOffsets[stcp].mnSize = 0;

        if (cb != 0xFF)
        {
            sal_uInt8 nRemainder = cb;

            aCHPXOffsets[stcp].mnOffset = m_rStream.Tell();
            aCHPXOffsets[stcp].mnSize = nRemainder;

            Word2CHPX aChpx = ReadWord2Chpx(m_rStream, aCHPXOffsets[stcp].mnOffset,
                aCHPXOffsets[stcp].mnSize);
            aConvertedChpx.push_back( ChpxToSprms(aChpx) );

            nByteCount += nRemainder;
        }
        else
            aConvertedChpx.emplace_back( );

        ++stcp;
    }

    std::vector<pxoffset> aPAPXOffsets(stcp);
    nMaxByteCount = m_rStream.remainingSize();
    sal_uInt16 cbPapx(0);
    m_rStream.ReadUInt16(cbPapx);
    if (cbPapx > nMaxByteCount)
    {
        SAL_WARN("sw.ww8", "WW8RStyle::ImportOldFormatStyles: truncating out of range "
            << cbPapx << " to " << nMaxByteCount);
        cbPapx = nMaxByteCount;
    }
    nByteCount = 2;
    stcp=0;
    while (nByteCount < cbPapx)
    {
        if (stcp == aPAPXOffsets.size())
        {
            m_rStream.SeekRel(cbPapx-nByteCount);
            break;
        }

        sal_uInt8 cb(0);
        m_rStream.ReadUChar( cb );
        nByteCount++;

        aPAPXOffsets[stcp].mnSize = 0;

        if (cb != 0xFF)
        {
            sal_uInt8 stc2(0);
            m_rStream.ReadUChar( stc2 );
            m_rStream.SeekRel(6);
            nByteCount+=7;
            sal_uInt8 nRemainder = cb-7;

            aPAPXOffsets[stcp].mnOffset = m_rStream.Tell();
            aPAPXOffsets[stcp].mnSize = nRemainder;

            m_rStream.SeekRel(nRemainder);
            nByteCount += nRemainder;
        }

        ++stcp;
    }

    sal_uInt16 iMac(0);
    m_rStream.ReadUInt16( iMac );

    if (iMac > nStyles) iMac = nStyles;

    for (stcp = 0; stcp < iMac; ++stcp)
    {
        sal_uInt8 stcNext(0), stcBase(0);
        m_rStream.ReadUChar( stcNext );
        m_rStream.ReadUChar( stcBase );

        sal_uInt8 stc = static_cast< sal_uInt8 >((stcp - cstcStd) & 255);

        /*
          #i64557# style based on itself
          every chain must end eventually at the null style (style code 222)
        */
        if (stc == stcBase)
            stcBase = 222;

        SwWW8StyInf &rSI = mpIo->m_vColl[stc];
        rSI.m_nBase = stcBase;

        ww::sti eSti = ww::GetCanonicalStiFromStc(stc);

        if (eSti == ww::stiNil)
            continue;

        if (stcp >= aPAPXOffsets.size())
            continue;

        rSI.m_bValid = true;

        if (ww::StandardStiIsCharStyle(eSti) && !aPAPXOffsets[stcp].mnSize)
            mpIo->m_vColl[stc].m_bColl = false;

        bool bOldNoImp = PrepareStyle(rSI, eSti, stc, stcNext);

        ImportSprms(aPAPXOffsets[stcp].mnOffset, aPAPXOffsets[stcp].mnSize,
            true);

        if (aConvertedChpx[stcp].size() > 0)
            ImportSprms(&(aConvertedChpx[stcp][0]),
                        static_cast< short >(aConvertedChpx[stcp].size()),
                        false);

        PostStyle(rSI, bOldNoImp);
    }
}

void WW8RStyle::ImportNewFormatStyles()
{
    ScanStyles();                       // Scan Based On

    for (sal_uInt16 i = 0; i < m_cstd; ++i) // import Styles
        if (mpIo->m_vColl[i].m_bValid)
            Import1Style( i );
}

void WW8RStyle::Import()
{
    mpIo->m_pDfltTextFormatColl  = mpIo->m_rDoc.GetDfltTextFormatColl();
    mpIo->m_pStandardFormatColl =
        mpIo->m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD, false);

    if( mpIo->m_nIniFlags & WW8FL_NO_STYLES )
        return;

    if (mpIo->m_xWwFib->GetFIBVersion() <= ww::eWW2)
        ImportOldFormatStyles();
    else
        ImportNewFormatStyles();

    for (sal_uInt16 i = 0; i < m_cstd; ++i)
    {
        // Follow chain
        SwWW8StyInf* pi = &mpIo->m_vColl[i];
        sal_uInt16 j = pi->m_nFollow;
        if( j < m_cstd )
        {
            SwWW8StyInf* pj = &mpIo->m_vColl[j];
            if ( j != i                             // rational Index ?
                 && pi->m_pFormat                        // Format ok ?
                 && pj->m_pFormat                        // Derived-Format ok ?
                 && pi->m_bColl                       // only possible for paragraph templates (WW)
                 && pj->m_bColl ){                    // identical Type ?
                    static_cast<SwTextFormatColl*>(pi->m_pFormat)->SetNextTextFormatColl(
                     *static_cast<SwTextFormatColl*>(pj->m_pFormat) );    // ok, register
            }
        }
    }

    // Missing special handling for default character template
    // "Absatz-Standardschriftart" ( Style-ID 65 ).
    // That is empty by default ( WW6 dt and US ) and not changeable
    // via WW-UI so this does not matter.
    // This could be done by:
    //  if( bNew ) rDoc.SetDefault( pDefCharFormat->GetAttrSet() );

    // for e.g. tables an always valid Std-Style is necessary

    if( mpIo->StyleExists(0) && !mpIo->m_vColl.empty() &&
        mpIo->m_vColl[0].m_pFormat && mpIo->m_vColl[0].m_bColl && mpIo->m_vColl[0].m_bValid )
        mpIo->m_pDfltTextFormatColl = static_cast<SwTextFormatColl*>(mpIo->m_vColl[0].m_pFormat);
    else
        mpIo->m_pDfltTextFormatColl = mpIo->m_rDoc.GetDfltTextFormatColl();

    // set Hyphenation flag on BASIC para-style
    if (mpIo->m_bNewDoc && mpIo->m_pStandardFormatColl)
    {
        if (mpIo->m_xWDop->fAutoHyphen
            && SfxItemState::SET != mpIo->m_pStandardFormatColl->GetItemState(
                                            RES_PARATR_HYPHENZONE, false) )
        {
            SvxHyphenZoneItem aAttr(true, RES_PARATR_HYPHENZONE);
            aAttr.GetMinLead()    = 2;
            aAttr.GetMinTrail()   = 2;
            aAttr.GetMaxHyphens() = 0;

            mpIo->m_pStandardFormatColl->SetFormatAttr( aAttr );
        }

        /*
        Word defaults to ltr not from environment like writer. Regardless of
        the page/sections rtl setting the standard style lack of rtl still
        means ltr
        */
        if (SfxItemState::SET != mpIo->m_pStandardFormatColl->GetItemState(RES_FRAMEDIR,
            false))
        {
           mpIo->m_pStandardFormatColl->SetFormatAttr(
                SvxFrameDirectionItem(SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR));
        }
    }

    // we do not read styles anymore:
    mpIo->m_pCurrentColl = nullptr;
}

rtl_TextEncoding SwWW8StyInf::GetCharSet() const
{
    if ((m_pFormat) && (m_pFormat->GetFrameDir().GetValue() == SvxFrameDirection::Horizontal_RL_TB))
        return m_eRTLFontSrcCharSet;
    return m_eLTRFontSrcCharSet;
}

rtl_TextEncoding SwWW8StyInf::GetCJKCharSet() const
{
    if ((m_pFormat) && (m_pFormat->GetFrameDir().GetValue() == SvxFrameDirection::Horizontal_RL_TB))
        return m_eRTLFontSrcCharSet;
    return m_eCJKFontSrcCharSet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
