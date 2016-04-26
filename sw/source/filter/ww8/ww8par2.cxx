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

#include <boost/noncopyable.hpp>
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
#include <charfmt.hxx>
#include <SwStyleNameMapper.hxx>
#include <fltshell.hxx>
#include <fmtanchr.hxx>
#include <fmtrowsplt.hxx>
#include <fmtfollowtextflow.hxx>
#include <numrule.hxx>
#include <sprmids.hxx>
#include <wwstyles.hxx>
#include "writerhelper.hxx"
#include "ww8struc.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"

#include <frmatr.hxx>

#include <iostream>
#include <memory>

using namespace ::com::sun::star;

class WW8SelBoxInfo
    : public std::vector<SwTableBox*>, private boost::noncopyable
{
public:
    short nGroupXStart;
    short nGroupWidth;
    bool bGroupLocked;

    WW8SelBoxInfo(short nXCenter, short nWidth)
        : nGroupXStart( nXCenter ), nGroupWidth( nWidth ), bGroupLocked(false)
    {}
};

WW8TabBandDesc::WW8TabBandDesc()
{
    memset(this, 0, sizeof(*this));
    for (size_t i = 0; i < sizeof(maDirections)/sizeof(sal_uInt16); ++i)
        maDirections[i] = 4;
}

WW8TabBandDesc::~WW8TabBandDesc()
{
    delete[] pTCs;
    delete[] pSHDs;
    delete[] pNewSHDs;
}

class WW8TabDesc: private boost::noncopyable
{
    std::vector<OUString> aNumRuleNames;
    sw::util::RedlineStack *mpOldRedlineStack;

    SwWW8ImplReader* pIo;

    WW8TabBandDesc* pFirstBand;
    WW8TabBandDesc* pActBand;

    SwPosition* pTmpPos;

    SwTableNode* pTableNd;            // table node
    const SwTableLines* pTabLines;  // row array of node
    SwTableLine* pTabLine;          // current row
    SwTableBoxes* pTabBoxes;        // boxes array in current row
    SwTableBox* pTabBox;            // current cell

    std::vector<std::unique_ptr<WW8SelBoxInfo>> m_MergeGroups;   // list of all cells to be merged

    WW8_TCell* pAktWWCell;

    short nRows;
    short nDefaultSwCols;
    short nBands;
    short nMinLeft;
    short nConvertedLeft;
    short nMaxRight;
    short nSwWidth;
    short nPreferredWidth;
    short nOrgDxaLeft;

    bool bOk;
    bool bClaimLineFormat;
    sal_Int16 eOri;
    bool bIsBiDi;
                                // 2. common admin info
    short nAktRow;
    short nAktBandRow;          // SW: row of current band
                                // 3. admin info for writer
    short nAktCol;

    sal_uInt16 nRowsToRepeat;

    // 4. methods

    sal_uInt16 GetLogicalWWCol() const;
    void SetTabBorders( SwTableBox* pBox, short nIdx );
    void SetTabShades( SwTableBox* pBox, short nWwIdx );
    void SetTabVertAlign( SwTableBox* pBox, short nWwIdx );
    void SetTabDirection( SwTableBox* pBox, short nWwIdx );
    void CalcDefaults();
    bool SetPamInCell(short nWwCol, bool bPam);
    void InsertCells( short nIns );
    void AdjustNewBand();

    WW8SelBoxInfo* FindMergeGroup(short nX1, short nWidth, bool bExact);

    // single box - maybe used in a merge group
    // (the merge groups are processed later at once)
    SwTableBox* UpdateTableMergeGroup(WW8_TCell& rCell,
        WW8SelBoxInfo* pActGroup, SwTableBox* pActBox, sal_uInt16 nCol  );
    void StartMiserableHackForUnsupportedDirection(short nWwCol);
    void EndMiserableHackForUnsupportedDirection(short nWwCol);

public:
    const SwTable* pTable;          // table
    SwPosition* pParentPos;
    SwFlyFrameFormat* pFlyFormat;
    SfxItemSet aItemSet;
    bool IsValidCell(short nCol) const;
    bool InFirstParaInCell() const;

    WW8TabDesc( SwWW8ImplReader* pIoClass, WW8_CP nStartCp );
    bool Ok() const { return bOk; }
    void CreateSwTable(SvxULSpaceItem* pULSpaceItem = nullptr);
    void UseSwTable();
    void SetSizePosition(SwFrameFormat* pFrameFormat);
    void TableCellEnd();
    void MoveOutsideTable();
    void ParkPaM();
    void FinishSwTable();
    void MergeCells();
    short GetMinLeft() const { return nConvertedLeft; }
    ~WW8TabDesc();

    const WW8_TCell* GetAktWWCell() const { return pAktWWCell; }
    short GetAktCol() const { return nAktCol; }
    // find name of numrule valid for current WW-COL
    OUString GetNumRuleName() const;
    void SetNumRuleName( const OUString& rName );

    sw::util::RedlineStack* getOldRedlineStack(){ return mpOldRedlineStack; }
};

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
            (void) bResult; // unused in non-debug
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
        sChar += OUString(pText->GetText()[--nPos]);
        m_pPaM->SetMark();
        --m_pPaM->GetMark()->nContent;
        m_rDoc.getIDocumentContentOperations().DeleteRange( *m_pPaM );
        m_pPaM->DeleteMark();
        SwFormatFootnote aFootnote(rDesc.meType == MAN_EDN);
        pFN = pText->InsertItem(aFootnote, nPos, nPos);
    }
    OSL_ENSURE(pFN, "Probleme beim Anlegen des Fussnoten-Textes");
    if (pFN)
    {

        SwPosition aTmpPos( *m_pPaM->GetPoint() );    // remember old cursor position
        WW8PLCFxSaveAll aSave;
        m_pPlcxMan->SaveAllPLCFx( aSave );
        WW8PLCFMan* pOldPlcxMan = m_pPlcxMan;

        const SwNodeIndex* pSttIdx = static_cast<SwTextFootnote*>(pFN)->GetStartNode();
        OSL_ENSURE(pSttIdx, "Probleme beim Anlegen des Fussnoten-Textes");

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
                m_pPaM->GetPoint()->nContent.Assign( pTNd, 0 );
                m_pPaM->SetMark();
                // Strip out tabs we may have inserted on export #i24762#
                if (rText.getLength() > 1 && rText[1] == 0x09)
                    ++m_pPaM->GetMark()->nContent;
                ++m_pPaM->GetMark()->nContent;
                m_pReffingStck->Delete(*m_pPaM);
                m_rDoc.getIDocumentContentOperations().DeleteRange( *m_pPaM );
                m_pPaM->DeleteMark();
            }
        }

        *m_pPaM->GetPoint() = aTmpPos;        // restore Cursor

        m_pPlcxMan = pOldPlcxMan;             // Restore attributes
        m_pPlcxMan->RestoreAllPLCFx( aSave );
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
        if (m_pPlcxMan->GetEdn())
            aDesc.mbAutoNum = 0 != *static_cast<short const *>(m_pPlcxMan->GetEdn()->GetData());
    }
    else
    {
        aDesc.meType = MAN_FTN;
        if (m_pPlcxMan->GetFootnote())
            aDesc.mbAutoNum = 0 != *static_cast<short const *>(m_pPlcxMan->GetFootnote()->GetData());
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
    bool bReadRes(false);
    WW8PLCFxDesc aPrevRes;

    while (pPap->HasFkp() && rStartCp != WW8_CP_MAX)
    {
        if (pPap->Where() != WW8_CP_MAX)
        {
            const sal_uInt8* pB = pPap->HasSprm(TabRowSprm(nLevel));
            if (pB && *pB == 1)
            {
                const sal_uInt8 *pLevel = nullptr;
                if (nullptr != (pLevel = pPap->HasSprm(0x6649)))
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
        if (bReadRes && aRes.nEndPos == aPrevRes.nEndPos && aRes.nStartPos == aPrevRes.nStartPos)
        {
            SAL_WARN("sw.ww8", "SearchRowEnd, loop in paragraph property chain");
            break;
        }
        bReadRes = true;
        aPrevRes = aRes;
        //Update our aRes to get the new starting point of the next properties
        rStartCp = aRes.nEndPos;
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
    if (!m_bTxbxFlySection && m_nAktColl < m_vColl.size())
        aRet.mpStyleApo = StyleExists(m_nAktColl) ? m_vColl[m_nAktColl].m_pWWFly : nullptr;

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

    aRet.m_bHasSprm37 = m_pPlcxMan->HasParaSprm( m_bVer67 ? 37 : 0x2423 );
    const sal_uInt8 *pSrpm29 = m_pPlcxMan->HasParaSprm( m_bVer67 ? 29 : 0x261B );
    aRet.m_bHasSprm29 = pSrpm29 != nullptr;
    aRet.m_nSprm29 = pSrpm29 ? *pSrpm29 : 0;

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
                if (!m_pTableDesc)
                {
                    OSL_ENSURE(m_pTableDesc, "What!");
                    bTestAllowed = false;
                }
                else
                {
                    // #i39468#
                    // If current cell isn't valid, the test is allowed.
                    // The cell isn't valid, if e.g. there is a new row
                    // <pTableDesc->nAktRow> >= <pTableDesc->pTabLines->Count()>
                    bTestAllowed =
                        m_pTableDesc->GetAktCol() == 0 &&
                        ( !m_pTableDesc->IsValidCell( m_pTableDesc->GetAktCol() ) ||
                          m_pTableDesc->InFirstParaInCell() );
                }
            }
        }
    }

    if (!bTestAllowed)
        return aRet;

    aRet.mbStartApo = bNowApo && !InAnyApo(); // APO-start
    aRet.mbStopApo = InEqualOrHigherApo(nCellLevel) && !bNowApo;  // APO-end

    //If it happens that we are in a table, then if its not the first cell
    //then any attributes that might otherwise cause the contents to jump
    //into another frame don't matter, a table row sticks together as one
    //unit no matter what else happens. So if we are not in a table at
    //all, or if we are in the first cell then test that the last frame
    //data is the same as the current one
    if (bNowApo && InEqualApo(nCellLevel))
    {
        // two bordering eachother
        if (!TestSameApo(aRet, pTabPos))
            aRet.mbStopApo = aRet.mbStartApo = true;
    }

    return aRet;
}

// helper methods for outline, numbering and bullets

