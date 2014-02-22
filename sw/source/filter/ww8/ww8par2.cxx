/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <pam.hxx>
#include <doc.hxx>
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
#include "../inc/wwstyles.hxx"
#include "writerhelper.hxx"
#include "ww8struc.hxx"
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
    std::vector<OUString> aNumRuleNames;
    sw::util::RedlineStack *mpOldRedlineStack;

    SwWW8ImplReader* pIo;

    WW8TabBandDesc* pFirstBand;
    WW8TabBandDesc* pActBand;

    SwPosition* pTmpPos;

    SwTableNode* pTblNd;            
    const SwTableLines* pTabLines;  
    SwTableLine* pTabLine;          
    SwTableBoxes* pTabBoxes;        
    SwTableBox* pTabBox;            

    WW8MergeGroups aMergeGroups;   

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
                                
    short nAktRow;
    short nAktBandRow;          
                                
    short nAktCol;

    sal_uInt16 nRowsToRepeat;

    

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

    
    
    SwTableBox* UpdateTableMergeGroup(WW8_TCell& rCell,
        WW8SelBoxInfo* pActGroup, SwTableBox* pActBox, sal_uInt16 nCol  );
    void StartMiserableHackForUnsupportedDirection(short nWwCol);
    void EndMiserableHackForUnsupportedDirection(short nWwCol);
    
    WW8TabDesc(const WW8TabDesc&);
    WW8TabDesc &operator=(const WW8TabDesc&);
public:
    const SwTable* pTable;          
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
    
    OUString GetNumRuleName() const;
    void SetNumRuleName( const OUString& rName );

    sw::util::RedlineStack* getOldRedlineStack(){ return mpOldRedlineStack; }
};

void sw::util::RedlineStack::close( const SwPosition& rPos,
    RedlineType_t eType, WW8TabDesc* pTabDesc )
{
    
    
    if( !close( rPos, eType ) )
    {
        if( pTabDesc && pTabDesc->getOldRedlineStack() )
        {
            bool const bResult =
                pTabDesc->getOldRedlineStack()->close(rPos, eType);
            OSL_ENSURE( bResult, "close without open!");
            (void) bResult; 
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

    
    
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    sal_Int32 nPos = pPaM->GetPoint()->nContent.GetIndex();

    OUString sChar;
    SwTxtAttr* pFN = 0;
    
    if (pTxt && nPos)
    {
        sChar += OUString(pTxt->GetTxt()[--nPos]);
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

        SwPosition aTmpPos( *pPaM->GetPoint() );    
        WW8PLCFxSaveAll aSave;
        pPlcxMan->SaveAllPLCFx( aSave );
        WW8PLCFMan* pOldPlcxMan = pPlcxMan;

        const SwNodeIndex* pSttIdx = ((SwTxtFtn*)pFN)->GetStartNode();
        OSL_ENSURE(pSttIdx, "Probleme beim Anlegen des Fussnoten-Textes");

        ((SwTxtFtn*)pFN)->SetSeqNo( rDoc.GetFtnIdxs().size() );

        bool bOld = bFtnEdn;
        bFtnEdn = true;

        
        Read_HdFtFtnText( pSttIdx, rDesc.mnStartCp, rDesc.mnLen, rDesc.meType);
        bFtEdOk = true;
        bFtnEdn = bOld;

        OSL_ENSURE(sChar.getLength()==1 && ((rDesc.mbAutoNum == (sChar[0] == 2))),
         "footnote autonumbering must be 0x02, and everthing else must not be");

        
        
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
        if (pTNd && !pTNd->GetTxt().isEmpty() && !sChar.isEmpty())
        {
            const OUString &rTxt = pTNd->GetTxt();
            if (rTxt[0] == sChar[0])
            {
                pPaM->GetPoint()->nContent.Assign( pTNd, 0 );
                pPaM->SetMark();
                
                if (rTxt.getLength() > 1 && rTxt[1] == 0x09)
                    pPaM->GetMark()->nContent++;
                pPaM->GetMark()->nContent++;
                pReffingStck->Delete(*pPaM);
                rDoc.DeleteRange( *pPaM );
                pPaM->DeleteMark();
            }
        }

        *pPaM->GetPoint() = aTmpPos;        

        pPlcxMan = pOldPlcxMan;             
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
                    return true;    
                }
            }
        }

        aRes.nStartPos = aRes.nEndPos;
        aRes.pMemPos = 0;
        
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);
        
        rStartCp = aRes.nEndPos;
    }

    return false;
}

ApoTestResults SwWW8ImplReader::TestApo(int nCellLevel, bool bTableRowEnd,
    const WW8_TablePos *pTabPos)
{
    const WW8_TablePos *pTopLevelTable = nCellLevel <= 1 ? pTabPos : 0;
    ApoTestResults aRet;
    
    
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

    aRet.mbStartApo = bNowApo && !InAnyApo(); 
    aRet.mbStopApo = InEqualOrHigherApo(nCellLevel) && !bNowApo;  

    
    
    
    
    
    
    if (bNowApo && InEqualApo(nCellLevel))
    {
        
        if (!TestSameApo(aRet, pTabPos))
            aRet.mbStopApo = aRet.mbStartApo = true;
    }

    return aRet;
}





static void SetBaseAnlv(SwNumFmt &rNum, WW8_ANLV &rAV, sal_uInt8 nSwLevel )
{
    static const SvxExtNumType eNumA[8] = { SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC };

    static const SvxAdjust eAdjA[4] = { SVX_ADJUST_LEFT,
        SVX_ADJUST_RIGHT, SVX_ADJUST_LEFT, SVX_ADJUST_LEFT };



    rNum.SetNumberingType( static_cast< sal_Int16 >(( rAV.nfc < 8 ) ?
                    eNumA[ rAV.nfc ] : SVX_NUM_NUMBER_NONE) );
    if ((rAV.aBits1 & 0x4) >> 2)
        rNum.SetIncludeUpperLevels(nSwLevel + 1);
    rNum.SetStart( SVBT16ToShort( rAV.iStartAt ) );
    rNum.SetNumAdjust( eAdjA[ rAV.aBits1 & 0x3] );

    rNum.SetCharTextDistance( SVBT16ToShort( rAV.dxaSpace ) );
    sal_Int16 nIndent = std::abs((sal_Int16)SVBT16ToShort( rAV.dxaIndent ));
    if( rAV.aBits1 & 0x08 )      
    {
        rNum.SetFirstLineOffset( -nIndent );
        rNum.SetLSpace( nIndent );
        rNum.SetAbsLSpace( nIndent );
    }
    else
        rNum.SetCharTextDistance( nIndent );        

    if( rAV.nfc == 5 || rAV.nfc == 7 )
    {
        OUString sP = "." + rNum.GetSuffix();
        rNum.SetSuffix( sP );   
    }
}

