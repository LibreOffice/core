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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svtools/rtftoken.h>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <swparrtf.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <swtblfmt.hxx>
#include <wrtswtbl.hxx>
#include <tblenum.hxx>
#include <frmatr.hxx>
#include <fmtrowsplt.hxx>

using namespace ::com::sun::star;

typedef SwTableBoxFmt* SwTableBoxFmtPtr;
SV_DECL_PTRARR( SwBoxFrmFmts, SwTableBoxFmtPtr, 25, 50 )

class SwShareBoxFmts;
extern void _DeleteBox( SwTable& rTbl, SwTableBox* pBox, SwUndo* = 0,
                    BOOL = TRUE, const BOOL = TRUE, SwShareBoxFmts* = 0 );

struct Row
{
    bool mbUseLeftRowPad, mbUseRightRowPad, mbUseTopRowPad, mbUseBottomRowPad;
    long mnLeftRowPad, mnRightRowPad, mnTopRowPad, mnBottomRowPad;
    USHORT mnBrdDist;
    Row() :
        mbUseLeftRowPad(false), mbUseRightRowPad(false),
        mbUseTopRowPad(false), mbUseBottomRowPad(false),
        mnLeftRowPad(0), mnRightRowPad(0), mnTopRowPad(0), mnBottomRowPad(0),
        mnBrdDist(MIN_BORDER_DIST)
    {}
};

static void SetRowBorder(SfxItemSet& rSet, const Row &rRow)
{
#if 1
    SvxBoxItem aBox((const SvxBoxItem&)rSet.Get(RES_BOX, false));
    aBox.SetDistance( static_cast< USHORT >(rRow.mbUseLeftRowPad ? rRow.mnLeftRowPad : rRow.mnBrdDist),
            BOX_LINE_LEFT);

    aBox.SetDistance( static_cast< USHORT >(rRow.mbUseRightRowPad ? rRow.mnRightRowPad : rRow.mnBrdDist),
            BOX_LINE_RIGHT);

    aBox.SetDistance( static_cast< USHORT >(rRow.mbUseTopRowPad ? rRow.mnTopRowPad : 0),
            BOX_LINE_TOP);

    aBox.SetDistance( static_cast< USHORT >(rRow.mbUseBottomRowPad ? rRow.mnBottomRowPad : 0),
            BOX_LINE_BOTTOM);

    rSet.Put(aBox);
#else
    const SfxPoolItem* pItem;
    if (SFX_ITEM_SET == rSet.GetItemState(RES_BOX, FALSE, &pItem))
    {
        SvxBoxItem aBox( *(SvxBoxItem*)pItem );
        aBox.SetDistance(rRow.mbUseLeftRowPad ? rRow.mnLeftRowPad : rRow.mnBrdDist,
                BOX_LINE_LEFT);

        aBox.SetDistance(rRow.mbUseRightRowPad ? rRow.mnRightRowPad : rRow.mnBrdDist,
                BOX_LINE_RIGHT);

        aBox.SetDistance(rRow.mbUseTopRowPad ? rRow.mnTopRowPad : 0,
                BOX_LINE_TOP);

        aBox.SetDistance(rRow.mbUseBottomRowPad ? rRow.mnBottomRowPad : 0,
                BOX_LINE_BOTTOM);

        rSet.Put(aBox);
    }
#endif
}

void rtfSections::PrependedInlineNode(const SwPosition &rPos,
    const SwNode &rNode)
{
    OSL_ENSURE(!mrReader.IsNewDoc() || !maSegments.empty(),
        "should not be possible, must be at least one segment in a new document");
    if ((!maSegments.empty()) && (maSegments.back().maStart == rPos.nNode))
        maSegments.back().maStart = SwNodeIndex(rNode);
}

bool SwRTFParser::IsBorderToken(int nToken)
{
    /*
        i30222 i28983
        Our ability to sense border tokens is broken rtftoken.h is
        organised in a way that ignores some border tokens. ReadBorderAttr
        still doesn't support the more exotic borders but at least this
        won't cause the parser to prematuerely exit the table
    */
    bool bResult = false;

    bResult =   (nToken >= RTF_BRDRDASHD && nToken <= RTF_BRDRTHTNMG) ||
                (nToken >=  RTF_BRDRTNTHSG && nToken <= RTF_BRDRWAVY);

    return bResult;
}

