/*************************************************************************
 *
 *  $RCSfile: textview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2000-10-13 09:55:28 $
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

#pragma hdrstop

#include <textview.hxx>
#include <texteng.hxx>
#include <textdoc.hxx>
#include <textdata.hxx>
#include <textdat2.hxx>

#include <undo.hxx>

#ifndef _SV_CURSOR_HXX //autogen
#include <vcl/cursor.hxx>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _NEW_HXX
#include <tools/new.hxx>
#endif

#include <sot/formats.hxx>

#ifndef _URLBMK_HXX
#include <urlbmk.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XBREAKITERATOR_HPP_
#include <com/sun/star/text/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/text/CharacterIteratorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_WORDTYPE_HPP_
#include <com/sun/star/text/WordType.hpp>
#endif

using namespace ::com::sun::star;


// -------------------------------------------------------------------------
// (+) class TextView
// -------------------------------------------------------------------------
TextView::TextView( TextEngine* pEng, Window* pWindow )
{
    mpWindow = pWindow;
    mpTextEngine = pEng;
    mpVirtDev = NULL;

    mbPaintSelection = TRUE;
    mbAutoScroll = TRUE;
    mbInsertMode = TRUE;
    mbReadOnly = FALSE;
    mbHighlightSelection = FALSE;
    mbAutoIndent = FALSE;
    mbCursorEnabled = TRUE;
//  mbInSelection = FALSE;

    mnTravelXPos = TRAVEL_X_DONTKNOW;

    mpSelFuncSet = new TextSelFunctionSet( this );
    mpSelEngine = new SelectionEngine( mpWindow, mpSelFuncSet );
    mpSelEngine->SetSelectionMode( RANGE_SELECTION );
    mpSelEngine->EnableDrag( TRUE );

    mpCursor = new Cursor;
    mpCursor->Show();
    pWindow->SetCursor( mpCursor );

    if ( pWindow->GetSettings().GetStyleSettings().GetSelectionOptions() & SELECTION_OPTION_INVERT )
        mbHighlightSelection = TRUE;

    pWindow->SetLineColor();

    mpDDInfo = 0;
}

TextView::~TextView()
{
    delete mpSelEngine;
    delete mpSelFuncSet;
    delete mpVirtDev;

    if ( mpWindow->GetCursor() == mpCursor )
        mpWindow->SetCursor( 0 );
    delete mpCursor;
    delete mpDDInfo;
}

void TextView::Invalidate()
{
    mpWindow->Invalidate();
}

void TextView::SetSelection( const TextSelection& rTextSel, BOOL bGotoCursor )
{
    // Falls jemand gerade ein leeres Attribut hinterlassen hat,
    // und dann der Outliner die Selektion manipulitert:
    if ( !maSelection.HasRange() )
        mpTextEngine->CursorMoved( maSelection.GetStart().GetPara() );

    // Wenn nach einem KeyInput die Selection manipuliert wird:
    mpTextEngine->CheckIdleFormatter();

    HideSelection();
    maSelection = rTextSel;
    mpTextEngine->ValidateSelection( maSelection );
    ShowSelection();
    ShowCursor( bGotoCursor );
}

void TextView::SetSelection( const TextSelection& rTextSel )
{
    SetSelection( rTextSel, mbAutoScroll );
}

const TextSelection& TextView::GetSelection() const
{
    return maSelection;
}

void TextView::DeleteSelected()
{
//  HideSelection();

    mpTextEngine->UndoActionStart( TEXTUNDO_DELETE );
    TextPaM aPaM = mpTextEngine->ImpDeleteText( maSelection );
    mpTextEngine->UndoActionEnd( TEXTUNDO_DELETE );

    maSelection = aPaM;
    mpTextEngine->FormatAndUpdate( this );
    ShowCursor();
}

void TextView::ImpPaint( OutputDevice* pOut, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange, TextSelection const* pSelection )
{
    if ( !mbPaintSelection )
        pSelection = NULL;
    else
    {
        // Richtige Hintergrundfarbe einstellen.
        // Ich bekomme leider nicht mit, ob sich diese inzwischen geaendert hat.
        Font aFont = mpTextEngine->GetFont();
        Color aColor = pOut->GetBackground().GetColor();
        aColor.SetTransparency( 0 );
        if ( aColor != aFont.GetFillColor() )
        {
            aFont.SetTransparent( FALSE );
            aFont.SetFillColor( aColor );
            mpTextEngine->maFont = aFont;
        }
    }

    mpTextEngine->ImpPaint( pOut, rStartPos, pPaintArea, pPaintRange, pSelection );
}

void TextView::Paint( const Rectangle& rRect )
{
    ImpPaint( rRect, FALSE );
}

void TextView::ImpPaint( const Rectangle& rRect, BOOL bUseVirtDev )
{
    if ( !mpTextEngine->GetUpdateMode() || mpTextEngine->IsInUndo() )
        return;

    if ( bUseVirtDev )
    {
        VirtualDevice* pVDev = GetVirtualDevice();

        const Color& rBackgroundColor = mpWindow->GetBackground().GetColor();
        if ( pVDev->GetFillColor() != rBackgroundColor )
            pVDev->SetFillColor( rBackgroundColor );
        if ( pVDev->GetBackground().GetColor() != rBackgroundColor )
            pVDev->SetBackground( rBackgroundColor );

        BOOL bVDevValid = TRUE;
        Size aOutSz( pVDev->GetOutputSizePixel() );
        if ( (  aOutSz.Width() < rRect.GetWidth() ) ||
             (  aOutSz.Height() < rRect.GetHeight() ) )
        {
            bVDevValid = pVDev->SetOutputSizePixel( rRect.GetSize() );
        }
        else
        {
            // Das VirtDev kann bei einem Resize sehr gross werden =>
            // irgendwann mal kleiner machen!
            if ( ( aOutSz.Height() > ( rRect.GetHeight() + 20 ) ) ||
                 ( aOutSz.Width() > ( rRect.GetWidth() + 20 ) ) )
            {
                bVDevValid = pVDev->SetOutputSizePixel( rRect.GetSize() );
            }
            else
            {
                pVDev->Erase();
            }
        }
        if ( !bVDevValid )
        {
            ImpPaint( rRect, FALSE /* ohne VDev */ );
            return;
        }

        Rectangle aTmpRec( Point( 0, 0 ), rRect.GetSize() );
        Point aStartPos = GetDocPos( rRect.TopLeft() );
        aStartPos.X() *= (-1);
        aStartPos.Y() *= (-1);

        TextSelection *pTmpPtr = mbHighlightSelection ? NULL : &maSelection;
        ImpPaint( pVDev, aStartPos, &aTmpRec, NULL, pTmpPtr );

        mpWindow->DrawOutDev( rRect.TopLeft(), rRect.GetSize(),
                                Point(0,0), rRect.GetSize(), *pVDev );

//      ShowSelection();
        if ( mbHighlightSelection )
            ImpHighlight( maSelection );
    }
    else
    {
        Point aStartPos( -maStartDocPos.X(), -maStartDocPos.Y() );

        TextSelection *pTmpPtr = mbHighlightSelection ? NULL : &maSelection;
        ImpPaint( mpWindow, aStartPos, &rRect, NULL, pTmpPtr );

//      ShowSelection();
        if ( mbHighlightSelection )
            ImpHighlight( maSelection );
    }
}