static void SetBaseAnlv(SwNumFormat &rNum, WW8_ANLV const &rAV, sal_uInt8 nSwLevel )
{
    static const SvxExtNumType eNumA[8] = { SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC };

    static const SvxAdjust eAdjA[4] = { SVX_ADJUST_LEFT,
        SVX_ADJUST_RIGHT, SVX_ADJUST_LEFT, SVX_ADJUST_LEFT };
    if (rAV.nfc < 8) {
        rNum.SetNumberingType( static_cast< sal_Int16 >(eNumA[ rAV.nfc ] ));
    } else {
        sal_Int16 nType= style::NumberingType::ARABIC;
        switch( rAV.nfc ) {
        case 14:
        case 19:nType = style::NumberingType::FULLWIDTH_ARABIC;    break;
        case 30:nType = style::NumberingType::TIAN_GAN_ZH;    break;
        case 31:nType = style::NumberingType::DI_ZI_ZH;    break;
        case 35:
        case 36:
        case 37:
        case 39:
            nType = style::NumberingType::NUMBER_LOWER_ZH;    break;
        case 34:nType = style::NumberingType::NUMBER_UPPER_ZH_TW;break;
        case 38:nType = style::NumberingType::NUMBER_UPPER_ZH;    break;
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
        rNum.SetNumberingType( nType );
    }

    if ((rAV.aBits1 & 0x4) >> 2)
    {
        rNum.SetIncludeUpperLevels(nSwLevel + 1);
    }
    rNum.SetStart( SVBT16ToShort( rAV.iStartAt ) );
    rNum.SetNumAdjust( eAdjA[ rAV.aBits1 & 0x3] );

    rNum.SetCharTextDistance( SVBT16ToShort( rAV.dxaSpace ) );
    sal_Int16 nIndent = std::abs((sal_Int16)SVBT16ToShort( rAV.dxaIndent ));
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
    const sal_uInt8* pText, bool bOutline)
{
    bool bInsert = false;                       // Default
    rtl_TextEncoding eCharSet = m_eStructCharSet;

    const WW8_FFN* pF = m_pFonts->GetFont(SVBT16ToShort(rAV.ftc)); // FontInfo
    bool bListSymbol = pF && ( pF->chs == 2 );      // Symbol/WingDings/...

    OUString sText;
    if (m_bVer67)
    {
        sText = OUString(reinterpret_cast<char const *>(pText), rAV.cbTextBefore + rAV.cbTextAfter, eCharSet);
    }
    else
    {
        for(sal_Int32 i = 0; i < rAV.cbTextBefore + rAV.cbTextAfter; ++i, pText += 2)
        {
            sText += OUString(sal_Unicode(SVBT16ToShort(*reinterpret_cast<SVBT16 const *>(pText))));
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
                aFont.SetName( aName );
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
// The characters before and after multipe digits do not apply because
// those are handled different by the writer and the result is in most
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
        m_bAktAND_fNumberAcross = 0 != pAD->fNumberAcross;
        WW8_ANLV const &rAV = pAD->eAnlv;
        SetBaseAnlv(aNF, rAV, nSwLevel);                    // set the base format
        SetAnlvStrings(aNF, rAV, pAD->rgchAnld, bOutLine ); // set the rest
    }
    pNumR->Set(nSwLevel, aNF);
}

// chapter numbering and bullets

// Chapter numbering happens in the style definition.
// Sprm 13 provides the level, Sprm 12 the content.

SwNumRule* SwWW8ImplReader::GetStyRule()
{
    if( m_pStyles->pStyRule )         // Bullet-Style already present
        return m_pStyles->pStyRule;

    const OUString aBaseName("WW8StyleNum");
    const OUString aName( m_rDoc.GetUniqueNumRuleName( &aBaseName, false) );

    // #i86652#
    sal_uInt16 nRul = m_rDoc.MakeNumRule( aName, nullptr, false,
                                    SvxNumberFormat::LABEL_ALIGNMENT );
    m_pStyles->pStyRule = m_rDoc.GetNumRuleTable()[nRul];
    // Auto == false-> Nummerierungsvorlage
    m_pStyles->pStyRule->SetAutoRule(false);

    return m_pStyles->pStyRule;
}

// Sprm 13
void SwWW8ImplReader::Read_ANLevelNo( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    m_nSwNumLevel = 0xff; // Default: invalid

    if( nLen <= 0 )
        return;

    // StyleDef ?
    if( m_pAktColl )
    {
        // only for SwTextFormatColl, not CharFormat
        // WW: 0 = no Numbering
        SwWW8StyInf * pColl = GetStyle(m_nAktColl);
        if (pColl != nullptr && pColl->m_bColl && *pData)
        {
            // Range WW:1..9 -> SW:0..8 no bullets / numbering

            if (*pData <= MAXLEVEL && *pData <= 9)
            {
                m_nSwNumLevel = *pData - 1;
                if (!m_bNoAttrImport)
                    static_cast<SwTextFormatColl*>(m_pAktColl)->AssignToListLevelOfOutlineStyle( m_nSwNumLevel );
                    // For WW-NoNumbering also NO_NUMBERING could be used.
                    // ( For normal numberierung NO_NUM has to be used:
                    //   NO_NUM : pauses numbering,
                    //   NO_NUMBERING : no numbering at all )

            }
            else if( *pData == 10 || *pData == 11 )
            {
                // remember type, the rest happens at Sprm 12
                m_pStyles->nWwNumLevel = *pData;
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
    SwWW8StyInf * pStyInf = GetStyle(m_nAktColl);
    if( !m_pAktColl || nLen <= 0                       // only for Styledef
        || (pStyInf && !pStyInf->m_bColl)              // ignore  CharFormat ->
        || ( m_nIniFlags & WW8FL_NO_OUTLINE ) )
    {
        m_nSwNumLevel = 0xff;
        return;
    }

    if( m_nSwNumLevel <= MAXLEVEL         // Value range mapping WW:1..9 -> SW:0..8
        && m_nSwNumLevel <= 9 ){          // No Bullets or Numbering

        // If NumRuleItems were set, either directly or through inheritance, disable them now
        m_pAktColl->SetFormatAttr( SwNumRuleItem() );

        const OUString aName("Outline");
        SwNumRule aNR( m_rDoc.GetUniqueNumRuleName( &aName ),
                       SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
                       OUTLINE_RULE );
        aNR = *m_rDoc.GetOutlineNumRule();

        SetAnld(&aNR, reinterpret_cast<WW8_ANLD const *>(pData), m_nSwNumLevel, true);

        // Missing Levels need not be replenished
        m_rDoc.SetOutlineNumRule( aNR );
    }else if( m_pStyles->nWwNumLevel == 10 || m_pStyles->nWwNumLevel == 11 ){
        SwNumRule* pNR = GetStyRule();
        SetAnld(pNR, reinterpret_cast<WW8_ANLD const *>(pData), 0, false);
        m_pAktColl->SetFormatAttr( SwNumRuleItem( pNR->GetName() ) );

        pStyInf = GetStyle(m_nAktColl);
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
    SetAnlvStrings(aNF, rAV, pO->rgch + nTextOfs, true); // and apply
    pNumR->Set(nSwLevel, aNF);
}

// The OLST is at the beginning of each section that contains outlines.
// The ANLDs that are connected to each outline-line contain only nonsense,
// so the OLSTs are remembered for the section to have usable information
// when outline-paragraphs occur.
void SwWW8ImplReader::Read_OLST( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    delete m_pNumOlst;
    if (nLen <= 0)
    {
        m_pNumOlst = nullptr;
        return;
    }
    m_pNumOlst = new WW8_OLST;
    if( nLen < sal::static_int_cast< sal_Int32 >(sizeof( WW8_OLST )) )   // fill if to short
        memset( m_pNumOlst, 0, sizeof( *m_pNumOlst ) );
    *m_pNumOlst = *reinterpret_cast<WW8_OLST const *>(pData);
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

SwNumRule *ANLDRuleMap::GetNumRule(sal_uInt8 nNumType)
{
    return (WW8_Numbering == nNumType ? mpNumberingNumRule : mpOutlineNumRule);
}

void ANLDRuleMap::SetNumRule(SwNumRule *pRule, sal_uInt8 nNumType)
{
    if (WW8_Numbering == nNumType)
        mpNumberingNumRule = pRule;
    else
        mpOutlineNumRule = pRule;
}

// StartAnl is called at the beginning of a row area that contains
// outline / numbering / bullets
void SwWW8ImplReader::StartAnl(const sal_uInt8* pSprm13)
{
    m_bAktAND_fNumberAcross = false;

    sal_uInt8 nT = static_cast< sal_uInt8 >(GetNumType(*pSprm13));
    if (nT == WW8_Pause || nT == WW8_None)
        return;

    m_nWwNumType = nT;
    SwNumRule *pNumRule = m_aANLDRules.GetNumRule(m_nWwNumType);

    // check for COL numbering:
    const sal_uInt8* pS12 = nullptr;// sprmAnld
    OUString sNumRule;

    if (m_pTableDesc)
    {
        sNumRule = m_pTableDesc->GetNumRuleName();
        if (!sNumRule.isEmpty())
        {
            pNumRule = m_rDoc.FindNumRulePtr(sNumRule);
            if (!pNumRule)
                sNumRule.clear();
            else
            {
                // this is ROW numbering ?
                pS12 = m_pPlcxMan->HasParaSprm(m_bVer67 ? 12 : 0xC63E); // sprmAnld
                if (pS12 && 0 != reinterpret_cast<WW8_ANLD const *>(pS12)->fNumberAcross)
                    sNumRule.clear();
            }
        }
    }

    SwWW8StyInf * pStyInf = GetStyle(m_nAktColl);
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
        if (m_pTableDesc)
        {
            if (!pS12)
                pS12 = m_pPlcxMan->HasParaSprm(m_bVer67 ? 12 : 0xC63E); // sprmAnld
            if (!pS12 || !reinterpret_cast<WW8_ANLD const *>(pS12)->fNumberAcross)
                m_pTableDesc->SetNumRuleName(pNumRule->GetName());
        }
    }

    m_bAnl = true;

    sNumRule = pNumRule ? pNumRule->GetName() : OUString();
    // set NumRules via stack
    m_pCtrlStck->NewAttr(*m_pPaM->GetPoint(),
        SfxStringItem(RES_FLTR_NUMRULE, sNumRule));

    m_aANLDRules.SetNumRule(pNumRule, m_nWwNumType);
}

// NextAnlLine() is called once for every row of a
// outline / numbering / bullet
void SwWW8ImplReader::NextAnlLine(const sal_uInt8* pSprm13)
{
    if (!m_bAnl)
        return;

    SwNumRule *pNumRule = m_aANLDRules.GetNumRule(m_nWwNumType);

    // pNd->UpdateNum ohne Regelwerk gibt GPF spaetestens beim Speichern als
    // sdw3

    // WW:10 = numberierung -> SW:0 & WW:11 = bullets -> SW:0
    if (*pSprm13 == 10 || *pSprm13 == 11)
    {
        m_nSwNumLevel = 0;
        if (!pNumRule->GetNumFormat(m_nSwNumLevel))
        {
            // not defined yet
            // sprmAnld o. 0
            const sal_uInt8* pS12 = m_pPlcxMan->HasParaSprm(m_bVer67 ? 12 : 0xC63E);
            SetAnld(pNumRule, reinterpret_cast<WW8_ANLD const *>(pS12), m_nSwNumLevel, false);
        }
    }
    else if( *pSprm13 > 0 && *pSprm13 <= MAXLEVEL )          // range WW:1..9 -> SW:0..8
    {
        m_nSwNumLevel = *pSprm13 - 1;             // outline
        // undefined
        if (!pNumRule->GetNumFormat(m_nSwNumLevel))
        {
            if (m_pNumOlst)                       // there was a OLST
            {
                //Assure upper levels are set, #i9556#
                for (sal_uInt8 nI = 0; nI < m_nSwNumLevel; ++nI)
                {
                    if (!pNumRule->GetNumFormat(nI))
                        SetNumOlst(pNumRule, m_pNumOlst, nI);
                }

                SetNumOlst(pNumRule, m_pNumOlst , m_nSwNumLevel);
            }
            else                                // no Olst -> use Anld
            {
                // sprmAnld
                const sal_uInt8* pS12 = m_pPlcxMan->HasParaSprm(m_bVer67 ? 12 : 0xC63E);
                SetAnld(pNumRule, reinterpret_cast<WW8_ANLD const *>(pS12), m_nSwNumLevel, false);
            }
        }
    }
    else
        m_nSwNumLevel = 0xff;                 // no number

    SwTextNode* pNd = m_pPaM->GetNode().GetTextNode();
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
        m_pPaM->Move(fnMoveBackward, fnGoContent);
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_FLTR_NUMRULE);
        *m_pPaM->GetPoint() = aTmpPos;
    }
    else
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_FLTR_NUMRULE);

    m_aANLDRules.mpNumberingNumRule = nullptr;
    /*
     #i18816#
     my take on this problem is that moving either way from an outline to a
     numbering doesn't halt the outline, while the numbering is always halted
    */
    bool bNumberingNotStopOutline =
        (((m_nWwNumType == WW8_Outline) && (nNewType == WW8_Numbering)) ||
        ((m_nWwNumType == WW8_Numbering) && (nNewType == WW8_Outline)));
    if (!bNumberingNotStopOutline)
        m_aANLDRules.mpOutlineNumRule = nullptr;

    m_nSwNumLevel = 0xff;
    m_nWwNumType = WW8_None;
    m_bAnl = false;
}

WW8TabBandDesc::WW8TabBandDesc( WW8TabBandDesc& rBand )
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
        pNewSHDs = new sal_uInt32[nWwCols];
        memcpy(pNewSHDs, rBand.pNewSHDs, nWwCols * sizeof(sal_uInt32));
    }
    memcpy(aDefBrcs, rBand.aDefBrcs, sizeof(aDefBrcs));
}

// ReadDef reads the cell position and the borders of a band
void WW8TabBandDesc::ReadDef(bool bVer67, const sal_uInt8* pS)
{
    if (!bVer67)
        pS++;

    short nLen = (sal_Int16)SVBT16ToShort( pS - 2 ); // not beautiful

    sal_uInt8 nCols = *pS;                       // number of cells
    short nOldCols = nWwCols;

    if( nCols > MAX_COL )
        return;

    nWwCols = nCols;

    const sal_uInt8* pT = &pS[1];
    nLen --;
    int i;
    for(i=0; i<=nCols; i++, pT+=2 )
        nCenter[i] = (sal_Int16)SVBT16ToShort( pT );    // X-borders
    nLen -= 2 * ( nCols + 1 );
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

    short nColsToRead = nFileCols;
    if (nColsToRead > nCols)
        nColsToRead = nCols;

    if( nColsToRead )
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
        WW8_TCell* pAktTC  = pTCs;
        if( bVer67 )
        {
            WW8_TCellVer6 const * pTc = reinterpret_cast<WW8_TCellVer6 const *>(pT);
            for(i=0; i<nColsToRead; i++, ++pAktTC,++pTc)
            {
                if( i < nColsToRead )
                {               // TC from file ?
                    sal_uInt8 aBits1 = pTc->aBits1Ver6;
                    pAktTC->bFirstMerged = sal_uInt8( ( aBits1 & 0x01 ) != 0 );
                    pAktTC->bMerged = sal_uInt8( ( aBits1 & 0x02 ) != 0 );
                    pAktTC->rgbrc[ WW8_TOP ]
                        = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_TOP ] ));
                    pAktTC->rgbrc[ WW8_LEFT ]
                        = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_LEFT ] ));
                    pAktTC->rgbrc[ WW8_BOT ]
                        = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_BOT ] ));
                    pAktTC->rgbrc[ WW8_RIGHT ]
                        = WW8_BRCVer9(WW8_BRC( pTc->rgbrcVer6[ WW8_RIGHT ] ));
                    if(    ( pAktTC->bMerged )
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
        }
        else
        {
            WW8_TCellVer8 const * pTc = reinterpret_cast<WW8_TCellVer8 const *>(pT);
            for (int k = 0; k < nColsToRead; ++k, ++pAktTC, ++pTc )
            {
                sal_uInt16 aBits1 = SVBT16ToShort( pTc->aBits1Ver8 );
                pAktTC->bFirstMerged    = sal_uInt8( ( aBits1 & 0x0001 ) != 0 );
                pAktTC->bMerged         = sal_uInt8( ( aBits1 & 0x0002 ) != 0 );
                pAktTC->bVertical       = sal_uInt8( ( aBits1 & 0x0004 ) != 0 );
                pAktTC->bBackward       = sal_uInt8( ( aBits1 & 0x0008 ) != 0 );
                pAktTC->bRotateFont     = sal_uInt8( ( aBits1 & 0x0010 ) != 0 );
                pAktTC->bVertMerge      = sal_uInt8( ( aBits1 & 0x0020 ) != 0 );
                pAktTC->bVertRestart    = sal_uInt8( ( aBits1 & 0x0040 ) != 0 );
                pAktTC->nVertAlign      = ( ( aBits1 & 0x0180 ) >> 7 );
                // note: in aBits1 there are 7 bits unused,
                //       followed by another 16 unused bits

                pAktTC->rgbrc[ WW8_TOP   ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_TOP   ]);
                pAktTC->rgbrc[ WW8_LEFT  ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_LEFT  ]);
                pAktTC->rgbrc[ WW8_BOT   ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_BOT   ]);
                pAktTC->rgbrc[ WW8_RIGHT ] = WW8_BRCVer9(pTc->rgbrcVer8[ WW8_RIGHT ]);
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
    if( pParamsTSetBRC && pTCs ) // set one or more cell border(s)
    {
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

        WW8_TCell* pAktTC  = pTCs + nitcFirst;
        WW8_BRCVer9 brcVer9;
        if( nBrcVer == 6 )
            brcVer9 = WW8_BRCVer9(WW8_BRC(*reinterpret_cast<WW8_BRCVer6 const *>(pParamsTSetBRC+3)));
        else if( nBrcVer == 8 )
            brcVer9 = WW8_BRCVer9(*reinterpret_cast<WW8_BRC const *>(pParamsTSetBRC+3));
        else
            brcVer9 = *reinterpret_cast<WW8_BRCVer9 const *>(pParamsTSetBRC+3);

        for( int i = nitcFirst; i < nitcLim; ++i, ++pAktTC )
        {
            if( bChangeTop )
                pAktTC->rgbrc[ WW8_TOP   ] = brcVer9;
            if( bChangeLeft )
                pAktTC->rgbrc[ WW8_LEFT  ] = brcVer9;
            if( bChangeBottom )
                pAktTC->rgbrc[ WW8_BOT   ] = brcVer9;
            if( bChangeRight )
                pAktTC->rgbrc[ WW8_RIGHT ] = brcVer9;
        }
    }
}