void SwWW8ImplReader::SetAnlvStrings(SwNumFmt &rNum, WW8_ANLV &rAV,
    const sal_uInt8* pTxt, bool bOutline)
{
    bool bInsert = false;                       
    rtl_TextEncoding eCharSet = eStructCharSet;

    const WW8_FFN* pF = pFonts->GetFont(SVBT16ToShort(rAV.ftc)); 
    bool bListSymbol = pF && ( pF->chs == 2 );      

    OUString sTxt;
    if (bVer67)
    {
        sTxt = OUString((sal_Char*)pTxt, rAV.cbTextBefore + rAV.cbTextAfter, eCharSet);
    }
    else
    {
        for(sal_Int32 i = 0; i < rAV.cbTextBefore + rAV.cbTextAfter; ++i, pTxt += 2)
        {
            sTxt += OUString(SVBT16ToShort(*(SVBT16*)pTxt));
        }
    }

    if( bOutline )
    {                             
        if( !rNum.GetIncludeUpperLevels()                          
            || rNum.GetNumberingType() == SVX_NUM_NUMBER_NONE )    
        {
                                                
            bInsert = true;                     

            
            if( bListSymbol )
            {
                
                OUStringBuffer aBuf;
                comphelper::string::padToLength(aBuf, rAV.cbTextBefore
                    + rAV.cbTextAfter, cBulletChar);
                sTxt = aBuf.makeStringAndClear();
            }
        }
    }
    else
    {                                       
        bInsert = true;
        if( bListSymbol )
        {
            FontFamily eFamily;
            OUString aName;
            FontPitch ePitch;

            if( GetFontParams( SVBT16ToShort( rAV.ftc ), eFamily, aName,
                                ePitch, eCharSet ) ){

                Font aFont;
                aFont.SetName( aName );
                aFont.SetFamily( eFamily );

                aFont.SetCharSet( eCharSet );
                rNum.SetNumberingType(SVX_NUM_CHAR_SPECIAL);

                rNum.SetBulletFont( &aFont );

                
                if (rAV.cbTextBefore || rAV.cbTextAfter)
                    rNum.SetBulletChar( sTxt[ 0 ] );
                else
                    rNum.SetBulletChar( 0x2190 );
            }
        }
    }
    if( bInsert )
    {
        if (rAV.cbTextBefore)
        {
            OUString sP( sTxt.copy( 0, rAV.cbTextBefore ) );
            rNum.SetPrefix( sP );
        }
        if( rAV.cbTextAfter )
        {
            OUString sP( rNum.GetSuffix() );
            sP += sTxt.copy( rAV.cbTextBefore, rAV.cbTextAfter);
            rNum.SetSuffix( sP );
        }



    }
}




void SwWW8ImplReader::SetAnld(SwNumRule* pNumR, WW8_ANLD* pAD, sal_uInt8 nSwLevel,
    bool bOutLine)
{
    SwNumFmt aNF;
    if (pAD)
    {                                                       
        bAktAND_fNumberAcross = 0 != pAD->fNumberAcross;
        WW8_ANLV &rAV = pAD->eAnlv;
        SetBaseAnlv(aNF, rAV, nSwLevel);                    
        SetAnlvStrings(aNF, rAV, pAD->rgchAnld, bOutLine ); 
    }
    pNumR->Set(nSwLevel, aNF);
}







SwNumRule* SwWW8ImplReader::GetStyRule()
{
    if( pStyles->pStyRule )         
        return pStyles->pStyRule;

    const OUString aBaseName("WW8StyleNum");
    const OUString aName( rDoc.GetUniqueNumRuleName( &aBaseName, false) );

    
    sal_uInt16 nRul = rDoc.MakeNumRule( aName, 0, false,
                                    SvxNumberFormat::LABEL_ALIGNMENT );
    pStyles->pStyRule = rDoc.GetNumRuleTbl()[nRul];
    
    pStyles->pStyRule->SetAutoRule(false);

    return pStyles->pStyRule;
}


void SwWW8ImplReader::Read_ANLevelNo( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    nSwNumLevel = 0xff; 

    if( nLen <= 0 )
        return;

    
    if( pAktColl )
    {
        
        
        SwWW8StyInf * pColl = GetStyle(nAktColl);
        if (pColl != NULL && pColl->bColl && *pData)
        {
            

            if (*pData <= MAXLEVEL && *pData <= 9)
            {
                nSwNumLevel = *pData - 1;
                if (!bNoAttrImport)
                    ((SwTxtFmtColl*)pAktColl)->AssignToListLevelOfOutlineStyle( nSwNumLevel );
                    
                    
                    
                    

            }
            else if( *pData == 10 || *pData == 11 )
            {
                
                pStyles->nWwNumLevel = *pData;
            }
        }
    }
    else
    {
        
        if (!bAnl)
            StartAnl(pData);        
        NextAnlLine(pData);
    }
}