void TextView::ImpHighlight( const TextSelection& rSel )
{
    TextSelection aSel( rSel );
    aSel.Justify();
    if ( aSel.HasRange() && !mpTextEngine->IsInUndo() && mpTextEngine->GetUpdateMode() )
    {
        BOOL bInvertSelection = FALSE;
#ifdef MAC
        bInvertSelection = ( mpWindow->GetBackground().GetColor() != COL_WHITE );
#endif
        mpCursor->Hide();

        DBG_ASSERT( !mpTextEngine->mpIdleFormatter->IsActive(), "ImpHighlight: Not formatted!" );

        Rectangle aVisArea( maStartDocPos, mpWindow->GetOutputSizePixel() );
        long nY = 0;
        ULONG nStartPara = aSel.GetStart().GetPara();
        ULONG nEndPara = aSel.GetEnd().GetPara();
        for ( ULONG nPara = 0; nPara <= nEndPara; nPara++ )
        {
            long nParaHeight = (long)mpTextEngine->CalcParaHeight( nPara );
            if ( ( nPara >= nStartPara ) && ( ( nY + nParaHeight ) > aVisArea.Top() ) )
            {
                TEParaPortion* pTEParaPortion = mpTextEngine->mpTEParaPortions->GetObject( nPara );
                USHORT nStartLine = 0;
                USHORT nEndLine = pTEParaPortion->GetLines().Count() -1;
                if ( nPara == nStartPara )
                    nStartLine = pTEParaPortion->GetLineNumber( aSel.GetStart().GetIndex(), FALSE );
                if ( nPara == nEndPara )
                    nEndLine = pTEParaPortion->GetLineNumber( aSel.GetEnd().GetIndex(), TRUE );

                // ueber die Zeilen iterieren....
                for ( USHORT nLine = nStartLine; nLine <= nEndLine; nLine++ )
                {
                    TextLine* pLine = pTEParaPortion->GetLines().GetObject( nLine );
                    USHORT nStartIndex = pLine->GetStart();
                    USHORT nEndIndex = pLine->GetEnd();
                    if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) )
                        nStartIndex = aSel.GetStart().GetIndex();
                    if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) )
                        nEndIndex = aSel.GetEnd().GetIndex();

                    // Kann passieren, wenn am Anfang einer umgebrochenen Zeile.
                    if ( nEndIndex < nStartIndex )
                        nEndIndex = nStartIndex;

                    Rectangle aTmpRec( mpTextEngine->GetEditCursor( TextPaM( nPara, nStartIndex ), FALSE ) );
                    aTmpRec.Top() += nY;
                    aTmpRec.Bottom() += nY;
                    Point aTopLeft( aTmpRec.TopLeft() );

                    aTmpRec = mpTextEngine->GetEditCursor( TextPaM( nPara, nEndIndex ), TRUE );
                    aTmpRec.Top() += nY;
                    aTmpRec.Bottom() += nY;
                    Point aBottomRight( aTmpRec.BottomRight() );
                    aBottomRight.X()--;

                    // Nur Painten, wenn im sichtbaren Bereich...
                    if ( ( aTopLeft.X() < aBottomRight.X() ) && ( aBottomRight.Y() >= aVisArea.Top() ) )
                    {
                        Point aPnt1( GetWindowPos( aTopLeft ) );
                        Point aPnt2( GetWindowPos( aBottomRight ) );

                        Rectangle aRect( aPnt1, aPnt2 );
                        /*! (pb) NOOLDSV
                        if ( bInvertSelection )
                            mpWindow->InvertRect( aRect );
                        else
                            mpWindow->HighlightRect( aRect );
                        */
                        mpWindow->Invert( aRect );
                    }
                }
            }
            nY += nParaHeight;

            if ( nY >= aVisArea.Bottom() )
                break;
        }
    }
}


void TextView::ShowSelection()
{
    if ( maSelection.HasRange() )
    {
        if ( mbHighlightSelection )
            ImpHighlight( maSelection );
        else
        {
            // Den Bereich neu zeichnen...
            Rectangle aOutArea( Point( 0, 0 ), mpWindow->GetOutputSizePixel() );
            Point aStartPos( -maStartDocPos.X(), -maStartDocPos.Y() );
            TextSelection aRange( maSelection );
            aRange.Justify();
            BOOL bVisCursor = mpCursor->IsVisible();
            mpCursor->Hide();
            ImpPaint( mpWindow, aStartPos, &aOutArea, &aRange, &maSelection );
            if ( bVisCursor )
                mpCursor->Show();
        }
    }
}

void TextView::HideSelection()
{
    if ( maSelection.HasRange() )
    {
        if ( mbHighlightSelection )
            ImpHighlight( maSelection );
        else
        {
            // Den Bereich neu zeichnen...
            Rectangle aOutArea( Point( 0, 0 ), mpWindow->GetOutputSizePixel() );
            Point aStartPos( -maStartDocPos.X(), -maStartDocPos.Y() );
            TextSelection aRange( maSelection );
            aRange.Justify();
            BOOL bVisCursor = mpCursor->IsVisible();
            mpCursor->Hide();
            ImpPaint( mpWindow, aStartPos, &aOutArea, &aRange, NULL );
            if ( bVisCursor )
                mpCursor->Show();
        }
    }
}

void TextView::ShowSelection( const TextSelection& rRange )
{
    if ( rRange.HasRange() )
    {
        if ( mbHighlightSelection )
            ImpHighlight( rRange );
        else
        {
            // Den Bereich neu zeichnen...
            Rectangle aOutArea( Point( 0, 0 ), mpWindow->GetOutputSizePixel() );
            Point aStartPos( -maStartDocPos.X(), -maStartDocPos.Y() );
            TextSelection aRange( rRange );
            aRange.Justify();
            BOOL bVisCursor = mpCursor->IsVisible();
            mpCursor->Hide();
            ImpPaint( mpWindow, aStartPos, &aOutArea, &aRange, &maSelection );
            if ( bVisCursor )
                mpCursor->Show();
        }
    }
}

VirtualDevice* TextView::GetVirtualDevice()
{
    if ( !mpVirtDev )
    {
        mpVirtDev = new VirtualDevice;
        mpVirtDev->SetLineColor();
    }
    return mpVirtDev;
}

void TextView::EraseVirtualDevice()
{
    delete mpVirtDev;
    mpVirtDev = 0;
}