void WW8TabBandDesc::ProcessSprmTTableBorders(int nBrcVer, const sal_uInt8* pParams)
{
    // sprmTTableBorders
    if( nBrcVer == 6 )
    {
        WW8_BRCVer6 const *pVer6 = reinterpret_cast<WW8_BRCVer6 const *>(pParams);
        for (int i = 0; i < 6; ++i)
            aDefBrcs[i] = WW8_BRCVer9(WW8_BRC(pVer6[i]));
    }
    else if ( nBrcVer == 8 )
    {
        static_assert(sizeof (WW8_BRC) == 4, "this has to match the msword size");
        for( int i = 0; i < 6; ++i )
            aDefBrcs[i] = WW8_BRCVer9(reinterpret_cast<WW8_BRC const *>(pParams)[i]);
    }
    else
        memcpy( aDefBrcs, pParams, sizeof( aDefBrcs ) );
}

void WW8TabBandDesc::ProcessSprmTDxaCol(const sal_uInt8* pParamsTDxaCol)
{
    // sprmTDxaCol (opcode 0x7623) changes the width of cells
    // whose index is within a certain range to be a certain value.

    if( nWwCols && pParamsTDxaCol ) // set one or more cell length(s)
    {
        sal_uInt8 nitcFirst= pParamsTDxaCol[0]; // first col to be changed
        sal_uInt8 nitcLim  = pParamsTDxaCol[1]; // (last col to be changed)+1
        short nDxaCol = (sal_Int16)SVBT16ToShort( pParamsTDxaCol + 2 );

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
    if( nWwCols && pParamsTInsert )        // set one or more cell length(s)
    {
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
            WW8_TCell* pAktTC  = pTCs + nitcFirst;
            int i = 0;
            while( i < nShlCnt )
            {
                // adjust the left x-position
                nCenter[nitcFirst + i] = nCenter[nitcLim + i];

                // adjust the cell's borders
                *pAktTC = pTCs[ nitcLim + i];

                ++i;
                ++pAktTC;
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
        memset( pSHDs, 0, nWwCols * sizeof( WW8_SHD ) );
    }

    short nAnz = nLen >> 1;
    if (nAnz > nWwCols)
        nAnz = nWwCols;

    SVBT16 const * pShd;
    int i;
    for(i=0, pShd = reinterpret_cast<SVBT16 const *>(pS); i<nAnz; i++, pShd++ )
        pSHDs[i].SetWWValue( *pShd );
}

void WW8TabBandDesc::ReadNewShd(const sal_uInt8* pS, bool bVer67)
{
    sal_uInt8 nLen = pS ? *(pS - 1) : 0;
    if (!nLen)
        return;

    if (!pNewSHDs)
        pNewSHDs = new sal_uInt32[nWwCols];

    short nAnz = nLen / 10; //10 bytes each
    if (nAnz > nWwCols)
        nAnz = nWwCols;

    int i=0;
    while (i < nAnz)
        pNewSHDs[i++] = SwWW8ImplReader::ExtractColour(pS, bVer67);

    while (i < nWwCols)
        pNewSHDs[i++] = COL_AUTO;
}

void WW8TabBandDesc::setcelldefaults(WW8_TCell *pCells, short nCols)
{
    memset( pCells, 0, nCols * sizeof( WW8_TCell ) );
}

const sal_uInt8 *HasTabCellSprm(WW8PLCFx_Cp_FKP* pPap, bool bVer67)
{
    const sal_uInt8 *pParams;
    if (bVer67)
        pParams = pPap->HasSprm(24);
    else
    {
        if (nullptr == (pParams = pPap->HasSprm(0x244B)))
            pParams = pPap->HasSprm(0x2416);
    }
    return pParams;
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
                case NS_sprm::LN_TTableWidth:
                    return sprmTTableWidth;
                case NS_sprm::LN_TTextFlow:
                    return sprmTTextFlow;
                case NS_sprm::LN_TTableHeader:
                    return sprmTTableHeader;
                case NS_sprm::LN_TFCantSplit:
                    return sprmTFCantSplit;
                case NS_sprm::LN_TJc90:
                    return sprmTJc;
                case NS_sprm::LN_TFBiDi:
                    return sprmTFBiDi;
                case NS_sprm::LN_TDelete:
                    return sprmTDelete;
                case NS_sprm::LN_TInsert:
                    return sprmTInsert;
                case NS_sprm::LN_TDxaCol:
                    return sprmTDxaCol;
                case NS_sprm::LN_TDyaRowHeight:
                    return sprmTDyaRowHeight;
                case NS_sprm::LN_TDxaLeft:
                    return sprmTDxaLeft;
                case NS_sprm::LN_TDxaGapHalf:
                    return sprmTDxaGapHalf;
                case NS_sprm::LN_TTableBorders80:
                    return sprmTTableBorders;
                case NS_sprm::LN_TDefTable:
                    return sprmTDefTable;
                case NS_sprm::LN_TDefTableShd80:
                    return sprmTDefTableShd;
                case NS_sprm::LN_TDefTableShd:
                    return sprmTDefTableNewShd;
                case NS_sprm::LN_TTableBorders:
                    return sprmTTableBorders90;
                case NS_sprm::LN_TSetBrc80:
                    return sprmTSetBrc;
                case NS_sprm::LN_TSetBrc:
                    return sprmTSetBrc90;
                case NS_sprm::LN_TCellPadding:
                    return sprmTCellPadding;
                case NS_sprm::LN_TCellPaddingDefault:
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
    mpOldRedlineStack(nullptr),
    pIo(pIoClass),
    pFirstBand(nullptr),
    pActBand(nullptr),
    pTmpPos(nullptr),
    pTableNd(nullptr),
    pTabLines(nullptr),
    pTabLine(nullptr),
    pTabBoxes(nullptr),
    pTabBox(nullptr),
    pAktWWCell(nullptr),
    nRows(0),
    nDefaultSwCols(0),
    nBands(0),
    nMinLeft(0),
    nConvertedLeft(0),
    nMaxRight(0),
    nSwWidth(0),
    nPreferredWidth(0),
    nOrgDxaLeft(0),
    bOk(true),
    bClaimLineFormat(false),
    eOri(text::HoriOrientation::NONE),
    bIsBiDi(false),
    nAktRow(0),
    nAktBandRow(0),
    nAktCol(0),
    nRowsToRepeat(0),
    pTable(nullptr),
    pParentPos(nullptr),
    pFlyFormat(nullptr),
    aItemSet(pIo->m_rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    pIo->m_bAktAND_fNumberAcross = false;

    static const sal_Int16 aOriArr[] =
    {
        text::HoriOrientation::LEFT, text::HoriOrientation::CENTER, text::HoriOrientation::RIGHT, text::HoriOrientation::CENTER
    };

    bool bOldVer = ww::IsSevenMinus(pIo->GetFib().GetFIBVersion());
    WW8_TablePos aTabPos;

    WW8PLCFxSave1 aSave;
    pIo->m_pPlcxMan->GetPap()->Save( aSave );

    WW8PLCFx_Cp_FKP* pPap = pIo->m_pPlcxMan->GetPapPLCF();

    eOri = text::HoriOrientation::LEFT;

    WW8TabBandDesc* pNewBand = new WW8TabBandDesc;

    wwSprmParser aSprmParser(pIo->GetFib().GetFIBVersion());

    // process pPap until end of table found
    do
    {
        short nTabeDxaNew      = SHRT_MAX;
        bool bTabRowJustRead   = false;
        const sal_uInt8* pShadeSprm = nullptr;
        const sal_uInt8* pNewShadeSprm = nullptr;
        const sal_uInt8* pTableBorders = nullptr;
        const sal_uInt8* pTableBorders90 = nullptr;
        std::vector<const sal_uInt8*> aTSetBrcs, aTSetBrc90s;
        WW8_TablePos *pTabPos  = nullptr;

        // search end of a tab row
        if(!(pIo->SearchRowEnd(pPap, nStartCp, pIo->m_nInTable)))
        {
            bOk = false;
            break;
        }

        // Get the SPRM chains:
        // first from PAP and then from PCD (of the Piece Table)
        WW8PLCFxDesc aDesc;
        pPap->GetSprms( &aDesc );
        WW8SprmIter aSprmIter(aDesc.pMemPos, aDesc.nSprmsLen, aSprmParser);

        for (int nLoop = 0; nLoop < 2; ++nLoop)
        {
            bool bRepeatedSprm = false;
            const sal_uInt8* pParams;
            while (aSprmIter.GetSprms() && nullptr != (pParams = aSprmIter.GetAktParams()))
            {
                sal_uInt16 nId = aSprmIter.GetAktId();
                wwTableSprm eSprm = GetTableSprm(nId, pIo->GetFib().GetFIBVersion());
                switch (eSprm)
                {
                    case sprmTTableWidth:
                        {
                        const sal_uInt8 b0 = pParams[0];
                        const sal_uInt8 b1 = pParams[1];
                        const sal_uInt8 b2 = pParams[2];
                        if (b0 == 3) // Twips
                            nPreferredWidth = b2 * 0x100 + b1;
                        }
                        break;
                    case sprmTTextFlow:
                        pNewBand->ProcessDirection(pParams);
                        break;
                    case sprmTFCantSplit:
                        pNewBand->bCantSplit = *pParams;
                        bClaimLineFormat = true;
                        break;
                    case sprmTTableBorders:
                        pTableBorders = pParams; // process at end
                        break;
                    case sprmTTableBorders90:
                        pTableBorders90 = pParams; // process at end
                        break;
                    case sprmTTableHeader:
                        if (!bRepeatedSprm)
                        {
                            nRowsToRepeat++;
                            bRepeatedSprm = true;
                        }
                        break;
                    case sprmTJc:
                        // sprmTJc  -  Justification Code
                        if (nRows == 0)
                            eOri = aOriArr[*pParams & 0x3];
                        break;
                    case sprmTFBiDi:
                        bIsBiDi = SVBT16ToShort(pParams) != 0;
                        break;
                    case sprmTDxaGapHalf:
                        pNewBand->nGapHalf = (sal_Int16)SVBT16ToShort( pParams );
                        break;
                    case sprmTDyaRowHeight:
                        pNewBand->nLineHeight = (sal_Int16)SVBT16ToShort( pParams );
                        bClaimLineFormat = true;
                        break;
                    case sprmTDefTable:
                        pNewBand->ReadDef(bOldVer, pParams);
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
                            short nDxaNew = (sal_Int16)SVBT16ToShort( pParams );
                            nOrgDxaLeft = nDxaNew;
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
                pNewBand->ProcessSprmTTableBorders(9, pTableBorders90);
            else if (pTableBorders)
                pNewBand->ProcessSprmTTableBorders(bOldVer ? 6 : 8,
                    pTableBorders);
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

        if (!pActBand)
            pActBand = pFirstBand = pNewBand;
        else
        {
            pActBand->pNextBand = pNewBand;
            pActBand = pNewBand;
        }
        nBands++;

        pNewBand = new WW8TabBandDesc;

        nRows++;
        pActBand->nRows++;

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
            bOk = false;
            break;
        }

        //Are we still in a table cell
        const sal_uInt8* pParams = HasTabCellSprm(pPap, bOldVer);
        const sal_uInt8 *pLevel = pPap->HasSprm(0x6649);
        // InTable
        if (!pParams || (1 != *pParams) ||
            (pLevel && (*pLevel <= pIo->m_nInTable)))
        {
            break;
        }

        //Get the end of row new table positioning data
        WW8_CP nMyStartCp=nStartCp;
        if (pIo->SearchRowEnd(pPap, nMyStartCp, pIo->m_nInTable))
            if (pIo->ParseTabPos(&aTabPos, pPap))
                pTabPos = &aTabPos;

        //Move back to this cell
        aRes.pMemPos = nullptr;
        aRes.nStartPos = nStartCp;

        // PlcxMan currently points too far ahead so we need to bring
        // it back to where we are trying to make a table
        pIo->m_pPlcxMan->GetPap()->nOrigStartPos = aRes.nStartPos;
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);

        //Does this row match up with the last row closely enough to be
        //considered part of the same table
        ApoTestResults aApo = pIo->TestApo(pIo->m_nInTable + 1, false, pTabPos);

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
            WW8FlyPara *pNewFly = pIo->ConstructApo(aApo, pTabPos);
            if (pNewFly)
                delete pNewFly;
            else
                break;
        }

        nStartCp = aRes.nEndPos;
    }
    while(true);

    if( bOk )
    {
        if( pActBand->nRows > 1 )
        {
            // last band has more than 1 cell
            delete pNewBand;
            pNewBand = new WW8TabBandDesc( *pActBand ); // create new
            pActBand->nRows--;      // wegen Sonderbehandlung Raender-Defaults
            pNewBand->nRows = 1;
            pActBand->pNextBand = pNewBand; // am Ende einschleifen
            nBands++;
            pNewBand = nullptr;                   // do not delete
        }
        CalcDefaults();
    }
    delete pNewBand;

    pIo->m_pPlcxMan->GetPap()->Restore( aSave );
}

WW8TabDesc::~WW8TabDesc()
{
    WW8TabBandDesc* pR = pFirstBand;
    while(pR)
    {
        WW8TabBandDesc* pR2 = pR->pNextBand;
        delete pR;
        pR = pR2;
    }

    delete pParentPos;
}

void WW8TabDesc::CalcDefaults()
{
    short nMinCols = SHRT_MAX;
    WW8TabBandDesc* pR;

    nMinLeft = SHRT_MAX;
    nMaxRight = SHRT_MIN;

    /*
    If we are an honestly inline centered table, then the normal rules of
    engagement for left and right margins do not apply. The multiple rows are
    centered regardless of the actual placement of rows, so we cannot have
    mismatched rows as is possible in other configurations.

    e.g. change the example bugdoc in word from text wrapping of none (inline)
    to around (in frame (bApo)) and the table splits into two very disjoint
    rows as the beginning point of each row are very different
    */
    if ((!pIo->InLocalApo()) && (eOri == text::HoriOrientation::CENTER))
    {
        for (pR = pFirstBand; pR; pR = pR->pNextBand)
            for( short i = pR->nWwCols; i >= 0; --i)
                pR->nCenter[i] = pR->nCenter[i] -  pR->nCenter[0];
    }

    // 1. Durchlauf: aeusserste L- und R-Grenzen finden
    for( pR = pFirstBand; pR; pR = pR->pNextBand )
    {
        if( pR->nCenter[0] < nMinLeft )
            nMinLeft = pR->nCenter[0];

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
            if (nCellWidth && ((nCellWidth - pR->nGapHalf*2) < MINLAY) && pR->nGapHalf < nCellWidth)
            {
                pR->nCenter[i+1] = pR->nCenter[i]+MINLAY+pR->nGapHalf * 2;
            }
        }

        if( pR->nCenter[pR->nWwCols] > nMaxRight )
            nMaxRight = pR->nCenter[pR->nWwCols];
    }
    nSwWidth = nMaxRight - nMinLeft;

    // If the table is right aligned we need to align all rows to the
    // row that has the furthest right point

    if(eOri == text::HoriOrientation::RIGHT)
    {
        for( pR = pFirstBand; pR; pR = pR->pNextBand )
        {
            int adjust = nMaxRight - pR->nCenter[pR->nWwCols];
            for( short i = 0; i < pR->nWwCols + 1; i++ )
            {
                pR->nCenter[i] = static_cast< short >(pR->nCenter[i] + adjust);
            }

        }
    }

    // 2. pass: Detect number of writer columns. This can exceed the count
    // of columns in WW by 2, because SW in constrast to WW does not provide
    // fringed left and right borders and has to fill with empty boxes.
    // Non existent cells can reduce the number of columns.

    // 3. pass: Replace border with defaults if needed
    nConvertedLeft = nMinLeft;

    short nLeftMaxThickness = 0, nRightMaxThickness=0;
    for( pR = pFirstBand ; pR; pR = pR->pNextBand )
    {
        if( !pR->pTCs )
        {
            pR->pTCs = new WW8_TCell[ pR->nWwCols ];
            memset( pR->pTCs, 0, pR->nWwCols * sizeof( WW8_TCell ) );
        }
        for (int k = 0; k < pR->nWwCols; ++k)
        {
            WW8_TCell* pT = &pR->pTCs[k];
            int i, j;
            for( i = 0; i < 4; i ++ )
            {
                if (pT->rgbrc[i].brcType()==0)
                {
                    // if shadow is set, its invalid
                    j = i;
                    switch( i )
                    {
                    case 0:
                        // outer top / horizontally inside
                        j = (pR == pFirstBand) ? 0 : 4;
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
                    // mangel mit Defaults ueber
                    pT->rgbrc[i] = pR->aDefBrcs[j];
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
    nSwWidth = nSwWidth + nRightMaxThickness;
    nMaxRight = nMaxRight + nRightMaxThickness;
    nConvertedLeft = nMinLeft-(nLeftMaxThickness/2);

    for( pR = pFirstBand; pR; pR = pR->pNextBand )
    {
        pR->nSwCols = pR->nWwCols;
        pR->bLEmptyCol = pR->nCenter[0] - nMinLeft >= MINLAY;
        pR->bREmptyCol = (nMaxRight - pR->nCenter[pR->nWwCols] - nRightMaxThickness) >= MINLAY;

        short nAddCols = short(pR->bLEmptyCol) + short(pR->bREmptyCol);
        sal_uInt16 i;
        sal_uInt16 j = ( pR->bLEmptyCol ) ? 1 : 0;
        for (i = 0; i < pR->nWwCols; ++i)
        {
            pR->nTransCell[i] = (sal_Int8)j;
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
        so the default mapping forward wont't work. So map it (and
        contiguous invalid cells backwards to the last valid cell instead.
        */
        if (i && !pR->bExist[i-1])
        {
            sal_uInt16 k=i-1;
            while (k && !pR->bExist[k])
                k--;
            for (sal_uInt16 n=k+1;n<i;n++)
                pR->nTransCell[n] = pR->nTransCell[k];
        }

        pR->nTransCell[i++] = (sal_Int8)(j++);  // Can exceed by 2 among other
        pR->nTransCell[i] = (sal_Int8)j;        // things because of bREmptyCol

        pR->nSwCols = pR->nSwCols + nAddCols;
        if( pR->nSwCols < nMinCols )
            nMinCols = pR->nSwCols;
    }

    /*
    #i9718#
    Find the largest of the borders on cells that adjoin top bottom and remove
    the val from the top and put in on the bottom cell. I can't seem to make
    disjoint upper and lowers to see what happens there.
    */

    if ((nMinLeft && !bIsBiDi && text::HoriOrientation::LEFT == eOri) ||
        (nMinLeft != -108 && bIsBiDi && text::HoriOrientation::RIGHT == eOri)) // Word sets the first nCenter value to -108 when no indent is used
        eOri = text::HoriOrientation::LEFT_AND_WIDTH; //  absolutely positioned

    nDefaultSwCols = nMinCols;  // because inserting cells is cheaper than merging
    if( nDefaultSwCols == 0 )
        bOk = false;
    pActBand = pFirstBand;
    nAktBandRow = 0;
    OSL_ENSURE( pActBand, "pActBand ist 0" );
}

void WW8TabDesc::SetSizePosition(SwFrameFormat* pFrameFormat)
{
    SwFrameFormat* pApply = pFrameFormat;
    if (!pApply )
        pApply = pTable->GetFrameFormat();
    OSL_ENSURE(pApply,"No frame");
    pApply->SetFormatAttr(aItemSet);
    if (pFrameFormat)
    {
        SwFormatFrameSize aSize = pFrameFormat->GetFrameSize();
        aSize.SetHeightSizeType(ATT_MIN_SIZE);
        aSize.SetHeight(MINLAY);
        pFrameFormat->SetFormatAttr(aSize);
        pTable->GetFrameFormat()->SetFormatAttr(SwFormatHoriOrient(0,text::HoriOrientation::FULL));
    }
}

void wwSectionManager::PrependedInlineNode(const SwPosition &rPos,
    const SwNode &rNode)
{
    OSL_ENSURE(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if ((!maSegments.empty()) && (maSegments.back().maStart == rPos.nNode))
        maSegments.back().maStart = SwNodeIndex(rNode);
}

void WW8TabDesc::CreateSwTable(SvxULSpaceItem* pULSpaceItem)
{
    ::SetProgressState(pIo->m_nProgress, pIo->m_pDocShell);   // Update

    // if there is already some content on the Node append new node to ensure
    // that this content remains ABOVE the table
    SwPosition* pPoint = pIo->m_pPaM->GetPoint();
    bool bInsNode = pPoint->nContent.GetIndex() != 0;
    bool bSetMinHeight = false;

    /*
     #i8062#
     Set fly anchor to its anchor pos, so that if a table starts immediately
     at this position a new node will be inserted before inserting the table.
    */
    if (!bInsNode && pIo->m_pFormatOfJustInsertedApo)
    {
        const SwPosition* pAPos =
            pIo->m_pFormatOfJustInsertedApo->GetAnchor().GetContentAnchor();
        if (pAPos && &pAPos->nNode.GetNode() == &pPoint->nNode.GetNode())
        {
            bInsNode = true;
            bSetMinHeight = true;

            SwFormatSurround aSur(pIo->m_pFormatOfJustInsertedApo->GetSurround());
            aSur.SetAnchorOnly(true);
            pIo->m_pFormatOfJustInsertedApo->SetFormatAttr(aSur);
        }
    }

    if (bSetMinHeight)
    {
        // minimize Fontsize to minimize height growth of the header/footer
        // set font size to 1 point to minimize y-growth of Hd/Ft
        SvxFontHeightItem aSz(20, 100, RES_CHRATR_FONTSIZE);
        pIo->NewAttr( aSz );
        pIo->m_pCtrlStck->SetAttr(*pPoint, RES_CHRATR_FONTSIZE);
    }

    if (bInsNode)
        pIo->AppendTextNode(*pPoint);

    pTmpPos = new SwPosition( *pIo->m_pPaM->GetPoint() );

    // The table is small: The number of columns is the lowest count of
    // columns of the origin, because inserting is faster than deleting.
    // The number of rows is the count of bands because (identically)
    // rows of a band can be duplicated easy.
    pTable = pIo->m_rDoc.InsertTable(
            SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 0 ),
            *pTmpPos, nBands, nDefaultSwCols, eOri );

    OSL_ENSURE(pTable && pTable->GetFrameFormat(), "insert table failed");
    if (!pTable || !pTable->GetFrameFormat())
        return;

    if (pULSpaceItem && pULSpaceItem->GetUpper() != 0)
        aItemSet.Put(*pULSpaceItem);

    SwTableNode* pTableNode = pTable->GetTableNode();
    OSL_ENSURE(pTableNode, "no table node!");
    if (pTableNode)
    {
        pIo->m_aSectionManager.PrependedInlineNode(*pIo->m_pPaM->GetPoint(),
            *pTableNode);
    }

    // Check if the node into which the table should be inserted already
    // contains a Pagedesc. If so that Pagedesc would be moved to the
    // row after the table, whats wrong. So delete and
    // set later to the table format.
    if (SwTextNode *const pNd = pTmpPos->nNode.GetNode().GetTextNode())
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
                aItemSet.Put(*pSetAttr);
                delete pSetAttr;
            }
        }
    }

    // total width of table
    if( nMaxRight - nMinLeft > MINLAY * nDefaultSwCols )
    {
        pTable->GetFrameFormat()->SetFormatAttr(SwFormatFrameSize(ATT_FIX_SIZE, nSwWidth));
        aItemSet.Put(SwFormatFrameSize(ATT_FIX_SIZE, nSwWidth));
    }

    SvxFrameDirectionItem aDirection(
        bIsBiDi ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR );
    pTable->GetFrameFormat()->SetFormatAttr(aDirection);

    if (text::HoriOrientation::LEFT_AND_WIDTH == eOri)
    {
        if (!pIo->m_nInTable && pIo->InLocalApo() && pIo->m_pSFlyPara->pFlyFormat &&
            GetMinLeft())
        {
            //If we are inside a frame and we have a border, the frames
            //placement does not consider the tables border, which word
            //displays outside the frame, so adjust here.
            SwFormatHoriOrient aHori(pIo->m_pSFlyPara->pFlyFormat->GetHoriOrient());
            sal_Int16 eHori = aHori.GetHoriOrient();
            if ((eHori == text::HoriOrientation::NONE) || (eHori == text::HoriOrientation::LEFT) ||
                (eHori == text::HoriOrientation::LEFT_AND_WIDTH))
            {
                //With multiple table, use last table settings. Perhaps
                //the maximum is what word does ?
                aHori.SetPos(pIo->m_pSFlyPara->nXPos + GetMinLeft());
                aHori.SetHoriOrient(text::HoriOrientation::NONE);
                pIo->m_pSFlyPara->pFlyFormat->SetFormatAttr(aHori);
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
            // set right to original DxaLeft (i28656)

            long nLeft = 0;
            if (!bIsBiDi)
                nLeft = GetMinLeft();
            else
            {
                if (nPreferredWidth)
                {
                    nLeft = pIo->m_aSectionManager.GetTextAreaWidth();
                    nLeft = nLeft - nPreferredWidth  - nOrgDxaLeft;
                }
                else
                    nLeft = -GetMinLeft();
            }

            aL.SetLeft(nLeft);

            aItemSet.Put(aL);
        }
    }

    mpOldRedlineStack = pIo->m_pRedlineStack;
    pIo->m_pRedlineStack = new sw::util::RedlineStack(pIo->m_rDoc);
}

void WW8TabDesc::UseSwTable()
{
    // init global Vars
    pTabLines = &pTable->GetTabLines();
    nAktRow = nAktCol = nAktBandRow = 0;

    pTableNd  = const_cast<SwTableNode*>((*pTabLines)[0]->GetTabBoxes()[0]->
        GetSttNd()->FindTableNode());
    OSL_ENSURE( pTableNd, "wo ist mein TabellenNode" );

    // #i69519# - Restrict rows to repeat to a decent value
    if ( nRowsToRepeat == static_cast<sal_uInt16>(nRows) )
        nRowsToRepeat = 1;

    pTableNd->GetTable().SetRowsToRepeat( nRowsToRepeat );
    // insert extra cells if needed and something like this
    AdjustNewBand();

    WW8DupProperties aDup(pIo->m_rDoc,pIo->m_pCtrlStck);
    pIo->m_pCtrlStck->SetAttr(*pIo->m_pPaM->GetPoint(), 0, false);

    // now set the correct PaM and prepare first merger group if any
    SetPamInCell(nAktCol, true);
    aDup.Insert(*pIo->m_pPaM->GetPoint());

    pIo->m_bWasTabRowEnd = false;
    pIo->m_bWasTabCellEnd = false;
}

void WW8TabDesc::MergeCells()
{
    short nRow;

    for (pActBand=pFirstBand, nRow=0; pActBand; pActBand=pActBand->pNextBand)
    {
        // insert current box into merge group if appropriate
        if( pActBand->pTCs )
        {
            for( short j = 0; j < pActBand->nRows; j++, nRow++ )
                for( short i = 0; i < pActBand->nWwCols; i++ )
                {
                    WW8SelBoxInfo* pActMGroup = nullptr;

                    // start a new merge group if appropriate

                    OSL_ENSURE(nRow < (sal_uInt16)pTabLines->size(),
                        "Too few lines, table ended early");
                    if (nRow >= (sal_uInt16)pTabLines->size())
                        return;
                    pTabLine = (*pTabLines)[ nRow ];
                    pTabBoxes = &pTabLine->GetTabBoxes();

                    sal_uInt16 nCol = pActBand->nTransCell[ i ];
                    if (!pActBand->bExist[i])
                        continue;
                    OSL_ENSURE(nCol < pTabBoxes->size(),
                        "Too few columns, table ended early");
                    if (nCol >= pTabBoxes->size())
                        return;
                    pTabBox = (*pTabBoxes)[nCol];
                    WW8_TCell& rCell = pActBand->pTCs[ i ];
                    // is this the left upper cell of a merge group ?

                    bool bMerge = false;
                    if ( rCell.bVertRestart && !rCell.bMerged )
                        bMerge = true;
                    else if (rCell.bFirstMerged && pActBand->bExist[i])
                    {
                        // Some tests to avoid merging cells which previously were
                        // declared invalid because of sharing the exact same dimensions
                        // as their previous cell

                        //If theres anything underneath/above we're ok.
                        if (rCell.bVertMerge || rCell.bVertRestart)
                            bMerge = true;
                        else
                        {
                            //If it's a hori merge only, and the only things in
                            //it are invalid cells then it's already taken care
                            //of, so don't merge.
                            for (sal_uInt16 i2 = i+1; i2 < pActBand->nWwCols; i2++ )
                                if (pActBand->pTCs[ i2 ].bMerged &&
                                    !pActBand->pTCs[ i2 ].bFirstMerged  )
                                {
                                    if (pActBand->bExist[i2])
                                    {
                                        bMerge = true;
                                        break;
                                    }
                                }
                                else
                                    break;
                        }
                    }

                    if (bMerge)
                    {
                        short nX1    = pActBand->nCenter[ i ];
                        short nWidth = pActBand->nWidth[ i ];

                        // 2. create current merge group
                        pActMGroup = new WW8SelBoxInfo( nX1, nWidth );

                        // determine size of new merge group
                        // before inserted the new merge group.
                        // Needed to correctly locked previously created merge groups.
                        // Calculate total width and set
                        short nSizCell = pActBand->nWidth[ i ];
                        for (sal_uInt16 i2 = i+1; i2 < pActBand->nWwCols; i2++ )
                            if (pActBand->pTCs[ i2 ].bMerged &&
                                !pActBand->pTCs[ i2 ].bFirstMerged  )
                            {
                                nSizCell = nSizCell + pActBand->nWidth[ i2 ];
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
                    UpdateTableMergeGroup( rCell, pActMGroup, pTabBox, i );
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
    short nRow = nAktRow + 1;
    if (nRow < (sal_uInt16)pTabLines->size())
    {
        if (SwTableLine *pLine = (*pTabLines)[nRow])
        {
            SwTableBoxes &rBoxes = pLine->GetTabBoxes();
            pTabBox2 = rBoxes.empty() ? nullptr : rBoxes.front();
        }
    }

    if (!pTabBox2 || !pTabBox2->GetSttNd())
    {
        MoveOutsideTable();
        return;
    }

    sal_uLong nSttNd = pTabBox2->GetSttIdx() + 1,
              nEndNd = pTabBox2->GetSttNd()->EndOfSectionIndex();

    if (pIo->m_pPaM->GetPoint()->nNode != nSttNd)
    {
        do
        {
            pIo->m_pPaM->GetPoint()->nNode = nSttNd;
        }
        while (pIo->m_pPaM->GetNode().GetNodeType() != ND_TEXTNODE && ++nSttNd < nEndNd);

        pIo->m_pPaM->GetPoint()->nContent.Assign(pIo->m_pPaM->GetContentNode(), 0);
        pIo->m_rDoc.SetTextFormatColl(*pIo->m_pPaM, const_cast<SwTextFormatColl*>(pIo->m_pDfltTextFormatColl));
    }
}

void WW8TabDesc::MoveOutsideTable()
{
    OSL_ENSURE(pTmpPos && pIo, "I've forgotten where the table is anchored");
    if (pTmpPos && pIo)
        *pIo->m_pPaM->GetPoint() = *pTmpPos;
}

void WW8TabDesc::FinishSwTable()
{
    pIo->m_pRedlineStack->closeall(*pIo->m_pPaM->GetPoint());
    delete pIo->m_pRedlineStack;
    pIo->m_pRedlineStack = mpOldRedlineStack;
    mpOldRedlineStack = nullptr;

    WW8DupProperties aDup(pIo->m_rDoc,pIo->m_pCtrlStck);
    pIo->m_pCtrlStck->SetAttr( *pIo->m_pPaM->GetPoint(), 0, false);

    MoveOutsideTable();
    delete pTmpPos;
    pTmpPos = nullptr;

    aDup.Insert(*pIo->m_pPaM->GetPoint());

    pIo->m_bWasTabRowEnd = false;
    pIo->m_bWasTabCellEnd = false;

    pIo->m_aInsertedTables.InsertTable(*pTableNd, *pIo->m_pPaM);

    MergeCells();

    // if needed group cells together that should be merged
    if (!m_MergeGroups.empty())
    {
        // process all merge groups one by one
        for (auto const& groupIt : m_MergeGroups)
        {
            sal_uInt16 nActBoxCount = groupIt->size();

            if( ( 1 < nActBoxCount ) && (*groupIt)[0] )
            {
                const sal_uInt16 nRowSpan = groupIt->size();
                for (sal_uInt16 n = 0; n < nRowSpan; ++n)
                {
                    SwTableBox* pCurrentBox = (*groupIt)[n];
                    const long nRowSpanSet = n == 0 ?
                                               nRowSpan :
                                             ((-1) * (nRowSpan - n));
                    pCurrentBox->setRowSpan( nRowSpanSet );
                }
            }
        }
        pIo->m_pFormatOfJustInsertedApo = nullptr;
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
        const short nToleranz = 4;
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
                nGrX1 = rActGroup.nGroupXStart - nToleranz;
                nGrX2 = rActGroup.nGroupXStart
                             +rActGroup.nGroupWidth  + nToleranz;

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
                                        && ( nX1 < nGrX2 - 2*nToleranz ) )
                            || (     ( nX2 > nGrX1 + 2*nToleranz )
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
    return (static_cast<size_t>(nCol) < SAL_N_ELEMENTS(pActBand->bExist)) &&
           pActBand->bExist[nCol] &&
           (sal_uInt16)nAktRow < pTabLines->size();
}

bool WW8TabDesc::InFirstParaInCell() const
{
    //e.g. #i19718#
    if (!pTabBox || !pTabBox->GetSttNd())
    {
        OSL_FAIL("Problem with table");
        return false;
    }

    if (!IsValidCell(GetAktCol()))
        return false;

    if (pIo->m_pPaM->GetPoint()->nNode == pTabBox->GetSttIdx() + 1)
        return true;

    return false;
}

void WW8TabDesc::StartMiserableHackForUnsupportedDirection(short nWwCol)
{
    OSL_ENSURE(pActBand, "Impossible");
    if (pActBand && pActBand->maDirections[nWwCol] == 3)
    {
        pIo->m_pCtrlStck->NewAttr(*pIo->m_pPaM->GetPoint(),
            SvxCharRotateItem(900, false, RES_CHRATR_ROTATE));
    }
}

void WW8TabDesc::EndMiserableHackForUnsupportedDirection(short nWwCol)
{
    OSL_ENSURE(pActBand, "Impossible");
    if (pActBand && pActBand->maDirections[nWwCol] == 3)
        pIo->m_pCtrlStck->SetAttr(*pIo->m_pPaM->GetPoint(), RES_CHRATR_ROTATE);
}

bool WW8TabDesc::SetPamInCell(short nWwCol, bool bPam)
{
    OSL_ENSURE( pActBand, "pActBand ist 0" );
    if (!pActBand)
        return false;

    sal_uInt16 nCol = pActBand->transCell(nWwCol);

    if ((sal_uInt16)nAktRow >= pTabLines->size())
    {
        OSL_ENSURE(false, "Actual row bigger than expected." );
        if (bPam)
            MoveOutsideTable();
        return false;
    }

    pTabLine = (*pTabLines)[nAktRow];
    pTabBoxes = &pTabLine->GetTabBoxes();

    if (nCol >= pTabBoxes->size())
    {
        if (bPam)
        {
            // The first paragraph in a cell with upper autospacing has upper
            // spacing set to 0
            if (
                 pIo->m_bParaAutoBefore && pIo->m_bFirstPara &&
                 !pIo->m_pWDop->fDontUseHTMLAutoSpacing
               )
            {
                pIo->SetUpperSpacing(*pIo->m_pPaM, 0);
            }

            // The last paragraph in a cell with lower autospacing has lower
            // spacing set to 0
            if (pIo->m_bParaAutoAfter && !pIo->m_pWDop->fDontUseHTMLAutoSpacing)
                pIo->SetLowerSpacing(*pIo->m_pPaM, 0);

            ParkPaM();
        }
        return false;
    }
    pTabBox = (*pTabBoxes)[nCol];
    if( !pTabBox->GetSttNd() )
    {
        OSL_ENSURE(pTabBox->GetSttNd(), "Probleme beim Aufbau der Tabelle");
        if (bPam)
            MoveOutsideTable();
        return false;
    }
    if (bPam)
    {
        pAktWWCell = &pActBand->pTCs[ nWwCol ];

       // The first paragraph in a cell with upper autospacing has upper spacing set to 0
        if(pIo->m_bParaAutoBefore && pIo->m_bFirstPara && !pIo->m_pWDop->fDontUseHTMLAutoSpacing)
            pIo->SetUpperSpacing(*pIo->m_pPaM, 0);

        // The last paragraph in a cell with lower autospacing has lower spacing set to 0
        if(pIo->m_bParaAutoAfter && !pIo->m_pWDop->fDontUseHTMLAutoSpacing)
            pIo->SetLowerSpacing(*pIo->m_pPaM, 0);

        //We need to set the pPaM on the first cell, invalid
        //or not so that we can collect paragraph properties over
        //all the cells, but in that case on the valid cell we do not
        //want to reset the fmt properties
        sal_uLong nSttNd = pTabBox->GetSttIdx() + 1,
                  nEndNd = pTabBox->GetSttNd()->EndOfSectionIndex();
        if (pIo->m_pPaM->GetPoint()->nNode != nSttNd)
        {
            do
            {
                pIo->m_pPaM->GetPoint()->nNode = nSttNd;
            }
            while (pIo->m_pPaM->GetNode().GetNodeType() != ND_TEXTNODE && ++nSttNd < nEndNd);
            pIo->m_pPaM->GetPoint()->nContent.Assign(pIo->m_pPaM->GetContentNode(), 0);
            // Precautionally set now, otherwise the style is not set for cells
            // that are inserted for margin balancing.
            pIo->m_rDoc.SetTextFormatColl(*pIo->m_pPaM, const_cast<SwTextFormatColl*>(pIo->m_pDfltTextFormatColl));
            // because this cells are invisible helper constructions only to simulate
            // the frayed view of WW-tables we do NOT need SetTextFormatCollAndListLevel()
        }

        // Better to turn Snap to Grid off for all paragraphs in tables
        if(SwTextNode *pNd = pIo->m_pPaM->GetNode().GetTextNode())
        {
            const SfxPoolItem &rItm = pNd->SwContentNode::GetAttr(RES_PARATR_SNAPTOGRID);
            const SvxParaGridItem &rSnapToGrid = static_cast<const SvxParaGridItem&>(rItm);

            if(rSnapToGrid.GetValue())
            {
                SvxParaGridItem aGridItem( rSnapToGrid );
                aGridItem.SetValue(false);

                SwPosition* pGridPos = pIo->m_pPaM->GetPoint();

                const sal_Int32 nEnd = pGridPos->nContent.GetIndex();
                pGridPos->nContent.Assign(pIo->m_pPaM->GetContentNode(), 0);
                pIo->m_pCtrlStck->NewAttr(*pGridPos, aGridItem);
                pGridPos->nContent.Assign(pIo->m_pPaM->GetContentNode(), nEnd);
                pIo->m_pCtrlStck->SetAttr(*pGridPos, RES_PARATR_SNAPTOGRID);
            }
        }

        StartMiserableHackForUnsupportedDirection(nWwCol);
    }
    return true;
}

void WW8TabDesc::InsertCells( short nIns )
{
    pTabLine = (*pTabLines)[nAktRow];
    pTabBoxes = &pTabLine->GetTabBoxes();
    pTabBox = (*pTabBoxes)[0];

    pIo->m_rDoc.GetNodes().InsBoxen( pTableNd, pTabLine, static_cast<SwTableBoxFormat*>(pTabBox->GetFrameFormat()),
                            const_cast<SwTextFormatColl*>(pIo->m_pDfltTextFormatColl), nullptr, pTabBoxes->size(), nIns );
    // The third parameter contains the FrameFormat of the boxes.
    // Here it is possible to optimize to save (reduce) FrameFormats.
}

void WW8TabDesc::SetTabBorders(SwTableBox* pBox, short nWwIdx)
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // faked cells -> no border

    SvxBoxItem aFormatBox( RES_BOX );
    if (pActBand->pTCs)     // neither Cell Border nor Default Border defined ?
    {
        WW8_TCell* pT = &pActBand->pTCs[nWwIdx];
        if (SwWW8ImplReader::IsBorder(pT->rgbrc))
            SwWW8ImplReader::SetBorder(aFormatBox, pT->rgbrc);
    }

    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwTOP))
    {
        aFormatBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwTOP],
            SvxBoxItemLine::TOP);
    }
    else
        aFormatBox.SetDistance(pActBand->mnDefaultTop, SvxBoxItemLine::TOP);
    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwBOTTOM))
    {
        aFormatBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwBOTTOM],
            SvxBoxItemLine::BOTTOM);
    }
    else
        aFormatBox.SetDistance(pActBand->mnDefaultBottom,SvxBoxItemLine::BOTTOM);

    // nGapHalf for WW is a *horizontal* gap between table cell and content.
    short nLeftDist =
        pActBand->mbHasSpacing ? pActBand->mnDefaultLeft : pActBand->nGapHalf;
    short nRightDist =
        pActBand->mbHasSpacing ? pActBand->mnDefaultRight : pActBand->nGapHalf;
    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwLEFT))
    {
        aFormatBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwLEFT],
            SvxBoxItemLine::LEFT);
    }
    else
        aFormatBox.SetDistance(nLeftDist, SvxBoxItemLine::LEFT);
    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwRIGHT))
    {
        aFormatBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwRIGHT],
            SvxBoxItemLine::RIGHT);
    }
    else
        aFormatBox.SetDistance(nRightDist,SvxBoxItemLine::RIGHT);

    pBox->GetFrameFormat()->SetFormatAttr(aFormatBox);
}

