/*************************************************************************
 *
 *  $RCSfile: ww8par2.cxx,v $
 *
 *  $Revision: 1.89 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:43:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */


#pragma hdrstop

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _SV_FONTTYPE_HXX
#include <vcl/fonttype.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif

#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>            // class SwTxtNode
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>           // SwNumRuleItem
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>          // RES_POOLCOLL_STANDARD
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>          // class SwTableLines, ...
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>           // class _SwSelBox
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>         // fuer den Attribut Stack
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif

#ifndef _WW8STRUC_HXX
#include "ww8struc.hxx"         // struct TC
#endif
#ifndef _WW8PAR_HXX
#include "ww8par.hxx"
#endif
#ifndef _WW8PAR2_HXX
#include "ww8par2.hxx"
#endif

#define MAX_COL 64  // WW6-Beschreibung: 32, WW6-UI: 31 & WW8-UI: 63!

class WW8SelBoxInfo: public SwSelBoxes_SAR
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

typedef WW8SelBoxInfo* WW8SelBoxInfoPtr;

SV_DECL_PTRARR_DEL(WW8MergeGroups, WW8SelBoxInfoPtr, 16,16)
SV_IMPL_PTRARR(WW8MergeGroups, WW8SelBoxInfoPtr)

struct WW8TabBandDesc
{
    WW8TabBandDesc* pNextBand;
    short nGapHalf;
    short mnDefaultLeft;
    short mnDefaultTop;
    short mnDefaultRight;
    short mnDefaultBottom;
    bool mbHasSpacing;
    short nLineHeight;
    short nRows;
    short nCenter[MAX_COL + 1]; // X-Rand aller Zellen dieses Bandes
    short nWidth[MAX_COL + 1];  // Laenge aller Zellen dieses Bandes
    short nWwCols;      // BYTE wuerde reichen, alignment -> short
    short nSwCols;      // SW: so viele Spalten fuer den Writer
    bool bLEmptyCol;    // SW: Links eine leere Zusatz-Spalte
    bool bREmptyCol;    // SW: dito rechts
    WW8_TCell* pTCs;
    BYTE nOverrideSpacing[MAX_COL + 1];
    short nOverrideValues[MAX_COL + 1][4];
    WW8_SHD* pSHDs;
    sal_uInt32* pNewSHDs;
    WW8_BRC aDefBrcs[6];


    // nur fuer WW6-7: diese Zelle hat WW-Flag bMerged (horizontal) gesetzt
    //bool bWWMergedVer6[MAX_COL];


    bool bExist[MAX_COL];           // Existiert diese Zelle ?
    UINT8 nTransCell[MAX_COL + 2];  // UEbersetzung WW-Index -> SW-Index

    WW8TabBandDesc() {  memset( this, 0, sizeof( *this ) ); };
    WW8TabBandDesc( WW8TabBandDesc& rBand );    // tief kopieren
    ~WW8TabBandDesc() { delete[] pTCs; delete[] pSHDs; delete[] pNewSHDs;};
    static void setcelldefaults(WW8_TCell *pCells, short nCells);
    void ReadDef(bool bVer67, const BYTE* pS);
    void ProcessSprmTSetBRC(bool bVer67, const BYTE* pParamsTSetBRC);
    void ProcessSprmTDxaCol(const BYTE* pParamsTDxaCol);
    void ProcessSprmTDelete(const BYTE* pParamsTDelete);
    void ProcessSprmTInsert(const BYTE* pParamsTInsert);
    void ProcessSpacing(const BYTE* pParamsTInsert);
    void ProcessSpecificSpacing(const BYTE* pParamsTInsert);
    void ReadShd(const BYTE* pS );
    void ReadNewShd(const BYTE* pS, bool bVer67);

    enum wwDIR {wwTOP = 0, wwLEFT = 1, wwBOTTOM = 2, wwRIGHT = 3};
};

class WW8TabDesc
{
    ::std::vector<String> aNumRuleNames;

    SwWW8ImplReader* pIo;

    WW8TabBandDesc* pFirstBand;
    WW8TabBandDesc* pActBand;

    SwPosition* pTmpPos;

    SwTableNode* pTblNd;            // Tabellen-Node
    const SwTableLines* pTabLines;  // Zeilen-Array davon
    SwTableLine* pTabLine;          // akt. Zeile
    SwTableBoxes* pTabBoxes;        // Boxen-Array in akt. Zeile
    SwTableBox* pTabBox;            // akt. Zelle

    WW8MergeGroups* pMergeGroups;   // Listen aller zu verknuepfenden Zellen

    WW8_TCell* pAktWWCell;

    short nRows;
    short nDefaultSwCols;
    short nBands;
    short nMinLeft;
    short nConvertedLeft;
    short nMaxRight;
    short nSwWidth;

    bool bOk;
    bool bHeader;
    bool bClaimLineFmt;
    SwHoriOrient eOri;
    bool bIsBiDi;
                                // 2. allgemeine Verwaltungsinfo
    short nAktRow;
    short nAktBandRow;          // SW: in dieser Zeile des akt. Bandes bin ich
                                // 3. Verwaltungsinfo fuer Writer
    short nAktCol;


    // 4. Methoden

    USHORT GetLogicalWWCol() const;
    void SetTabBorders( SwTableBox* pBox, short nIdx );
    void SetTabShades( SwTableBox* pBox, short nWwIdx );
    void SetTabVertAlign( SwTableBox* pBox, short nWwIdx );
    void CalcDefaults();
    bool SetPamInCell(short nWwCol, bool bPam);
    void InsertCells( short nIns );
    void AdjustNewBand();

    // durchsucht pMergeGroups, meldet Index der ersten, passenden Gruppe bzw.
    // -1 Details siehe bei der Implementierung
    bool FindMergeGroup(short nX1, short nWidth, bool bExact, short& nMGrIdx);

    // einzelne Box ggfs. in eine Merge-Gruppe aufnehmen
    // (die Merge-Gruppen werden dann spaeter auf einen Schlag abgearbeitet)
    SwTableBox* UpdateTableMergeGroup(WW8_TCell& rCell,
        WW8SelBoxInfo* pActGroup, SwTableBox* pActBox, USHORT nCol  );
    //No copying
    WW8TabDesc(const WW8TabDesc&);
    WW8TabDesc &operator=(const WW8TabDesc&);
public:
    const SwTable* pTable;          // Tabelle
    SwPosition* pParentPos;
    SwFlyFrmFmt* pFlyFmt;
    SfxItemSet aItemSet;
    bool IsValidCell(short nCol) const;

    WW8TabDesc( SwWW8ImplReader* pIoClass, WW8_CP nStartCp );
    bool Ok() const { return bOk; }
    void CreateSwTable();
    void UseSwTable();
    void SetSizePosition(SwFrmFmt* pFrmFmt);
    void TableCellEnd();
    void FinishSwTable();
    void MergeCells();
    short GetMinLeft() const { return nConvertedLeft; }
    ~WW8TabDesc();
    SwPosition *GetPos() { return pTmpPos; }

    const WW8_TCell* GetAktWWCell() const { return pAktWWCell; }
    const short GetAktCol() const { return nAktCol; }
    // find name of numrule valid for current WW-COL
    const String& GetNumRuleName() const;
    void SetNumRuleName( const String& rName );
};