BOOL TextView::KeyInput( const KeyEvent& rKeyEvent )
{
    BOOL bDone      = TRUE;
    BOOL bModified  = FALSE;
    BOOL bMoved     = FALSE;
    BOOL bEndKey    = FALSE;    // spezielle CursorPosition
    BOOL bAllowIdle = TRUE;

    // Um zu pruefen ob durch irgendeine Aktion mModified, das lokale
    // bModified wird z.B. bei Cut/Paste nicht gesetzt, weil dort an anderen
    // Stellen das updaten erfolgt.
    BOOL bWasModified = mpTextEngine->IsModified();
    mpTextEngine->SetModified( FALSE );

    TextSelection aCurSel( maSelection );
    TextSelection aOldSel( aCurSel );

    USHORT nCode = rKeyEvent.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !mbReadOnly )
                    Cut();
            }
            break;
            case KEYFUNC_COPY:
            {
                Copy();
            }
            break;
            case KEYFUNC_PASTE:
            {
                if ( !mbReadOnly )
                    Paste();
            }
            break;
            case KEYFUNC_UNDO:
            {
                if ( !mbReadOnly )
                    Undo();
            }
            break;
            case KEYFUNC_REDO:
            {
                if ( !mbReadOnly )
                    Redo();
            }
            break;

            default:    // wird dann evtl. unten bearbeitet.
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }
    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( nCode )
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
                {
                    aCurSel = ImpMoveCursor( rKeyEvent );
                    bMoved = TRUE;
                    if ( nCode == KEY_END )
                        bEndKey = TRUE;
                }
            }
            break;
            case KEY_BACKSPACE:
            case KEY_DELETE:
            {
                if ( !mbReadOnly && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    BYTE nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                    BYTE nMode = rKeyEvent.GetKeyCode().IsMod1() ? DELMODE_RESTOFWORD : DELMODE_SIMPLE;
                    if ( ( nMode == DELMODE_RESTOFWORD ) && rKeyEvent.GetKeyCode().IsShift() )
                        nMode = DELMODE_RESTOFCONTENT;

                    mpTextEngine->UndoActionStart( TEXTUNDO_DELETE );
                    aCurSel = ImpDelete( nDel, nMode );
                    mpTextEngine->UndoActionEnd( TEXTUNDO_DELETE );
                    bModified = TRUE;
                    bAllowIdle = FALSE;
                }
                else
                    bDone = FALSE;
            }
            break;
            case KEY_TAB:
            {
                if ( !mbReadOnly && !rKeyEvent.GetKeyCode().IsShift() &&
                        !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() &&
                        ImplCheckTextLen( 'x' ) )
                {
                    aCurSel = mpTextEngine->ImpInsertText( aCurSel, '\t', !IsInsertMode() );
                    bModified = TRUE;
                }
                else
                    bDone = FALSE;
            }
            break;
            case KEY_RETURN:
            {
                // Shift-RETURN darf nicht geschluckt werden, weil dann keine
                // mehrzeilige Eingabe in Dialogen/Property-Editor moeglich.
                if ( !mbReadOnly && !rKeyEvent.GetKeyCode().IsMod1() &&
                        !rKeyEvent.GetKeyCode().IsMod2() && ImplCheckTextLen( 'x' ) )
                {
                    mpTextEngine->UndoActionStart( TEXTUNDO_INSERT );
                    aCurSel = mpTextEngine->ImpInsertParaBreak( aCurSel );
                    if ( mbAutoIndent )
                    {
                        TextNode* pPrev = mpTextEngine->mpDoc->GetNodes().GetObject( aCurSel.GetEnd().GetPara() - 1 );
                        USHORT n = 0;
                        while ( ( n < pPrev->GetText().Len() ) && (
                                    ( pPrev->GetText().GetChar( n ) == ' ' ) ||
                                    ( pPrev->GetText().GetChar( n ) == '\t' ) ) )
                        {
                            n++;
                        }
                        if ( n )
                            aCurSel = mpTextEngine->ImpInsertText( aCurSel, pPrev->GetText().Copy( 0, n ) );
                    }
                    mpTextEngine->UndoActionEnd( TEXTUNDO_INSERT );
                    bModified = TRUE;
                }
                else
                    bDone = FALSE;
            }
            break;
            case KEY_INSERT:
            {
                if ( !mbReadOnly )
                    SetInsertMode( !IsInsertMode() );
            }
            break;
            default:
            {
                if ( TextEngine::IsSimpleCharInput( rKeyEvent ) )
                {
                    xub_Unicode nCharCode = rKeyEvent.GetCharCode();
                    if ( !mbReadOnly && ImplCheckTextLen( nCharCode ) ) // sonst trotzdem das Zeichen schlucken...
                    {
                        aCurSel = mpTextEngine->ImpInsertText( aCurSel, nCharCode, !IsInsertMode() );
                        bModified = TRUE;
                    }
                }
                else
                    bDone = FALSE;
            }
        }
    }

    if ( aCurSel != aOldSel )
        maSelection = aCurSel;

    mpTextEngine->UpdateSelections();

    if ( ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) )
        mnTravelXPos = TRAVEL_X_DONTKNOW;

    if ( bModified )
    {
        // Idle-Formatter nur, wenn AnyInput.
        if ( bAllowIdle && Application::AnyInput( INPUT_KEYBOARD) )
            mpTextEngine->IdleFormatAndUpdate( this );
        else
            mpTextEngine->FormatAndUpdate( this);
    }
    else if ( bMoved )
    {
        // Selection wird jetzt gezielt in ImpMoveCursor gemalt.
        ImpShowCursor( mbAutoScroll, TRUE, bEndKey );
    }

    if ( mpTextEngine->IsModified() )
        mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
    else if ( bWasModified )
        mpTextEngine->SetModified( TRUE );

    return bDone;
}

void TextView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpSelEngine->SelMouseButtonUp( rMouseEvent );
}

void TextView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    mpTextEngine->CheckIdleFormatter(); // Falls schnelles Tippen und MouseButtonDown
    mnTravelXPos = TRAVEL_X_DONTKNOW;

    mpTextEngine->SetActiveView( this );

    mpSelEngine->SelMouseButtonDown( rMouseEvent );

    // Sonderbehandlungen
    if ( !rMouseEvent.IsShift() && ( rMouseEvent.GetClicks() >= 2 ) )
    {
        if ( rMouseEvent.IsMod2() )
        {
            HideSelection();
            maSelection.GetStart() = maSelection.GetEnd();
            SetCursorAtPoint( rMouseEvent.GetPosPixel() );  // Wird von SelectionEngine bei MOD2 nicht gesetzt
        }

        if ( rMouseEvent.GetClicks() == 2 )
        {
            // Wort selektieren
            if ( maSelection.GetEnd().GetIndex() < mpTextEngine->GetTextLen( maSelection.GetEnd().GetPara() ) )
            {
                HideSelection();
                TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject(  maSelection.GetEnd().GetPara() );
                uno::Reference < text::XBreakIterator > xBI = mpTextEngine->GetBreakIterator();
                text::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), maSelection.GetEnd().GetIndex(), mpTextEngine->GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                maSelection.GetStart().GetIndex() = aBoundary.startPos;
                maSelection.GetEnd().GetIndex() = aBoundary.endPos;
                ShowSelection();
                ShowCursor( TRUE, TRUE );
            }
        }
        else if ( rMouseEvent.GetClicks() == 3 )
        {
            // Absatz selektieren
            if ( maSelection.GetEnd().GetIndex() < mpTextEngine->GetTextLen( maSelection.GetEnd().GetPara() ) )
            {
                HideSelection();
                maSelection.GetStart().GetIndex() = 0;
                maSelection.GetEnd().GetIndex() = mpTextEngine->mpDoc->GetNodes().GetObject( maSelection.GetEnd().GetPara() )->GetText().Len();
                ShowSelection();
                ShowCursor( TRUE, TRUE );
            }
        }
    }
}


void TextView::MouseMove( const MouseEvent& rMouseEvent )
{
    mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpSelEngine->SelMouseMove( rMouseEvent );
}

