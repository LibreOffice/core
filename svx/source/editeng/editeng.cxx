/*************************************************************************
 *
 *  $RCSfile: editeng.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: mt $ $Date: 2001-02-27 16:37:36 $
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

#include <eeng_pch.hxx>

// #define _SOLAR__PRIVATE 1
#pragma hdrstop

#define USE_SVXFONT

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#include <svxfont.hxx>
#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>
#include <editstat.hxx>
#include <editdbg.hxx>
#include <eerdll2.hxx>
#include <eerdll.hxx>
#include <editeng.hrc>
#include <flditem.hxx>
#include <txtrange.hxx>
#include <vcl/graph.hxx>

#include "akrnitem.hxx"
#include "cntritem.hxx"
#include "colritem.hxx"
#include "crsditem.hxx"
#include "escpitem.hxx"
#include "fhgtitem.hxx"
#include "fontitem.hxx"
#include "fwdtitem.hxx"
#include "kernitem.hxx"
#include "lrspitem.hxx"
#include "postitem.hxx"
#include "shdditem.hxx"
#include "udlnitem.hxx"
#include "wghtitem.hxx"
#include "wrlmitem.hxx"
#include "brshitem.hxx"
#include "cscoitem.hxx"
#include "langitem.hxx"


#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif

#include <numitem.hxx>
#include <bulitem.hxx>

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _XPOLY_HXX //autogen
#include <xpoly.hxx>
#endif


#ifndef SVX_LIGHT
#include <srchdlg.hxx>
#endif

// Spaeter -> TOOLS\STRING.H (fuer Grep: WS_TARGET)

//using namespace ::rtl;
//using namespace ::utl;
using namespace ::com::sun::star;
//using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
//using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::linguistic2;



DBG_NAME( EditEngine );
DBG_NAMEEX( EditView );

#if defined ( DEBUG ) || defined ( DBG_UTIL )
static sal_Bool bDebugPaint = sal_False;
#endif

SV_IMPL_VARARR( EECharAttribArray, EECharAttrib );


// ----------------------------------------------------------------------
// EditEngine
// ----------------------------------------------------------------------
EditEngine::EditEngine( SfxItemPool* pItemPool )
{
    DBG_CTOR( EditEngine, 0 );
    pImpEditEngine = new ImpEditEngine( this, pItemPool );
}

EditEngine::~EditEngine()
{
    DBG_DTOR( EditEngine, 0 );
    delete pImpEditEngine;
}

void EditEngine::EnableUndo( sal_Bool bEnable )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->EnableUndo( bEnable );
}

sal_Bool EditEngine::IsUndoEnabled()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsUndoEnabled();
}

sal_Bool EditEngine::IsInUndo()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsInUndo();
}

SfxUndoManager& EditEngine::GetUndoManager()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetUndoManager();
}

void EditEngine::UndoActionStart( sal_uInt16 nId )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Aufruf von UndoActionStart im Undomodus!" );
    if ( !pImpEditEngine->IsInUndo() )
        pImpEditEngine->UndoActionStart( nId );
}

void EditEngine::UndoActionEnd( sal_uInt16 nId )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Aufruf von UndoActionEnd im Undomodus!" );
    if ( !pImpEditEngine->IsInUndo() )
        pImpEditEngine->UndoActionEnd( nId );
}

void EditEngine::SetRefDevice( OutputDevice* pRefDev )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetRefDevice( pRefDev );
}

OutputDevice* EditEngine::GetRefDevice() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetRefDevice();
}

void EditEngine::SetRefMapMode( const MapMode& rMapMode )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetRefMapMode( rMapMode );
}

MapMode EditEngine::GetRefMapMode()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetRefMapMode();
}

const SfxItemSet& EditEngine::GetEmptyItemSet()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEmptyItemSet();
}

void EditEngine::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect )
{
    DBG_CHKTHIS( EditEngine, 0 );
    Draw( pOutDev, rOutRect, Point( 0, 0 ) );
}

void EditEngine::Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation )
{
    DBG_CHKTHIS( EditEngine, 0 );
    // Mit 2 Punkten erzeugen, da bei Positivem Punkt, LONGMAX als Size
    // Bottom und Right im Bereich > LONGMAX landen.
    Rectangle aBigRec( -0x3FFFFFFF, -0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF );
    if( pOutDev->GetConnectMetaFile() )
        pOutDev->Push();
    Point aStartPos( rStartPos );
    if ( IsVertical() )
    {
        aStartPos.X() += GetPaperSize().Width();
        aStartPos = Rotate( aStartPos, nOrientation, rStartPos );
    }
    pImpEditEngine->Paint( pOutDev, aBigRec, aStartPos, sal_False, nOrientation );
    if( pOutDev->GetConnectMetaFile() )
        pOutDev->Pop();
}

void EditEngine::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos )
{
    Draw( pOutDev, rOutRect, rStartDocPos, sal_True );
}

void EditEngine::Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos, sal_Bool bClip )
{
    DBG_CHKTHIS( EditEngine, 0 );

#if defined( DBG_UTIL ) || defined( DEBUG )
    if ( bDebugPaint )
        EditDbg::ShowEditEngineData( this, sal_False );
#endif

    // Auf Pixelgrenze ausrichten, damit genau das gleiche
    // wie bei Paint().
    Rectangle aOutRect( pOutDev->LogicToPixel( rOutRect ) );
    aOutRect = pOutDev->PixelToLogic( aOutRect );

    Point aStartPos;
    if ( !IsVertical() )
    {
        aStartPos.X() = aOutRect.Left() - rStartDocPos.X();
        aStartPos.Y() = aOutRect.Top() - rStartDocPos.Y();
    }
    else
    {
        aStartPos.X() = aOutRect.Right() + rStartDocPos.Y();
        aStartPos.Y() = aOutRect.Top() - rStartDocPos.X();
    }

    sal_Bool bClipRegion = pOutDev->IsClipRegion();
    sal_Bool bMetafile = pOutDev->GetConnectMetaFile() ? sal_True : sal_False;
    Region aOldRegion = pOutDev->GetClipRegion();

#ifdef EDIT_PRINTER_LOG
    if ( pOutDev->GetOutDevType() == OUTDEV_PRINTER )
    {
        SvFileStream aLog( "d:\\editprn.log", STREAM_WRITE );
        aLog.Seek( STREAM_SEEK_TO_END );
        aLog << '' << endl << "Printing: ";
        aLog << GetText( "\n\r" ).GetStr();
        aLog << endl << endl;
         aLog << "Ref-Device: " << String( (sal_uInt32)GetRefDevice() ).GetStr() << " Type=" << String( (sal_uInt16)GetRefDevice()->GetOutDevType() ).GetStr() << ", MapX=" << String( GetRefDevice()->GetMapMode().GetScaleX().GetNumerator() ).GetStr() << "/" << String( GetRefDevice()->GetMapMode().GetScaleX().GetDenominator() ).GetStr() <<endl;
        aLog << "Paper-Width: " << String( GetPaperSize().Width() ).GetStr() << ",\tOut-Width: " << String( rOutRect.GetWidth() ).GetStr() << ",\tCalculated: " << String( CalcTextWidth() ).GetStr() << endl;
        aLog << "Paper-Height: " << String( GetPaperSize().Height() ).GetStr() << ",\tOut-Height: " << String( rOutRect.GetHeight() ).GetStr() << ",\tCalculated: " << String( GetTextHeight() ).GetStr() << endl;

        aLog << endl;
    }
#endif

    // Wenn es eine gab => Schnittmenge !
    // Bei der Metafileaufzeichnung Push/Pop verwenden.
    if ( bMetafile )
        pOutDev->Push();

    // Immer die Intersect-Methode, weil beim Metafile ein Muss!
    if ( bClip )
    {
        // Clip only if neccesary...
        if ( !rStartDocPos.X() && !rStartDocPos.Y() &&
             ( rOutRect.GetHeight() >= (long)GetTextHeight() ) &&
             ( rOutRect.GetWidth() >= (long)CalcTextWidth() ) )
        {
            bClip = FALSE;
        }
        else
        {
            // Einige Druckertreiber bereiten Probleme, wenn Buchstaben die
            // ClipRegion streifen, deshalb lieber ein Pixel mehr...
            Rectangle aClipRect( aOutRect );
            if ( pOutDev->GetOutDevType() == OUTDEV_PRINTER )
            {
                Size aPixSz( 1, 0 );
                aPixSz = pOutDev->PixelToLogic( aPixSz );
                aClipRect.Right() += aPixSz.Width();
                aClipRect.Bottom() += aPixSz.Width();
            }
            pOutDev->IntersectClipRegion( aClipRect );
        }
    }

    pImpEditEngine->Paint( pOutDev, aOutRect, aStartPos );

    if ( bMetafile )
        pOutDev->Pop();
    else if ( bClipRegion )
        pOutDev->SetClipRegion( aOldRegion );
    else
        pOutDev->SetClipRegion();
}

void EditEngine::InsertView( EditView* pEditView, sal_uInt16 nIndex )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_CHKOBJ( pEditView, EditView, 0 );

    if ( nIndex > pImpEditEngine->GetEditViews().Count() )
        nIndex = pImpEditEngine->GetEditViews().Count();

    pImpEditEngine->GetEditViews().Insert( pEditView, nIndex );
    EditSelection aStartSel;
    aStartSel = pImpEditEngine->GetEditDoc().GetStartPaM();
    pEditView->pImpEditView->SetEditSelection( aStartSel );
    if ( !pImpEditEngine->GetActiveView() )
        pImpEditEngine->SetActiveView( pEditView );
}

EditView* EditEngine::RemoveView( EditView* pView )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_CHKOBJ( pView, EditView, 0 );

    pView->HideCursor();
    EditView* pRemoved = 0;
    sal_uInt16 nPos = pImpEditEngine->GetEditViews().GetPos( pView );
    DBG_ASSERT( nPos != USHRT_MAX, "RemoveView mit ungueltigem Index" );
    if ( nPos != USHRT_MAX )
    {
        pRemoved = pImpEditEngine->GetEditViews().GetObject( nPos );
        pImpEditEngine->GetEditViews().Remove( nPos );
        if ( pImpEditEngine->GetActiveView() == pView )
        {
            pImpEditEngine->SetActiveView( 0 );
            pImpEditEngine->GetSelEngine().SetCurView( 0 );
        }
    }
    return pRemoved;
}

EditView* EditEngine::RemoveView( sal_uInt16 nIndex )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditView* pView = pImpEditEngine->GetEditViews().GetObject( nIndex );
    if ( pView )
        return RemoveView( pView );
    return NULL;
}

EditView* EditEngine::GetView( sal_uInt16 nIndex ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditViews().GetObject( nIndex );
}

sal_uInt16 EditEngine::GetViewCount() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditViews().Count();
}

sal_Bool EditEngine::HasView( EditView* pView ) const
{
    return pImpEditEngine->GetEditViews().GetPos( pView ) != USHRT_MAX;
}

EditView* EditEngine::GetActiveView() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetActiveView();
}

void EditEngine::SetActiveView( EditView* pView )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( pView )
        DBG_CHKOBJ( pView, EditView, 0 );
    pImpEditEngine->SetActiveView( pView );
}

void EditEngine::SetDefTab( sal_uInt16 nDefTab )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->GetEditDoc().SetDefTab( nDefTab );
    if ( pImpEditEngine->IsFormatted() )
    {
        pImpEditEngine->FormatFullDoc();
        pImpEditEngine->UpdateViews( (EditView*) 0 );
    }
}

sal_uInt16 EditEngine::GetDefTab() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditDoc().GetDefTab();
}

void EditEngine::SetPaperSize( const Size& rNewSize )
{
    DBG_CHKTHIS( EditEngine, 0 );

    Size aOldSize( pImpEditEngine->GetPaperSize() );
    pImpEditEngine->SetValidPaperSize( rNewSize );
    Size aNewSize( pImpEditEngine->GetPaperSize() );

    sal_Bool bAutoPageSize = pImpEditEngine->GetStatus().AutoPageSize();
    if ( bAutoPageSize || ( aNewSize.Width() != aOldSize.Width() ) )
    {
        for ( sal_uInt16 nView = 0; nView < pImpEditEngine->aEditViews.Count(); nView++ )
        {
            EditView* pView = pImpEditEngine->aEditViews[nView];
             DBG_CHKOBJ( pView, EditView, 0 );
            if ( bAutoPageSize )
                pView->pImpEditView->RecalcOutputArea();
            else if ( pView->pImpEditView->DoAutoSize() )
            {
                pView->pImpEditView->ResetOutputArea( Rectangle(
                    pView->pImpEditView->GetOutputArea().TopLeft(), aNewSize ) );
            }
        }

        if ( bAutoPageSize || pImpEditEngine->IsFormatted() )
        {
            // Aendern der Breite hat bei AutoPageSize keine Wirkung, da durch
            // Textbreite bestimmt.
            // Optimierung erst nach Vobis-Auslieferung aktivieren...
//          if ( !bAutoPageSize )
                pImpEditEngine->FormatFullDoc();
//          else
//          {
//              pImpEditEngine->FormatDoc();            // PageSize, falls Aenderung
//              pImpEditEngine->CheckAutoPageSize();    // Falls nichts formatiert wurde
//          }

            pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );

            if ( pImpEditEngine->GetUpdateMode() && pImpEditEngine->GetActiveView() )
                pImpEditEngine->pActiveView->ShowCursor( sal_False, sal_False );
        }
    }
}

const Size& EditEngine::GetPaperSize() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetPaperSize();
}

void EditEngine::SetVertical( BOOL bVertical )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetVertical( bVertical );
}

BOOL EditEngine::IsVertical() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsVertical();
}

USHORT EditEngine::GetScriptType( const ESelection& rSelection ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->CreateSel( rSelection ) );
    return pImpEditEngine->GetScriptType( aSel );
}

void EditEngine::SetPolygon( const XPolyPolygon& rPoly )
{
    DBG_CHKTHIS( EditEngine, 0 );
    SetPolygon( rPoly, NULL );
}

void EditEngine::SetPolygon( const XPolyPolygon& rPoly, const XPolyPolygon* pXorPoly )
{
    DBG_CHKTHIS( EditEngine, 0 );

    sal_Bool bSimple = sal_False;
    if( pXorPoly && rPoly.Count() == 1 )
    {
        // Handelt es sich um eine offene Kurve?
        const XPolygon& rP = rPoly[0];
        USHORT nCnt = rP.GetPointCount();

        if( ( nCnt == 0 ) || ( rP[0] != rP[nCnt - 1] ) )
            // Offene Kurve
            bSimple = sal_True;
    }

    TextRanger* pRanger = new TextRanger( rPoly, pXorPoly, 30, 2, 2, bSimple, sal_True );
    pImpEditEngine->SetTextRanger( pRanger );
    pImpEditEngine->SetPaperSize( pRanger->GetBoundRect().GetSize() );
}

void EditEngine::ClearPolygon()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetTextRanger( 0 );
}

const PolyPolygon* EditEngine::GetPolygon()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetTextRanger() ?
        &pImpEditEngine->GetTextRanger()->GetPolyPolygon() : NULL;
}

const Size& EditEngine::GetMinAutoPaperSize() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetMinAutoPaperSize();
}

void EditEngine::SetMinAutoPaperSize( const Size& rSz )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetMinAutoPaperSize( rSz );
}

const Size& EditEngine::GetMaxAutoPaperSize() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetMaxAutoPaperSize();
}

void EditEngine::SetMaxAutoPaperSize( const Size& rSz )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetMaxAutoPaperSize( rSz );
}

XubString EditEngine::GetText( LineEnd eEnd ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditDoc().GetText( eEnd );
}

XubString EditEngine::GetText( const ESelection& rESelection, const LineEnd eEnd ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->CreateSel( rESelection ) );
    return pImpEditEngine->GetSelected( aSel, eEnd );
}

sal_uInt32 EditEngine::GetTextLen() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditDoc().GetTextLen();
}

sal_uInt16 EditEngine::GetParagraphCount() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aEditDoc.Count();
}

sal_uInt16 EditEngine::GetLineCount( sal_uInt16 nParagraph ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetLineCount( nParagraph );
}

sal_uInt16 EditEngine::GetLineLen( sal_uInt16 nParagraph, sal_uInt16 nLine ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetLineLen( nParagraph, nLine );
}

sal_uInt32 EditEngine::GetLineHeight( sal_uInt16 nParagraph, sal_uInt16 nLine )
{
    DBG_CHKTHIS( EditEngine, 0 );
    // Falls jemand mit einer leeren Engine ein GetLineHeight() macht.
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    return pImpEditEngine->GetLineHeight( nParagraph, nLine );
}

sal_uInt32 EditEngine::GetMaxFontHeight( sal_uInt16 nParagraph, sal_uInt16 nLine )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetMaxFontHeight( nParagraph, nLine );
}

sal_uInt16 EditEngine::GetFirstLineOffset( sal_uInt16 nParagraph )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();
    ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nParagraph );
    return ( pPortion ? pPortion->GetFirstLineOffset() : 0 );
}

sal_uInt32 EditEngine::GetTextHeight( sal_uInt16 nParagraph ) const
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nHeight = pImpEditEngine->GetParaHeight( nParagraph );
     return nHeight;
}

XubString EditEngine::GetWord( sal_uInt16 nPara, sal_uInt16 nIndex )
{
    ESelection aESel( nPara, nIndex, nPara, nIndex );
    EditSelection aSel( pImpEditEngine->CreateSel( aESel ) );
    aSel = pImpEditEngine->SelectWord( aSel );
    return pImpEditEngine->GetSelected( aSel );
}

sal_Bool EditEngine::PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pEditView )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_CHKOBJ( pEditView, EditView, 0 );
    DBG_ASSERT( pEditView, "Keine View - keine Kekse !" );

    sal_Bool bDone = sal_True;

#ifndef SVX_LIGHT

    sal_Bool bModified  = sal_False;
    sal_Bool bMoved     = sal_False;
    sal_Bool bEndKey    = sal_False;    // spezielle CursorPosition
    sal_Bool bAllowIdle = sal_True;
    sal_Bool bReadOnly  = pEditView->IsReadOnly();

    EditSelection aCurSel( pEditView->pImpEditView->GetEditSelection() );
    DBG_ASSERT( !aCurSel.IsInvalid(), "Blinde Selection in EditEngine::PostKeyEvent" );

    String aAutoText( pImpEditEngine->GetAutoCompleteText() );
    if ( pImpEditEngine->GetAutoCompleteText().Len() )
        pImpEditEngine->SetAutoCompleteText( String(), sal_True );

    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !bReadOnly )
                {
                    pImpEditEngine->UndoActionStart( EDITUNDO_CUT );
                    aCurSel = pImpEditEngine->CutCopy( pEditView, sal_True );
                    pImpEditEngine->UndoActionEnd( EDITUNDO_CUT );
                    bModified = sal_True;
                }
            }
            break;
            case KEYFUNC_COPY:
            {
                aCurSel = pImpEditEngine->CutCopy( pEditView, sal_False );
            }
            break;
            case KEYFUNC_PASTE:
            {
                if ( !bReadOnly && pEditView->IsPasteEnabled() )
                {
                    pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );
                    aCurSel = pImpEditEngine->Paste( pEditView, pImpEditEngine->GetStatus().AllowPasteSpecial() );
                    pImpEditEngine->UndoActionEnd( EDITUNDO_PASTE );
                    bModified = sal_True;
                }
            }
            break;
            case KEYFUNC_UNDO:
            {
                if ( !bReadOnly )
                    pEditView->Undo();
                return sal_True;
            }
            // break;
            case KEYFUNC_REDO:
            {
                if ( !bReadOnly )
                    pEditView->Redo();
                return sal_True;
            }
            // break;

            default:    // wird dann evtl. unten bearbeitet.
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }
    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( nCode )
        {
           #if defined( DBG_UTIL ) || defined( DEBUG )
            case KEY_F11:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    bDebugPaint = !bDebugPaint;
                    ByteString aInfo( "DebugPaint: " );
                    aInfo += bDebugPaint ? "On" : "Off";
                    InfoBox( NULL, String( aInfo, RTL_TEXTENCODING_ASCII_US ) ).Execute();
                }
                bDone = FALSE;
            }
            break;
            case KEY_F12:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    EditDbg::ShowEditEngineData( this );
                    #ifdef EDIT_PRINTER_LOG
                        SvFileStream aLog( "d:\\editprn.log", STREAM_WRITE );
                        aLog.Seek( STREAM_SEEK_TO_END );
                        aLog << '' << endl << "Debug: ";
                        aLog << GetText( "\n\r" ).GetStr();
                        aLog << endl << endl;
                        aLog << "Ref-Device: " << String( (sal_uInt32)GetRefDevice() ).GetStr() << " Type=" << String( (sal_uInt16)GetRefDevice()->GetOutDevType() ).GetStr() << ", MapX=" << String( GetRefDevice()->GetMapMode().GetScaleX().GetNumerator() ).GetStr() << "/" << String( GetRefDevice()->GetMapMode().GetScaleX().GetDenominator() ).GetStr() <<endl;
                        aLog << "Paper-Width: " << String( GetPaperSize().Width() ).GetStr() << ",\tCalculated: " << String( CalcTextWidth() ).GetStr() << endl;
                        aLog << "Paper-Height: " << String( GetPaperSize().Height() ).GetStr() << ",\tCalculated: " << String( GetTextHeight() ).GetStr() << endl;
                        aLog << endl;
                    #endif
                }
                bDone = FALSE;
            }
            break;
           #endif
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    aCurSel = pImpEditEngine->MoveCursor( rKeyEvent, pEditView );
                    bMoved = sal_True;
                    if ( nCode == KEY_END )
                        bEndKey = sal_True;
                }
            }
            break;
            case KEY_BACKSPACE:
            case KEY_DELETE:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    BYTE nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                    BYTE nMode = rKeyEvent.GetKeyCode().IsMod1() ? DELMODE_RESTOFWORD : DELMODE_SIMPLE;
                    if ( ( nMode == DELMODE_RESTOFWORD ) && rKeyEvent.GetKeyCode().IsShift() )
                        nMode = DELMODE_RESTOFCONTENT;
                    pEditView->pImpEditView->DrawSelection();
                    pImpEditEngine->UndoActionStart( EDITUNDO_DELETE );
                    aCurSel = pImpEditEngine->DeleteLeftOrRight( aCurSel, nDel, nMode );
                    pImpEditEngine->UndoActionEnd( EDITUNDO_DELETE );
                    bModified = sal_True;
                    bAllowIdle = sal_False;
                }
            }
            break;
            case KEY_TAB:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    sal_Bool bShift = rKeyEvent.GetKeyCode().IsShift();
                    if ( pImpEditEngine->GetStatus().DoTabIndenting() &&
                        ( aCurSel.Min().GetNode() != aCurSel.Max().GetNode() ) )
                    {
                        pImpEditEngine->IndentBlock( pEditView, !bShift );
                    }
                    else if ( !bShift )
                    {
                        sal_Bool bSel = pEditView->HasSelection();
                        if ( bSel )
                            pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
                        if ( pImpEditEngine->GetStatus().DoAutoCorrect() )
                            aCurSel = pImpEditEngine->AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode() );
                        aCurSel = pImpEditEngine->InsertTab( aCurSel );
                        if ( bSel )
                            pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
                        bModified = sal_True;
                    }
                }
                else
                    bDone = sal_False;
            }
            break;
            case KEY_RETURN:
            {
                if ( !bReadOnly )
                {
                    pEditView->pImpEditView->DrawSelection();
                    if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    {
                        pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
                        if ( rKeyEvent.GetKeyCode().IsShift() )
                            aCurSel = pImpEditEngine->InsertLineBreak( aCurSel );
                        else
                        {
                            if ( !aAutoText.Len() )
                            {
                                if ( pImpEditEngine->GetStatus().DoAutoCorrect() )
                                    aCurSel = pImpEditEngine->AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode() );
                                aCurSel = pImpEditEngine->InsertParaBreak( aCurSel );
                            }
                            else
                            {
                                DBG_ASSERT( !aCurSel.HasRange(), "Selektion bei Complete?!" );
                                EditPaM aStart( pImpEditEngine->WordLeft( aCurSel.Max() ) );
                                aCurSel = pImpEditEngine->InsertText(
                                                EditSelection( aStart, aCurSel.Max() ), aAutoText );
                                pImpEditEngine->SetAutoCompleteText( String(), sal_True );
                            }
                        }
                        pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
                        bModified = sal_True;
                    }
                }
            }
            break;
            case KEY_INSERT:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    pEditView->SetInsertMode( !pEditView->IsInsertMode() );
            }
            break;
            default:
            {
                if ( !bReadOnly && IsSimpleCharInput( rKeyEvent ) )
                {
                    xub_Unicode nCharCode = rKeyEvent.GetCharCode();
                    pEditView->pImpEditView->DrawSelection();
                    // Autokorrektur ?
                    if ( ( pImpEditEngine->GetStatus().DoAutoCorrect() ) &&
                         (  ( nCharCode == ' ' ) || ( nCharCode == '*' ) ||
                             ( nCharCode == '\"' ) || ( nCharCode == '\'' ) ||
                            ( nCharCode == '_' )  ) )
                    {
                        aCurSel = pImpEditEngine->AutoCorrect( aCurSel, nCharCode, !pEditView->IsInsertMode() );
                    }
                    else
                    {
                        aCurSel = pImpEditEngine->InsertText( (const EditSelection&)aCurSel, nCharCode, !pEditView->IsInsertMode() );
                    }
                    // AutoComplete ???
                    if ( pImpEditEngine->GetStatus().DoAutoComplete() && ( nCharCode != ' ' ) )
                    {
                        // Aber nur wenn Wort-Ende...
                        sal_uInt16 nIndex = aCurSel.Max().GetIndex();
                        if ( ( nIndex >= aCurSel.Max().GetNode()->Len() ) ||
                             ( pImpEditEngine->aWordDelimiters.Search( aCurSel.Max().GetNode()->GetChar( nIndex ) ) != STRING_NOTFOUND ) )
                        {
                            EditPaM aStart( pImpEditEngine->WordLeft( aCurSel.Max() ) );
                            String aWord = pImpEditEngine->GetSelected( EditSelection( aStart, aCurSel.Max() ) );
                            if ( aWord.Len() >= 3 )
                            {
                                String aComplete;

                                International aInt = International( pImpEditEngine->GetLanguage( EditPaM( aStart.GetNode(), aStart.GetIndex()+1 ) ) );
                                for( int n = MONDAY; n <= SUNDAY; ++n )
                                {
                                    const String& rDay = aInt.GetDayText( (DayOfWeek)n );
                                    // MT: Auf International umstellen, wenn Compare mit Laengenangabe moeglich!
                                    if( aWord.CompareIgnoreCaseToAscii( rDay, aWord.Len() ) == COMPARE_EQUAL )
                                    {
                                        aComplete = rDay;
                                        break;
                                    }
                                }

                                if ( !aComplete.Len() )
                                {
                                    for( int n = 1; n <= 12; ++n )
                                    {
                                        const String& rMon = aInt.GetMonthText( n );
                                        // MT: Auf International umstellen, wenn Compare mit Laengenangabe moeglich!
                                        if( aWord.CompareIgnoreCaseToAscii( rMon, aWord.Len() ) == COMPARE_EQUAL )
                                        {
                                            aComplete = rMon;
                                            break;
                                        }
                                    }
                                }

                                if( aComplete.Len() && ( ( aWord.Len() + 1 ) < aComplete.Len() ) )
                                {
                                    pImpEditEngine->SetAutoCompleteText( aComplete, sal_False );
                                    Point aPos = pImpEditEngine->PaMtoEditCursor( aCurSel.Max() ).TopLeft();
                                    aPos = pEditView->pImpEditView->GetWindowPos( aPos );
                                    aPos = pEditView->pImpEditView->GetWindow()->LogicToPixel( aPos );
                                    aPos = pEditView->GetWindow()->OutputToScreenPixel( aPos );
                                    aPos.Y() -= 3;
                                    Help::ShowQuickHelp( pEditView->GetWindow(), Rectangle( aPos, Size( 1, 1 ) ), aComplete, QUICKHELP_BOTTOM|QUICKHELP_LEFT );
                                }
                            }
                        }
                    }
                    bModified = sal_True;
                }
                else
                    bDone = sal_False;
            }
        }
    }

    pEditView->pImpEditView->SetEditSelection( aCurSel );
    pImpEditEngine->UpdateSelections();

    if ( ( !IsVertical() && ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) ) ||
         ( IsVertical() && ( nCode != KEY_LEFT ) && ( nCode != KEY_RIGHT ) ))
        pEditView->pImpEditView->nTravelXPos = TRAVEL_X_DONTKNOW;

    if ( bModified )
    {
        DBG_ASSERT( !bReadOnly, "ReadOnly but modified???" );
        // Idle-Formatter nur, wenn AnyInput.
        if ( bAllowIdle && pImpEditEngine->GetStatus().UseIdleFormatter()
                && Application::AnyInput( INPUT_KEYBOARD) )
            pImpEditEngine->IdleFormatAndUpdate( pEditView );
        else
            pImpEditEngine->FormatAndUpdate( pEditView );
    }
    else if ( bMoved )
    {
        sal_Bool bGotoCursor = pEditView->pImpEditView->DoAutoScroll();
        pEditView->pImpEditView->ShowCursor( bGotoCursor, sal_True, bEndKey );
        pImpEditEngine->CallStatusHdl();
    }

#endif

    return bDone;
}

sal_uInt32 EditEngine::GetTextHeight() const
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nHeight = !IsVertical() ? pImpEditEngine->GetTextHeight() : pImpEditEngine->CalcTextWidth();
    return nHeight;
}

sal_uInt32 EditEngine::CalcTextWidth()
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    sal_uInt32 nWidth = !IsVertical() ? pImpEditEngine->CalcTextWidth() : pImpEditEngine->GetTextHeight();
     return nWidth;
}

void EditEngine::SetUpdateMode( sal_Bool bUpdate )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetUpdateMode( bUpdate );
    if ( pImpEditEngine->pActiveView )
        pImpEditEngine->pActiveView->ShowCursor( sal_False, sal_False );
}

sal_Bool EditEngine::GetUpdateMode() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetUpdateMode();
}

void EditEngine::Clear()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->Clear();
}

void EditEngine::SetText( const XubString& rText )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetText( rText );
    if ( rText.Len() )
        pImpEditEngine->FormatAndUpdate();
}

sal_uInt32 EditEngine::Read( SvStream& rInput, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs /* = NULL */ )
{
    DBG_CHKTHIS( EditEngine, 0 );
    sal_Bool bUndoEnabled = pImpEditEngine->IsUndoEnabled();
    pImpEditEngine->EnableUndo( sal_False );
    pImpEditEngine->SetText( XubString() );
    EditPaM aPaM( pImpEditEngine->GetEditDoc().GetStartPaM() );
    pImpEditEngine->Read( rInput, eFormat, EditSelection( aPaM, aPaM ), pHTTPHeaderAttrs );
    pImpEditEngine->EnableUndo( bUndoEnabled );
    return rInput.GetError();
}

