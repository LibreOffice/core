/*************************************************************************
 *
 *  $RCSfile: viewtab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif


#pragma hdrstop

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#include <hintids.hxx>
#include "uiparam.hxx"
#include "uitool.hxx"

#ifndef _SVX_RULRITEM_HXX //autogen
#include <svx/rulritem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"
#include "cmdid.h"
#include "viewopt.hxx"
#include "tabcol.hxx"
#include "frmfmt.hxx"       // FrameFormat
#include "pagedesc.hxx"     // Aktuelles Seitenformat
#include "wview.hxx"
#include "fmtcol.hxx"
#include "section.hxx"

/*--------------------------------------------------------------------
    Beschreibung:   Debug-Methode
 --------------------------------------------------------------------*/
#ifdef DEBUGLIN


static void DebugTabCols(const SwTabCols &rCols)
{
    String aTmp("TabCols LeftMin ");
    aTmp += rCols.GetLeftMin() / 56;
    aTmp += "Left: ";
    aTmp += rCols.GetLeft() / 56;
    aTmp += "Right: ";
    aTmp += rCols.GetRight() / 56;
    aTmp += "RightMax: ";
    aTmp += rCols.GetRightMax() / 56;
    aTmp += ": ";
    const USHORT nCount = rCols.Count();
    for(USHORT i = 0; i < nCount; ++i) {
        aTmp += rCols[i] / 56;
        aTmp += ' ';
    }
    InfoBox(0, aTmp).Execute();
}



static void DebugColItems(SvxColumnItem& rColItem)
{
    String aTmp("Aktuell: ");
    aTmp += rColItem.GetActColumn();
    aTmp += " ColLeft: ";
    aTmp +=  String(rColItem.GetLeft() / 56);
    aTmp += "   ColRight: ";
    aTmp +=  String(rColItem.GetRight() / 56);
    for(USHORT i = 0; i < rColItem.Count(); ++i) {
        aTmp += " Start: ";
        aTmp += String(rColItem[i].nStart/56);
        aTmp += " End: ";
        aTmp += String(rColItem[i].nEnd/56);
    }

    InfoBox(0, aTmp).Execute();
}


static void DebugLRSpace(const SvxLongLRSpaceItem& rLRSpace)
{
    String aTmp("Left: ");
    aTmp += rLRSpace.GetLeft() / 56;
    aTmp += "   Right: ";
    aTmp += rLRSpace.GetRight() / 56;
    InfoBox(0, aTmp).Execute();
}



static void DebugULSpace(const SvxLongULSpaceItem& rULSpace)
{
    String aTmp("Upper: ");
    aTmp += rULSpace.GetUpper() / 56;
    aTmp += "   Lower: ";
    aTmp += rULSpace.GetLower() / 56;

    InfoBox(0, aTmp).Execute();
}



static void DebugTabStops(const SvxTabStopItem& rTabs)
{
    String aTmp("Tabs: ");

    // Def Tabs loeschen
    for(USHORT i = 0; i < rTabs.Count(); ++i)
    {
        aTmp += String(rTabs[i].GetTabPos() / 56);
        aTmp += " : ";
    }
    InfoBox(0, aTmp).Execute();
}



static void DebugParaMargin(const SvxLRSpaceItem& rLRSpace)
{
    String aTmp("ParaLeft: ");
    aTmp += rLRSpace.GetTxtLeft() / 56;
    aTmp += "   ParaRight: ";
    aTmp += rLRSpace.GetRight() / 56;
    aTmp += "   FLI: ";
    aTmp += rLRSpace.GetTxtFirstLineOfst() / 56;
    InfoBox(0, aTmp).Execute();
}
#define DEBUGTABCOLS( bla )     DebugTabCols( bla )
#define DEBUGCOLITEMS( bla )    DebugColItems   ( bla )
#define DEBUGLRSPACE( bla )     DebugLRSpace    ( bla )
#define DEBUGULSPACE( bla )     DebugULSpace    ( bla )
#define DEBUGTABSTOPS( bla )    DebugTabStops   ( bla )
#define DEBUGPARAMARGIN( bla )  DebugParaMargin ( bla )
#else
#define DEBUGTABCOLS
#define DEBUGCOLITEMS
#define DEBUGLRSPACE
#define DEBUGULSPACE
#define DEBUGTABSTOPS
#define DEBUGPARAMARGIN
#endif


/*--------------------------------------------------------------------
    Beschreibung:   Columns eintueten
 --------------------------------------------------------------------*/


void lcl_FillSvxColumn(const SwFmtCol& rCol,
                          USHORT nTotalWidth,
                          SvxColumnItem& rColItem,
                          long nDistance)
{
    const SwColumns& rCols = rCol.GetColumns();

    USHORT nWidth = 0;

    for ( USHORT i = 0; i < rCols.Count(); ++i )
    {
        SwColumn* pCol = rCols[i];
        const USHORT nStart = USHORT(pCol->GetLeft() + nWidth + nDistance);
        nWidth += rCol.CalcColWidth(i, nTotalWidth);
        const USHORT nEnd = USHORT(nWidth - pCol->GetRight() + nDistance);

        SvxColumnDescription aColDesc(nStart, nEnd, TRUE);
        rColItem.Append(aColDesc);
    }

    // ?? gleichverteilte Spalten
}

/*--------------------------------------------------------------------
    Beschreibung:   ColumnItem in ColumnInfo ueberfuehren
 --------------------------------------------------------------------*/


void lcl_ConvertToCols(const SvxColumnItem& rColItem,
                          USHORT nTotalWidth,
                          SwFmtCol& rCols)
{
    ASSERT( rCols.GetNumCols() == rColItem.Count(), "Column count mismatch" );

    USHORT nLeft    = 0;
    SwTwips nSumAll= 0;  // Summiere alle Spalten und Raender auf

    SwColumns& rArr = rCols.GetColumns();

    // Tabcols der Reihe nach
    for( USHORT i=0; i < rColItem.Count()-1; ++i )
    {
        DBG_ASSERT(rColItem[i+1].nStart >= rColItem[i].nEnd,"\201berlappende Spalten" );
        USHORT nStart = rColItem[i+1].nStart;
        USHORT nEnd = rColItem[i].nEnd;
        if(nStart < nEnd)
            nStart = nEnd;
        const USHORT nDiff  = nStart - nEnd;
        const USHORT nRight = nDiff / 2;

        USHORT nWidth = rColItem[i].nEnd - rColItem[i].nStart;
        nWidth += nLeft + nRight;

        SwColumn* pCol = rArr[i];
        pCol->SetWishWidth( USHORT(long(rCols.GetWishWidth()) * long(nWidth) /
                                                            long(nTotalWidth) ));
        pCol->SetLeft( nLeft );
        pCol->SetRight( nRight );
        nSumAll += pCol->GetWishWidth();

        nLeft = nRight;
    }
    rArr[rColItem.Count()-1]->SetLeft( nLeft );

    //Die Differenz aus der Gesamtwunschbreite und der Summe der bisher berechneten
    // Spalten und Raender sollte die Breite der letzten Spalte ergeben.
    rArr[rColItem.Count()-1]->SetWishWidth( rCols.GetWishWidth() - (USHORT)nSumAll );

    rCols.SetOrtho(FALSE, 0, 0 );
}

/*--------------------------------------------------------------------
    Beschreibung:   Tabs loeschen
 --------------------------------------------------------------------*/