void TextView::Command( const CommandEvent& rCEvt )
{
    mpTextEngine->CheckIdleFormatter(); // Falls schnelles Tippen und MouseButtonDown
    mpTextEngine->SetActiveView( this );

    if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        DeleteSelected();
        delete mpTextEngine->mpIMEInfos;
        mpTextEngine->mpIMEInfos = new TEIMEInfos( GetSelection().GetEnd() );
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        DBG_ASSERT( mpTextEngine->mpIMEInfos, "COMMAND_ENDEXTTEXTINPUT => Kein Start ?" );
        if( mpTextEngine->mpIMEInfos )
        {
            TEParaPortion* pPortion = mpTextEngine->mpTEParaPortions->GetObject( mpTextEngine->mpIMEInfos->aPos.GetPara() );
            pPortion->MarkSelectionInvalid( mpTextEngine->mpIMEInfos->aPos.GetIndex(), 0 );

            delete mpTextEngine->mpIMEInfos;
            mpTextEngine->mpIMEInfos = NULL;

            mpTextEngine->FormatAndUpdate( this );
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        DBG_ASSERT( mpTextEngine->mpIMEInfos, "COMMAND_EXTTEXTINPUT => Kein Start ?" );
        if( mpTextEngine->mpIMEInfos )
        {
            const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

            TextSelection aSel( mpTextEngine->mpIMEInfos->aPos );
            aSel.GetEnd().GetIndex() += mpTextEngine->mpIMEInfos->nLen;
            SetSelection( aSel );
            DeleteSelected();
            aSel.GetEnd() = aSel.GetStart();
            aSel = mpTextEngine->ImpInsertText( aSel, pData->GetText() );

            if ( pData->GetTextAttr() )
            {
                mpTextEngine->mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().Len() );
                mpTextEngine->mpIMEInfos->bCursor = pData->IsCursorVisible();
            }
            else
            {
                mpTextEngine->mpIMEInfos->DestroyAttribs();
            }

            TEParaPortion* pPPortion = mpTextEngine->mpTEParaPortions->GetObject( mpTextEngine->mpIMEInfos->aPos.GetPara() );
            pPPortion->MarkSelectionInvalid( mpTextEngine->mpIMEInfos->aPos.GetIndex(), 0 );
            mpTextEngine->FormatAndUpdate( this );

            aSel.GetStart() = aSel.GetEnd();
            SetSelection( aSel );
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUTPOS )
    {
        if ( mpTextEngine->mpIMEInfos && mpTextEngine->mpIMEInfos->nLen )
        {
            const CommandExtTextInputPosData* pData = rCEvt.GetExtTextInputPosData();

            USHORT nChars = pData->GetChars();
            USHORT nStart = mpTextEngine->mpIMEInfos->aPos.GetIndex() + pData->GetFirstPos();

            TextPaM aPaM( mpTextEngine->mpIMEInfos->aPos );
            Rectangle* pRects = new Rectangle[ nChars ];
            for ( USHORT n = 0; n < nChars; n++ )
            {
                aPaM.GetIndex() = nStart+n;
                Rectangle aR1 = mpTextEngine->PaMtoEditCursor( aPaM, 0 );
                aR1.Move( -GetStartDocPos().X(), -GetStartDocPos().Y() );
                aPaM.GetIndex()++;
                Rectangle aR2 = mpTextEngine->PaMtoEditCursor( aPaM );
                aR2.Move( -GetStartDocPos().X(), -GetStartDocPos().Y() );

                pRects[n] = aR1;
                pRects[n].Right() = aR2.Left();

            }
            GetWindow()->SetExtTextInputPos( pData->GetFirstPos(), pData->GetChars(), pRects );
            delete pRects;
        }
        else
            GetWindow()->SetExtTextInputPos( 0, 0, NULL );
    }
    else
    {
        mpSelEngine->Command( rCEvt );
    }
}

void TextView::ShowCursor( BOOL bGotoCursor, BOOL bForceVisCursor )
{
    // Die Einstellung hat mehr Gewicht:
    if ( !mbAutoScroll )
        bGotoCursor = FALSE;
    ImpShowCursor( bGotoCursor, bForceVisCursor, FALSE );
}

void TextView::HideCursor()
{
    mpCursor->Hide();
}

void TextView::Scroll( long ndX, long ndY )
{
    DBG_ASSERT( mpTextEngine->IsFormatted(), "Scroll: Nicht formatiert!" );

    if ( !ndX && !ndY )
        return;

    Point aNewStartPos( maStartDocPos );

    // Vertical:
    aNewStartPos.Y() -= ndY;
    if ( aNewStartPos.Y() < 0 )
        aNewStartPos.Y() = 0;

    // Horizontal:
    aNewStartPos.X() -= ndX;
    if ( aNewStartPos.X() < 0 )
        aNewStartPos.X() = 0;

    long nDiffX = maStartDocPos.X() - aNewStartPos.X();
    long nDiffY = maStartDocPos.Y() - aNewStartPos.Y();

    if ( nDiffX || nDiffY )
    {
        BOOL bVisCursor = mpCursor->IsVisible();
        mpCursor->Hide();
        mpWindow->Update();
        maStartDocPos = aNewStartPos;

        mpWindow->Scroll( nDiffX, nDiffY );
        mpWindow->Update();
        mpCursor->SetPos( mpCursor->GetPos() + Point( nDiffX, nDiffY ) );
        if ( bVisCursor && !mbReadOnly )
            mpCursor->Show();
    }
}

void TextView::Undo()
{
    mpTextEngine->SetActiveView( this );
    mpTextEngine->GetUndoManager().Undo( 1 );
}

void TextView::Redo()
{
    mpTextEngine->SetActiveView( this );
    mpTextEngine->GetUndoManager().Redo( 0 );
}

void TextView::Cut()
{
    mpTextEngine->UndoActionStart( TEXTUNDO_CUT );
    Copy();
    DeleteSelected();
    mpTextEngine->UndoActionEnd( TEXTUNDO_CUT );
}

void TextView::Copy()
{
    Clipboard::Clear();
    TextSelection aSel( maSelection );
    aSel.Justify();
    SvMemoryStream aMem;
    mpTextEngine->Write( aMem, &aSel );
    aMem << '\0';
    Clipboard::CopyData( (const char*)aMem.GetData(), aMem.Tell(), FORMAT_STRING );
    if ( mpTextEngine->HasAttrib( TEXTATTR_HYPERLINK ) )
    {
        // Dann auch als HTML
        aMem.SetStreamSize( 0 );
        aMem.Seek( 0 );
        mpTextEngine->Write( aMem, &aSel, TRUE );
        aMem << '\0';
        Clipboard::CopyData( (const char*)aMem.GetData(), aMem.Tell(), SOT_FORMATSTR_ID_HTML );
    }
}

void TextView::Paste()
{
    BOOL bPaste = TRUE;
    if ( mpTextEngine->GetMaxTextLen() )
    {
        // QuickCheck, kein String > 64K moeglich...
        String aStr( Clipboard::PasteString() );
        aStr.ConvertLineEnd( LINEEND_LF );
        bPaste = ImplCheckTextLen( aStr );
    }

    if ( bPaste )
    {
        mpTextEngine->UndoActionStart( TEXTUNDO_PASTE );

        ULONG nLen = Clipboard::GetDataLen( FORMAT_STRING );
        void* pBuf = SvMemAlloc( nLen );
        Clipboard::PasteData( pBuf, nLen, FORMAT_STRING );

        // commented out while converting to Unicode
        // long nStringLen = strlen( (const char*) pBuf );  // 0-terminiert, Datenmuell im Clipborad dahinter.

        SvMemoryStream aMem( (char*)pBuf, nLen /*nStringLen*/, STREAM_READ );
        aMem.ObjectOwnsMemory( TRUE );
        mpTextEngine->Read( aMem, &maSelection );

        mpTextEngine->UndoActionEnd( TEXTUNDO_PASTE );
    }
}

String TextView::GetSelected()
{
    return mpTextEngine->GetText( maSelection, GetSystemLineEnd() );
}

void TextView::SetInsertMode( BOOL bInsert )
{
    if ( mbInsertMode != bInsert )
    {
        mbInsertMode = bInsert;
        ShowCursor( mbAutoScroll, FALSE );
    }
}

void TextView::SetReadOnly( BOOL bReadOnly )
{
    if ( mbReadOnly != bReadOnly )
    {
        mbReadOnly = bReadOnly;
        if ( !mbReadOnly )
            ShowCursor( mbAutoScroll, FALSE );
        else
            HideCursor();
    }
}