void SwRTFParser::ReadTable( int nToken )
{
    nInsTblRow = USHRT_MAX;

    if (CantUseTables())
    {
        // alle Tabellen-Tokens ueberlesen
        nToken = GetNextToken();        // RTF_TROWD ueberlesen
        do {
            if( RTF_TABLEDEF != (nToken & ~(0xff | RTF_SWGDEFS)) &&
                RTF_UNKNOWNCONTROL != nToken )
            {
                SkipToken( -1 );
                break;
            }
            nToken = GetNextToken();
        } while( IsParserWorking() );
        return ;
    }

    // verhinder Tabelle in Tabelle/Footnote
    SwTwips nTblSz = 0;
    int bReadNewCell = FALSE, bChkExistTbl = FALSE;


    enum Limits {eMAXCELLS=64000};

    SvBools aMergeBackup;
    int nCount = aMergeBoxes.Count();
    for (USHORT i = 0; i < nCount; ++i)
        aMergeBackup.Insert(aMergeBoxes[i], i);

    // kein TROWD aber ein TabellenToken -> zwischen TROWD und Tab.Token
    // waren andere Zeichen (siehe Bug 27445.rtf)
    if( RTF_TROWD == nToken || !pTableNode )
    {
        if( RTF_TROWD == nToken )
            nToken = GetNextToken();        // RTF_TROWD ueberlesen

        // Flags fuer die gemergten Boxen loeschen
        aMergeBoxes.Remove( 0, aMergeBoxes.Count() );
        aMergeBoxes.Insert( (BOOL)FALSE, USHORT(0) );
        nAktBox = 0;

        // wenn schon in einer Tabellen, dann splitte oder benutze
        // die bisherigen Boxen weiter
        bChkExistTbl = 0 != pDoc->GetNodes()[ pPam->GetPoint()->nNode ]->FindTableNode();
    }
    else
    {
        bReadNewCell = TRUE;
        SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
        SwTableLine* pLine = rLns[ rLns.Count()-1 ];
        // very robust to avoid crashes like bug 127425 + crash reports 118743
        if( pLine )
        {
            USHORT nTmpBox = nAktBox;
            if( nTmpBox > pLine->GetTabBoxes().Count() )
                nTmpBox = pLine->GetTabBoxes().Count();

            for( USHORT n = nTmpBox; n; )
            {
                const SwTableBox *pTmp = pLine->GetTabBoxes()[ --n ];
                if( pTmp )
                {
                    const SwFrmFmt* pTmpFmt = pTmp->GetFrmFmt();
                    if( pTmpFmt )
                        nTblSz += pTmpFmt->GetFrmSize().GetWidth();
                }
            }
        }
    }


    sal_Int16 eAdjust = text::HoriOrientation::LEFT;      // default fuer Tabellen
    SwTwips nLSpace = 0;
    Row aRow;

    bool bUseLeftCellPad = false, bUseRightCellPad = false,
        bUseTopCellPad = false, bUseBottomCellPad = false;
    long nLeftCellPad = 0, nRightCellPad = 0, nTopCellPad = 0,
        nBottomCellPad = 0;

    sal_Int16 eVerOrient = text::VertOrientation::NONE;
    long nLineHeight = 0;
    USHORT nBoxCnt = aMergeBoxes.Count()-1;
    SwBoxFrmFmts aBoxFmts;
    SwTableBoxFmt* pBoxFmt = pDoc->MakeTableBoxFmt();
    SvxFrameDirection eDir = FRMDIR_HORI_LEFT_TOP;
    bool bCantSplit = false;

    int bWeiter = TRUE;
    do {
        switch( nToken )
        {
        case RTF_TRPADDFL:
            aRow.mbUseLeftRowPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_TRPADDFT:
            aRow.mbUseTopRowPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_TRPADDFR:
            aRow.mbUseRightRowPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_TRPADDFB:
            aRow.mbUseBottomRowPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_TRPADDL:
            aRow.mnLeftRowPad = nTokenValue;
            break;
        case RTF_TRPADDT:
            aRow.mnTopRowPad = nTokenValue;
            break;
        case RTF_TRPADDR:
            aRow.mnRightRowPad = nTokenValue;
            break;
        case RTF_TRPADDB:
            aRow.mnBottomRowPad = nTokenValue;
            break;

        case RTF_CLPADFL:
            bUseLeftCellPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_CLPADFT:
            bUseTopCellPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_CLPADFR:
            bUseRightCellPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_CLPADFB:
            bUseBottomCellPad = (nTokenValue == 3) ? true : false;
            break;
        case RTF_CLPADL:
            nLeftCellPad = nTokenValue;
            break;
        case RTF_CLPADT:
            nTopCellPad = nTokenValue;
            break;
        case RTF_CLPADR:
            nRightCellPad = nTokenValue;
            break;
        case RTF_CLPADB:
            nBottomCellPad = nTokenValue;
            break;

        case RTF_TRRH:
            nLineHeight = nTokenValue;
            break;

        case RTF_CLMRG:
            // would crash later on reading \cellx (#i112657#):
            // the first cell cannot be merged with earlier ones.
            if (nBoxCnt != 0)
            {
                aMergeBoxes[ nBoxCnt ] = TRUE;
            }
            break;

        case RTF_CELLX:
            if (!bTrowdRead) {
                SwTableBoxFmt* pFmt = pBoxFmt;
                SwTwips nSize = nTokenValue - nTblSz;
                if( aMergeBoxes[ nBoxCnt ] )
                {
                    // neue Zellen lesen und noch keine Formate vorhanden,
                    // dann benutze das der vorhergebende
                    if( bReadNewCell && !aBoxFmts.Count() )
                    {
                        SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
                        SwTableLine* pLine = rLns[ rLns.Count()-1 ];
                        if(nAktBox!=0)
                            --nAktBox;
                        pFmt = (SwTableBoxFmt*)pLine->GetTabBoxes()[ nAktBox ]->GetFrmFmt();
                    }
                    else
                        pFmt = aBoxFmts[ aBoxFmts.Count()-1 ];

                    // #i73790# - method renamed
                    pBoxFmt->ResetAllFmtAttr();

                    nSize += pFmt->GetFrmSize().GetWidth();
                }
                else
                {
                  //
                  if (nSize<=2*aRow.mnBrdDist) {
                    aRow.mnRightRowPad=0;
                    aRow.mbUseRightRowPad=TRUE;
                  }
                    SetRowBorder((SfxItemSet&)pBoxFmt->GetAttrSet(), aRow);
                    aBoxFmts.Insert( pBoxFmt, aBoxFmts.Count() );
                    pBoxFmt = pDoc->MakeTableBoxFmt();
                }

                if( !nSize )
                    nSize = COL_DFLT_WIDTH;
                pFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nSize, 0 ));
                nTblSz = nTokenValue;
                aMergeBoxes.Insert( (BOOL)FALSE, ++nBoxCnt );

                SvxBoxItem aBox(pFmt->GetBox());

                if (bUseRightCellPad)
                    aBox.SetDistance( static_cast< USHORT >(nRightCellPad), BOX_LINE_RIGHT);
                if (bUseBottomCellPad)
                    aBox.SetDistance( static_cast< USHORT >(nBottomCellPad), BOX_LINE_BOTTOM);

                //Yes, these are the wrong way around, there appears to
                //be a bug in word where these are swapped.
                if (bUseLeftCellPad)
                    aBox.SetDistance( static_cast< USHORT >(nLeftCellPad), BOX_LINE_TOP);
                if (bUseTopCellPad)
                    aBox.SetDistance( static_cast< USHORT >(nTopCellPad), BOX_LINE_LEFT);


                pFmt->SetFmtAttr(aBox);

                bUseLeftCellPad = false;
                bUseRightCellPad = false;
                bUseTopCellPad = false;
                bUseBottomCellPad = false;
            }
            break;

        case RTF_TRGAPH:
                //$flr bug #117887#: RTF: wrong internal table cell margin imported (A13)
                aRow.mnBrdDist = (nTokenValue>0?(USHORT)nTokenValue:0); // filter out negative values of \trgaph
            break;

        case RTF_TRQL:          eAdjust = text::HoriOrientation::LEFT;    break;
        case RTF_TRQR:          eAdjust = text::HoriOrientation::RIGHT;   break;
        case RTF_TRQC:          eAdjust = text::HoriOrientation::CENTER;  break;

                                // mit text::VertOrientation::TOP kommt der Dialog nicht klar!
        case RTF_CLVERTALT:     eVerOrient = text::VertOrientation::NONE;     break;

        case RTF_CLVERTALC:     eVerOrient = text::VertOrientation::CENTER;   break;
        case RTF_CLVERTALB:     eVerOrient = text::VertOrientation::BOTTOM;   break;

        case RTF_TRLEFT:
            if( text::HoriOrientation::LEFT == eAdjust )
                eAdjust = text::HoriOrientation::LEFT_AND_WIDTH;
            nLSpace = nTokenValue;
            nTblSz = nTokenValue;
            break;

        case RTF_TRHDR:
            nRowsToRepeat++;
            break;

        case RTF_CLTXLRTB:
        case RTF_CLTXTBRL:
        case RTF_INTBL:     // das wissen wir !
        case RTF_CLMGF:
        case RTF_CLVMGF:
        case RTF_CLVMRG:
            break;
        case RTF_LTRROW:
            eDir = FRMDIR_HORI_LEFT_TOP;
            break;
        case RTF_RTLROW:
            eDir = FRMDIR_HORI_RIGHT_TOP;
            break;
        case RTF_TRBRDRB:
        case RTF_TRBRDRH:
        case RTF_TRBRDRL:
        case RTF_TRBRDRR:
        case RTF_TRBRDRT:
        case RTF_TRBRDRV:
                break;
        case RTF_TRKEEP:
                bCantSplit = true;
                break;

        default:
            if( ( nToken & ~(0xff | RTF_TABLEDEF)) == RTF_SHADINGDEF )
            {
                if( aMergeBoxes[ nBoxCnt ] )
                    break;
                ReadBackgroundAttr( nToken,
                        (SfxItemSet&)pBoxFmt->GetAttrSet(), TRUE );
            }
            else if( ( nToken & ~(0xff | RTF_TABLEDEF) ) == RTF_BRDRDEF ||
                      IsBorderToken(nToken))
            {
                if( aMergeBoxes[ nBoxCnt ] )
                    break;

                SfxItemSet& rSet = (SfxItemSet&)pBoxFmt->GetAttrSet();
                if(!IsBorderToken( nToken ))
                    ReadBorderAttr( nToken, rSet, TRUE );
                else
                    NextToken( nToken );
            }
            else if( RTF_TABLEDEF != (nToken & ~(0xff | RTF_SWGDEFS)) )
            {
                if( RTF_UNKNOWNCONTROL == nToken )
                    NextToken( nToken );
                else
                    bWeiter = FALSE;
            }
            break;
        }

        if( text::VertOrientation::NONE != eVerOrient )
        {
            if( !aMergeBoxes[ nBoxCnt ] )
                pBoxFmt->SetFmtAttr( SwFmtVertOrient( 0, eVerOrient ));
            eVerOrient = text::VertOrientation::NONE;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );

    // das letzte temp. BoxFmt loeschen
    delete pBoxFmt;

    // es wurde keine einzige Box erkannt
    if( nAktBox == nBoxCnt || ( bReadNewCell && !pTableNode ))
    {
        int nC = aMergeBackup.Count();
        for (USHORT i = 0; i < nC; ++i)
            aMergeBoxes.Insert(aMergeBackup[i], i);
        SkipToken( -1 );            // zum Letzen gueltigen zurueck
        return;
    }

    nTblSz -= nLSpace;

    int bNewTbl = TRUE;
    SwTableLine* pNewLine;
    bTrowdRead=true;

    // lege eine neue Tabelle an oder erweiter die aktuelle um eine neue Line
    // oder Box !
    SwNode* pNd;

    if( bChkExistTbl )
    {
        // es ist eine Tabelle vorhanden, teste mal ob die weiter benutzt
        // werden kann oder ob sie abgesplittet und neu gefuellt werden
        // muss.
        pTableNode = pPam->GetNode()->FindTableNode();

        // Cursor kann nur in der letzten Line stehen

        // das Attribut darf nicht ueber das Modify an der
        // Tabelle gesetzt werden, denn sonst werden alle
        // Boxen wieder auf 0 zurueck gesetzt !!!!!
        SwFrmFmt* pFmt = pTableNode->GetTable().GetFrmFmt();
        const SwFmtFrmSize& rTblSz = pFmt->GetFrmSize();
        const SwFmtHoriOrient& rHoriz = pFmt->GetHoriOrient();

        const SwTableLines* pLns = &pTableNode->GetTable().GetTabLines();

        if( 1 == pLns->Count() )
        {
            if( eAdjust != rHoriz.GetHoriOrient() )
            {
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( SwFmtHoriOrient( 0,
                                                            eAdjust ) );
            }
            if( rTblSz.GetWidth() != nTblSz )
            {
                SwFmtFrmSize aSz( rTblSz );
                aSz.SetWidth( nTblSz );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );
            }

            if( text::HoriOrientation::LEFT_AND_WIDTH == eAdjust &&
                nLSpace != pFmt->GetLRSpace().GetLeft() )
            {
                SvxLRSpaceItem aL( RES_LR_SPACE ); aL.SetLeft( nLSpace );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aL );
            }
        }
        else if
            (
              1 < pLns->Count() &&
              (
                rTblSz.GetWidth() != nTblSz ||
                rHoriz.GetHoriOrient() != eAdjust ||
                (
                  text::HoriOrientation::LEFT_AND_WIDTH == eAdjust &&
                  nLSpace != pFmt->GetLRSpace().GetLeft()
                ) ||
                pTableNode->GetTable().GetTabSortBoxes().Count() >= eMAXCELLS
              )
            )
        {
            // Tabelle ab der PaM-Position splitten
            // die vorherige Line!
            pNewLine = (*pLns)[ pLns->Count() - 2 ];
            SwTableBox* pBox = pNewLine->GetTabBoxes()[ 0 ];
            while( ( pLns = &pBox->GetTabLines() )->Count() )
                pBox = (*pLns)[ 0 ]->GetTabBoxes()[ 0 ];

            SwNodeIndex aTmpIdx( *pBox->GetSttNd() );
            pDoc->GetNodes().SplitTable( aTmpIdx, HEADLINE_NONE, FALSE );
            pTableNode = pPam->GetNode()->FindTableNode();
            pFmt = pTableNode->GetTable().GetFrmFmt();

            SwFmtFrmSize aSz( rTblSz );
            aSz.SetWidth( nTblSz );
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( SwFmtHoriOrient( 0,
                                                            eAdjust ) );
            if( text::HoriOrientation::LEFT_AND_WIDTH == eAdjust && nLSpace )
            {
                SvxLRSpaceItem aL( RES_LR_SPACE ); aL.SetLeft( nLSpace );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aL );
            }
        }

        pLns = &pTableNode->GetTable().GetTabLines();
        pNewLine = (*pLns)[ pLns->Count() - 1 ];

        // jetzt die Boxen abgleichen
        USHORT nBoxes = Min( pNewLine->GetTabBoxes().Count(), aBoxFmts.Count() );
        USHORT n;

        for( n = 0; n < nBoxes; ++n )
        {
            SwTableBox* pBox = pNewLine->GetTabBoxes()[ n ];
            *pBox->GetFrmFmt() = *aBoxFmts[ n ];
            delete aBoxFmts[ n ];
        }
        aBoxFmts.Remove( 0, n );

        if( aBoxFmts.Count() )      // es muessen noch neue zugefuegt werden
            nAktBox = n;
        else                        // es mussen noch Boxen geloescht werden
        {
            // remove ContentIndex of other Bound
            pPam->SetMark(); pPam->DeleteMark();
            while( n < pNewLine->GetTabBoxes().Count() )
                _DeleteBox( pTableNode->GetTable(),
                            pNewLine->GetTabBoxes()[ n ], 0, FALSE, FALSE );
        }

        pOldTblNd = pTableNode;
        bNewTbl = FALSE;

        {
            //TabellenUmrandungen optimieren
            void* p = pFmt;
            aTblFmts.Insert( p, aTblFmts.Count() );
        }
    }
    else
    {
        if( !bReadNewCell && ( pNd = pDoc->GetNodes()[
            pPam->GetPoint()->nNode.GetIndex()-1 ])->IsEndNode() )
        {
            pTableNode = pNd->StartOfSectionNode()->GetTableNode();
            if( pTableNode )
            {
                // dann test mal ob wirklich nur eine neue Line eingefuegt
                // werden soll!
                SwTable &rTable = pTableNode->GetTable();
                SwFrmFmt* pFmt = rTable.GetFrmFmt();
                const SwFmtFrmSize& rTblSz = pFmt->GetFrmSize();
                const SwFmtHoriOrient& rHoriz = pFmt->GetHoriOrient();
                if (
                    rTblSz.GetWidth() != nTblSz ||
                    rHoriz.GetHoriOrient() != eAdjust ||
                    rTable.GetTabSortBoxes().Count() >= eMAXCELLS
                    )
                {
                    pTableNode = 0;
                }
            }
        }

        if( pTableNode && !bForceNewTable)
        {

            // das Attribut darf nicht ueber das Modify an der
            // Tabelle gesetzt werden, denn sonst werden alle
            // Boxen wieder auf 0 zurueck gesetzt !!!!!
            SwFrmFmt* pFmt = pTableNode->GetTable().GetFrmFmt();
            const SwFmtFrmSize& rTblSz = pFmt->GetFrmSize();
            if( rTblSz.GetWidth() < nTblSz )
            {
                SwFmtFrmSize aSz( rTblSz );
                aSz.SetWidth( nTblSz );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );
            }

            SwTableLines& rLns = pTableNode->GetTable().GetTabLines();

            if( bReadNewCell )
                pNewLine = rLns[ rLns.Count()-1 ];
            else
            {
                pNewLine = new SwTableLine(
                        (SwTableLineFmt*)rLns[ rLns.Count()-1 ]->GetFrmFmt(),
                        aBoxFmts.Count(), 0 );
                pNewLine->ClaimFrmFmt();
                pNewLine->GetFrmFmt()->ResetFmtAttr( RES_FRM_SIZE );
                rLns.C40_INSERT( SwTableLine, pNewLine, rLns.Count() );
            }
            bNewTbl = FALSE;
        }
        else
        {
            bForceNewTable = false;
            const SwTable *pTable =
                pDoc->InsertTable(
                    SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 0 ),
                    *pPam->GetPoint(), 1, 1, eAdjust, 0, 0, FALSE, FALSE );
            bContainsTablePara=true;
            pTableNode = pTable ? pTable->GetTableNode() : 0;

            if (pTableNode)
            {
                maSegments.PrependedInlineNode(*pPam->GetPoint(),
                    *pTableNode);
            }
            else
            {
                SkipToken( -1 );            // zum Letzen gueltigen zurueck
                return;
            }

            SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
            pNewLine = rLns[ rLns.Count()-1 ];

            SwFrmFmt* pFmt = pTableNode->GetTable().GetFrmFmt();
            SwFmtFrmSize aSz( pFmt->GetFrmSize() );
            aSz.SetWidth( nTblSz );
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );
            ((SfxItemSet&)pFmt->GetAttrSet()).Put(SvxFrameDirectionItem(eDir, RES_FRAMEDIR));

            if( text::HoriOrientation::LEFT_AND_WIDTH == eAdjust && nLSpace )
            {
                SvxLRSpaceItem aL( RES_LR_SPACE ); aL.SetLeft( nLSpace );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aL );
            }

            nAktBox = 0;
            pOldTblNd = pTableNode;

            {
                // TabellenUmrandungen optimieren
                void* p = pFmt;
                aTblFmts.Insert( p, aTblFmts.Count() );
            }
        }
    }

    if( nLineHeight )
    {
        SwFrmSize eSize;
        if( 0 > nLineHeight )
            eSize = ATT_FIX_SIZE, nLineHeight = -nLineHeight;
        else
            eSize = ATT_MIN_SIZE;
        pNewLine->ClaimFrmFmt()->SetFmtAttr(SwFmtFrmSize(eSize, 0, nLineHeight));
    }

    pNewLine->ClaimFrmFmt()->SetFmtAttr(SwFmtRowSplit(!bCantSplit));

    if( aBoxFmts.Count() )
    {
        // setze das default Style
        SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
        if( !pColl )
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );

        USHORT nStt = 0;
        if( bNewTbl )
        {
            SwTableBox* pBox = pNewLine->GetTabBoxes()[0];
            pBoxFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
            pBoxFmt->Remove( pBox );
            delete pBoxFmt;
            aBoxFmts[0]->Add( pBox );
            SwTxtNode* pTNd = pDoc->GetNodes()[ pBox->GetSttIdx()+1 ]
                                            ->GetTxtNode();
            OSL_ENSURE( pTNd, "wo ist der Textnode dieser Box?" );
            pTNd->ChgFmtColl( pColl );
            ++nStt;
            nRowsToRepeat=0;
        }

        for( ; nStt < aBoxFmts.Count(); ++nStt )
        {
            pDoc->GetNodes().InsBoxen( pTableNode, pNewLine,
                    aBoxFmts[ nStt ],
                    // Formate fuer den TextNode der Box
                    pColl, 0,
                    nAktBox + nStt, 1 );
        }
    }

    if( bChkExistTbl )
        nAktBox = 0;

    maInsertedTables.InsertTable(*pTableNode, *pPam);

    SwNodeIndex aOldIdx(pPam->GetPoint()->nNode);
    SwNodeIdx aOldPos(aOldIdx);
    SwPaM aRg(*pPam);

    bool bFailure = true;
    if (pNewLine)
    {
        SwTableBoxes &rBoxes = pNewLine->GetTabBoxes();
        if (SwTableBox* pBox = (nAktBox < rBoxes.Count() ? rBoxes[nAktBox] : 0))
        {
            if (const SwStartNode *pStart = pBox->GetSttNd())
            {
                if (const SwEndNode *pEnd = pStart->EndOfSectionNode())
                {
                    pPam->GetPoint()->nNode = *pEnd;
                    pPam->Move( fnMoveBackward, fnGoCntnt );
                    bFailure = false;
                }
            }
        }
    }

    OSL_ENSURE(!bFailure, "RTF Table failure");
    if (bFailure)
    {
        SkipToken( -1 );            // zum Letzen gueltigen zurueck
        return;
    }

    //It might be that there was content at this point which is not already in
    //a table, but which is being followed by properties to place it into the
    //table. e.g. #109199#. If this is the case then move the para/char
    //properties inside the table, and move any content of that paragraph into
    //the table
    bool bInTable = aRg.GetPoint()->nNode.GetNode().FindTableNode();
    if (!bInTable)
    {
        SwNodeIndex aNewIdx(pPam->GetPoint()->nNode);
        SwNodeIdx aNewPos(aNewIdx);

        if (aRg.GetPoint()->nContent.GetIndex())
        {
            //If there is content in this node then move it entirely inside the
            //table
            aRg.SetMark();
            aRg.GetMark()->nContent.Assign(aRg.GetCntntNode(), 0);
            pDoc->MoveRange(aRg, *pPam->GetPoint(),
                    IDocumentContentOperations::DOC_MOVEDEFAULT);
        }

        //Update the attribute stack entries to reflect that the properties
        //which were intended to be inside the tablerow are now left outside
        //the table after the row was placed before the current insertion point
        SvxRTFItemStack& rAttrStk = GetAttrStack();
        for (USHORT n = 0; n < rAttrStk.Count(); ++n)
        {
            SvxRTFItemStackType* pStk = rAttrStk[n];
            pStk->MoveFullNode(aOldPos, aNewPos);
        }
    }
    SkipToken( -1 );            // zum Letzen gueltigen zurueck
}