sal_uInt32 EditEngine::Write( SvStream& rOutput, EETextFormat eFormat )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditPaM aStartPaM( pImpEditEngine->GetEditDoc().GetStartPaM() );
    EditPaM aEndPaM( pImpEditEngine->GetEditDoc().GetEndPaM() );
    pImpEditEngine->Write( rOutput, eFormat, EditSelection( aStartPaM, aEndPaM ) );
    return rOutput.GetError();
}

EditTextObject* EditEngine::CreateTextObject()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->CreateTextObject();
}

EditTextObject* EditEngine::CreateTextObject( const ESelection& rESelection )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->CreateSel( rESelection ) );
    return pImpEditEngine->CreateTextObject( aSel );
}

void EditEngine::SetText( const EditTextObject& rTextObject )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetText( rTextObject );
    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::ShowParagraph( sal_uInt16 nParagraph, sal_Bool bShow )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->ShowParagraph( nParagraph, bShow );
}

sal_Bool EditEngine::IsParagraphVisible( sal_uInt16 nParagraph )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsParagraphVisible( nParagraph );
}

void EditEngine::SetStatusEventHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetStatusEventHdl( rLink );
}

Link EditEngine::GetStatusEventHdl() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStatusEventHdl();
}

void EditEngine::SetImportHdl( const Link& rLink )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->aImportHdl = rLink;
}