TextSelection TextView::ImpMoveCursor( const KeyEvent& rKeyEvent )
{
    // Eigentlich nur bei Up/Down noetig, aber was solls.
    mpTextEngine->CheckIdleFormatter();

    TextPaM aPaM( maSelection.GetEnd() );
    TextPaM aOldEnd( aPaM );

    BOOL bCtrl = rKeyEvent.GetKeyCode().IsMod1() ? TRUE : FALSE;

    switch ( rKeyEvent.GetKeyCode().GetCode() )
    {
        case KEY_UP:        aPaM = CursorUp( aPaM );
                            break;
        case KEY_DOWN:      aPaM = CursorDown( aPaM );
                            break;
        case KEY_HOME:      aPaM = bCtrl ? CursorStartOfDoc() : CursorStartOfLine( aPaM );
                            break;
        case KEY_END:       aPaM = bCtrl ? CursorEndOfDoc() : CursorEndOfLine( aPaM );
                            break;
        case KEY_PAGEUP:    aPaM = bCtrl ? CursorStartOfDoc() : PageUp( aPaM );
                            break;
        case KEY_PAGEDOWN:  aPaM = bCtrl ? CursorEndOfDoc() : PageDown( aPaM );
                            break;
        case KEY_LEFT:      aPaM = CursorLeft( aPaM, bCtrl );
                            break;
        case KEY_RIGHT:     aPaM = CursorRight( aPaM, bCtrl );
                            break;
    }

    // Bewirkt evtl. ein CreateAnchor oder Deselection all
    mpSelEngine->CursorPosChanging( rKeyEvent.GetKeyCode().IsShift(), rKeyEvent.GetKeyCode().IsMod1() );

    if ( aOldEnd != aPaM )
    {
        mpTextEngine->CursorMoved( aOldEnd.GetPara() );


        TextSelection aOldSelection( maSelection );
        maSelection.GetEnd() = aPaM;
        if ( rKeyEvent.GetKeyCode().IsShift() )
        {
            // Dann wird die Selektion erweitert...
            ShowSelection( TextSelection( aOldEnd, aPaM ) );
        }
        else
        {
            maSelection.GetStart() = aPaM;
        }
    }

    return maSelection;
}

void TextView::InsertText( const XubString& rStr, BOOL bSelect )
{
//  HideSelection();

    TextSelection aNewSel( maSelection );

    mpTextEngine->UndoActionStart( TEXTUNDO_INSERT );
    TextPaM aPaM = mpTextEngine->ImpInsertText( maSelection, rStr );
    mpTextEngine->UndoActionEnd( TEXTUNDO_INSERT );

    if ( bSelect )
    {
        aNewSel.Justify();
        aNewSel.GetEnd() = aPaM;
        maSelection = aNewSel;
    }
    else
        maSelection = aPaM;

    mpTextEngine->FormatAndUpdate( this );
}

TextPaM TextView::CursorLeft( const TextPaM& rPaM, BOOL bWordMode )
{
    TextPaM aPaM( rPaM );

    if ( aPaM.GetIndex() )
    {
        TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        uno::Reference < text::XBreakIterator > xBI = mpTextEngine->GetBreakIterator();
        if ( bWordMode )
        {
            text::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), rPaM.GetIndex(), mpTextEngine->GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
            if ( aBoundary.startPos == rPaM.GetIndex() )
                aBoundary = xBI->previousWord( pNode->GetText(), rPaM.GetIndex(), mpTextEngine->GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
            aPaM.GetIndex() = aBoundary.startPos;
        }
        else
        {
            sal_Int32 nCount = 1;
            aPaM.GetIndex() = xBI->previousCharacters( pNode->GetText(), aPaM.GetIndex(), mpTextEngine->GetLocale(), text::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
        }
    }
    else if ( aPaM.GetPara() )
    {
        aPaM.GetPara()--;
        TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        aPaM.GetIndex() = pNode->GetText().Len();
    }
    return aPaM;
}

TextPaM TextView::CursorRight( const TextPaM& rPaM, BOOL bWordMode )
{
    TextPaM aPaM( rPaM );

    TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
    if ( aPaM.GetIndex() < pNode->GetText().Len() )
    {
        uno::Reference < text::XBreakIterator > xBI = mpTextEngine->GetBreakIterator();
        if ( bWordMode )
        {
            text::Boundary aBoundary = xBI->nextWord(  pNode->GetText(), aPaM.GetIndex(), mpTextEngine->GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
            aPaM.GetIndex() = aBoundary.startPos;
        }
        else
        {
            sal_Int32 nCount = 1;
            aPaM.GetIndex() = xBI->nextCharacters( pNode->GetText(), aPaM.GetIndex(), mpTextEngine->GetLocale(), text::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
        }
    }
    else if ( aPaM.GetPara() < ( mpTextEngine->mpDoc->GetNodes().Count()-1) )
    {
        aPaM.GetPara()++;
        aPaM.GetIndex() = 0;
    }

    return aPaM;
}

TextPaM TextView::ImpDelete( BYTE nMode, BYTE nDelMode )
{
    if ( maSelection.HasRange() )   // dann nur Sel. loeschen
        return mpTextEngine->ImpDeleteText( maSelection );

    TextPaM aStartPaM = maSelection.GetStart();
    TextPaM aEndPaM = aStartPaM;
    if ( nMode == DEL_LEFT )
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorLeft( aEndPaM );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject(  aEndPaM.GetPara() );
            uno::Reference < text::XBreakIterator > xBI = mpTextEngine->GetBreakIterator();
            text::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), maSelection.GetEnd().GetIndex(), mpTextEngine->GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
            if ( aBoundary.startPos == maSelection.GetEnd().GetIndex() )
                aBoundary = xBI->previousWord( pNode->GetText(), maSelection.GetEnd().GetIndex(), mpTextEngine->GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
            aEndPaM.GetIndex() = aBoundary.startPos;
        }
        else    // DELMODE_RESTOFCONTENT
        {
            if ( aEndPaM.GetIndex() != 0 )
                aEndPaM.GetIndex() = 0;
            else if ( aEndPaM.GetPara() )
            {
                // Absatz davor
                aEndPaM.GetPara()--;
                aEndPaM.GetIndex() = 0;
            }
        }
    }
    else
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorRight( aEndPaM );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject(  aEndPaM.GetPara() );
            uno::Reference < text::XBreakIterator > xBI = mpTextEngine->GetBreakIterator();
            text::Boundary aBoundary = xBI->nextWord( pNode->GetText(), maSelection.GetEnd().GetIndex(), mpTextEngine->GetLocale(), text::WordType::ANYWORD_IGNOREWHITESPACES );
            aEndPaM.GetIndex() = aBoundary.startPos;
        }
        else    // DELMODE_RESTOFCONTENT
        {
            TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject( aEndPaM.GetPara() );
            if ( aEndPaM.GetIndex() < pNode->GetText().Len() )
                aEndPaM.GetIndex() = pNode->GetText().Len();
            else if ( aEndPaM.GetPara() < ( mpTextEngine->mpDoc->GetNodes().Count() - 1 ) )
            {
                // Absatz danach
                aEndPaM.GetPara()++;
                TextNode* pNextNode = mpTextEngine->mpDoc->GetNodes().GetObject( aEndPaM.GetPara() );
                aEndPaM.GetIndex() = pNextNode->GetText().Len();
            }
        }
    }

    return mpTextEngine->ImpDeleteText( TextSelection( aStartPaM, aEndPaM ) );
}



TextPaM TextView::CursorUp( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    long nX;
    if ( mnTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = mpTextEngine->GetEditCursor( rPaM, FALSE ).Left();
        mnTravelXPos = nX+1;
    }
    else
        nX = mnTravelXPos;

    TEParaPortion* pPPortion = mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), FALSE );
    if ( nLine )    // gleicher Absatz
    {
        USHORT nCharPos = mpTextEngine->GetCharPos( rPaM.GetPara(), nLine-1, nX );
        aPaM.GetIndex() = nCharPos;
        // Wenn davor eine autom.Umgebrochene Zeile, und ich muss genau an das
        // Ende dieser Zeile, landet der Cursor in der aktuellen Zeile am Anfang
        // Siehe Problem: Letztes Zeichen einer autom.umgebr. Zeile = Cursor
        TextLine* pLine = pPPortion->GetLines().GetObject( nLine - 1 );
        if ( aPaM.GetIndex() && ( aPaM.GetIndex() == pLine->GetEnd() ) )
            aPaM.GetIndex()--;
    }
    else if ( rPaM.GetPara() )  // vorheriger Absatz
    {
        aPaM.GetPara()--;
        pPPortion = mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        USHORT nL = pPPortion->GetLines().Count() - 1;
        USHORT nCharPos = mpTextEngine->GetCharPos( aPaM.GetPara(), nL, nX+1 );
        aPaM.GetIndex() = nCharPos;
    }

    return aPaM;
}