void wwSectionManager::SetCurrentSectionHasFootnote()
{
    ASSERT(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if (!maSegments.empty())
        maSegments.back().mbHasFootnote = true;
}

bool wwSectionManager::CurrentSectionIsVertical() const
{
    ASSERT(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if (!maSegments.empty())
        return maSegments.back().IsVertical();
    return false;
}

short wwSectionManager::GetPageLeft() const
{
    return !maSegments.empty() ? maSegments.back().nPgLeft : 0;
}

short wwSectionManager::GetPageRight() const
{
    return !maSegments.empty() ? maSegments.back().nPgRight : 0;
}

short wwSectionManager::GetPageWidth() const
{
    return !maSegments.empty() ? maSegments.back().nPgWidth : 0;
}

sal_uInt16 SwWW8ImplReader::End_Ftn()
{
    /*
    #84095#
    Ignoring Footnote outside of the normal Text. People will put footnotes
    into field results and field commands.
    */
    if (bIgnoreText ||
        pPaM->GetPoint()->nNode < rDoc.GetNodes().GetEndOfExtras().GetIndex())
    {
        return 0;
    }

    ASSERT(!maFtnStack.empty(), "footnote end without start");
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
        sChar.Append(pTxt->GetTxt().GetChar(--nPos));
        pPaM->SetMark();
        pPaM->GetMark()->nContent--;
        rDoc.Delete( *pPaM );
        pPaM->DeleteMark();
        SwFmtFtn aFtn(rDesc.meType == MAN_EDN);
        pFN = pTxt->Insert(aFtn, nPos, nPos);
    }
    ASSERT(pFN, "Probleme beim Anlegen des Fussnoten-Textes");
    if (pFN)
    {

        SwPosition aTmpPos( *pPaM->GetPoint() );    // merke alte Cursorposition
        WW8PLCFxSaveAll aSave;
        pPlcxMan->SaveAllPLCFx( aSave );
        WW8PLCFMan* pOldPlcxMan = pPlcxMan;

        const SwNodeIndex* pSttIdx = ((SwTxtFtn*)pFN)->GetStartNode();
        ASSERT(pSttIdx, "Probleme beim Anlegen des Fussnoten-Textes");

        ((SwTxtFtn*)pFN)->SetSeqNo( rDoc.GetFtnIdxs().Count() );

        bool bOld = bFtnEdn;
        bFtnEdn = true;

        // read content of Ft-/End-Note
        Read_HdFtFtnText( pSttIdx, rDesc.mnStartCp, rDesc.mnLen, rDesc.meType);
        bFtEdOk = true;
        bFtnEdn = bOld;

        ASSERT(sChar.Len()==1 && ((rDesc.mbAutoNum == (sChar.GetChar(0) == 2))),
         "footnote autonumbering must be 0x02, and everthing else must not be");

        // If no automatic numbering use the following char from the main text
        // as the footnote number
        if (!rDesc.mbAutoNum)
            ((SwTxtFtn*)pFN)->SetNumber(0, &sChar);

        /*
            Delete the footnote char from the footnote if its at the beginning
            as usual. Might not be if the user has already deleted it, e.g.
            #i14737#
        */
        SwNodeIndex& rNIdx = pPaM->GetPoint()->nNode;
        rNIdx = pSttIdx->GetIndex() + 1;
        SwTxtNode* pTNd = rNIdx.GetNode().GetTxtNode();
        if (pTNd && pTNd->GetTxt().Len() && sChar.Len())
        {
            if (pTNd->GetTxt().GetChar(0) == sChar.GetChar(0))
            {
                pPaM->GetPoint()->nContent.Assign( pTNd, 0 );
                pPaM->SetMark();
                pPaM->GetMark()->nContent++;
                rDoc.Delete( *pPaM );
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
    #84095#
    Ignoring Footnote outside of the normal Text. People will put footnotes
    into field results and field commands.
    */
    if (bIgnoreText ||
        pPaM->GetPoint()->nNode < rDoc.GetNodes().GetEndOfExtras().GetIndex())
    {
        return 0;
    }

    FtnDescriptor aDesc;
    bool bAutoNum = true;
    if( eEDN == pRes->nSprmId )
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

    while
    (
        (
         (bVer67 && !pWwFib->fComplex) ||
         (pPap->GetPCDIdx() < pPap->GetPCDIMax())
        ) &&
        pPap->HasFkp()
    )
    {
        if (pPap->Where() != LONG_MAX)
        {
            const BYTE* pB = pPap->HasSprm(TabRowSprm(nLevel));
            if (pB && *pB == 1)
            {
                const BYTE *pLevel = 0;
                if ((pLevel = pPap->HasSprm(0x6649)))
                {
                    if (nLevel + 1 == *pLevel)
                        return true;
                }
                else
                {
                    ASSERT(!nLevel || pLevel, "sublevel without level sprm");
                    return true;    // RowEnd found
                }
            }
        }

        aRes.nStartPos = aRes.nEndPos;
        aRes.pMemPos = 0;
        //Seek to our next block of properties
        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = LONG_MAX;
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
    WW8_TablePos *pTabPos)
{
    ApoTestResults aRet;
    // Frame in Style Definition (word appears to ignore them if inside an
    // text autoshape, e.g. #94418#)
    if (!bTxbxFlySection)
        aRet.mpStyleApo = StyleExists(nAktColl) ? pCollA[nAktColl].pWWFly : 0;

    /*
    #i1140#
    If I have a table and apply a style to one of its frames that should cause
    a paragraph that its applied to it to only exist as a seperate floating
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

    aRet.mpSprm37 = pPlcxMan->HasParaSprm( bVer67 ? 37 : 0x2423 );
    aRet.mpSprm29 = pPlcxMan->HasParaSprm( bVer67 ? 29 : 0x261B );

    // Is there some frame data here
    bool bNowApo = aRet.HasFrame() || pTabPos;

    bool bTestAllowed = !bTxbxFlySection && !bTableRowEnd;
    if (bTestAllowed)
    {
        if (nCellLevel == nInTable)
        {
            bTestAllowed =
                (!(nInTable && pTableDesc && pTableDesc->GetAktCol()));
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
    if (bNowApo && !bTableRowEnd && InLocalApo())
    {
        // two bordering eachother
        if (!TestSameApo(aRet, pTabPos))
            aRet.mbStopApo = aRet.mbStartApo = true;
    }

    return aRet;
}
//---------------------------------------------------------------------
//   Hilfroutinen fuer Kapitelnummerierung und Aufzaehlung / Gliederung
//---------------------------------------------------------------------

static void SetBaseAnlv(SwNumFmt &rNum, WW8_ANLV &rAV, BYTE nSwLevel )
{
    static SvxExtNumType eNumA[8] = { SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC };

    static SvxAdjust eAdjA[4] = { SVX_ADJUST_LEFT,
        SVX_ADJUST_RIGHT, SVX_ADJUST_LEFT, SVX_ADJUST_LEFT };
//          eigentlich folgende 2, aber Writer-UI bietet es nicht an
//      SVX_ADJUST_CENTER, SVX_ADJUST_BLOCKLINE };

    rNum.SetNumberingType(( SVBT8ToByte( rAV.nfc ) < 8 ) ?
                    eNumA[SVBT8ToByte( rAV.nfc ) ] : SVX_NUM_NUMBER_NONE);
    if ((SVBT8ToByte(rAV.aBits1 ) & 0x4) >> 2)
        rNum.SetIncludeUpperLevels(nSwLevel + 1);
    rNum.SetStart( SVBT16ToShort( rAV.iStartAt ) );
//  rNum.eNumAdjust = eAdjA[rAV.jc];
    rNum.SetNumAdjust( eAdjA[SVBT8ToByte( rAV.aBits1 ) & 0x3] );

    rNum.SetCharTextDistance( SVBT16ToShort( rAV.dxaSpace ) );
    INT16 nIndent = Abs((INT16)SVBT16ToShort( rAV.dxaIndent ));
    if( SVBT8ToByte( rAV.aBits1 ) & 0x08 )      //fHang
    {
        rNum.SetFirstLineOffset( -nIndent );
        rNum.SetLSpace( nIndent );
        rNum.SetAbsLSpace( nIndent );
    }
    else
    {
        rNum.SetCharTextDistance( nIndent );        // Breite der Nummer fehlt
    }
    if( SVBT8ToByte( rAV.nfc ) == 5 || SVBT8ToByte( rAV.nfc ) == 7 )
    {
        String sP( rNum.GetSuffix() );
        sP.Insert( '.', 0 );
        rNum.SetSuffix( sP );   // Ordinalzahlen
    }
}

void SwWW8ImplReader::SetAnlvStrings(SwNumFmt &rNum, WW8_ANLV &rAV,
    const BYTE* pTxt, bool bOutline)
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
    {                             // Gliederung
        if( !rNum.GetIncludeUpperLevels()           // es sind  <= 1 Nummern anzuzeigen
            || rNum.GetNumberingType() == SVX_NUM_NUMBER_NONE ){    // oder dieser Level hat keine
                                                // eigenen Ziffern
            bInsert = true;                     // -> dann uebernehme Zeichen

            // replace by simple Bullet ?
            if( bListSymbol )
                //JP 14.08.96: cBulletChar benutzen, damit auf dem MAC
                //              richtig gemappt wird
                sTxt.Fill(  SVBT8ToByte( rAV.cbTextBefore )
                          + SVBT8ToByte( rAV.cbTextAfter  ), cBulletChar );
            }
    }
    else
    {                                       // Nummerierung / Aufzaehlung
        bInsert = true;
//      if( SVBT16ToShort( rAV.ftc ) == 1
//          || SVBT16ToShort( rAV.ftc ) == 3 ){ // Symbol / WingDings
        if( bListSymbol )
        {
            FontFamily eFamily;
            String aName;
            FontPitch ePitch;

            if( GetFontParams( SVBT16ToShort( rAV.ftc ), eFamily, aName,
                                ePitch, eCharSet ) ){
//              USHORT nSiz = ( SVBT16ToShort( rAV.hps ) ) ?
//                          SVBT16ToShort( rAV.hps ) : 24; // Groesse in 1/2 Pt
//                      darf nach JP nicht gesetzt werden, da immer die Size
//                      genommen wird, die am ZeilenAnfang benutzt wird
                Font aFont;
                aFont.SetName( aName );
                aFont.SetFamily( eFamily );
//              aFont.SetPitch( ePitch );       // darf nach JP nicht
                aFont.SetCharSet( eCharSet );
                rNum.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
//              if( rAV.ico )       // geht in UI und SWG-Writer/Reader nicht
//                  aFont.SetColor( Color( GetCol( rAV.ico ) ) );
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
// Die Zeichen vor und hinter mehreren Ziffern koennen leider nicht uebernommen
// werden, da sie der Writer ganz anders behandelt und das Ergebnis i.A.
// schlechter als ohne waere.
    }
}

// SetAnld bekommt einen WW-ANLD-Descriptor und einen Level und modifiziert
// die durch pNumR anggebeben NumRules. Wird benutzt fuer alles ausser
// Gliederung im Text
void SwWW8ImplReader::SetAnld(SwNumRule* pNumR, WW8_ANLD* pAD, BYTE nSwLevel,
    bool bOutLine)
{
    SwNumFmt aNF;
    if (pAD)
    {                                   // Es gibt einen Anld-Sprm
        bAktAND_fNumberAcross = 0 != SVBT8ToByte( pAD->fNumberAcross );
        WW8_ANLV &rAV = pAD->eAnlv;
        SetBaseAnlv(aNF, rAV, nSwLevel);        // Setze Basis-Format
        SetAnlvStrings(aNF, rAV, pAD->rgchAnld, bOutLine );// und Rest
    }
    pNumR->Set(nSwLevel, aNF);
}



//-------------------------------------------------------
//          Kapitelnummerierung und Kapitelbullets
//-------------------------------------------------------
// Kapitelnummerierung findet in Styledefinionen statt. Sprm 13 gibt den Level
// an, Sprm 12 den Inhalt

SwNumRule* SwWW8ImplReader::GetStyRule()
{
    if( pStyles->pStyRule )         // Bullet-Style bereits vorhanden
        return pStyles->pStyRule;

    const String aBaseName(CREATE_CONST_ASC( "WW8StyleNum" ));
    const String aName( rDoc.GetUniqueNumRuleName( &aBaseName, false) );

    USHORT nRul = rDoc.MakeNumRule( aName );
    pStyles->pStyRule = rDoc.GetNumRuleTbl()[nRul];
    // Auto == false-> Nummerierungsvorlage
    pStyles->pStyRule->SetAutoRule(false);

    return pStyles->pStyRule;
}

// Sprm 13
void SwWW8ImplReader::Read_ANLevelNo( USHORT, const BYTE* pData, short nLen )
{
    nSwNumLevel = 0xff; // Default: ungueltig

    if( nLen <= 0 )
        return;

    // StyleDef ?
    if( pAktColl )
    {
        // nur fuer SwTxtFmtColl, nicht CharFmt
        // WW: 0 = no Numbering
        if (pCollA[nAktColl].bColl && *pData)
        {
            // Bereich WW:1..9 -> SW:0..8 keine Aufzaehlung / Nummerierung

            if (*pData <= MAXLEVEL && *pData <= 9)
            {
                nSwNumLevel = *pData - 1;
                if (!bNoAttrImport)
                    ((SwTxtFmtColl*)pAktColl)->SetOutlineLevel( nSwNumLevel );
                    // Bei WW-NoNumbering koennte auch NO_NUMBERING gesetzt
                    // werden. ( Bei normaler Nummerierung muss NO_NUM gesetzt
                    // werden: NO_NUM : Nummerierungs-Pause,
                    // NO_NUMBERING : ueberhaupt keine Nummerierung )

            }
            else if( *pData == 10 || *pData == 11 )
            {
                // Typ merken, der Rest geschieht bei Sprm 12
                pStyles->nWwNumLevel = *pData;
            }
        }
    }
    else
    {
        //Not StyleDef
        if (!bAnl && !(nIniFlags & WW8FL_NO_NUMRULE))
            StartAnl(pData);        // Anfang der Gliederung / Aufzaehlung
        NextAnlLine(pData);
    }
}

void SwWW8ImplReader::Read_ANLevelDesc( USHORT, const BYTE* pData, short nLen ) // Sprm 12
{
    if( !pAktColl || nLen <= 0                  // nur bei Styledef
        || !pCollA[nAktColl].bColl              // CharFmt -> ignorieren
        || ( nIniFlags & WW8FL_NO_OUTLINE ) ){
        nSwNumLevel = 0xff;
        return;
    }
    if( nSwNumLevel <= MAXLEVEL         // Bereich WW:1..9 -> SW:0..8
        && nSwNumLevel <= 9 ){      // keine Aufzaehlung / Nummerierung

                                        // Falls bereits direkt oder durch
                                        // Vererbung NumruleItems gesetzt sind,
                                        // dann jetzt ausschalten #56163
        pAktColl->SetAttr( SwNumRuleItem() );

        String aName(CREATE_CONST_ASC( "Outline" ));
        SwNumRule aNR( rDoc.GetUniqueNumRuleName(&aName), OUTLINE_RULE );
        aNR = *rDoc.GetOutlineNumRule();

        SetAnld(&aNR, (WW8_ANLD*)pData, nSwNumLevel, true);

            // fehlende Level muessen nicht aufgefuellt werden

        rDoc.SetOutlineNumRule( aNR );
    }else if( pStyles->nWwNumLevel == 10 || pStyles->nWwNumLevel == 11 ){
        SwNumRule* pNR = GetStyRule();
        SetAnld(pNR, (WW8_ANLD*)pData, 0, false);
        pAktColl->SetAttr( SwNumRuleItem( pNR->GetName() ) );
        pCollA[nAktColl].bHasStyNumRule = true;
    }
}

//-----------------------------------------
//      Nummerierung / Aufzaehlung
//-----------------------------------------

// SetNumOlst() traegt die Numrules fuer diese Zeile ins SwNumFmt ein
// ( nur fuer Gliederungen im Text; Aufzaehlungen / Nummerierungen laufen
// ueber ANLDs )
// dabei wird die Info aus dem OLST geholt und nicht aus dem ANLD ( s.u. )
void SwWW8ImplReader::SetNumOlst(SwNumRule* pNumR, WW8_OLST* pO, BYTE nSwLevel)
{
    SwNumFmt aNF;
    WW8_ANLV &rAV = pO->rganlv[nSwLevel];
    SetBaseAnlv(aNF, rAV, nSwLevel);
                                            // ... und then the Strings
    int nTxtOfs = 0;
    BYTE i;
    register WW8_ANLV* pAV1;                 // search String-Positions
    for (i = 0, pAV1 = pO->rganlv; i < nSwLevel; ++i, ++pAV1)
    {
        nTxtOfs += SVBT8ToByte(pAV1->cbTextBefore)
            + SVBT8ToByte(pAV1->cbTextAfter);
    }

    if (!bVer67)
        nTxtOfs *= 2;
    SetAnlvStrings(aNF, rAV, pO->rgch + nTxtOfs, true); // und rein
    pNumR->Set(nSwLevel, aNF);
}




// der OLST kommt am Anfang jeder Section, die Gliederungen enthaelt. Die ANLDs,
// die an jeder Gliederungszeile haengen, enthalten nur Stuss, also werden die
// OLSTs waehrend der Section gemerkt, damit die Informationen beim Auftreten
// von Gliederungsabsaetzen zugreifbar ist.
void SwWW8ImplReader::Read_OLST( USHORT, const BYTE* pData, short nLen )
{
    if( nIniFlags & WW8FL_NO_NUMRULE )
        return;
    if( nLen <= 0 ){
        DELETEZ( pNumOlst );
        return;
    }
    if( pNumOlst )
        delete pNumOlst;                    // nur sicherheitshalber
    pNumOlst = new WW8_OLST;
    if( nLen < sizeof( WW8_OLST ) )         // auffuellen, falls zu kurz
        memset( pNumOlst, 0, sizeof( *pNumOlst ) );
    *pNumOlst = *(WW8_OLST*)pData;
}

WW8LvlType GetNumType(BYTE nWwLevelNo)
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

// StartAnl wird am Anfang eines Zeilenbereichs gerufen,
//  der Gliederung / Nummerierung / Aufzaehlung enthaelt
void SwWW8ImplReader::StartAnl( const BYTE* pSprm13 )
{
    bAktAND_fNumberAcross = false;

    BYTE nT = GetNumType( *pSprm13 );
    if( nT == WW8_Pause || nT == WW8_None )
        return;

    nWwNumType = nT;

    // check for COL numbering:
    const BYTE* pS12 = 0;// sprmAnld
    String sNumRule;
    if( pTableDesc )
    {
        sNumRule = pTableDesc->GetNumRuleName();
        if( sNumRule.Len() )
        {
            mpNumRule = rDoc.FindNumRulePtr( sNumRule );
            if( !mpNumRule )
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

    if (!sNumRule.Len() && pCollA[nAktColl].bHasStyNumRule)
    {
        sNumRule = pCollA[nAktColl].pFmt->GetNumRule().GetValue();
        mpNumRule = rDoc.FindNumRulePtr(sNumRule);
        if (!mpNumRule)
            sNumRule.Erase();
    }

    if (!sNumRule.Len())
    {
        if (!mpNumRule)
            mpNumRule = rDoc.GetNumRuleTbl()[rDoc.MakeNumRule(sNumRule)];
        if( pTableDesc )
        {
            if (!pS12)
                pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E); // sprmAnld
            if (!pS12 || !SVBT8ToByte( ((WW8_ANLD*)pS12)->fNumberAcross))
                pTableDesc->SetNumRuleName( mpNumRule->GetName() );
        }
    }

    bAnl = true;

    // NumRules ueber Stack setzen
    pCtrlStck->NewAttr(*pPaM->GetPoint(),
            SfxStringItem(RES_FLTR_NUMRULE, mpNumRule->GetName()));
}


// NextAnlLine() wird fuer jede Zeile einer
// Gliederung / Nummerierung / Aufzaehlung einmal gerufen
void SwWW8ImplReader::NextAnlLine(const BYTE* pSprm13)
{
    if (!bAnl)
        return;

    // pNd->UpdateNum ohne Regelwerk gibt GPF spaetestens beim Speichern als
    // sdw3

    // WW:10 = Nummerierung -> SW:0 & WW:11 = Auffzaehlung -> SW:0
    if (*pSprm13 == 10 || *pSprm13 == 11)
    {
        nSwNumLevel = 0;
        if (!mpNumRule->GetNumFmt(nSwNumLevel))
        {
            // noch nicht definiert
            // sprmAnld o. 0
            const BYTE* pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E);
            SetAnld(mpNumRule, (WW8_ANLD*)pS12, nSwNumLevel, false);
        }
    }
    else if (*pSprm13 <= MAXLEVEL)          // Bereich WW:1..9 -> SW:0..8
    {
        nSwNumLevel = *pSprm13 - 1;             // Gliederung
        // noch nicht definiert
        if (!mpNumRule->GetNumFmt(nSwNumLevel))
        {
            if (pNumOlst)                       // es gab ein OLST
            {
                //Assure upper levels are set, #i9556#
                for (BYTE nI = 0; nI < nSwNumLevel; ++nI)
                {
                    if (!mpNumRule->GetNumFmt(nI))
                        SetNumOlst(mpNumRule, pNumOlst, nI);
                }

                SetNumOlst(mpNumRule, pNumOlst , nSwNumLevel);
            }
            else                                // kein Olst, nimm Anld
            {
                // sprmAnld
                const BYTE* pS12 = pPlcxMan->HasParaSprm(bVer67 ? 12 : 0xC63E);
                SetAnld(mpNumRule, (WW8_ANLD*)pS12, nSwNumLevel, false);
            }
        }
    }
    else
        nSwNumLevel = 0xff;                 // keine Nummer

    BYTE nLevel = (nSwNumLevel < MAXLEVEL) ? nSwNumLevel : NO_NUM;
    SwNodeNum aNum(nLevel);
    SwTxtNode* pNd = pPaM->GetNode()->GetTxtNode();
    pNd->UpdateNum(aNum);
}

// StopAnl() wird am Ende des Zeilenbereiches gerufen
void SwWW8ImplReader::StopAnl(bool bGoBack)
{
    if( bGoBack )
    {
        SwPosition aTmpPos( *pPaM->GetPoint() );    // eine Zeile zu spaet
        // gehe zurueck
        pPaM->Move( fnMoveBackward, fnGoCntnt );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_NUMRULE ); // Setze NumRules in Stack
        // restore Pam
        *pPaM->GetPoint() = aTmpPos;
    }
    else
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_NUMRULE ); // Setze NumRules in Stack

    nSwNumLevel = 0xff;
    nWwNumType = WW8_None;
    bAnl = false;
    mpNumRule = 0;
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

// ReadDef liest die Zellenpositionen und ggfs die Umrandungen eines Bandes ein
void WW8TabBandDesc::ReadDef(bool bVer67, const BYTE* pS)
{
    if (!bVer67)
        pS++;

    short nLen = (INT16)SVBT16ToShort( pS - 2 ); // nicht schoen
    BYTE nCols = *pS;                       // Anzahl der Zellen
    short nOldCols = nWwCols;

    if( nCols > MAX_COL )
        return;

    nWwCols = nCols;

    const BYTE* pT = &pS[1];
    nLen --;
    int i;
    for(i=0; i<=nCols; i++, pT+=2 )
        nCenter[i] = (INT16)SVBT16ToShort( pT );    // X-Raender
    nLen -= 2 * ( nCols + 1 );
    if( nCols != nOldCols ) // andere Spaltenzahl
    {
        delete[] pTCs, pTCs = 0;
        delete[] pSHDs, pSHDs = 0;
        delete[] pNewSHDs, pNewSHDs = 0;
    }

    short nFileCols = nLen / ( bVer67 ? 10 : 20 );  // wirklich abgespeichert

    if (!pTCs && nCols)
    {
        // lege leere TCs an
        pTCs = new WW8_TCell[nCols];
        setcelldefaults(pTCs,nCols);
    }

    if( nFileCols )
    {
        // lies TCs ein

        /*
            Achtung: ab Ver8 ist ein reserve-ushort je TC eingefuegt und auch
                     der Border-Code ist doppelt so gross, daher ist hier
                             kein simples kopieren moeglich,
                             d.h.: pTCs[i] = *pTc;  geht leider nicht.
            ---
            Vorteil: Arbeitstruktur ist jetzt viel bequemer zu handhaben!
        */
        WW8_TCell* pAktTC  = pTCs;
        if( bVer67 )
        {
            WW8_TCellVer6* pTc = (WW8_TCellVer6*)pT;
            for(i=0; i<nFileCols; i++, ++pAktTC,++pTc)
            {
                if( i < nFileCols )
                {               // TC aus File ?
                    BYTE aBits1 = SVBT8ToByte( pTc->aBits1Ver6 );
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
                        // Cell gemerged -> merken
                        //bWWMergedVer6[i] = true;
                        memcpy( pTCs[i-1].rgbrc[ WW8_RIGHT ].aBits1,
                                pTc->rgbrcVer6[  WW8_RIGHT ].aBits1, sizeof( SVBT16 ) );
                            // right Border in vorige Zelle uebernehmen
                            // Hier darf bExist nicht auf false gesetzt werden, da WW
                            // in den Textboxen diese Zellen nicht mitzaehlt....
                    }
                }
            }
        }
        else
        {
            WW8_TCellVer8* pTc = (WW8_TCellVer8*)pT;
            for(int i=0; i<nFileCols; i++, ++pAktTC, ++pTc )
            {
                UINT16 aBits1 = SVBT16ToShort( pTc->aBits1Ver8 );
                pAktTC->bFirstMerged    = ( ( aBits1 & 0x0001 ) != 0 );
                pAktTC->bMerged         = ( ( aBits1 & 0x0002 ) != 0 );
                pAktTC->bVertical       = ( ( aBits1 & 0x0004 ) != 0 );
                pAktTC->bBackward       = ( ( aBits1 & 0x0008 ) != 0 );
                pAktTC->bRotateFont     = ( ( aBits1 & 0x0010 ) != 0 );
                pAktTC->bVertMerge      = ( ( aBits1 & 0x0020 ) != 0 );
                pAktTC->bVertRestart    = ( ( aBits1 & 0x0040 ) != 0 );
                pAktTC->nVertAlign      = ( ( aBits1 & 0x0180 ) >> 7 );
                // am Rande: im aBits1 verstecken sich noch 7 Reserve-Bits,
                //           anschliessend folgen noch 16 weitere Reserve-Bits

                // In Version 8 koennen wir alle Bordercodes auf einmal kopieren!
                memcpy( pAktTC->rgbrc, pTc->rgbrcVer8, 4 * sizeof( WW8_BRC ) );
            }
        }
    }
}

void WW8TabBandDesc::ProcessSprmTSetBRC(bool bVer67, const BYTE* pParamsTSetBRC)
{
    if( pParamsTSetBRC && pTCs ) // set one or more cell border(s)
    {
        BYTE nitcFirst= pParamsTSetBRC[0];// first col to be changed
        BYTE nitcLim  = pParamsTSetBRC[1];// (last col to be changed)+1
        BYTE nFlag    = *(pParamsTSetBRC+2);

        bool bChangeRight  = (nFlag & 0x08) ? true : false;
        bool bChangeBottom = (nFlag & 0x04) ? true : false;
        bool bChangeLeft   = (nFlag & 0x02) ? true : false;
        bool bChangeTop    = (nFlag & 0x01) ? true : false;

        WW8_TCell* pAktTC  = pTCs + nitcFirst;
        if( bVer67 )
        {
            WW8_BRCVer6* pBRC = (WW8_BRCVer6*)(pParamsTSetBRC+3);

            for( int i = nitcFirst; i < nitcLim; i++, ++pAktTC )
            {
                if( bChangeTop )
                    memcpy( pAktTC->rgbrc[ WW8_TOP  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                if( bChangeLeft )
                    memcpy( pAktTC->rgbrc[ WW8_LEFT ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                if( bChangeBottom )
                    memcpy( pAktTC->rgbrc[ WW8_BOT  ].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
                if( bChangeRight )
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT].aBits1,
                            pBRC->aBits1,
                            sizeof( SVBT16 ) );
            }
        }
        else
        {
            WW8_BRC* pBRC = (WW8_BRC*)(pParamsTSetBRC+3);

            for( int i = nitcFirst; i < nitcLim; i++, ++pAktTC )
            {
                if( bChangeTop )
                    memcpy( pAktTC->rgbrc[ WW8_TOP  ].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
                if( bChangeLeft )
                    memcpy( pAktTC->rgbrc[ WW8_LEFT ].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
                if( bChangeBottom )
                    memcpy( pAktTC->rgbrc[ WW8_BOT  ].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
                if( bChangeRight )
                    memcpy( pAktTC->rgbrc[ WW8_RIGHT].aBits1,
                            pBRC->aBits1,
                            sizeof( WW8_BRC ) );
            }



        }
    }
}


void WW8TabBandDesc::ProcessSprmTDxaCol(const BYTE* pParamsTDxaCol)
{
    // sprmTDxaCol (opcode 0x7623) changes the width of cells
    // whose index is within a certain range to be a certain value.

    if( nWwCols && pParamsTDxaCol ) // set one or more cell length(s)
    {
        BYTE nitcFirst= pParamsTDxaCol[0]; // first col to be changed
        BYTE nitcLim  = pParamsTDxaCol[1]; // (last col to be changed)+1
        short nDxaCol = (INT16)SVBT16ToShort( pParamsTDxaCol + 2 );
        short nOrgWidth;
        short nDelta;

        for( int i = nitcFirst; (i < nitcLim) && (i < nWwCols); i++ )
        {
            nOrgWidth  = nCenter[i+1] - nCenter[i];
            nDelta     = nDxaCol - nOrgWidth;
            for( int j = i+1; j <= nWwCols; j++ )
            {
                nCenter[j] += nDelta;
            }
        }
    }
}

void WW8TabBandDesc::ProcessSprmTInsert(const BYTE* pParamsTInsert)
{
    if( nWwCols && pParamsTInsert )        // set one or more cell length(s)
    {
        BYTE nitcInsert = pParamsTInsert[0]; // position at which to insert
        BYTE nctc  = pParamsTInsert[1];      // number of cells
        USHORT ndxaCol = SVBT16ToShort( pParamsTInsert+2 );

        short nNewWwCols;
        if (nitcInsert > nWwCols)
            nNewWwCols = nitcInsert+nctc;
        else
            nNewWwCols = nWwCols+nctc;

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

void WW8TabBandDesc::ProcessSpacing(const BYTE* pParams)
{
    BYTE nLen = pParams ? *(pParams - 1) : 0;
    ASSERT(nLen == 6, "Unexpected spacing len");
    if (nLen != 6)
        return;
    mbHasSpacing=true;
    BYTE nWhichCell = *pParams++;
    ASSERT(nWhichCell == 0, "Expected cell to be 0!");
    *pParams++; //unknown byte

    BYTE nSideBits = *pParams++;
    ASSERT(nSideBits < 0x10, "Unexpected value for nSideBits");
    *pParams++; //unknown byte
    USHORT nValue =  SVBT16ToShort( pParams );
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
                ASSERT(!this, "Impossible");
                break;
        }
    }
}

void WW8TabBandDesc::ProcessSpecificSpacing(const BYTE* pParams)
{
    BYTE nLen = pParams ? *(pParams - 1) : 0;
    ASSERT(nLen == 6, "Unexpected spacing len");
    if (nLen != 6)
        return;
    BYTE nWhichCell = *pParams++;
    ASSERT(nWhichCell < nWwCols, "Cell out of range in spacings");
    if (nWhichCell >= nWwCols)
        return;

    *pParams++; //unknown byte
    BYTE nSideBits = *pParams++;
    ASSERT(nSideBits < 0x10, "Unexpected value for nSideBits");
    nOverrideSpacing[nWhichCell] |= nSideBits;

    ASSERT(nOverrideSpacing[nWhichCell] < 0x10,
        "Unexpected value for nSideBits");
    BYTE nUnknown2 = *pParams++;
    ASSERT(nUnknown2 == 0x3, "Unexpected value for spacing2");
    USHORT nValue =  SVBT16ToShort( pParams );

    for (int i=0; i < 4; i++)
    {
        if (nSideBits & (1 << i))
            nOverrideValues[nWhichCell][i] = nValue;
    }
}

void WW8TabBandDesc::ProcessSprmTDelete(const BYTE* pParamsTDelete)
{
    if( nWwCols && pParamsTDelete )        // set one or more cell length(s)
    {
        BYTE nitcFirst= pParamsTDelete[0]; // first col to be deleted
        BYTE nitcLim  = pParamsTDelete[1]; // (last col to be deleted)+1

        BYTE nShlCnt  = nWwCols - nitcLim; // count of cells to be shifted


        WW8_TCell* pAktTC  = pTCs + nitcFirst;
        int i = 0;
        for( ; i < nShlCnt; i++, ++pAktTC )
        {
            // adjust the left x-position
            nCenter[nitcFirst + i] = nCenter[nitcLim + i];

            // adjust the cell's borders
            *pAktTC = pTCs[ nitcLim + i];
        }
        // adjust the left x-position of the dummy at the very end
        nCenter[nitcFirst + i] = nCenter[nitcLim + i];

        nWwCols -= (nitcLim - nitcFirst);
    }
}

// ReadShd liest ggfs die Hintergrundfarben einer Zeile ein.
// Es muss vorher ReadDef aufgerufen worden sein
void WW8TabBandDesc::ReadShd(const BYTE* pS )
{
    BYTE nLen = pS ? *(pS - 1) : 0;
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

void WW8TabBandDesc::ReadNewShd(const BYTE* pS, bool bVer67)
{
    BYTE nLen = pS ? *(pS - 1) : 0;
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
#if 0
    //Theres the possibility that it should be something like this
    for (int i=0;i<nCols;++i)
    {
        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[0].aBits1);
        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[0].aBits2);

        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[1].aBits1);
        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[1].aBits2);

        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[2].aBits1);
        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[2].aBits2);

        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[3].aBits1);
        ShortToSVBT16(0xFFFF,pCells[i].rgbrc[3].aBits2);
    }
#endif
}

const BYTE *HasTabCellSprm(WW8PLCFx_Cp_FKP* pPap, bool bVer67)
{
    const BYTE *pParams;
    if (bVer67)
        pParams = pPap->HasSprm(24);
    else
    {
        if (!(pParams = pPap->HasSprm(0x244B)))
            pParams = pPap->HasSprm(0x2416);
    }
    return pParams;
}

WW8TabDesc::WW8TabDesc(SwWW8ImplReader* pIoClass, WW8_CP nStartCp)
    : pIo(pIoClass), pFirstBand(0), pActBand(0), pTmpPos(0), pTblNd(0),
    pTabLines(0), pTabLine(0), pTabBoxes(0), pTabBox(0), pMergeGroups(0),
    pAktWWCell(0), nRows(0), nDefaultSwCols(0), nBands(0), nMinLeft(0),
    nConvertedLeft(0), nMaxRight(0), nSwWidth(0), bOk(true), bHeader(false),
    bClaimLineFmt(false), eOri(HORI_NONE), bIsBiDi(false), nAktRow(0),
    nAktBandRow(0), nAktCol(0), pTable(0), pParentPos(0), pFlyFmt(0),
    aItemSet(pIo->rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    pIo->bAktAND_fNumberAcross = false;

    static const SwHoriOrient aOriArr[] =
    {
        HORI_LEFT, HORI_CENTER, HORI_RIGHT, HORI_CENTER
    };

    bool bVer67   = pIo->bVer67;
    bool bComplex = pIo->pWwFib->fComplex;
    WW8_TablePos aTabPos;

    WW8PLCFxSave1 aSave;
    pIo->pPlcxMan->GetPap()->Save( aSave );

    WW8PLCFx_Cp_FKP* pPap = pIo->pPlcxMan->GetPapPLCF();

    eOri = HORI_LEFT;

       WW8TabBandDesc* pNewBand = new WW8TabBandDesc;

    wwSprmParser aSprmParser(pIo->GetFib().nVersion);

    // process pPap until end of table found
    do
    {
        short nTabeDxaNew      = SHRT_MAX;
        bool bTabRowJustRead   = false;
        const BYTE* pShadeSprm = 0;
        const BYTE* pNewShadeSprm = 0;
        WW8_TablePos *pTabPos  = 0;

        // Suche Ende einer TabZeile
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

        const BYTE* pParams = aSprmIter.GetAktParams();
        for( int nLoop = 0; nLoop < 2; ++nLoop )
        {
            while ( aSprmIter.GetSprms() &&
                (0 != (pParams = aSprmIter.GetAktParams())) )
            {
                switch( aSprmIter.GetAktId() )
                {
                case 187:
                case 0xD605:
                    // sprmTTableBorders
                    if( bVer67 )
                    {
                        for( int i = 0; i < 6; ++i )
                        {
                            pNewBand->aDefBrcs[i].aBits1[0] = pParams[   2*i ];
                            pNewBand->aDefBrcs[i].aBits1[1] = pParams[ 1+2*i ];
                        }
                    }
                    else // aDefBrcs = *(BRC(*)[6])pS;
                        memcpy( pNewBand->aDefBrcs, pParams, 24 );
                    break;
                case 186:
                case 0x3404:
                    if( nRows == 0 )  // only 1. Line is Header
                    {
                        // sprmTTableHeader
                        bHeader = true;
                    }
                    break;
                case 182:
                case 0x5400:
                    if( nRows == 0 )
                    {
                        // sprmTJc  -  Justification Code
                        eOri = aOriArr[ *pParams & 0x3 ];
                    }
                    break;
                case 0x560B:
                    bIsBiDi = SVBT16ToShort(pParams) ? true : false;
                    break;
                case 184:
                case 0x9602:
                    {
                        // sprmTDxaGapHalf
                        pNewBand->nGapHalf = (INT16)SVBT16ToShort( pParams );
                    }
                    break;
                case 189:
                case 0x9407:
                    {
                        // sprmTDyaRowHeight
                        pNewBand->nLineHeight = (INT16)SVBT16ToShort( pParams );
                        bClaimLineFmt = true;
                    }
                    break;
                case 190:
                case 0xD608:
                    {
                        // DefineTabRow
                        pNewBand->ReadDef( bVer67, pParams );
                        bTabRowJustRead = true;
                    }
                    break;
                case 191:
                case 0xD609:
                    // TableShades
                    pShadeSprm = pParams;
                    break;
                case 0xD612:
                    pNewShadeSprm = pParams;
                    break;
                //
                // * * * * Bug #69885#: the following codes were added * * * *
                //                      (khz, 1.Feb.2000)
                case 183:
                case 0x9601:
                    {
                        // sprmTDxaLeft
                        //
                        // our Writer cannot shift single table lines
                        // horizontally so we have to find the smallest
                        // parameter (meaning the left-most position) and then
                        // shift the whole table to that margin (see below)
                        short nDxaNew = (INT16)SVBT16ToShort( pParams );
                        if( nDxaNew < nTabeDxaNew )
                            nTabeDxaNew = nDxaNew;
                    }
                    break;
                case 193:
                case 0xD620:
                    {
                        // sprmTSetBrc
                        pNewBand->ProcessSprmTSetBRC( bVer67, pParams );
                    }
                    break;
                case 196:
                case 0x7623:
                    {
                        // sprmTDxaCol
                        pNewBand->ProcessSprmTDxaCol(pParams);
                    }
                    break;
                case 194:
                case 0x7621:
                    {
                        // sprmTInsert
                        pNewBand->ProcessSprmTInsert(pParams);
                    }
                    break;
                case 195:
                case 0x5622:
                    {
                        // sprmTDelete
                        pNewBand->ProcessSprmTDelete(pParams);
                    }
                    break;
                case 0xD634:
                    pNewBand->ProcessSpacing(pParams);
                    break;
                case 0xD632:
                    pNewBand->ProcessSpecificSpacing(pParams);
                    break;
                }
                aSprmIter++;
            }

            if( !nLoop )
            {
                pPap->GetPCDSprms(  aDesc );
                aSprmIter.SetSprms( aDesc.pMemPos, aDesc.nSprmsLen );
            }
        }

        // #55171: WW-Tabellen koennen Fly-Wechsel beinhalten daher hier
        // Tabellen abbrechen und neu beginnen noch steht *pPap noch vor
        // TabRowEnd, daher kann TestApo() mit letztem Parameter false und
        // damit wirksam gerufen werden.

        if (bTabRowJustRead)
        {
            if (pShadeSprm)
                pNewBand->ReadShd(pShadeSprm);
            if (pNewShadeSprm)
                pNewBand->ReadNewShd(pNewShadeSprm, bVer67);
        }

        if( nTabeDxaNew < SHRT_MAX )
        {
            short* pCenter  = pNewBand->nCenter;
            for( int i = 0; i < pNewBand->nWwCols; i++, ++pCenter )
                *pCenter += nTabeDxaNew; // Shift left x-position
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
            aRes.nEndPos = LONG_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);

        //Are we at the end of available properties
        if (
            (pPap->Where() == LONG_MAX) ||
            (
             (!bVer67 || bComplex) && (pPap->GetPCDIdx() >= pPap->GetPCDIMax())
            )
           )
        {
            bOk = false;
            break;
        }

        //Are we still in a table cell
        pParams = HasTabCellSprm(pPap, pIo->bVer67);
        const BYTE *pLevel = pPap->HasSprm(0x6649);
        // InTable
        if (!pParams || (1 != *pParams) ||
            (pLevel && (*pLevel <= pIo->nInTable)))
        {
            break;
        }

        //Get the end of row new table positioning data
        WW8_CP nMyStartCp=nStartCp;
        if (pIo->SearchRowEnd(pPap, nMyStartCp, pIo->nInTable))
            if (SwWW8ImplReader::ParseTabPos(&aTabPos, pPap))
                pTabPos = &aTabPos;

        //Move back to this cell
        aRes.pMemPos = 0;
        aRes.nStartPos = nStartCp;

        if (!(pPap->SeekPos(aRes.nStartPos)))
        {
            aRes.nEndPos = LONG_MAX;
            pPap->SetDirty(true);
        }
        pPap->GetSprms(&aRes);
        pPap->SetDirty(false);

        //Does this row match up with the last row closely enough to be
        //considered part of the same table
        ApoTestResults aApo = pIo->TestApo(pIo->nInTable, false, pTabPos);

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
            // Letztes Band hat mehr als 1 Zeile
            delete pNewBand;
            pNewBand = new WW8TabBandDesc( *pActBand ); // neues machen
            pActBand->nRows--;      // wegen Sonderbehandlung Raender-Defaults
            pNewBand->nRows = 1;
            pActBand->pNextBand = pNewBand; // am Ende einschleifen
            nBands++;
            pNewBand = 0;                   // nicht loeschen
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
    delete pMergeGroups;
}

void WW8TabDesc::CalcDefaults()
{
    short nMinCols = SHRT_MAX;
    register WW8TabBandDesc* pR;

    nMinLeft = SHRT_MAX;
    nMaxRight = SHRT_MIN;

    /*
    #101175#
    If we are an honestly inline centered table, then the normal rules of
    engagement for left and right margins do not apply. The multiple rows are
    centered regardless of the actual placement of rows, so we cannot have
    mismatched rows as is possible in other configurations.

    e.g. change the example bugdoc in word from text wrapping of none (inline)
    to around (in frame (bApo)) and the table splits into two very disjoint
    rows as the beginning point of each row are very different
    */
    if ((!pIo->InLocalApo()) && (eOri == HORI_CENTER))
    {
        for (pR = pFirstBand; pR; pR = pR->pNextBand)
            for( short i = pR->nWwCols; i >= 0; --i)
                pR->nCenter[i] -=  pR->nCenter[0];
    }

    // 1. Durchlauf: aeusserste L- und R-Grenzen finden
    for( pR = pFirstBand; pR; pR = pR->pNextBand )
    {
        if( pR->nCenter[0] < nMinLeft )
            nMinLeft = pR->nCenter[0];

        for( short i = 0; i < pR->nWwCols; i++ )
        {
           /*
            #74387# If the margins are so large as to make the displayable
            area inside them smaller than the minimum allowed then adjust the
            width to fit. But only do it if the two cells are not the exact
            same value, if they are then the cell does not really exist and will
            be blended together into the same cell through the use of the
            nTrans(late) array.
            */
            if (
                (pR->nCenter[i+1] - pR->nCenter[i]) &&
                ((pR->nCenter[i+1] - pR->nCenter[i] - pR->nGapHalf*2) < MINLAY)
               )
            {
                pR->nCenter[i+1] = pR->nCenter[i]+MINLAY+pR->nGapHalf * 2;
            }
        }

        if( pR->nCenter[pR->nWwCols] > nMaxRight )
            nMaxRight = pR->nCenter[pR->nWwCols];
    }
    nSwWidth = nMaxRight - nMinLeft;

    // #109830# If the table is right aligned we need to align all rows to the
    // row that has the furthest right point

    if(eOri == HORI_RIGHT)
    {
        for( pR = pFirstBand; pR; pR = pR->pNextBand )
        {
            int adjust = nMaxRight - pR->nCenter[pR->nWwCols];
            for( short i = 0; i < pR->nWwCols + 1; i++ )
            {
                pR->nCenter[i] += adjust;
            }

        }
    }

    // 2. Durchlauf: Zahl der Writer-Spalten feststellen Die Zahl der Writer
    // Spalten kann um bis zu 2 hoeher sein als im WW, da der SW im Gegensatz
    // zu WW keine ausgefransten linken und rechten Raender kann und diese
    // durch leere Boxen aufgefuellt werden.  Durch nichtexistente Zellen
    // koennen auch Zellen wegfallen

        // 3. Durchlauf: Wo noetig die Umrandungen durch die Defaults ersetzen
    nConvertedLeft = nMinLeft;

    short nLeftMaxThickness = 0, nRightMaxThickness=0;
    for( pR = pFirstBand ; pR; pR = pR->pNextBand )
    {
        if( !pR->pTCs )
        {
            pR->pTCs = new WW8_TCell[ pR->nWwCols ];
            memset( pR->pTCs, 0, pR->nWwCols * sizeof( WW8_TCell ) );
        }
        for(int k = 0; k < pR->nWwCols; k++ )
        {
            register WW8_TCell* pT = &pR->pTCs[k];
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
                        // Aussen oben  / Innen waagerecht
                        j = (pR == pFirstBand) ? 0 : 4;
                        break;
                    case 1:
                        // Aussen links / Innen senkrecht
                        j = k ? 5 : 1;
                        break;
                    case 2:
                        // Aussen unten / Innen waagerecht
                        j = pR->pNextBand ? 4 : 2;
                        break;
                    case 3:
                        // Aussen rechts/ Innen senkrecht
                        j = (k == pR->nWwCols - 1) ? 3 : 5;
                        break;
                    }
                    // mangel mit Defaults ueber
                    pT->rgbrc[i] = pR->aDefBrcs[j];
                }
            }
        }
        /*
        Similiar to graphics and other elements word does not totally
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
            pR->nCenter[pR->nWwCols] += nThickness;
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
    nSwWidth += nRightMaxThickness;
    nMaxRight += nRightMaxThickness;
    nConvertedLeft = nMinLeft-(nLeftMaxThickness/2);

    for( pR = pFirstBand; pR; pR = pR->pNextBand )
    {
        pR->nSwCols = pR->nWwCols;
        pR->bLEmptyCol = pR->nCenter[0] - nMinLeft >= MINLAY;
        pR->bREmptyCol = nMaxRight - pR->nCenter[pR->nWwCols] >= MINLAY;

        short nAddCols = pR->bLEmptyCol + pR->bREmptyCol;
        USHORT i;
        USHORT j = ( pR->bLEmptyCol ) ? 1 : 0;
        for( i = 0; i < pR->nWwCols; i++ )
        {
            pR->nTransCell[i] = (INT8)j;
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

        ASSERT(i,"no columns in row ?");

        /*
        #96345#
        If the last cell was "false" then there is no valid cell following it,
        so the default mapping forward wont't work. So map it (and
        contigious invalid cells backwards to the last valid cell instead.
        */
        if (i && pR->bExist[i-1] == false)
        {
            USHORT k=i-1;
            while (k && pR->bExist[k] == false)
                k--;
            for (USHORT n=k+1;n<i;n++)
                pR->nTransCell[n] = pR->nTransCell[k];
        }

        pR->nTransCell[i++] = (INT8)(j++);  // Wird u.a. wegen bREmptyCol um
        pR->nTransCell[i] = (INT8)j;        // max. 2 ueberindiziert

        pR->nSwCols += nAddCols;
        if( pR->nSwCols < nMinCols )
            nMinCols = pR->nSwCols;
    }

    if (nMinLeft && (HORI_LEFT == eOri))
        eOri = HORI_LEFT_AND_WIDTH; //  absolutely positioned

    nDefaultSwCols = nMinCols;  // da Zellen einfuegen billiger ist als Mergen
    if( nDefaultSwCols == 0 )
        bOk = false;
    pActBand = pFirstBand;
    nAktBandRow = 0;
    ASSERT( pActBand, "pActBand ist 0" );
}

void WW8TabDesc::SetSizePosition(SwFrmFmt* pFrmFmt)
{
    SwFrmFmt* pApply = pFrmFmt;
    if (!pApply )
        pApply = pTable->GetFrmFmt();
    ASSERT(pApply,"No frame");
    pApply->SetAttr(aItemSet);
    if (pFrmFmt)
    {
        SwFmtFrmSize aSize = pFrmFmt->GetFrmSize();
        aSize.SetSizeType(ATT_MIN_SIZE);
        aSize.SetHeight(MINLAY);
        pFrmFmt->SetAttr(aSize);
        pTable->GetFrmFmt()->SetAttr(SwFmtHoriOrient(0,HORI_FULL));
    }
}

void wwSectionManager::PrependedInlineNode(const SwPosition &rPos,
    const SwNode &rNode)
{
    ASSERT(!maSegments.empty(),
        "should not be possible, must be at least one segment");
    if ((!maSegments.empty()) && (maSegments.back().maStart == rPos.nNode))
        maSegments.back().maStart = SwNodeIndex(rNode);
}

void WW8TabDesc::CreateSwTable()
{
    ::SetProgressState( pIo->nProgress, pIo->rDoc.GetDocShell() );   // Update

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
            pIo->pFmtOfJustInsertedApo->SetAttr(aSur);
        }
    }

    if (bSetMinHeight == true)
    {
        // minimize Fontsize to minimize height growth of the header/footer
        // set font size to 1 point to minimize y-growth of Hd/Ft
        SvxFontHeightItem aSz(20);
        pIo->NewAttr( aSz );
        pIo->pCtrlStck->SetAttr(*pPoint, RES_CHRATR_FONTSIZE);
    }

    if (bInsNode)
        pIo->AppendTxtNode(*pPoint);

    pTmpPos = new SwPosition( *pIo->pPaM->GetPoint() );

    // Die Tabelle ist beim Einfuegen noch recht klein: Zahl der Spalten ist
    // die kleinste Spaltenanzahl des Originals, da Spalten einfuegen
    // schneller geht als Loeschen Zahl der Zeilen ist die Zahl der Baender,
    // da sich die (identischen) Zeilen eines Bandes prima duplizieren lassen
    pTable = pIo->rDoc.InsertTable( *pTmpPos, nBands, nDefaultSwCols, eOri );

    ASSERT(pTable, "insert table failed");
    if (!pTable)
        return;

    SwTableNode* pTableNode = pTable->GetTableNode();
    ASSERT(pTableNode, "no table node!");
    if (pTableNode)
    {
        pIo->maSectionManager.PrependedInlineNode(*pIo->pPaM->GetPoint(),
            *pTableNode);
    }

    // Abfrage, ob im Node, in dem die Tabelle eingefuegt werden soll, bereits
    // ein Pagedesc steht. Dann wuerde der PageDesc in die naechste Zeile
    // hinter der Tabelle rutschen, wo er nichts zu suchen hat.  -> loeschen
    // und spaeter an das Tabellenformat setzen
    if (SwTxtNode* pNd = pIo->rDoc.GetNodes()[pTmpPos->nNode]->GetTxtNode())
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

            // evtl den PageDesc/Break jetzt an der Tabelle setzen
            if (pSetAttr)
            {
                aItemSet.Put(*pSetAttr);
                delete pSetAttr;
            }
        }
    }

    // Gesamtbreite der Tabelle
    if( nMaxRight - nMinLeft > MINLAY * nDefaultSwCols )
    {
        pTable->GetFrmFmt()->SetAttr(SwFmtFrmSize(ATT_FIX_SIZE, nSwWidth));
        aItemSet.Put(SwFmtFrmSize(ATT_FIX_SIZE, nSwWidth));
    }

    SvxFrameDirectionItem aDirection(
        bIsBiDi ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP );
    aItemSet.Put(aDirection);

    if (HORI_LEFT_AND_WIDTH == eOri)
    {
        if (!pIo->nInTable && pIo->InLocalApo() && pIo->pSFlyPara->pFlyFmt &&
            GetMinLeft())
        {
            //If we are inside a frame and we have a border, the frames
            //placement does not consider the tables border, which word
            //displays outside the frame, so adjust here.
            SwFmtHoriOrient aHori(pIo->pSFlyPara->pFlyFmt->GetHoriOrient());
            SwHoriOrient eHori = aHori.GetHoriOrient();
            if ((eHori == HORI_NONE) || (eHori == HORI_LEFT) ||
                (eHori == HORI_LEFT_AND_WIDTH))
            {
                //With multiple table, use last table settings. Perhaps
                //the maximum is what word does ?
                aHori.SetPos(pIo->pSFlyPara->nXPos + GetMinLeft());
                aHori.SetHoriOrient(HORI_NONE);
                pIo->pSFlyPara->pFlyFmt->SetAttr(aHori);
            }
        }
        else
        {
            //If bApo is set, then this table is being placed in a floating
            //frame, and the frame matches the left and right *lines* of the
            //table, so the space to the left of the table isn't to be used
            //inside the frame, in word the dialog involved greys out the
            //ability to set the margin.
            SvxLRSpaceItem aL;
            aL.SetLeft( GetMinLeft() );
            aItemSet.Put(aL);
        }
    }
}