Link EditEngine::GetImportHdl() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aImportHdl;
}

EditTextObject* EditEngine::CreateTextObject( sal_uInt16 nPara, sal_uInt16 nParas )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( nPara < pImpEditEngine->GetEditDoc().Count(), "CreateTextObject: Startpara out of Range" );
    DBG_ASSERT( nPara+nParas-1 < pImpEditEngine->GetEditDoc().Count(), "CreateTextObject: Endpara out of Range" );

    ContentNode* pStartNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
    ContentNode* pEndNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara+nParas-1 );
    DBG_ASSERT( pStartNode, "Start-Absatz existiert nicht: CreateTextObject" );
    DBG_ASSERT( pEndNode, "End-Absatz existiert nicht: CreateTextObject" );

    if ( pStartNode && pEndNode )
    {
        EditSelection aTmpSel;
        aTmpSel.Min() = EditPaM( pStartNode, 0 );
        aTmpSel.Max() = EditPaM( pEndNode, pEndNode->Len() );
        return pImpEditEngine->CreateTextObject( aTmpSel );
    }
    return 0;
}

void EditEngine::RemoveParagraph( sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( pImpEditEngine->GetEditDoc().Count() > 1, "Der erste Absatz darf nicht geloescht werden!" );
    if( pImpEditEngine->GetEditDoc().Count() <= 1 )
        return;

    ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
    ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nPara );
    DBG_ASSERT( pPortion && pNode, "Absatz nicht gefunden: RemoveParagraph" );
    if ( pNode && pPortion )
    {
        // Keine Undokappselung noetig.
        pImpEditEngine->ImpRemoveParagraph( nPara );
        pImpEditEngine->InvalidateFromParagraph( nPara );
        pImpEditEngine->UpdateSelections();
        pImpEditEngine->FormatAndUpdate();
    }
}