TextPaM TextView::CursorDown( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    long nX;
    if ( mnTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = mpTextEngine->GetEditCursor( rPaM, FALSE ).Left();
        mnTravelXPos = nX+1;
    }
    else
        nX = mnTravelXPos;

    TEParaPortion* pPPortion = mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), FALSE );
    if ( nLine < ( pPPortion->GetLines().Count() - 1 ) )
    {
        USHORT nCharPos = mpTextEngine->GetCharPos( rPaM.GetPara(), nLine+1, nX );
        aPaM.GetIndex() = nCharPos;

        // Sonderbehandlung siehe CursorUp...
        TextLine* pLine = pPPortion->GetLines().GetObject( nLine + 1 );
        if ( ( aPaM.GetIndex() == pLine->GetEnd() ) && ( aPaM.GetIndex() > pLine->GetStart() ) && aPaM.GetIndex() < pPPortion->GetNode()->GetText().Len() )
            aPaM.GetIndex()--;
    }
    else if ( rPaM.GetPara() < ( mpTextEngine->mpDoc->GetNodes().Count() - 1 ) )    // naechster Absatz
    {
        aPaM.GetPara()++;
        pPPortion = mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        USHORT nCharPos = mpTextEngine->GetCharPos( aPaM.GetPara(), 0, nX+1 );
        aPaM.GetIndex() = nCharPos;
        TextLine* pLine = pPPortion->GetLines().GetObject( 0 );
        if ( ( aPaM.GetIndex() == pLine->GetEnd() ) && ( aPaM.GetIndex() > pLine->GetStart() ) && ( pPPortion->GetLines().Count() > 1 ) )
            aPaM.GetIndex()--;
    }

    return aPaM;
}

TextPaM TextView::CursorStartOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), FALSE );
    TextLine* pLine = pPPortion->GetLines().GetObject( nLine );
    aPaM.GetIndex() = pLine->GetStart();

    return aPaM;
}

TextPaM TextView::CursorEndOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    USHORT nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), FALSE );
    TextLine* pLine = pPPortion->GetLines().GetObject( nLine );
    aPaM.GetIndex() = pLine->GetEnd();

    if ( pLine->GetEnd() > pLine->GetStart() )  // Leerzeile
    {
        xub_Unicode cLastChar = pPPortion->GetNode()->GetText().GetChar((USHORT)(aPaM.GetIndex()-1) );
        if ( ( cLastChar == ' ' ) && ( aPaM.GetIndex() != pPPortion->GetNode()->GetText().Len() ) )
        {
            // Bei einem Blank in einer autom. umgebrochenen Zeile macht es Sinn,
            // davor zu stehen, da der Anwender hinter das Wort will.
            // Wenn diese geaendert wird, Sonderbehandlung fuer Pos1 nach End!
            aPaM.GetIndex()--;
        }
    }
    return aPaM;
}

TextPaM TextView::CursorStartOfParagraph( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );
    aPaM.GetIndex() = 0;
    return aPaM;
}

TextPaM TextView::CursorEndOfParagraph( const TextPaM& rPaM )
{
    TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject( rPaM.GetPara() );
    TextPaM aPaM( rPaM );
    aPaM.GetIndex() = pNode->GetText().Len();
    return aPaM;
}

TextPaM TextView::CursorStartOfDoc()
{
    TextPaM aPaM( 0, 0 );
    return aPaM;
}

TextPaM TextView::CursorEndOfDoc()
{
    ULONG nNode = mpTextEngine->mpDoc->GetNodes().Count() - 1;
    TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject( nNode );
    TextPaM aPaM( nNode, pNode->GetText().Len() );
    return aPaM;
}

TextPaM TextView::PageUp( const TextPaM& rPaM )
{
    Rectangle aRec = mpTextEngine->PaMtoEditCursor( rPaM );
    Point aTopLeft = aRec.TopLeft();
    aTopLeft.Y() -= mpWindow->GetOutputSizePixel().Height() * 9/10;
    aTopLeft.X() += 1;
    if ( aTopLeft.Y() < 0 )
        aTopLeft.Y() = 0;

    TextPaM aPaM = mpTextEngine->GetPaM( aTopLeft );
    return aPaM;
}

TextPaM TextView::PageDown( const TextPaM& rPaM )
{
    Rectangle aRec = mpTextEngine->PaMtoEditCursor( rPaM );
    Point aBottomRight = aRec.BottomRight();
    aBottomRight.Y() += mpWindow->GetOutputSizePixel().Height() * 9/10;
    aBottomRight.X() += 1;
    long nHeight = mpTextEngine->GetTextHeight();
    if ( aBottomRight.Y() > nHeight )
        aBottomRight.Y() = nHeight-1;

    TextPaM aPaM = mpTextEngine->GetPaM( aBottomRight );
    return aPaM;
}

void TextView::ImpShowCursor( BOOL bGotoCursor, BOOL bForceVisCursor, BOOL bSpecial )
{
    if ( mpTextEngine->IsFormatting() )
        return;
    if ( mpTextEngine->GetUpdateMode() == FALSE )
        return;
    if ( mpTextEngine->IsInUndo() )
        return;

    mpTextEngine->CheckIdleFormatter();
    if ( !mpTextEngine->IsFormatted() )
        mpTextEngine->FormatAndUpdate( this );


    TextPaM aPaM( maSelection.GetEnd() );
    Rectangle aEditCursor = mpTextEngine->PaMtoEditCursor( aPaM, bSpecial );
    if ( !IsInsertMode() && !maSelection.HasRange() )
    {
        TextNode* pNode = mpTextEngine->mpDoc->GetNodes().GetObject( aPaM.GetPara() );
        if ( pNode->GetText().Len() && ( aPaM.GetIndex() < pNode->GetText().Len() ) )
        {
            long nWidth = (long)mpTextEngine->CalcTextWidth( aPaM.GetPara(), aPaM.GetIndex(), 1 );
            aEditCursor.Right() += nWidth;
        }
    }

    Size aOutSz = mpWindow->GetOutputSizePixel();
    if ( aEditCursor.GetHeight() > aOutSz.Height() )
        aEditCursor.Bottom() = aEditCursor.Top() + aOutSz.Height() - 1;

    if ( bGotoCursor  )
    {
        long nVisStartY = maStartDocPos.Y();
        long nVisEndY = maStartDocPos.Y() + aOutSz.Height();
        long nVisStartX = maStartDocPos.X();
        long nVisEndX = maStartDocPos.X() + aOutSz.Width();
        long nMoreX = aOutSz.Width() / 4;

        Point aNewStartPos( maStartDocPos );

        if ( aEditCursor.Bottom() > nVisEndY )
        {
            aNewStartPos.Y() += ( aEditCursor.Bottom() - nVisEndY );
        }
        else if ( aEditCursor.Top() < nVisStartY )
        {
            aNewStartPos.Y() -= ( nVisStartY - aEditCursor.Top() );
        }

        if ( aEditCursor.Right() > nVisEndX )
        {
            aNewStartPos.X() += ( aEditCursor.Right() - nVisEndX );

            // Darfs ein bischen mehr sein?
            aNewStartPos.X() += nMoreX;
        }
        else if ( aEditCursor.Left() < nVisStartX )
        {
            aNewStartPos.X() -= ( nVisStartX - aEditCursor.Left() );

            // Darfs ein bischen mehr sein?
            aNewStartPos.X() -= nMoreX;
        }

        // X kann durch das 'bischen mehr' falsch sein:
//      ULONG nMaxTextWidth = mpTextEngine->GetMaxTextWidth();
//      if ( !nMaxTextWidth || ( nMaxTextWidth > 0x7FFFFFFF ) )
//          nMaxTextWidth = 0x7FFFFFFF;
//      long nMaxX = (long)nMaxTextWidth - aOutSz.Width();
        long nMaxX = mpTextEngine->CalcTextWidth() - aOutSz.Width();
        if ( nMaxX < 0 )
            nMaxX = 0;

        if ( aNewStartPos.X() < 0 )
            aNewStartPos.X() = 0;
        else if ( aNewStartPos.X() > nMaxX )
            aNewStartPos.X() = nMaxX;

        // Y sollte nicht weiter unten als noetig liegen:
        long nYMax = mpTextEngine->GetTextHeight() - aOutSz.Height();
        if ( nYMax < 0 )
            nYMax = 0;
        if ( aNewStartPos.Y() > nYMax )
            aNewStartPos.Y() = nYMax;

        if ( aNewStartPos != maStartDocPos )
        {
            Scroll( -(aNewStartPos.X() - maStartDocPos.X()), -(aNewStartPos.Y() - maStartDocPos.Y()) );
            mpTextEngine->Broadcast( TextHint( TEXT_HINT_VIEWSCROLLED ) );
        }
    }

    Point aPoint( GetWindowPos( aEditCursor.TopLeft() ) );
    mpCursor->SetPos( aPoint );
    mpCursor->SetSize( aEditCursor.GetSize() );
    if ( bForceVisCursor && mbCursorEnabled )
        mpCursor->Show();
}