void WW8TabDesc::UseSwTable()
{
    // globale Varis initialisieren
    pTabLines = &pTable->GetTabLines();
    nAktRow = nAktCol = nAktBandRow = 0;

    pTblNd  = (SwTableNode*)(*pTabLines)[0]->GetTabBoxes()[0]->
        GetSttNd()->FindTableNode();
    ASSERT( pTblNd, "wo ist mein TabellenNode" );
    pTblNd->GetTable().SetHeadlineRepeat( bHeader );

    // ggfs. Zusatz-Zellen einfuegen u.dgl.
    AdjustNewBand();

    WW8DupProperties aDup(pIo->rDoc,pIo->pCtrlStck);
    pIo->pCtrlStck->SetAttr(*pIo->pPaM->GetPoint(), 0, false);

    // jetzt den PaM korrekt setzen und ggfs. erste Mergegruppe vorbereiten...
    SetPamInCell(nAktCol, true);
    aDup.Insert(*pIo->pPaM->GetPoint());

    pIo->bWasTabRowEnd = false;
}

void WW8TabDesc::MergeCells()
{
    short nRow;

    for (pActBand=pFirstBand, nRow=0; pActBand; pActBand=pActBand->pNextBand)
    {
        //
        // ggfs. aktuelle Box in entsprechende Merge-Gruppe eintragen
        //
        if( pActBand->pTCs )
        {
            for( short j = 0; j < pActBand->nRows; j++, nRow++ )
                for( short i = 0; i < pActBand->nWwCols; i++ )
                {
                    SwTableBox* pTargetBox;
                    WW8SelBoxInfoPtr pActMGroup = 0;
                    //
                    // ggfs. eine neue Merge-Gruppe beginnen
                    //
                    ASSERT(nRow < pTabLines->Count(),
                        "Too few lines, table ended early");
                    if (nRow >= pTabLines->Count())
                        return;
                    pTabLine = (*pTabLines)[ nRow ];
                    pTabBoxes = &pTabLine->GetTabBoxes();

                    USHORT nCol = pActBand->nTransCell[ i ];
                    ASSERT(nCol < pTabBoxes->Count(),
                        "Too few columns, table ended early");
                    if (nCol >= pTabBoxes->Count())
                        return;
                    pTabBox = (*pTabBoxes)[nCol];
                    WW8_TCell& rCell = pActBand->pTCs[ i ];
                    // ist dies die obere, linke-Zelle einer Merge-Gruppe ?

                    bool bMerge = false;
                    if ( rCell.bVertRestart && !rCell.bMerged )
                        bMerge = true;
                    else if (rCell.bFirstMerged && pActBand->bExist[i])
                    {
                        //#91211# Some tests to avoid merging cells
                        //which previously were declared invalid because
                        //of sharing the exact same dimensions as their
                        //previous cell

                        //If theres anything underneath/above we're ok.
                        if (rCell.bVertMerge || rCell.bVertRestart)
                            bMerge = true;
                        else
                        {
                        //If its a hori merge only, and the only things in
                        //it are invalid cells then its already taken care
                        //of, so don't merge.
                            for (USHORT i2 = i+1; i2 < pActBand->nWwCols; i2++ )
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

                        // 0. falls noetig das Array fuer die Merge-Gruppen
                        // anlegen
                        if( !pMergeGroups )
                            pMergeGroups = new WW8MergeGroups;
                        else
                        {
                            // 1. ggfs. alte Mergegruppe(n) schliessen, die
                            // den von unserer neuen Gruppe betroffenen
                            // X-Bereich ueberdecken
                            short nMGrIdx;
                            while(FindMergeGroup( nX1, nWidth, false, nMGrIdx))
                                (*pMergeGroups)[ nMGrIdx ]->bGroupLocked = true;
                        }

                        // 2. aktuelle Merge-Gruppe anlegen
                        pActMGroup = new WW8SelBoxInfo( nX1, nWidth );

                        // 3. und in Gruppen-Array eintragen
                        pMergeGroups->Insert(pActMGroup, pMergeGroups->Count());

                        // 4. Target-Box anlegen und als 0. in Merge-Group
                        // setzen
                        pIo->rDoc.GetNodes().InsBoxen(pTblNd, pTabLine,
                            (SwTableBoxFmt*)pTabBox->GetFrmFmt(),
                            (SwTxtFmtColl*)pIo->rDoc.GetDfltTxtFmtColl(), 0,
                            nCol );

                        pTargetBox = (*pTabBoxes)[ nCol ];

                        // eingefuegte Box wieder aus der Row entfernen
                        // (wird vom Merge() dann endgueltig richtig eingebaut)
                        pTabBoxes->Remove( nCol );

                        // und ab damit in die Merge-Group
                        pActMGroup->Insert( pTargetBox, pActMGroup->Count() );
                        //
                        // 5. Target-Box formatieren
                        //
                        pTargetBox->SetUpper( 0 );

                        // eigenes Frame-Format fuer diese Box anlegen
                        SwFrmFmt* pNewFrmFmt = pTargetBox->ClaimFrmFmt();

                        // Border der O-L-Box der Gruppe wird Border der
                        // Targetbox
                        pNewFrmFmt->SetAttr( pTabBox->GetFrmFmt()->GetBox() );
                        // Gesamtbreite ermitteln und zuweisen
                        short nSizCell = pActBand->nWidth[ i ];
                        for (USHORT i2 = i+1; i2 < pActBand->nWwCols; i2++ )
                            if (pActBand->pTCs[ i2 ].bMerged &&
                                !pActBand->pTCs[ i2 ].bFirstMerged  )
                            {
                                nSizCell += pActBand->nWidth[ i2 ];
                            }
                            else
                                break;
                        pActMGroup->nGroupWidth = nSizCell;
                        pNewFrmFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                            pActMGroup->nGroupWidth ));
                    }

                    // ggfs. akt. Box zu einer Merge-Gruppe hinzufuegen (dies
                    // kann eine soeben angelegte, oder eine andere Gruppe
                    // sein)
                    UpdateTableMergeGroup( rCell, pActMGroup, pTabBox, i );
                }
        }
    }
}