void WW8TabDesc::SetTabShades( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // faked cells -> no color

    bool bFound=false;
    if (pActBand->pNewSHDs && pActBand->pNewSHDs[nWwIdx] != COL_AUTO)
    {
        Color aColor(pActBand->pNewSHDs[nWwIdx]);
        pBox->GetFrameFormat()->SetFormatAttr(SvxBrushItem(aColor, RES_BACKGROUND));
        bFound = true;
    }

    //If there was no new shades, or no new shade setting
    if (pActBand->pSHDs && !bFound)
    {
        WW8_SHD& rSHD = pActBand->pSHDs[nWwIdx];
        if (!rSHD.GetValue())       // auto
            return;

        SwWW8Shade aSh( pIo->m_bVer67, rSHD );
        pBox->GetFrameFormat()->SetFormatAttr(SvxBrushItem(aSh.aColor, RES_BACKGROUND));
    }
}

SvxFrameDirection MakeDirection(sal_uInt16 nCode, bool bIsBiDi)
{
    SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
    // 1: Asian layout with rotated CJK characters
    // 5: Asian layout
    // 3: Western layout rotated by 90 degrees
    // 4: Western layout
    switch (nCode)
    {
        default:
            OSL_ENSURE(eDir == 4, "unknown direction code, maybe it's a bitfield");
            //fall-through
        case 3:
            eDir = bIsBiDi ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP; // #i38158# - Consider RTL tables
            break;
        case 5:
            eDir = FRMDIR_VERT_TOP_RIGHT;
            break;
        case 1:
            eDir = FRMDIR_VERT_TOP_RIGHT;
            break;
        case 4:
            eDir = bIsBiDi ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP; // #i38158# - Consider RTL tables
            break;
    }
    return eDir;
}

