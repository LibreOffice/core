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

#include <boost/scoped_ptr.hpp>
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
#include <editeng/fhgtitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/pgrditem.hxx>
#include <msfilter.hxx>
#include <pam.hxx>              // fuer SwPam
#include <doc.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>            // class SwTxtNode
#include <paratr.hxx>           // SwNumRuleItem
#include <poolfmt.hxx>          // RES_POOLCOLL_STANDARD
#include <swtable.hxx>          // class SwTableLines, ...
#include <tblsel.hxx>           // class _SwSelBox
#include <mdiexp.hxx>
#include <fmtpdsc.hxx>
#include <txtftn.hxx>
#include <frmfmt.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <charfmt.hxx>
#include <SwStyleNameMapper.hxx>
#include <fltshell.hxx>         // for the attribute stack
#include <fmtanchr.hxx>
#include <fmtrowsplt.hxx>
#include <fmtfollowtextflow.hxx> // #i33818#
#include <numrule.hxx>
#include "../inc/wwstyles.hxx"
#include "writerhelper.hxx"
#include "ww8struc.hxx"         // struct TC
#include "ww8par.hxx"
#include "ww8par2.hxx"

#include <frmatr.hxx>

#include <iostream>

using namespace ::com::sun::star;


class WW8SelBoxInfo
    : public std::vector<SwTableBox*>
{
private:
    WW8SelBoxInfo(const WW8SelBoxInfo&);
    WW8SelBoxInfo& operator=(const WW8SelBoxInfo&);
public:
    short nGroupXStart;
    short nGroupWidth;
    bool bGroupLocked;

    WW8SelBoxInfo(short nXCenter, short nWidth)
        : nGroupXStart( nXCenter ), nGroupWidth( nWidth ), bGroupLocked(false)
    {}
};

typedef boost::ptr_vector<WW8SelBoxInfo> WW8MergeGroups;

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

class WW8TabDesc
{
    std::vector<String> aNumRuleNames;
    sw::util::RedlineStack *mpOldRedlineStack;

    SwWW8ImplReader* pIo;

    WW8TabBandDesc* pFirstBand;
    WW8TabBandDesc* pActBand;

    SwPosition* pTmpPos;

    SwTableNode* pTblNd;            // table node
    const SwTableLines* pTabLines;  // row array of node
    SwTableLine* pTabLine;          // current row
    SwTableBoxes* pTabBoxes;        // boxes array in current row
    SwTableBox* pTabBox;            // current cell

    WW8MergeGroups aMergeGroups;   // list of all cells to be merged

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
    bool bClaimLineFmt;
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
    //No copying
    WW8TabDesc(const WW8TabDesc&);
    WW8TabDesc &operator=(const WW8TabDesc&);
public:
    const SwTable* pTable;          // table
    SwPosition* pParentPos;
    SwFlyFrmFmt* pFlyFmt;
    SfxItemSet aItemSet;
    bool IsValidCell(short nCol) const;
    bool InFirstParaInCell() const;

    WW8TabDesc( SwWW8ImplReader* pIoClass, WW8_CP nStartCp );
    bool Ok() const { return bOk; }
    void CreateSwTable();
    void UseSwTable();
    void SetSizePosition(SwFrmFmt* pFrmFmt);
    void TableCellEnd();
    void MoveOutsideTable();
    void ParkPaM();
    void FinishSwTable();
    void MergeCells();
    short GetMinLeft() const { return nConvertedLeft; }
    ~WW8TabDesc();
    SwPosition *GetPos() { return pTmpPos; }