sal_uInt16 EditEngine::GetTextLen( sal_uInt16 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
    DBG_ASSERT( pNode, "Absatz nicht gefunden: GetTextLen" );
    if ( pNode )
        return pNode->Len();
    return 0;
}

XubString EditEngine::GetText( sal_uInt16 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    XubString aStr;
    if ( nPara < pImpEditEngine->GetEditDoc().Count() )
        aStr = pImpEditEngine->GetEditDoc().GetParaAsString( nPara );
    return aStr;
}

void EditEngine::ClearModifyFlag()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetModifyFlag( sal_False );
}

void EditEngine::SetModified()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetModifyFlag( sal_True );
}

sal_Bool EditEngine::IsModified() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->IsModified();
}

sal_Bool EditEngine::IsInSelectionMode() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return ( pImpEditEngine->IsInSelectionMode() ||
                pImpEditEngine->GetSelEngine().IsInSelection() );
}

void EditEngine::StopSelectionMode()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->StopSelectionMode();
}

void EditEngine::InsertParagraph( sal_uInt16 nPara, const EditTextObject& rTxtObj )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( nPara > GetParagraphCount() )
    {
        DBG_ASSERTWARNING( nPara == USHRT_MAX, "AbsatzNr zu Groá, aber nicht LIST_APPEND! " );
        nPara = GetParagraphCount();
    }

    pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );

    // Keine Undoklammerung noetig.
    EditPaM aPaM( pImpEditEngine->InsertParagraph( nPara ) );
    // Bei einem InsertParagraph von aussen sollen keine Harten
    // Attribute uebernommen werden !
    pImpEditEngine->RemoveCharAttribs( nPara );
    pImpEditEngine->InsertText( rTxtObj, EditSelection( aPaM, aPaM ) );

    pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );

    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::InsertParagraph( sal_uInt16 nPara, const XubString& rTxt )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( nPara > GetParagraphCount() )
    {
        DBG_ASSERTWARNING( nPara == USHRT_MAX, "AbsatzNr zu Groá, aber nicht LIST_APPEND! " );
        nPara = GetParagraphCount();
    }

    pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM( pImpEditEngine->InsertParagraph( nPara ) );
    // Bei einem InsertParagraph von aussen sollen keine Harten
    // Attribute uebernommen werden !
    pImpEditEngine->RemoveCharAttribs( nPara );
    pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
    pImpEditEngine->ImpInsertText( EditSelection( aPaM, aPaM ), rTxt );
    pImpEditEngine->FormatAndUpdate();
}