void SwWW8ImplReader::Read_ANLevelDesc( sal_uInt16, const sal_uInt8* pData, short nLen ) 
{
    SwWW8StyInf * pStyInf = GetStyle(nAktColl);
    if( !pAktColl || nLen <= 0                       
        || (pStyInf && !pStyInf->bColl)              
        || ( nIniFlags & WW8FL_NO_OUTLINE ) )
    {
        nSwNumLevel = 0xff;
        return;
    }

    if( nSwNumLevel <= MAXLEVEL         
        && nSwNumLevel <= 9 ){          

        
        pAktColl->SetFmtAttr( SwNumRuleItem() );

        const OUString aName("Outline");
        SwNumRule aNR( rDoc.GetUniqueNumRuleName( &aName ),
                       SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
                       OUTLINE_RULE );
        aNR = *rDoc.GetOutlineNumRule();

        SetAnld(&aNR, (WW8_ANLD*)pData, nSwNumLevel, true);

        
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








void SwWW8ImplReader::SetNumOlst(SwNumRule* pNumR, WW8_OLST* pO, sal_uInt8 nSwLevel)
{
    SwNumFmt aNF;
    WW8_ANLV &rAV = pO->rganlv[nSwLevel];
    SetBaseAnlv(aNF, rAV, nSwLevel);
                                            
    int nTxtOfs = 0;
    sal_uInt8 i;
    WW8_ANLV* pAV1;                 
    for (i = 0, pAV1 = pO->rganlv; i < nSwLevel; ++i, ++pAV1)
        nTxtOfs += pAV1->cbTextBefore + pAV1->cbTextAfter;

    if (!bVer67)
        nTxtOfs *= 2;
    SetAnlvStrings(aNF, rAV, pO->rgch + nTxtOfs, true); 
    pNumR->Set(nSwLevel, aNF);
}





void SwWW8ImplReader::Read_OLST( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    delete pNumOlst;
    if (nLen <= 0)
    {
        pNumOlst = 0;
        return;
    }
    pNumOlst = new WW8_OLST;
    if( nLen < sal::static_int_cast< sal_Int32 >(sizeof( WW8_OLST )) )   
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



void SwWW8ImplReader::StartAnl(const sal_uInt8* pSprm13)
{
    bAktAND_fNumberAcross = false;

    sal_uInt8 nT = static_cast< sal_uInt8 >(GetNumType(*pSprm13));
    if (nT == WW8_Pause || nT == WW8_None)
        return;

    nWwNumType = nT;
    SwNumRule *pNumRule = maANLDRules.GetNumRule(nWwNumType);

    
    const sal_uInt8* pS12 = 0;
    OUString sNumRule;

    if (pTableDesc)
    {
        sNumRule = pTableDesc->GetNumRuleName();
        if (!sNumRule.isEmpty())
        {
            pNumRule = rDoc.FindNumRulePtr(sNumRule);
            if (!pNumRule)
                sNumRule = "";
            else
            {
                
                pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E); 
                if (pS12 && 0 != ((WW8_ANLD*)pS12)->fNumberAcross)
                    sNumRule = "";
            }
        }
    }

    SwWW8StyInf * pStyInf = GetStyle(nAktColl);
    if (sNumRule.isEmpty() && pStyInf != NULL &&  pStyInf->bHasStyNumRule)
    {
        sNumRule = pStyInf->pFmt->GetNumRule().GetValue();
        pNumRule = rDoc.FindNumRulePtr(sNumRule);
        if (!pNumRule)
            sNumRule = "";
    }

    if (sNumRule.isEmpty())
    {
        if (!pNumRule)
        {
            
            pNumRule = rDoc.GetNumRuleTbl()[
                            rDoc.MakeNumRule( sNumRule, 0, false,
                                              SvxNumberFormat::LABEL_ALIGNMENT ) ];
        }
        if (pTableDesc)
        {
            if (!pS12)
                pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E); 
            if (!pS12 || !((WW8_ANLD*)pS12)->fNumberAcross)
                pTableDesc->SetNumRuleName(pNumRule->GetName());
        }
    }

    bAnl = true;

    
    pCtrlStck->NewAttr(*pPaM->GetPoint(),
        SfxStringItem(RES_FLTR_NUMRULE, pNumRule->GetName()));

    maANLDRules.SetNumRule(pNumRule, nWwNumType);
}



void SwWW8ImplReader::NextAnlLine(const sal_uInt8* pSprm13)
{
    if (!bAnl)
        return;

    SwNumRule *pNumRule = maANLDRules.GetNumRule(nWwNumType);

    
    

    
    if (*pSprm13 == 10 || *pSprm13 == 11)
    {
        nSwNumLevel = 0;
        if (!pNumRule->GetNumFmt(nSwNumLevel))
        {
            
            
            const sal_uInt8* pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E);
            SetAnld(pNumRule, (WW8_ANLD*)pS12, nSwNumLevel, false);
        }
    }
    else if( *pSprm13 > 0 && *pSprm13 <= MAXLEVEL )          
    {
        nSwNumLevel = *pSprm13 - 1;             
        
        if (!pNumRule->GetNumFmt(nSwNumLevel))
        {
            if (pNumOlst)                       
            {
                
                for (sal_uInt8 nI = 0; nI < nSwNumLevel; ++nI)
                {
                    if (!pNumRule->GetNumFmt(nI))
                        SetNumOlst(pNumRule, pNumOlst, nI);
                }

                SetNumOlst(pNumRule, pNumOlst , nSwNumLevel);
            }
            else                                
            {
                
                const sal_uInt8* pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E);
                SetAnld(pNumRule, (WW8_ANLD*)pS12, nSwNumLevel, false);
            }
        }
    }
    else
        nSwNumLevel = 0xff;                 

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