// in die naechste Box dieser Line (opt.: falls es nicht die letzte ist)
void SwRTFParser::GotoNextBox()
{
    nInsTblRow = USHRT_MAX;

    OSL_ENSURE( pTableNode, "Kein Tabellennode, dann auch keine Box" );

    if (!pTableNode)
        return;

    SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
    SwTableLine* pLine = rLns[ rLns.Count()-1 ];
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    SwTableBox* pBox = rBoxes[ rBoxes.Count()-1 ];

    if( ++nAktBox >= aMergeBoxes.Count() )
        nAktBox = aMergeBoxes.Count()-1;

    if( !aMergeBoxes[ nAktBox ] )
    {
        int bMove = TRUE;
        if( pBox->GetSttIdx() > pPam->GetPoint()->nNode.GetIndex() )
        {
            USHORT nRealBox = 0;
            for( USHORT nTmp = 0; nTmp < nAktBox; ++nTmp )
                if( !aMergeBoxes[ nTmp ] )
                    ++nRealBox;

            if( nRealBox < rBoxes.Count() )
            {
                pPam->GetPoint()->nNode = *rBoxes[ nRealBox ]->GetSttNd()->EndOfSectionNode();
                pPam->Move( fnMoveBackward, fnGoCntnt );
                bMove = FALSE;
            }
        }

        if( bMove && nAktBox + 1 == aMergeBoxes.Count() )
            // dann hinter die Tabelle
            pPam->Move( fnMoveForward, fnGoNode );
    }
    else if( !pDoc->GetNodes()[ pPam->GetPoint()->nNode ]->IsCntntNode() )
        // dann in die vorherige ans Ende
        pPam->Move( fnMoveBackward, fnGoCntnt );
}