void WW8TabDesc::SetTabDirection(SwTableBox* pBox, short nWwIdx)
{
    if (nWwIdx < 0 || nWwIdx >= pActBand->nWwCols)
        return;
    SvxFrameDirectionItem aItem(MakeDirection(pActBand->maDirections[nWwIdx], bIsBiDi), RES_FRAMEDIR);
    pBox->GetFrameFormat()->SetFormatAttr(aItem);
}

void WW8TabDesc::SetTabVertAlign( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;

    sal_Int16 eVertOri=text::VertOrientation::TOP;

    if( pActBand->pTCs )
    {
        WW8_TCell* pT = &pActBand->pTCs[nWwIdx];
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
    if( pActBand->nSwCols > nDefaultSwCols )        // split cells
        InsertCells( pActBand->nSwCols - nDefaultSwCols );

    SetPamInCell( 0, false);
    OSL_ENSURE( pTabBoxes && pTabBoxes->size() == (sal_uInt16)pActBand->nSwCols,
        "Falsche Spaltenzahl in Tabelle" );

    if( bClaimLineFormat )
    {
        pTabLine->ClaimFrameFormat();            // necessary because of cell height
        SwFormatFrameSize aF( ATT_MIN_SIZE, 0, 0 );  // default

        if (pActBand->nLineHeight == 0)    // 0 = Auto
            aF.SetHeightSizeType( ATT_VAR_SIZE );
        else
        {
            if (pActBand->nLineHeight < 0) // positive = min, negative = exact
            {
                aF.SetHeightSizeType(ATT_FIX_SIZE);
                pActBand->nLineHeight = -pActBand->nLineHeight;
            }
            if (pActBand->nLineHeight < MINLAY) // invalid cell height
                pActBand->nLineHeight = MINLAY;

            aF.SetHeight(pActBand->nLineHeight);// set min/exact height
        }
        pTabLine->GetFrameFormat()->SetFormatAttr(aF);
    }

    //Word stores 1 for bCantSplit if the row cannot be split, we set true if
    //we can split the row
    bool bSetCantSplit = pActBand->bCantSplit;
    pTabLine->GetFrameFormat()->SetFormatAttr(SwFormatRowSplit(!bSetCantSplit));

    short i;    // SW-Index
    short j;    // WW-Index
    short nW;   // Width
    SwFormatFrameSize aFS( ATT_FIX_SIZE );
    j = pActBand->bLEmptyCol ? -1 : 0;

    for( i = 0; i < pActBand->nSwCols; i++ )
    {
        // set cell width
        if( j < 0 )
            nW = pActBand->nCenter[0] - nMinLeft;
        else
        {
            //Set j to first non invalid cell
            while ((j < pActBand->nWwCols) && (!pActBand->bExist[j]))
                j++;

            if( j < pActBand->nWwCols )
                nW = pActBand->nCenter[j+1] - pActBand->nCenter[j];
            else
                nW = nMaxRight - pActBand->nCenter[j];
            pActBand->nWidth[ j ] = nW;
        }

        SwTableBox* pBox = (*pTabBoxes)[i];
        // could be reduced further by intelligent moving of FrameFormats
        pBox->ClaimFrameFormat();

        SetTabBorders(pBox, j);

        // #i18128# word has only one line between adjoining vertical cells
        // we have to mimic this in the filter by picking the larger of the
        // sides and using that one on one side of the line (right)
        SvxBoxItem aCurrentBox(sw::util::ItemGet<SvxBoxItem>(*(pBox->GetFrameFormat()), RES_BOX));
        const ::editeng::SvxBorderLine *pLeftLine = aCurrentBox.GetLine(SvxBoxItemLine::LEFT);
        int nCurrentRightLineWidth = 0;
        if(pLeftLine)
            nCurrentRightLineWidth = pLeftLine->GetScaledWidth();

        if (i != 0)
        {
            SwTableBox* pBox2 = (*pTabBoxes)[i-1];
            SvxBoxItem aOldBox(sw::util::ItemGet<SvxBoxItem>(*(pBox2->GetFrameFormat()), RES_BOX));
            const ::editeng::SvxBorderLine *pRightLine = aOldBox.GetLine(SvxBoxItemLine::RIGHT);
            int nOldBoxRightLineWidth = 0;
            if(pRightLine)
                nOldBoxRightLineWidth = pRightLine->GetScaledWidth();

            if(nOldBoxRightLineWidth>nCurrentRightLineWidth)
                aCurrentBox.SetLine(aOldBox.GetLine(SvxBoxItemLine::RIGHT), SvxBoxItemLine::LEFT);

            aOldBox.SetLine(nullptr, SvxBoxItemLine::RIGHT);
            pBox2->GetFrameFormat()->SetFormatAttr(aOldBox);
        }

        pBox->GetFrameFormat()->SetFormatAttr(aCurrentBox);

        SetTabVertAlign(pBox, j);
        SetTabDirection(pBox, j);
        if( pActBand->pSHDs || pActBand->pNewSHDs)
            SetTabShades(pBox, j);
        j++;

        aFS.SetWidth( nW );
        pBox->GetFrameFormat()->SetFormatAttr( aFS );

        // skip non existing cells
        while( ( j < pActBand->nWwCols ) && !pActBand->bExist[j] )
        {
            pActBand->nWidth[j] = pActBand->nCenter[j+1] - pActBand->nCenter[j];
            j++;
        }
    }
}

void WW8TabDesc::TableCellEnd()
{
    ::SetProgressState(pIo->m_nProgress, pIo->m_pDocShell);   // Update

    EndMiserableHackForUnsupportedDirection(nAktCol);

    // new line/row
    if( pIo->m_bWasTabRowEnd )
    {
        // bWasTabRowEnd will be deactivated in
        // SwWW8ImplReader::ProcessSpecial()

        sal_uInt16 iCol = GetLogicalWWCol();
        if (iCol < aNumRuleNames.size())
        {
            aNumRuleNames.erase(aNumRuleNames.begin() + iCol,
                aNumRuleNames.end());
        }

        nAktCol = 0;
        nAktRow++;
        nAktBandRow++;
        OSL_ENSURE( pActBand , "pActBand ist 0" );
        if( pActBand )
        {
            if( nAktRow >= nRows )  // nothing to at end of table
                return;

            bool bNewBand = nAktBandRow >= pActBand->nRows;
            if( bNewBand )
            {                       // new band needed ?
                pActBand = pActBand->pNextBand;
                nAktBandRow = 0;
                OSL_ENSURE( pActBand, "pActBand ist 0" );
                AdjustNewBand();
            }
            else
            {
                SwTableBox* pBox = (*pTabBoxes)[0];
                SwSelBoxes aBoxes;
                pIo->m_rDoc.InsertRow( SwTable::SelLineFromBox( pBox, aBoxes ) );
            }
        }
    }
    else
    {                       // new column ( cell )
        nAktCol++;
    }
    SetPamInCell(nAktCol, true);

    // finish Annotated Level Numbering ?
    if (pIo->m_bAnl && !pIo->m_bAktAND_fNumberAcross && pActBand)
        pIo->StopAllAnl(IsValidCell(nAktCol));
}

// if necessary register the box for the merge group for this column
SwTableBox* WW8TabDesc::UpdateTableMergeGroup(  WW8_TCell&     rCell,
                                                WW8SelBoxInfo* pActGroup,
                                                SwTableBox*    pActBox,
                                                sal_uInt16         nCol )
{
    // set default for return
    SwTableBox* pResult = nullptr;

    // check if the box has to be merged
    // If cell is the first one to be merged, a new merge group has to be provided.
    // E.g., it could be that a cell is the first one to be merged, but no
    // new merge group is provided, because the potential other cell to be merged
    // doesn't exist - see method <WW8TabDesc::MergeCells>.
    if ( pActBand->bExist[ nCol ] &&
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
                pActBand->nCenter[ nCol ], pActBand->nWidth[  nCol ], true );
        }
        if( pTheMergeGroup )
        {
            // add current box to merge group
            pTheMergeGroup->push_back(pActBox);
            // return target box
            pResult = (*pTheMergeGroup)[ 0 ];
        }
    }
    return pResult;
}