void WW8TabBandDesc::ReadDef(bool bVer67, const sal_uInt8* pS)
{
    if (!bVer67)
        pS++;

    short nLen = (sal_Int16)SVBT16ToShort( pS - 2 ); 

    sal_uInt8 nCols = *pS;                       
    short nOldCols = nWwCols;

    if( nCols > MAX_COL )
        return;

    nWwCols = nCols;

    const sal_uInt8* pT = &pS[1];
    nLen --;
    int i;
    for(i=0; i<=nCols; i++, pT+=2 )
        nCenter[i] = (sal_Int16)SVBT16ToShort( pT );    
    nLen -= 2 * ( nCols + 1 );
    if( nCols != nOldCols ) 
    {
        delete[] pTCs, pTCs = 0;
        delete[] pSHDs, pSHDs = 0;
        delete[] pNewSHDs, pNewSHDs = 0;
    }

    short nFileCols = nLen / ( bVer67 ? 10 : 20 );  

    if (!pTCs && nCols)
    {
        
        pTCs = new WW8_TCell[nCols];
        setcelldefaults(pTCs,nCols);
    }

    short nColsToRead = nFileCols;
    if (nColsToRead > nCols)
        nColsToRead = nCols;

    if( nColsToRead )
    {
        

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
                {               
                    sal_uInt8 aBits1 = pTc->aBits1Ver6;
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
                        
                        
                        memcpy( pTCs[i-1].rgbrc[ WW8_RIGHT ].aBits1,
                                pTc->rgbrcVer6[  WW8_RIGHT ].aBits1, sizeof( SVBT16 ) );
                            
                            
                            
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
                
                

                
                memcpy( pAktTC->rgbrc, pTc->rgbrcVer8, 4 * sizeof( WW8_BRC ) );
            }
        }

        
        
        
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
    if( pParamsTSetBRC && pTCs ) 
    {
        sal_uInt8 nitcFirst= pParamsTSetBRC[0];
        sal_uInt8 nitcLim  = pParamsTSetBRC[1];
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
    
    if( bVer67 )
    {
        for( int i = 0; i < 6; ++i )
        {
            aDefBrcs[i].aBits1[0] = pParams[   2*i ];
            aDefBrcs[i].aBits1[1] = pParams[ 1+2*i ];
        }
    }
    else 
        memcpy( aDefBrcs, pParams, 24 );
}

void WW8TabBandDesc::ProcessSprmTDxaCol(const sal_uInt8* pParamsTDxaCol)
{
    
    

    if( nWwCols && pParamsTDxaCol ) 
    {
        sal_uInt8 nitcFirst= pParamsTDxaCol[0]; 
        sal_uInt8 nitcLim  = pParamsTDxaCol[1]; 
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
    if( nWwCols && pParamsTInsert )        
    {
        sal_uInt8 nitcInsert = pParamsTInsert[0]; 
        if (nitcInsert >= MAX_COL)  
            return;
        sal_uInt8 nctc  = pParamsTInsert[1];      
        sal_uInt16 ndxaCol = SVBT16ToShort( pParamsTInsert+2 );

        short nNewWwCols;
        if (nitcInsert > nWwCols)
        {
            nNewWwCols = nitcInsert+nctc;
            
            
            if (nNewWwCols > MAX_COL)
            {
                nNewWwCols = MAX_COL;
                nctc = ::sal::static_int_cast<sal_uInt8>(nNewWwCols-nitcInsert);
            }
        }
        else
        {
            nNewWwCols = nWwCols+nctc;
            
            
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

        
        if (nitcInsert <= nWwCols)
        {
            
            nCenter[nWwCols + nctc] = nCenter[nWwCols]+nctc*ndxaCol;
            for( int i = nWwCols-1; i >= nitcInsert; i--)
            {
                
                nCenter[i + nctc] = nCenter[i]+nctc*ndxaCol;

                
                pTCs[i + nctc] = pTCs[i];
            }
        }

        
        for( int i = nWwCols; i > nitcInsert+nWwCols; i--)
            nCenter[i] = i ? (nCenter[i - 1]+ndxaCol) : 0;

        
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
    ++pParams; 
    ++pParams; 

    sal_uInt8 nSideBits = *pParams++;
    OSL_ENSURE(nSideBits < 0x10, "Unexpected value for nSideBits");
    ++pParams; 
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

    ++pParams; 
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
    if( nWwCols && pParamsTDelete )        
    {
        sal_uInt8 nitcFirst= pParamsTDelete[0]; 
        if (nitcFirst >= nWwCols) 
            return;
        sal_uInt8 nitcLim  = pParamsTDelete[1]; 
        if (nitcLim <= nitcFirst) 
            return;

        /*
         * sprmTDelete causes any rgdxaCenter and rgtc entries whose index is
         * greater than or equal to itcLim to be moved
         */
        int nShlCnt  = nWwCols - nitcLim; 

        if (nShlCnt >= 0) 
        {
            WW8_TCell* pAktTC  = pTCs + nitcFirst;
            int i = 0;
            while( i < nShlCnt )
            {
                
                nCenter[nitcFirst + i] = nCenter[nitcLim + i];

                
                *pAktTC = pTCs[ nitcLim + i];

                ++i;
                ++pAktTC;
            }
            
            nCenter[nitcFirst + i] = nCenter[nitcLim + i];
        }

        short nCellsDeleted = nitcLim - nitcFirst;
        
        if (nCellsDeleted > nWwCols)
            nCellsDeleted = nWwCols;
        nWwCols -= nCellsDeleted;
    }
}



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

    short nAnz = nLen / 10; 
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

    
    do
    {
        short nTabeDxaNew      = SHRT_MAX;
        bool bTabRowJustRead   = false;
        const sal_uInt8* pShadeSprm = 0;
        const sal_uInt8* pNewShadeSprm = 0;
        WW8_TablePos *pTabPos  = 0;

        
        if(!(pIo->SearchRowEnd(pPap, nStartCp, pIo->nInTable)))
        {
            bOk = false;
            break;
        }

        
        
        WW8PLCFxDesc aDesc;
        pPap->GetSprms( &aDesc );
        WW8SprmIter aSprmIter(aDesc.pMemPos, aDesc.nSprmsLen, aSprmParser);

        for (int nLoop = 0; nLoop < 2; ++nLoop)
        {
            bool bRepeatedSprm = false;
            const sal_uInt8* pParams;
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
                        if (b0 == 3) 
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

        
        if (
             !pPap->HasFkp() || pPap->Where() == WW8_CP_MAX ||
             aRes.nStartPos == WW8_CP_MAX
           )
        {
            bOk = false;
            break;
        }

        
        const sal_uInt8* pParams = HasTabCellSprm(pPap, bOldVer);
        const sal_uInt8 *pLevel = pPap->HasSprm(0x6649);
        
        if (!pParams || (1 != *pParams) ||
            (pLevel && (*pLevel <= pIo->nInTable)))
        {
            break;
        }

        
        WW8_CP nMyStartCp=nStartCp;
        if (pIo->SearchRowEnd(pPap, nMyStartCp, pIo->nInTable))
            if (pIo->ParseTabPos(&aTabPos, pPap))
                pTabPos = &aTabPos;

        
        aRes.pMemPos = 0;
        aRes.nStartPos = nStartCp;

        
        
        pIo->pPlcxMan->GetPap()->nOrigStartPos = aRes.nStartPos;
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = WW8_CP_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);

        
        
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
            
            delete pNewBand;
            pNewBand = new WW8TabBandDesc( *pActBand ); 
            pActBand->nRows--;      
            pNewBand->nRows = 1;
            pActBand->pNextBand = pNewBand; 
            nBands++;
            pNewBand = 0;                   
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
                    
                    j = i;
                    switch( i )
                    {
                    case 0:
                        
                        j = (pR == pFirstBand) ? 0 : 4;
                        break;
                    case 1:
                        
                        j = k ? 5 : 1;
                        break;
                    case 2:
                        
                        j = pR->pNextBand ? 4 : 2;
                        break;
                    case 3:
                        
                        j = (k == pR->nWwCols - 1) ? 3 : 5;
                        break;
                    }
                    
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

        pR->nTransCell[i++] = (sal_Int8)(j++);  
        pR->nTransCell[i] = (sal_Int8)j;        

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
        (nMinLeft != -108 && bIsBiDi && text::HoriOrientation::RIGHT == eOri)) 
        eOri = text::HoriOrientation::LEFT_AND_WIDTH; 

    nDefaultSwCols = nMinCols;  
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
    ::SetProgressState(pIo->nProgress, pIo->mpDocShell);   

    
    
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
        
        
        SvxFontHeightItem aSz(20, 100, RES_CHRATR_FONTSIZE);
        pIo->NewAttr( aSz );
        pIo->pCtrlStck->SetAttr(*pPoint, RES_CHRATR_FONTSIZE);
    }

    if (bInsNode)
        pIo->AppendTxtNode(*pPoint);

    pTmpPos = new SwPosition( *pIo->pPaM->GetPoint() );

    
    
    
    
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

            
            if (pSetAttr)
            {
                aItemSet.Put(*pSetAttr);
                delete pSetAttr;
            }
        }
    }

    
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
            
            
            
            SwFmtHoriOrient aHori(pIo->pSFlyPara->pFlyFmt->GetHoriOrient());
            sal_Int16 eHori = aHori.GetHoriOrient();
            if ((eHori == text::HoriOrientation::NONE) || (eHori == text::HoriOrientation::LEFT) ||
                (eHori == text::HoriOrientation::LEFT_AND_WIDTH))
            {
                
                
                aHori.SetPos(pIo->pSFlyPara->nXPos + GetMinLeft());
                aHori.SetHoriOrient(text::HoriOrientation::NONE);
                pIo->pSFlyPara->pFlyFmt->SetFmtAttr(aHori);
            }
        }
        else
        {
            
            
            
            
            
            SvxLRSpaceItem aL( RES_LR_SPACE );
            

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
    
    pTabLines = &pTable->GetTabLines();
    nAktRow = nAktCol = nAktBandRow = 0;

    pTblNd  = (SwTableNode*)(*pTabLines)[0]->GetTabBoxes()[0]->
        GetSttNd()->FindTableNode();
    OSL_ENSURE( pTblNd, "wo ist mein TabellenNode" );

    
    if ( nRowsToRepeat == static_cast<sal_uInt16>(nRows) )
        nRowsToRepeat = 1;

    pTblNd->GetTable().SetRowsToRepeat( nRowsToRepeat );
    
    AdjustNewBand();

    WW8DupProperties aDup(pIo->rDoc,pIo->pCtrlStck);
    pIo->pCtrlStck->SetAttr(*pIo->pPaM->GetPoint(), 0, false);

    
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
        
        if( pActBand->pTCs )
        {
            for( short j = 0; j < pActBand->nRows; j++, nRow++ )
                for( short i = 0; i < pActBand->nWwCols; i++ )
                {
                    WW8SelBoxInfo* pActMGroup = 0;
                    //
                    
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
                    

                    bool bMerge = false;
                    if ( rCell.bVertRestart && !rCell.bMerged )
                        bMerge = true;
                    else if (rCell.bFirstMerged && pActBand->bExist[i])
                    {
                        
                        
                        

                        
                        if (rCell.bVertMerge || rCell.bVertRestart)
                            bMerge = true;
                        else
                        {
                            
                            
                            
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

                        
                        pActMGroup = new WW8SelBoxInfo( nX1, nWidth );

                        
                        
                        
                        
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

                        
                        aMergeGroups.push_back(pActMGroup);
                    }

                    
                    
                    UpdateTableMergeGroup( rCell, pActMGroup, pTabBox, i );
                }
            }
    }
}





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

    
    if( !aMergeGroups.empty() )
    {
        
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



//




//
WW8SelBoxInfo* WW8TabDesc::FindMergeGroup(short nX1, short nWidth, bool bExact)
{
    if( !aMergeGroups.empty() )
    {
        
        const short nToleranz = 4;
        
        short nX2 = nX1 + nWidth;
        
        short nGrX1;
        short nGrX2;

        
        for ( short iGr = aMergeGroups.size() - 1; iGr >= 0; --iGr )
        {
            
            WW8SelBoxInfo& rActGroup = aMergeGroups[ iGr ];
            if (!rActGroup.bGroupLocked)
            {
                
                nGrX1 = rActGroup.nGroupXStart - nToleranz;
                nGrX2 = rActGroup.nGroupXStart
                             +rActGroup.nGroupWidth  + nToleranz;
                //
                
                //
                if( ( nX1 > nGrX1 ) && ( nX2 < nGrX2 ) )
                {
                    return &rActGroup;
                }
                //
                
                //
                if( !bExact )
                {
                    
                    if(    (     ( nX1 > nGrX1 )
                                        && ( nX1 < nGrX2 - 2*nToleranz ) )
                            || (     ( nX2 > nGrX1 + 2*nToleranz )
                                        && ( nX2 < nGrX2 ) )
                            
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
            
            
            if (
                 pIo->bParaAutoBefore && pIo->bFirstPara &&
                 !pIo->pWDop->fDontUseHTMLAutoSpacing
               )
            {
                pIo->SetUpperSpacing(*pIo->pPaM, 0);
            }

            
            
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

       
        if(pIo->bParaAutoBefore && pIo->bFirstPara && !pIo->pWDop->fDontUseHTMLAutoSpacing)
            pIo->SetUpperSpacing(*pIo->pPaM, 0);

        
        if(pIo->bParaAutoAfter && !pIo->pWDop->fDontUseHTMLAutoSpacing)
            pIo->SetLowerSpacing(*pIo->pPaM, 0);

        
        
        
        
        if (pIo->pPaM->GetPoint()->nNode != pTabBox->GetSttIdx() + 1)
        {
            pIo->pPaM->GetPoint()->nNode = pTabBox->GetSttIdx() + 1;
            pIo->pPaM->GetPoint()->nContent.Assign(pIo->pPaM->GetCntntNode(), 0);
            
            
            pIo->rDoc.SetTxtFmtColl(*pIo->pPaM, (SwTxtFmtColl*)pIo->pDfltTxtFmtColl);
            
            
        }

        
        if(SwTxtNode *pNd = pIo->pPaM->GetNode()->GetTxtNode())
        {
            const SfxPoolItem &rItm = pNd->SwCntntNode::GetAttr(RES_PARATR_SNAPTOGRID);
            SvxParaGridItem &rSnapToGrid = (SvxParaGridItem&)(rItm);

            if(rSnapToGrid.GetValue())
            {
                SvxParaGridItem aGridItem( rSnapToGrid );
                aGridItem.SetValue(false);

                SwPosition* pGridPos = pIo->pPaM->GetPoint();

                const sal_Int32 nEnd = pGridPos->nContent.GetIndex();
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
    
    
}

void WW8TabDesc::SetTabBorders(SwTableBox* pBox, short nWwIdx)
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 


    SvxBoxItem aFmtBox( RES_BOX );
    if (pActBand->pTCs)     
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
        return;                 

    bool bFound=false;
    if (pActBand->pNewSHDs && pActBand->pNewSHDs[nWwIdx] != COL_AUTO)
    {
        Color aColor(pActBand->pNewSHDs[nWwIdx]);
        pBox->GetFrmFmt()->SetFmtAttr(SvxBrushItem(aColor, RES_BACKGROUND));
        bFound = true;
    }

    
    if (pActBand->pSHDs && !bFound)
    {
        WW8_SHD& rSHD = pActBand->pSHDs[nWwIdx];
        if (!rSHD.GetValue())       
            return;

        SwWW8Shade aSh( pIo->bVer67, rSHD );
        pBox->GetFrmFmt()->SetFmtAttr(SvxBrushItem(aSh.aColor, RES_BACKGROUND));
    }
}

SvxFrameDirection MakeDirection(sal_uInt16 nCode, bool bIsBiDi)
{
    SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
    
    
    
    
    switch (nCode)
    {
        default:
            OSL_ENSURE(eDir == 4, "unknown direction code, maybe it's a bitfield");
        case 3:
            eDir = bIsBiDi ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP; 
            break;
        case 5:
            eDir = FRMDIR_VERT_TOP_RIGHT;
            break;
        case 1:
            eDir = FRMDIR_VERT_TOP_RIGHT;
            break;
        case 4:
            eDir = bIsBiDi ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP; 
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
    if( pActBand->nSwCols > nDefaultSwCols )        
        InsertCells( pActBand->nSwCols - nDefaultSwCols );

    SetPamInCell( 0, false);
    OSL_ENSURE( pTabBoxes && pTabBoxes->size() == (sal_uInt16)pActBand->nSwCols,
        "Falsche Spaltenzahl in Tabelle" );

    if( bClaimLineFmt )
    {
        pTabLine->ClaimFrmFmt();            
        SwFmtFrmSize aF( ATT_MIN_SIZE, 0, 0 );  

        if (pActBand->nLineHeight == 0)    
            aF.SetHeightSizeType( ATT_VAR_SIZE );
        else
        {
            if (pActBand->nLineHeight < 0) 
            {
                aF.SetHeightSizeType(ATT_FIX_SIZE);
                pActBand->nLineHeight = -pActBand->nLineHeight;
            }
            if (pActBand->nLineHeight < MINLAY) 
                pActBand->nLineHeight = MINLAY;

            aF.SetHeight(pActBand->nLineHeight);
        }
        pTabLine->GetFrmFmt()->SetFmtAttr(aF);
    }

    
    
    
    
    
    bool bSetCantSplit = pActBand->bCantSplit;
    if(bSetCantSplit)
        bSetCantSplit = pActBand->bCantSplit90;

    pTabLine->GetFrmFmt()->SetFmtAttr(SwFmtRowSplit(!bSetCantSplit));

    short i;    
    short j;    
    short nW;   
    SwFmtFrmSize aFS( ATT_FIX_SIZE );
    j = pActBand->bLEmptyCol ? -1 : 0;

    for( i = 0; i < pActBand->nSwCols; i++ )
    {
        
        if( j < 0 )
            nW = pActBand->nCenter[0] - nMinLeft;
        else
        {
            
            while ((j < pActBand->nWwCols) && (!pActBand->bExist[j]))
                j++;

            if( j < pActBand->nWwCols )
                nW = pActBand->nCenter[j+1] - pActBand->nCenter[j];
            else
                nW = nMaxRight - pActBand->nCenter[j];
            pActBand->nWidth[ j ] = nW;
        }

        SwTableBox* pBox = (*pTabBoxes)[i];
        
        pBox->ClaimFrmFmt();

        SetTabBorders(pBox, j);

        
        
        
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

        
        while( ( j < pActBand->nWwCols ) && !pActBand->bExist[j] )
        {
            pActBand->nWidth[j] = pActBand->nCenter[j+1] - pActBand->nCenter[j];
            j++;
        }
    }
}

void WW8TabDesc::TableCellEnd()
{
    ::SetProgressState(pIo->nProgress, pIo->mpDocShell);   

    EndMiserableHackForUnsupportedDirection(nAktCol);

    
    if( pIo->bWasTabRowEnd )
    {
        
        

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
            if( nAktRow >= nRows )  
                return;

            bool bNewBand = nAktBandRow >= pActBand->nRows;
            if( bNewBand )
            {                       
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
    {                       
        nAktCol++;
    }
    SetPamInCell(nAktCol, true);

    
    if (pIo->bAnl && !pIo->bAktAND_fNumberAcross)
        pIo->StopAllAnl(IsValidCell(nAktCol));
}


SwTableBox* WW8TabDesc::UpdateTableMergeGroup(  WW8_TCell&     rCell,
                                                WW8SelBoxInfo* pActGroup,
                                                SwTableBox*    pActBox,
                                                sal_uInt16         nCol )
{
    
    SwTableBox* pResult = 0;

    
    
    
    
    
    if ( pActBand->bExist[ nCol ] &&
         ( ( rCell.bFirstMerged && pActGroup ) ||
           rCell.bMerged ||
           rCell.bVertMerge ||
           rCell.bVertRestart ) )
    {
        
        WW8SelBoxInfo* pTheMergeGroup = 0;
        if( pActGroup )
            
            pTheMergeGroup = pActGroup;
        else
        {
            
            pTheMergeGroup = FindMergeGroup(
                pActBand->nCenter[ nCol ], pActBand->nWidth[  nCol ], true );
        }
        if( pTheMergeGroup )
        {
            
            pTheMergeGroup->push_back(pActBox);
            
            pResult = (*pTheMergeGroup)[ 0 ];
        }
    }
    return pResult;
}

sal_uInt16 WW8TabDesc::GetLogicalWWCol() const 
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

bool SwWW8ImplReader::StartTable(WW8_CP nStartCp)
{
    
    bFirstPara = true;
    
    
    if (bReadNoTbl)
        return false;

    if (pTableDesc)
        maTableStack.push(pTableDesc);

    
    
    WW8FlyPara* pTableWFlyPara( 0L );
    WW8SwFlyPara* pTableSFlyPara( 0L );
    
    
    
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
                
                
                pTableSFlyPara = new WW8SwFlyPara(*pPaM, *this, *pTableWFlyPara,
                    maSectionManager.GetWWPageTopMargin(),
                    maSectionManager.GetPageLeft(), maSectionManager.GetTextAreaWidth(),
                    nIniFlyDx, nIniFlyDy);

                
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
            
            
            pTableDesc->pFlyFmt->SetFmtAttr( SwFmtFollowTextFlow( sal_True ) );
        }
        else
            pTableDesc->SetSizePosition(0);
        pTableDesc->UseSwTable();
    }
    else
        PopTableDesc();

    
    delete pTableWFlyPara;
    delete pTableSFlyPara;

    return 0 != pTableDesc;
}

void SwWW8ImplReader::TabCellEnd()
{
    if (nInTable && pTableDesc)
        pTableDesc->TableCellEnd();

    bFirstPara = true;    
    bReadTable = false;
    mpTableEndPaM.reset();
}

void SwWW8ImplReader::Read_TabCellEnd( sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( ( nLen > 0 ) && ( *pData == 1 ) )
        bWasTabCellEnd = true;
}

void SwWW8ImplReader::Read_TabRowEnd( sal_uInt16, const sal_uInt8* pData, short nLen )   
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

    
    
    bFirstPara = false;

    pTableDesc->FinishSwTable();
    PopTableDesc();

    bReadTable = true;
    
    if ( nInTable > 1 )
    {
        mpTableEndPaM.reset(new SwPaM(*pPaM));
    }
}




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

const SwFmt* SwWW8ImplReader::GetStyleWithOrgWWName( OUString& rName ) const
{
    SwFmt* pRet = 0;
    if( !vColl.empty() )
    {
        for(sal_uInt16 nI = 0; nI < pStyles->GetCount(); nI++ )
            if(    vColl[ nI ].bValid
                && (rName.equals( vColl[ nI ].GetOrgWWName())) )
            {
                pRet = vColl[ nI ].pFmt;
                break;
            }
    }
    return pRet;
}





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
        pParaSprms = pSprms;   
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
    if( 0 < nLen ) 
    {
        if (bOdd)
            nLen = nLen - WW8SkipEven( pStStrm );
        else
            nLen = nLen - WW8SkipOdd( pStStrm );

        sal_Int16 cbUPX(0);
        pStStrm->ReadInt16( cbUPX );

        nLen-=2;

        if ( cbUPX > nLen )
            cbUPX = nLen;       

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
                sal_Size nPos = pStStrm->Tell(); 
                                                 
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

    if( bPara ) 
        nLen = ImportUPX(nLen, true, bOdd);
    ImportUPX(nLen, false, bOdd);                   
}

WW8RStyle::WW8RStyle(WW8Fib& _rFib, SwWW8ImplReader* pI)
    : WW8Style(*pI->pTableStream, _rFib), maSprmParser(_rFib.GetFIBVersion()),
    pIo(pI), pStStrm(pI->pTableStream), pStyRule(0), nWwNumLevel(0)
{
    pIo->vColl.resize(cstd);
}

void WW8RStyle::Set1StyleDefaults()
{
    
    if (!bCJKFontChanged)   
        pIo->SetNewFontAttr(ftcFE, true, RES_CHRATR_CJK_FONT);

    if (!bCTLFontChanged)   
        pIo->SetNewFontAttr(ftcBi, true, RES_CHRATR_CTL_FONT);

    
    if (!bFontChanged)      
        pIo->SetNewFontAttr(ftcAsci, true, RES_CHRATR_FONT);

    if( !pIo->bNoAttrImport )
    {
        
        if ( !bTxtColChanged )
            pIo->pAktColl->SetFmtAttr(SvxColorItem(Color(COL_AUTO), RES_CHRATR_COLOR));

        
        if( !bFSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            pIo->pAktColl->SetFmtAttr(aAttr);
            aAttr.SetWhich(RES_CHRATR_CJK_FONTSIZE);
            pIo->pAktColl->SetFmtAttr(aAttr);
        }

        
        if( !bFCTLSizeChanged )
        {
            SvxFontHeightItem aAttr(200, 100, RES_CHRATR_FONTSIZE);
            aAttr.SetWhich(RES_CHRATR_CTL_FONTSIZE);
            pIo->pAktColl->SetFmtAttr(aAttr);
        }

        if( !bWidowsChanged )  
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
        
        sw::util::ParaStyleMapper::StyleResult aResult =
            pIo->maParaStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }
    else
    {
        
        sw::util::CharStyleMapper::StyleResult aResult =
            pIo->maCharStyleMapper.GetStyle(rSI.GetOrgWWName(), eSti);
        pColl = aResult.first;
        bStyExist = aResult.second;
    }

    bool bImport = !bStyExist || pIo->mbNewDoc; 
    bool bOldNoImp = pIo->bNoAttrImport;
    rSI.bImportSkipped = !bImport;

    if( !bImport )
        pIo->bNoAttrImport = true;
    else
    {
        if (bStyExist)
        {
            pColl->ResetAllFmtAttr(); 
        }
        pColl->SetAuto(false);          
    }                                   
    pIo->pAktColl = pColl;
    rSI.pFmt = pColl;                  
    rSI.bImportSkipped = !bImport;

    
    sal_uInt16 j = rSI.nBase;
    if (j != nThisStyle && j < cstd )
    {
        SwWW8StyInf* pj = &pIo->vColl[j];
        if (rSI.pFmt && pj->pFmt && rSI.bColl == pj->bColl)
        {
            rSI.pFmt->SetDerivedFrom( pj->pFmt );  
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

    rSI.nFollow = nNextStyle;       

    pStyRule = 0;                   
    bTxtColChanged = bFontChanged = bCJKFontChanged = bCTLFontChanged =
        bFSizeChanged = bFCTLSizeChanged = bWidowsChanged = false;
    pIo->SetNAktColl( nThisStyle );
    pIo->bStyNormal = nThisStyle == 0;
    return bOldNoImp;
}

void WW8RStyle::PostStyle(SwWW8StyInf &rSI, bool bOldNoImp)
{
    

    pIo->bHasBorder = pIo->bShdTxtCol = pIo->bCharShdTxtCol
        = pIo->bSpec = pIo->bObj = pIo->bSymbol = false;
    pIo->nCharFmt = -1;

    
    if ((rSI.nBase >= cstd || pIo->vColl[rSI.nBase].bImportSkipped) && rSI.bColl)
    {
        
        
        Set1StyleDefaults();
    }

    pStyRule = 0;                   
    pIo->bStyNormal = false;
    pIo->SetNAktColl( 0 );
    pIo->bNoAttrImport = bOldNoImp;
    
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

    rSI.bImported = true;                      
                                                //
    

    if( rSI.nBase < cstd && !pIo->vColl[rSI.nBase].bImported )
        Import1Style( rSI.nBase );

    pStStrm->Seek( rSI.nFilePos );

    short nSkip, cbStd;
    OUString sName;

    boost::scoped_ptr<WW8_STD> xStd(Read1Style(nSkip, &sName, &cbStd));

    if (xStd)
        rSI.SetOrgWWIdent( sName, xStd->sti );

    

    if ( !xStd || sName.isEmpty() || ((1 != xStd->sgc) && (2 != xStd->sgc)) )
    {
        pStStrm->SeekRel( nSkip );
        return;
    }

    bool bOldNoImp = PrepareStyle(rSI, static_cast<ww::sti>(xStd->sti), nNr, xStd->istdNext);

    
    long nPos = pStStrm->Tell();

    
    
    
    
    

    
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

void WW8RStyle::ScanStyles()        
{                                   
    for (sal_uInt16 i = 0; i < cstd; ++i)
    {
        short nSkip;
        SwWW8StyInf &rSI = pIo->vColl[i];

        rSI.nFilePos = pStStrm->Tell();        
        WW8_STD* pStd = Read1Style( nSkip, 0, 0 );  
        rSI.bValid = (0 != pStd);
        if (rSI.bValid)
        {
            rSI.nBase = pStd->istdBase;        
            rSI.bColl = ( pStd->sgc == 1 );    
        }
        else
            rSI = SwWW8StyInf();

        delete pStd;
        pStStrm->SeekRel( nSkip );              
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
        aChpx.fFldVanish = (nFlags8 & 0x10) >> 4;
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
        pIo->vColl[i].bColl = true;
        
        pIo->vColl[i].nBase = 222;
    }

    rtl_TextEncoding eStructChrSet = WW8Fib::GetFIBCharset(
        pIo->pWwFib->chseTables);

    sal_uInt16 cstcStd;
    rSt.ReadUInt16( cstcStd );

    sal_uInt16 cbName;
    rSt.ReadUInt16( cbName );
    sal_uInt16 nByteCount = 2;
    sal_uInt16 stcp=0;
    while (nByteCount < cbName)
    {
        sal_uInt8 nCount;
        rSt.ReadUChar( nCount );
        nByteCount++;

        sal_uInt8 stc = static_cast< sal_uInt8 >((stcp - cstcStd) & 255);
        if (stc >=pIo->vColl.size())
            continue;

        SwWW8StyInf &rSI = pIo->vColl[stc];
        if (nCount != 0xFF)    
        {
            OUString sName;
            if (nCount == 0)   
            {
                ww::sti eSti = ww::GetCanonicalStiFromStc(stc);
                if (const sal_Char *pStr = GetEnglishNameFromSti(eSti))
                    sName = OUString(pStr, strlen(pStr), RTL_TEXTENCODING_ASCII_US);
                else
                    sName = "Unknown";
            }
            else               
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
                OUString sName = OUString(pStr, strlen(pStr), RTL_TEXTENCODING_ASCII_US);
                rSI.SetOrgWWIdent(sName, stc);
            }
        }
        stcp++;
    }

    sal_uInt16 nStyles=stcp;

    std::vector<pxoffset> aCHPXOffsets(stcp);
    sal_uInt16 cbChpx;
    rSt.ReadUInt16( cbChpx );
    nByteCount = 2;
    stcp=0;
    std::vector< std::vector<sal_uInt8> > aConvertedChpx;
    while (nByteCount < cbChpx)
    {
        sal_uInt8 cb;
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

        stcp++;
        if (stcp == nStyles)
    {
            rSt.SeekRel(cbChpx-nByteCount);
            nByteCount += cbChpx-nByteCount;
    }
    }

    std::vector<pxoffset> aPAPXOffsets(stcp);
    sal_uInt16 cbPapx;
    rSt.ReadUInt16( cbPapx );
    nByteCount = 2;
    stcp=0;
    while (nByteCount < cbPapx)
    {
        sal_uInt8 cb;
        rSt.ReadUChar( cb );
        nByteCount++;

        aPAPXOffsets[stcp].mnSize = 0;

        if (cb != 0xFF)
        {
            sal_uInt8 stc2;
            rSt.ReadUChar( stc2 );
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
    rSt.ReadUInt16( iMac );

    if (iMac > nStyles) iMac = nStyles;

    for (stcp = 0; stcp < iMac; ++stcp)
    {
        sal_uInt8 stcNext, stcBase;
        rSt.ReadUChar( stcNext );
        rSt.ReadUChar( stcBase );

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
    ScanStyles();                       

    for (sal_uInt16 i = 0; i < cstd; ++i) 
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
        
        SwWW8StyInf* pi = &pIo->vColl[i];
        sal_uInt16 j = pi->nFollow;
        if( j < cstd )
        {
            SwWW8StyInf* pj = &pIo->vColl[j];
            if ( j != i                             
                 && pi->pFmt                        
                 && pj->pFmt                        
                 && pi->bColl                       
                 && pj->bColl ){                    
                    ( (SwTxtFmtColl*)pi->pFmt )->SetNextTxtFmtColl(
                     *(SwTxtFmtColl*)pj->pFmt );    
            }
        }
    }

    
    
    
    
    
    
    //
    

    if( pIo->StyleExists(0) && !pIo->vColl.empty() &&
        pIo->vColl[0].pFmt && pIo->vColl[0].bColl && pIo->vColl[0].bValid )
        pIo->pDfltTxtFmtColl = (SwTxtFmtColl*)pIo->vColl[0].pFmt;
    else
        pIo->pDfltTxtFmtColl = pIo->rDoc.GetDfltTxtFmtColl();

    
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

    
    pIo->pAktColl = 0;
}

rtl_TextEncoding SwWW8StyInf::GetCharSet() const
{
    if ((pFmt) && (pFmt->GetFrmDir().GetValue() == FRMDIR_HORI_RIGHT_TOP))
        return eRTLFontSrcCharSet;
    return eLTRFontSrcCharSet;
}

rtl_TextEncoding SwWW8StyInf::GetCJKCharSet() const
{
    if ((pFmt) && (pFmt->GetFrmDir().GetValue() == FRMDIR_HORI_RIGHT_TOP))
        return eRTLFontSrcCharSet;
    return eCJKFontSrcCharSet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
