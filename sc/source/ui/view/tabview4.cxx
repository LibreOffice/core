/*************************************************************************
 *
 *  $RCSfile: tabview4.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#define _MACRODLG_HXX
#define _BIGINT_HXX
#define _SVCONTNR_HXX
#define BASIC_NODIALOGS
#define _SFXMNUITEM_HXX
#define _SVDXOUT_HXX
#define _SVDATTR_HXX
#define _SFXMNUITEM_HXX
#define _DLGCFG_HXX
#define _SFXMNUMGR_HXX
#define _SFXBASIC_HXX
#define _MODALDLG_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXMGR_HXX
#define _BASE_DLGS_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUMGR_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX
#define _PASSWD_HXX
//#define _SFXFILEDLG_HXX
//#define _SFXREQUEST_HXX
#define _SFXOBJFACE_HXX

// INCLUDE ---------------------------------------------------------------

#include <vcl/help.hxx>

#include "tabview.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "gridwin.hxx"
#include "globstr.hrc"
#include "cell.hxx"
#include "dociter.hxx"

extern USHORT nScFillModeMouseModifier;             // global.cxx

// STATIC DATA -----------------------------------------------------------

//==================================================================

//
// ---  Referenz-Eingabe / Fill-Cursor
//

void ScTabView::HideTip()
{
    if ( nTipVisible )
    {
        Help::HideTip( nTipVisible );
        nTipVisible = 0;
    }
}

void ScTabView::ShowRefTip()
{
    BOOL bDone = FALSE;
    if ( aViewData.GetRefType() == SC_REFTYPE_REF && Help::IsQuickHelpEnabled() )
    {
        USHORT nStartX = aViewData.GetRefStartX();
        USHORT nStartY = aViewData.GetRefStartY();
        USHORT nEndX   = aViewData.GetRefEndX();
        USHORT nEndY   = aViewData.GetRefEndY();
        if ( nEndX != nStartX || nEndY != nStartY )     // nicht fuer einzelne Zelle
        {
            BOOL bLeft = ( nEndX < nStartX );
            BOOL bTop  = ( nEndY < nStartY );
            PutInOrder( nStartX, nEndX );
            PutInOrder( nStartY, nEndY );
            USHORT nCols = nEndX+1-nStartX;
            USHORT nRows = nEndY+1-nStartY;

            String aHelp = ScGlobal::GetRscString( STR_QUICKHELP_REF );
            aHelp.SearchAndReplace( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("%1")),
                                    String::CreateFromInt32(nRows) );
            aHelp.SearchAndReplace( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("%2")),
                                    String::CreateFromInt32(nCols) );

            ScSplitPos eWhich = aViewData.GetActivePart();
            Window* pWin = pGridWin[eWhich];
            if ( pWin )
            {
                Point aStart = aViewData.GetScrPos( nStartX, nStartY, eWhich );
                Point aEnd = aViewData.GetScrPos( nEndX+1, nEndY+1, eWhich );

                Point aPos( bLeft ? aStart.X() : ( aEnd.X() + 3 ),
                            bTop ? aStart.Y() : ( aEnd.Y() + 3 ) );
                USHORT nFlags = ( bLeft ? QUICKHELP_RIGHT : QUICKHELP_LEFT ) |
                                ( bTop ? QUICKHELP_BOTTOM : QUICKHELP_TOP );

                // nicht ueber die editierte Formel
                if ( !bTop && aViewData.HasEditView( eWhich ) &&
                        nEndY+1 == aViewData.GetEditViewRow() )
                {
                    //  dann an der oberen Kante der editierten Zelle ausrichten
                    aPos.Y() -= 2;      // die 3 von oben
                    nFlags = ( nFlags & ~QUICKHELP_TOP ) | QUICKHELP_BOTTOM;
                }

                Rectangle aRect( pWin->OutputToScreenPixel( aPos ), Size(1,1) );

                //! Test, ob geaendert ??

                HideTip();
                nTipVisible = Help::ShowTip( pWin, aRect, aHelp, nFlags );
                bDone = TRUE;
            }
        }
    }

    if (!bDone)
        HideTip();
}

void ScTabView::StopRefMode()
{
    //  SC_FOLLOW_NONE: nur aktiven Part umschalten
    //  auch ohne IsRefMode, fuer RangeFinder-Verschieberei
    AlignToCursor( aViewData.GetCurX(), aViewData.GetCurY(), SC_FOLLOW_NONE );

    if (aViewData.IsRefMode())
    {
        aViewData.SetRefMode( FALSE, SC_REFTYPE_NONE );

        HideTip();

        if ( aViewData.GetTabNo() >= aViewData.GetRefStartZ() &&
                aViewData.GetTabNo() <= aViewData.GetRefEndZ() )
        {
            ScDocument* pDoc = aViewData.GetDocument();
            USHORT nStartX = aViewData.GetRefStartX();
            USHORT nStartY = aViewData.GetRefStartY();
            USHORT nEndX = aViewData.GetRefEndX();
            USHORT nEndY = aViewData.GetRefEndY();
            if ( nStartX == nEndX && nStartY == nEndY )
                pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, aViewData.GetTabNo() );

            PaintArea( nStartX,nStartY,nEndX,nEndY, SC_UPDATE_MARKS );
        }

        pSelEngine->Reset();
        pSelEngine->SetAddMode( FALSE );        //! sollte das nicht bei Reset passieren?

        ScSplitPos eOld = pSelEngine->GetWhich();
        ScSplitPos eNew = aViewData.GetActivePart();
        if ( eNew != eOld )
        {
            pSelEngine->SetWindow( pGridWin[ eNew ] );
            pSelEngine->SetWhich( eNew );
            pSelEngine->SetVisibleArea( Rectangle(Point(),
                                        pGridWin[eNew]->GetOutputSizePixel()) );
            pGridWin[eOld]->MoveMouseStatus(*pGridWin[eNew]);
        }
    }
}

void ScTabView::DoneRefMode( BOOL bContinue )
{
    ScDocument* pDoc = aViewData.GetDocument();
    if ( aViewData.GetRefType() == SC_REFTYPE_REF && bContinue )
        SC_MOD()->AddRefEntry();

    BOOL bWasRef = aViewData.IsRefMode();
    aViewData.SetRefMode( FALSE, SC_REFTYPE_NONE );

    HideTip();

    //  Paint:
    if ( bWasRef && aViewData.GetTabNo() >= aViewData.GetRefStartZ() &&
                    aViewData.GetTabNo() <= aViewData.GetRefEndZ() )
    {
        USHORT nStartX = aViewData.GetRefStartX();
        USHORT nStartY = aViewData.GetRefStartY();
        USHORT nEndX = aViewData.GetRefEndX();
        USHORT nEndY = aViewData.GetRefEndY();
        if ( nStartX == nEndX && nStartY == nEndY )
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, aViewData.GetTabNo() );

        PaintArea( nStartX,nStartY,nEndX,nEndY, SC_UPDATE_MARKS );
    }
}

void ScTabView::UpdateRef( USHORT nCurX, USHORT nCurY, USHORT nCurZ )
{
    ScDocument* pDoc = aViewData.GetDocument();

    if (!aViewData.IsRefMode())
    {
        //  Das kommt vor, wenn bei einem Referenz-Dialog als erstes mit Control in die
        //  die Tabelle geklickt wird. Dann die neue Referenz an den alten Inhalt anhaengen:

        ScModule* pScMod = SC_MOD();
        if (pScMod->IsFormulaMode())
            pScMod->AddRefEntry();

        InitRefMode( nCurX, nCurY, nCurZ, SC_REFTYPE_REF );
    }

    if ( nCurX != aViewData.GetRefEndX() || nCurY != aViewData.GetRefEndY() ||
         nCurZ != aViewData.GetRefEndZ() )
    {
        ScMarkData& rMark = aViewData.GetMarkData();
        USHORT nTab = aViewData.GetTabNo();

        USHORT nStartX = aViewData.GetRefStartX();
        USHORT nStartY = aViewData.GetRefStartY();
        USHORT nEndX = aViewData.GetRefEndX();
        USHORT nEndY = aViewData.GetRefEndY();
        if ( nStartX == nEndX && nStartY == nEndY )
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, nTab );
        ScUpdateRect aRect( nStartX, nStartY, nEndX, nEndY );

        aViewData.SetRefEnd( nCurX, nCurY, nCurZ );

        nStartX = aViewData.GetRefStartX();
        nStartY = aViewData.GetRefStartY();
        nEndX = aViewData.GetRefEndX();
        nEndY = aViewData.GetRefEndY();
        if ( nStartX == nEndX && nStartY == nEndY )
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, nTab );
        aRect.SetNew( nStartX, nStartY, nEndX, nEndY );

        ScRefType eType = aViewData.GetRefType();
        if ( eType == SC_REFTYPE_REF )
        {
            ScRange aRef(
                    aViewData.GetRefStartX(), aViewData.GetRefStartY(), aViewData.GetRefStartZ(),
                    aViewData.GetRefEndX(), aViewData.GetRefEndY(), aViewData.GetRefEndZ() );
            SC_MOD()->SetReference( aRef, pDoc, &rMark );
            ShowRefTip();
        }
        else if ( eType == SC_REFTYPE_EMBED_LT || eType == SC_REFTYPE_EMBED_RB )
        {
            PutInOrder(nStartX,nEndX);
            PutInOrder(nStartY,nEndY);
            pDoc->SetEmbedded( ScTripel(nStartX,nStartY,nTab), ScTripel(nEndX,nEndY,nTab) );
            ScDocShell* pDocSh = aViewData.GetDocShell();
            pDocSh->UpdateOle( &aViewData, TRUE );
            pDocSh->SetDocumentModified();
        }

        USHORT nPaintStartX;
        USHORT nPaintStartY;
        USHORT nPaintEndX;
        USHORT nPaintEndY;
        if (aRect.GetDiff( nPaintStartX, nPaintStartY, nPaintEndX, nPaintEndY ))
            PaintArea( nPaintStartX, nPaintStartY, nPaintEndX, nPaintEndY, SC_UPDATE_MARKS );
    }

    //  Tip-Hilfe fuer Auto-Fill

    if ( aViewData.GetRefType() == SC_REFTYPE_FILL && Help::IsQuickHelpEnabled() )
    {
        String aHelpStr;
        ScRange aMarkRange;
        aViewData.GetSimpleArea( aMarkRange );
        USHORT nEndX = aViewData.GetRefEndX();
        USHORT nEndY = aViewData.GetRefEndY();
        ScRange aDelRange;
        if ( aViewData.GetFillMode() == SC_FILL_MATRIX && !(nScFillModeMouseModifier & KEY_MOD1) )
        {
            aHelpStr = ScGlobal::GetRscString( STR_TIP_RESIZEMATRIX );
            USHORT nCols = nEndX + 1 - aViewData.GetRefStartX();    // Reihenfolge ist richtig
            USHORT nRows = nEndY + 1 - aViewData.GetRefStartY();
            aHelpStr.SearchAndReplace( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("%1")),
                                       String::CreateFromInt32(nRows) );
            aHelpStr.SearchAndReplace( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("%2")),
                                       String::CreateFromInt32(nCols) );
        }
        else if ( aViewData.GetDelMark( aDelRange ) )
            aHelpStr = ScGlobal::GetRscString( STR_QUICKHELP_DELETE );
        else if ( nEndX != aMarkRange.aEnd.Col() || nEndY != aMarkRange.aEnd.Row() )
            aHelpStr = pDoc->GetAutoFillPreview( aMarkRange, nEndX, nEndY );

        //  je nach Richtung die obere oder untere Ecke:
        USHORT nAddX = ( nEndX >= aMarkRange.aEnd.Col() ) ? 1 : 0;
        USHORT nAddY = ( nEndY >= aMarkRange.aEnd.Row() ) ? 1 : 0;
        Point aPos = aViewData.GetScrPos( nEndX+nAddX, nEndY+nAddY, aViewData.GetActivePart() );
        aPos.X() += 8;
        aPos.Y() += 4;
        Window* pWin = GetActiveWin();
        if ( pWin )
            aPos = pWin->OutputToScreenPixel( aPos );
        Rectangle aRect( aPos, aPos );
        USHORT nAlign = QUICKHELP_LEFT|QUICKHELP_TOP;
        Help::ShowQuickHelp(pWin, aRect, aHelpStr, nAlign);
    }
}

void ScTabView::InitRefMode( USHORT nCurX, USHORT nCurY, USHORT nCurZ, ScRefType eType, BOOL bPaint )
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    if (!aViewData.IsRefMode())
    {
        aViewData.SetRefMode( TRUE, eType );
        aViewData.SetRefStart( nCurX, nCurY, nCurZ );
        aViewData.SetRefEnd( nCurX, nCurY, nCurZ );

        if (nCurZ == aViewData.GetTabNo() && bPaint)
        {
            USHORT nStartX = nCurX;
            USHORT nStartY = nCurY;
            USHORT nEndX = nCurX;
            USHORT nEndY = nCurY;
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, aViewData.GetTabNo() );

            //! nur Markierung ueber Inhalte zeichnen!
            PaintArea( nStartX,nStartY,nEndX,nEndY, SC_UPDATE_MARKS );

            //  SetReference ohne Merge-Anpassung
            ScRange aRef( nCurX,nCurY,nCurZ, nCurX,nCurY,nCurZ );
            SC_MOD()->SetReference( aRef, pDoc, &rMark );
        }
    }
}

void ScTabView::EndSelection()
{
    ScModule* pScMod = SC_MOD();
    BOOL bRefMode = pScMod->IsFormulaMode();
    if ( bRefMode )
        pScMod->EndReference();
}

//  UpdateScrollBars - sichtbaren Bereich und Scrollweite der Scrollbars einstellen

long lcl_UpdateBar( ScrollBar& rScroll, USHORT nSize )      // Size = (komplette) Zellen
{
    long nOldPos;
    long nNewPos;

    nOldPos = rScroll.GetThumbPos();
    rScroll.SetPageSize( nSize );
    nNewPos = rScroll.GetThumbPos();
#ifndef UNX
    rScroll.SetPageSize( 1 );               // immer moeglich !
#endif

    return nNewPos - nOldPos;
}

void lcl_SetScrollRange( ScrollBar& rBar, USHORT nDocEnd, USHORT nPos, USHORT nVis,
                            USHORT nMax, USHORT nStart )
{
    ++nVis;
    ++nMax;     // fuer teilweise sichtbare Zellen
    USHORT nEnd = Max(nDocEnd, (USHORT)(nPos+nVis)) + nVis;
    if (nEnd > nMax)
        nEnd = nMax;

    rBar.SetRangeMax( nEnd - nStart );      // RangeMin muss selber verwaltet werden
}

void ScTabView::UpdateScrollBars()
{
    long        nDiff;
    BOOL        bTop =   ( aViewData.GetVSplitMode() != SC_SPLIT_NONE );
    BOOL        bRight = ( aViewData.GetHSplitMode() != SC_SPLIT_NONE );
    ScDocument* pDoc = aViewData.GetDocument();
    USHORT      nTab = aViewData.GetTabNo();
    USHORT      nUsedX;
    USHORT      nUsedY;
    pDoc->GetTableArea( nTab, nUsedX, nUsedY );     //! cachen !!!!!!!!!!!!!!!

    USHORT nVisXL = 0;
    USHORT nVisXR = 0;
    USHORT nVisYB = 0;
    USHORT nVisYT = 0;

    USHORT nStartX = 0;
    USHORT nStartY = 0;
    if (aViewData.GetHSplitMode()==SC_SPLIT_FIX)
        nStartX = aViewData.GetFixPosX();
    if (aViewData.GetVSplitMode()==SC_SPLIT_FIX)
        nStartY = aViewData.GetFixPosY();

    nVisXL = aViewData.VisibleCellsX( SC_SPLIT_LEFT );
    lcl_SetScrollRange( aHScrollLeft, nUsedX, aViewData.GetPosX(SC_SPLIT_LEFT), nVisXL,
                            MAXCOL, 0 );
    aHScrollLeft.SetVisibleSize( nVisXL );
    aHScrollLeft.SetThumbPos( aViewData.GetPosX( SC_SPLIT_LEFT ) );

    nVisYB = aViewData.VisibleCellsY( SC_SPLIT_BOTTOM );
    lcl_SetScrollRange( aVScrollBottom, nUsedY, aViewData.GetPosY(SC_SPLIT_BOTTOM), nVisYB,
                            MAXROW, nStartY );
    aVScrollBottom.SetVisibleSize( nVisYB );
    aVScrollBottom.SetThumbPos( aViewData.GetPosY( SC_SPLIT_BOTTOM ) - nStartY );

    if (bRight)
    {
        nVisXR = aViewData.VisibleCellsX( SC_SPLIT_RIGHT );
        lcl_SetScrollRange( aHScrollRight, nUsedX, aViewData.GetPosX(SC_SPLIT_RIGHT), nVisXR,
                            MAXCOL, nStartX );
        aHScrollRight.SetVisibleSize( nVisXR );
        aHScrollRight.SetThumbPos( aViewData.GetPosX( SC_SPLIT_RIGHT ) - nStartX );
    }

    if (bTop)
    {
        nVisYT = aViewData.VisibleCellsY( SC_SPLIT_TOP );
        lcl_SetScrollRange( aVScrollTop, nUsedY, aViewData.GetPosY(SC_SPLIT_TOP), nVisYT,
                            MAXROW, 0 );
        aVScrollTop.SetVisibleSize( nVisYT );
        aVScrollTop.SetThumbPos( aViewData.GetPosY( SC_SPLIT_TOP ) );
    }

    //      Bereich testen

    nDiff = lcl_UpdateBar( aHScrollLeft, nVisXL );
    if (nDiff) ScrollX( nDiff, SC_SPLIT_LEFT );
    if (bRight)
    {
        nDiff = lcl_UpdateBar( aHScrollRight, nVisXR );
        if (nDiff) ScrollX( nDiff, SC_SPLIT_RIGHT );
    }

    nDiff = lcl_UpdateBar( aVScrollBottom, nVisYB );
    if (nDiff) ScrollY( nDiff, SC_SPLIT_BOTTOM );
    if (bTop)
    {
        nDiff = lcl_UpdateBar( aVScrollTop, nVisYT );
        if (nDiff) ScrollY( nDiff, SC_SPLIT_TOP );
    }

    //      set visible area for online spelling

    if ( aViewData.IsActive() )
    {
        ScSplitPos eActive = aViewData.GetActivePart();
        ScHSplitPos eHWhich = WhichH( eActive );
        ScVSplitPos eVWhich = WhichV( eActive );
        USHORT nPosX = aViewData.GetPosX(eHWhich);
        USHORT nPosY = aViewData.GetPosY(eVWhich);
        USHORT nEndX = nPosX + ( ( eHWhich == SC_SPLIT_LEFT ) ? nVisXL : nVisXR );
        USHORT nEndY = nPosY + ( ( eVWhich == SC_SPLIT_TOP ) ? nVisYT : nVisYB );
        if ( nEndX > MAXCOL ) nEndX = MAXCOL;
        if ( nEndY > MAXROW ) nEndY = MAXROW;
        ScRange aVisible( nPosX, nPosY, nTab, nEndX, nEndY, nTab );
        if ( pDoc->SetVisibleSpellRange( aVisible ) )
            SC_MOD()->AnythingChanged();                // if visible area has changed
    }
}

#ifndef HDR_SLIDERSIZE
#define HDR_SLIDERSIZE      2
#endif

void ScTabView::InvertHorizontal( ScVSplitPos eWhich, long nDragPos )
{
    for (USHORT i=0; i<4; i++)
        if (WhichV((ScSplitPos)i)==eWhich)
        {
            ScGridWindow* pWin = pGridWin[i];
            if (pWin)
            {
                Rectangle aRect( 0,nDragPos, pWin->GetOutputSizePixel().Width()-1,nDragPos+HDR_SLIDERSIZE-1 );
                pWin->Update();
                pWin->DoInvertRect( aRect );    // Pixel
            }
        }
}

void ScTabView::InvertVertical( ScHSplitPos eWhich, long nDragPos )
{
    for (USHORT i=0; i<4; i++)
        if (WhichH((ScSplitPos)i)==eWhich)
        {
            ScGridWindow* pWin = pGridWin[i];
            if (pWin)
            {
                Rectangle aRect( nDragPos,0, nDragPos+HDR_SLIDERSIZE-1,pWin->GetOutputSizePixel().Height()-1 );
                pWin->Update();
                pWin->DoInvertRect( aRect );    // Pixel
            }
        }
}

//==================================================================

void ScTabView::InterpretVisible()
{
    //  make sure all visible cells are interpreted,
    //  so the next paint will not execute a macro function

    ScDocument* pDoc = aViewData.GetDocument();
    if ( !pDoc->GetAutoCalc() )
        return;

    USHORT nTab = aViewData.GetTabNo();
    for (USHORT i=0; i<4; i++)
    {
        //  rely on gridwin pointers to find used panes
        //  no IsVisible test in case the whole view is not yet shown

        if (pGridWin[i])
        {
            ScHSplitPos eHWhich = WhichH( ScSplitPos(i) );
            ScVSplitPos eVWhich = WhichV( ScSplitPos(i) );

            USHORT  nX1 = aViewData.GetPosX( eHWhich );
            USHORT  nY1 = aViewData.GetPosY( eVWhich );
            USHORT  nX2 = nX1 + aViewData.VisibleCellsX( eHWhich );
            USHORT  nY2 = nY1 + aViewData.VisibleCellsY( eVWhich );

            if (nX2 > MAXCOL) nX2 = MAXCOL;
            if (nY2 > MAXROW) nY2 = MAXROW;

            ScCellIterator aIter( pDoc, nX1, nY1, nTab, nX2, nY2, nTab );
            ScBaseCell* pCell = aIter.GetFirst();
            while ( pCell )
            {
                if ( pCell->GetCellType() == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell)->GetDirty() )
                    ((ScFormulaCell*)pCell)->Interpret();

                pCell = aIter.GetNext();
            }
        }
    }
}