void WW8TabDesc::FinishSwTable()
{
    WW8DupProperties aDup(pIo->rDoc,pIo->pCtrlStck);
    pIo->pCtrlStck->SetAttr( *pIo->pPaM->GetPoint(), 0, false);

    *pIo->pPaM->GetPoint() = *pTmpPos;
    delete pTmpPos, pTmpPos = 0;

    aDup.Insert(*pIo->pPaM->GetPoint());

    pIo->bWasTabRowEnd = false;
    if( pIo->rDoc.GetRootFrm() )
    {
        // exitiert schon ein Layout, dann muss an dieser Tabelle die
        // BoxFrames neu erzeugt werden.
        pTblNd->DelFrms();
        pTblNd->MakeFrms( &pIo->pPaM->GetPoint()->nNode );
    }

    MergeCells();

    // falls noetig, zu mergende Zellen gruppenweise zusammenfassen
    if( pMergeGroups )
    {
        // bearbeite alle Merge-Gruppen nacheinander
        SwTableBox*    pTargetBox;
        WW8SelBoxInfo* pActMGroup;
        USHORT         nActBoxCount;
        for( USHORT iGr = 0; iGr < pMergeGroups->Count(); iGr++ )
        {
            pActMGroup   = (*pMergeGroups)[ iGr ];
            // Anzahl der zu mergenden Boxen PLUS Eins (fuer die TargetBox)
            nActBoxCount = pActMGroup->Count();

            if( ( 1 < nActBoxCount ) && pActMGroup && (*pActMGroup)[ 0 ] )
            {
                // beachten: der 0. Eintrag ist die ebenso wie
                // die obere, linke Box formatierte Target-Box !!!
                pTargetBox = (*pActMGroup)[ 0 ];
                //
                // Target-Box verschoenern: unteren Rand der letzten Box nehmen
                //
                SwFrmFmt* pTargetFrmFmt = pTargetBox->GetFrmFmt();
                SvxBoxItem aBoxItem( pTargetFrmFmt->GetBox() );
                aBoxItem.SetLine( (*pActMGroup)[ nActBoxCount-1 ]->
                    GetFrmFmt()->GetBox().GetBottom(), BOX_LINE_BOTTOM );
                pTargetFrmFmt->SetAttr( aBoxItem );

                // oeffne Merge-Array mit passender Groesse
                SwSelBoxes aBoxes( nActBoxCount-1 );

                // alle Boxen dieser Gruppe in aBoxes versammeln
                aBoxes.Insert( pActMGroup->GetData()+1, nActBoxCount-1 );

                // Sicherheitspruefung: Gruppe mit nur EINER Box fuehrt beim
                // Mergen zum Programmabsturz, deshalb diesen unsinnigen Fall
                // abfangen!
                USHORT nMergeTest;
                if( 2 == nActBoxCount )
                {
                    // nur 1 Zelle in der Merge-Gruppe: wahrscheinlich Word-Bug
                    nMergeTest = TBLMERGE_TOOCOMPLEX;
                }
                else
                {   //
                    // Vorsicht: erstmal austesten,
                    // ob sich die aBoxes ueberhaupt mergen lassen wuerden...
                    //

                    // siehe   /SW/INC/TBLSEL.HXX
                    nMergeTest = CheckMergeSel( aBoxes );
                    BYTE  nTry = 1;
                    //
                    // Falls das Merge so nicht machbar ist,
                    // pruefen, ob es ginge, wenn wir die letzte Box
                    // oder die beiden letzten Boxen
                    // oder gar die drei letzten Boxen
                    // aus der Merge-Gruppe rausnehmen wuerden
                    while(     ( TBLMERGE_TOOCOMPLEX == nMergeTest )
                                    && ( 4 > nTry                          )
                                    && ( 3 < nActBoxCount /* wichtig ! */  ) )
                    {
                        nTry++;
                        nActBoxCount--;
                        // letzte Box aus der Gruppe rausnehmen
                        aBoxes.Remove( nActBoxCount-1 );
                        // und nachsehen, ob Merge nun moeglich waere
                        nMergeTest = CheckMergeSel( aBoxes );
                    }
                }
                //
                // Genug geprueft - wir schreiten zur Tat...
                //
                switch( nMergeTest )
                {
                case TBLMERGE_OK:
                    {
                    //
                    // You succeed at last...
                    //
                    SwTxtNode* pTxtNd;
                    SwPosition aInsPos( *pTargetBox->GetSttNd() );
                    SwNodeIndex& rInsPosNd = aInsPos.nNode;
                    SwPaM aPam( aInsPos );

                    //Move content of cells being merged to new cell
                    for (USHORT n=1;n<pActMGroup->Count();n++)
                    {
                        SwPaM aPamTest( pIo->rDoc.GetNodes() );
                        //If theres nothing in the cell (after the first) ignore it
                        if ((n > 1) && (IsEmptyBox( *(*pActMGroup)[n], aPamTest)))
                            continue;
                        aPam.GetPoint()->nNode.Assign( *(*pActMGroup)[n]->
                            GetSttNd()->EndOfSectionNode(), -1 );

                        SwCntntNode* pCNd = aPam.GetCntntNode();
                        USHORT nL = pCNd ? pCNd->Len() : 0;
                        aPam.GetPoint()->nContent.Assign( pCNd, nL );

                        SwNodeIndex aSttNdIdx(*(*pActMGroup)[n]->GetSttNd(),1);

                        pIo->AppendTxtNode( *aPam.GetPoint() );
                        SwNodeRange aRg( aSttNdIdx, aPam.GetPoint()->nNode );
                        rInsPosNd++;

                        pIo->rDoc.Move( aRg, rInsPosNd );
                        rInsPosNd.Assign( pIo->rDoc.GetNodes(),
                            rInsPosNd.GetNode().EndOfSectionIndex() - 2 );
                        pTxtNd = rInsPosNd.GetNode().GetTxtNode();
                        if( pTxtNd )
                            aInsPos.nContent.Assign(
                                    pTxtNd, pTxtNd->GetTxt().Len() );
                    }

                    SwNodeIndex aIdx( *pTargetBox->GetSttNd()->
                        EndOfSectionNode(), -1 );
                    pIo->rDoc.GetNodes().Delete( aIdx, 1 );

                    aPam.GetPoint()->nNode = *pTargetBox->GetSttNd();
                    aPam.GetPoint()->nContent.Assign( 0, 0 );

                    ((SwTable*)pTable)->Merge( &pIo->rDoc, aBoxes, pTargetBox );

                    }
                    break;
                case TBLMERGE_NOSELECTION:
                    /*NOP*/     // war wohl nix?
                    break;
                case TBLMERGE_TOOCOMPLEX:
                    // Pech: Trotz aller Probiererei geht das Merge immer noch
                    // nicht!
                    //
                    // Beachten: die Daten der Gruppe schlummern bereits in
                    // der Target-Box. Da kein Merge moeglich ist, setzen wir
                    // die Target-Box in die Tabelle an die Stelle der oberen
                    // linken Box.
                    {
                        const SwTableBox* pBox  = (*pActMGroup)[ 1 ];
                        SwTableLine*      pLine = (SwTableLine*)pBox->GetUpper();

                        USHORT nPos = pLine->GetTabBoxes().GetPos( pBox );

                        ASSERT( USHRT_MAX != nPos, "GetPos fehlgeschlagen");

                        SwStartNode* pSttNd = (SwStartNode*)pBox->GetSttNd();

                        ASSERT( pSttNd, "Box ohne Start-Node ?!");

                        pTargetBox->ChgFrmFmt(
                            (SwTableBoxFmt*)pBox->GetFrmFmt() );
                        pTargetBox->SetUpper(  pLine );

                        // erst die Box loeschen!!
                        pLine->GetTabBoxes().DeleteAndDestroy( nPos );
                        // dann die pTargetBox einfuegen
                        pLine->GetTabBoxes().C40_INSERT( SwTableBox,
                            pTargetBox, nPos );
                        // dann die Nodes loeschen!!
                        pIo->rDoc.DeleteSection( pSttNd );
                    }
                    break;
                default:            // was wollen wir denn hier ???
                    ASSERT(!this,"CheckMergeSel() with undefined return value");
                    break;
                }
            }
        }
        DELETEZ( pMergeGroups );
    }
    // nun noch ggfs. die doppelten Innenraender korrigieren (Bug #53525#)
    if( pTable )
        ((SwTable*)pTable)->GCBorderLines();    // Garbage Collect
}