void lcl_EraseDefTabs(SvxTabStopItem& rTabStops)
{
    // Def Tabs loeschen
    for ( USHORT i = 0; i < rTabStops.Count(); )
    {
        // Hier auch den DefTab auf Null rausschmeissen
        if ( SVX_TAB_ADJUST_DEFAULT == rTabStops[i].GetAdjustment() ||
            rTabStops[i].GetTabPos() == 0 )
        {
            rTabStops.Remove(i);
            continue;
        }
        ++i;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Seitenrand umdrehen
 --------------------------------------------------------------------*/


void SwView::SwapPageMargin(const SwPageDesc& rDesc, SvxLRSpaceItem& rLRSpace)
{
    USHORT nPhyPage, nVirPage;
    GetWrtShell().GetPageNum( nPhyPage, nVirPage );

    if ( rDesc.GetUseOn() == PD_MIRROR && (nPhyPage % 2) == 0 )
    {
        USHORT nTmp = rLRSpace.GetRight();
        rLRSpace.SetRight( rLRSpace.GetLeft() );
        rLRSpace.SetLeft( nTmp );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Wenn der Rahmenrand verschoben wird, sollen die
                    Spaltentrenner an der gleichen absoluten Position bleiben
 --------------------------------------------------------------------*/


void lcl_Scale(long& nVal, long nScale)
{
    nVal *= nScale;
    nVal >>= 8;
}

void ResizeFrameCols(SwFmtCol& rCol,
                    long nOldWidth,
                    long nNewWidth,
                    long nLeftDelta )
{
    SwColumns& rArr = rCol.GetColumns();
    long nWishSum = (long)rCol.GetWishWidth();
    long nWishDiff = (nWishSum * 100/nOldWidth * nNewWidth) / 100 - nWishSum;
    long nNewWishWidth = nWishSum + nWishDiff;
    if(nNewWishWidth > 0xffffl)
    {
        // wenn die Wunschbreite zu gross wird, dann muessen alle Werte passend skaliert werden
        long nScale = (0xffffl << 8)/ nNewWishWidth;
        for(USHORT i = 0; i < rArr.Count(); i++)
        {
            SwColumn* pCol = rArr.GetObject(i);
            long nVal = pCol->GetWishWidth();
            lcl_Scale(nVal, nScale);
            pCol->SetWishWidth((USHORT) nVal);
            nVal = pCol->GetLeft();
            lcl_Scale(nVal, nScale);
            pCol->SetLeft((USHORT) nVal);
            nVal = pCol->GetRight();
            lcl_Scale(nVal, nScale);
            pCol->SetRight((USHORT) nVal);
        }
        lcl_Scale(nNewWishWidth, nScale);
        lcl_Scale(nWishDiff, nScale);
    }
    rCol.SetWishWidth( (USHORT) (nNewWishWidth) );

    if( nLeftDelta >= 2 || nLeftDelta <= -2)
        rArr[0]->SetWishWidth(rArr[0]->GetWishWidth() + (USHORT)nWishDiff);
    else
        rArr[rArr.Count()-1]->SetWishWidth(rArr[rArr.Count()-1]->GetWishWidth() + (USHORT)nWishDiff);

}
/*--------------------------------------------------------------------
    Beschreibung:   Hier werden alle Aenderungen der Tableiste
                    wieder in das Modell geschossen
 --------------------------------------------------------------------*/


void SwView::ExecTabWin( SfxRequest& rReq )
{
    SwWrtShell &rSh         = GetWrtShell();
    const USHORT nFrmType   = rSh.IsObjSelected() ?
                                    FRMTYPE_DRAWOBJ :
                                        rSh.GetFrmType(0,TRUE);
    const BOOL  bFrmSelection = rSh.IsFrmSelected();
    BOOL bBrowse = rSh.IsBrowseMode();


    const USHORT nSlot      = rReq.GetSlot();
    const USHORT nDescId    = rSh.GetCurPageDesc();
    const SwPageDesc& rDesc = rSh.GetPageDesc( nDescId );

    const SwFmtHeader& rHeaderFmt = rDesc.GetMaster().GetHeader();
    SwFrmFmt *pHeaderFmt = (SwFrmFmt*)rHeaderFmt.GetHeaderFmt();

    const SwFmtFooter& rFooterFmt = rDesc.GetMaster().GetFooter();
    SwFrmFmt *pFooterFmt = (SwFrmFmt*)rFooterFmt.GetFooterFmt();

    const SwFmtFrmSize &rFrmSize = rDesc.GetMaster().GetFrmSize();
    const SwRect& rPrtRect = rSh.GetAnyCurRect(RECT_PAGE);
    const long nPageWidth  = bBrowse ?
                                rPrtRect.Width() :
                                    rFrmSize.GetWidth();
    const long nPageHeight = bBrowse ?
                                rPrtRect.Height() :
                                    rFrmSize.GetHeight();
    const SfxItemSet* pArgs = rReq.GetArgs();
    // RULER_BORDERS wird sicherheitshalber nicht recorded
    // da gibt es Aerger im Sfx
    const SfxPoolItem* pItem;

    BOOL bUnlockView = FALSE;
    rSh.StartAllAction();

    switch  ( nSlot )
    {
    case RES_LR_SPACE:
    case SID_ATTR_LRSPACE:
    {
        if(SFX_ITEM_SET == pArgs->GetItemState(RES_LR_SPACE, FALSE, &pItem))
        {
            const  SvxLRSpaceItem* pLR = (const SvxLRSpaceItem*)pItem;

            SvxLongLRSpaceItem aLongLR((long)pLR->GetLeft(),
                                        (long)pLR->GetRight(),
                                        SID_ATTR_LONG_LRSPACE);
            SfxRequest aReq(SID_ATTR_LONG_LRSPACE, SFX_CALLMODE_SLOT, GetPool());
            aReq.AppendItem(aLongLR);
            ExecTabWin( aReq );
        }
    }
    break;
    case SID_ATTR_LONG_LRSPACE:
    {
        SvxLongLRSpaceItem aLongLR( (const SvxLongLRSpaceItem&)rReq.GetArgs()->
                                                    Get( SID_ATTR_LONG_LRSPACE ) );
        SvxLRSpaceItem aLR;
        DEBUGLRSPACE(aLongLR);
        BOOL bSect = 0 != (nFrmType & FRMTYPE_COLSECT);
        BOOL bTableColumns = 0 != rSh.GetTableFmt();
        if(bSect && (bTableColumns || nFrmType & FRMTYPE_COLUMN))
        {
            SwRect aLastRect = rSh.GetAnyCurRect(RECT_SECTION);
            long nOldLeft = (long)(aLastRect.Left() - DOCUMENTBORDER);
            long nOldRight = (long)(nPageWidth + DOCUMENTBORDER - aLastRect.Right());
            //welche Seite wurde veraendert? Rundungsfehler vermeiden
            BOOL bLeftChanged = Abs(nOldLeft - aLongLR.GetLeft()) > Abs(nOldRight - aLongLR.GetRight());
            const SwFrmFmt* pFmt = 0;
            if(bTableColumns)
            {
                SwTabCols aTabCols;
                rSh.GetTabCols(aTabCols);
                //fuer die erste bzw. letzte Spalte wird der Tabellenrand veraendert, sonst
                //die Spaltentrenner
                USHORT nCurColNo = rSh.GetCurTabColNum();
                USHORT nColCount = aTabCols.Count() + 1;

                if(0 == nCurColNo && bLeftChanged)
                {
                    //die erste
                    long nNewColPos = aLongLR.GetLeft();
                    if(nNewColPos >= aTabCols.GetLeftMin())
                        aTabCols.SetLeft((USHORT)nNewColPos - aTabCols.GetLeftMin()+ DOCUMENTBORDER);

                }
                else if(nColCount - 1 == nCurColNo && !bLeftChanged)
                {
                    //die letzte
                    long nNewColPos = aLongLR.GetRight();
                    nNewColPos = nPageWidth - nNewColPos + DOCUMENTBORDER - aTabCols.GetLeftMin();
                    if(nNewColPos <= aTabCols.GetRightMax())
                        aTabCols.SetRight((USHORT)nNewColPos);
                }
                else
                {
                    //mittendrin
                    long nLeftBorder;
                    long nRightBorder;
                    if(bLeftChanged)
                    {
                        nLeftBorder = nCurColNo - 1 ? aTabCols[nCurColNo - 2] : aTabCols.GetLeft();
                        nRightBorder =  nCurColNo < nColCount - 1 ? aTabCols[nCurColNo] : aTabCols.GetRight();
                    }
                    else
                    {
                        nLeftBorder = nCurColNo ? aTabCols[nCurColNo - 1] : aTabCols.GetLeft();
                        nRightBorder =  nCurColNo + 1 < nColCount ? aTabCols[nCurColNo + 1] : aTabCols.GetRight();
                    }

                    long nNewColPos = bLeftChanged ? aLongLR.GetLeft() : aLongLR.GetRight();
                    //hier muss noch die Border-Distance der Zelle eingerechnet werden
                    if(bLeftChanged)
                        nNewColPos = nNewColPos + DOCUMENTBORDER - aTabCols.GetLeftMin();
                    else
                        nNewColPos = nPageWidth - nNewColPos + DOCUMENTBORDER - aTabCols.GetLeftMin();


                    if(nNewColPos < nLeftBorder + MINLAY)
                        nNewColPos  = nLeftBorder + MINLAY;
                    else if(nNewColPos > nRightBorder - MINLAY)
                        nNewColPos = nRightBorder - MINLAY;
                    aTabCols[nCurColNo - (bLeftChanged ? 1 : 0)] = (USHORT)nNewColPos;
                }
                rSh.SetTabCols(aTabCols, FALSE);
            }
            else if(0 != (pFmt = rSh.GetFlyFrmFmt()))
            {
                SwFmtCol aCol = pFmt->GetCol();
                SwColumns& aCols = aCol.GetColumns();
//              const SwRect &rSizeRect = rSh.GetAnyCurRect(RECT_FLY_PRT_EMBEDDED);
                const SwRect aFlyRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED);
                const long lWidth = aFlyRect.Width();
                USHORT nCurFrameCol = rSh.GetCurOutColNum() - 1;
                USHORT nColCount = aCols.Count();
                SfxItemSet aSet( GetPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_HORI_ORIENT, RES_HORI_ORIENT,
                                            RES_COL, RES_COL, 0 );

                if((0 == nCurFrameCol && bLeftChanged) ||
                        (nColCount - 1 == nCurFrameCol && !bLeftChanged))
                {
                    //in LRSpace umsetzen
                    long nDeltaX = bLeftChanged ?
                                    DOCUMENTBORDER + aLongLR.GetLeft() - aFlyRect.Left()
                                    : 0 ;
                    SwFmtHoriOrient aHoriOrient( pFmt->GetHoriOrient() );
                    aHoriOrient.SetHoriOrient( HORI_NONE );
                    aHoriOrient.SetPos( aHoriOrient.GetPos() + nDeltaX );

                    SwFmtFrmSize aSize( pFmt->GetFrmSize() );
                    long nOldWidth = (long) aSize.GetWidth();

                    long nWidthDiff = nOldLeft - aLongLR.GetLeft() +
                                            nOldRight - aLongLR.GetRight();
                    long nNewWidth = nOldWidth + nWidthDiff;
                    if(aSize.GetWidthPercent())
                    {
                        SwRect aRect;
                        rSh.CalcBoundRect(aRect, FLY_IN_CNTNT);
                        long nPrtWidth = aRect.Width();
                        aSize.SetWidthPercent(BYTE((
                            nNewWidth) * 100 /nPrtWidth));
                    }
                    else
                        aSize.SetWidth( nNewWidth );

                    ::ResizeFrameCols(aCol, nOldWidth, (long)aSize.GetWidth(), nDeltaX );
                    aSet.Put( aSize );
                    aSet.Put( aHoriOrient );

                }
                else //Spalten veraendert
                {
                    long nLeftBorder = 0;
                    long nRightBorder = lWidth;
                    long nOldColPos = 0;
                    long nNewColPos = bLeftChanged ? aLongLR.GetLeft() : aLongLR.GetRight();
                    if(!bLeftChanged)
                        nNewColPos = nPageWidth - nNewColPos;

                    //mittendrin
                    long nWidth = 0;
                    for ( USHORT i = 0; i < aCols.Count(); ++i )
                    {
                        SwColumn* pCol = aCols[i];
                        nWidth += aCol.CalcColWidth(i, lWidth);
                        if( i == (bLeftChanged ? nCurFrameCol - 1 : nCurFrameCol))
                            nOldColPos = nWidth;
                        if( i == (bLeftChanged ? nCurFrameCol - 2 : nCurFrameCol - 1))
                            nLeftBorder = nWidth;
                        if(i == (bLeftChanged ? nCurFrameCol : nCurFrameCol + 1))
                            nRightBorder = nWidth;
                    }
                    nNewColPos -= aFlyRect.Left();
                    nNewColPos += DOCUMENTBORDER;
                    if(nNewColPos < nLeftBorder + MINLAY)
                        nNewColPos  = nLeftBorder + MINLAY;
                    else if(nNewColPos > nRightBorder - MINLAY)
                        nNewColPos = nRightBorder - MINLAY;


                    // in WishWidth umrechnen:
                    nNewColPos = nNewColPos * long(aCol.GetWishWidth()) / lWidth;
                    nOldColPos = nOldColPos * long(aCol.GetWishWidth()) / lWidth;
                    long nDiff = nNewColPos - nOldColPos;

                    SwColumn* pCol = aCols[bLeftChanged ? nCurFrameCol : nCurFrameCol + 1];
                    pCol->SetWishWidth(long(pCol->GetWishWidth()) - nDiff );
                    //den Nachbarn in umgekehrter Weise veraendern
                    pCol = aCols[bLeftChanged ? nCurFrameCol - 1 : nCurFrameCol ];
                    pCol->SetWishWidth(long(pCol->GetWishWidth()) + nDiff );
                }
                aSet.Put( aCol );
                rSh.StartAction();
                rSh.Push();
                rSh.SetFlyFrmAttr( aSet );
                //die Rahmenselektion wieder aufheben
                if(!bFrmSelection && rSh.IsFrmSelected())
                {
                    rSh.UnSelectFrm();
                    rSh.LeaveSelFrmMode();
                }
                rSh.Pop();
                rSh.EndAction();

            }
            else
            {
                //Seitenspalten
                USHORT nCurCol = rSh.GetCurOutColNum();
                nCurCol--;
                const SwFrmFmt& rMaster = rDesc.GetMaster();
                SwFmtCol aCol(rMaster.GetCol());
                SwColumns& aCols = aCol.GetColumns();
                //SvxColumnItem aColItem(nCurCol);
                const SwRect aPageRect = rSh.GetAnyCurRect(RECT_PAGE_PRT);
                const long lWidth = aPageRect.Width();
                USHORT nColCount = aCols.Count();
                SfxItemSet aSet( GetPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_HORI_ORIENT, RES_HORI_ORIENT,
                                            RES_COL, RES_COL, 0 );

                if((0 == nCurCol && bLeftChanged) ||
                        (nColCount - 1 == nCurCol && !bLeftChanged))
                {
                    // Seitenraender einstellen
                    SvxLRSpaceItem aLR;
                    aLR.SetLeft ((aPageRect.Left()/* - DOCUMENTBORDER*/));
                    aLR.SetRight((nPageWidth + /*DOCUMENTBORDER*/ - aPageRect.Right()));
                    if(bLeftChanged)
                        aLR.SetLeft((USHORT)aLongLR.GetLeft());
                    else
                        aLR.SetRight((USHORT)aLongLR.GetRight());
                    SwapPageMargin( rDesc, aLR );
                    SwPageDesc aDesc( rDesc );
                    aDesc.GetMaster().SetAttr( aLR );
                    rSh.ChgPageDesc( nDescId, aDesc );
                }
                else //Spalten veraendert ?
                {
                    long nLeftBorder = 0;
                    long nRightBorder = nPageWidth;
                    long nOldColPos = 0;
                    long nNewColPos = bLeftChanged ? aLongLR.GetLeft() : aLongLR.GetRight();
                    if(!bLeftChanged)
                        nNewColPos = nPageWidth - nNewColPos;

                    //mittendrin
                    long nWidth = 0;
                    for ( USHORT i = 0; i < aCols.Count(); ++i )
                    {
                        SwColumn* pCol = aCols[i];
                        nWidth += aCol.CalcColWidth(i, lWidth);
                        if( i == (bLeftChanged ? nCurCol - 1 : nCurCol))
                            nOldColPos = nWidth;
                        if( i == (bLeftChanged ? nCurCol - 2 : nCurCol - 1))
                            nLeftBorder = nWidth;
                        if(i == (bLeftChanged ? nCurCol : nCurCol + 1))
                            nRightBorder = nWidth;
                    }
                    nNewColPos -= aPageRect.Left();
//                  nNewColPos += DOCUMENTBORDER;
                    if(nNewColPos < nLeftBorder + MINLAY)
                        nNewColPos  = nLeftBorder + MINLAY;
                    else if(nNewColPos > nRightBorder - MINLAY)
                        nNewColPos = nRightBorder - MINLAY;


                    // in WishWidth umrechnen:
                    nNewColPos = nNewColPos * long(aCol.GetWishWidth()) / lWidth;
                    nOldColPos = nOldColPos * long(aCol.GetWishWidth()) / lWidth;
                    long nDiff = nNewColPos - nOldColPos;

                    SwColumn* pCol = aCols[bLeftChanged ? nCurCol : nCurCol + 1];
                    pCol->SetWishWidth(long(pCol->GetWishWidth()) - nDiff );
                    //den Nachbarn in umgekehrter Weise veraendern
                    pCol = aCols[bLeftChanged ? nCurCol - 1 : nCurCol ];
                    pCol->SetWishWidth(long(pCol->GetWishWidth()) + nDiff );
                }
                SwPageDesc aDesc( rDesc );
                aDesc.GetMaster().SetAttr( aCol );
                rSh.ChgPageDesc( rSh.GetCurPageDesc(), aDesc );


/*              aSet.Put( aCol );
//              rSh.StartAction();
//              rSh.Push();
//              rSh.SetFlyFrmAttr( aSet );*/
            }
        }
        else if ( bFrmSelection || nFrmType & FRMTYPE_FLY_ANY )
        {
            SwFrmFmt* pFmt = ((SwFrmFmt*)rSh.GetFlyFrmFmt());
            const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED);
            long nDeltaX = DOCUMENTBORDER + aLongLR.GetLeft() -
                        rRect.Left();
            SwFmtHoriOrient aHoriOrient( pFmt->GetHoriOrient() );
            aHoriOrient.SetHoriOrient( HORI_NONE );
            aHoriOrient.SetPos( aHoriOrient.GetPos() + nDeltaX );

            SwFmtFrmSize aSize( pFmt->GetFrmSize() );
            long nOldWidth = (long) aSize.GetWidth();

            if(aSize.GetWidthPercent())
            {
                SwRect aRect;
                rSh.CalcBoundRect(aRect, FLY_IN_CNTNT);
                long nPrtWidth = aRect.Width();
                aSize.SetWidthPercent(BYTE((nPageWidth - aLongLR.GetLeft() - aLongLR.GetRight()) * 100 /nPrtWidth));
            }
            else
                aSize.SetWidth( nPageWidth -
                        (aLongLR.GetLeft() + aLongLR.GetRight()));

            SfxItemSet aSet( GetPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                        RES_HORI_ORIENT, RES_HORI_ORIENT,
                                        RES_COL, RES_COL, 0 );

            if( nFrmType & FRMTYPE_COLUMN )
            {
                SwFmtCol aCol(pFmt->GetCol());

                ::ResizeFrameCols(aCol, nOldWidth, (long)aSize.GetWidth(), nDeltaX );
                aSet.Put(aCol);
            }

            aSet.Put( aSize );
            aSet.Put( aHoriOrient );

            rSh.StartAction();
            rSh.Push();
            rSh.SetFlyFrmAttr( aSet );
            //die Rahmenselektion wieder aufheben
            if(!bFrmSelection && rSh.IsFrmSelected())
            {
                rSh.UnSelectFrm();
                rSh.LeaveSelFrmMode();
            }
            rSh.Pop();
            rSh.EndAction();
        }
        else if ( nFrmType & ( FRMTYPE_HEADER | FRMTYPE_FOOTER ))
        {
            // Seitenraender rausrechnen
            USHORT nOld = rDesc.GetMaster().GetLRSpace().GetLeft();
            aLongLR.SetLeft( nOld > (USHORT)aLongLR.GetLeft() ? 0 : aLongLR.GetLeft() - nOld );

            nOld = rDesc.GetMaster().GetLRSpace().GetRight();
            aLongLR.SetRight( nOld > (USHORT)aLongLR.GetRight() ? 0 : aLongLR.GetRight() - nOld );
            aLR.SetLeft((USHORT)aLongLR.GetLeft());
            aLR.SetRight((USHORT)aLongLR.GetRight());

            if ( nFrmType & FRMTYPE_HEADER )
                pHeaderFmt->SetAttr( aLR );
            else
                pFooterFmt->SetAttr( aLR );
        }
        else if( nFrmType == FRMTYPE_DRAWOBJ)
        {
            SwRect aRect( rSh.GetObjRect() );
            aRect.Left( aLongLR.GetLeft() + DOCUMENTBORDER );
            aRect.Right( nPageWidth + DOCUMENTBORDER - aLongLR.GetRight());
            rSh.SetObjRect( aRect );
        }
        else
        {   // Seitenraender einstellen
            aLR.SetLeft((USHORT)aLongLR.GetLeft());
            aLR.SetRight((USHORT)aLongLR.GetRight());
            SwapPageMargin( rDesc, aLR );
            SwPageDesc aDesc( rDesc );
            aDesc.GetMaster().SetAttr( aLR );
            rSh.ChgPageDesc( nDescId, aDesc );
        }
    }
    break;
    case RES_UL_SPACE:
    case SID_ATTR_ULSPACE:
    {
        if(SFX_ITEM_SET == pArgs->GetItemState(RES_UL_SPACE, FALSE, &pItem))
        {
            const  SvxULSpaceItem* pUL = (const SvxULSpaceItem*)pItem;

            SvxLongULSpaceItem aLongUL((long)pUL->GetUpper(), (long)pUL->GetLower(),
                                        SID_ATTR_LONG_ULSPACE);
            SfxRequest aReq(SID_ATTR_LONG_ULSPACE, SFX_CALLMODE_SLOT, GetPool());
            aReq.AppendItem(aLongUL);
            ExecTabWin( aReq );
        }
    }
    break;
    case SID_ATTR_LONG_ULSPACE:
    {
        SvxLongULSpaceItem aLongULSpace( (const SvxLongULSpaceItem&)rReq.GetArgs()->
                                                        Get( SID_ATTR_LONG_ULSPACE ));
        DEBUGULSPACE(aLongULSpace);

        if( bFrmSelection || nFrmType & FRMTYPE_FLY_ANY )
        {
            SwFrmFmt* pFmt = ((SwFrmFmt*)rSh.GetFlyFrmFmt());

            const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED);
            const long nDeltaY = DOCUMENTBORDER + aLongULSpace.GetUpper() -
                                    rRect.Top();
            const long nHeight = nPageHeight -
                            (aLongULSpace.GetUpper() + aLongULSpace.GetLower());

            SwFmtVertOrient aVertOrient(pFmt->GetVertOrient());
            aVertOrient.SetVertOrient(VERT_NONE);
            aVertOrient.SetPos(aVertOrient.GetPos() + nDeltaY );

            SwFmtFrmSize aSize(pFmt->GetFrmSize());
            if(aSize.GetHeightPercent())
            {
                SwRect aRect;
                rSh.CalcBoundRect(aRect, FLY_IN_CNTNT);
                long nPrtHeight = aRect.Height();
                aSize.SetHeightPercent(BYTE(nHeight * 100 /nPrtHeight));
            }
            else
                aSize.SetHeight(nHeight );

            SfxItemSet aSet( GetPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                        RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
            aSet.Put( aSize );
            aSet.Put( aVertOrient );
            rSh.SetFlyFrmAttr( aSet );
        }
        else if( nFrmType == FRMTYPE_DRAWOBJ )
        {
            SwRect aRect( rSh.GetObjRect() );
            const SwRect &rPageRect = rSh.GetAnyCurRect(RECT_PAGE);
            aRect.Top( aLongULSpace.GetUpper() + rPageRect.Top() );
            aRect.Bottom( rPageRect.Bottom() - aLongULSpace.GetLower() );
            rSh.SetObjRect( aRect ) ;
        }
        else
        {   SwPageDesc aDesc( rDesc );

            if ( nFrmType & ( FRMTYPE_HEADER | FRMTYPE_FOOTER ))
            {

                const BOOL bHead = nFrmType & FRMTYPE_HEADER ? TRUE : FALSE;
                SvxULSpaceItem aUL( rDesc.GetMaster().GetULSpace() );
                if ( bHead )
                    aUL.SetUpper( (USHORT)aLongULSpace.GetUpper() );
                else
                    aUL.SetLower( (USHORT)aLongULSpace.GetLower() );
                aDesc.GetMaster().SetAttr( aUL );

                SwFmtFrmSize aSz( bHead ? pHeaderFmt->GetFrmSize() :
                                          pFooterFmt->GetFrmSize() );
                aSz.SetSizeType( ATT_FIX_SIZE );
                aSz.SetHeight(nPageHeight - aLongULSpace.GetLower() -
                                            aLongULSpace.GetUpper() );
                if ( bHead )
                    pHeaderFmt->SetAttr( aSz );
                else
                    pFooterFmt->SetAttr( aSz );
            }
            else
            {
                SvxULSpaceItem aUL;
                aUL.SetUpper((USHORT)aLongULSpace.GetUpper());
                aUL.SetLower((USHORT)aLongULSpace.GetLower());
                aDesc.GetMaster().SetAttr(aUL);
            }

            rSh.ChgPageDesc( nDescId, aDesc );
        }
    }
    break;
    case SID_ATTR_TABSTOP:
    {
        SvxTabStopItem aTabStops( (const SvxTabStopItem&)rReq.GetArgs()->
                                                    Get( RES_PARATR_TABSTOP ));
        DEBUGTABSTOPS(aTabStops);
        const SvxTabStopItem& rDefTabs =
                    (const SvxTabStopItem&)rSh.GetDefault(RES_PARATR_TABSTOP);

        // Default-Tab an Pos 0
        SfxItemSet aSet( GetPool(), RES_LR_SPACE, RES_LR_SPACE );
        rSh.GetAttr( aSet );
        const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)aSet.Get(RES_LR_SPACE);

        if ( rLR.GetTxtFirstLineOfst() < 0 )
        {
            SvxTabStop aSwTabStop( 0, SVX_TAB_ADJUST_DEFAULT );
            aTabStops.Insert( aSwTabStop );
        }

        // auffuellen mit Default-Tabs
        USHORT nDef = ::GetTabDist( rDefTabs );
        ::MakeDefTabs( nDef, aTabStops );

        SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
        if( pColl && pColl->IsAutoUpdateFmt() )
        {
            SfxItemSet aSet(GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP);
            aSet.Put(aTabStops);
            rSh.AutoUpdatePara( pColl, aSet);
        }
        else
            rSh.SetAttr( aTabStops );
        break;
    }
    case SID_ATTR_PARA_LRSPACE:
    {
        SvxLRSpaceItem aParaMargin((const SvxLRSpaceItem&)rReq.
                                        GetArgs()->Get(SID_ATTR_PARA_LRSPACE));

        if(nFrmType & FRMTYPE_FLY_ANY)
        {
// os: Wegen #42444# wird hier der Umrandungsabstand eingefuegt
            SfxItemSet aCoreSet( GetPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
            SvxBoxInfoItem aBoxInfo;
            aCoreSet.Put( aBoxInfo );
            rSh.GetFlyFrmAttr( aCoreSet );
            const SvxBoxItem& rBox = (const SvxBoxItem&)aCoreSet.Get(RES_BOX);
            USHORT nDist = (USHORT)rBox.GetDistance();
            aParaMargin.SetRight(Max((long)(aParaMargin.GetRight() - nDist), (long)0));
            if(aParaMargin.GetLeft() < nDist)
                nDist = aParaMargin.GetLeft();
            aParaMargin.SetTxtLeft(aParaMargin.GetTxtLeft() - nDist);
        }
        DEBUGPARAMARGIN(aParaMargin);

        aParaMargin.SetWhich( RES_LR_SPACE );
        SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
        if( pColl && pColl->IsAutoUpdateFmt() )
        {
            SfxItemSet aSet(GetPool(), RES_LR_SPACE, RES_LR_SPACE);
            aSet.Put(aParaMargin);
            rSh.AutoUpdatePara( pColl, aSet);
        }
        else
            rSh.SetAttr( aParaMargin );

        if ( aParaMargin.GetTxtFirstLineOfst() < 0 )
        {
            SfxItemSet aSet( GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP );

            rSh.GetAttr( aSet );
            const SvxTabStopItem&  rTabStops = (const SvxTabStopItem&)aSet.Get(RES_PARATR_TABSTOP);

            // Haben wir einen Tab an Stelle Null
            for ( USHORT i = 0; i < rTabStops.Count(); ++i )
                if ( rTabStops[i].GetTabPos() == 0 )
                    break;

            if ( i >= rTabStops.Count() )
            {
                // Kein DefTab
                SvxTabStopItem aTabStops;
                aTabStops = rTabStops;

                ::lcl_EraseDefTabs(aTabStops);

                SvxTabStop aSwTabStop( 0, SVX_TAB_ADJUST_DEFAULT );
                aTabStops.Insert(aSwTabStop);

                const SvxTabStopItem& rDefTabs =
                    (const SvxTabStopItem&)rSh.GetDefault(RES_PARATR_TABSTOP);
                USHORT nDef = ::GetTabDist(rDefTabs);
                ::MakeDefTabs( nDef, aTabStops );

                if( pColl && pColl->IsAutoUpdateFmt())
                {
                    SfxItemSet aSet(GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP);
                    aSet.Put(aTabStops);
                    rSh.AutoUpdatePara( pColl, aSet);
                }
                else
                    rSh.SetAttr( aTabStops );
            }
        }
    }
    break;
    case SID_RULER_BORDERS:
    {
        SvxColumnItem aColItem((const SvxColumnItem&)rReq.
                                            GetArgs()->Get(SID_RULER_BORDERS));

        BOOL bSect = 0 != (nFrmType & FRMTYPE_COLSECT);
        if( bSetTabColFromDoc || !bSect && rSh.GetTableFmt() )
        {
            ASSERT(aColItem.Count(), "ColDesc ist leer!!");

            const BOOL bSingleLine = ((const SfxBoolItem&)rReq.
                            GetArgs()->Get(SID_RULER_ACT_LINE_ONLY)).GetValue();

            DEBUGCOLITEMS(aColItem);
            SwTabCols aTabCols;
            if ( bSetTabColFromDoc )
                rSh.GetMouseTabCols( aTabCols, aTabColFromDocPos );
            else
                rSh.GetTabCols(aTabCols);
            DEBUGTABCOLS(aTabCols);

            // linker Tabellenrand
            long nBorder = long(aColItem.GetLeft()) -
                             (long(aTabCols.GetLeftMin()) - DOCUMENTBORDER);
            aTabCols.SetLeft( nBorder );

            nBorder = nPageWidth - aTabCols.GetLeftMin() +
                              DOCUMENTBORDER - aColItem.GetRight();

            aTabCols.SetRight( nBorder );

            // Tabcols der Reihe nach
            // Die letzte Col wird durch den Rand definiert
            for ( USHORT i = 0; i < aColItem.Count()-1; ++i )
            {
                const SvxColumnDescription& rCol = aColItem[i];
                aTabCols[i] = rCol.nEnd + aTabCols.GetLeft();
                aTabCols.SetHidden( i, !rCol.bVisible );
            }
            DEBUGTABCOLS(aTabCols);

            if ( bSetTabColFromDoc )
            {
                if( !rSh.IsViewLocked() )
                {
                    bUnlockView = TRUE;
                    rSh.LockView( TRUE );
                }
                rSh.SetMouseTabCols( aTabCols, bSingleLine,
                                               aTabColFromDocPos );
            }
            else
                rSh.SetTabCols(aTabCols, bSingleLine);

        }
        else
        {
            if ( bFrmSelection || nFrmType & FRMTYPE_FLY_ANY || bSect)
            {
                SwSectionFmt *pSectFmt = 0;
                SfxItemSet aSet( GetPool(), RES_COL, RES_COL );
                if(bSect)
                {
                    const SwSection *pSect = rSh.GetAnySection();
                    ASSERT( pSect, "Welcher Bereich?");
                    pSectFmt = pSect->GetFmt();
                }
                else
                {
                    rSh.GetFlyFrmAttr( aSet );
                }
                SwFmtCol aCols(
                    bSect ?
                        pSectFmt->GetCol() :
                            (const SwFmtCol&)aSet.Get( RES_COL, FALSE ));

                const long lWidth = rSh.GetAnyCurRect(bSect ? RECT_SECTION : RECT_FLY_PRT_EMBEDDED).Width();
                ::lcl_ConvertToCols( aColItem, USHORT(lWidth), aCols );
                DEBUGCOLITEMS(aColItem);
                aSet.Put( aCols );
                if(bSect)
                    rSh.SetSectionAttr( aSet, pSectFmt );
                else
                {
                    rSh.StartAction();
                    rSh.Push();
                    rSh.SetFlyFrmAttr( aSet );
                    //die Rahmenselektion wieder aufheben
                    if(!bFrmSelection && rSh.IsFrmSelected())
                    {
                        rSh.UnSelectFrm();
                        rSh.LeaveSelFrmMode();
                    }
                    rSh.Pop();
                    rSh.EndAction();
                }
            }
            else
            {
                SwFmtCol aCols( rDesc.GetMaster().GetCol() );
                ::lcl_ConvertToCols( aColItem,
                                USHORT(rSh.GetAnyCurRect(RECT_PAGE_PRT).Width()),
                                aCols );
                DEBUGCOLITEMS(aColItem);
                SwPageDesc aDesc( rDesc );
                aDesc.GetMaster().SetAttr( aCols );
                rSh.ChgPageDesc( rSh.GetCurPageDesc(), aDesc );
            }
        }
        break;
    }
    default:
        ASSERT( !this, "Falsche SlotId");
    }
    rSh.EndAllAction();

    if( bUnlockView )
        rSh.LockView( FALSE );

    bSetTabColFromDoc = bTabColFromDoc = FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Hier wird der Status der Tableiste ermittelt
                    sprich alle relevanten Attribute an der CursorPos
                    werden der Tableiste uebermittelt
 --------------------------------------------------------------------*/


void SwView::StateTabWin(SfxItemSet& rSet)
{
    SwWrtShell &rSh         = GetWrtShell();

    const Point* pPt = IsTabColFromDoc() ? &aTabColFromDocPos : 0;
    const USHORT nFrmType   = rSh.IsObjSelected()
                ? FRMTYPE_DRAWOBJ
                : rSh.GetFrmType( pPt, TRUE );

    const BOOL  bFrmSelection = rSh.IsFrmSelected();

    BOOL bBrowse = rSh.IsBrowseMode();
    WinBits nStyle = pHLineal->GetStyle();
    if(!(nStyle&WB_EXTRAFIELD) != bBrowse)
    {
        if(bBrowse)
            nStyle &= ~WB_EXTRAFIELD;
        else
            nStyle |= WB_EXTRAFIELD;
        pHLineal->SetStyle(nStyle);
    }

    // PageOffset/Begrenzer
    const SwRect& rPageRect = rSh.GetAnyCurRect( RECT_PAGE, pPt );
    const SwRect& rPagePrtRect = rSh.GetAnyCurRect( RECT_PAGE_PRT, pPt );
    const long nPageWidth  = rPageRect.Width();
    const long nPageHeight = rPageRect.Height();

    const SwPageDesc& rDesc = rSh.GetPageDesc( rSh.GetCurPageDesc() );
    SvxLRSpaceItem aPageLRSpace( rDesc.GetMaster().GetLRSpace() );
    SwapPageMargin( rDesc, aPageLRSpace );

    SfxItemSet aCoreSet( GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP,
                                    RES_LR_SPACE,        RES_UL_SPACE, 0 );
    rSh.GetAttr( aCoreSet );
    USHORT nSelectionType = rSh.GetSelectionType();

    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
        case RES_LR_SPACE:
        case SID_ATTR_LRSPACE:
        case SID_ATTR_LONG_LRSPACE:
        {
            SvxLongLRSpaceItem aLongLR( (long)aPageLRSpace.GetLeft(),
                                        (long)aPageLRSpace.GetRight(),
                                        SID_ATTR_LONG_LRSPACE);
            if(bBrowse)
            {
                aLongLR.SetLeft(rPagePrtRect.Left());
                aLongLR.SetRight(nPageWidth - rPagePrtRect.Right());
            }
            if ( ( nFrmType & FRMTYPE_HEADER || nFrmType & FRMTYPE_FOOTER ) &&
                 !(nFrmType & FRMTYPE_COLSECT) )
            {
                SwFrmFmt *pFmt = (SwFrmFmt*) (nFrmType & FRMTYPE_HEADER ?
                                rDesc.GetMaster().GetHeader().GetHeaderFmt() :
                                rDesc.GetMaster().GetFooter().GetFooterFmt());
                SwRect aRect( rSh.GetAnyCurRect( RECT_HEADERFOOTER, pPt));
                aRect.Pos() -= rSh.GetAnyCurRect( RECT_PAGE, pPt ).Pos();
                const SvxLRSpaceItem& aLR = pFmt->GetLRSpace();
                aLongLR.SetLeft ( (long)aLR.GetLeft() + (long)aRect.Left() );
                aLongLR.SetRight( (nPageWidth -
                                    (long)aRect.Right() + (long)aLR.GetRight()));
            }
            else
            {
                SwRect aRect;
                if( !bFrmSelection && (nFrmType & FRMTYPE_COLSECT) )
                    aRect = rSh.GetAnyCurRect(RECT_SECTION, pPt);
                else if ( bFrmSelection || nFrmType & FRMTYPE_FLY_ANY )
                    aRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED, pPt);
                else if( nFrmType & FRMTYPE_DRAWOBJ)
                    aRect = rSh.GetObjRect();
                if( aRect.Width() )
                {
                    aLongLR.SetLeft ((long)(aRect.Left() - DOCUMENTBORDER));
                    aLongLR.SetRight((long)(nPageWidth + DOCUMENTBORDER - aRect.Right()));
                }
            }
            if( nWhich == SID_ATTR_LONG_LRSPACE )
                rSet.Put( aLongLR );
            else
            {
                SvxLRSpaceItem aLR((USHORT)aLongLR.GetLeft(),
                                    (USHORT)aLongLR.GetRight(),
                                    nWhich);
                rSet.Put(aLR);
            }
            DEBUGLRSPACE(aLongLR);
            break;
        }
        case SID_ATTR_LONG_ULSPACE:
        case SID_ATTR_ULSPACE:
        case RES_UL_SPACE:
        {
            // Rand Seite Oben Unten
            SvxULSpaceItem aUL( rDesc.GetMaster().GetULSpace() );
            SvxLongULSpaceItem aLongUL( (long)aUL.GetUpper(),
                                        (long)aUL.GetLower(),
                                        SID_ATTR_LONG_ULSPACE);

            if ( bFrmSelection || nFrmType & FRMTYPE_FLY_ANY )
            {
                // Dokumentkoordinaten Frame auf Seitenkoordinaten umbrechen
                const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED, pPt);
                aLongUL.SetUpper((USHORT)(rRect.Top() - rPageRect.Top() ));
                aLongUL.SetLower((USHORT)(rPageRect.Bottom() - rRect.Bottom() ));
            }
            else if ( nFrmType & FRMTYPE_HEADER || nFrmType & FRMTYPE_FOOTER )
            {
                SwRect aRect( rSh.GetAnyCurRect( RECT_HEADERFOOTER, pPt));
                aRect.Pos() -= rSh.GetAnyCurRect( RECT_PAGE, pPt ).Pos();
                aLongUL.SetUpper( (USHORT)aRect.Top() );
                aLongUL.SetLower( (USHORT)(nPageHeight - aRect.Bottom()) );
            }
            else if( nFrmType & FRMTYPE_DRAWOBJ)
            {
                const SwRect &rRect = rSh.GetObjRect();
                aLongUL.SetUpper((rRect.Top() - rPageRect.Top()));
                aLongUL.SetLower((rPageRect.Bottom() - rRect.Bottom()));
            }
            else if(bBrowse)
            {
                aLongUL.SetUpper(rPagePrtRect.Top());
                aLongUL.SetLower(nPageHeight - rPagePrtRect.Bottom());
            }
            if( nWhich == SID_ATTR_LONG_ULSPACE )
                rSet.Put( aLongUL );
            else
            {
                SvxULSpaceItem aUL((USHORT)aLongUL.GetUpper(),
                                    (USHORT)aLongUL.GetLower(),
                                    nWhich);
                rSet.Put(aUL);
            }
            DEBUGULSPACE(aLongUL);
            break;
        }
        case RES_PARATR_TABSTOP:
        {
            if ( ISA( SwWebView ) ||
                 IsTabColFromDoc() ||
                 ( nSelectionType & SwWrtShell::SEL_GRF) ||
                    (nSelectionType & SwWrtShell::SEL_FRM) ||
                    (nSelectionType & SwWrtShell::SEL_OLE) ||
                    SFX_ITEM_AVAILABLE > aCoreSet.GetItemState(RES_LR_SPACE)
                 )
                rSet.DisableItem( RES_PARATR_TABSTOP );
            else
            {   SvxTabStopItem aTabStops((const SvxTabStopItem&)
                                            aCoreSet.Get( RES_PARATR_TABSTOP ));

                const SvxTabStopItem& rDefTabs = (const SvxTabStopItem&)
                                            rSh.GetDefault(RES_PARATR_TABSTOP);

                DBG_ASSERT(pHLineal, "warum ist das Lineal nicht da?")
                pHLineal->SetDefTabDist( ::GetTabDist(rDefTabs) );
                ::lcl_EraseDefTabs(aTabStops);
                DEBUGTABSTOPS(aTabStops);
                rSet.Put(aTabStops);
            }
            break;
        }
        case SID_ATTR_PARA_LRSPACE:
        {
            if ( nSelectionType & SwWrtShell::SEL_GRF ||
                    nSelectionType & SwWrtShell::SEL_FRM ||
                    nSelectionType & SwWrtShell::SEL_OLE ||
                    nFrmType == FRMTYPE_DRAWOBJ )
                rSet.DisableItem(SID_ATTR_PARA_LRSPACE);
            else
            {
                SvxLRSpaceItem aLR;
                if ( !IsTabColFromDoc() )
                    aLR = (const SvxLRSpaceItem&)aCoreSet.Get(RES_LR_SPACE);
                DEBUGPARAMARGIN(aLR);
                aLR.SetWhich(SID_ATTR_PARA_LRSPACE);
                if(nFrmType & FRMTYPE_FLY_ANY)
                {
// os: Wegen #42444# wird hier der Umrandungsabstand eingefuegt
                    USHORT nDist;
                    if( IsTabColFromDoc() )
                    {
                        const SwRect& rPrtRect = rSh.GetAnyCurRect(
                                        RECT_FLY_PRT_EMBEDDED, pPt );
                        nDist = rPrtRect.Left();
                    }
                    else
                    {
                        SfxItemSet aCoreSet( GetPool(),
                                                RES_BOX, RES_BOX,
                                                SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
                        SvxBoxInfoItem aBoxInfo;
                        aCoreSet.Put( aBoxInfo );
                        rSh.GetFlyFrmAttr( aCoreSet );
                        const SvxBoxItem& rBox = (const SvxBoxItem&)aCoreSet.Get(RES_BOX);
                        nDist = (USHORT)rBox.GetDistance();
                    }
                    aLR.SetRight(aLR.GetRight() + nDist);
                    aLR.SetTxtLeft(aLR.GetTxtLeft() + nDist);
                }
                rSet.Put(aLR);
            }
            break;
        }
        case SID_RULER_BORDERS:
        {
            if ( IsTabColFromDoc() ||
                    ( rSh.GetTableFmt() && !bFrmSelection &&
                    !(nFrmType & FRMTYPE_COLSECT ) ) )
            {
                SwTabCols aTabCols;
                USHORT    nNum;
                if ( 0 != ( bSetTabColFromDoc = IsTabColFromDoc() ) )
                {
                    rSh.GetMouseTabCols( aTabCols, aTabColFromDocPos );
                    nNum = rSh.GetCurMouseTabColNum( aTabColFromDocPos );
                }
                else
                {
                    rSh.GetTabCols( aTabCols );
                    nNum = rSh.GetCurTabColNum();
                }

                ASSERT(nNum <= aTabCols.Count(), "TabCol not found");
                int nLft = aTabCols.GetLeftMin() -
                                  USHORT(DOCUMENTBORDER) +
                                  aTabCols.GetLeft();

                int nRgt = (USHORT)nPageWidth -
                                  (aTabCols.GetLeftMin() +
                                  aTabCols.GetRight() -
                                  USHORT(DOCUMENTBORDER) );

                const USHORT nL = nLft > 0 ? nLft : 0;
                const USHORT nR = nRgt > 0 ? nRgt : 0;

                SvxColumnItem aColItem(nNum, nL, nR);

                USHORT nStart = 0,
                       nEnd;

                for ( USHORT i = 0; i < aTabCols.Count(); ++i )
                {
                    nEnd  = aTabCols[i] - aTabCols.GetLeft();
                    SvxColumnDescription aColDesc( nStart, nEnd,
                                                   !aTabCols.IsHidden(i) );
                    aColItem.Append(aColDesc);
                    nStart = nEnd;
                }
                SvxColumnDescription aColDesc(nStart,
                                aTabCols.GetRight() - aTabCols.GetLeft(), TRUE);
                aColItem.Append(aColDesc);
                rSet.Put(aColItem);
                DEBUGCOLITEMS(aColItem);
            }
            else if ( bFrmSelection || nFrmType & ( FRMTYPE_COLUMN | FRMTYPE_COLSECT ) )
            {
                // Aus Rahmen oder Seite ?
                USHORT nNum = 0;
                if(bFrmSelection)
                {
                    const SwFrmFmt* pFmt = rSh.GetFlyFrmFmt();
                    if(pFmt)
                        nNum = pFmt->GetCol().GetNumCols();
                }
                else
                    nNum = rSh.GetCurColNum();

                if(
                    //eigentlich sollte FRMTYPE_COLSECT nicht enthalten sein, wenn der Rahmen selektiert ist!
                    !bFrmSelection &&
                    nFrmType & FRMTYPE_COLSECT )
                {
                    const SwSection *pSect = rSh.GetAnySection();
                    ASSERT( pSect, "Welcher Bereich?");
                    if( pSect )
                    {
                        SwSectionFmt *pFmt = pSect->GetFmt();
                        const SwFmtCol& rCol = pFmt->GetCol();
                        SvxColumnItem aColItem(--nNum);
                        const SwRect &rRect = rSh.GetAnyCurRect(RECT_SECTION, pPt);

                        ::lcl_FillSvxColumn(rCol, USHORT(rRect.Width()), aColItem, 0);

                        aColItem.SetLeft ((USHORT)(rRect.Left() - DOCUMENTBORDER ));
                        aColItem.SetRight((USHORT)(nPageWidth   - rRect.Right() -
                                                                    DOCUMENTBORDER ));

                        aColItem.SetOrtho(aColItem.CalcOrtho());

                        rSet.Put(aColItem);
                        DEBUGCOLITEMS(aColItem);
                    }
                }
                else if( bFrmSelection || nFrmType & FRMTYPE_FLY_ANY )
                {
                    // Spalten in Rahmen
                    if ( nNum  )
                    {
                        nNum--;
                        const SwFrmFmt* pFmt = rSh.GetFlyFrmFmt() ;

                        const SwFmtCol& rCol = pFmt->GetCol();
                        SvxColumnItem aColItem(nNum);
                        const SwRect &rSizeRect = rSh.GetAnyCurRect(RECT_FLY_PRT_EMBEDDED, pPt);

                        const long lWidth = rSizeRect.Width();
                        const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED, pPt);
                        long nDist2 = (rRect.Width() - lWidth) /2;
                        ::lcl_FillSvxColumn(rCol, USHORT(lWidth), aColItem, nDist2);

                        SfxItemSet aFrameSet(GetPool(), RES_LR_SPACE, RES_LR_SPACE);
                        rSh.GetFlyFrmAttr( aFrameSet );

                        aColItem.SetLeft ((USHORT)(rRect.Left() - DOCUMENTBORDER ));
                        aColItem.SetRight((USHORT)(nPageWidth   - rRect.Right() -
                                                                    DOCUMENTBORDER ));

                        aColItem.SetOrtho(aColItem.CalcOrtho());

                        rSet.Put(aColItem);
                        DEBUGCOLITEMS(aColItem);
                    }
                    else
                        rSet.DisableItem(SID_RULER_BORDERS);
                }
                else
                {   // Spalten auf der Seite
                    nNum--;
                    const SwFrmFmt& rMaster = rDesc.GetMaster();
                    SwFmtCol aCol(rMaster.GetCol());
                    SvxColumnItem aColItem(nNum);
                    const long lWidth = rSh.GetAnyCurRect(RECT_PAGE_PRT, pPt).Width();
                    const SvxBoxItem& rBox = (const SvxBoxItem&)rMaster.GetAttr(RES_BOX);
                    long nDist = rBox.GetDistance();
                    ::lcl_FillSvxColumn(aCol, USHORT(lWidth), aColItem, nDist);

                    if(bBrowse)
                    {
                        aColItem.SetLeft((USHORT)rPagePrtRect.Left());
                        aColItem.SetRight(USHORT(nPageWidth - rPagePrtRect.Right()));
                    }
                    else
                    {
                        aColItem.SetLeft (aPageLRSpace.GetLeft());
                        aColItem.SetRight(aPageLRSpace.GetRight());
                    }
                    aColItem.SetOrtho(aColItem.CalcOrtho());

                    rSet.Put(aColItem);
                    DEBUGCOLITEMS(aColItem);
                }
            }
            else
                rSet.DisableItem(SID_RULER_BORDERS);
            break;
        }
        case SID_RULER_PAGE_POS:
        {
            SvxPagePosSizeItem aPagePosSize(
                    Point( DOCUMENTBORDER , rPageRect.Top()) ,
                    nPageWidth, nPageHeight);
            rSet.Put(aPagePosSize);
            break;
        }
        case SID_RULER_LR_MIN_MAX:
        {
            Rectangle aRectangle;
            if( ( nFrmType & FRMTYPE_COLSECT ) && !IsTabColFromDoc() &&
                ( nFrmType & ( FRMTYPE_TABLE|FRMTYPE_COLUMN ) ) )
            {
                if( nFrmType & FRMTYPE_TABLE )
                {
                    const USHORT nNum = rSh.GetCurTabColNum();
                    SwTabCols aTabCols;
                    rSh.GetTabCols( aTabCols );

                    int nLft = aTabCols.GetLeftMin() -
                                    USHORT(DOCUMENTBORDER) +
                                    aTabCols.GetLeft();

                    int nRgt = (USHORT)nPageWidth -
                                    (aTabCols.GetLeftMin() +
                                    aTabCols.GetRight() -
                                    USHORT(DOCUMENTBORDER) );

                    const USHORT nL = nLft > 0 ? nLft : 0;
                    const USHORT nR = nRgt > 0 ? nRgt : 0;

                    aRectangle.Left() = nL;
                    if(nNum > 1)
                        aRectangle.Left() += aTabCols[nNum - 2];
                    if(nNum)
                        aRectangle.Left() += MINLAY;
                    if(aTabCols.Count() <= nNum + 1 )
                        aRectangle.Right() = nR;
                    else
                        aRectangle.Right() = nPageWidth - (nL + aTabCols[nNum + 1]);

                    if(nNum < aTabCols.Count())
                        aRectangle.Right() += MINLAY;
                }
                else
                {
                    const SwFrmFmt* pFmt =  rSh.GetFlyFrmFmt();
                    const SwFmtCol* pCols = pFmt ? &pFmt->GetCol():
                                                   &rDesc.GetMaster().GetCol();
                    const SwColumns& rCols = pCols->GetColumns();
                    USHORT nNum = rSh.GetCurOutColNum();
                    USHORT nCount = Min(USHORT(nNum + 1), rCols.Count());
                    const SwRect aRect( rSh.GetAnyCurRect( pFmt
                                                    ? RECT_FLY_PRT_EMBEDDED
                                                    : RECT_PAGE_PRT, pPt ));
                    const SwRect aAbsRect( rSh.GetAnyCurRect( pFmt
                                                    ? RECT_FLY_EMBEDDED
                                                    : RECT_PAGE, pPt ));

                    //die Breite im Rahmen bzw. innerhalbe der Seitenraender
                    const USHORT nTotalWidth = (USHORT)aRect.Width();
                    //die gesamte Rahmenbreite - die Differenz ist der doppelte Abstand zum Rand
                    const USHORT nOuterWidth = (USHORT)aAbsRect.Width();
                    int nWidth = 0,
                        nStart = 0,
                        nEnd = 0;
                    aRectangle.Left() = 0;
                    for ( int i = 0; i < nCount; ++i )
                    {
                        SwColumn* pCol = rCols[i];
                        nStart = pCol->GetLeft() + nWidth;
                        if(i == nNum - 2)
                            aRectangle.Left() = nStart;
                        nWidth += pCols->CalcColWidth( i, nTotalWidth );
                        nEnd = nWidth - pCol->GetRight();
                    }
                    aRectangle.Right() = nPageWidth - nEnd;
                    aRectangle.Left() -= DOCUMENTBORDER;
                    aRectangle.Right() += DOCUMENTBORDER;
                    if(nNum > 1)
                    {
                        aRectangle.Left() += MINLAY;
                        aRectangle.Left() += aRect.Left();
                    }
                    if(pFmt) //Bereich in Rahmen - hier darf man bis zum Rand
                        aRectangle.Left()  = aRectangle.Right() = 0;
                    else
                    {
                        // das Rechteck an die richtige absolute Position verschieben
                        aRectangle.Left() += aAbsRect.Left();
                        aRectangle.Right() -= aAbsRect.Left();
                        // Abstand zur Umrandung mit einbeziehen
                        aRectangle.Right() -= (nOuterWidth - nTotalWidth) / 2;
                    }

                    if(nNum < rCols.Count())
                    {
                        aRectangle.Right() += MINLAY;
                    }
                    else
                        // rechts ist jetzt nur noch der Seitenrand
                        aRectangle.Right() = 0;


                }
            }
            else if ( ((nFrmType & FRMTYPE_TABLE) || IsTabColFromDoc()) &&
                 !bFrmSelection )
            {
                BOOL bColumn;
                if ( IsTabColFromDoc() )
                    bColumn = rSh.GetCurMouseColNum( aTabColFromDocPos ) != 0;
                else
                    bColumn = (nFrmType & (FRMTYPE_COLUMN|FRMTYPE_FLY_ANY|
                                            FRMTYPE_COLSECTOUTTAB)) ?
                                            TRUE : FALSE;
                if ( !bColumn )
                {
                    if( nFrmType & FRMTYPE_FLY_ANY && IsTabColFromDoc() )
                    {
                        SwRect aRect( rSh.GetAnyCurRect(
                                            RECT_FLY_PRT_EMBEDDED, pPt ) );
                        aRect.Pos() += rSh.GetAnyCurRect( RECT_FLY_EMBEDDED,
                                                                pPt ).Pos();

                        aRectangle.Left()  = aRect.Left() - DOCUMENTBORDER;
                        aRectangle.Right() = nPageWidth - ( aRect.Right()
                                                         - DOCUMENTBORDER );
                    }
                    else if( bBrowse )
                    {
                        aRectangle.Left()  = rPagePrtRect.Left();
                        aRectangle.Right() = nPageWidth - rPagePrtRect.Right();
                    }
                    else
                    {
                        aRectangle.Left()  = aPageLRSpace.GetLeft();
                        aRectangle.Right() = aPageLRSpace.GetRight();
                    }
                }
                else
                {   //hier nur fuer Tabelle in mehrspaltigen Seiten und Rahmen
                    BOOL bSectOutTbl = (nFrmType & FRMTYPE_TABLE) ? TRUE : FALSE;
                    BOOL bFrame = (nFrmType & FRMTYPE_FLY_ANY) ? TRUE : FALSE;
                    BOOL bColSct =  (nFrmType & ( bSectOutTbl
                                                    ? FRMTYPE_COLSECTOUTTAB
                                                    : FRMTYPE_COLSECT )
                                                ) ? TRUE : FALSE;
                    //Damit man auch mit der Mouse ziehen kann,
                    //ohne in der Tabelle zu stehen
                    CurRectType eRecType = RECT_PAGE_PRT;
                    int nNum = IsTabColFromDoc() ?
                                rSh.GetCurMouseColNum( aTabColFromDocPos ):
                                rSh.GetCurOutColNum();
                    const SwFrmFmt* pFmt = NULL;
                    if( bColSct )
                    {
                        eRecType = bSectOutTbl ? RECT_OUTTABSECTION
                                               : RECT_SECTION;
                        const SwSection *pSect = rSh.GetAnySection( bSectOutTbl );
                        ASSERT( pSect, "Welcher Bereich?");
                        pFmt = pSect->GetFmt();
                    }
                    else if( bFrame )
                    {
                        pFmt = rSh.GetFlyFrmFmt();
                        eRecType = RECT_FLY_PRT_EMBEDDED;
                    }

                    const SwFmtCol* pCols = pFmt ? &pFmt->GetCol():
                                                   &rDesc.GetMaster().GetCol();
                    const SwColumns& rCols = pCols->GetColumns();
                    const USHORT nBorder = pFmt ? pFmt->GetBox().GetDistance() :
                                                  rDesc.GetMaster().GetBox().GetDistance();

                    /* RECT_FLY_PRT_EMBEDDED returns the relative position to
                        RECT_FLY_EMBEDDED
                        the absolute position must be added here
                    */
                    SwRect aRect( rSh.GetAnyCurRect( eRecType, pPt ) );
                    if(RECT_FLY_PRT_EMBEDDED == eRecType)
                        aRect.Pos() += rSh.GetAnyCurRect( RECT_FLY_EMBEDDED,
                                                                pPt ).Pos();

                    const USHORT nTotalWidth = (USHORT)aRect.Width();
                    //nStart und nEnd initialisieren fuer nNum == 0
                    int nWidth = 0,
                        nStart = 0,
                        nEnd = nTotalWidth;

                    if( nNum > rCols.Count() )
                    {
                        ASSERT( !this, "es wird auf dem falschen FmtCol gearbeitet!" );
                        nNum = rCols.Count();
                    }

                    for( int i = 0; i < nNum; ++i )
                    {
                        SwColumn* pCol = rCols[i];
                        nStart = pCol->GetLeft() + nWidth;
                        nWidth += pCols->CalcColWidth( i, nTotalWidth );
                        nEnd = nWidth - pCol->GetRight();
                    }
                    if( bFrame | bColSct )
                    {
                        aRectangle.Left()  = aRect.Left()
                                                - DOCUMENTBORDER + nStart;
                        aRectangle.Right() = nPageWidth
                                                - aRectangle.Left()
                                                    - nEnd + nStart;
                    }
                    else if(!bBrowse)
                    {
                        aRectangle.Left()  = aPageLRSpace.GetLeft() + nStart;
                        aRectangle.Right() = nPageWidth - nEnd - aPageLRSpace.GetLeft();
                    }
                    else
                    {
                        long nLeft = rPagePrtRect.Left();
                        aRectangle.Left()  = nStart + nLeft;
                        aRectangle.Right() = nPageWidth - nEnd - nLeft;
                    }
                    if(!bFrame)
                    {
                        aRectangle.Left() += nBorder;
                        aRectangle.Right() -= nBorder;
                    }
                }
            }
            else if ( nFrmType & ( FRMTYPE_HEADER  | FRMTYPE_FOOTER ))
            {
                aRectangle.Left()  = aPageLRSpace.GetLeft();
                aRectangle.Right() = aPageLRSpace.GetRight();
            }
            else
                aRectangle.Left()  = aRectangle.Right() = 0;

            SfxRectangleItem aLR( SID_RULER_LR_MIN_MAX , aRectangle);
            rSet.Put(aLR);
        }
        break;
        case SID_RULER_PROTECT:
        {
            if(bFrmSelection)
            {
                BYTE nProtect = pWrtShell->IsSelObjProtected((FlyProtectType)(FLYPROTECT_SIZE|FLYPROTECT_POS|FLYPROTECT_CONTENT));

                SvxProtectItem aProt(SID_RULER_PROTECT);
                aProt.SetCntntProtect((nProtect & FLYPROTECT_CONTENT)   != 0);
                aProt.SetSizeProtect ((nProtect & FLYPROTECT_SIZE)      != 0);
                aProt.SetPosProtect  ((nProtect & FLYPROTECT_POS)       != 0);
                rSet.Put(aProt);
            }
            else
            {
                SvxProtectItem aProtect(SID_RULER_PROTECT);
                if(bBrowse && !(nFrmType & (FRMTYPE_DRAWOBJ|FRMTYPE_COLUMN)) && !rSh.GetTableFmt())
                {
                    aProtect.SetSizeProtect(TRUE);
                    aProtect.SetPosProtect(TRUE);
                }
                rSet.Put(aProtect);
            }
        }
        break;
        }
        nWhich = aIter.NextWhich();
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.130  2000/09/18 16:06:14  willem.vandorp
    OpenOffice header added.

    Revision 1.129  2000/09/07 15:59:33  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.128  2000/08/29 13:59:32  ama
    New: Negative margins

    Revision 1.127  2000/08/23 12:49:45  kz
    function Max (long, long)

    Revision 1.126  2000/05/26 07:21:35  os
    old SW Basic API Slots removed

    Revision 1.125  2000/05/10 11:53:20  os
    Basic API removed

    Revision 1.124  2000/04/18 15:02:51  os
    UNICODE

    Revision 1.123  2000/01/17 13:32:55  jp
    Bug #70616#: StateTabWin - if IsTabColFromDoc-Flag set then get/set info only over layoutpos

    Revision 1.122  1999/11/23 14:56:37  os
    #69106# LR_MIN_MAX in frames corrected

    Revision 1.121  1999/06/21 07:20:54  OS
    #66284##66928# correct display of frames with columns


      Rev 1.120   21 Jun 1999 09:20:54   OS
   #66284##66928# correct display of frames with columns

      Rev 1.119   04 Jun 1999 09:29:22   OS
   #66284# Rahmen nach Attributanwendung deselektieren

      Rev 1.118   04 Jun 1999 09:00:04   OS
   #66436# spaltige Bereiche auf spaltigen Seiten funktionieren jetzt auch

      Rev 1.117   17 May 1999 13:06:14   OS
   auch einspaltige Rahmen zeigen keine Spalten an

      Rev 1.116   11 May 1999 21:51:06   JP
   Task #66127#: Methoden rund ums Lineal verbessert und Schnittstellen veraendert/erweitert

      Rev 1.115   28 Apr 1999 22:38:36   JP
   Bug #65544#: StateTabWin: bei Tabellen in Bereichen mit den richtigen Werten arbeiten

      Rev 1.114   23 Apr 1999 15:15:36   AMA
   Fix #65252#: Nichtspaltige Bereiche in Tabellen in spaltigen Bereichen

      Rev 1.113   20 Apr 1999 14:08:26   OS
   #64974# Probleme mit spaltigen Bereichen beoben

      Rev 1.112   06 Apr 1999 16:29:32   OS
   #60062# spaltige Bereiche auch in Tabellen und spaltigen Rahmen fast richtig

      Rev 1.111   10 Dec 1998 15:17:06   AMA
   Fix #57749#: Fussnoten innerhalb von spaltigen Bereichen

      Rev 1.110   03 Dec 1998 09:19:20   OS
   #60062# LR_MIN_MAX fuer spaltige Bereiche sollte jetzt richtig sein

      Rev 1.109   27 Nov 1998 14:57:32   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.108   18 Sep 1998 16:55:54   OS
   #56584# WishWidth ueber USHRT_MAX verhindern

      Rev 1.107   20 Aug 1998 13:30:46   OS
   GetFrmType reicht fuer verkettete Rahmen nicht aus -> IsFrmSelected benutzen
   Keine Absatzattribute fuer sel. Rahmen anzeigen #55277#,#55257#

      Rev 1.106   01 Apr 1998 17:31:48   OM
   #31111 Rahmen in protecteten Rahmen sind auch geschuetzt

      Rev 1.105   04 Mar 1998 10:01:14   MH
   chg: wg. internal Compilererrror

      Rev 1.104   07 Feb 1998 10:38:12   OS
   Set/GetStyle am Lineal fuer BrowseMode-Umschaltung #41371#

      Rev 1.103   21 Nov 1997 15:00:24   MA
   includes

      Rev 1.102   15 Sep 1997 11:49:12   OS
   zusaetzlicher Kommentar

      Rev 1.101   15 Sep 1997 11:29:30   OS
   Slotumleitung nicht ueber Dispatcher #43754#

      Rev 1.100   12 Sep 1997 10:36:08   OS
   ITEMID_* definiert

      Rev 1.99   10 Sep 1997 16:55:32   OS
   Rahmen: Absatzeinzuege innerhalb der Umrandung #42444#

      Rev 1.98   15 Aug 1997 11:47:58   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.97   11 Aug 1997 10:28:32   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.96   07 Aug 1997 14:58:58   OM
   Headerfile-Umstellung

      Rev 1.95   06 Aug 1997 10:27:54   OS
   Seitenraender richtig tauschen #42452#

      Rev 1.94   30 Jul 1997 19:05:50   HJS
   includes

      Rev 1.93   29 Jul 1997 14:22:40   AMA
   Fix #42203#: GPF durch nicht zurueckgesetztes bSetTabCol...

      Rev 1.92   09 Jul 1997 12:54:30   OS
   MinMaxItem im BrowseMode mit richtigen Raendern fuellen #41484#

      Rev 1.91   17 Jun 1997 15:44:30   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.90   09 Jun 1997 14:28:10   MA
   chg: Browse-Flag nur noch am Doc

      Rev 1.89   07 Apr 1997 19:06:32   MH
   chg: header

      Rev 1.88   12 Mar 1997 16:42:38   OS
   AutoUpdate von Vorlagen: Absatzeinzuege, Tabulatoren

      Rev 1.87   14 Feb 1997 09:13:42   JP
   Bug #36135#: ExceTabWin - bei SetMouseTabCols die View locken

      Rev 1.86   13 Feb 1997 12:06:14   MA
   chg: Keine Tabulatoren im Lineal fuer Web

      Rev 1.85   04 Feb 1997 08:14:32   OS
   relative Rahmen richtig setzen

      Rev 1.84   08 Jan 1997 10:55:00   OS
   include fuer DEBUGLIN

      Rev 1.83   28 Nov 1996 14:58:18   OS
   keine Tabulatoren bei uneindeutigen Absatzeinzuegen

      Rev 1.82   26 Nov 1996 16:55:20   OS
   MinMax fuer Tabellen im BrowseMode korrigiert

      Rev 1.81   21 Nov 1996 11:52:52   OS
   Abstaende beruecksichtigen

      Rev 1.80   04 Nov 1996 13:30:36   OS
   UL-Spaces an Rahmen fuer das VLineal beruecksichtigen

      Rev 1.79   24 Oct 1996 09:32:46   OS
   Klammern vergessen...

      Rev 1.78   21 Oct 1996 19:06:48   OS
   ProtectItem vollstaendig senden; Exec-Methoden in der BrowseView stimmen jetzt

      Rev 1.77   18 Oct 1996 08:57:42   OS
   spaltige Rahmen und seiten wieder richtig anzeigen, ProtectItem fuer die Seitenraender mitschicken

      Rev 1.76   25 Sep 1996 10:56:52   PL
   Falsches Makro

      Rev 1.75   24 Sep 1996 16:44:04   OS
   letzter Bug in BrowseView

      Rev 1.74   23 Sep 1996 19:38:32   HJS
   add: viewopt.hxx

      Rev 1.73   23 Sep 1996 15:36:32   MA
   ViewWin vernichtet

      Rev 1.72   23 Sep 1996 12:13:32   OS
   richtige Raender in der BrowseView

      Rev 1.71   12 Sep 1996 17:00:34   OS
   GetAnyCurRect() ersetzt GetCur*Rect

------------------------------------------------------------------------*/