sal_uInt16 WW8TabDesc::GetLogicalWWCol() const // returns number of col as INDICATED within WW6 UI status line -1
{
    sal_uInt16 nCol = 0;
    if( pActBand && pActBand->pTCs)
    {
        for( sal_uInt16 iCol = 1; iCol <= nAktCol && iCol <= pActBand->nWwCols; ++iCol )
        {
            if( !pActBand->pTCs[ iCol-1 ].bMerged )
                ++nCol;
        }
    }
    return nCol;
}

// find name of numrule valid for current WW-COL
OUString WW8TabDesc::GetNumRuleName() const
{
    sal_uInt16 nCol = GetLogicalWWCol();
    if (nCol < aNumRuleNames.size())
        return aNumRuleNames[nCol];
    return OUString();
}

void WW8TabDesc::SetNumRuleName( const OUString& rName )
{
    sal_uInt16 nCol = GetLogicalWWCol();
    for (sal_uInt16 nSize = static_cast< sal_uInt16 >(aNumRuleNames.size()); nSize <= nCol; ++nSize)
        aNumRuleNames.push_back(OUString());
    aNumRuleNames[nCol] = rName;
}

bool SwWW8ImplReader::StartTable(WW8_CP nStartCp, SvxULSpaceItem* pULSpaceItem)
{
    // Entering a table so make sure the FirstPara flag gets set
    m_bFirstPara = true;
    // keine rekursiven Tabellen Nicht bei EinfuegenDatei in Tabelle oder
    // Fussnote
    if (m_bReadNoTable)
        return false;

    if (m_pTableDesc)
        m_aTableStack.push(m_pTableDesc);

    // #i33818# - determine absolute position object attributes,
    // if possible. It's needed for nested tables.
    WW8FlyPara* pTableWFlyPara( nullptr );
    WW8SwFlyPara* pTableSFlyPara( nullptr );
    // #i45301# - anchor nested table inside Writer fly frame
    // only at-character, if absolute position object attributes are available.
    // Thus, default anchor type is as-character anchored.
    RndStdIds eAnchor( FLY_AS_CHAR );
    if ( m_nInTable )
    {
        WW8_TablePos* pNestedTabPos( nullptr );
        WW8_TablePos aNestedTabPos;
        WW8PLCFxSave1 aSave;
        m_pPlcxMan->GetPap()->Save( aSave );
        WW8PLCFx_Cp_FKP* pPap = m_pPlcxMan->GetPapPLCF();
        WW8_CP nMyStartCp = nStartCp;
        if ( SearchRowEnd( pPap, nMyStartCp, m_nInTable ) &&
             ParseTabPos( &aNestedTabPos, pPap ) )
        {
            pNestedTabPos = &aNestedTabPos;
        }
        m_pPlcxMan->GetPap()->Restore( aSave );
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
                    m_aSectionManager.GetPageLeft(), m_aSectionManager.GetTextAreaWidth(),
                    m_nIniFlyDx, m_nIniFlyDy);

                // #i45301# - anchor nested table Writer fly frame at-character
                eAnchor = FLY_AT_CHAR;
            }
        }
    }

    m_pTableDesc = new WW8TabDesc( this, nStartCp );

    if( m_pTableDesc->Ok() )
    {
        int nNewInTable = m_nInTable + 1;

        if ((eAnchor == FLY_AT_CHAR)
            && !m_aTableStack.empty() && !InEqualApo(nNewInTable) )
        {
            m_pTableDesc->pParentPos = new SwPosition(*m_pPaM->GetPoint());
            SfxItemSet aItemSet(m_rDoc.GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1);
            // #i33818# - anchor the Writer fly frame for the nested table at-character.
            // #i45301#
            SwFormatAnchor aAnchor( eAnchor );
            aAnchor.SetAnchor( m_pTableDesc->pParentPos );
            aItemSet.Put( aAnchor );
            m_pTableDesc->pFlyFormat = m_rDoc.MakeFlySection( eAnchor,
                                                      m_pTableDesc->pParentPos, &aItemSet);
            OSL_ENSURE( m_pTableDesc->pFlyFormat->GetAnchor().GetAnchorId() == eAnchor,
                   "Not the anchor type requested!" );
            MoveInsideFly(m_pTableDesc->pFlyFormat);
        }
        m_pTableDesc->CreateSwTable(pULSpaceItem);
        if (m_pTableDesc->pFlyFormat)
        {
            m_pTableDesc->SetSizePosition(m_pTableDesc->pFlyFormat);
            // #i33818# - Use absolute position object attributes,
            // if existing, and apply them to the created Writer fly frame.
            if ( pTableWFlyPara && pTableSFlyPara )
            {
                WW8FlySet aFlySet( *this, pTableWFlyPara, pTableSFlyPara, false );
                SwFormatAnchor aAnchor( FLY_AT_CHAR );
                aAnchor.SetAnchor( m_pTableDesc->pParentPos );
                aFlySet.Put( aAnchor );
                m_pTableDesc->pFlyFormat->SetFormatAttr( aFlySet );
            }
            else
            {
                SwFormatHoriOrient aHori =
                            m_pTableDesc->pTable->GetFrameFormat()->GetHoriOrient();
                m_pTableDesc->pFlyFormat->SetFormatAttr(aHori);
                m_pTableDesc->pFlyFormat->SetFormatAttr( SwFormatSurround( SURROUND_NONE ) );
            }
            // #i33818# - The nested table doesn't have to leave
            // the table cell. Thus, the Writer fly frame has to follow the text flow.
            m_pTableDesc->pFlyFormat->SetFormatAttr( SwFormatFollowTextFlow( true ) );
        }
        else
            m_pTableDesc->SetSizePosition(nullptr);
        m_pTableDesc->UseSwTable();
    }
    else
        PopTableDesc();

    // #i33818#
    delete pTableWFlyPara;
    delete pTableSFlyPara;

    return nullptr != m_pTableDesc;
}