// durchsucht pMergeGroups, meldet Index der ersten, passenden Gruppe bzw. -1
//
// Parameter: nXcenter  = Mittenposition der anfragenden Box
//            nWidth    = Breite der anfragenden Box
//            bExact    = Flag, ob Box in dieser Gruppe passen muss,
//                          oder diese nur zu tangieren braucht
//
bool WW8TabDesc::FindMergeGroup(short nX1, short nWidth, bool bExact,
    short& nMGrIdx)
{
    nMGrIdx = -1;
    if( pMergeGroups )
    {
        // noch als gueltig angesehener Bereich in der Naehe der Grenzen
        const short nToleranz = 4;
        // die aktuell untersuchte Gruppe
        WW8SelBoxInfoPtr pActGroup;
        // Boxgrenzen
        short nX2 = nX1 + nWidth;
        // ungefaehre Gruppengrenzen
        short nGrX1;
        short nGrX2;

        for( USHORT iGr = 0; iGr < pMergeGroups->Count(); iGr++ )
        {
            // die aktuell untersuchte Gruppe
            pActGroup = (*pMergeGroups)[ iGr ];
            if (!pActGroup->bGroupLocked)
            {
                // ungefaehre Gruppengrenzen mit Toleranz nach *aussen* hin
                nGrX1 = pActGroup->nGroupXStart - nToleranz;
                nGrX2 = pActGroup->nGroupXStart
                             +pActGroup->nGroupWidth  + nToleranz;
                //
                // Falls Box reinpasst, melde auf jeden Fall den Erfolg
                //
                if( ( nX1 > nGrX1 ) && ( nX2 < nGrX2 ) )
                {
                    nMGrIdx = iGr;  break;
                }
                //
                // hat die Box Bereiche mit der Gruppe gemeinsam?
                //
                if( !bExact )
                {
                    // melde Erfolg, wenn nX1 *oder* nX2 innerhalb der Gruppe liegen
                    if(    (     ( nX1 > nGrX1 )
                                        && ( nX1 < nGrX2 - 2*nToleranz ) )
                            || (     ( nX2 > nGrX1 + 2*nToleranz )
                                        && ( nX2 < nGrX2 ) )
                            // oder nX1 und nX2 die Gruppe umfassen
                            || (     ( nX1 <=nGrX1 )
                                        && ( nX2 >=nGrX2 ) ) )
                    {
                        nMGrIdx = iGr;  break;
                    }
                }
            }
        }
    }
    return ( -1 < nMGrIdx );
}