void EditEngine::SetText( sal_uInt16 nPara, const EditTextObject& rTxtObj )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection* pSel = pImpEditEngine->SelectParagraph( nPara );
    if ( pSel )
    {
        pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
        pImpEditEngine->InsertText( rTxtObj, *pSel );
        pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
        pImpEditEngine->FormatAndUpdate();
        delete pSel;
    }
}

void EditEngine::SetText( sal_uInt16 nPara, const XubString& rTxt )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection* pSel = pImpEditEngine->SelectParagraph( nPara );
    if ( pSel )
    {
        pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
        pImpEditEngine->ImpInsertText( *pSel, rTxt );
        pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
        pImpEditEngine->FormatAndUpdate();
        delete pSel;
    }
}

void EditEngine::SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet )
{
    DBG_CHKTHIS( EditEngine, 0 );
    // Keine Undoklammerung noetig.
    pImpEditEngine->SetParaAttribs( nPara, rSet );
    pImpEditEngine->FormatAndUpdate();
}

const SfxItemSet& EditEngine::GetParaAttribs( sal_uInt16 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetParaAttribs( nPara );
}

sal_Bool EditEngine::HasParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->HasParaAttrib( nPara, nWhich );
}

const SfxPoolItem& EditEngine::GetParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditEngine, 0 );
return pImpEditEngine->GetParaAttrib( nPara, nWhich );
}

void EditEngine::GetCharAttribs( sal_uInt16 nPara, EECharAttribArray& rLst ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->GetCharAttribs( nPara, rLst );
}

SfxItemSet EditEngine::GetAttribs( const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );
    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
    return pImpEditEngine->GetAttribs( aSel );
}

Font EditEngine::GetStandardFont( sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return GetStandardSvxFont( nPara );
}

SvxFont EditEngine::GetStandardSvxFont( sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
    return pNode->GetCharAttribs().GetDefFont();
}

void EditEngine::StripPortions()
{
    DBG_CHKTHIS( EditEngine, 0 );
    VirtualDevice aTmpDev;
    Rectangle aBigRec( Point( 0, 0 ), Size( 0x7FFFFFFF, 0x7FFFFFFF ) );
    if ( IsVertical() )
    {
        aBigRec.Right() = 0;
        aBigRec.Left() = -0x7FFFFFFF;
    }
    pImpEditEngine->Paint( &aTmpDev, aBigRec, Point(), sal_True );
}

void EditEngine::GetPortions( sal_uInt16 nPara, SvUShorts& rList )
{
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatFullDoc();

    ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nPara );
    if ( pParaPortion )
    {
        sal_uInt16 nEnd = 0;
        sal_uInt16 nTextPortions = pParaPortion->GetTextPortions().Count();
        for ( sal_uInt16 n = 0; n < nTextPortions; n++ )
        {
            nEnd += pParaPortion->GetTextPortions()[n]->GetLen();
            rList.Insert( nEnd, rList.Count() );
        }
    }
}

void EditEngine::SetFlatMode( sal_Bool bFlat)
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetFlatMode( bFlat );
}

sal_Bool EditEngine::IsFlatMode() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return !( pImpEditEngine->aStatus.UseCharAttribs() );
}

void EditEngine::SetControlWord( sal_uInt32 nWord )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( nWord != pImpEditEngine->aStatus.GetControlWord() )
    {
        sal_uInt32 nPrev = pImpEditEngine->aStatus.GetControlWord();
        pImpEditEngine->aStatus.GetControlWord() = nWord;

        sal_uInt32 nChanges = nPrev ^ nWord;
        if ( pImpEditEngine->IsFormatted() )
        {
            // ggf. neu formatieren:
            if ( ( nChanges & EE_CNTRL_USECHARATTRIBS ) ||
                 ( nChanges & EE_CNTRL_USEPARAATTRIBS ) ||
                 ( nChanges & EE_CNTRL_ONECHARPERLINE ) ||
                 ( nChanges & EE_CNTRL_STRETCHING ) ||
                 ( nChanges & EE_CNTRL_OUTLINER ) ||
                 ( nChanges & EE_CNTRL_NOCOLORS ) ||
                 ( nChanges & EE_CNTRL_OUTLINER2 ) )
            {
                if ( ( nChanges & EE_CNTRL_USECHARATTRIBS ) ||
                     ( nChanges & EE_CNTRL_USEPARAATTRIBS ) )
                {
                    sal_Bool bUseCharAttribs = ( nWord & EE_CNTRL_USECHARATTRIBS ) ? sal_True : sal_False;
                    pImpEditEngine->GetEditDoc().CreateDefFont( bUseCharAttribs );
                }

                pImpEditEngine->FormatFullDoc();
                pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );
            }
        }

#ifndef SVX_LIGHT
        sal_Bool bSpellingChanged = nChanges & EE_CNTRL_ONLINESPELLING ? sal_True : sal_False;
        sal_Bool bRedLinesChanged = nChanges & EE_CNTRL_NOREDLINES ? sal_True : sal_False;

        if ( bSpellingChanged || bRedLinesChanged )
        {
            pImpEditEngine->StopOnlineSpellTimer();
            if ( bSpellingChanged && ( nWord & EE_CNTRL_ONLINESPELLING ) )
            {
                // WrongListen anlegen, Timer starten...
                sal_uInt16 nNodes = pImpEditEngine->GetEditDoc().Count();
                for ( sal_uInt16 n = 0; n < nNodes; n++ )
                {
                    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( n );
                    pNode->CreateWrongList();
                }
                pImpEditEngine->StartOnlineSpellTimer();
            }
            else
            {
                long nY = 0;
                sal_uInt16 nNodes = pImpEditEngine->GetEditDoc().Count();
                for ( sal_uInt16 n = 0; n < nNodes; n++ )
                {
                    ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( n );
                    ParaPortion* pPortion = pImpEditEngine->GetParaPortions().GetObject( n );
                    sal_Bool bWrongs = ( bSpellingChanged || ( nWord & EE_CNTRL_ONLINESPELLING ) ) ? pNode->GetWrongList()->HasWrongs() : sal_False;
                    if ( bSpellingChanged )         // Also aus
                        pNode->DestroyWrongList();  // => vorm Paint weghaun.
                    if ( bWrongs )
                    {
                        pImpEditEngine->aInvalidRec.Left() = 0;
                        pImpEditEngine->aInvalidRec.Right() = pImpEditEngine->GetPaperSize().Width();
                        pImpEditEngine->aInvalidRec.Top() = nY+1;
                        pImpEditEngine->aInvalidRec.Bottom() = nY+pPortion->GetHeight()-1;
                        pImpEditEngine->UpdateViews( pImpEditEngine->pActiveView );
                    }
                    nY += pPortion->GetHeight();
                }
            }
        }
#endif // !SVX_LIGHT
    }
}

sal_uInt32 EditEngine::GetControlWord() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aStatus.GetControlWord();
}

long EditEngine::GetFirstLineStartX( sal_uInt16 nParagraph )
{
    DBG_CHKTHIS( EditEngine, 0 );

    long nX = 0;
    ParaPortion* pPPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nParagraph );
    if ( pPPortion )
    {
        DBG_ASSERT( pImpEditEngine->IsFormatted(), "GetDocPosTopLeft: Doc nicht formatiert" );
        if ( !pImpEditEngine->IsFormatted() )
            pImpEditEngine->FormatDoc();
        EditLine* pFirstLine = pPPortion->GetLines()[0];
        nX = pFirstLine->GetStartPosX();
    }
    return nX;
}

Point EditEngine::GetDocPos( const Point& rPaperPos ) const
{
    Point aDocPos( rPaperPos );
    if ( IsVertical() )
    {
        aDocPos.X() = rPaperPos.Y();
        aDocPos.Y() = GetPaperSize().Width() - rPaperPos.X();
    }
    return aDocPos;
}


Point EditEngine::GetDocPosTopLeft( sal_uInt16 nParagraph )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( pImpEditEngine->IsFormatted(), "GetDocPosTopLeft: Doc nicht formatiert" );
    ParaPortion* pPPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nParagraph );
    DBG_ASSERT( pPPortion, "Absatz nicht gefunden: GetWindowPosTopLeft" );
    Point aPoint;
    if ( pPPortion )
    {
        // Falls jemand mit einer leeren Engine ein GetLineHeight() macht.
        if ( !pImpEditEngine->IsFormatted() )
            pImpEditEngine->FormatDoc();
        if ( pPPortion->GetLines().Count() )
        {
            // So richtiger, falls grosses Bullet.
            EditLine* pFirstLine = pPPortion->GetLines()[0];
            aPoint.X() = pFirstLine->GetStartPosX();
        }
        else
        {
            const SvxLRSpaceItem& rLRItem = pImpEditEngine->GetLRSpaceItem( pPPortion->GetNode() );
            aPoint.X() = pImpEditEngine->GetXValue( (short)(rLRItem.GetTxtLeft() + rLRItem.GetTxtFirstLineOfst()) );
        }
        aPoint.Y() = pImpEditEngine->GetParaPortions().GetYOffset( pPPortion );
    }
    return aPoint;
}