BOOL TextView::SetCursorAtPoint( const Point& rPosPixel )
{
//  if ( !Rectangle( Point(), mpWindow->GetOutputSizePixel() ).IsInside( rPosPixel ) && !mbInSelection )
//      return FALSE;

    mpTextEngine->CheckIdleFormatter();

    Point aDocPos = GetDocPos( rPosPixel );

    TextPaM aPaM = mpTextEngine->GetPaM( aDocPos );

    // aTmpNewSel: Diff zwischen alt und neu, nicht die neue Selektion
    TextSelection aTmpNewSel( maSelection.GetEnd(), aPaM );
    maSelection.GetEnd() = aPaM;

    if ( !mpSelEngine->HasAnchor() )
    {
        if ( maSelection.GetStart() != aPaM )
            mpTextEngine->CursorMoved( maSelection.GetStart().GetPara() );
        maSelection.GetStart() = aPaM;
    }
    else
    {
        ShowSelection( aTmpNewSel );
    }

    BOOL bForceCursor =  mpDDInfo ? FALSE : TRUE; // && !mbInSelection
    ImpShowCursor( mbAutoScroll, bForceCursor, FALSE );
    return TRUE;
}

BOOL TextView::IsSelectionAtPoint( const Point& rPosPixel )
{
//  if ( !Rectangle( Point(), mpWindow->GetOutputSizePixel() ).IsInside( rPosPixel ) && !mbInSelection )
//      return FALSE;

    Point aDocPos = GetDocPos( rPosPixel );
    TextPaM aPaM = mpTextEngine->GetPaM( aDocPos, FALSE );
    // Bei Hyperlinks D&D auch ohne Selektion starten.
    // BeginDrag wird aber nur gerufen, wenn IsSelectionAtPoint()
    // Problem: IsSelectionAtPoint wird bei Command() nicht gerufen,
    // wenn vorher im MBDown schon FALSE returnt wurde.
    return ( IsInSelection( aPaM ) ||
            ( /* mpSelEngine->IsInCommand() && */ mpTextEngine->FindAttrib( aPaM, TEXTATTR_HYPERLINK ) ) );
}