    const WW8_TCell* GetAktWWCell() const { return pAktWWCell; }
    short GetAktCol() const { return nAktCol; }
    // find name of numrule valid for current WW-COL
    const String& GetNumRuleName() const;
    void SetNumRuleName( const String& rName );

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

sal_uInt16 SwWW8ImplReader::End_Ftn()
{
    /*
    Ignoring Footnote outside of the normal Text. People will put footnotes
    into field results and field commands.
    */
    if (bIgnoreText ||
        pPaM->GetPoint()->nNode < rDoc.GetNodes().GetEndOfExtras().GetIndex())
    {
        return 0;
    }

    OSL_ENSURE(!maFtnStack.empty(), "footnote end without start");
    if (maFtnStack.empty())
        return 0;

    bool bFtEdOk = false;
    const FtnDescriptor &rDesc = maFtnStack.back();

    //Get the footnote character and remove it from the txtnode. We'll
    //replace it with the footnote
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    xub_StrLen nPos = pPaM->GetPoint()->nContent.GetIndex();

    String sChar;
    SwTxtAttr* pFN = 0;
    //There should have been a footnote char, we will replace this.
    if (pTxt && nPos)
    {
        sChar.Append(pTxt->GetTxt()[--nPos]);
        pPaM->SetMark();
        pPaM->GetMark()->nContent--;
        rDoc.DeleteRange( *pPaM );
        pPaM->DeleteMark();
        SwFmtFtn aFtn(rDesc.meType == MAN_EDN);
        pFN = pTxt->InsertItem(aFtn, nPos, nPos);
    }
    OSL_ENSURE(pFN, "Probleme beim Anlegen des Fussnoten-Textes");
    if (pFN)
    {

        SwPosition aTmpPos( *pPaM->GetPoint() );    // remember old cursor position
        WW8PLCFxSaveAll aSave;
        pPlcxMan->SaveAllPLCFx( aSave );
        WW8PLCFMan* pOldPlcxMan = pPlcxMan;

        const SwNodeIndex* pSttIdx = ((SwTxtFtn*)pFN)->GetStartNode();
        OSL_ENSURE(pSttIdx, "Probleme beim Anlegen des Fussnoten-Textes");

        ((SwTxtFtn*)pFN)->SetSeqNo( rDoc.GetFtnIdxs().size() );

        bool bOld = bFtnEdn;
        bFtnEdn = true;

        // read content of Ft-/End-Note
        Read_HdFtFtnText( pSttIdx, rDesc.mnStartCp, rDesc.mnLen, rDesc.meType);
        bFtEdOk = true;
        bFtnEdn = bOld;

        OSL_ENSURE(sChar.Len()==1 && ((rDesc.mbAutoNum == (sChar.GetChar(0) == 2))),
         "footnote autonumbering must be 0x02, and everthing else must not be");

        // If no automatic numbering use the following char from the main text
        // as the footnote number
        if (!rDesc.mbAutoNum)
            ((SwTxtFtn*)pFN)->SetNumber(0, sChar);

        /*
            Delete the footnote char from the footnote if its at the beginning
            as usual. Might not be if the user has already deleted it, e.g.
            #i14737#
        */
        SwNodeIndex& rNIdx = pPaM->GetPoint()->nNode;
        rNIdx = pSttIdx->GetIndex() + 1;
        SwTxtNode* pTNd = rNIdx.GetNode().GetTxtNode();
        if (pTNd && !pTNd->GetTxt().isEmpty() && sChar.Len())
        {
            if (pTNd->GetTxt()[0] == sChar.GetChar(0))
            {
                pPaM->GetPoint()->nContent.Assign( pTNd, 0 );
                pPaM->SetMark();
                // Strip out tabs we may have inserted on export #i24762#
                if (pTNd->GetTxt()[1] == 0x09)
                    pPaM->GetMark()->nContent++;
                pPaM->GetMark()->nContent++;
                pReffingStck->Delete(*pPaM);
                rDoc.DeleteRange( *pPaM );
                pPaM->DeleteMark();
            }
        }

        *pPaM->GetPoint() = aTmpPos;        // restore Cursor

        pPlcxMan = pOldPlcxMan;             // Restore attributes
        pPlcxMan->RestoreAllPLCFx( aSave );
    }

    if (bFtEdOk)
        maSectionManager.SetCurrentSectionHasFootnote();

    maFtnStack.pop_back();
    return 0;
}

long SwWW8ImplReader::Read_Ftn(WW8PLCFManResult* pRes)
{
    /*
    Ignoring Footnote outside of the normal Text. People will put footnotes
    into field results and field commands.
    */
    if (bIgnoreText ||
        pPaM->GetPoint()->nNode < rDoc.GetNodes().GetEndOfExtras().GetIndex())
    {
        return 0;
    }

    FtnDescriptor aDesc;
    aDesc.mbAutoNum = true;
    if (eEDN == pRes->nSprmId)
    {
        aDesc.meType = MAN_EDN;
        if (pPlcxMan->GetEdn())
            aDesc.mbAutoNum = 0 != *(short*)pPlcxMan->GetEdn()->GetData();
    }
    else
    {
        aDesc.meType = MAN_FTN;
        if (pPlcxMan->GetFtn())
            aDesc.mbAutoNum = 0 != *(short*)pPlcxMan->GetFtn()->GetData();
    }

    aDesc.mnStartCp = pRes->nCp2OrIdx;
    aDesc.mnLen = pRes->nMemLen;

    maFtnStack.push_back(aDesc);

    return 0;
}

bool SwWW8ImplReader::SearchRowEnd(WW8PLCFx_Cp_FKP* pPap, WW8_CP &rStartCp,
    int nLevel) const
{
    WW8PLCFxDesc aRes;
    aRes.pMemPos = 0;
    aRes.nEndPos = rStartCp;

    while (pPap->HasFkp() && rStartCp != WW8_CP_MAX)
    {
        if (pPap->Where() != WW8_CP_MAX)
        {
            const sal_uInt8* pB = pPap->HasSprm(TabRowSprm(nLevel));
            if (pB && *pB == 1)
            {
                const sal_uInt8 *pLevel = 0;
                if (0 != (pLevel = pPap->HasSprm(0x6649)))
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
        aRes.pMemPos = 0;
        //Seek to our next block of properties
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);
        //Update our aRes to get the new starting point of the next properties
        rStartCp = aRes.nEndPos;
    }

    return false;
}

ApoTestResults SwWW8ImplReader::TestApo(int nCellLevel, bool bTableRowEnd,
    const WW8_TablePos *pTabPos)
{
    const WW8_TablePos *pTopLevelTable = nCellLevel <= 1 ? pTabPos : 0;
    ApoTestResults aRet;
    // Frame in Style Definition (word appears to ignore them if inside an
    // text autoshape)
    if (!bTxbxFlySection && nAktColl < vColl.size())
        aRet.mpStyleApo = StyleExists(nAktColl) ? vColl[nAktColl].pWWFly : 0;

    /*
    #i1140#
    If I have a table and apply a style to one of its frames that should cause
    a paragraph that its applied to it to only exist as a separate floating
    frame, then the behavour depends on which cell that it has been applied
    to. If its the first cell of a row then the whole table row jumps into the
    new frame, if its not then then the paragraph attributes are applied
    "except" for the floating frame stuff. i.e. its ignored. So if theres a
    table, and we're not in the first cell then we ignore the fact that the
    paragraph style wants to be in a different frame.

    This sort of mindbending inconsistency is surely why frames are deprecated
    in word 97 onwards and hidden away from the user


    #i1532# & #i5379#
    If we are already a table in a frame then we must grab the para properties
    to see if we are still in that frame.
    */

    aRet.m_bHasSprm37 = pPlcxMan->HasParaSprm( bVer67 ? 37 : 0x2423 );
    const sal_uInt8 *pSrpm29 = pPlcxMan->HasParaSprm( bVer67 ? 29 : 0x261B );
    aRet.m_bHasSprm29 = pSrpm29 != NULL;
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

    bool bTestAllowed = !bTxbxFlySection && !bTableRowEnd;
    if (bTestAllowed)
    {
        //Test is allowed if there is no table.
        //Otherwise only allowed if we are in the
        //first paragraph of the first cell of a row.
        //(And only if the row we are inside is at the
        //same level as the previous row, think tables
        //in tables)
        if (nCellLevel == nInTable)
        {

            if (!nInTable)
                bTestAllowed = true;
            else
            {
                if (!pTableDesc)
                {
                    OSL_ENSURE(pTableDesc, "What!");
                    bTestAllowed = false;
                }
                else
                {
                    // #i39468#
                    // If current cell isn't valid, the test is allowed.
                    // The cell isn't valid, if e.g. there is a new row
                    // <pTableDesc->nAktRow> >= <pTableDesc->pTabLines->Count()>
                    bTestAllowed =
                        pTableDesc->GetAktCol() == 0 &&
                        ( !pTableDesc->IsValidCell( pTableDesc->GetAktCol() ) ||
                          pTableDesc->InFirstParaInCell() );
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
//---------------------------------------------------------------------
// helper methods for outline, numbering and bullets
//---------------------------------------------------------------------

static void SetBaseAnlv(SwNumFmt &rNum, WW8_ANLV &rAV, sal_uInt8 nSwLevel )
{
    static const SvxExtNumType eNumA[8] = { SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC };

    static const SvxAdjust eAdjA[4] = { SVX_ADJUST_LEFT,
        SVX_ADJUST_RIGHT, SVX_ADJUST_LEFT, SVX_ADJUST_LEFT };
//      in fact the following 2, but writer UI does not provide
//      SVX_ADJUST_CENTER, SVX_ADJUST_BLOCKLINE };

    rNum.SetNumberingType( static_cast< sal_Int16 >(( SVBT8ToByte( rAV.nfc ) < 8 ) ?
                    eNumA[SVBT8ToByte( rAV.nfc ) ] : SVX_NUM_NUMBER_NONE) );
    if ((SVBT8ToByte(rAV.aBits1 ) & 0x4) >> 2)
        rNum.SetIncludeUpperLevels(nSwLevel + 1);
    rNum.SetStart( SVBT16ToShort( rAV.iStartAt ) );
    rNum.SetNumAdjust( eAdjA[SVBT8ToByte( rAV.aBits1 ) & 0x3] );

    rNum.SetCharTextDistance( SVBT16ToShort( rAV.dxaSpace ) );
    sal_Int16 nIndent = std::abs((sal_Int16)SVBT16ToShort( rAV.dxaIndent ));
    if( SVBT8ToByte( rAV.aBits1 ) & 0x08 )      //fHang
    {
        rNum.SetFirstLineOffset( -nIndent );
        rNum.SetLSpace( nIndent );
        rNum.SetAbsLSpace( nIndent );
    }
    else
        rNum.SetCharTextDistance( nIndent );        // width of number is missing

    if( SVBT8ToByte( rAV.nfc ) == 5 || SVBT8ToByte( rAV.nfc ) == 7 )
    {
        String sP( rNum.GetSuffix() );
        sP.Insert( '.', 0 );
        rNum.SetSuffix( sP );   // ordinal number
    }
}

void SwWW8ImplReader::SetAnlvStrings(SwNumFmt &rNum, WW8_ANLV &rAV,
    const sal_uInt8* pTxt, bool bOutline)
{
    bool bInsert = false;                       // Default
    CharSet eCharSet = eStructCharSet;

    const WW8_FFN* pF = pFonts->GetFont(SVBT16ToShort(rAV.ftc)); // FontInfo
    bool bListSymbol = pF && ( pF->chs == 2 );      // Symbol/WingDings/...

    String sTxt;
    if (bVer67)
    {
        sTxt = String( (sal_Char*)pTxt,  SVBT8ToByte( rAV.cbTextBefore )
                                 + SVBT8ToByte( rAV.cbTextAfter  ), eCharSet );
    }
    else
    {
        for(xub_StrLen i = SVBT8ToByte(rAV.cbTextBefore);
            i < SVBT8ToByte(rAV.cbTextAfter); ++i, pTxt += 2)
        {
            sTxt.Append(SVBT16ToShort(*(SVBT16*)pTxt));
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
                comphelper::string::padToLength(aBuf, SVBT8ToByte(rAV.cbTextBefore)
                    + SVBT8ToByte(rAV.cbTextAfter), cBulletChar);
                sTxt = aBuf.makeStringAndClear();
            }
        }
    }
    else
    {                                       // numbering / bullets
        bInsert = true;
        if( bListSymbol )
        {
            FontFamily eFamily;
            String aName;
            FontPitch ePitch;

            if( GetFontParams( SVBT16ToShort( rAV.ftc ), eFamily, aName,
                                ePitch, eCharSet ) ){

                Font aFont;
                aFont.SetName( aName );
                aFont.SetFamily( eFamily );

                aFont.SetCharSet( eCharSet );
                rNum.SetNumberingType(SVX_NUM_CHAR_SPECIAL);

                rNum.SetBulletFont( &aFont );

                // take only the very first character
                if( rAV.cbTextBefore || rAV.cbTextAfter)
                    rNum.SetBulletChar( sTxt.GetChar( 0 ) );
                else
                    rNum.SetBulletChar( 0x2190 );
            }
        }
    }
    if( bInsert )
    {
        if( rAV.cbTextBefore )
        {
            String sP( sTxt.Copy( 0, SVBT8ToByte( rAV.cbTextBefore ) ) );
            rNum.SetPrefix( sP );
        }
        if( SVBT8ToByte( rAV.cbTextAfter ) )
        {
            String sP( rNum.GetSuffix() );
            sP.Insert( sTxt.Copy( SVBT8ToByte( rAV.cbTextBefore ),
                                  SVBT8ToByte( rAV.cbTextAfter  ) ) );
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
void SwWW8ImplReader::SetAnld(SwNumRule* pNumR, WW8_ANLD* pAD, sal_uInt8 nSwLevel,
    bool bOutLine)
{
    SwNumFmt aNF;
    if (pAD)
    {                                                       // there is a Anld-Sprm
        bAktAND_fNumberAcross = 0 != SVBT8ToByte( pAD->fNumberAcross );
        WW8_ANLV &rAV = pAD->eAnlv;
        SetBaseAnlv(aNF, rAV, nSwLevel);                    // set the base format
        SetAnlvStrings(aNF, rAV, pAD->rgchAnld, bOutLine ); // set the rest
    }
    pNumR->Set(nSwLevel, aNF);
}

//-------------------------------------------------------
// chapter numbering and bullets
//-------------------------------------------------------
// Chapter numbering happens in the style definition.
// Sprm 13 provides the level, Sprm 12 the content.

SwNumRule* SwWW8ImplReader::GetStyRule()
{
    if( pStyles->pStyRule )         // Bullet-Style already present
        return pStyles->pStyRule;

    const OUString aBaseName("WW8StyleNum");
    const OUString aName( rDoc.GetUniqueNumRuleName( &aBaseName, false) );

    // #i86652#
    sal_uInt16 nRul = rDoc.MakeNumRule( aName, 0, false,
                                    SvxNumberFormat::LABEL_ALIGNMENT );
    pStyles->pStyRule = rDoc.GetNumRuleTbl()[nRul];
    // Auto == false-> Nummerierungsvorlage
    pStyles->pStyRule->SetAutoRule(false);

    return pStyles->pStyRule;
}

// Sprm 13
void SwWW8ImplReader::Read_ANLevelNo( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    nSwNumLevel = 0xff; // Default: invalid

    if( nLen <= 0 )
        return;

    // StyleDef ?
    if( pAktColl )
    {
        // only for SwTxtFmtColl, not CharFmt
        // WW: 0 = no Numbering
        SwWW8StyInf * pColl = GetStyle(nAktColl);
        if (pColl != NULL && pColl->bColl && *pData)
        {
            // Range WW:1..9 -> SW:0..8 no bullets / numbering

            if (*pData <= MAXLEVEL && *pData <= 9)
            {
                nSwNumLevel = *pData - 1;
                if (!bNoAttrImport)
                    ((SwTxtFmtColl*)pAktColl)->AssignToListLevelOfOutlineStyle( nSwNumLevel ); //<-end,zhaojianwei
                    // For WW-NoNumbering also NO_NUMBERING could be used.
                    // ( For normal numberierung NO_NUM has to be used:
                    //   NO_NUM : pauses numbering,
                    //   NO_NUMBERING : no numbering at all )

            }
            else if( *pData == 10 || *pData == 11 )
            {
                // remember type, the rest happens at Sprm 12
                pStyles->nWwNumLevel = *pData;
            }
        }
    }
    else
    {
        //Not StyleDef
        if (!bAnl)
            StartAnl(pData);        // begin of outline / bullets
        NextAnlLine(pData);
    }
}

void SwWW8ImplReader::Read_ANLevelDesc( sal_uInt16, const sal_uInt8* pData, short nLen ) // Sprm 12
{
    SwWW8StyInf * pStyInf = GetStyle(nAktColl);
    if( !pAktColl || nLen <= 0                       // only for Styledef
        || (pStyInf && !pStyInf->bColl)              // ignore  CharFmt ->
        || ( nIniFlags & WW8FL_NO_OUTLINE ) )
    {
        nSwNumLevel = 0xff;
        return;
    }


    if( nSwNumLevel <= MAXLEVEL         // Value range mapping WW:1..9 -> SW:0..8
        && nSwNumLevel <= 9 ){          // No Bullets or Numbering

        // If NumRuleItems were set, either directly or through inheritance, disable them now
        pAktColl->SetFmtAttr( SwNumRuleItem() );

        const OUString aName("Outline");
        SwNumRule aNR( rDoc.GetUniqueNumRuleName( &aName ),
                       SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
                       OUTLINE_RULE );
        aNR = *rDoc.GetOutlineNumRule();

        SetAnld(&aNR, (WW8_ANLD*)pData, nSwNumLevel, true);

        // Missing Levels need not be replenished
        rDoc.SetOutlineNumRule( aNR );
    }else if( pStyles->nWwNumLevel == 10 || pStyles->nWwNumLevel == 11 ){
        SwNumRule* pNR = GetStyRule();
        SetAnld(pNR, (WW8_ANLD*)pData, 0, false);
        pAktColl->SetFmtAttr( SwNumRuleItem( pNR->GetName() ) );

        pStyInf = GetStyle(nAktColl);
        if (pStyInf != NULL)
            pStyInf->bHasStyNumRule = true;
    }
}

//-----------------------------------------
// Numbering / Bullets
//-----------------------------------------

// SetNumOlst() carries the Numrules for this cell to SwNumFmt.
// For this the info is fetched from OLST and not from ANLD ( see later )
// ( only for outline inside text; Bullets / numbering use ANLDs )
void SwWW8ImplReader::SetNumOlst(SwNumRule* pNumR, WW8_OLST* pO, sal_uInt8 nSwLevel)
{
    SwNumFmt aNF;
    WW8_ANLV &rAV = pO->rganlv[nSwLevel];
    SetBaseAnlv(aNF, rAV, nSwLevel);
                                            // ... and then the Strings
    int nTxtOfs = 0;
    sal_uInt8 i;
    WW8_ANLV* pAV1;                 // search String-Positions
    for (i = 0, pAV1 = pO->rganlv; i < nSwLevel; ++i, ++pAV1)
    {
        nTxtOfs += SVBT8ToByte(pAV1->cbTextBefore)
            + SVBT8ToByte(pAV1->cbTextAfter);
    }

    if (!bVer67)
        nTxtOfs *= 2;
    SetAnlvStrings(aNF, rAV, pO->rgch + nTxtOfs, true); // and apply
    pNumR->Set(nSwLevel, aNF);
}

// The OLST is at the beginning of each section that contains outlines.
// The ANLDs that are connected to each outline-line contain only nonsense,
// so the OLSTs are remembered for the section to have usable information
// when outline-paragraphs occur.
void SwWW8ImplReader::Read_OLST( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    delete pNumOlst;
    if (nLen <= 0)
    {
        pNumOlst = 0;
        return;
    }
    pNumOlst = new WW8_OLST;
    if( nLen < sal::static_int_cast< sal_Int32 >(sizeof( WW8_OLST )) )   // fill if to short
        memset( pNumOlst, 0, sizeof( *pNumOlst ) );
    *pNumOlst = *(WW8_OLST*)pData;
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
    bAktAND_fNumberAcross = false;

    sal_uInt8 nT = static_cast< sal_uInt8 >(GetNumType(*pSprm13));
    if (nT == WW8_Pause || nT == WW8_None)
        return;

    nWwNumType = nT;
    SwNumRule *pNumRule = maANLDRules.GetNumRule(nWwNumType);

    // check for COL numbering:
    const sal_uInt8* pS12 = 0;// sprmAnld
    String sNumRule;

    if (pTableDesc)
    {
        sNumRule = pTableDesc->GetNumRuleName();
        if (sNumRule.Len())
        {
            pNumRule = rDoc.FindNumRulePtr(sNumRule);
            if (!pNumRule)
                sNumRule.Erase();
            else
            {
                // this is ROW numbering ?
                pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E); // sprmAnld
                if (pS12 && 0 != SVBT8ToByte(((WW8_ANLD*)pS12)->fNumberAcross))
                    sNumRule.Erase();
            }
        }
    }

    SwWW8StyInf * pStyInf = GetStyle(nAktColl);
    if (!sNumRule.Len() && pStyInf != NULL &&  pStyInf->bHasStyNumRule)
    {
        sNumRule = pStyInf->pFmt->GetNumRule().GetValue();
        pNumRule = rDoc.FindNumRulePtr(sNumRule);
        if (!pNumRule)
            sNumRule.Erase();
    }

    if (!sNumRule.Len())
    {
        if (!pNumRule)
        {
            // #i86652#
            pNumRule = rDoc.GetNumRuleTbl()[
                            rDoc.MakeNumRule( sNumRule, 0, false,
                                              SvxNumberFormat::LABEL_ALIGNMENT ) ];
        }
        if (pTableDesc)
        {
            if (!pS12)
                pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E); // sprmAnld
            if (!pS12 || !SVBT8ToByte( ((WW8_ANLD*)pS12)->fNumberAcross))
                pTableDesc->SetNumRuleName(pNumRule->GetName());
        }
    }

    bAnl = true;

    // set NumRules via stack
    pCtrlStck->NewAttr(*pPaM->GetPoint(),
        SfxStringItem(RES_FLTR_NUMRULE, pNumRule->GetName()));

    maANLDRules.SetNumRule(pNumRule, nWwNumType);
}

// NextAnlLine() is called once for every row of a
// outline / numbering / bullet
void SwWW8ImplReader::NextAnlLine(const sal_uInt8* pSprm13)
{
    if (!bAnl)
        return;

    SwNumRule *pNumRule = maANLDRules.GetNumRule(nWwNumType);

    // pNd->UpdateNum ohne Regelwerk gibt GPF spaetestens beim Speichern als
    // sdw3

    // WW:10 = numberierung -> SW:0 & WW:11 = bullets -> SW:0
    if (*pSprm13 == 10 || *pSprm13 == 11)
    {
        nSwNumLevel = 0;
        if (!pNumRule->GetNumFmt(nSwNumLevel))
        {
            // not defined yet
            // sprmAnld o. 0
            const sal_uInt8* pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E);
            SetAnld(pNumRule, (WW8_ANLD*)pS12, nSwNumLevel, false);
        }
    }
    else if( *pSprm13 > 0 && *pSprm13 <= MAXLEVEL )          // range WW:1..9 -> SW:0..8
    {
        nSwNumLevel = *pSprm13 - 1;             // outline
        // undefined
        if (!pNumRule->GetNumFmt(nSwNumLevel))
        {
            if (pNumOlst)                       // there was a OLST
            {
                //Assure upper levels are set, #i9556#
                for (sal_uInt8 nI = 0; nI < nSwNumLevel; ++nI)
                {
                    if (!pNumRule->GetNumFmt(nI))
                        SetNumOlst(pNumRule, pNumOlst, nI);
                }

                SetNumOlst(pNumRule, pNumOlst , nSwNumLevel);
            }
            else                                // no Olst -> use Anld
            {
                // sprmAnld
                const sal_uInt8* pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E);
                SetAnld(pNumRule, (WW8_ANLD*)pS12, nSwNumLevel, false);
            }
        }
    }
    else
        nSwNumLevel = 0xff;                 // no number

    SwTxtNode* pNd = pPaM->GetNode()->GetTxtNode();
    if (nSwNumLevel < MAXLEVEL)
        pNd->SetAttrListLevel( nSwNumLevel );
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
        SwPosition aTmpPos(*pPaM->GetPoint());
        pPaM->Move(fnMoveBackward, fnGoCntnt);
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_FLTR_NUMRULE);
        *pPaM->GetPoint() = aTmpPos;
    }
    else
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_FLTR_NUMRULE);

    maANLDRules.mpNumberingNumRule = 0;
    /*
     #i18816#
     my take on this problem is that moving either way from an outline to a
     numbering doesn't halt the outline, while the numbering is always halted
    */
    bool bNumberingNotStopOutline =
        (((nWwNumType == WW8_Outline) && (nNewType == WW8_Numbering)) ||
        ((nWwNumType == WW8_Numbering) && (nNewType == WW8_Outline)));
    if (!bNumberingNotStopOutline)
        maANLDRules.mpOutlineNumRule = 0;

    nSwNumLevel = 0xff;
    nWwNumType = WW8_None;
    bAnl = false;
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
        delete[] pTCs, pTCs = 0;
        delete[] pSHDs, pSHDs = 0;
        delete[] pNewSHDs, pNewSHDs = 0;
    }

    short nFileCols = nLen / ( bVer67 ? 10 : 20 );  // realy saved

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
            WW8_TCellVer6* pTc = (WW8_TCellVer6*)pT;
            for(i=0; i<nColsToRead; i++, ++pAktTC,++pTc)
            {
                if( i < nColsToRead )
                {               // TC from file ?
                    sal_uInt8 aBits1 = SVBT8ToByte( pTc->aBits1Ver6 );
                    pAktTC->bFirstMerged    = ( ( aBits1 & 0x01 ) != 0 );
                    pAktTC->bMerged     = ( ( aBits1 & 0x02 ) != 0 );
                    memcpy( pAktTC->rgbrc[ WW8_TOP      ].aBits1,
                                    pTc->rgbrcVer6[ WW8_TOP     ].aBits1, sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_LEFT     ].aBits1,
                                    pTc->rgbrcVer6[ WW8_LEFT    ].aBits1, sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_BOT      ].aBits1,
                                    pTc->rgbrcVer6[ WW8_BOT     ].aBits1, sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT    ].aBits1,
                                    pTc->rgbrcVer6[ WW8_RIGHT   ].aBits1, sizeof( SVBT16 ) );
                    if(    ( pAktTC->bMerged )
                            && ( i > 0             ) )
                    {
                        // Cell merged -> remember
                        //bWWMergedVer6[i] = true;
                        memcpy( pTCs[i-1].rgbrc[ WW8_RIGHT ].aBits1,
                                pTc->rgbrcVer6[  WW8_RIGHT ].aBits1, sizeof( SVBT16 ) );
                            // apply right border to previous cell
                            // bExist must not be set to false, because WW
                            // does not count this cells in text boxes....
                    }
                }
            }
        }
        else
        {
            WW8_TCellVer8* pTc = (WW8_TCellVer8*)pT;
            for (int k = 0; k < nColsToRead; ++k, ++pAktTC, ++pTc )
            {
                sal_uInt16 aBits1 = SVBT16ToShort( pTc->aBits1Ver8 );
                pAktTC->bFirstMerged    = ( ( aBits1 & 0x0001 ) != 0 );
                pAktTC->bMerged         = ( ( aBits1 & 0x0002 ) != 0 );
                pAktTC->bVertical       = ( ( aBits1 & 0x0004 ) != 0 );
                pAktTC->bBackward       = ( ( aBits1 & 0x0008 ) != 0 );
                pAktTC->bRotateFont     = ( ( aBits1 & 0x0010 ) != 0 );
                pAktTC->bVertMerge      = ( ( aBits1 & 0x0020 ) != 0 );
                pAktTC->bVertRestart    = ( ( aBits1 & 0x0040 ) != 0 );
                pAktTC->nVertAlign      = ( ( aBits1 & 0x0180 ) >> 7 );
                // note: in aBits1 there are 7 bits unused,
                //       followed by another 16 unused bits

                // In Version 8 koennen we can copy all border codes at once!
                memcpy( pAktTC->rgbrc, pTc->rgbrcVer8, 4 * sizeof( WW8_BRC ) );
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

void WW8TabBandDesc::ProcessSprmTSetBRC(bool bVer67, const sal_uInt8* pParamsTSetBRC)
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

        bool bChangeRight  = (nFlag & 0x08) ? true : false;
        bool bChangeBottom = (nFlag & 0x04) ? true : false;
        bool bChangeLeft   = (nFlag & 0x02) ? true : false;
        bool bChangeTop    = (nFlag & 0x01) ? true : false;

        WW8_TCell* pAktTC  = pTCs + nitcFirst;
        if( bVer67 )
        {
            WW8_BRCVer6* pBRC = (WW8_BRCVer6*)(pParamsTSetBRC+3);

            for( int i = nitcFirst; i < nitcLim; ++i, ++pAktTC )
            {
                if( bChangeTop )
                {
                    memcpy( pAktTC->rgbrc[ WW8_TOP  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                }
                if( bChangeLeft )
                {
                    memcpy( pAktTC->rgbrc[ WW8_LEFT ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                }
                if( bChangeBottom )
                {
                    memcpy( pAktTC->rgbrc[ WW8_BOT  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                }
                if( bChangeRight )
                {
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                }
            }
        }
        else
        {
            WW8_BRC* pBRC = (WW8_BRC*)(pParamsTSetBRC+3);

            for( int i = nitcFirst; i < nitcLim; ++i, ++pAktTC )
            {
                if( bChangeTop )
                {
                    memcpy( pAktTC->rgbrc[ WW8_TOP  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_TOP  ].aBits2,
                            pBRC->aBits2,
                            sizeof( SVBT16 ) );
                }
                if( bChangeLeft )
                {
                    memcpy( pAktTC->rgbrc[ WW8_LEFT ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_LEFT ].aBits2,
                            pBRC->aBits2,
                            sizeof( SVBT16 ) );
                }
                if( bChangeBottom )
                {
                    memcpy( pAktTC->rgbrc[ WW8_BOT  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_BOT  ].aBits2,
                            pBRC->aBits2,
                            sizeof( SVBT16 ) );
                }
                if( bChangeRight )
                {
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT].aBits2,
                            pBRC->aBits2,
                            sizeof( SVBT16 ) );
                }
            }
        }
    }
}

void WW8TabBandDesc::ProcessSprmTTableBorders(bool bVer67, const sal_uInt8* pParams)
{
    // sprmTTableBorders
    if( bVer67 )
    {
        for( int i = 0; i < 6; ++i )
        {
            aDefBrcs[i].aBits1[0] = pParams[   2*i ];
            aDefBrcs[i].aBits1[1] = pParams[ 1+2*i ];
        }
    }
    else // aDefBrcs = *(BRC(*)[6])pS;
        memcpy( aDefBrcs, pParams, 24 );
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
        short nOrgWidth;
        short nDelta;

        for( int i = nitcFirst; (i < nitcLim) && (i < nWwCols); i++ )
        {
            nOrgWidth  = nCenter[i+1] - nCenter[i];
            nDelta     = nDxaCol - nOrgWidth;
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
                OSL_ENSURE(!this, "Impossible");
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

    SVBT16* pShd;
    int i;
    for(i=0, pShd = (SVBT16*)pS; i<nAnz; i++, pShd++ )
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
        if (0 == (pParams = pPap->HasSprm(0x244B)))
            pParams = pPap->HasSprm(0x2416);
    }
    return pParams;
}

enum wwTableSprm
{
    sprmNil,

    sprmTTableWidth,sprmTTextFlow, sprmTFCantSplit, sprmTFCantSplit90,sprmTJc, sprmTFBiDi, sprmTDefTable,
    sprmTDyaRowHeight, sprmTDefTableShd, sprmTDxaLeft, sprmTSetBrc,
    sprmTDxaCol, sprmTInsert, sprmTDelete, sprmTTableHeader,
    sprmTDxaGapHalf, sprmTTableBorders,

    sprmTDefTableNewShd, sprmTSpacing, sprmTNewSpacing
};

wwTableSprm GetTableSprm(sal_uInt16 nId, ww::WordVersion eVer)
{
    switch (eVer)
    {
        case ww::eWW8:
            switch (nId)
            {
                case 0xF614:
                    return sprmTTableWidth;
                case 0x7629:
                    return sprmTTextFlow;
                case 0x3403:
                    return sprmTFCantSplit;
                case 0x3404:
                    return sprmTTableHeader;
                case 0x3466:
                    return sprmTFCantSplit90;
                case 0x5400:
                    return sprmTJc;
                case 0x560B:
                    return sprmTFBiDi;
                case 0x5622:
                    return sprmTDelete;
                case 0x7621:
                    return sprmTInsert;
                case 0x7623:
                    return sprmTDxaCol;
                case 0x9407:
                    return sprmTDyaRowHeight;
                case 0x9601:
                    return sprmTDxaLeft;
                case 0x9602:
                    return sprmTDxaGapHalf;
                case 0xD605:
                    return sprmTTableBorders;
                case 0xD608:
                    return sprmTDefTable;
                case 0xD609:
                    return sprmTDefTableShd;
                case 0xD612:
                    return sprmTDefTableNewShd;
                case 0xD620:
                    return sprmTSetBrc;
                case 0xD632:
                    return sprmTSpacing;
                case 0xD634:
                    return sprmTNewSpacing;
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
    mpOldRedlineStack(0),
    pIo(pIoClass),
    pFirstBand(0),
    pActBand(0),
    pTmpPos(0),
    pTblNd(0),
    pTabLines(0),
    pTabLine(0),
    pTabBoxes(0),
    pTabBox(0),
    pAktWWCell(0),
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
    bClaimLineFmt(false),
    eOri(text::HoriOrientation::NONE),
    bIsBiDi(false),
    nAktRow(0),
    nAktBandRow(0),
    nAktCol(0),
    nRowsToRepeat(0),
    pTable(0),
    pParentPos(0),
    pFlyFmt(0),
    aItemSet(pIo->rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    pIo->bAktAND_fNumberAcross = false;

    static const sal_Int16 aOriArr[] =
    {
        text::HoriOrientation::LEFT, text::HoriOrientation::CENTER, text::HoriOrientation::RIGHT, text::HoriOrientation::CENTER
    };

    bool bOldVer = ww::IsSevenMinus(pIo->GetFib().GetFIBVersion());
    WW8_TablePos aTabPos;

    WW8PLCFxSave1 aSave;
    pIo->pPlcxMan->GetPap()->Save( aSave );

    WW8PLCFx_Cp_FKP* pPap = pIo->pPlcxMan->GetPapPLCF();

    eOri = text::HoriOrientation::LEFT;

    WW8TabBandDesc* pNewBand = new WW8TabBandDesc;

    wwSprmParser aSprmParser(pIo->GetFib().GetFIBVersion());

    // process pPap until end of table found
    do
    {
        short nTabeDxaNew      = SHRT_MAX;
        bool bTabRowJustRead   = false;
        const sal_uInt8* pShadeSprm = 0;
        const sal_uInt8* pNewShadeSprm = 0;
        WW8_TablePos *pTabPos  = 0;

        // search end of a tab row
        if(!(pIo->SearchRowEnd(pPap, nStartCp, pIo->nInTable)))
        {
            bOk = false;
            break;
        }

        // Get the SPRM chains:
        // first from PAP and then from PCD (of the Piece Table)
        WW8PLCFxDesc aDesc;
        pPap->GetSprms( &aDesc );
        WW8SprmIter aSprmIter(aDesc.pMemPos, aDesc.nSprmsLen, aSprmParser);

        const sal_uInt8* pParams = aSprmIter.GetAktParams();
        for (int nLoop = 0; nLoop < 2; ++nLoop)
        {
            bool bRepeatedSprm = false;
            while (aSprmIter.GetSprms() && 0 != (pParams = aSprmIter.GetAktParams()))
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
                        bClaimLineFmt = true;
                        break;
                    case sprmTFCantSplit90:
                        pNewBand->bCantSplit90 = *pParams;
                        bClaimLineFmt = true;
                        break;
                    case sprmTTableBorders:
                        pNewBand->ProcessSprmTTableBorders(bOldVer, pParams);
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
                        bIsBiDi = SVBT16ToShort(pParams) ? true : false;
                        break;
                    case sprmTDxaGapHalf:
                        pNewBand->nGapHalf = (sal_Int16)SVBT16ToShort( pParams );
                        break;
                    case sprmTDyaRowHeight:
                        pNewBand->nLineHeight = (sal_Int16)SVBT16ToShort( pParams );
                        bClaimLineFmt = true;
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
                        pNewBand->ProcessSprmTSetBRC(bOldVer, pParams);
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
                    case sprmTNewSpacing:
                        pNewBand->ProcessSpacing(pParams);
                        break;
                    case sprmTSpacing:
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
            if (pShadeSprm)
                pNewBand->ReadShd(pShadeSprm);
            if (pNewShadeSprm)
                pNewBand->ReadNewShd(pNewShadeSprm, bOldVer);
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
        aRes.pMemPos = 0;
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
        pParams = HasTabCellSprm(pPap, bOldVer);
        const sal_uInt8 *pLevel = pPap->HasSprm(0x6649);
        // InTable
        if (!pParams || (1 != *pParams) ||
            (pLevel && (*pLevel <= pIo->nInTable)))
        {
            break;
        }

        //Get the end of row new table positioning data
        WW8_CP nMyStartCp=nStartCp;
        if (pIo->SearchRowEnd(pPap, nMyStartCp, pIo->nInTable))
            if (pIo->ParseTabPos(&aTabPos, pPap))
                pTabPos = &aTabPos;

        //Move back to this cell
        aRes.pMemPos = 0;
        aRes.nStartPos = nStartCp;

        // PlcxMan currently points too far ahead so we need to bring
        // it back to where we are trying to make a table
        pIo->pPlcxMan->GetPap()->nOrigStartPos = aRes.nStartPos;
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);

        //Does this row match up with the last row closely enough to be
        //considered part of the same table
        ApoTestResults aApo = pIo->TestApo(pIo->nInTable + 1, false, pTabPos);

        /*
        ##513##, #79474# If this is not sufficent, then we should look at
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
    while( 1 );

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
            pNewBand = 0;                   // do not delete
        }
        CalcDefaults();
    }
    delete pNewBand;

    pIo->pPlcxMan->GetPap()->Restore( aSave );
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
    // Non exisitent cells can reduce the number of columns.

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
                if (pT->rgbrc[i].IsZeroed(pIo->bVer67))
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
            if ( pIo->bVer67 ?
             !(SVBT16ToShort(pR->pTCs[pR->nWwCols-1].rgbrc[3].aBits1) & 0x20)
           : !(SVBT16ToShort(pR->pTCs[pR->nWwCols-1].rgbrc[3].aBits2) & 0x2000))
            {
                short nThickness = pR->pTCs[pR->nWwCols-1].rgbrc[3].
                    DetermineBorderProperties(pIo->bVer67);
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
            if ( pIo->bVer67 ?
                  !(SVBT16ToShort(pR->pTCs[0].rgbrc[1].aBits1) & 0x20)
                : !(SVBT16ToShort(pR->pTCs[0].rgbrc[1].aBits2) & 0x2000))
            {
                short nThickness = pR->pTCs[0].rgbrc[1].
                    DetermineBorderProperties(pIo->bVer67);
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

        short nAddCols = pR->bLEmptyCol + pR->bREmptyCol;
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
        contigious invalid cells backwards to the last valid cell instead.
        */
        if (i && pR->bExist[i-1] == false)
        {
            sal_uInt16 k=i-1;
            while (k && pR->bExist[k] == false)
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

void WW8TabDesc::SetSizePosition(SwFrmFmt* pFrmFmt)
{
    SwFrmFmt* pApply = pFrmFmt;
    if (!pApply )
        pApply = pTable->GetFrmFmt();
    OSL_ENSURE(pApply,"No frame");
    pApply->SetFmtAttr(aItemSet);
    if (pFrmFmt)
    {
        SwFmtFrmSize aSize = pFrmFmt->GetFrmSize();
        aSize.SetHeightSizeType(ATT_MIN_SIZE);
        aSize.SetHeight(MINLAY);
        pFrmFmt->SetFmtAttr(aSize);
        pTable->GetFrmFmt()->SetFmtAttr(SwFmtHoriOrient(0,text::HoriOrientation::FULL));
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

void WW8TabDesc::CreateSwTable()
{
    ::SetProgressState(pIo->nProgress, pIo->mpDocShell);   // Update

    // if there is already some content on the Node append new node to ensure
    // that this content remains ABOVE the table
    SwPosition* pPoint = pIo->pPaM->GetPoint();
    bool bInsNode = pPoint->nContent.GetIndex() ? true : false;
    bool bSetMinHeight = false;

    /*
     #i8062#
     Set fly anchor to its anchor pos, so that if a table starts immediately
     at this position a new node will be inserted before inserting the table.
    */
    if (!bInsNode && pIo->pFmtOfJustInsertedApo)
    {
        const SwPosition* pAPos =
            pIo->pFmtOfJustInsertedApo->GetAnchor().GetCntntAnchor();
        if (pAPos && &pAPos->nNode.GetNode() == &pPoint->nNode.GetNode())
        {
            bInsNode = true;
            bSetMinHeight = true;

            SwFmtSurround aSur(pIo->pFmtOfJustInsertedApo->GetSurround());
            aSur.SetAnchorOnly(true);
            pIo->pFmtOfJustInsertedApo->SetFmtAttr(aSur);
        }
    }

    if (bSetMinHeight == true)
    {
        // minimize Fontsize to minimize height growth of the header/footer
        // set font size to 1 point to minimize y-growth of Hd/Ft
        SvxFontHeightItem aSz(20, 100, RES_CHRATR_FONTSIZE);
        pIo->NewAttr( aSz );
        pIo->pCtrlStck->SetAttr(*pPoint, RES_CHRATR_FONTSIZE);
    }

    if (bInsNode)
        pIo->AppendTxtNode(*pPoint);

    pTmpPos = new SwPosition( *pIo->pPaM->GetPoint() );

    // The table is small: The number of columns is the lowest count of
    // columns of the origin, because inserting is faster than deleting.
    // The number of rows is the count of bands because (identically)
    // rows of a band can be duplicated easy.
    pTable = pIo->rDoc.InsertTable(
            SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 0 ),
            *pTmpPos, nBands, nDefaultSwCols, eOri, 0, 0, sal_False, sal_True );

    OSL_ENSURE(pTable && pTable->GetFrmFmt(), "insert table failed");
    if (!pTable || !pTable->GetFrmFmt())
        return;

    SwTableNode* pTableNode = pTable->GetTableNode();
    OSL_ENSURE(pTableNode, "no table node!");
    if (pTableNode)
    {
        pIo->maSectionManager.PrependedInlineNode(*pIo->pPaM->GetPoint(),
            *pTableNode);
    }

    // Check if the node into which the table should be inserted already
    // contains a Pagedesc. If so that Pagedesc would be moved to the
    // row after the table, whats wrong. So delete and
    // set later to the table format.
    if (SwTxtNode *const pNd = pTmpPos->nNode.GetNode().GetTxtNode())
    {
        if (const SfxItemSet* pSet = pNd->GetpSwAttrSet())
        {
            SfxPoolItem *pSetAttr = 0;
            const SfxPoolItem* pItem;
            if (SFX_ITEM_SET == pSet->GetItemState(RES_BREAK, false, &pItem))
            {
                pSetAttr = new SvxFmtBreakItem( *(SvxFmtBreakItem*)pItem );
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
        pTable->GetFrmFmt()->SetFmtAttr(SwFmtFrmSize(ATT_FIX_SIZE, nSwWidth));
        aItemSet.Put(SwFmtFrmSize(ATT_FIX_SIZE, nSwWidth));
    }

    SvxFrameDirectionItem aDirection(
        bIsBiDi ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR );
    pTable->GetFrmFmt()->SetFmtAttr(aDirection);

    if (text::HoriOrientation::LEFT_AND_WIDTH == eOri)
    {
        if (!pIo->nInTable && pIo->InLocalApo() && pIo->pSFlyPara->pFlyFmt &&
            GetMinLeft())
        {
            //If we are inside a frame and we have a border, the frames
            //placement does not consider the tables border, which word
            //displays outside the frame, so adjust here.
            SwFmtHoriOrient aHori(pIo->pSFlyPara->pFlyFmt->GetHoriOrient());
            sal_Int16 eHori = aHori.GetHoriOrient();
            if ((eHori == text::HoriOrientation::NONE) || (eHori == text::HoriOrientation::LEFT) ||
                (eHori == text::HoriOrientation::LEFT_AND_WIDTH))
            {
                //With multiple table, use last table settings. Perhaps
                //the maximum is what word does ?
                aHori.SetPos(pIo->pSFlyPara->nXPos + GetMinLeft());
                aHori.SetHoriOrient(text::HoriOrientation::NONE);
                pIo->pSFlyPara->pFlyFmt->SetFmtAttr(aHori);
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
                    nLeft = pIo->maSectionManager.GetTextAreaWidth();
                    nLeft = nLeft - nPreferredWidth  - nOrgDxaLeft;
                }
                else
                    nLeft = -GetMinLeft();
            }

            aL.SetLeft(nLeft);

            aItemSet.Put(aL);
        }
    }

    mpOldRedlineStack = pIo->mpRedlineStack;
    pIo->mpRedlineStack = new sw::util::RedlineStack(pIo->rDoc);
}

void WW8TabDesc::UseSwTable()
{
    // init global Vars
    pTabLines = &pTable->GetTabLines();
    nAktRow = nAktCol = nAktBandRow = 0;

    pTblNd  = (SwTableNode*)(*pTabLines)[0]->GetTabBoxes()[0]->
        GetSttNd()->FindTableNode();
    OSL_ENSURE( pTblNd, "wo ist mein TabellenNode" );

    // #i69519# - Restrict rows to repeat to a decent value
    if ( nRowsToRepeat == static_cast<sal_uInt16>(nRows) )
        nRowsToRepeat = 1;

    pTblNd->GetTable().SetRowsToRepeat( nRowsToRepeat );
    // insert extra cells if needed and something like this
    AdjustNewBand();

    WW8DupProperties aDup(pIo->rDoc,pIo->pCtrlStck);
    pIo->pCtrlStck->SetAttr(*pIo->pPaM->GetPoint(), 0, false);

    // now set the correct PaM and prepare first merger group if any
    SetPamInCell(nAktCol, true);
    aDup.Insert(*pIo->pPaM->GetPoint());

    pIo->bWasTabRowEnd = false;
    pIo->bWasTabCellEnd = false;
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
                    WW8SelBoxInfo* pActMGroup = 0;
                    //
                    // start a new merge group if appropriate
                    //
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
                        //If its a hori merge only, and the only things in
                        //it are invalid cells then its already taken care
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
                            if (p == 0)
                            {
                                break;
                            }
                            p->bGroupLocked = true;
                        }

                        // 3. push to group array
                        aMergeGroups.push_back(pActMGroup);
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
    SwTableBox *pTabBox2 = 0;
    short nRow = nAktRow + 1;
    if (nRow < (sal_uInt16)pTabLines->size())
    {
        if (SwTableLine *pLine = (*pTabLines)[nRow])
        {
            SwTableBoxes &rBoxes = pLine->GetTabBoxes();
            pTabBox2 = rBoxes.empty() ? 0 : rBoxes.front();
        }
    }

    if (!pTabBox2 || !pTabBox2->GetSttNd())
    {
        MoveOutsideTable();
        return;
    }

    if (pIo->pPaM->GetPoint()->nNode != pTabBox2->GetSttIdx() + 1)
    {
        pIo->pPaM->GetPoint()->nNode = pTabBox2->GetSttIdx() + 1;
        pIo->pPaM->GetPoint()->nContent.Assign(pIo->pPaM->GetCntntNode(), 0);
        pIo->rDoc.SetTxtFmtColl(*pIo->pPaM, (SwTxtFmtColl*)pIo->pDfltTxtFmtColl);
    }
}

void WW8TabDesc::MoveOutsideTable()
{
    OSL_ENSURE(pTmpPos && pIo, "I've forgotten where the table is anchored");
    if (pTmpPos && pIo)
        *pIo->pPaM->GetPoint() = *pTmpPos;
}

void WW8TabDesc::FinishSwTable()
{
    pIo->mpRedlineStack->closeall(*pIo->pPaM->GetPoint());
    delete pIo->mpRedlineStack;
    pIo->mpRedlineStack = mpOldRedlineStack;
    mpOldRedlineStack = 0;

    WW8DupProperties aDup(pIo->rDoc,pIo->pCtrlStck);
    pIo->pCtrlStck->SetAttr( *pIo->pPaM->GetPoint(), 0, false);

    MoveOutsideTable();
    delete pTmpPos, pTmpPos = 0;

    aDup.Insert(*pIo->pPaM->GetPoint());

    pIo->bWasTabRowEnd = false;
    pIo->bWasTabCellEnd = false;

    pIo->maInsertedTables.InsertTable(*pTblNd, *pIo->pPaM);

    MergeCells();

    // if needed group cells together that should be merged
    if( !aMergeGroups.empty() )
    {
        // process all merge groups one by one
        for (
                WW8MergeGroups::iterator groupIt = aMergeGroups.begin();
                groupIt != aMergeGroups.end();
                ++groupIt)
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
        pIo->pFmtOfJustInsertedApo = 0;
        aMergeGroups.clear();
    }
}


// browse aMergeGroups, detect the index of the first fitting group or -1 otherwise
//
// Parameter: nXcenter  = center position of asking box
//            nWidth    = width of asking box
//            bExact    = flag, if box has to fit into group
//                              or only has to touch
//
WW8SelBoxInfo* WW8TabDesc::FindMergeGroup(short nX1, short nWidth, bool bExact)
{
    if( !aMergeGroups.empty() )
    {
        // still valid area near the boundery
        const short nToleranz = 4;
        // box boundery
        short nX2 = nX1 + nWidth;
        // approximate group boundery
        short nGrX1;
        short nGrX2;

        // improvement: search backwards
        for ( short iGr = aMergeGroups.size() - 1; iGr >= 0; --iGr )
        {
            // the currently inspected group
            WW8SelBoxInfo& rActGroup = aMergeGroups[ iGr ];
            if (!rActGroup.bGroupLocked)
            {
                // approximate group boundery with room (tolerance) to the *outside*
                nGrX1 = rActGroup.nGroupXStart - nToleranz;
                nGrX2 = rActGroup.nGroupXStart
                             +rActGroup.nGroupWidth  + nToleranz;
                //
                // If box fits report success
                //
                if( ( nX1 > nGrX1 ) && ( nX2 < nGrX2 ) )
                {
                    return &rActGroup;
                }
                //
                // does the box share areas with the group?
                //
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
    return 0;
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

    if (pIo->pPaM->GetPoint()->nNode == pTabBox->GetSttIdx() + 1)
        return true;

    return false;
}

void WW8TabDesc::StartMiserableHackForUnsupportedDirection(short nWwCol)
{
    OSL_ENSURE(pActBand, "Impossible");
    if (pActBand && pActBand->maDirections[nWwCol] == 3)
    {
        pIo->pCtrlStck->NewAttr(*pIo->pPaM->GetPoint(),
            SvxCharRotateItem(900, false, RES_CHRATR_ROTATE));
    }
}

void WW8TabDesc::EndMiserableHackForUnsupportedDirection(short nWwCol)
{
    OSL_ENSURE(pActBand, "Impossible");
    if (pActBand && pActBand->maDirections[nWwCol] == 3)
        pIo->pCtrlStck->SetAttr(*pIo->pPaM->GetPoint(), RES_CHRATR_ROTATE);
}

bool WW8TabDesc::SetPamInCell(short nWwCol, bool bPam)
{
    OSL_ENSURE( pActBand, "pActBand ist 0" );

    sal_uInt16 nCol = pActBand->transCell(nWwCol);

    if ((sal_uInt16)nAktRow >= pTabLines->size())
    {
        OSL_ENSURE(!this, "Actual row bigger than expected." );
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
                 pIo->bParaAutoBefore && pIo->bFirstPara &&
                 !pIo->pWDop->fDontUseHTMLAutoSpacing
               )
            {
                pIo->SetUpperSpacing(*pIo->pPaM, 0);
            }

            // The last paragraph in a cell with lower autospacing has lower
            // spacing set to 0
            if (pIo->bParaAutoAfter && !pIo->pWDop->fDontUseHTMLAutoSpacing)
                pIo->SetLowerSpacing(*pIo->pPaM, 0);

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
        if(pIo->bParaAutoBefore && pIo->bFirstPara && !pIo->pWDop->fDontUseHTMLAutoSpacing)
            pIo->SetUpperSpacing(*pIo->pPaM, 0);

        // The last paragraph in a cell with lower autospacing has lower spacing set to 0
        if(pIo->bParaAutoAfter && !pIo->pWDop->fDontUseHTMLAutoSpacing)
            pIo->SetLowerSpacing(*pIo->pPaM, 0);

        //We need to set the pPaM on the first cell, invalid
        //or not so that we can collect paragraph proproties over
        //all the cells, but in that case on the valid cell we do not
        //want to reset the fmt properties
        if (pIo->pPaM->GetPoint()->nNode != pTabBox->GetSttIdx() + 1)
        {
            pIo->pPaM->GetPoint()->nNode = pTabBox->GetSttIdx() + 1;
            pIo->pPaM->GetPoint()->nContent.Assign(pIo->pPaM->GetCntntNode(), 0);
            // Precautionally set now, otherwise the style is not set for cells
            // that are inserted for margin balancing.
            pIo->rDoc.SetTxtFmtColl(*pIo->pPaM, (SwTxtFmtColl*)pIo->pDfltTxtFmtColl);
            // because this cells are invisible helper constructions only to simulate
            // the frayed view of WW-tables we do NOT need SetTxtFmtCollAndListLevel()
        }

        // Better to turn Snap to Grid off for all paragraphs in tables
        if(SwTxtNode *pNd = pIo->pPaM->GetNode()->GetTxtNode())
        {
            const SfxPoolItem &rItm = pNd->SwCntntNode::GetAttr(RES_PARATR_SNAPTOGRID);
            SvxParaGridItem &rSnapToGrid = (SvxParaGridItem&)(rItm);

            if(rSnapToGrid.GetValue())
            {
                SvxParaGridItem aGridItem( rSnapToGrid );
                aGridItem.SetValue(false);

                SwPosition* pGridPos = pIo->pPaM->GetPoint();

                xub_StrLen nEnd = pGridPos->nContent.GetIndex();
                pGridPos->nContent.Assign(pIo->pPaM->GetCntntNode(), 0);
                pIo->pCtrlStck->NewAttr(*pGridPos, aGridItem);
                pGridPos->nContent.Assign(pIo->pPaM->GetCntntNode(), nEnd);
                pIo->pCtrlStck->SetAttr(*pGridPos, RES_PARATR_SNAPTOGRID);
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

    pIo->rDoc.GetNodes().InsBoxen( pTblNd, pTabLine, (SwTableBoxFmt*)pTabBox->GetFrmFmt(),
                            (SwTxtFmtColl*)pIo->pDfltTxtFmtColl, 0, pTabBoxes->size(), nIns );
    // The third parameter contains the FrmFmt of the boxes.
    // Here it is possible to optimize to save (reduce) FrmFmts.
}

void WW8TabDesc::SetTabBorders(SwTableBox* pBox, short nWwIdx)
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // faked cells -> no border


    SvxBoxItem aFmtBox( RES_BOX );
    if (pActBand->pTCs)     // neither Cell Border nor Default Border defined ?
    {
        WW8_TCell* pT = &pActBand->pTCs[nWwIdx];
        if (pIo->IsBorder(pT->rgbrc))
            pIo->SetBorder(aFmtBox, pT->rgbrc);
    }

    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwTOP))
    {
        aFmtBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwTOP],
            BOX_LINE_TOP);
    }
    else
        aFmtBox.SetDistance(pActBand->mnDefaultTop, BOX_LINE_TOP);
    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwBOTTOM))
    {
        aFmtBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwBOTTOM],
            BOX_LINE_BOTTOM);
    }
    else
        aFmtBox.SetDistance(pActBand->mnDefaultBottom,BOX_LINE_BOTTOM);

    // nGapHalf for WW is a *horizontal* gap between table cell and content.
    short nLeftDist =
        pActBand->mbHasSpacing ? pActBand->mnDefaultLeft : pActBand->nGapHalf;
    short nRightDist =
        pActBand->mbHasSpacing ? pActBand->mnDefaultRight : pActBand->nGapHalf;
    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwLEFT))
    {
        aFmtBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwLEFT],
            BOX_LINE_LEFT);
    }
    else
        aFmtBox.SetDistance(nLeftDist, BOX_LINE_LEFT);
    if (pActBand->nOverrideSpacing[nWwIdx] & (1 << WW8TabBandDesc::wwRIGHT))
    {
        aFmtBox.SetDistance(
            pActBand->nOverrideValues[nWwIdx][WW8TabBandDesc::wwRIGHT],
            BOX_LINE_RIGHT);
    }
    else
        aFmtBox.SetDistance(nRightDist,BOX_LINE_RIGHT);

    pBox->GetFrmFmt()->SetFmtAttr(aFmtBox);
}

void WW8TabDesc::SetTabShades( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // faked cells -> no color

    bool bFound=false;
    if (pActBand->pNewSHDs && pActBand->pNewSHDs[nWwIdx] != COL_AUTO)
    {
        Color aColor(pActBand->pNewSHDs[nWwIdx]);
        pBox->GetFrmFmt()->SetFmtAttr(SvxBrushItem(aColor, RES_BACKGROUND));
        bFound = true;
    }

    //If there was no new shades, or no new shade setting
    if (pActBand->pSHDs && !bFound)
    {
        WW8_SHD& rSHD = pActBand->pSHDs[nWwIdx];
        if (!rSHD.GetValue())       // auto
            return;

        SwWW8Shade aSh( pIo->bVer67, rSHD );
        pBox->GetFrmFmt()->SetFmtAttr(SvxBrushItem(aSh.aColor, RES_BACKGROUND));
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
            OSL_ENSURE(eDir == 4, "unknown direction code, maybe its a bitfield");
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
    pBox->GetFrmFmt()->SetFmtAttr(aItem);
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

    pBox->GetFrmFmt()->SetFmtAttr( SwFmtVertOrient(0,eVertOri) );
}

void WW8TabDesc::AdjustNewBand()
{
    if( pActBand->nSwCols > nDefaultSwCols )        // split cells
        InsertCells( pActBand->nSwCols - nDefaultSwCols );

    SetPamInCell( 0, false);
    OSL_ENSURE( pTabBoxes && pTabBoxes->size() == (sal_uInt16)pActBand->nSwCols,
        "Falsche Spaltenzahl in Tabelle" );

    if( bClaimLineFmt )
    {
        pTabLine->ClaimFrmFmt();            // necessary because of cell height
        SwFmtFrmSize aF( ATT_MIN_SIZE, 0, 0 );  // default

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
        pTabLine->GetFrmFmt()->SetFmtAttr(aF);
    }

    //Word stores 1 for bCantSplit if the row cannot be split, we set true if
    //we can split the row
    // bCantSplit: Always true for rows containing merged cells (Word <= 2000 crashes otherwise)
    // So in case bCantSplit is true, we check for bCantSplit90, which has been introduced for
    // Word versions >= 2002.
    bool bSetCantSplit = pActBand->bCantSplit;
    if(bSetCantSplit)
        bSetCantSplit = pActBand->bCantSplit90;

    pTabLine->GetFrmFmt()->SetFmtAttr(SwFmtRowSplit(!bSetCantSplit));

    short i;    // SW-Index
    short j;    // WW-Index
    short nW;   // Width
    SwFmtFrmSize aFS( ATT_FIX_SIZE );
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
        // could be reduced further by intelligent moving of FrmFmts
        pBox->ClaimFrmFmt();

        SetTabBorders(pBox, j);

        // #i18128# word has only one line between adjoining vertical cells
        // we have to mimick this in the filter by picking the larger of the
        // sides and using that one on one side of the line (right)
        SvxBoxItem aCurrentBox(sw::util::ItemGet<SvxBoxItem>(*(pBox->GetFrmFmt()), RES_BOX));
        const ::editeng::SvxBorderLine *pLeftLine = aCurrentBox.GetLine(BOX_LINE_LEFT);
        int nCurrentRightLineWidth = 0;
        if(pLeftLine)
            nCurrentRightLineWidth = pLeftLine->GetScaledWidth();

        if (i != 0)
        {
            SwTableBox* pBox2 = (*pTabBoxes)[i-1];
            SvxBoxItem aOldBox(sw::util::ItemGet<SvxBoxItem>(*(pBox2->GetFrmFmt()), RES_BOX));
            const ::editeng::SvxBorderLine *pRightLine = aOldBox.GetLine(BOX_LINE_RIGHT);
            int nOldBoxRightLineWidth = 0;
            if(pRightLine)
                nOldBoxRightLineWidth = pRightLine->GetScaledWidth();

            if(nOldBoxRightLineWidth>nCurrentRightLineWidth)
                aCurrentBox.SetLine(aOldBox.GetLine(BOX_LINE_RIGHT), BOX_LINE_LEFT);

            aOldBox.SetLine(0, BOX_LINE_RIGHT);
            pBox2->GetFrmFmt()->SetFmtAttr(aOldBox);
        }

        pBox->GetFrmFmt()->SetFmtAttr(aCurrentBox);

        SetTabVertAlign(pBox, j);
        SetTabDirection(pBox, j);
        if( pActBand->pSHDs || pActBand->pNewSHDs)
            SetTabShades(pBox, j);
        j++;

        aFS.SetWidth( nW );
        pBox->GetFrmFmt()->SetFmtAttr( aFS );

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
    ::SetProgressState(pIo->nProgress, pIo->mpDocShell);   // Update

    EndMiserableHackForUnsupportedDirection(nAktCol);

    // new line/row
    if( pIo->bWasTabRowEnd )
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
                pIo->rDoc.InsertRow( pTable->SelLineFromBox( pBox, aBoxes ) );
            }
        }
    }
    else
    {                       // new column ( cell )
        nAktCol++;
    }
    SetPamInCell(nAktCol, true);

    // finish Annotated Level Numbering ?
    if (pIo->bAnl && !pIo->bAktAND_fNumberAcross)
        pIo->StopAllAnl(IsValidCell(nAktCol));
}

// if necessary register the box for the merge group for this column
SwTableBox* WW8TabDesc::UpdateTableMergeGroup(  WW8_TCell&     rCell,
                                                WW8SelBoxInfo* pActGroup,
                                                SwTableBox*    pActBox,
                                                sal_uInt16         nCol )
{
    // set default for return
    SwTableBox* pResult = 0;

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
        WW8SelBoxInfo* pTheMergeGroup = 0;
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
const String& WW8TabDesc::GetNumRuleName() const
{
    sal_uInt16 nCol = GetLogicalWWCol();
    if (nCol < aNumRuleNames.size())
        return aNumRuleNames[nCol];
    else
        return aEmptyStr;
}

void WW8TabDesc::SetNumRuleName( const String& rName )
{
    sal_uInt16 nCol = GetLogicalWWCol();
    for (sal_uInt16 nSize = static_cast< sal_uInt16 >(aNumRuleNames.size()); nSize <= nCol; ++nSize)
        aNumRuleNames.push_back(aEmptyStr);
    aNumRuleNames[nCol] = rName;
}

bool SwWW8ImplReader::StartTable(WW8_CP nStartCp)
{
    // Entering a table so make sure the FirstPara flag gets set
    bFirstPara = true;
    // keine rekursiven Tabellen Nicht bei EinfuegenDatei in Tabelle oder
    // Fussnote
    if (bReadNoTbl)
        return false;

    if (pTableDesc)
        maTableStack.push(pTableDesc);

    // #i33818# - determine absolute position object attributes,
    // if possible. It's needed for nested tables.
    WW8FlyPara* pTableWFlyPara( 0L );
    WW8SwFlyPara* pTableSFlyPara( 0L );
    // #i45301# - anchor nested table inside Writer fly frame
    // only at-character, if absolute position object attributes are available.
    // Thus, default anchor type is as-character anchored.
    RndStdIds eAnchor( FLY_AS_CHAR );
    if ( nInTable )
    {
        WW8_TablePos* pNestedTabPos( 0L );
        WW8_TablePos aNestedTabPos;
        WW8PLCFxSave1 aSave;
        pPlcxMan->GetPap()->Save( aSave );
        WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
        WW8_CP nMyStartCp = nStartCp;
        if ( SearchRowEnd( pPap, nMyStartCp, nInTable ) &&
             ParseTabPos( &aNestedTabPos, pPap ) )
        {
            pNestedTabPos = &aNestedTabPos;
        }
        pPlcxMan->GetPap()->Restore( aSave );
        if ( pNestedTabPos )
        {
            ApoTestResults aApo = TestApo( nInTable + 1, false, pNestedTabPos );
            pTableWFlyPara = ConstructApo( aApo, pNestedTabPos );
            if ( pTableWFlyPara )
            {
                // <WW8SwFlyPara> constructor has changed - new 4th parameter
                // containing WW8 page top margin.
                pTableSFlyPara = new WW8SwFlyPara(*pPaM, *this, *pTableWFlyPara,
                    maSectionManager.GetWWPageTopMargin(),
                    maSectionManager.GetPageLeft(), maSectionManager.GetTextAreaWidth(),
                    nIniFlyDx, nIniFlyDy);

                // #i45301# - anchor nested table Writer fly frame at-character
                eAnchor = FLY_AT_CHAR;
            }
        }
    }

    pTableDesc = new WW8TabDesc( this, nStartCp );

    if( pTableDesc->Ok() )
    {
        int nNewInTable = nInTable + 1;

        if ((eAnchor == FLY_AT_CHAR)
            && !maTableStack.empty() && !InEqualApo(nNewInTable) )
        {
            pTableDesc->pParentPos = new SwPosition(*pPaM->GetPoint());
            SfxItemSet aItemSet(rDoc.GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1);
            // #i33818# - anchor the Writer fly frame for the nested table at-character.
            // #i45301#
            SwFmtAnchor aAnchor( eAnchor );
            aAnchor.SetAnchor( pTableDesc->pParentPos );
            aItemSet.Put( aAnchor );
            pTableDesc->pFlyFmt = rDoc.MakeFlySection( eAnchor,
                                                      pTableDesc->pParentPos, &aItemSet);
            OSL_ENSURE( pTableDesc->pFlyFmt->GetAnchor().GetAnchorId() == eAnchor,
                   "Not the anchor type requested!" );
            MoveInsideFly(pTableDesc->pFlyFmt);
        }
        pTableDesc->CreateSwTable();
        if (pTableDesc->pFlyFmt)
        {
            pTableDesc->SetSizePosition(pTableDesc->pFlyFmt);
            // #i33818# - Use absolute position object attributes,
            // if existing, and apply them to the created Writer fly frame.
            if ( pTableWFlyPara && pTableSFlyPara )
            {
                WW8FlySet aFlySet( *this, pTableWFlyPara, pTableSFlyPara, false );
                SwFmtAnchor aAnchor( FLY_AT_CHAR );
                aAnchor.SetAnchor( pTableDesc->pParentPos );
                aFlySet.Put( aAnchor );
                pTableDesc->pFlyFmt->SetFmtAttr( aFlySet );
            }
            else
            {
                SwFmtHoriOrient aHori =
                            pTableDesc->pTable->GetFrmFmt()->GetHoriOrient();
                pTableDesc->pFlyFmt->SetFmtAttr(aHori);
                pTableDesc->pFlyFmt->SetFmtAttr( SwFmtSurround( SURROUND_NONE ) );
            }
            // #i33818# - The nested table doesn't have to leave
            // the table cell. Thus, the Writer fly frame has to follow the text flow.
            pTableDesc->pFlyFmt->SetFmtAttr( SwFmtFollowTextFlow( sal_True ) );
        }
        else
            pTableDesc->SetSizePosition(0);
        pTableDesc->UseSwTable();
    }
    else
        PopTableDesc();

    // #i33818#
    delete pTableWFlyPara;
    delete pTableSFlyPara;

    return 0 != pTableDesc;
}

void SwWW8ImplReader::TabCellEnd()
{
    if (nInTable && pTableDesc)
        pTableDesc->TableCellEnd();

    bFirstPara = true;    // We have come to the end of a cell so FirstPara flag
    bReadTable = false;
    mpTableEndPaM.reset();
}

void SwWW8ImplReader::Read_TabCellEnd( sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( ( nLen > 0 ) && ( *pData == 1 ) )
        bWasTabCellEnd = true;
}

void SwWW8ImplReader::Read_TabRowEnd( sal_uInt16, const sal_uInt8* pData, short nLen )   // Sprm25
{
    if( ( nLen > 0 ) && ( *pData == 1 ) )
        bWasTabRowEnd = true;
}

void SwWW8ImplReader::PopTableDesc()
{
    if (pTableDesc && pTableDesc->pFlyFmt)
    {
        MoveOutsideFly(pTableDesc->pFlyFmt,*pTableDesc->pParentPos);
    }

    delete pTableDesc;
    if (maTableStack.empty())
        pTableDesc = 0;
    else
    {
       pTableDesc = maTableStack.top();
       maTableStack.pop();
    }
}

void SwWW8ImplReader::StopTable()
{
    OSL_ENSURE(pTableDesc, "Panic, stop table with no table!");
    if (!pTableDesc)
        return;

    // We are leaving a table so make sure the next paragraph doesn't think
    // it's the first paragraph
    bFirstPara = false;

    pTableDesc->FinishSwTable();
    PopTableDesc();

    bReadTable = true;
    // #i101116# - Keep PaM on table end only for nested tables
    if ( nInTable > 1 )
    {
        mpTableEndPaM.reset(new SwPaM(*pPaM));
    }
}

// GetTableLeft() is needed for graphic objects bound to paragraphs in tables.
// For indented tables the base for WW is the margin that would be used without
// the table; SW uses the left table margin.
short SwWW8ImplReader::GetTableLeft()
{
    return (pTableDesc) ? pTableDesc->GetMinLeft() : 0;
}

bool SwWW8ImplReader::IsInvalidOrToBeMergedTabCell() const
{
    if( !pTableDesc )
        return false;

    const WW8_TCell* pCell = pTableDesc->GetAktWWCell();

    return     !pTableDesc->IsValidCell( pTableDesc->GetAktCol() )
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
    if( !vColl.empty() )
    {
        for(sal_uInt16 nI = 0; nI < pStyles->GetCount(); nI++ )
            if(    vColl[ nI ].bValid
                && (nLFOIndex == vColl[ nI ].nLFOIndex) )
                nRes = nI;
    }
    return nRes;
}

const SwFmt* SwWW8ImplReader::GetStyleWithOrgWWName( String& rName ) const
{
    SwFmt* pRet = 0;
    if( !vColl.empty() )
    {
        for(sal_uInt16 nI = 0; nI < pStyles->GetCount(); nI++ )
            if(    vColl[ nI ].bValid
                && (rName.Equals( vColl[ nI ].GetOrgWWName())) )
            {
                pRet = vColl[ nI ].pFmt;
                break;
            }
    }
    return pRet;
}

//-----------------------------------------
//          class WW8RStyle
//-----------------------------------------

const sal_uInt8* WW8RStyle::HasParaSprm( sal_uInt16 nId ) const
{
    if( !pParaSprms || !nSprmsLen )
        return 0;

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

    pParaSprms = 0;
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
        *pStStrm >> cbUPX;

        nLen-=2;

        if ( cbUPX > nLen )
            cbUPX = nLen;       // shrink cbUPX to nLen

        if( (1 < cbUPX) || ( (0 < cbUPX) && !bPAP ) )
        {
            if( bPAP )
            {
                sal_uInt16 id;
                *pStStrm >> id;

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
    : WW8Style(*pI->pTableStream, _rFib), maSprmParser(_rFib.GetFIBVersion()),
    pIo(pI), pStStrm(pI->pTableStream), pStyRule(0), nWwNumLevel(0)
{
    pIo->vColl.resize(cstd);
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

    if( !pIo->bNoAttrImport )
    {
        // Style has no text color set, winword default is auto
        if ( !bTxtColChanged )
            pIo->pAktColl->SetFmtAttr(SvxColorItem(Color(COL_AUTO), RES_CHRATR_COLOR));

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !bFSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            pIo->pAktColl->SetFmtAttr(aAttr);
            aAttr.SetWhich(RES_CHRATR_CJK_FONTSIZE);
            pIo->pAktColl->SetFmtAttr(aAttr);
        }

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !bFCTLSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            aAttr.SetWhich(RES_CHRATR_CTL_FONTSIZE);
            pIo->pAktColl->SetFmtAttr(aAttr);
        }

        if( !bWidowsChanged )  // Widows ?
        {
            pIo->pAktColl->SetFmtAttr( SvxWidowsItem( 2, RES_PARATR_WIDOWS ) );
            pIo->pAktColl->SetFmtAttr( SvxOrphansItem( 2, RES_PARATR_ORPHANS ) );
        }
    }
}

bool WW8RStyle::PrepareStyle(SwWW8StyInf &rSI, ww::sti eSti, sal_uInt16 nThisStyle, sal_uInt16 nNextStyle)
{
    SwFmt* pColl;
    bool bStyExist;

    if (rSI.bColl)
    {
        // Para-Style
        sw::util::ParaStyleMapper::StyleResult aResult =
            pIo->maParaStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }
    else
    {
        // Char-Style
        sw::util::CharStyleMapper::StyleResult aResult =
            pIo->maCharStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }

    bool bImport = !bStyExist || pIo->mbNewDoc; // import content ?
    bool bOldNoImp = pIo->bNoAttrImport;
    rSI.bImportSkipped = !bImport;

    if( !bImport )
        pIo->bNoAttrImport = true;
    else
    {
        if (bStyExist)
        {
            pColl->ResetAllFmtAttr(); // #i73790# - method renamed
        }
        pColl->SetAuto(false);          // suggested by JP
    }                                   // but changes the UI
    pIo->pAktColl = pColl;
    rSI.pFmt = pColl;                  // remember translation WW->SW
    rSI.bImportSkipped = !bImport;

    // Set Based on style
    sal_uInt16 j = rSI.nBase;
    if (j != nThisStyle && j < cstd )
    {
        SwWW8StyInf* pj = &pIo->vColl[j];
        if (rSI.pFmt && pj->pFmt && rSI.bColl == pj->bColl)
        {
            rSI.pFmt->SetDerivedFrom( pj->pFmt );  // ok, set Based on
            rSI.eLTRFontSrcCharSet = pj->eLTRFontSrcCharSet;
            rSI.eRTLFontSrcCharSet = pj->eRTLFontSrcCharSet;
            rSI.eCJKFontSrcCharSet = pj->eCJKFontSrcCharSet;
            rSI.n81Flags = pj->n81Flags;
            rSI.n81BiDiFlags = pj->n81BiDiFlags;
            rSI.nOutlineLevel = pj->nOutlineLevel;
            rSI.bParaAutoBefore = pj->bParaAutoBefore;
            rSI.bParaAutoAfter = pj->bParaAutoAfter;

            if (pj->pWWFly)
                rSI.pWWFly = new WW8FlyPara(pIo->bVer67, pj->pWWFly);
        }
    }
    else if( pIo->mbNewDoc && bStyExist )
        rSI.pFmt->SetDerivedFrom(0);

    rSI.nFollow = nNextStyle;       // remember Follow

    pStyRule = 0;                   // recreate if necessary
    bTxtColChanged = bFontChanged = bCJKFontChanged = bCTLFontChanged =
        bFSizeChanged = bFCTLSizeChanged = bWidowsChanged = false;
    pIo->SetNAktColl( nThisStyle );
    pIo->bStyNormal = nThisStyle == 0;
    return bOldNoImp;
}

void WW8RStyle::PostStyle(SwWW8StyInf &rSI, bool bOldNoImp)
{
    // Reset attribute flags, because there are no style-ends.

    pIo->bHasBorder = pIo->bShdTxtCol = pIo->bCharShdTxtCol
        = pIo->bSpec = pIo->bObj = pIo->bSymbol = false;
    pIo->nCharFmt = -1;

    // If Style basiert auf Nichts oder Basis ignoriert
    if ((rSI.nBase >= cstd || pIo->vColl[rSI.nBase].bImportSkipped) && rSI.bColl)
    {
        // If Char-Styles does not work
        // -> set hard WW-Defaults
        Set1StyleDefaults();
    }

    pStyRule = 0;                   // to be on the safe side
    pIo->bStyNormal = false;
    pIo->SetNAktColl( 0 );
    pIo->bNoAttrImport = bOldNoImp;
    // reset the list-remember-fields, if used when reading styles
    pIo->nLFOPosition = USHRT_MAX;
    pIo->nListLevel = WW8ListManager::nMaxLevel;
}

void WW8RStyle::Import1Style( sal_uInt16 nNr )
{
    if (nNr >= pIo->vColl.size())
        return;

    SwWW8StyInf &rSI = pIo->vColl[nNr];

    if( rSI.bImported || !rSI.bValid )
        return;

    rSI.bImported = true;                      // set flag now to avoid endless loops
                                                //
    // valid and not NUL and not yet imported

    if( rSI.nBase < cstd && !pIo->vColl[rSI.nBase].bImported )
        Import1Style( rSI.nBase );

    pStStrm->Seek( rSI.nFilePos );

    short nSkip, cbStd;
    String sName;

    boost::scoped_ptr<WW8_STD> xStd(Read1Style(nSkip, &sName, &cbStd));// read Style

    if (xStd)
        rSI.SetOrgWWIdent( sName, xStd->sti );

    // either no Name or unused Slot or unknown Style

    if ( !xStd || (0 == sName.Len()) || ((1 != xStd->sgc) && (2 != xStd->sgc)) )
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
    ImportGrupx(nSkip, xStd->sgc == 1, rSI.nFilePos & 1);

    PostStyle(rSI, bOldNoImp);

    pStStrm->Seek( nPos+nSkip );
}

void WW8RStyle::RecursiveReg(sal_uInt16 nNr)
{
    if (nNr >= pIo->vColl.size())
        return;

    SwWW8StyInf &rSI = pIo->vColl[nNr];
    if( rSI.bImported || !rSI.bValid )
        return;

    rSI.bImported = true;

    if( rSI.nBase < cstd && !pIo->vColl[rSI.nBase].bImported )
        RecursiveReg(rSI.nBase);

    pIo->RegisterNumFmtOnStyle(nNr);

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
        pIo->vColl[i].bImported = false;

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
        if (pIo->vColl[i].bValid)
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
        SwWW8StyInf &rSI = pIo->vColl[i];

        rSI.nFilePos = pStStrm->Tell();        // remember FilePos
        WW8_STD* pStd = Read1Style( nSkip, 0, 0 );  // read STD
        rSI.bValid = (0 != pStd);
        if (rSI.bValid)
        {
            rSI.nBase = pStd->istdBase;        // remember Basis
            rSI.bColl = ( pStd->sgc == 1 );    // Para-Style
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

    while (1)
    {
        sal_uInt8 nFlags8;
        rSt >> nFlags8;
        nCount++;

        aChpx.fBold = nFlags8 & 0x01;
        aChpx.fItalic = (nFlags8 & 0x02) >> 1;
        aChpx.fRMarkDel = (nFlags8 & 0x04) >> 2;
        aChpx.fOutline = (nFlags8 & 0x08) >> 3;
        aChpx.fFldVanish = (nFlags8 & 0x10) >> 4;
        aChpx.fSmallCaps = (nFlags8 & 0x20) >> 5;
        aChpx.fCaps = (nFlags8 & 0x40) >> 6;
        aChpx.fVanish = (nFlags8 & 0x80) >> 7;

        if (nCount >= nSize) break;
        rSt >> nFlags8;
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
        rSt >> nFlags8;
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
        rSt >> nFlags8;
        nCount++;

        aChpx.fsFtcBi = nFlags8 & 0x01;
        aChpx.fsHpsBi = (nFlags8 & 0x02) >> 1;
        aChpx.fsLidBi = (nFlags8 & 0x04) >> 2;

        if (nCount >= nSize) break;
        rSt >> aChpx.ftc;
        nCount+=2;

        if (nCount >= nSize) break;
        rSt >> aChpx.hps;
        nCount+=2;

        if (nCount >= nSize) break;
        rSt >> nFlags8;
        nCount++;

        aChpx.qpsSpace = nFlags8 & 0x3F;
        aChpx.fSysVanish = (nFlags8 & 0x40) >> 6;
        aChpx.fNumRun = (nFlags8 & 0x80) >> 7;

        if (nCount >= nSize) break;
        rSt >> nFlags8;
        nCount++;

        aChpx.ico = nFlags8 & 0x1F;
        aChpx.kul = (nFlags8 & 0xE0) >> 5;

        if (nCount >= nSize) break;
        rSt >> aChpx.hpsPos;
        nCount++;

        if (nCount >= nSize) break;
        rSt >> aChpx.icoBi;
        nCount++;

        if (nCount >= nSize) break;
        rSt >> aChpx.lid;
        nCount+=2;

        if (nCount >= nSize) break;
        rSt >> aChpx.ftcBi;
        nCount+=2;

        if (nCount >= nSize) break;
        rSt >> aChpx.hpsBi;
        nCount+=2;

        if (nCount >= nSize) break;
        rSt >> aChpx.lidBi;
        nCount+=2;

        if (nCount >= nSize) break;
        rSt >> aChpx.fcPic;
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
        pIo->vColl[i].bColl = true;
        //every chain must end eventually at the null style (style code 222)
        pIo->vColl[i].nBase = 222;
    }

    rtl_TextEncoding eStructChrSet = WW8Fib::GetFIBCharset(
        pIo->pWwFib->chseTables);

    sal_uInt16 cstcStd;
    rSt >> cstcStd;

    sal_uInt16 cbName;
    rSt >> cbName;
    sal_uInt16 nByteCount = 2;
    sal_uInt16 stcp=0;
    while (nByteCount < cbName)
    {
        sal_uInt8 nCount;
        rSt >> nCount;
        nByteCount++;

        sal_uInt8 stc = static_cast< sal_uInt8 >((stcp - cstcStd) & 255);
        if (stc >=pIo->vColl.size())
            continue;

        SwWW8StyInf &rSI = pIo->vColl[stc];
        if (nCount != 0xFF)    // undefined style
        {
            String sName;
            if (nCount == 0)   // inbuilt style
            {
                ww::sti eSti = ww::GetCanonicalStiFromStc(stc);
                if (const sal_Char *pStr = GetEnglishNameFromSti(eSti))
                    sName = String(pStr, RTL_TEXTENCODING_ASCII_US);
                else
                    sName = OUString("Unknown");
            }
            else               // user style
            {
                OString aTmp = read_uInt8s_ToOString(rSt, nCount);
                nByteCount += aTmp.getLength();
                sName = OStringToOUString(aTmp, eStructChrSet);
            }
            rSI.SetOrgWWIdent(sName, stc);
            rSI.bImported = true;
        }
        else
        {
            ww::sti eSti = ww::GetCanonicalStiFromStc(stc);
            if (const sal_Char *pStr = GetEnglishNameFromSti(eSti))
            {
                String sName = String(pStr, RTL_TEXTENCODING_ASCII_US);
                rSI.SetOrgWWIdent(sName, stc);
            }
        }
        stcp++;
    }

    sal_uInt16 nStyles=stcp;

    std::vector<pxoffset> aCHPXOffsets(stcp);
    sal_uInt16 cbChpx;
    rSt >> cbChpx;
    nByteCount = 2;
    stcp=0;
    std::vector< std::vector<sal_uInt8> > aConvertedChpx;
    while (nByteCount < cbChpx)
    {
        sal_uInt8 cb;
        rSt >> cb;
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

        stcp++;
        if (stcp == nStyles)
    {
            rSt.SeekRel(cbChpx-nByteCount);
            nByteCount += cbChpx-nByteCount;
    }
    }

    std::vector<pxoffset> aPAPXOffsets(stcp);
    sal_uInt16 cbPapx;
    rSt >> cbPapx;
    nByteCount = 2;
    stcp=0;
    while (nByteCount < cbPapx)
    {
        sal_uInt8 cb;
        rSt >> cb;
        nByteCount++;

        aPAPXOffsets[stcp].mnSize = 0;

        if (cb != 0xFF)
        {
            sal_uInt8 stc2;
            rSt >> stc2;
            rSt.SeekRel(6);
            nByteCount+=7;
            sal_uInt8 nRemainder = cb-7;

            aPAPXOffsets[stcp].mnOffset = rSt.Tell();
            aPAPXOffsets[stcp].mnSize = nRemainder;

            rSt.SeekRel(nRemainder);
            nByteCount += nRemainder;
        }

        stcp++;

        if (stcp == nStyles)
    {
            rSt.SeekRel(cbPapx-nByteCount);
            nByteCount += cbPapx-nByteCount;
    }
    }

    sal_uInt16 iMac;
    rSt >> iMac;

    if (iMac > nStyles) iMac = nStyles;

    for (stcp = 0; stcp < iMac; ++stcp)
    {
        sal_uInt8 stcNext, stcBase;
        rSt >> stcNext;
        rSt >> stcBase;

        sal_uInt8 stc = static_cast< sal_uInt8 >((stcp - cstcStd) & 255);

        /*
          #i64557# style based on itself
          every chain must end eventually at the null style (style code 222)
        */
        if (stc == stcBase)
            stcBase = 222;

        SwWW8StyInf &rSI = pIo->vColl[stc];
        rSI.nBase = stcBase;

        ww::sti eSti = ww::GetCanonicalStiFromStc(stc);

        if (eSti == ww::stiNil)
            continue;

        rSI.bValid = true;

        if (ww::StandardStiIsCharStyle(eSti) && !aPAPXOffsets[stcp].mnSize)
            pIo->vColl[stc].bColl = false;

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
        if (pIo->vColl[i].bValid)
            Import1Style( i );
}

void WW8RStyle::ImportStyles()
{
    if (ww::eWW2 == pIo->pWwFib->GetFIBVersion())
        ImportOldFormatStyles();
    else
        ImportNewFormatStyles();
}

void WW8RStyle::Import()
{
    pIo->pDfltTxtFmtColl  = pIo->rDoc.GetDfltTxtFmtColl();
    pIo->pStandardFmtColl =
        pIo->rDoc.GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false);

    if( pIo->nIniFlags & WW8FL_NO_STYLES )
        return;

    ImportStyles();

    for (sal_uInt16 i = 0; i < cstd; ++i)
    {
        // Follow chain
        SwWW8StyInf* pi = &pIo->vColl[i];
        sal_uInt16 j = pi->nFollow;
        if( j < cstd )
        {
            SwWW8StyInf* pj = &pIo->vColl[j];
            if ( j != i                             // rational Index ?
                 && pi->pFmt                        // Format ok ?
                 && pj->pFmt                        // Derived-Format ok ?
                 && pi->bColl                       // only possible for paragraph templates (WW)
                 && pj->bColl ){                    // identical Typ ?
                    ( (SwTxtFmtColl*)pi->pFmt )->SetNextTxtFmtColl(
                     *(SwTxtFmtColl*)pj->pFmt );    // ok, register
            }
        }
    }

    // Missing special handling for default character template
    // "Absatz-Standardschriftart" ( Style-ID 65 ).
    // That is empty by default ( WW6 dt and US ) and not changeable
    // via WW-UI so this does not matter.
    // This could be done by:
    //  if( bNew ) rDoc.SetDefault( pDefCharFmt->GetAttrSet() );
    //
    // for e.g. tables an always valid Std-Style is necessary

    if( pIo->StyleExists(0) && !pIo->vColl.empty() &&
        pIo->vColl[0].pFmt && pIo->vColl[0].bColl && pIo->vColl[0].bValid )
        pIo->pDfltTxtFmtColl = (SwTxtFmtColl*)pIo->vColl[0].pFmt;
    else
        pIo->pDfltTxtFmtColl = pIo->rDoc.GetDfltTxtFmtColl();


    // set Hyphenation flag on BASIC para-style
    if (pIo->mbNewDoc && pIo->pStandardFmtColl)
    {
        if (pIo->pWDop->fAutoHyphen
            && SFX_ITEM_SET != pIo->pStandardFmtColl->GetItemState(
                                            RES_PARATR_HYPHENZONE, false) )
        {
            SvxHyphenZoneItem aAttr(true, RES_PARATR_HYPHENZONE);
            aAttr.GetMinLead()    = 2;
            aAttr.GetMinTrail()   = 2;
            aAttr.GetMaxHyphens() = 0;

            pIo->pStandardFmtColl->SetFmtAttr( aAttr );
        }

        /*
        Word defaults to ltr not from environment like writer. Regardless of
        the page/sections rtl setting the standard style lack of rtl still
        means ltr
        */
        if (SFX_ITEM_SET != pIo->pStandardFmtColl->GetItemState(RES_FRAMEDIR,
            false))
        {
           pIo->pStandardFmtColl->SetFmtAttr(
                SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));
        }
    }

    // we do not read styles anymore:
    pIo->pAktColl = 0;
}

CharSet SwWW8StyInf::GetCharSet() const
{
    if ((pFmt) && (pFmt->GetFrmDir().GetValue() == FRMDIR_HORI_RIGHT_TOP))
        return eRTLFontSrcCharSet;
    return eLTRFontSrcCharSet;
}

CharSet SwWW8StyInf::GetCJKCharSet() const
{
    if ((pFmt) && (pFmt->GetFrmDir().GetValue() == FRMDIR_HORI_RIGHT_TOP))
        return eRTLFontSrcCharSet;
    return eCJKFontSrcCharSet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