sal_Bool EditEngine::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
{
    DBG_CHKTHIS( EditEngine, 0 );

    sal_Bool bTextPos = sal_False;

    Point aDocPos = GetDocPos( rPaperPos );

    if ( ( aDocPos.Y() > 0  ) && ( aDocPos.Y() < (long)GetTextHeight() ) )
    {
        EditPaM aPaM = pImpEditEngine->GetPaM( aDocPos, sal_False );
        if ( aPaM.GetNode() )
        {
            ParaPortion* pParaPortion = pImpEditEngine->FindParaPortion( aPaM.GetNode() );
            DBG_ASSERT( pParaPortion, "ParaPortion?" );
            sal_uInt16 nLine = pParaPortion->GetLineNumber( aPaM.GetIndex() );
            EditLine* pLine = pParaPortion->GetLines().GetObject( nLine );
            if ( ( aDocPos.X() >= pLine->GetStartPosX() - nBorder ) &&
                 ( aDocPos.X() <= pLine->GetStartPosX() +
                         pImpEditEngine->CalcLineWidth( pParaPortion, pLine ) + nBorder ) )
            {
                 bTextPos = sal_True;
            }
        }
    }
    return bTextPos;
}

void EditEngine::SetEditTextObjectPool( SfxItemPool* pPool )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetEditTextObjectPool( pPool );
}

SfxItemPool* EditEngine::GetEditTextObjectPool() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetEditTextObjectPool();
}

void EditEngine::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->SetAttribs( aSel, rSet );
}

void EditEngine::QuickMarkInvalid( const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( rSel.nStartPara < pImpEditEngine->GetEditDoc().Count(), "MarkInvalid: Start out of Range!" );
    DBG_ASSERT( rSel.nEndPara < pImpEditEngine->GetEditDoc().Count(), "MarkInvalid: End out of Range!" );
    for ( sal_uInt16 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nPara );
        if ( pPortion )
            pPortion->MarkSelectionInvalid( 0, pPortion->GetNode()->Len() );
    }
}

void EditEngine::QuickInsertText( const XubString& rText, const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpInsertText( aSel, rText );
}

void EditEngine::QuickDelete( const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpDeleteSelection( aSel );
}

void EditEngine::QuickMarkToBeRepainted( sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );
    ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nPara );
    if ( pPortion )
        pPortion->SetMustRepaint( sal_True );
}

void EditEngine::QuickInsertLineBreak( const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->InsertLineBreak( aSel );
}

void EditEngine::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    pImpEditEngine->ImpInsertFeature( aSel, rFld );
}

void EditEngine::QuickFormatDoc( sal_Bool bFull )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( bFull )
        pImpEditEngine->FormatFullDoc();
    else
        pImpEditEngine->FormatDoc();
    pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );
}

void EditEngine::QuickRemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->RemoveCharAttribs( nPara, nWhich );
}

void EditEngine::SetStyleSheet( sal_uInt16 nPara, SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetStyleSheet( nPara, pStyle );
}

SfxStyleSheet* EditEngine::GetStyleSheet( sal_uInt16 nPara ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStyleSheet( nPara );
}

void EditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetStyleSheetPool( pSPool );
}

SfxStyleSheetPool* EditEngine::GetStyleSheetPool()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStyleSheetPool();
}

void EditEngine::SetWordDelimiters( const XubString& rDelimiters )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->aWordDelimiters = rDelimiters;
    if ( pImpEditEngine->aWordDelimiters.Search( CH_FEATURE ) == STRING_NOTFOUND )
        pImpEditEngine->aWordDelimiters.Insert( CH_FEATURE );
}

XubString EditEngine::GetWordDelimiters() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aWordDelimiters;
}

void EditEngine::SetGroupChars( const XubString& rChars )
{
    DBG_CHKTHIS( EditEngine, 0 );
    DBG_ASSERT( ( rChars.Len() % 2 ) == 0, "SetGroupChars: Ungerade Anzahl!" );
    pImpEditEngine->aGroupChars = rChars;
}

XubString EditEngine::GetGroupChars() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->aGroupChars;
}

void EditEngine::EnablePasteSpecial( sal_Bool bEnable )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( bEnable )
        pImpEditEngine->GetStatus().TurnOnFlags( EE_CNTRL_PASTESPECIAL );
    else
        pImpEditEngine->GetStatus().TurnOffFlags( EE_CNTRL_PASTESPECIAL );
}

sal_Bool EditEngine::IsPasteSpecialEnabled() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStatus().AllowPasteSpecial();
}

void EditEngine::EnableIdleFormatter( sal_Bool bEnable )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( bEnable )
        pImpEditEngine->GetStatus().TurnOnFlags( EE_CNTRL_DOIDLEFORMAT );
    else
        pImpEditEngine->GetStatus().TurnOffFlags( EE_CNTRL_DOIDLEFORMAT);
}

sal_Bool EditEngine::IsIdleFormatterEnabled() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetStatus().UseIdleFormatter();
}

void EditEngine::EraseVirtualDevice()
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->EraseVirtualDevice();
}

void EditEngine::SetSpeller( Reference< XSpellChecker1 >  &xSpeller )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetSpeller( xSpeller );
}
Reference< XSpellChecker1 >  EditEngine::GetSpeller()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetSpeller();
}
Reference< XHyphenator >  EditEngine::GetHyphenator() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetHyphenator();
}

void EditEngine::SetHyphenator( Reference< XHyphenator > & xHyph )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetHyphenator( xHyph );
}

void EditEngine::SetDefaultLanguage( LanguageType eLang )
{
#if SUPD >= 630
//  DBG_ERROR( "DefaultLanguage not longer supported" );
#endif
}

LanguageType EditEngine::GetDefaultLanguage() const
{
#if SUPD >= 630
//  DBG_ERROR( "DefaultLanguage not longer supported" );
#endif
    return pImpEditEngine->GetLanguage( EditPaM( pImpEditEngine->GetEditDoc().SaveGetObject( 0 ), 0 ) );
}

sal_Bool __EXPORT EditEngine::SpellNextDocument()
{
    DBG_CHKTHIS( EditEngine, 0 );
    return sal_False;
}

EESpellState EditEngine::HasSpellErrors( LanguageType eLang )
{
#if SUPD >= 630
//  DBG_ERROR( "DefaultLanguage not longer supported" );
#endif
    return HasSpellErrors();
}

EESpellState EditEngine::HasSpellErrors()
{
#ifdef SVX_LIGHT
    return EE_SPELL_NOSPELLER;
#else
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->GetSpeller().is()  )
        return EE_SPELL_NOSPELLER;

    return pImpEditEngine->HasSpellErrors();
#endif
}

sal_Bool EditEngine::HasText( const SvxSearchItem& rSearchItem )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->HasText( rSearchItem );
}

void EditEngine::SetGlobalCharStretching( sal_uInt16 nX, sal_uInt16 nY )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetCharStretching( nX, nY );
}

void EditEngine::GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->GetCharStretching( rX, rY );
}

void EditEngine::DoStretchChars( sal_uInt16 nX, sal_uInt16 nY )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->DoStretchChars( nX, nY );
}

void EditEngine::SetBigTextObjectStart( sal_uInt16 nStartAtPortionCount )
{
    DBG_CHKTHIS( EditEngine, 0 );
    pImpEditEngine->SetBigTextObjectStart( nStartAtPortionCount );
}

sal_uInt16 EditEngine::GetBigTextObjectStart() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    return pImpEditEngine->GetBigTextObjectStart();
}

sal_Bool EditEngine::ShouldCreateBigTextObject() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    sal_uInt16 nTextPortions = 0;
    sal_uInt16 nParas = pImpEditEngine->GetEditDoc().Count();
    for ( sal_uInt16 nPara = 0; nPara < nParas; nPara++  )
    {
        ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
        nTextPortions += pParaPortion->GetTextPortions().Count();
    }
    return ( nTextPortions >= pImpEditEngine->GetBigTextObjectStart() ) ? sal_True : sal_False;
}

sal_Bool EditEngine::UpdateFields()
{
    DBG_CHKTHIS( EditEngine, 0 );
    sal_Bool bChanges = pImpEditEngine->UpdateFields();
    if ( bChanges )
        pImpEditEngine->FormatAndUpdate();
    return bChanges;
}

void EditEngine::RemoveFields( sal_Bool bKeepFieldText, TypeId aType )
{
    DBG_CHKTHIS( EditEngine, 0 );

    if ( bKeepFieldText )
        pImpEditEngine->UpdateFields();

    sal_uInt16 nParas = pImpEditEngine->GetEditDoc().Count();
    for ( sal_uInt16 nPara = 0; nPara < nParas; nPara++  )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( nPara );
        const CharAttribArray& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for ( sal_uInt16 nAttr = rAttrs.Count(); nAttr; )
        {
            const EditCharAttrib* pAttr = rAttrs[--nAttr];
            if ( pAttr->Which() == EE_FEATURE_FIELD )
            {
                const SvxFieldData* pFldData = ((const SvxFieldItem*)pAttr->GetItem())->GetField();
                if ( pFldData && ( !aType || ( pFldData->IsA( aType ) ) ) )
                {
                    DBG_ASSERT( pAttr->GetItem()->ISA( SvxFieldItem ), "Kein FeldItem..." );
                    EditSelection aSel( EditPaM( pNode, pAttr->GetStart() ), EditPaM( pNode, pAttr->GetEnd() ) );
                    String aFieldText = ((EditCharAttribField*)pAttr)->GetFieldValue();
                    pImpEditEngine->ImpInsertText( aSel, aFieldText );
                }
            }
        }
    }
}

sal_Bool EditEngine::CopyClipboard() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    const EditDoc& rDoc = pImpEditEngine->GetEditDoc();

    EditSelection aSel( rDoc.GetStartPaM(), rDoc.GetEndPaM() );
    if ( aSel.HasRange() )
    {
        pImpEditEngine->CopyData( aSel, EXCHANGE_CLIPBOARD );
        return sal_True;
    }
    return sal_False;
}