void SwWW8ImplReader::TabCellEnd()
{
    if (m_nInTable && m_pTableDesc)
        m_pTableDesc->TableCellEnd();

    m_bFirstPara = true;    // We have come to the end of a cell so FirstPara flag
    m_bReadTable = false;
    m_pTableEndPaM.reset();
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
    if (m_pTableDesc && m_pTableDesc->pFlyFormat)
    {
        MoveOutsideFly(m_pTableDesc->pFlyFormat,*m_pTableDesc->pParentPos);
    }

    delete m_pTableDesc;
    if (m_aTableStack.empty())
        m_pTableDesc = nullptr;
    else
    {
       m_pTableDesc = m_aTableStack.top();
       m_aTableStack.pop();
    }
}

void SwWW8ImplReader::StopTable()
{
    OSL_ENSURE(m_pTableDesc, "Panic, stop table with no table!");
    if (!m_pTableDesc)
        return;

    // We are leaving a table so make sure the next paragraph doesn't think
    // it's the first paragraph
    m_bFirstPara = false;

    m_pTableDesc->FinishSwTable();
    PopTableDesc();

    m_bReadTable = true;
    // #i101116# - Keep PaM on table end only for nested tables
    if ( m_nInTable > 1 )
    {
        m_pTableEndPaM.reset(new SwPaM(*m_pPaM, m_pPaM));
    }
}

bool SwWW8ImplReader::IsInvalidOrToBeMergedTabCell() const
{
    if( !m_pTableDesc )
        return false;

    const WW8_TCell* pCell = m_pTableDesc->GetAktWWCell();

    return     !m_pTableDesc->IsValidCell( m_pTableDesc->GetAktCol() )
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
        for(sal_uInt16 nI = 0; nI < m_pStyles->GetCount(); nI++ )
            if(    m_vColl[ nI ].m_bValid
                && (nLFOIndex == m_vColl[ nI ].m_nLFOIndex) )
                nRes = nI;
    }
    return nRes;
}

const SwFormat* SwWW8ImplReader::GetStyleWithOrgWWName( OUString& rName ) const
{
    SwFormat* pRet = nullptr;
    if( !m_vColl.empty() )
    {
        for(sal_uInt16 nI = 0; nI < m_pStyles->GetCount(); nI++ )
            if(    m_vColl[ nI ].m_bValid
                && (rName.equals( m_vColl[ nI ].GetOrgWWName())) )
            {
                pRet = m_vColl[ nI ].m_pFormat;
                break;
            }
    }
    return pRet;
}

//          class WW8RStyle

const sal_uInt8* WW8RStyle::HasParaSprm( sal_uInt16 nId ) const
{
    if( !pParaSprms || !nSprmsLen )
        return nullptr;

    return maSprmParser.findSprmData(nId, pParaSprms, nSprmsLen);
}

void WW8RStyle::ImportSprms(sal_uInt8 *pSprms, short nLen, bool bPap)
{
    if (!nLen)
        return;

    if( bPap )
    {
        pParaSprms = pSprms;   // for HasParaSprms()
        nSprmsLen = nLen;
    }

    WW8SprmIter aSprmIter(pSprms, nLen, maSprmParser);
    while (const sal_uInt8* pSprm = aSprmIter.GetSprms())
    {
        pIo->ImportSprm(pSprm);
        aSprmIter.advance();
    }

    pParaSprms = nullptr;
    nSprmsLen = 0;
}

void WW8RStyle::ImportSprms(sal_Size nPosFc, short nLen, bool bPap)
{
    if (!nLen)
        return;

    if (checkSeek(*pStStrm, nPosFc))
    {
        sal_uInt8 *pSprms = new sal_uInt8[nLen];
        nLen = pStStrm->Read(pSprms, nLen);
        ImportSprms(pSprms, nLen, bPap);
        delete[] pSprms;
    }
}

static inline short WW8SkipOdd(SvStream* pSt )
{
    if ( pSt->Tell() & 0x1 )
    {
        sal_uInt8 c;
        return pSt->Read( &c, 1 );
    }
    return 0;
}

static inline short WW8SkipEven(SvStream* pSt )
{
    if (!(pSt->Tell() & 0x1))
    {
        sal_uInt8 c;
        return pSt->Read( &c, 1 );
    }
    return 0;
}