BOOL TextView::IsInSelection( const TextPaM& rPaM )
{
    TextSelection aSel = maSelection;
    aSel.Justify();

    ULONG nStartNode = aSel.GetStart().GetPara();
    ULONG nEndNode = aSel.GetEnd().GetPara();
    ULONG nCurNode = rPaM.GetPara();

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return TRUE;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
                return TRUE;
    }
    else if ( ( nCurNode == nStartNode ) && ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) )
        return TRUE;
    else if ( ( nCurNode == nEndNode ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
        return TRUE;

    return FALSE;
}

void TextView::ImpHideDDCursor()
{
    if ( mpDDInfo && mpDDInfo->mbVisCursor )
    {
        mpDDInfo->maCursor.Hide();
        mpDDInfo->mbVisCursor = FALSE;
    }
}

void TextView::ImpShowDDCursor()
{
    if ( !mpDDInfo->mbVisCursor )
    {
        Rectangle aCursor = mpTextEngine->PaMtoEditCursor( mpDDInfo->maDropPos, TRUE );
        aCursor.Right()++;
        aCursor.SetPos( GetWindowPos( aCursor.TopLeft() ) );

        mpDDInfo->maCursor.SetWindow( mpWindow );
        mpDDInfo->maCursor.SetPos( aCursor.TopLeft() );
        mpDDInfo->maCursor.SetSize( aCursor.GetSize() );
        mpDDInfo->maCursor.Show();
        mpDDInfo->mbVisCursor = TRUE;
    }
}

void TextView::BeginDrag()
{
    delete mpDDInfo;
    mpDDInfo = new TextDDInfo;

    mpDDInfo->mbStarterOfDD = TRUE;

    DragServer::Clear();

    TextSelection aSel( maSelection );
    aSel.Justify();

    // D&D eines Hyperlinks.
    // Besser waere es im MBDown sich den MBDownPaM zu merken,
    // ist dann aber inkompatibel => spaeter mal umstellen.
    TextPaM aPaM( mpTextEngine->GetPaM( GetDocPos( GetWindow()->GetPointerPosPixel() ) ) );

    const TextCharAttrib* pAttr = mpTextEngine->FindCharAttrib( aPaM, TEXTATTR_HYPERLINK );
    if ( pAttr )
    {
        aSel = aPaM;
        aSel.GetStart().GetIndex() = pAttr->GetStart();
        aSel.GetEnd().GetIndex() = pAttr->GetEnd();

        const TextAttribHyperLink& rLink = (const TextAttribHyperLink&)pAttr->GetAttr();
        String aText( rLink.GetDescription() );
        if ( !aText.Len() )
            aText = mpTextEngine->GetText( aSel );
        INetBookmark aBookmark( rLink.GetURL(), aText );
        aBookmark.CopyDragServer();
    }

    DragServer::CopyString( mpTextEngine->GetText( aSel ) );

    Region* pDDRegion = NULL;
#ifdef MAC
    // ... Region ermitteln
    Size aOutSz = mpWindow->GetOutputSizePixel();
    Rectangle aStartCursor( mpTextEngine->GetEditCursor( aSel.GetStart(), FALSE ) );
    Rectangle aEndCursor( mpTextEngine->GetEditCursor( aSel.GetEnd(), TRUE ) );

    Point aTopLeft = aStartCursor.TopLeft();
    aTopLeft -= maStartDocPos;

    Point aBottomRight = aStartCursor.BottomRight();
    aBottomRight -= maStartDocPos;

    if ( aStartCursor.Top() != aEndCursor.Top() )
    {
        // Dann einfach das Rechteck mit den kompletten Zeilem
        aTopLeft.X() = 0;
        aBottomRight.X() = aOutSz.Width();
    }

    pDDRegion = new Region( Rectangle( aTopLeft, aBottomRight ) );
#endif

    mpCursor->Hide();

    mpWindow->ExecuteDrag( Pointer( POINTER_MOVEDATA ),
                        Pointer( POINTER_COPYDATA ), DRAG_ALL, pDDRegion );

    DragServer::Clear();
    ImpHideDDCursor();

    delete mpDDInfo;
    mpDDInfo = 0;
}

BOOL TextView::QueryDrop( DropEvent& rEvt )
{
    if ( rEvt.IsLeaveWindow() )
    {
        ImpHideDDCursor();
        return FALSE;
    }

    BOOL bDrop = FALSE;
    if ( !mbReadOnly && DragServer::HasFormat( 0, FORMAT_STRING ) &&
        ( ( rEvt.GetAction() == DROP_COPY ) || ( rEvt.GetAction() == DROP_MOVE ) ) )
    {
        if ( !mpDDInfo )
            mpDDInfo = new TextDDInfo;

        TextPaM aPrevDropPos = mpDDInfo->maDropPos;
        Point aDocPos = GetDocPos( rEvt.GetPosPixel() );
        mpDDInfo->maDropPos = mpTextEngine->GetPaM( aDocPos );

        Size aOutSize = mpWindow->GetOutputSizePixel();
        if ( ( rEvt.GetPosPixel().X() < 0 ) || ( rEvt.GetPosPixel().X() > aOutSize.Width() ) ||
             ( rEvt.GetPosPixel().Y() < 0 ) || ( rEvt.GetPosPixel().Y() > aOutSize.Height() ) )
        {
            // Scrollen ?
        }

        // Nicht in Selektion droppen:
        if ( IsInSelection( mpDDInfo->maDropPos ) )
        {
            ImpHideDDCursor();
            return FALSE;
        }

        // Alten Cursor wegzeichnen...
        if ( !mpDDInfo->mbVisCursor || ( aPrevDropPos != mpDDInfo->maDropPos ) )
        {
            ImpHideDDCursor();
            ImpShowDDCursor();
        }
        bDrop = TRUE;
    }
    return bDrop;
}

BOOL TextView::Drop( const DropEvent& rEvt )
{
    BOOL bDone = FALSE;
    if ( !mbReadOnly && mpDDInfo )
    {
        ImpHideDDCursor();

        // Daten fuer das loeschen nach einem DROP_MOVE:
        TextSelection aPrevSel( maSelection );
        aPrevSel.Justify();
        ULONG nPrevParaCount = mpTextEngine->GetParagraphCount();
        USHORT nPrevStartParaLen = mpTextEngine->GetTextLen( aPrevSel.GetStart().GetPara() );

        BOOL bStarterOfDD = FALSE;
        for ( USHORT nView = mpTextEngine->GetViewCount(); nView && !bStarterOfDD; )
            bStarterOfDD = mpTextEngine->GetView( --nView )->mpDDInfo ? mpTextEngine->GetView( nView )->mpDDInfo->mbStarterOfDD : FALSE;

        HideSelection();
        maSelection = mpDDInfo->maDropPos;

        mpTextEngine->UndoActionStart( TEXTUNDO_DRAGANDDROP );

        String aText = DragServer::PasteString( 0 );
        aText.ConvertLineEnd( LINEEND_LF );

        if ( aText.Len() && ( aText.GetChar( aText.Len()-1 ) == LINE_SEP ) )
            aText.Erase( aText.Len()-1 );

        if ( ImplCheckTextLen( aText ) )
            maSelection = mpTextEngine->ImpInsertText( mpDDInfo->maDropPos, aText );

        if ( aPrevSel.HasRange() &&
                ( rEvt.GetAction() == DROP_MOVE ) || !bStarterOfDD )
        {
            // ggf. Selection anpasssen:
            if ( ( mpDDInfo->maDropPos.GetPara() < aPrevSel.GetStart().GetPara() ) ||
                 ( ( mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                        && ( mpDDInfo->maDropPos.GetIndex() < aPrevSel.GetStart().GetIndex() ) ) )
            {
                ULONG nNewParasBeforeSelection =
                    mpTextEngine->GetParagraphCount() - nPrevParaCount;

                aPrevSel.GetStart().GetPara() += nNewParasBeforeSelection;
                aPrevSel.GetEnd().GetPara() += nNewParasBeforeSelection;

                if ( mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                {
                    USHORT nNewChars =
                        mpTextEngine->GetTextLen( aPrevSel.GetStart().GetPara() ) - nPrevStartParaLen;

                    aPrevSel.GetStart().GetIndex() += nNewChars;
                    if ( aPrevSel.GetStart().GetPara() == aPrevSel.GetEnd().GetPara() )
                        aPrevSel.GetEnd().GetIndex() += nNewChars;
                }
            }
            else
            {
                // aktuelle Selektion anpassen
                TextPaM aPaM = maSelection.GetStart();
                aPaM.GetPara() -= ( aPrevSel.GetEnd().GetPara() - aPrevSel.GetStart().GetPara() );
                if ( aPrevSel.GetEnd().GetPara() == mpDDInfo->maDropPos.GetPara() )
                {
                    aPaM.GetIndex() -= aPrevSel.GetEnd().GetIndex();
                    if ( aPrevSel.GetStart().GetPara() == mpDDInfo->maDropPos.GetPara() )
                        aPaM.GetIndex() += aPrevSel.GetStart().GetIndex();
                }
                maSelection = aPaM;

            }
            mpTextEngine->ImpDeleteText( aPrevSel );
        }

        mpTextEngine->UndoActionEnd( TEXTUNDO_DRAGANDDROP );

        delete mpDDInfo;
        mpDDInfo = 0;

        mpTextEngine->FormatAndUpdate( this );

        mpTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
    }

    return bDone;
}

void TextView::SetPaintSelection( BOOL bPaint )
{
    if ( bPaint != mbPaintSelection )
    {
        mbPaintSelection = bPaint;
        ShowSelection( maSelection );
    }
}

void TextView::SetHighlightSelection( BOOL bSelectByHighlight )
{
    if ( bSelectByHighlight != mbHighlightSelection )
    {
        // Falls umschalten zwischendurch moeglich...
        mbHighlightSelection = bSelectByHighlight;
    }
}

BOOL TextView::Read( SvStream& rInput )
{
    BOOL bDone = mpTextEngine->Read( rInput, &maSelection );
    ShowCursor();
    return bDone;
}

BOOL TextView::Write( SvStream& rOutput )
{
    return mpTextEngine->Read( rOutput, &maSelection );
}

BOOL TextView::ImplCheckTextLen( const String& rNewText )
{
    BOOL bOK = TRUE;
    if ( mpTextEngine->GetMaxTextLen() )
    {
        ULONG n = mpTextEngine->GetTextLen();
        n += rNewText.Len();
        if ( n > mpTextEngine->GetMaxTextLen() )
        {
            // nur dann noch ermitteln, wie viel Text geloescht wird
            n -= mpTextEngine->GetTextLen( maSelection );
            if ( n > mpTextEngine->GetMaxTextLen() )
            {
                // Beep hat hier eigentlich nichts verloren, sondern lieber ein Hdl,
                // aber so funktioniert es wenigstens in ME, BasicIDE, SourceView
                Sound::Beep();
                bOK = FALSE;
            }
        }
    }
    return bOK;
}


// -------------------------------------------------------------------------
// (+) class TextSelFunctionSet
// -------------------------------------------------------------------------
TextSelFunctionSet::TextSelFunctionSet( TextView* pView )
{
    mpView = pView;
}

void __EXPORT TextSelFunctionSet::BeginDrag()
{
    mpView->BeginDrag();
}

void __EXPORT TextSelFunctionSet::CreateAnchor()
{
//  TextSelection aSel( mpView->GetSelection() );
//  aSel.GetStart() = aSel.GetEnd();
//  mpView->SetSelection( aSel );

    // Es darf kein ShowCursor folgen:
    mpView->HideSelection();
    mpView->maSelection.GetStart() = mpView->maSelection.GetEnd();
}

BOOL __EXPORT TextSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, BOOL )
{
    return mpView->SetCursorAtPoint( rPointPixel );
}

BOOL __EXPORT TextSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    return mpView->IsSelectionAtPoint( rPointPixel );
}

void __EXPORT TextSelFunctionSet::DeselectAll()
{
    CreateAnchor();
}

void __EXPORT TextSelFunctionSet::DeselectAtPoint( const Point& )
{
    // Nur bei Mehrfachselektion
}

void __EXPORT TextSelFunctionSet::DestroyAnchor()
{
    // Nur bei Mehrfachselektion
}