sal_Bool EditEngine::CopyClipboard( const ESelection& rSel ) const
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    if ( aSel.HasRange() )
    {
        pImpEditEngine->CopyData( aSel, EXCHANGE_CLIPBOARD );
        return sal_True;
    }
    return sal_False;
}

sal_Bool EditEngine::PasteClipboard()
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->HasData( EXCHANGE_CLIPBOARD ) )
        return sal_False;
    SetText( String() );
    EditPaM aPaM = pImpEditEngine->GetEditDoc().GetStartPaM();
    pImpEditEngine->PasteData( aPaM, EXCHANGE_CLIPBOARD, sal_True );
    return sal_True;
}

sal_Bool EditEngine::CopyDragServer() const
{
    DBG_CHKTHIS( EditEngine, 0 );
    const EditDoc& rDoc = pImpEditEngine->GetEditDoc();

    EditSelection aSel( rDoc.GetStartPaM(), rDoc.GetEndPaM() );
    if ( aSel.HasRange() )
    {
        pImpEditEngine->CopyData( aSel, EXCHANGE_DRAGSERVER );
        return sal_True;
    }
    return sal_False;
}

sal_Bool EditEngine::CopyDragServer( const ESelection& rSel ) const
{
    DBG_CHKTHIS( EditEngine, 0 );

    EditSelection aSel( pImpEditEngine->
        ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );

    if ( aSel.HasRange() )
    {
        pImpEditEngine->CopyData( aSel, EXCHANGE_DRAGSERVER );
        return sal_True;
    }
    return sal_False;
}

sal_Bool EditEngine::PasteDragServer( sal_uInt16 nItem )
{
    DBG_CHKTHIS( EditEngine, 0 );
    if ( !pImpEditEngine->HasData( EXCHANGE_DRAGSERVER ) )
        return sal_False;
    SetText( String() );
    EditPaM aPaM = pImpEditEngine->GetEditDoc().GetStartPaM();
    pImpEditEngine->PasteData( aPaM, EXCHANGE_DRAGSERVER, sal_True );
    return sal_True;
}

sal_Bool EditEngine::HasOnlineSpellErrors() const
{
#ifndef SVX_LIGHT
    DBG_CHKTHIS( EditEngine, 0 );
    sal_uInt16 nNodes = pImpEditEngine->GetEditDoc().Count();
    for ( sal_uInt16 n = 0; n < nNodes; n++ )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().GetObject( n );
        if ( pNode->GetWrongList() && pNode->GetWrongList()->Count() )
            return sal_True;
    }
#endif // !SVX_LIGHT
    return sal_False;
}

void EditEngine::CompleteOnlineSpelling()
{
#ifndef SVX_LIGHT
    DBG_CHKTHIS( EditEngine, 0 );
    if ( pImpEditEngine->GetStatus().DoOnlineSpelling() )
    {
        pImpEditEngine->StopOnlineSpellTimer();
        pImpEditEngine->DoOnlineSpelling( 0, sal_True, sal_False );
    }
#endif SVX_LIGHT
}

USHORT EditEngine::FindParagraph( long nDocPosY )
{
    return pImpEditEngine->GetParaPortions().FindParagraph( nDocPosY );
}

ParagraphInfos EditEngine::GetParagraphInfos( sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditEngine, 0 );

    // Funktioniert nur, wenn nicht bereits in der Formatierung...
    if ( !pImpEditEngine->IsFormatted() )
        pImpEditEngine->FormatDoc();

    ParagraphInfos aInfos;
    aInfos.bValid = pImpEditEngine->IsFormatted();
    if ( pImpEditEngine->IsFormatted() )
    {
        ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
        EditLine* pLine = pParaPortion ? pParaPortion->GetLines().GetObject( 0 ) : NULL;
        DBG_ASSERT( pParaPortion && pLine, "GetParagraphInfos - Paragraph out of range" );
        if ( pParaPortion && pLine )
        {
            aInfos.nParaHeight = (USHORT)pParaPortion->GetHeight();
            aInfos.nLines = pParaPortion->GetLines().Count();
            aInfos.nFirstLineStartX = pLine->GetStartPosX();
            aInfos.nFirstLineOffset = pParaPortion->GetFirstLineOffset();
            aInfos.nFirstLineHeight = pLine->GetHeight();
            aInfos.nFirstLineTextHeight = pLine->GetTxtHeight();
            aInfos.nFirstLineMaxAscent = pLine->GetMaxAscent();
        }
    }
    return aInfos;
}

// =====================================================================
// ======================   Virtuelle Methoden   =======================
// =====================================================================
void __EXPORT EditEngine::DrawingText( const Point&, const XubString&, const long*, const SvxFont&, sal_uInt16 nPara, sal_uInt16 nIndex )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void __EXPORT EditEngine::PaintingFirstLine( sal_uInt16, const Point&, long, const Point&, short, OutputDevice* )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void __EXPORT EditEngine::ParagraphInserted( sal_uInt16 )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void __EXPORT EditEngine::ParagraphDeleted( sal_uInt16 )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

sal_Bool __EXPORT EditEngine::FormattingParagraph( sal_uInt16 )
{
    // return sal_True, wenn die Attribute geaendert wurden...
    DBG_CHKTHIS( EditEngine, 0 );
    return sal_False;
}

void __EXPORT EditEngine::ParaAttribsChanged( sal_uInt16 /* nParagraph */ )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void __EXPORT EditEngine::StyleSheetChanged( SfxStyleSheet* /* pStyle */ )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void __EXPORT EditEngine::ParagraphHeightChanged( sal_uInt16 )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

XubString __EXPORT EditEngine::GetUndoComment( sal_uInt16 nId ) const
{
    DBG_CHKTHIS( EditEngine, 0 );
    XubString aComment;
    switch ( nId )
    {
        case EDITUNDO_REMOVECHARS:
        case EDITUNDO_CONNECTPARAS:
        case EDITUNDO_REMOVEFEATURE:
        case EDITUNDO_DELCONTENT:
        case EDITUNDO_DELETE:
        case EDITUNDO_CUT:
            aComment = XubString( EditResId( RID_EDITUNDO_DEL ) );
        break;
        case EDITUNDO_MOVEPARAGRAPHS:
        case EDITUNDO_MOVEPARAS:
        case EDITUNDO_DRAGANDDROP:
            aComment = XubString( EditResId( RID_EDITUNDO_MOVE ) );
        break;
        case EDITUNDO_INSERTFEATURE:
        case EDITUNDO_SPLITPARA:
        case EDITUNDO_INSERTCHARS:
        case EDITUNDO_PASTE:
        case EDITUNDO_INSERT:
        case EDITUNDO_READ:
            aComment = XubString( EditResId( RID_EDITUNDO_INSERT ) );
        break;
        case EDITUNDO_SRCHANDREPL:
        case EDITUNDO_REPLACEALL:
            aComment = XubString( EditResId( RID_EDITUNDO_REPLACE ) );
        break;
        case EDITUNDO_ATTRIBS:
        case EDITUNDO_PARAATTRIBS:
        case EDITUNDO_STRETCH:
            aComment = XubString( EditResId( RID_EDITUNDO_SETATTRIBS ) );
        break;
        case EDITUNDO_RESETATTRIBS:
            aComment = XubString( EditResId( RID_EDITUNDO_RESETATTRIBS ) );
        break;
        case EDITUNDO_STYLESHEET:
            aComment = XubString( EditResId( RID_EDITUNDO_SETSTYLE ) );
        break;
        case EDITUNDO_TRANSLITERATE:
            aComment = XubString( EditResId( RID_EDITUNDO_TRANSLITERATE ) );
        break;
        case EDITUNDO_INDENTBLOCK:
        case EDITUNDO_UNINDENTBLOCK:
            aComment = XubString( EditResId( RID_EDITUNDO_INDENT ) );
        break;
    }
    return aComment;
}

Rectangle EditEngine::GetBulletArea( sal_uInt16 nPara )
{
    return Rectangle( Point(), Point() );
}

XubString __EXPORT EditEngine::CalcFieldValue( const SvxFieldItem& rField, sal_uInt16, sal_uInt16, Color*&, Color*& )
{
    DBG_CHKTHIS( EditEngine, 0 );
    return ' ';
}

void __EXPORT EditEngine::FieldClicked( const SvxFieldItem& rField, sal_uInt16, sal_uInt16 )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

void __EXPORT EditEngine::FieldSelected( const SvxFieldItem& rField, sal_uInt16, sal_uInt16 )
{
    DBG_CHKTHIS( EditEngine, 0 );
}

// =====================================================================
// ======================   Statische Methoden   =======================
// =====================================================================
SfxItemPool* EditEngine::CreatePool( sal_Bool bPersistentRefCounts )
{
    SfxItemPool* pPool = new EditEngineItemPool( bPersistentRefCounts );
    return pPool;
}

sal_uInt32 EditEngine::RegisterClipboardFormatName()
{
    static sal_uInt32 nFormat = 0;
    if ( !nFormat )
        nFormat = Exchange::RegisterFormatName( String( RTL_CONSTASCII_USTRINGPARAM( "EditEngineFormat" ) ) );
    return nFormat;
}