bool WW8TabDesc::IsValidCell(short nCol) const
{
    return pActBand->bExist[nCol] && (USHORT)nAktRow < pTabLines->Count();
}


bool WW8TabDesc::SetPamInCell(short nWwCol, bool bPam)
{
    ASSERT( pActBand, "pActBand ist 0" );

    USHORT nCol = pActBand->nTransCell[nWwCol];

    if ((USHORT)nAktRow >= pTabLines->Count())
    {
        ASSERT(!this, "Actual row bigger than expected." );
        return false;
    }

    pTabLine = (*pTabLines)[nAktRow];
    pTabBoxes = &pTabLine->GetTabBoxes();

    if ( nCol >= pTabBoxes->Count() )
        return false;
    pTabBox = (*pTabBoxes)[nCol];
    if( !pTabBox->GetSttNd() )
    {
        ASSERT(pTabBox->GetSttNd(), "Probleme beim Aufbau der Tabelle");
        return false;
    }
    if( bPam )
    {
        pAktWWCell = &pActBand->pTCs[ nWwCol ];
        //We need to set the pPaM on the first cell, invalid
        //or not so that we can collect paragraph proproties over
        //all the cells, but in that case on the valid cell we do not
        //want to reset the fmt properties
        if (pIo->pPaM->GetPoint()->nNode != pTabBox->GetSttIdx() + 1)
        {
            pIo->pPaM->GetPoint()->nNode = pTabBox->GetSttIdx() + 1;
            pIo->pPaM->GetPoint()->nContent.Assign( pIo->pPaM->GetCntntNode(), 0 );
            // Zur Sicherheit schon jetzt setzen, da bei den Zellen, die
            // zum Randausgleich eingefuegt werden, sonst der Style
            // nicht gesetzt wird.
            pIo->rDoc.SetTxtFmtColl( *pIo->pPaM, (SwTxtFmtColl*)pIo->pDfltTxtFmtColl );
            // uebrigens: da diese Zellen unsichtbare Hilfskonstruktionen sind,
            //            und nur dazu dienen, zerfranste Aussehen der WW-Tabelle
            //            nachzuahmen, braucht NICHT SetTxtFmtCollAndListLevel()
            //            verwendet zu werden.
        }
    }
    return true;
}

void WW8TabDesc::InsertCells( short nIns )
{
    pTabLine = (*pTabLines)[nAktRow];
    pTabBoxes = &pTabLine->GetTabBoxes();
    pTabBox = (*pTabBoxes)[0];

    pIo->rDoc.GetNodes().InsBoxen( pTblNd, pTabLine, (SwTableBoxFmt*)pTabBox->GetFrmFmt(),
                            (SwTxtFmtColl*)pIo->pDfltTxtFmtColl, 0, pTabBoxes->Count(), nIns );
    // mit dem Dritten Parameter wird das FrmFmt der Boxen angegeben.
    // hier kann man auch noch optimieren, um FrmFmts zu sparen
}

void WW8TabDesc::SetTabBorders(SwTableBox* pBox, short nWwIdx)
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // kuenstlich erzeugte Zellen -> Kein Rand


    SvxBoxItem aFmtBox;
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

    // nGapHalf bedeutet bei WW ein *horizontaler* Abstand zwischen
    // Tabellenzelle und -Inhalt
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

    pBox->GetFrmFmt()->SetAttr(aFmtBox);
}

void WW8TabDesc::SetTabShades( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;                 // kuenstlich erzeugte Zellen -> Keine Farbe

    bool bFound=false;
    if (pActBand->pNewSHDs && pActBand->pNewSHDs[nWwIdx] != COL_AUTO)
    {
        Color aColor(pActBand->pNewSHDs[nWwIdx]);
        pBox->GetFrmFmt()->SetAttr(SvxBrushItem(aColor));
        bFound = true;
    }

    //If there was no new shades, or no new shade setting
    if (pActBand->pSHDs && !bFound)
    {
        WW8_SHD& rSHD = pActBand->pSHDs[nWwIdx];
        if (!rSHD.GetValue())       // auto
            return;

        SwWW8Shade aSh( pIo->bVer67, rSHD );
        pBox->GetFrmFmt()->SetAttr(SvxBrushItem(aSh.aColor));
    }
}

void WW8TabDesc::SetTabVertAlign( SwTableBox* pBox, short nWwIdx )
{
    if( nWwIdx < 0 || nWwIdx >= pActBand->nWwCols )
        return;

    SwVertOrient eVertOri=VERT_TOP;

    if( pActBand->pTCs )
    {
        WW8_TCell* pT = &pActBand->pTCs[nWwIdx];
        switch (pT->nVertAlign)
        {
            case 0:
            default:
                eVertOri = VERT_TOP;
                break;
            case 1:
                eVertOri = VERT_CENTER;
                break;
            case 2:
                eVertOri = VERT_BOTTOM;
                break;
        }
    }

    pBox->GetFrmFmt()->SetAttr( SwFmtVertOrient(0,eVertOri) );
}

void WW8TabDesc::AdjustNewBand()
{
    if( pActBand->nSwCols > nDefaultSwCols )        // Zellen splitten
        InsertCells( pActBand->nSwCols - nDefaultSwCols );

    SetPamInCell( 0, false);
    ASSERT( pTabBoxes && pTabBoxes->Count() == (USHORT)pActBand->nSwCols,
        "Falsche Spaltenzahl in Tabelle" )

    if( bClaimLineFmt )
    {
        pTabLine->ClaimFrmFmt();            // noetig wg. Zeilenhoehe
        SwFmtFrmSize aF( ATT_MIN_SIZE, 0, 0 );  // default

        if( pActBand->nLineHeight == 0 )    // 0 = Auto
            aF.SetSizeType( ATT_VAR_SIZE );
        else
        {
            if( pActBand->nLineHeight < 0 ) // Pos = min, Neg = exakt
            {
                aF.SetSizeType( ATT_FIX_SIZE );
                pActBand->nLineHeight = -pActBand->nLineHeight;
            }
            if( pActBand->nLineHeight < MINLAY ) // nicht erlaubte Zeilenhoehe
                pActBand->nLineHeight = MINLAY;

            aF.SetHeight( pActBand->nLineHeight );// Min- / Exakt-Hoehe setzen
        }
        pTabLine->GetFrmFmt()->SetAttr( aF );
    }

    short i;    // SW-Index
    short j;    // WW-Index
    short nW;   // Breite
    SwFmtFrmSize aFS( ATT_FIX_SIZE );
    j = pActBand->bLEmptyCol ? -1 : 0;

    for( i = 0; i < pActBand->nSwCols; i++ )
    {
        // setze Zellenbreite
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

        register SwTableBox* pBox = (*pTabBoxes)[i];
        // liesse sich durch intelligentes Umhaengen der FrmFmts noch weiter
        // verringern
        pBox->ClaimFrmFmt();

        SetTabBorders(pBox, j);
        SetTabVertAlign(pBox, j);
        if( pActBand->pSHDs || pActBand->pNewSHDs)
            SetTabShades(pBox, j);
        j++;

        aFS.SetWidth( nW );
        pBox->GetFrmFmt()->SetAttr( aFS );

        // ueberspringe nicht existente Zellen
        while( ( j < pActBand->nWwCols ) && !pActBand->bExist[j] )
        {
            pActBand->nWidth[j] = pActBand->nCenter[j+1] - pActBand->nCenter[j];
            j++;
        }
    }
}