short WW8RStyle::ImportUPX(short nLen, bool bPAP, bool bOdd)
{
    if( 0 < nLen ) // Empty ?
    {
        if (bOdd)
            nLen = nLen - WW8SkipEven( pStStrm );
        else
            nLen = nLen - WW8SkipOdd( pStStrm );

        sal_Int16 cbUPX(0);
        pStStrm->ReadInt16( cbUPX );

        nLen-=2;

        if ( cbUPX > nLen )
            cbUPX = nLen;       // shrink cbUPX to nLen

        if( (1 < cbUPX) || ( (0 < cbUPX) && !bPAP ) )
        {
            if( bPAP )
            {
                sal_uInt16 id;
                pStStrm->ReadUInt16( id );

                cbUPX-=  2;
                nLen-=  2;
            }

            if( 0 < cbUPX )
            {
                sal_Size nPos = pStStrm->Tell(); // if something is interpreted wrong,
                                                 // this should make it work again
                ImportSprms( nPos, cbUPX, bPAP );

                if ( pStStrm->Tell() != nPos + cbUPX )
                    pStStrm->Seek( nPos+cbUPX );

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
        nLen = nLen - WW8SkipEven( pStStrm );
    else
        nLen = nLen - WW8SkipOdd( pStStrm );

    if( bPara ) // Grupx.Papx
        nLen = ImportUPX(nLen, true, bOdd);
    ImportUPX(nLen, false, bOdd);                   // Grupx.Chpx
}

WW8RStyle::WW8RStyle(WW8Fib& _rFib, SwWW8ImplReader* pI)
    : WW8Style(*pI->m_pTableStream, _rFib)
    , maSprmParser(_rFib.GetFIBVersion())
    , pIo(pI)
    , pStStrm(pI->m_pTableStream)
    , pStyRule(nullptr)
    , pParaSprms(nullptr)
    , nSprmsLen(0)
    , nWwNumLevel(0)
    , bTextColChanged(false)
    , bFontChanged(false)
    , bCJKFontChanged(false)
    , bCTLFontChanged(false)
    , bFSizeChanged(false)
    , bFCTLSizeChanged(false)
    , bWidowsChanged(false)
{
    pIo->m_vColl.resize(cstd);
}

void WW8RStyle::Set1StyleDefaults()
{
    // see #i25247#, #i25561#, #i48064#, #i92341# for default font
    if (!bCJKFontChanged)   // Style no CJK Font? set the default
        pIo->SetNewFontAttr(ftcFE, true, RES_CHRATR_CJK_FONT);

    if (!bCTLFontChanged)   // Style no CTL Font? set the default
        pIo->SetNewFontAttr(ftcBi, true, RES_CHRATR_CTL_FONT);

    // western 2nd to make western charset conversion the default
    if (!bFontChanged)      // Style has no Font? set the default,
        pIo->SetNewFontAttr(ftcAsci, true, RES_CHRATR_FONT);

    if( !pIo->m_bNoAttrImport )
    {
        // Style has no text color set, winword default is auto
        if ( !bTextColChanged )
            pIo->m_pAktColl->SetFormatAttr(SvxColorItem(Color(COL_AUTO), RES_CHRATR_COLOR));

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !bFSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            pIo->m_pAktColl->SetFormatAttr(aAttr);
            aAttr.SetWhich(RES_CHRATR_CJK_FONTSIZE);
            pIo->m_pAktColl->SetFormatAttr(aAttr);
        }

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !bFCTLSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            aAttr.SetWhich(RES_CHRATR_CTL_FONTSIZE);
            pIo->m_pAktColl->SetFormatAttr(aAttr);
        }

        if( !bWidowsChanged )  // Widows ?
        {
            pIo->m_pAktColl->SetFormatAttr( SvxWidowsItem( 2, RES_PARATR_WIDOWS ) );
            pIo->m_pAktColl->SetFormatAttr( SvxOrphansItem( 2, RES_PARATR_ORPHANS ) );
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
            pIo->m_aParaStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }
    else
    {
        // Char-Style
        sw::util::CharStyleMapper::StyleResult aResult =
            pIo->m_aCharStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }

    bool bImport = !bStyExist || pIo->m_bNewDoc; // import content ?

    // Do not override character styles the list import code created earlier.
    if (bImport && bStyExist && rSI.GetOrgWWName().startsWith("WW8Num"))
        bImport = false;

    bool bOldNoImp = pIo->m_bNoAttrImport;
    rSI.m_bImportSkipped = !bImport;

    if( !bImport )
        pIo->m_bNoAttrImport = true;
    else
    {
        if (bStyExist)
        {
            pColl->ResetAllFormatAttr(); // #i73790# - method renamed
        }
        pColl->SetAuto(false);          // suggested by JP
    }                                   // but changes the UI
    pIo->m_pAktColl = pColl;
    rSI.m_pFormat = pColl;                  // remember translation WW->SW
    rSI.m_bImportSkipped = !bImport;

    // Set Based on style
    sal_uInt16 j = rSI.m_nBase;
    if (j != nThisStyle && j < cstd )
    {
        SwWW8StyInf* pj = &pIo->m_vColl[j];
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

            if (pj->m_pWWFly)
                rSI.m_pWWFly = new WW8FlyPara(pIo->m_bVer67, pj->m_pWWFly);
        }
    }
    else if( pIo->m_bNewDoc && bStyExist )
        rSI.m_pFormat->SetDerivedFrom();

    rSI.m_nFollow = nNextStyle;       // remember Follow

    pStyRule = nullptr;                   // recreate if necessary
    bTextColChanged = bFontChanged = bCJKFontChanged = bCTLFontChanged =
        bFSizeChanged = bFCTLSizeChanged = bWidowsChanged = false;
    pIo->SetNAktColl( nThisStyle );
    pIo->m_bStyNormal = nThisStyle == 0;
    return bOldNoImp;
}

void WW8RStyle::PostStyle(SwWW8StyInf &rSI, bool bOldNoImp)
{
    // Reset attribute flags, because there are no style-ends.

    pIo->m_bHasBorder = pIo->m_bSpec = pIo->m_bObj = pIo->m_bSymbol = false;
    pIo->m_nCharFormat = -1;

    // If Style basiert auf Nichts oder Basis ignoriert
    if ((rSI.m_nBase >= cstd || pIo->m_vColl[rSI.m_nBase].m_bImportSkipped) && rSI.m_bColl)
    {
        // If Char-Styles does not work
        // -> set hard WW-Defaults
        Set1StyleDefaults();
    }

    pStyRule = nullptr;                   // to be on the safe side
    pIo->m_bStyNormal = false;
    pIo->SetNAktColl( 0 );
    pIo->m_bNoAttrImport = bOldNoImp;
    // reset the list-remember-fields, if used when reading styles
    pIo->m_nLFOPosition = USHRT_MAX;
    pIo->m_nListLevel = WW8ListManager::nMaxLevel;
}

void WW8RStyle::Import1Style( sal_uInt16 nNr )
{
    if (nNr >= pIo->m_vColl.size())
        return;

    SwWW8StyInf &rSI = pIo->m_vColl[nNr];

    if( rSI.m_bImported || !rSI.m_bValid )
        return;

    rSI.m_bImported = true;                      // set flag now to avoid endless loops

    // valid and not NUL and not yet imported

    if( rSI.m_nBase < cstd && !pIo->m_vColl[rSI.m_nBase].m_bImported )
        Import1Style( rSI.m_nBase );

    pStStrm->Seek( rSI.m_nFilePos );

    short nSkip, cbStd;
    OUString sName;

    std::unique_ptr<WW8_STD> xStd(Read1Style(nSkip, &sName, &cbStd));// read Style

    if (xStd)
        rSI.SetOrgWWIdent( sName, xStd->sti );

    // either no Name or unused Slot or unknown Style

    if ( !xStd || sName.isEmpty() || ((1 != xStd->sgc) && (2 != xStd->sgc)) )
    {
        pStStrm->SeekRel( nSkip );
        return;
    }

    bool bOldNoImp = PrepareStyle(rSI, static_cast<ww::sti>(xStd->sti), nNr, xStd->istdNext);

    // if something is interpreted wrong, this should make it work again
    long nPos = pStStrm->Tell();

    //Variable parts of the STD start at even byte offsets, but "inside
    //the STD", which I take to meaning even in relation to the starting
    //position of the STD, which matches findings in #89439#, generally it
    //doesn't matter as the STSHI starts off nearly always on an even
    //offset

    //Import of the Style Contents
    ImportGrupx(nSkip, xStd->sgc == 1, rSI.m_nFilePos & 1);

    PostStyle(rSI, bOldNoImp);

    pStStrm->Seek( nPos+nSkip );
}

void WW8RStyle::RecursiveReg(sal_uInt16 nNr)
{
    if (nNr >= pIo->m_vColl.size())
        return;

    SwWW8StyInf &rSI = pIo->m_vColl[nNr];
    if( rSI.m_bImported || !rSI.m_bValid )
        return;

    rSI.m_bImported = true;

    if( rSI.m_nBase < cstd && !pIo->m_vColl[rSI.m_nBase].m_bImported )
        RecursiveReg(rSI.m_nBase);

    pIo->RegisterNumFormatOnStyle(nNr);

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
    for (i=0; i < cstd; ++i)
        pIo->m_vColl[i].m_bImported = false;

    /*
     Register the num formats and tabstop changes on the styles recursively.
    */

    /*
     In the same loop apply the tabstop changes required because we need to
     change their location if theres a special indentation for the first line,
     By avoiding making use of each styles margins during reading of their
     tabstops we don't get problems with doubly adjusting tabstops that
     are inheritied.
    */
    for (i=0; i < cstd; ++i)
    {
        if (pIo->m_vColl[i].m_bValid)
        {
            RecursiveReg(i);
        }
    }
}

void WW8RStyle::ScanStyles()        // investigate style dependencies
{                                   // and detect Filepos for each Style
    for (sal_uInt16 i = 0; i < cstd; ++i)
    {
        short nSkip;
        SwWW8StyInf &rSI = pIo->m_vColl[i];

        rSI.m_nFilePos = pStStrm->Tell();        // remember FilePos
        WW8_STD* pStd = Read1Style( nSkip, nullptr, nullptr );  // read STD
        rSI.m_bValid = (nullptr != pStd);
        if (rSI.m_bValid)
        {
            rSI.m_nBase = pStd->istdBase;        // remember Basis
            rSI.m_bColl = ( pStd->sgc == 1 );    // Para-Style
        }
        else
            rSI = SwWW8StyInf();

        delete pStd;
        pStStrm->SeekRel( nSkip );              // skip Names and Sprms
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

Word2CHPX ReadWord2Chpx(SvStream &rSt, sal_Size nOffset, sal_uInt8 nSize)
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
    struct pxoffset { sal_Size mnOffset; sal_uInt8 mnSize; };
}

void WW8RStyle::ImportOldFormatStyles()
{
    for (sal_uInt16 i=0; i < cstd; ++i)
    {
        pIo->m_vColl[i].m_bColl = true;
        //every chain must end eventually at the null style (style code 222)
        pIo->m_vColl[i].m_nBase = 222;
    }

    rtl_TextEncoding eStructChrSet = WW8Fib::GetFIBCharset(
        pIo->m_pWwFib->chseTables, pIo->m_pWwFib->lid);

    sal_uInt16 cstcStd(0);
    rSt.ReadUInt16( cstcStd );

    size_t nMaxByteCount = rSt.remainingSize();
    sal_uInt16 cbName(0);
    rSt.ReadUInt16(cbName);
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
        rSt.ReadUChar( nCount );
        nByteCount++;

        sal_uInt8 stc = static_cast< sal_uInt8 >((stcp - cstcStd) & 255);
        if (stc >=pIo->m_vColl.size())
            continue;

        SwWW8StyInf &rSI = pIo->m_vColl[stc];
        OUString sName;

        if (nCount != 0xFF)    // undefined style
        {
            if (nCount != 0)   // user style
            {
                OString aTmp = read_uInt8s_ToOString(rSt, nCount);
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
    nMaxByteCount = rSt.remainingSize();
    sal_uInt16 cbChpx(0);
    rSt.ReadUInt16(cbChpx);
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
            rSt.SeekRel(cbChpx-nByteCount);
            nByteCount += cbChpx-nByteCount;
            break;
        }

        sal_uInt8 cb(0);
        rSt.ReadUChar( cb );
        nByteCount++;

        aCHPXOffsets[stcp].mnSize = 0;

        if (cb != 0xFF)
        {
            sal_uInt8 nRemainder = cb;

            aCHPXOffsets[stcp].mnOffset = rSt.Tell();
            aCHPXOffsets[stcp].mnSize = nRemainder;

            Word2CHPX aChpx = ReadWord2Chpx(rSt, aCHPXOffsets[stcp].mnOffset,
                aCHPXOffsets[stcp].mnSize);
            aConvertedChpx.push_back( ChpxToSprms(aChpx) );

            nByteCount += nRemainder;
        }
        else
            aConvertedChpx.push_back( std::vector<sal_uInt8>() );

        ++stcp;
    }

    std::vector<pxoffset> aPAPXOffsets(stcp);
    nMaxByteCount = rSt.remainingSize();
    sal_uInt16 cbPapx(0);
    rSt.ReadUInt16(cbPapx);
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
            rSt.SeekRel(cbPapx-nByteCount);
            nByteCount += cbPapx-nByteCount;
            break;
        }

        sal_uInt8 cb(0);
        rSt.ReadUChar( cb );
        nByteCount++;

        aPAPXOffsets[stcp].mnSize = 0;

        if (cb != 0xFF)
        {
            sal_uInt8 stc2(0);
            rSt.ReadUChar( stc2 );
            rSt.SeekRel(6);
            nByteCount+=7;
            sal_uInt8 nRemainder = cb-7;

            aPAPXOffsets[stcp].mnOffset = rSt.Tell();
            aPAPXOffsets[stcp].mnSize = nRemainder;

            rSt.SeekRel(nRemainder);
            nByteCount += nRemainder;
        }

        ++stcp;
    }

    sal_uInt16 iMac(0);
    rSt.ReadUInt16( iMac );

    if (iMac > nStyles) iMac = nStyles;

    for (stcp = 0; stcp < iMac; ++stcp)
    {
        sal_uInt8 stcNext(0), stcBase(0);
        rSt.ReadUChar( stcNext );
        rSt.ReadUChar( stcBase );

        sal_uInt8 stc = static_cast< sal_uInt8 >((stcp - cstcStd) & 255);

        /*
          #i64557# style based on itself
          every chain must end eventually at the null style (style code 222)
        */
        if (stc == stcBase)
            stcBase = 222;

        SwWW8StyInf &rSI = pIo->m_vColl[stc];
        rSI.m_nBase = stcBase;

        ww::sti eSti = ww::GetCanonicalStiFromStc(stc);

        if (eSti == ww::stiNil)
            continue;

        if (stcp >= aPAPXOffsets.size())
            continue;

        rSI.m_bValid = true;

        if (ww::StandardStiIsCharStyle(eSti) && !aPAPXOffsets[stcp].mnSize)
            pIo->m_vColl[stc].m_bColl = false;

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

    for (sal_uInt16 i = 0; i < cstd; ++i) // import Styles
        if (pIo->m_vColl[i].m_bValid)
            Import1Style( i );
}

void WW8RStyle::ImportStyles()
{
    if (pIo->m_pWwFib->GetFIBVersion() <= ww::eWW2)
        ImportOldFormatStyles();
    else
        ImportNewFormatStyles();
}

void WW8RStyle::Import()
{
    pIo->m_pDfltTextFormatColl  = pIo->m_rDoc.GetDfltTextFormatColl();
    pIo->m_pStandardFormatColl =
        pIo->m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD, false);

    if( pIo->m_nIniFlags & WW8FL_NO_STYLES )
        return;

    ImportStyles();

    for (sal_uInt16 i = 0; i < cstd; ++i)
    {
        // Follow chain
        SwWW8StyInf* pi = &pIo->m_vColl[i];
        sal_uInt16 j = pi->m_nFollow;
        if( j < cstd )
        {
            SwWW8StyInf* pj = &pIo->m_vColl[j];
            if ( j != i                             // rational Index ?
                 && pi->m_pFormat                        // Format ok ?
                 && pj->m_pFormat                        // Derived-Format ok ?
                 && pi->m_bColl                       // only possible for paragraph templates (WW)
                 && pj->m_bColl ){                    // identical Typ ?
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

    if( pIo->StyleExists(0) && !pIo->m_vColl.empty() &&
        pIo->m_vColl[0].m_pFormat && pIo->m_vColl[0].m_bColl && pIo->m_vColl[0].m_bValid )
        pIo->m_pDfltTextFormatColl = static_cast<SwTextFormatColl*>(pIo->m_vColl[0].m_pFormat);
    else
        pIo->m_pDfltTextFormatColl = pIo->m_rDoc.GetDfltTextFormatColl();

    // set Hyphenation flag on BASIC para-style
    if (pIo->m_bNewDoc && pIo->m_pStandardFormatColl)
    {
        if (pIo->m_pWDop->fAutoHyphen
            && SfxItemState::SET != pIo->m_pStandardFormatColl->GetItemState(
                                            RES_PARATR_HYPHENZONE, false) )
        {
            SvxHyphenZoneItem aAttr(true, RES_PARATR_HYPHENZONE);
            aAttr.GetMinLead()    = 2;
            aAttr.GetMinTrail()   = 2;
            aAttr.GetMaxHyphens() = 0;

            pIo->m_pStandardFormatColl->SetFormatAttr( aAttr );
        }

        /*
        Word defaults to ltr not from environment like writer. Regardless of
        the page/sections rtl setting the standard style lack of rtl still
        means ltr
        */
        if (SfxItemState::SET != pIo->m_pStandardFormatColl->GetItemState(RES_FRAMEDIR,
            false))
        {
           pIo->m_pStandardFormatColl->SetFormatAttr(
                SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));
        }
    }

    // we do not read styles anymore:
    pIo->m_pAktColl = nullptr;
}

rtl_TextEncoding SwWW8StyInf::GetCharSet() const
{
    if ((m_pFormat) && (m_pFormat->GetFrameDir().GetValue() == FRMDIR_HORI_RIGHT_TOP))
        return m_eRTLFontSrcCharSet;
    return m_eLTRFontSrcCharSet;
}

rtl_TextEncoding SwWW8StyInf::GetCJKCharSet() const
{
    if ((m_pFormat) && (m_pFormat->GetFrameDir().GetValue() == FRMDIR_HORI_RIGHT_TOP))
        return m_eRTLFontSrcCharSet;
    return m_eCJKFontSrcCharSet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