void EditEngine::SetGetAutoCorrectHdl( const Link& rHdl )
{
    EE_DLL()->GetGlobalData()->SetGetAutoCorrectHdl( rHdl );
}

sal_uInt16 EditEngine::GetAvailableSearchOptions()
{
#ifdef SVX_LIGHT
    return 0;
#else
    return SEARCH_OPTIONS_SEARCH | SEARCH_OPTIONS_REPLACE |
            SEARCH_OPTIONS_REPLACE_ALL | SEARCH_OPTIONS_WHOLE_WORDS |
            SEARCH_OPTIONS_BACKWARDS | SEARCH_OPTIONS_REG_EXP |
            SEARCH_OPTIONS_EXACT | SEARCH_OPTIONS_SELECTION;
#endif
}

void EditEngine::SetFontInfoInItemSet( SfxItemSet& rSet, const Font& rFont )
{
    SvxFont aSvxFont( rFont );
    SetFontInfoInItemSet( rSet, aSvxFont );

}

void EditEngine::SetFontInfoInItemSet( SfxItemSet& rSet, const SvxFont& rFont )
{
    rSet.Put( SvxLanguageItem( rFont.GetLanguage(), EE_CHAR_LANGUAGE ) );
    rSet.Put( SvxFontItem( rFont.GetFamily(), rFont.GetName(), XubString(), rFont.GetPitch(), rFont.GetCharSet(), EE_CHAR_FONTINFO ) );
    rSet.Put( SvxFontHeightItem( rFont.GetSize().Height(), 100, EE_CHAR_FONTHEIGHT )  );
    rSet.Put( SvxFontWidthItem( (sal_uInt16)rFont.GetSize().Width(), 100, EE_CHAR_FONTWIDTH )  );
    rSet.Put( SvxShadowedItem( rFont.IsShadow(), EE_CHAR_SHADOW )  );
    rSet.Put( SvxEscapementItem( rFont.GetEscapement(), rFont.GetPropr(), EE_CHAR_ESCAPEMENT )  );
    rSet.Put( SvxWeightItem( rFont.GetWeight(), EE_CHAR_WEIGHT )  );
    rSet.Put( SvxColorItem( rFont.GetColor(), EE_CHAR_COLOR )  );
    rSet.Put( SvxUnderlineItem( rFont.GetUnderline(), EE_CHAR_UNDERLINE )  );
    rSet.Put( SvxCrossedOutItem( rFont.GetStrikeout(), EE_CHAR_STRIKEOUT )  );
    rSet.Put( SvxPostureItem( rFont.GetItalic(), EE_CHAR_ITALIC )  );
    rSet.Put( SvxContourItem( rFont.IsOutline(), EE_CHAR_OUTLINE )  );
    rSet.Put( SvxAutoKernItem( rFont.IsKerning(), EE_CHAR_PAIRKERNING ) );
    rSet.Put( SvxKerningItem( rFont.GetFixKerning(), EE_CHAR_KERNING ) );
    rSet.Put( SvxWordLineModeItem( rFont.IsWordLineMode(), EE_CHAR_WLM ) );
}

Font EditEngine::CreateFontFromItemSet( const SfxItemSet& rItemSet )
{
    return CreateSvxFontFromItemSet( rItemSet );
}

SvxFont EditEngine::CreateSvxFontFromItemSet( const SfxItemSet& rItemSet )
{
    SvxFont aFont;
    CreateFont( aFont, rItemSet );
    return aFont;
}

sal_Bool EditEngine::DoesKeyMoveCursor( const KeyEvent& rKeyEvent )
{
    sal_Bool bDoesMove = sal_False;

    switch ( rKeyEvent.GetKeyCode().GetCode() )
    {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        case KEY_HOME:
        case KEY_END:
        case KEY_PAGEUP:
        case KEY_PAGEDOWN:
        {
            if ( !rKeyEvent.GetKeyCode().IsMod2() )
                bDoesMove = sal_True;
        }
        break;
    }
    return bDoesMove;
}

sal_Bool EditEngine::DoesKeyChangeText( const KeyEvent& rKeyEvent )
{
    sal_Bool bDoesChange = sal_False;

    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_UNDO:
            case KEYFUNC_REDO:
            case KEYFUNC_CUT:
            case KEYFUNC_PASTE: bDoesChange = sal_True;
            break;
            default:    // wird dann evtl. unten bearbeitet.
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }
    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( rKeyEvent.GetKeyCode().GetCode() )
        {
            case KEY_DELETE:
            case KEY_BACKSPACE: bDoesChange = sal_True;
            break;
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    bDoesChange = sal_True;
            }
            break;
            default:
            {
                bDoesChange = IsSimpleCharInput( rKeyEvent );
            }
        }
    }
    return bDoesChange;
}

sal_Bool EditEngine::IsSimpleCharInput( const KeyEvent& rKeyEvent )
{
    if( EditEngine::IsPrintable( rKeyEvent.GetCharCode() ) &&
#ifndef MAC
        ( KEY_MOD2 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) ) &&
        ( KEY_MOD1 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) ) )
#else
        KEY_MOD1 != rKeyEvent.GetKeyCode().GetModifier() )
#endif
    {
        return sal_True;
    }
    return sal_False;
}

// Mal in den Outliner schieben...
void EditEngine::ImportBulletItem( SvxNumBulletItem& rNumBullet, sal_uInt16 nLevel,
                                    const SvxBulletItem* pOldBullet, const SvxLRSpaceItem* pOldLRSpace )
{
    if ( pOldBullet || pOldLRSpace )
    {
        // Numberformat dynamisch, weil Zuweisungsoperator nicht implementiert.

        // Altes NumBulletItem nur uebernehmen, wenn kein altes BulletItem
        const SvxNumberFormat* pFmt = ( !pOldBullet && ( rNumBullet.GetNumRule()->GetLevelCount() > nLevel ) ) ?
                                        rNumBullet.GetNumRule()->Get( nLevel ) : NULL;
        SvxNumberFormat* pNumberFormat = pFmt
                                            ? new SvxNumberFormat( *pFmt )
                                            : new SvxNumberFormat( SVX_NUM_NUMBER_NONE );
        if ( pOldBullet )
        {
            // Style
            SvxExtNumType eNumType;
            switch( pOldBullet->GetStyle() )
            {
                case BS_BMP:            eNumType = SVX_NUM_BITMAP;              break;
                case BS_BULLET:         eNumType = SVX_NUM_CHAR_SPECIAL;        break;
                case BS_ROMAN_BIG:      eNumType = SVX_NUM_ROMAN_UPPER;         break;
                case BS_ROMAN_SMALL:    eNumType = SVX_NUM_ROMAN_LOWER;         break;
                case BS_ABC_BIG:        eNumType = SVX_NUM_CHARS_UPPER_LETTER;  break;
                case BS_ABC_SMALL:      eNumType = SVX_NUM_CHARS_LOWER_LETTER;  break;
                case BS_123:            eNumType = SVX_NUM_ARABIC;              break;
                default:                eNumType = SVX_NUM_NUMBER_NONE;         break;
            }
#if SUPD > 622
            pNumberFormat->SetNumberingType( eNumType );
#else
            pNumberFormat->SetNumType( eNumType );
#endif

            // Justification
            SvxAdjust eAdjust;
            switch( pOldBullet->GetJustification() & (BJ_HRIGHT|BJ_HCENTER|BJ_HLEFT) )
            {
                case BJ_HRIGHT:     eAdjust = SVX_ADJUST_RIGHT;     break;
                case BJ_HCENTER:    eAdjust = SVX_ADJUST_CENTER;    break;
                default:            eAdjust = SVX_ADJUST_LEFT;      break;
            }
            pNumberFormat->SetNumAdjust(eAdjust);

            // Prefix/Suffix
            pNumberFormat->SetPrefix( pOldBullet->GetPrevText() );
            pNumberFormat->SetSuffix( pOldBullet->GetFollowText() );

            //Font
            if ( eNumType != SVX_NUM_BITMAP )
            {
                Font aTmpFont = pOldBullet->GetFont();
                pNumberFormat->SetBulletFont( &aTmpFont );
            }

            // Color
            pNumberFormat->SetBulletColor( pOldBullet->GetFont().GetColor() );

            // Start
            pNumberFormat->SetStart( pOldBullet->GetStart() );

            // Scale
            pNumberFormat->SetBulletRelSize( pOldBullet->GetScale() );

            // Bullet/Bitmap
            if( eNumType == SVX_NUM_CHAR_SPECIAL )
            {
                pNumberFormat->SetBulletChar( pOldBullet->GetSymbol() );
            }
            else if( eNumType == SVX_NUM_BITMAP )
            {
                SvxBrushItem aBItem( Graphic( pOldBullet->GetBitmap() ), GPOS_NONE );
                pNumberFormat->SetGraphicBrush( &aBItem );
            }
        }

        // Einzug und Erstzeileneinzug
        if ( pOldLRSpace )
        {
            short nLSpace = (short)pOldLRSpace->GetTxtLeft();
            pNumberFormat->SetLSpace( nLSpace );
            pNumberFormat->SetAbsLSpace( nLSpace );
            pNumberFormat->SetFirstLineOffset( pOldLRSpace->GetTxtFirstLineOfst() );
        }

        rNumBullet.GetNumRule()->SetLevel( nLevel, *pNumberFormat );
        delete pNumberFormat;
    }
}