void SwRTFParser::NewTblLine()
{
    nInsTblRow = USHRT_MAX;

    // erweiter die aktuelle um eine neue Line
    BOOL bMakeCopy = FALSE;
    SwNode* pNd = pDoc->GetNodes()[ pPam->GetPoint()->nNode.GetIndex()-1 ];
    if( !pNd->IsEndNode() ||
        !(pNd = pNd->StartOfSectionNode())->IsTableNode() )
    {
        if( !pOldTblNd )
            return ;

        bMakeCopy = TRUE;
        pNd = pOldTblNd;
    }
    pTableNode = (SwTableNode*)pNd;

    SwTableLines* pLns = &pTableNode->GetTable().GetTabLines();
    SwTableLine* pLine = (*pLns)[ pLns->Count()-1 ];
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    SwTableBox* pBox = rBoxes[ rBoxes.Count()-1 ];

    if(nRowsToRepeat>0)
        pTableNode->GetTable().SetRowsToRepeat( nRowsToRepeat );

    if( !bMakeCopy &&
        64000 < pTableNode->GetTable().GetTabSortBoxes().Count() )
    {
        bMakeCopy = TRUE;       // spaetestens jetzt eine neue anfangen!
    }

    if( bMakeCopy )
    {
        // und die Selektion kopieren
        SwSelBoxes aBoxes;
        pTableNode->GetTable().SelLineFromBox( pBox, aBoxes );
        pTableNode->GetTable().MakeCopy( pDoc, *pPam->GetPoint(),
                                        aBoxes, FALSE );
        ULONG nNd = pPam->GetPoint()->nNode.GetIndex()-1;
        pTableNode = pDoc->GetNodes()[ nNd ]->FindTableNode();
        pOldTblNd = pTableNode;

        nRowsToRepeat=0;
        pTableNode->GetTable().SetRowsToRepeat(nRowsToRepeat);
        pLns = &pTableNode->GetTable().GetTabLines();
    }
    else
        pTableNode->GetTable().AppendRow( pDoc );

    pBox = (*pLns)[ pLns->Count()-1 ]->GetTabBoxes()[0];

    ULONG nOldPos = pPam->GetPoint()->nNode.GetIndex();
    pPam->GetPoint()->nNode = *pBox->GetSttNd();
    pPam->Move( fnMoveForward );
    nAktBox = 0;

    // alle Nodes in den Boxen auf die "default" Vorlage setzten
    {
        SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
        if( !pColl )
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        pPam->SetMark();

        pLine = (*pLns)[ pLns->Count()-1 ];
        pBox = pLine->GetTabBoxes()[ pLine->GetTabBoxes().Count() -1 ];
        pPam->GetPoint()->nNode = *pBox->GetSttNd()->EndOfSectionNode();
        pPam->Move( fnMoveBackward );
        pDoc->SetTxtFmtColl( *pPam, pColl );
        // Bug 73940 - remove ALL attributes (NumRules/Break/etc.)
        {
            SwNodeIndex aIdx( pPam->GetMark()->nNode );
            SwNodeIndex& rEndIdx = pPam->GetPoint()->nNode;
            while( aIdx <= rEndIdx )
            {
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if( pCNd && pCNd->HasSwAttrSet() )
                    pCNd->ResetAllAttr();
                aIdx++;
            }
        }
        pPam->Exchange();
        pPam->DeleteMark();
    }

    // alle Attribute, die schon auf den nachfolgen zeigen auf die neue
    // Box umsetzen !!
    SvxRTFItemStack& rAttrStk = GetAttrStack();
    const SvxRTFItemStackType* pStk;
    for( USHORT n = 0; n < rAttrStk.Count(); ++n )
        if( ( pStk = rAttrStk[ n ])->GetSttNodeIdx() == ULONG(nOldPos) &&
            !pStk->GetSttCnt() )
            ((SvxRTFItemStackType*)pStk)->SetStartPos( SwxPosition( pPam ) );
}

void SwRTFParser::CheckInsNewTblLine()
{
    if( USHRT_MAX != nInsTblRow )
    {
        if( nInsTblRow > GetOpenBrakets() || IsPardTokenRead() )
            nInsTblRow = USHRT_MAX;
        else if( !pTableNode )      // Tabelle nicht mehr vorhanden ?
            NewTblLine();           // evt. Line copieren
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