void WW8TabDesc::TableCellEnd()
{
    ::SetProgressState( pIo->nProgress, pIo->rDoc.GetDocShell() );   // Update

    // neue Zeile
    if( pIo->bWasTabRowEnd )
    {
        // bWasTabRowEnd will be deactivated in
        // SwWW8ImplReader::ProcessSpecial()

        USHORT iCol = GetLogicalWWCol();
        if (iCol < aNumRuleNames.size())
        {
            aNumRuleNames.erase(aNumRuleNames.begin() + iCol,
                aNumRuleNames.end());
        }

        nAktCol = 0;
        nAktRow++;
        nAktBandRow++;
        ASSERT( pActBand , "pActBand ist 0" );
        if( pActBand )
        {
            if( nAktRow >= nRows )  // am Tabellenende gibt's nichts sinnvolles
                return;                 // mehr zu tun

            bool bNewBand = nAktBandRow >= pActBand->nRows;
            if( bNewBand )
            {                       // neues Band noetig ?
                pActBand = pActBand->pNextBand; //
                nAktBandRow = 0;
                ASSERT( pActBand, "pActBand ist 0" );
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
    {                       // neue Spalte ( Zelle )
        nAktCol++;
    }
    SetPamInCell(nAktCol, true);

    // finish Annotated Level Numbering ?
    if( pIo->bAnl && !pIo->bAktAND_fNumberAcross )
        pIo->StopAnl( IsValidCell( nAktCol ) );//false);//!bRowEnded ); //
}

// ggfs. die Box in fuer diese Col offene Merge-Gruppe eintragen
SwTableBox* WW8TabDesc::UpdateTableMergeGroup(  WW8_TCell&     rCell,
                                                WW8SelBoxInfo* pActGroup,
                                                SwTableBox*    pActBox,
                                                USHORT         nCol )
{
    // Rueckgabewert defaulten
    SwTableBox* pResult = 0;

    // pruefen, ob die Box zu mergen ist
    if( pActBand->bExist[ nCol ] &&
        (rCell.bFirstMerged
        || rCell.bMerged
        || rCell.bVertMerge
        || rCell.bVertRestart) )
    {
        // passende Merge-Gruppe ermitteln
        WW8SelBoxInfo* pTheMergeGroup = 0;
        if( pActGroup )
            // Gruppe uebernehmen
            pTheMergeGroup = pActGroup;
        else
        {
            // Gruppe finden
            short nMGrIdx;
            if( FindMergeGroup( pActBand->nCenter[ nCol ],
                                pActBand->nWidth[  nCol ], true, nMGrIdx ) )
                pTheMergeGroup = (*pMergeGroups)[ nMGrIdx ];
        }
        if( pTheMergeGroup )
        {
            // aktuelle Box der Merge-Gruppe hinzufuegen
            pTheMergeGroup->Insert( pActBox, pTheMergeGroup->Count() );

            // Target-Box zurueckmelden
            pResult = (*pTheMergeGroup)[ 0 ];
        }
    }
    return pResult;
}


USHORT WW8TabDesc::GetLogicalWWCol() const // returns number of col as INDICATED within WW6 UI status line -1
{
    USHORT nCol = 0;
    if( pActBand && pActBand->pTCs)
    {
        for( USHORT iCol = 1; iCol <= nAktCol; ++iCol )
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
    USHORT nCol = GetLogicalWWCol();
    if (nCol < aNumRuleNames.size())
        return aNumRuleNames[nCol];
    else
        return aEmptyStr;
}

void WW8TabDesc::SetNumRuleName( const String& rName )
{
    USHORT nCol = GetLogicalWWCol();
    for (USHORT nSize = aNumRuleNames.size(); nSize <= nCol; ++nSize)
        aNumRuleNames.push_back(aEmptyStr);
    aNumRuleNames[nCol] = rName;
}

bool SwWW8ImplReader::StartTable(WW8_CP nStartCp)
{
    // keine rekursiven Tabellen Nicht bei EinfuegenDatei in Tabelle oder
    // Fussnote
    if (bReadNoTbl)
        return false;

    if (pTableDesc)
        aTableStack.push(pTableDesc);

    pTableDesc = new WW8TabDesc( this, nStartCp );

    if( pTableDesc->Ok() )
    {
        int nNewInTable = nInTable + 1;
        if (InEqualApo(nNewInTable))
        {
            ASSERT(pSFlyPara->pFlyFmt,
                "how could we be in a local apo and have no apo");
        }

        if (!aTableStack.empty() && !InEqualApo(nNewInTable))
        {
            pTableDesc->pParentPos = new SwPosition(*pPaM->GetPoint());
            SfxItemSet aItemSet(rDoc.GetAttrPool(),
                RES_FRMATR_BEGIN, RES_FRMATR_END-1);
            SwFmtAnchor aAnchor(FLY_IN_CNTNT);
            aAnchor.SetAnchor(pTableDesc->pParentPos);
            aItemSet.Put(aAnchor);
            pTableDesc->pFlyFmt = rDoc.MakeFlySection(FLY_IN_CNTNT,
                pTableDesc->pParentPos, &aItemSet);
            ASSERT(pTableDesc->pFlyFmt->GetAnchor().GetAnchorId()
                == FLY_IN_CNTNT, "Not the anchor type requested!");
            MoveInsideFly(pTableDesc->pFlyFmt);
        }
        pTableDesc->CreateSwTable();
        if (pTableDesc->pFlyFmt)
        {
            SwFmtHoriOrient aHori = pTableDesc->pTable->GetFrmFmt()->
                GetHoriOrient();
            pTableDesc->SetSizePosition(pTableDesc->pFlyFmt);
            pTableDesc->pFlyFmt->SetAttr(SwFmtSurround(SURROUND_NONE));
            pTableDesc->pFlyFmt->SetAttr(aHori);
        }
        else
            pTableDesc->SetSizePosition(0);
        pTableDesc->UseSwTable();
    }
    else
        PopTableDesc();

    return 0 != pTableDesc;
}

void SwWW8ImplReader::TabCellEnd()
{
    if (nInTable && pTableDesc)
        pTableDesc->TableCellEnd();
}

void SwWW8ImplReader::Read_TabRowEnd( USHORT, const BYTE* pData, short nLen )   // Sprm25
{
    if( ( nLen > 0 ) && ( *pData == 1 ) )
        bWasTabRowEnd = true;
}

void SwWW8ImplReader::PopTableDesc()
{
    if (pTableDesc && pTableDesc->pFlyFmt)
        MoveOutsideFly(pTableDesc->pFlyFmt,*pTableDesc->pParentPos);

    delete pTableDesc;
    if (aTableStack.empty())
        pTableDesc = 0;
    else
    {
       pTableDesc = aTableStack.top();
       aTableStack.pop();
    }
}

void SwWW8ImplReader::StopTable()
{
    pTableDesc->FinishSwTable();
    PopTableDesc();
}

// GetTableLeft() wird fuer absatzgebundene Grafikobjekte in Tabellen
// gebraucht.
// WW nimmt bei eingerueckten Tabellen den Absatzrand, der ohne Tabelle
// gueltig waere, als Basis; SW benutzt den linken Tabellenrand.
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

const USHORT SwWW8ImplReader::StyleUsingLFO( USHORT nLFOIndex ) const
{
    USHORT nRes = USHRT_MAX;
    if( pCollA )
    {
        for(USHORT nI = 0; nI < pStyles->GetCount(); nI++ )
            if(    pCollA[ nI ].bValid
                && (nLFOIndex == pCollA[ nI ].nLFOIndex) )
                nRes = nI;
    }
    return nRes;
}

const SwFmt* SwWW8ImplReader::GetStyleWithOrgWWName( String& rName ) const
{
    SwFmt* pRet = 0;
    if( pCollA )
    {
        for(USHORT nI = 0; nI < pStyles->GetCount(); nI++ )
            if(    pCollA[ nI ].bValid
                && (rName.Equals( pCollA[ nI ].GetOrgWWName())) )
            {
                pRet = pCollA[ nI ].pFmt;
                break;
            }
    }
    return pRet;
}

//-----------------------------------------
//          class WW8RStyle
//-----------------------------------------

const BYTE* WW8RStyle::HasParaSprm( USHORT nId ) const
{
    if( !pParaSprms || !nSprmsLen )
        return 0;

    const BYTE* pSprms = pParaSprms;
    USHORT i, x;

    for( i=0; i < nSprmsLen; )
    {
        USHORT nAktId = maSprmParser.GetSprmId(pSprms);
        // Sprm found ?
        if( nAktId == nId )
            return pSprms + maSprmParser.DistanceToData(nId);

        x = maSprmParser.GetSprmSize(nAktId, pSprms);
        i += x;
        pSprms += x;
    }
    return 0;                               // Sprm not found
}

void WW8RStyle::ImportSprms(long nPosFc, short nLen, bool bPap)
{
    if( pStStrm->IsEof() )
        return;

    BYTE *pSprms0 = new BYTE[nLen];
    if( bPap )
    {
        pParaSprms = pSprms0;   // fuer HasParaSprms()
        nSprmsLen = nLen;
    }
    BYTE* pSprms1 = pSprms0;
    pStStrm->Seek( nPosFc );
    pStStrm->Read( pSprms0, nLen );

    while ( nLen > 0 )
    {
        USHORT nL1 = pIo->ImportSprm(pSprms1);
        nLen -= nL1;
        pSprms1 += nL1;
    }

    delete[] pSprms0;
    pParaSprms = 0;
    nSprmsLen = 0;
}

static inline short WW8SkipOdd(SvStream* pSt )
{
    if ( pSt->Tell() & 0x1 )
    {
        UINT8 c;
        pSt->Read( &c, 1 );
        return 1;
    }
    return 0;
}

static inline short WW8SkipEven(SvStream* pSt )
{
    if (!(pSt->Tell() & 0x1))
    {
        UINT8 c;
        pSt->Read( &c, 1 );
        return 1;
    }
    return 0;
}

short WW8RStyle::ImportUPX(short nLen, bool bPAP, bool bOdd)
{
    INT16 cbUPX;

    if( 0 < nLen ) // Empty ?
    {
        if (bOdd)
            nLen -= WW8SkipEven( pStStrm );
        else
            nLen -= WW8SkipOdd( pStStrm );

        *pStStrm >> cbUPX;

        nLen-=2;

        if ( cbUPX > nLen )
            cbUPX = nLen;       // !cbUPX auf nLen verkleinert!

        if( (1 < cbUPX) || ( (0 < cbUPX) && !bPAP ) )
        {
            if( bPAP )
            {
                UINT16 id;
                *pStStrm >> id;

                cbUPX-=  2;
                nLen-=  2;
            }

            if( 0 < cbUPX )
            {
                ULONG nPos = pStStrm->Tell();   // falls etwas falsch interpretiert
                                                // wird, gehts danach wieder richtig
                ImportSprms( nPos, cbUPX, bPAP );

                if ( pStStrm->Tell() != nPos + cbUPX )
                    pStStrm->Seek( nPos+cbUPX );

                nLen -= cbUPX;
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
        nLen -= WW8SkipEven( pStStrm );
    else
        nLen -= WW8SkipOdd( pStStrm );

    if( bPara ) // Grupx.Papx
        nLen = ImportUPX(nLen, true, bOdd);
    ImportUPX(nLen, false, bOdd);                   // Grupx.Chpx
}

WW8RStyle::WW8RStyle(WW8Fib& rFib, SwWW8ImplReader* pI)
    : WW8Style(*pI->pTableStream, rFib), maSprmParser(rFib.nVersion),
    pIo(pI), pStStrm(pI->pTableStream), pStyRule(0), nWwNumLevel(0)
{
    pIo->pCollA = new SwWW8StyInf[ cstd ]; // Style-UEbersetzung WW->SW
    pIo->nColls = cstd;
}

void WW8RStyle::Set1StyleDefaults()
{
    if (!bCJKFontChanged)   // Style no CJK Font? set the default
        pIo->SetNewFontAttr(ftcStandardChpCJKStsh, true, RES_CHRATR_CJK_FONT);

    if (!bCTLFontChanged)   // Style no CTL Font? set the default
        pIo->SetNewFontAttr(ftcStandardChpCTLStsh, true, RES_CHRATR_CTL_FONT);

    //#88976# western 2nd to make western charset conversion the default
    if (!bFontChanged)      // Style has no Font? set the default,
        pIo->SetNewFontAttr(ftcStandardChpStsh, true, RES_CHRATR_FONT);

    if( !pIo->bNoAttrImport )
    {
        // Style has no text color set, winword default is auto
        if ( !bTxtColChanged )
            pIo->pAktColl->SetAttr(SvxColorItem(Color(COL_AUTO)));

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !bFSizeChanged )
        {
            SvxFontHeightItem aAttr(200);
            pIo->pAktColl->SetAttr(aAttr);
            aAttr.SetWhich(RES_CHRATR_CJK_FONTSIZE);
            pIo->pAktColl->SetAttr(aAttr);
        }

        // Style has no FontSize ? WinWord Default is 10pt for western and asian
        if( !bFCTLSizeChanged )
        {
            SvxFontHeightItem aAttr(200);
            aAttr.SetWhich(RES_CHRATR_CTL_FONTSIZE);
            pIo->pAktColl->SetAttr(aAttr);
        }

        if( pIo->pWDop->fWidowControl && !bWidowsChanged )  // Widows ?
        {
            pIo->pAktColl->SetAttr( SvxWidowsItem( 2 ) );
            pIo->pAktColl->SetAttr( SvxOrphansItem( 2 ) );
        }
    }
}

//-----------------------------------------
//      Zeichenvorlagen
//-----------------------------------------
SwCharFmt* WW8RStyle::SearchCharFmt( const String& rName )
{
    USHORT n;
                            // Suche zuerst in den Doc-Styles
    for( n = pIo->rDoc.GetCharFmts()->Count(); n > 0; )
        if( (*pIo->rDoc.GetCharFmts())[ --n ]->GetName().Equals( rName ) )
            return (*pIo->rDoc.GetCharFmts())[ n ];

                    // Collection noch nicht gefunden, vielleicht im Pool ?
    n = SwStyleNameMapper::GetPoolIdFromUIName( rName , GET_POOLID_CHRFMT );
    if ( n != USHRT_MAX )       // gefunden oder Standard
        return pIo->rDoc.GetCharFmtFromPool( n );

    return 0;
}

SwCharFmt* WW8RStyle::MakeNewCharFmt( WW8_STD* pStd, const String& rName )
{
    String aName( rName );
    SwCharFmt* pFmt = 0;

    // eingebauter Style, den wir nicht kennen ? oder Namen gibts schon ?
    if (pStd->sti != WW8_STD::STI_USER || SearchCharFmt(aName))
    {
        if (!aName.EqualsAscii("WW-", 0, 3))    // noch kein "WW-"
            aName.InsertAscii("WW-", 0);        // dann aender ihn

        if (SearchCharFmt(aName))               // Namen gibt's immer noch ?
        {
            for (USHORT n = 1; n < 1000; ++n)
            {
                String aName1( aName );
                aName1 += String::CreateFromInt32( n );
                if ((pFmt = SearchCharFmt(aName1)) == 0)
                {
                    aName = aName1;
                    break;   // unbenutzten Namen gefunden
                }
            }
        }
    }

    if (!pFmt)   // unbenutzter Collection-Name gefunden, erzeuge neue Coll
        pFmt = pIo->rDoc.MakeCharFmt(aName, pIo->rDoc.GetDfltCharFmt());

    return pFmt;
}

SwCharFmt* WW8RStyle::MakeOrGetCharFmt(bool * pbStyExist, WW8_STD* pStd,
    const String& rName )
{
    static USHORT __READONLY_DATA aArr1[]={
        RES_POOLCHR_FOOTNOTE, 0, RES_POOLCHR_LINENUM,
        RES_POOLCHR_PAGENO, RES_POOLCHR_ENDNOTE };

    static USHORT __READONLY_DATA aArr2[]={
        RES_POOLCHR_INET_NORMAL, RES_POOLCHR_INET_VISIT,
        RES_POOLCHR_HTML_STRONG, RES_POOLCHR_HTML_EMPHASIS };

    // Einfuegen: immer neue Styles generieren || nicht abgeschaltet
    if ( pIo->mbNewDoc && !(pIo->nIniFlags & WW8FL_NO_DEFSTYLES) )
    {
        SwCharFmt* pFmt = 0;

        // Default-Style bekannt ?
        short nIdx = pStd->sti - 38;
        if(    nIdx >= 0 && nIdx < 5
            && aArr1[nIdx]!=0 )
            pFmt = pIo->rDoc.GetCharFmtFromPool( aArr1[ nIdx ] );
        else
        {
            nIdx = pStd->sti - 85;
            if(    nIdx >= 0
                && nIdx <  4 )
                pFmt = pIo->rDoc.GetCharFmtFromPool( aArr2[ nIdx ] );
        }
        if( pFmt )
        {
            *pbStyExist = true;
            return pFmt;
        }
    }
    *pbStyExist = false;
    String aName( rName );
    xub_StrLen nPos = aName.Search( ',' );
    if( STRING_NOTFOUND != nPos )   // mehrere Namen mit Komma getrennt
        aName.Erase( nPos );        // gibts im SW nicht
    return MakeNewCharFmt( pStd, aName );   // nicht gefunden
}

//-----------------------------------------
//      Absatzvorlagen
//-----------------------------------------

SwTxtFmtColl* WW8RStyle::SearchFmtColl( const String& rName )
{
                            // Suche zuerst in den Doc-Styles
    SwTxtFmtColl* pColl = pIo->rDoc.FindTxtFmtCollByName( rName );
    if( !pColl )
    {
                    // Collection noch nicht gefunden, vielleicht im Pool ?
        USHORT n = SwStyleNameMapper::GetPoolIdFromUIName( rName , GET_POOLID_TXTCOLL );
        if ( n != USHRT_MAX )       // gefunden oder Standard
            pColl = pIo->rDoc.GetTxtCollFromPoolSimple(n, false);
    }
    return pColl;
}

SwTxtFmtColl* WW8RStyle::MakeNewFmtColl( WW8_STD* pStd, const String& rName )
{
    String aName( rName );
    SwTxtFmtColl* pColl = 0;

    // eingebauter Style, den wir nicht kennen
    // oder Namen gibts schon ?
    if (pStd->sti != WW8_STD::STI_USER || SearchFmtColl(aName))
    {
        if (!aName.EqualsIgnoreCaseAscii("WW-", 0, 3))  // noch kein "WW-"
            aName.InsertAscii("WW-" , 0);   // dann AEnder ihn

        // Namen gibt's immer noch ?
        if (SearchFmtColl(aName))
        {
            for (USHORT n = 1; n < 1000; ++n)
            {
                // dann bastel neuen
                String aName1(aName);
                aName += String::CreateFromInt32(n);
                if ((pColl = SearchFmtColl(aName1)) == 0)
                {
                    aName = aName1;
                    // unbenutzten Namen gefunden
                    break;
                }
            }
        }
    }

    if (!pColl)   // unbenutzter Collection-Name gefunden, erzeuge neue Coll
    {
        pColl = pIo->rDoc.MakeTxtFmtColl(aName,
            const_cast<SwTxtFmtColl *>(pIo->rDoc.GetDfltTxtFmtColl()));
    }

    return pColl;
}

SwTxtFmtColl* WW8RStyle::MakeOrGetFmtColl(bool * pbStyExist, WW8_STD* pStd,
    const String& rName)
{
#define RES_NONE RES_POOLCOLL_DOC_END

    static const RES_POOL_COLLFMT_TYPE aArr[]={
        RES_POOLCOLL_STANDARD, RES_POOLCOLL_HEADLINE1, RES_POOLCOLL_HEADLINE2,                                  // Ueberschrift 2
        RES_POOLCOLL_HEADLINE3, RES_POOLCOLL_HEADLINE4, RES_POOLCOLL_HEADLINE5,
        RES_POOLCOLL_HEADLINE6, RES_POOLCOLL_HEADLINE7, RES_POOLCOLL_HEADLINE8,
        RES_POOLCOLL_HEADLINE9,

        RES_POOLCOLL_TOX_IDX1, RES_POOLCOLL_TOX_IDX2, RES_POOLCOLL_TOX_IDX3,
        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
        RES_POOLCOLL_TOX_CNTNT1,

        RES_POOLCOLL_TOX_CNTNT2, RES_POOLCOLL_TOX_CNTNT3, RES_POOLCOLL_TOX_CNTNT4,
        RES_POOLCOLL_TOX_CNTNT5, RES_POOLCOLL_TOX_CNTNT6, RES_POOLCOLL_TOX_CNTNT7,
        RES_POOLCOLL_TOX_CNTNT8, RES_POOLCOLL_TOX_CNTNT9,
        RES_NONE, RES_POOLCOLL_FOOTNOTE,

        RES_NONE, RES_POOLCOLL_HEADER, RES_POOLCOLL_FOOTER, RES_POOLCOLL_TOX_IDXH,
        RES_NONE, RES_NONE, RES_POOLCOLL_JAKETADRESS, RES_POOLCOLL_SENDADRESS,
        RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_ENDNOTE, RES_NONE, RES_NONE, RES_NONE,
        RES_POOLCOLL_LISTS_BEGIN, RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,

        RES_NONE,RES_NONE, RES_POOLCOLL_DOC_TITEL, RES_NONE, RES_POOLCOLL_SIGNATURE, RES_NONE,
        RES_POOLCOLL_TEXT, RES_POOLCOLL_TEXT_MOVE, RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_DOC_SUBTITEL };


    ASSERT( ( sizeof( aArr ) / sizeof( RES_POOL_COLLFMT_TYPE ) == 75 ),
            "Style-UEbersetzungstabelle hat falsche Groesse" );

    if( pStd->sti < sizeof( aArr ) / sizeof( RES_POOL_COLLFMT_TYPE )
        && aArr[pStd->sti]!=RES_NONE    // Default-Style bekannt
        && !( pIo->nIniFlags & WW8FL_NO_DEFSTYLES ) ) // nicht abgeschaltet
    {
        SwTxtFmtColl* pCol =
            pIo->rDoc.GetTxtCollFromPoolSimple(aArr[pStd->sti], false);
        if(pCol)
        {
            *pbStyExist = true;
            return pCol;
        }
    }
    String aName( rName );
    xub_StrLen nPos = aName.Search( ',' );
    if( STRING_NOTFOUND != nPos )   // mehrere Namen mit Komma getrennt
        aName.Erase( nPos );        // gibts im SW nicht
    SwTxtFmtColl* pCol = SearchFmtColl( aName );
    if( pCol )
    {
        *pbStyExist = true;
        return pCol;
    }
    *pbStyExist = false;
    return MakeNewFmtColl( pStd, aName );   // nicht gefunden
}

void WW8RStyle::Import1Style( USHORT nNr )
{
    SwWW8StyInf* pSI = &pIo->pCollA[nNr];

    if( pSI->bImported || !pSI->bValid )
        return;

    pSI->bImported = true;                      // jetzt schon Flag setzen
                                                // verhindert endlose Rekursion
                                                //
    // gueltig und nicht NIL und noch nicht Importiert

    if( pSI->nBase < cstd && !pIo->pCollA[pSI->nBase].bImported )
        Import1Style( pSI->nBase );

    pStStrm->Seek( pSI->nFilePos );

    short nSkip, cbStd;
    String sName;

    WW8_STD* pStd = Read1Style( nSkip, &sName, &cbStd );// lies Style

    if (pStd)
        pSI->SetOrgWWIdent( sName, pStd->sti );

    // either no Name or unused Slot or unknown Style

    if ( !pStd || (0 == sName.Len()) || ((1 != pStd->sgc) && (2 != pStd->sgc)) )
    {
        pStStrm->SeekRel( nSkip );
        return;
    }
                                                // echter Para- oder Char-Style
    SwFmt* pColl;
    bool bStyExist;
    if (pStd->sgc == 1)             // Para-Style
        pColl = MakeOrGetFmtColl( &bStyExist, pStd, sName );
    else                                // Char-Style
        pColl = MakeOrGetCharFmt( &bStyExist, pStd, sName );

    bool bImport = !bStyExist || pIo->mbNewDoc; // Inhalte Importieren ?
    bool bOldNoImp = pIo->bNoAttrImport;
    pSI->bImportSkipped = !bImport;

    if( !bImport )
        pIo->bNoAttrImport = true;
    else
    {
        if (bStyExist)
            pColl->ResetAllAttr();
        pColl->SetAuto(false);          // nach Empfehlung JP
    }                                   // macht die UI aber anders
    pIo->pAktColl = pColl;
    pSI->pFmt = pColl;                  // UEbersetzung WW->SW merken
    pSI->bImportSkipped = !bImport;

    // Setze Based on
    USHORT j = pSI->nBase;
    if (j != nNr && j < cstd )
    {
        SwWW8StyInf* pj = &pIo->pCollA[j];
        if (pSI->pFmt && pj->pFmt && pSI->bColl == pj->bColl)
        {
            pSI->pFmt->SetDerivedFrom( pj->pFmt );  // ok, Based on eintragen
            pSI->eLTRFontSrcCharSet = pj->eLTRFontSrcCharSet;
            pSI->eRTLFontSrcCharSet = pj->eRTLFontSrcCharSet;
            pSI->nLeftParaMgn = pj->nLeftParaMgn;
            pSI->nTxtFirstLineOfst = pj->nTxtFirstLineOfst;
            pSI->n81Flags = pj->n81Flags;
            pSI->n81BiDiFlags = pj->n81BiDiFlags;
            pSI->bInvisFlag = pj->bInvisFlag;

            if (pj->pWWFly)
                pSI->pWWFly = new WW8FlyPara(pIo->bVer67, pj->pWWFly);
        }
    }
    else if( pIo->mbNewDoc && bStyExist )
    {
        if( pStd->sgc == 1 )
            pSI->pFmt->SetDerivedFrom( pIo->pStandardFmtColl );
        else
            pSI->pFmt->SetDerivedFrom( pIo->rDoc.GetDfltCharFmt() );
    }

    pSI->nFollow = pStd->istdNext;              // Follow merken

    long nPos = pStStrm->Tell();        // falls etwas falsch interpretiert
                                    // wird, gehts danach wieder richtig

    pStyRule = 0;                   // falls noetig, neu anlegen
    bTxtColChanged = bFontChanged = bCJKFontChanged = bCTLFontChanged =
        bFSizeChanged = bFCTLSizeChanged = bWidowsChanged = false;
    pIo->SetNAktColl( nNr );
    pIo->bStyNormal = nNr == 0;

    if( pStd && ( pStd->sgc == 1 || pStd->sgc == 2 ) )
    {
        //Variable parts of the STD start at even byte offsets, but "inside
        //the STD", which I take to meaning even in relation to the starting
        //position of the STD, which matches findings in #89439#, generally it
        //doesn't matter as the STSHI starts off nearly always on an even
        //offset

        //Import of the Style Contents
        ImportGrupx( nSkip, pStd->sgc == 1, pSI->nFilePos & 1);
                        // Alle moeglichen Attribut-Flags zuruecksetzen,
                        // da es in Styles keine Attr-Enden gibt
        pIo->bHasBorder = pIo->bShdTxtCol = pIo->bCharShdTxtCol
        = pIo->bSpec = pIo->bObj = pIo->bSymbol = false;
        pIo->nCharFmt = -1;
    }


    // If Style basiert auf Nichts oder Basis ignoriert
    if( pStd && (pSI->nBase >= cstd || pIo->pCollA[pSI->nBase].bImportSkipped)
        && pStd->sgc == 1 )
    {
        //! Char-Styles funktionieren aus
        // unerfindlichen Gruenden nicht
        // -> dann evtl. harte WW-Defaults
        // reinsetzen
        Set1StyleDefaults();
    }

    pStyRule = 0;                   // zur Sicherheit
    pIo->bStyNormal = false;
    pIo->SetNAktColl( 0 );
    pIo->bNoAttrImport = bOldNoImp;
    // rasch nochmal die Listen-Merk-Felder zuruecksetzen,
    // fuer den Fall dass sie beim einlesen des Styles verwendet wurden
    pIo->nLFOPosition = USHRT_MAX;
    pIo->nListLevel = WW8ListManager::nMaxLevel;

    pStStrm->Seek( nPos+nSkip );

    DELETEZ( pStd );
}

void WW8RStyle::RecursiveReg(USHORT nNr)
{
    SwWW8StyInf* pSI = &pIo->pCollA[nNr];
    if (pSI)
    {
        if( pSI->bImported || !pSI->bValid )
            return;

        pSI->bImported = true;

        if( pSI->nBase < cstd && !pIo->pCollA[pSI->nBase].bImported )
            RecursiveReg(pSI->nBase);

        pIo->RegisterNumFmtOnStyle(nNr);
        pIo->NeedAdjustStyleTabStops(pIo->pCollA[nNr].nLeftParaMgn,
            pIo->pCollA[nNr].nTxtFirstLineOfst,pSI);
    }
}

/*
 After all styles are imported then we can recursively apply numbering
 styles to them, and change their tab stop settings if they turned out
 to have special first line indentation.
*/
void WW8RStyle::PostProcessStyles()
{
    USHORT i;
    /*
     Clear all imported flags so that we can recursively apply numbering
     formats and use it to mark handled ones
    */
    for (i=0; i < cstd; ++i)
        pIo->pCollA[i].bImported = false;

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
        if (pIo->pCollA[i].bValid)
        {
            RecursiveReg(i);
        }
    }
}

void WW8RStyle::ScanStyles()        // untersucht Style-Abhaengigkeiten
{                               // und ermittelt die Filepos fuer jeden Style
/*
    WW8_FC nStyleStart = rFib.fcStshf;
    pStStrm->Seek( nStyleStart );
    */
    USHORT i;
    for( i=0; i<cstd; i++ ){
        short nSkip;
        SwWW8StyInf* pSI = &pIo->pCollA[i];

        pSI->nFilePos = pStStrm->Tell();        // merke FilePos
        WW8_STD* pStd = Read1Style( nSkip, 0, 0 );  // read STD
        pSI->bValid   = ( 0 != pStd );
        if( pSI->bValid
            && !(pStd->sgc == 2 && ( pIo->nIniFlags & WW8FL_NO_ZSTYLES ) ) ){
            pSI->nBase = pStd->istdBase;        // merke Basis
            pSI->bColl = ( pStd->sgc == 1 );    // Para-Style
            pSI->bValid = true;
        }
        else
            memset( &pSI , 0, sizeof( &pSI ) );

        delete pStd;
        pStStrm->SeekRel( nSkip );              // ueberlese Namen und Sprms
    }
}

void WW8RStyle::Import()
{
    pIo->pDfltTxtFmtColl  = pIo->rDoc.GetDfltTxtFmtColl();
    pIo->pStandardFmtColl =
        pIo->rDoc.GetTxtCollFromPoolSimple(RES_POOLCOLL_STANDARD, false);

    if( pIo->nIniFlags & WW8FL_NO_STYLES )
        return;

    ScanStyles();                       // Scanne Based On

    USHORT i;
    for( i=0; i<cstd; i++ ) // Styles importieren
        if( pIo->pCollA[i].bValid )
            Import1Style( i );

    for( i=0; i<cstd; i++ )
    {   // Follow einstellen
        SwWW8StyInf* pi = &pIo->pCollA[i];
        USHORT j = pi->nFollow;
        if( j < cstd )
        {
            SwWW8StyInf* pj = &pIo->pCollA[j];
            if ( j != i                             // sinnvoller Index ?
                 && pi->pFmt                        // Format ok ?
                 && pj->pFmt                        // Derived-Format ok ?
                 && pi->bColl                       // geht nur bei Absatz-Vorlagen (WW)
                 && pj->bColl ){                    // beides gleicher Typ ?
                    ( (SwTxtFmtColl*)pi->pFmt )->SetNextTxtFmtColl(
                     *(SwTxtFmtColl*)pj->pFmt );    // ok, eintragen
            }
        }
    }
// Die Sonderbehandlung zur Setzen der
// Default-Zeichenvorlage "Absatz-Standardschriftart" ( Style-ID 65 ) fehlt
// Sie ist aber defaultmaessig leer ( WW6 dt und US ) und von der
// WW-UI nicht zu veraendern, so dass das nicht stoert.
// Der Mechanismus waere folgender:
//  if( bNew ) rDoc.SetDefault( pDefCharFmt->GetAttrSet() );
//
    // fuer z.B. Tabellen wird ein immer gueltiger Std-Style gebraucht

    if( pIo->pCollA[0].pFmt && pIo->pCollA[0].bColl && pIo->pCollA[0].bValid )
        pIo->pDfltTxtFmtColl = (SwTxtFmtColl*)pIo->pCollA[0].pFmt;
    else
        pIo->pDfltTxtFmtColl = pIo->rDoc.GetDfltTxtFmtColl();


    // set Hyphenation flag on BASIC para-style
    if (pIo->mbNewDoc && pIo->pStandardFmtColl)
    {
        if (pIo->pWDop->fAutoHyphen
            && SFX_ITEM_SET != pIo->pStandardFmtColl->GetItemState(
                                            RES_PARATR_HYPHENZONE, false) )
        {
            SvxHyphenZoneItem aAttr(true);
            aAttr.GetMinLead()    = 2;
            aAttr.GetMinTrail()   = 2;
            aAttr.GetMaxHyphens() = 0;

            pIo->pStandardFmtColl->SetAttr( aAttr );
        }

        /*
        Word defaults to ltr not from environment like writer. Regardless of
        the page/sections rtl setting the standard style lack of rtl still
        means ltr
        */
        if (SFX_ITEM_SET != pIo->pStandardFmtColl->GetItemState(RES_FRAMEDIR,
            false))
        {
           pIo->pStandardFmtColl->SetAttr(
                SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP));
        }
    }

    // wir sind jetzt nicht mehr beim Style einlesen:
    pIo->pAktColl = 0;
}

CharSet SwWW8StyInf::GetCharSet() const
{
    if ((pFmt) && (pFmt->GetFrmDir().GetValue() == FRMDIR_HORI_RIGHT_TOP))
        return eRTLFontSrcCharSet;
    return eLTRFontSrcCharSet;
}

//-----------------------------------------
//      Document Info
//-----------------------------------------

// ReadDocInfo() liegt hier und nicht nicht in wwpar.cxx::LoadDoc1(),
// da hier das noetige sfxcore.hxx bereits includet ist.

void SwWW8ImplReader::ReadDocInfo()
{
    if( pStg )
    {
        SfxDocumentInfo* pNeu;
        if( rDoc.GetpInfo() )                           // soo ein Umstand......
            pNeu = new SfxDocumentInfo( *rDoc.GetpInfo() );
        else
            pNeu = new SfxDocumentInfo();

        pNeu->LoadPropertySet( pStg );  // DocInfo laden
        rDoc.SetInfo( *pNeu );
        delete pNeu;
    }
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
