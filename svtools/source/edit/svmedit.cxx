/*************************************************************************
 *
 *  $RCSfile: svmedit.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-08 10:41:47 $
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

#include <vcl/rc.h>
#include <vcl/decoview.hxx>

#include <svmedit.hxx>
#include <xtextedt.hxx>
#include <brdcst.hxx>
#include <lstner.hxx>

#ifndef _UNDO_HXX
#include <undo.hxx>
#endif


// IDs erstmal aus VCL geklaut, muss mal richtig delivert werden...
#define SV_MENU_EDIT_UNDO           1
#define SV_MENU_EDIT_CUT            2
#define SV_MENU_EDIT_COPY           3
#define SV_MENU_EDIT_PASTE          4
#define SV_MENU_EDIT_DELETE         5
#define SV_MENU_EDIT_SELECTALL      6
#define SV_MENU_EDIT_INSERTSYMBOL   7

#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif


class TextWindow : public Window
{
private:
    ExtTextEngine*  mpExtTextEngine;
    ExtTextView*    mpExtTextView;

    BOOL            mbInMBDown;
    BOOL            mbFocusSelectionHide;
    BOOL            mbIgnoreTab;
    BOOL            mbActivePopup;

public:
                    TextWindow( Window* pParent );
                    ~TextWindow();

    ExtTextEngine*  GetTextEngine() const { return mpExtTextEngine; }
    ExtTextView*    GetTextView() const { return mpExtTextView; }

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvent );

    virtual BOOL    QueryDrop( DropEvent& rEvt );
    virtual BOOL    Drop( const DropEvent& rEvt );

    virtual void    Command( const CommandEvent& rCEvt );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();

    virtual void    GetFocus();
    virtual void    LoseFocus();

    BOOL            IsAutoFocusHide() const { return mbFocusSelectionHide; }
    void            SetAutoFocusHide( BOOL bAutoHide ) { mbFocusSelectionHide = bAutoHide; }

    BOOL            IsIgnoreTab() const { return mbIgnoreTab; }
    void            SetIgnoreTab( BOOL bIgnore ) { mbIgnoreTab = bIgnore; }
};


class ImpSvMEdit : public SfxListener
{
private:
    MultiLineEdit*      pSvMultiLineEdit;

    TextWindow*         mpTextWindow;
    ScrollBar*          mpHScrollBar;
    ScrollBar*          mpVScrollBar;
    ScrollBarBox*       mpScrollBox;

    Point               maTextWindowOffset;
    ULONG               mnTextWidth;

protected:
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    void                ImpInitScrollBars();
    void                ImpSetScrollBarRanges();
    DECL_LINK(          ScrollHdl, ScrollBar* );

public:
                ImpSvMEdit( MultiLineEdit* pSvMultiLineEdit, WinBits nWinStyle );
                ~ImpSvMEdit();

    void        SetModified( BOOL bMod );
    BOOL        IsModified() const;

    void        SetReadOnly( BOOL bRdOnly );
    BOOL        IsReadOnly() const;

    void        SetMaxTextLen( ULONG nLen );
    ULONG       GetMaxTextLen() const;

    void        SetInsertMode( BOOL bInsert );
    BOOL        IsInsertMode() const;

    void        InsertText( const String& rStr );
    String      GetSelected() const;

    void        SetSelection( const Selection& rSelection );
    Selection   GetSelection() const;

    void        Cut();
    void        Copy();
    void        Paste();

    void        SetText( const String& rStr );
    String      GetText() const;
    String      GetTextLines() const;

    void        Resize();
    void        GetFocus();

    BOOL        HandleCommand( const CommandEvent& rCEvt );

    void        Enable( BOOL bEnable );

    Size        CalcMinimumSize() const;
    Size        CalcSize( USHORT nColumns, USHORT nLines ) const;
    void        GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const;

    void        SetAlign( WinBits nWinStyle );

    TextWindow* GetTextWindow() { return mpTextWindow; }
    ScrollBar*  GetHScrollBar() { return mpHScrollBar; }
    ScrollBar*  GetVScrollBar() { return mpVScrollBar; }

    void        SetTextWindowOffset( const Point& rOffset );
};


ImpSvMEdit::ImpSvMEdit( MultiLineEdit* pEdt, WinBits nWinStyle )
{
    pSvMultiLineEdit = pEdt;

    mnTextWidth = 0;

    mpTextWindow = new TextWindow( pEdt );
    mpHScrollBar = ( nWinStyle & WB_HSCROLL ) ? new ScrollBar( pEdt, WB_HSCROLL|WB_DRAG ) : NULL;
    mpVScrollBar = ( nWinStyle & WB_VSCROLL ) ? new ScrollBar( pEdt, WB_VSCROLL|WB_DRAG ) : NULL;
    mpScrollBox = ( mpVScrollBar && mpHScrollBar ) ? new ScrollBarBox( pEdt, WB_SIZEABLE ) : NULL;

    SetAlign( nWinStyle );
    StartListening( *mpTextWindow->GetTextEngine() );

    if ( nWinStyle & WB_NOHIDESELECTION )
        mpTextWindow->SetAutoFocusHide( FALSE );
    else
        mpTextWindow->SetAutoFocusHide( TRUE );

    if ( nWinStyle & WB_READONLY )
        mpTextWindow->GetTextView()->SetReadOnly( TRUE );

    if ( nWinStyle & WB_IGNORETAB )
        mpTextWindow->SetIgnoreTab( TRUE );

    mpTextWindow->Show();
    if ( mpHScrollBar )
    {
        mpHScrollBar->Show();
        mpHScrollBar->SetScrollHdl( LINK( this, ImpSvMEdit, ScrollHdl ) );
    }
    if ( mpVScrollBar )
    {
        mpVScrollBar->Show();
        mpVScrollBar->SetScrollHdl( LINK( this, ImpSvMEdit, ScrollHdl ) );
    }
    if ( mpScrollBox )
        mpScrollBox->Show();

    ImpInitScrollBars();
}

ImpSvMEdit::~ImpSvMEdit()
{
    EndListening( *mpTextWindow->GetTextEngine() );
    delete mpTextWindow;
    delete mpHScrollBar;
    delete mpVScrollBar;
    delete mpScrollBox;
}

void ImpSvMEdit::ImpSetScrollBarRanges()
{
    if ( mpVScrollBar )
    {
        ULONG nTextHeight = mpTextWindow->GetTextEngine()->GetTextHeight();
        mpVScrollBar->SetRange( Range( 0, (long)nTextHeight ) );
    }
    if ( mpHScrollBar )
    {
//      ULONG nTextWidth = mpTextWindow->GetTextEngine()->CalcTextWidth();
        // Es gibt kein Notify bei Breiten-Aenderung...
//      ULONG nW = Max( (ULONG)mpTextWindow->GetOutputSizePixel().Width()*5, (ULONG)nTextWidth );
//      mpHScrollBar->SetRange( Range( 0, (long)nW ) );
        mpHScrollBar->SetRange( Range( 0, (long)mnTextWidth ) );
    }
}

void ImpSvMEdit::ImpInitScrollBars()
{
    static const sal_Unicode sampleText[] = { 'x', '\0' };
    if ( mpHScrollBar || mpVScrollBar )
    {
        ImpSetScrollBarRanges();
        Size aCharBox;
        aCharBox.Width() = mpTextWindow->GetTextWidth( sampleText );
        aCharBox.Height() = mpTextWindow->GetTextHeight();
        Size aOutSz = mpTextWindow->GetOutputSizePixel();
        if ( mpHScrollBar )
        {
            mpHScrollBar->SetVisibleSize( aOutSz.Width() );
            mpHScrollBar->SetPageSize( aOutSz.Width() * 8 / 10 );
            mpHScrollBar->SetLineSize( aCharBox.Width()*10 );
            mpHScrollBar->SetThumbPos( mpTextWindow->GetTextView()->GetStartDocPos().X() );
        }
        if ( mpVScrollBar )
        {
            mpVScrollBar->SetVisibleSize( aOutSz.Height() );
            mpVScrollBar->SetPageSize( aOutSz.Height() * 8 / 10 );
            mpVScrollBar->SetLineSize( aCharBox.Height() );
            mpVScrollBar->SetThumbPos( mpTextWindow->GetTextView()->GetStartDocPos().Y() );
        }
    }
}

IMPL_LINK( ImpSvMEdit, ScrollHdl, ScrollBar*, pCurScrollBar )
{
    long nDiffX = 0, nDiffY = 0;

    if ( pCurScrollBar == mpVScrollBar )
        nDiffY = mpTextWindow->GetTextView()->GetStartDocPos().Y() - pCurScrollBar->GetThumbPos();
    else if ( pCurScrollBar == mpHScrollBar )
        nDiffX = mpTextWindow->GetTextView()->GetStartDocPos().X() - pCurScrollBar->GetThumbPos();

    mpTextWindow->GetTextView()->Scroll( nDiffX, nDiffY );
    // mpTextWindow->GetTextView()->ShowCursor( FALSE, TRUE );

    return 0;
}


// void ImpSvMEdit::ImpModified()
// {
//  // Wann wird das gerufen ?????????????????????
//  pSvMultiLineEdit->Modify();
// }

void ImpSvMEdit::SetAlign( WinBits nWinStyle )
{
    if ( nWinStyle & WB_CENTER )
        mpTextWindow->GetTextEngine()->SetTextAlign( TXTALIGN_CENTER );
    else if ( nWinStyle & WB_RIGHT )
        mpTextWindow->GetTextEngine()->SetTextAlign( TXTALIGN_RIGHT );
    else
        mpTextWindow->GetTextEngine()->SetTextAlign( TXTALIGN_LEFT );
}

void ImpSvMEdit::SetTextWindowOffset( const Point& rOffset )
{
    maTextWindowOffset = rOffset;
    Resize();
}

void ImpSvMEdit::SetModified( BOOL bMod )
{
    mpTextWindow->GetTextEngine()->SetModified( bMod );
}

BOOL ImpSvMEdit::IsModified() const
{
    return mpTextWindow->GetTextEngine()->IsModified();
}

void ImpSvMEdit::SetInsertMode( BOOL bInsert )
{
    mpTextWindow->GetTextView()->SetInsertMode( bInsert );
}

void ImpSvMEdit::SetReadOnly( BOOL bRdOnly )
{
    mpTextWindow->GetTextView()->SetReadOnly( bRdOnly );
    // Farbe anpassen ???????????????????????????
}

BOOL ImpSvMEdit::IsReadOnly() const
{
    return mpTextWindow->GetTextView()->IsReadOnly();
}

void ImpSvMEdit::SetMaxTextLen( ULONG nLen )
{
    mpTextWindow->GetTextEngine()->SetMaxTextLen( nLen );
}

ULONG ImpSvMEdit::GetMaxTextLen() const
{
    return mpTextWindow->GetTextEngine()->GetMaxTextLen();
}

void ImpSvMEdit::InsertText( const String& rStr )
{
    mpTextWindow->GetTextView()->InsertText( rStr );
}

String ImpSvMEdit::GetSelected() const
{
    return mpTextWindow->GetTextView()->GetSelected();
}

void ImpSvMEdit::Resize()
{
    Size aSz = pSvMultiLineEdit->GetOutputSizePixel();
    long nSBWidth = pSvMultiLineEdit->GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = pSvMultiLineEdit->CalcZoom( nSBWidth );

    if ( mpHScrollBar )
        aSz.Height() -= nSBWidth;
    if ( mpVScrollBar )
        aSz.Width() -= nSBWidth;

    Size aTextWindowSz( aSz );
    aTextWindowSz.Width() -= maTextWindowOffset.X();
    aTextWindowSz.Height() -= maTextWindowOffset.Y();
    mpTextWindow->SetPosSizePixel( maTextWindowOffset, aTextWindowSz );

    if ( !mpHScrollBar )
        mpTextWindow->GetTextEngine()->SetMaxTextWidth( aSz.Width() );

    if ( mpHScrollBar )
        mpHScrollBar->SetPosSizePixel( 0, aSz.Height(), aSz.Width(), nSBWidth );

    if ( mpVScrollBar )
        mpVScrollBar->SetPosSizePixel( aSz.Width(), 0, nSBWidth, aSz.Height() );

    if ( mpScrollBox )
        mpScrollBox->SetPosSizePixel( aSz.Width(), aSz.Height(), nSBWidth, nSBWidth );

    ImpInitScrollBars();
}

void ImpSvMEdit::GetFocus()
{
    mpTextWindow->GrabFocus();
}

void ImpSvMEdit::Cut()
{
    if ( !mpTextWindow->GetTextView()->IsReadOnly() )
        mpTextWindow->GetTextView()->Cut();
}

void ImpSvMEdit::Copy()
{
    mpTextWindow->GetTextView()->Copy();
}

void ImpSvMEdit::Paste()
{
    if ( !mpTextWindow->GetTextView()->IsReadOnly() )
        mpTextWindow->GetTextView()->Paste();
}

void ImpSvMEdit::SetText( const String& rStr )
{
    BOOL bWasModified = mpTextWindow->GetTextEngine()->IsModified();
    mpTextWindow->GetTextEngine()->SetText( rStr );
    if ( !bWasModified )
        mpTextWindow->GetTextEngine()->SetModified( FALSE );

    mpTextWindow->GetTextView()->SetSelection( TextSelection() );
}

String ImpSvMEdit::GetText() const
{
    return mpTextWindow->GetTextEngine()->GetText();
}

String ImpSvMEdit::GetTextLines() const
{
    return mpTextWindow->GetTextEngine()->GetTextLines();
}

void ImpSvMEdit::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( TextHint ) )
    {
        const TextHint& rTextHint = (const TextHint&)rHint;
        if( rTextHint.GetId() == TEXT_HINT_VIEWSCROLLED )
        {
            if ( mpHScrollBar )
                mpHScrollBar->SetThumbPos( mpTextWindow->GetTextView()->GetStartDocPos().X() );
            if ( mpVScrollBar )
                mpVScrollBar->SetThumbPos( mpTextWindow->GetTextView()->GetStartDocPos().Y() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTHEIGHTCHANGED )
        {
            if ( mpTextWindow->GetTextView()->GetStartDocPos().Y() )
            {
                long nOutHeight = mpTextWindow->GetOutputSizePixel().Height();
                long nTextHeight = mpTextWindow->GetTextEngine()->GetTextHeight();
                if ( nTextHeight < nOutHeight )
                    mpTextWindow->GetTextView()->Scroll( 0, mpTextWindow->GetTextView()->GetStartDocPos().Y() );
            }

            ImpSetScrollBarRanges();
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTFORMATTED )
        {
            if ( mpHScrollBar )
            {
                ULONG nWidth = mpTextWindow->GetTextEngine()->CalcTextWidth();
                if ( nWidth != mnTextWidth )
                {
                    mnTextWidth = nWidth;
                    mpHScrollBar->SetRange( Range( 0, (long)mnTextWidth ) );
                    mpHScrollBar->SetThumbPos( mpTextWindow->GetTextView()->GetStartDocPos().X() );
                }
            }
        }
        else if( rTextHint.GetId() == TEXT_HINT_MODIFIED )
        {
            pSvMultiLineEdit->Modify();
        }
    }
}

void ImpSvMEdit::SetSelection( const Selection& rSelection )
{
    String aText = mpTextWindow->GetTextEngine()->GetText();

    Selection aNewSelection( rSelection );
    if ( aNewSelection.Min() < 0 )
        aNewSelection.Min() = 0;
    else if ( aNewSelection.Min() > aText.Len() )
        aNewSelection.Min() = aText.Len();
    if ( aNewSelection.Max() < 0 )
        aNewSelection.Max() = 0;
    else if ( aNewSelection.Max() > aText.Len() )
        aNewSelection.Max() = aText.Len();

    long nEnd = Max( aNewSelection.Min(), aNewSelection.Max() );
    TextSelection aTextSel;
    ULONG nPara = 0;
    USHORT nChar = 0;
    USHORT x = 0;
    while ( x <= nEnd )
    {
        if ( x == aNewSelection.Min() )
            aTextSel.GetStart() = TextPaM( nPara, nChar );
        if ( x == aNewSelection.Max() )
            aTextSel.GetEnd() = TextPaM( nPara, nChar );

        if ( ( x < aText.Len() ) && ( aText.GetChar( x ) == '\n' ) )
        {
            nPara++;
            nChar = 0;
        }
        else
            nChar++;
        x++;
    }
    mpTextWindow->GetTextView()->SetSelection( aTextSel );
}

Selection ImpSvMEdit::GetSelection() const
{
    Selection aSel;
    TextSelection aTextSel( mpTextWindow->GetTextView()->GetSelection() );
    aTextSel.Justify();
    // Selektion flachklopfen => jeder Umbruch ein Zeichen...

    ExtTextEngine* pExtTextEngine = mpTextWindow->GetTextEngine();
    // Absaetze davor:
    ULONG n;
    for ( n = 0; n < aTextSel.GetStart().GetPara(); n++ )
    {
        aSel.Min() += pExtTextEngine->GetTextLen( n );
        aSel.Min()++;
    }

    // Erster Absatz mit Selektion:
    aSel.Max() = aSel.Min();
    aSel.Min() += aTextSel.GetStart().GetIndex();

    for ( n = aTextSel.GetStart().GetPara(); n < aTextSel.GetEnd().GetPara(); n++ )
    {
        aSel.Max() += pExtTextEngine->GetTextLen( n );
        aSel.Max()++;
    }

    aSel.Max() += aTextSel.GetEnd().GetIndex();

    return aSel;
}

Size ImpSvMEdit::CalcMinimumSize() const
{
    Size aSz(   mpTextWindow->GetTextEngine()->CalcTextWidth(),
                mpTextWindow->GetTextEngine()->GetTextHeight() );

    if ( mpHScrollBar )
        aSz.Height() += mpHScrollBar->GetSizePixel().Height();
    if ( mpVScrollBar )
        aSz.Width() += mpVScrollBar->GetSizePixel().Width();

    return aSz;
}

Size ImpSvMEdit::CalcSize( USHORT nColumns, USHORT nLines ) const
{
    static const sal_Unicode sampleText[] = { 'X', '\0' };

    Size aSz;
    Size aCharSz;
    aCharSz.Width() = mpTextWindow->GetTextWidth( sampleText );
    aCharSz.Height() = mpTextWindow->GetTextHeight();

    if ( nLines )
        aSz.Height() = nLines*aCharSz.Height();
    else
        aSz.Height() = mpTextWindow->GetTextEngine()->GetTextHeight();

    if ( nColumns )
        aSz.Width() = nColumns*aCharSz.Width();
    else
        aSz.Width() = mpTextWindow->GetTextEngine()->CalcTextWidth();

    if ( mpHScrollBar )
        aSz.Height() += mpHScrollBar->GetSizePixel().Height();
    if ( mpVScrollBar )
        aSz.Width() += mpVScrollBar->GetSizePixel().Width();

    return aSz;
}

void ImpSvMEdit::GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const
{
    static const sal_Unicode sampleText[] = { 'x', '\0' };
    Size aOutSz = mpTextWindow->GetOutputSizePixel();
    Size aCharSz( mpTextWindow->GetTextWidth( sampleText ), mpTextWindow->GetTextHeight() );
    rnCols = (USHORT) (aOutSz.Width()/aCharSz.Width());
    rnLines = (USHORT) (aOutSz.Height()/aCharSz.Height());
}

void ImpSvMEdit::Enable( BOOL bEnable )
{
    mpTextWindow->Enable( bEnable );
    if ( mpHScrollBar )
        mpHScrollBar->Enable( bEnable );
    if ( mpVScrollBar )
        mpVScrollBar->Enable( bEnable );
}

BOOL ImpSvMEdit::HandleCommand( const CommandEvent& rCEvt )
{
    BOOL bDone = FALSE;
    if ( ( rCEvt.GetCommand() == COMMAND_WHEEL ) ||
         ( rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL ) ||
         ( rCEvt.GetCommand() == COMMAND_AUTOSCROLL ) )
    {
        mpTextWindow->HandleScrollCommand( rCEvt, mpHScrollBar, mpVScrollBar );
        bDone = TRUE;
    }
    return bDone;
}


TextWindow::TextWindow( Window* pParent ) : Window( pParent )
{
    mbInMBDown = FALSE;
    mbFocusSelectionHide = FALSE;
    mbIgnoreTab = FALSE;
    mbActivePopup = FALSE;

    SetPointer( Pointer( POINTER_TEXT ) );

    mpExtTextEngine = new ExtTextEngine;
    mpExtTextEngine->SetMaxTextLen( STRING_MAXLEN );
    mpExtTextView = new ExtTextView( mpExtTextEngine, this );
    mpExtTextEngine->InsertView( mpExtTextView );
    mpExtTextEngine->EnableUndo( TRUE );
    mpExtTextView->ShowCursor();

    Color aBackgroundColor = GetSettings().GetStyleSettings().GetWorkspaceColor();
    SetBackground( aBackgroundColor );
    pParent->SetBackground( aBackgroundColor );
    EnableDrop( TRUE );
}

TextWindow::~TextWindow()
{
    delete mpExtTextView;
    delete mpExtTextEngine;
}

void TextWindow::MouseMove( const MouseEvent& rMEvt )
{
    mpExtTextView->MouseMove( rMEvt );
    Window::MouseMove( rMEvt );
}

void TextWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    mbInMBDown = TRUE;  // Dann im GetFocus nicht alles selektieren wird
    mpExtTextView->MouseButtonDown( rMEvt );
    Window::MouseButtonDown( rMEvt );
    GrabFocus();
    mbInMBDown = FALSE;
}

void TextWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    mpExtTextView->MouseButtonUp( rMEvt );
    Window::MouseButtonUp( rMEvt );
}

void TextWindow::KeyInput( const KeyEvent& rKEvent )
{
    BOOL bDone = FALSE;
    USHORT nCode = rKEvent.GetKeyCode().GetCode();
    if ( (nCode == KEY_A) && rKEvent.GetKeyCode().IsMod1() )
    {
        mpExtTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFF, 0xFFFF ) ) );
        bDone = TRUE;
    }
    else if ( (nCode == KEY_S) && rKEvent.GetKeyCode().IsShift() && rKEvent.GetKeyCode().IsMod1() )
    {
        if ( Edit::GetGetSpecialCharsFunction() )
        {
            // Damit die Selektion erhalten bleibt
            mbActivePopup = TRUE;
            XubString aChars = Edit::GetGetSpecialCharsFunction()( this, GetFont() );
            if ( aChars.Len() )
            {
                mpExtTextView->InsertText( aChars );
                mpExtTextView->GetTextEngine()->SetModified( TRUE );
            }
            mbActivePopup = FALSE;
            bDone = TRUE;
        }
    }
    else if ( nCode == KEY_TAB )
    {
        if ( !mbIgnoreTab || rKEvent.GetKeyCode().IsMod1() )
            bDone = mpExtTextView->KeyInput( rKEvent  );
    }
    else
    {
        bDone = mpExtTextView->KeyInput( rKEvent  );
    }

    if ( !bDone )
        Window::KeyInput( rKEvent );
}

void TextWindow::Paint( const Rectangle& rRect )
{
    mpExtTextView->Paint( rRect );
}

void TextWindow::Resize()
{
}

void TextWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        PopupMenu* pPopup = Edit::CreatePopupMenu();
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_HIDEDISABLED )
            pPopup->SetMenuFlags( MENU_FLAG_HIDEDISABLEDENTRIES );
        if ( !mpExtTextView->HasSelection() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_COPY, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, FALSE );
        }
        if ( mpExtTextView->IsReadOnly() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, FALSE );
            pPopup->EnableItem( SV_MENU_EDIT_INSERTSYMBOL, FALSE );
        }
        if ( !mpExtTextView->GetTextEngine()->HasUndoManager() || !mpExtTextView->GetTextEngine()->GetUndoManager().GetUndoActionCount() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_UNDO, FALSE );
        }
//      if ( ( maSelection.Min() == 0 ) && ( maSelection.Max() == maText.Len() ) )
//      {
//          pPopup->EnableItem( SV_MENU_EDIT_SELECTALL, FALSE );
//      }
        if ( !Edit::GetGetSpecialCharsFunction() )
        {
            USHORT nPos = pPopup->GetItemPos( SV_MENU_EDIT_INSERTSYMBOL );
            pPopup->RemoveItem( nPos );
            pPopup->RemoveItem( nPos-1 );
        }

        mbActivePopup = TRUE;
        Point aPos = rCEvt.GetMousePosPixel();
        if ( !rCEvt.IsMouseEvent() )
        {
            // !!! Irgendwann einmal Menu zentriert in der Selektion anzeigen !!!
            Size aSize = GetOutputSizePixel();
            aPos = Point( aSize.Width()/2, aSize.Height()/2 );
        }
//      pPopup->RemoveDisabledEntries();
        USHORT n = pPopup->Execute( this, aPos );
        Edit::DeletePopupMenu( pPopup );
        switch ( n )
        {
            case SV_MENU_EDIT_UNDO:     mpExtTextView->Undo();
                                        mpExtTextEngine->SetModified( TRUE );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_CUT:      mpExtTextView->Cut();
                                        mpExtTextEngine->SetModified( TRUE );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_COPY:     mpExtTextView->Copy();
                                        break;
            case SV_MENU_EDIT_PASTE:    mpExtTextView->Paste();
                                        mpExtTextEngine->SetModified( TRUE );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_DELETE:   mpExtTextView->DeleteSelected();
                                        mpExtTextEngine->SetModified( TRUE );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_SELECTALL:    mpExtTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
                                            break;
            case SV_MENU_EDIT_INSERTSYMBOL:
                {
                    XubString aChars = Edit::GetGetSpecialCharsFunction()( this, GetFont() );
                    if ( aChars.Len() )
                    {
                        mpExtTextView->InsertText( aChars );
                        mpExtTextEngine->SetModified( TRUE );
                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                    }
                }
                break;
        }
        mbActivePopup = FALSE;
    }
    else
    {
        mpExtTextView->Command( rCEvt );
    }
    Window::Command( rCEvt );
}

void TextWindow::GetFocus()
{
    Window::GetFocus();
    if ( !mbActivePopup )
    {
        BOOL bGotoCursor = !mpExtTextView->IsReadOnly();
        if ( mbFocusSelectionHide && IsReallyVisible() && !mpExtTextView->IsReadOnly()
                && ( !mbInMBDown || ( GetSettings().GetStyleSettings().GetSelectionOptions() & SELECTION_OPTION_FOCUS ) ) )
        {
            // Alles selektieren, aber nicht scrollen
            BOOL bAutoScroll = mpExtTextView->IsAutoScroll();
            mpExtTextView->SetAutoScroll( FALSE );
            mpExtTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFF, 0xFFFF ) ) );
            mpExtTextView->SetAutoScroll( bAutoScroll );
            bGotoCursor = FALSE;
        }
        mpExtTextView->SetPaintSelection( TRUE );
        mpExtTextView->ShowCursor( bGotoCursor );
        SetInputContext( InputContext( GetFont(), !mpExtTextView->IsReadOnly() ? INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT : 0 ) );
    }
}

void TextWindow::LoseFocus()
{
    Window::LoseFocus();

    if ( mbFocusSelectionHide && !mbActivePopup )
        mpExtTextView->SetPaintSelection( FALSE );
}

BOOL TextWindow::QueryDrop( DropEvent& rEvt )
{
    return mpExtTextView->QueryDrop( rEvt );
}

BOOL TextWindow::Drop( const DropEvent& rEvt )
{
    return mpExtTextView->Drop( rEvt );
}


MultiLineEdit::MultiLineEdit( Window* pParent, WinBits nWinStyle )
    : Edit( pParent, nWinStyle )
{
    SetType( WINDOW_MULTILINEEDIT );
    pImpSvMEdit = new ImpSvMEdit( this, nWinStyle );
    ImplInitSettings( TRUE, TRUE, TRUE );
    pUpdateDataTimer = 0;

    SetCompoundControl( TRUE );
    SetStyle( ImplInitStyle( nWinStyle ) );
}

MultiLineEdit::MultiLineEdit( Window* pParent, const ResId& rResId )
    : Edit( pParent, rResId.SetRT( RSC_MULTILINEEDIT ) )
{
    SetType( WINDOW_MULTILINEEDIT );
    WinBits nWinStyle = rResId.aWinBits;
    pImpSvMEdit = new ImpSvMEdit( this, nWinStyle );
    ImplInitSettings( TRUE, TRUE, TRUE );
    pUpdateDataTimer = 0;

    USHORT nMaxLen = Edit::GetMaxTextLen();
    if ( nMaxLen )
        SetMaxTextLen( nMaxLen );

    SetText( Edit::GetText() );

    if ( IsVisible() )
        pImpSvMEdit->Resize();

    SetCompoundControl( TRUE );
    SetStyle( ImplInitStyle( nWinStyle ) );
}

MultiLineEdit::~MultiLineEdit()
{
    delete pImpSvMEdit;
    delete pUpdateDataTimer;
}

WinBits MultiLineEdit::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    if ( !(nStyle & WB_IGNORETAB ))
        nStyle |= WINDOW_DLGCTRL_MOD1TAB;

    return nStyle;
}


void MultiLineEdit::ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // Der Font muss immer mit manipuliert werden, weil die TextEngine
    // sich nicht um TextColor/Background kuemmert

    Color aTextColor = rStyleSettings.GetFieldTextColor();
    if ( IsControlForeground() )
        aTextColor = GetControlForeground();

    Font aFont = rStyleSettings.GetFieldFont();
    if ( IsControlFont() )
        aFont.Merge( GetControlFont() );
    aFont.SetTransparent( FALSE );
    SetZoomedPointFont( aFont );
    Font TheFont = GetFont();
    TheFont.SetColor( aTextColor );
    TheFont.SetFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );
    pImpSvMEdit->GetTextWindow()->SetFont( TheFont );
    pImpSvMEdit->GetTextWindow()->GetTextEngine()->SetFont( TheFont );
    pImpSvMEdit->GetTextWindow()->SetTextColor( aTextColor );

    if ( bBackground )
    {
        if( IsControlBackground() )
            pImpSvMEdit->GetTextWindow()->SetBackground( GetControlBackground() );
        else
            pImpSvMEdit->GetTextWindow()->SetBackground( rStyleSettings.GetFieldColor() );
        // Auch am MultiLineEdit einstellen, weil die TextComponent
        // ggf. die Scrollbars hidet.
        SetBackground( pImpSvMEdit->GetTextWindow()->GetBackground() );
    }
}

void MultiLineEdit::Modify()
{
    aModifyHdlLink.Call( this );
    if ( pUpdateDataTimer )
        pUpdateDataTimer->Start();
}

IMPL_LINK( MultiLineEdit, ImpUpdateDataHdl, Timer*, EMPTYARG )
{
    UpdateData();
    return 0;
}

void MultiLineEdit::UpdateData()
{
    aUpdateDataHdlLink.Call( this );
}

void MultiLineEdit::SetModifyFlag()
{
    pImpSvMEdit->SetModified( TRUE );
}

void MultiLineEdit::ClearModifyFlag()
{
    pImpSvMEdit->SetModified( TRUE );
}

BOOL MultiLineEdit::IsModified() const
{
    return pImpSvMEdit->IsModified();
}

void MultiLineEdit::EnableUpdateData( ULONG nTimeout )
{
    if ( !nTimeout )
        DisableUpdateData();
    else
    {
        if ( !pUpdateDataTimer )
        {
            pUpdateDataTimer = new Timer;
            pUpdateDataTimer->SetTimeoutHdl( LINK( this, MultiLineEdit, ImpUpdateDataHdl ) );
        }
        pUpdateDataTimer->SetTimeout( nTimeout );
    }
}

void MultiLineEdit::SetReadOnly( BOOL bReadOnly )
{
    pImpSvMEdit->SetReadOnly( bReadOnly );
    Edit::SetReadOnly( bReadOnly );
}

BOOL MultiLineEdit::IsReadOnly() const
{
    return pImpSvMEdit->IsReadOnly();
}

void MultiLineEdit::SetMaxTextLen( ULONG nMaxLen )
{
    pImpSvMEdit->SetMaxTextLen( nMaxLen );
}

ULONG MultiLineEdit::GetMaxTextLen() const
{
    return pImpSvMEdit->GetMaxTextLen();
}

void MultiLineEdit::ReplaceSelected( const String& rStr )
{
    pImpSvMEdit->InsertText( rStr );
}

void MultiLineEdit::DeleteSelected()
{
    pImpSvMEdit->InsertText( String() );
}

String MultiLineEdit::GetSelected() const
{
    return pImpSvMEdit->GetSelected();
}

void MultiLineEdit::Cut()
{
    pImpSvMEdit->Cut();
}

void MultiLineEdit::Copy()
{
    pImpSvMEdit->Copy();
}

void MultiLineEdit::Paste()
{
    pImpSvMEdit->Paste();
}

void MultiLineEdit::SetText( const String& rStr )
{
    pImpSvMEdit->SetText( rStr );
}

String MultiLineEdit::GetText() const
{
    return pImpSvMEdit->GetText();
}

String MultiLineEdit::GetTextLines() const
{
    return pImpSvMEdit->GetTextLines();
}

void MultiLineEdit::Resize()
{
    pImpSvMEdit->Resize();
}

void MultiLineEdit::GetFocus()
{
    Edit::GetFocus();
    pImpSvMEdit->GetFocus();
}

void MultiLineEdit::SetSelection( const Selection& rSelection )
{
    pImpSvMEdit->SetSelection( rSelection );
}

Selection MultiLineEdit::GetSelection() const
{
    return pImpSvMEdit->GetSelection();
}

Size MultiLineEdit::CalcMinimumSize() const
{
    Size aSz = pImpSvMEdit->CalcMinimumSize();

    long nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Width() += nLeft+nRight;
    aSz.Height() += nTop+nBottom;

    return aSz;
}

Size MultiLineEdit::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    long nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );

    // In der Hoehe auf ganze Zeilen justieren

    long nHeight = aSz.Height() - nTop - nBottom;
    long nLineHeight = pImpSvMEdit->CalcSize( 1, 1 ).Height();
    long nLines = nHeight / nLineHeight;
    if ( nLines < 1 )
        nLines = 1;

    aSz.Height() = nLines * nLineHeight;
    aSz.Height() += nTop+nBottom;

    return aSz;
}

Size MultiLineEdit::CalcSize( USHORT nColumns, USHORT nLines ) const
{
    Size aSz = pImpSvMEdit->CalcSize( nColumns, nLines );

    long nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Width() += nLeft+nRight;
    aSz.Height() += nTop+nBottom;
    return aSz;
}

void MultiLineEdit::GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const
{
    pImpSvMEdit->GetMaxVisColumnsAndLines( rnCols, rnLines );
}

void MultiLineEdit::StateChanged( StateChangedType nType )
{
    if( nType == STATE_CHANGE_ENABLE )
    {
        pImpSvMEdit->Enable( IsEnabled() );
    }
    else if( nType == STATE_CHANGE_READONLY )
    {
        pImpSvMEdit->SetReadOnly( IsReadOnly() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        pImpSvMEdit->GetTextWindow()->SetZoom( GetZoom() );
        ImplInitSettings( TRUE, FALSE, FALSE );
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        ImplInitSettings( TRUE, FALSE, FALSE );
        Resize();
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        pImpSvMEdit->SetAlign( GetStyle() );
        SetStyle( ImplInitStyle( GetStyle() ) );
    }

    Control::StateChanged( nType );
}

void MultiLineEdit::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( TRUE, TRUE, TRUE );
        Resize();
        Invalidate();
    }
    else
        Control::DataChanged( rDCEvt );
}

void MultiLineEdit::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    ImplInitSettings( TRUE, TRUE, TRUE );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = pImpSvMEdit->GetTextWindow()->GetDrawPixelFont( pDev );
    aFont.SetTransparent( TRUE );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    BOOL bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    BOOL bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
    if ( bBorder || bBackground )
    {
        Rectangle aRect( aPos, aSize );
        if ( bBorder )
        {
            DecorationView aDecoView( pDev );
            aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    // Inhalt
    if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
    {
        if ( !(nFlags & WINDOW_DRAW_NODISABLE ) && !IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pDev->SetTextColor( rStyleSettings.GetDisableColor() );
        }
        else
        {
            pDev->SetTextColor( GetTextColor() );
        }
    }

    XubString aText = GetText();
    Size aTextSz( pDev->GetTextWidth( aText ), pDev->GetTextHeight() );
    ULONG nLines = (ULONG) (aSize.Height() / aTextSz.Height());
    if ( !nLines )
        nLines = 1;
    aTextSz.Height() = nLines*aTextSz.Height();
    long nOnePixel = GetDrawPixel( pDev, 1 );
    long nOffX = 3*nOnePixel;
    long nOffY = 2*nOnePixel;

    // Clipping?
    if ( ( nOffY < 0  ) || ( (nOffY+aTextSz.Height()) > aSize.Height() ) || ( (nOffX+aTextSz.Width()) > aSize.Width() ) )
    {
        Rectangle aClip( aPos, aSize );
        if ( aTextSz.Height() > aSize.Height() )
            aClip.Bottom() += aTextSz.Height() - aSize.Height() + 1;  // Damit HP-Drucker nicht 'weg-optimieren'
        pDev->IntersectClipRegion( aClip );
    }

    TextEngine aTE;
    aTE.SetText( GetText() );
    aTE.SetMaxTextWidth( aSize.Width() );
    aTE.SetFont( aFont );
    aTE.SetTextAlign( pImpSvMEdit->GetTextWindow()->GetTextEngine()->GetTextAlign() );
    aTE.Draw( pDev, Point( aPos.X() + nOffX, aPos.Y() + nOffY ) );

    pDev->Pop();
}

long MultiLineEdit::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        nDone = pImpSvMEdit->HandleCommand( *rNEvt.GetCommandEvent() );
    }
    return nDone ? nDone : Edit::Notify( rNEvt );
}

long MultiLineEdit::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;

    if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && ( !GetTextView()->IsCursorEnabled() ) )
    {
        const KeyEvent& rKEvent = *rNEvt.GetKeyEvent();
        if ( !rKEvent.GetKeyCode().IsShift() && ( rKEvent.GetKeyCode().GetGroup() == KEYGROUP_CURSOR ) )
        {
            nDone = 1;
            TextSelection aSel = pImpSvMEdit->GetTextWindow()->GetTextView()->GetSelection();
            if ( aSel.HasRange() )
            {
                aSel.GetStart() = aSel.GetEnd();
                pImpSvMEdit->GetTextWindow()->GetTextView()->SetSelection( aSel );
            }
            else
            {
                switch ( rKEvent.GetKeyCode().GetCode() )
                {
                    case KEY_UP:
                    {
                        if ( pImpSvMEdit->GetVScrollBar() )
                            pImpSvMEdit->GetVScrollBar()->DoScrollAction( SCROLL_LINEUP );
                    }
                    break;
                    case KEY_DOWN:
                    {
                        if ( pImpSvMEdit->GetVScrollBar() )
                            pImpSvMEdit->GetVScrollBar()->DoScrollAction( SCROLL_LINEDOWN );
                    }
                    break;
                    case KEY_PAGEUP :
                    {
                        if ( pImpSvMEdit->GetVScrollBar() )
                            pImpSvMEdit->GetVScrollBar()->DoScrollAction( SCROLL_PAGEUP );
                    }
                    break;
                    case KEY_PAGEDOWN:
                    {
                        if ( pImpSvMEdit->GetVScrollBar() )
                            pImpSvMEdit->GetVScrollBar()->DoScrollAction( SCROLL_PAGEDOWN );
                    }
                    break;
                    case KEY_LEFT:
                    {
                        if ( pImpSvMEdit->GetHScrollBar() )
                            pImpSvMEdit->GetHScrollBar()->DoScrollAction( SCROLL_LINEUP );
                    }
                    break;
                    case KEY_RIGHT:
                    {
                        if ( pImpSvMEdit->GetHScrollBar() )
                            pImpSvMEdit->GetHScrollBar()->DoScrollAction( SCROLL_LINEDOWN );
                    }
                    break;
                    case KEY_HOME:
                    {
                        if ( rKEvent.GetKeyCode().IsMod1() )
                            pImpSvMEdit->GetTextWindow()->GetTextView()->
                                SetSelection( TextSelection( TextPaM( 0, 0 ) ) );
                    }
                    break;
                    case KEY_END:
                    {
                        if ( rKEvent.GetKeyCode().IsMod1() )
                            pImpSvMEdit->GetTextWindow()->GetTextView()->
                                SetSelection( TextSelection( TextPaM( 0xFFFF, 0xFFFF ) ) );
                    }
                    break;
                    default:
                    {
                        nDone = 0;
                    }
                }
            }
        }
    }

    return nDone ? nDone : Edit::PreNotify( rNEvt );
}

//
// Internas fuer abgeleitete Klassen, z.B. TextComponent

ExtTextEngine* MultiLineEdit::GetTextEngine() const
{
    return pImpSvMEdit->GetTextWindow()->GetTextEngine();
}

ExtTextView* MultiLineEdit::GetTextView() const
{
    return pImpSvMEdit->GetTextWindow()->GetTextView();
}

ScrollBar* MultiLineEdit::GetHScrollBar() const
{
    return pImpSvMEdit->GetHScrollBar();
}


ScrollBar* MultiLineEdit::GetVScrollBar() const
{
    return pImpSvMEdit->GetVScrollBar();
}

void MultiLineEdit::EnableFocusSelectionHide( BOOL bHide )
{
    pImpSvMEdit->GetTextWindow()->SetAutoFocusHide( bHide );
}

BOOL MultiLineEdit::IsFocusSelectionHideEnabled() const
{
    return pImpSvMEdit->GetTextWindow()->IsAutoFocusHide();
}
