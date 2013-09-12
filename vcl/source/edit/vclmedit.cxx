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

#include <memory>

#include <tools/rc.h>
#include <vcl/builder.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>

#include <vcl/vclmedit.hxx>
#include <vcl/xtextedt.hxx>
#include <svl/brdcst.hxx>
#include <svl/undo.hxx>
#include <svl/lstner.hxx>
#include <svl/smplhint.hxx>

#include <svids.hrc>
#include <vcl/scrbar.hxx>


class TextWindow : public Window
{
private:
    ExtTextEngine*  mpExtTextEngine;
    ExtTextView*    mpExtTextView;

    bool            mbInMBDown;
    bool            mbFocusSelectionHide;
    bool            mbIgnoreTab;
    bool            mbActivePopup;
    bool            mbSelectOnTab;
    bool            mbTextSelectable;

public:
                    TextWindow( Window* pParent );
                    ~TextWindow();

    ExtTextEngine*  GetTextEngine() const { return mpExtTextEngine; }
    ExtTextView*    GetTextView() const { return mpExtTextView; }

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvent );

    virtual void    Command( const CommandEvent& rCEvt );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();

    virtual void    GetFocus();
    virtual void    LoseFocus();

    bool            IsAutoFocusHide() const { return mbFocusSelectionHide; }
    void            SetAutoFocusHide( bool bAutoHide ) { mbFocusSelectionHide = bAutoHide; }

    bool            IsIgnoreTab() const { return mbIgnoreTab; }
    void            SetIgnoreTab( bool bIgnore ) { mbIgnoreTab = bIgnore; }

    void            DisableSelectionOnFocus() { mbSelectOnTab = false; }

    void            SetTextSelectable( bool bTextSelectable ) { mbTextSelectable = bTextSelectable; }
};


class ImpVclMEdit : public SfxListener
{
private:
    VclMultiLineEdit*   pVclMultiLineEdit;

    TextWindow*         mpTextWindow;
    ScrollBar*          mpHScrollBar;
    ScrollBar*          mpVScrollBar;
    ScrollBarBox*       mpScrollBox;

    Point               maTextWindowOffset;
    xub_StrLen          mnTextWidth;
    mutable Selection   maSelection;

protected:
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    void                ImpUpdateSrollBarVis( WinBits nWinStyle );
    void                ImpInitScrollBars();
    void                ImpSetScrollBarRanges();
    void                ImpSetHScrollBarThumbPos();
    DECL_LINK(          ScrollHdl, ScrollBar* );

public:
                ImpVclMEdit( VclMultiLineEdit* pVclMultiLineEdit, WinBits nWinStyle );
                ~ImpVclMEdit();

    void        SetModified( sal_Bool bMod );
    sal_Bool        IsModified() const;

    void        SetReadOnly( sal_Bool bRdOnly );
    sal_Bool        IsReadOnly() const;

    void        SetMaxTextLen( xub_StrLen nLen );
    xub_StrLen  GetMaxTextLen() const;

    void        SetMaxTextWidth( sal_uLong nMaxWidth );

    sal_Bool        IsInsertMode() const;

    void        InsertText( const OUString& rStr );
    OUString    GetSelected() const;
    OUString    GetSelected( LineEnd aSeparator ) const;

    void        SetSelection( const Selection& rSelection );
    const Selection& GetSelection() const;

    void        Cut();
    void        Copy();
    void        Paste();

    void        SetText( const OUString& rStr );
    OUString    GetText() const;
    OUString    GetText( LineEnd aSeparator ) const;
    OUString    GetTextLines( LineEnd aSeparator ) const;

    void        Resize();
    void        GetFocus();

    sal_Bool        HandleCommand( const CommandEvent& rCEvt );

    void        Enable( sal_Bool bEnable );

    Size        CalcMinimumSize() const;
    Size        CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const;
    void        GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const;

    void        SetAlign( WinBits nWinStyle );

    void        InitFromStyle( WinBits nWinStyle );

    TextWindow* GetTextWindow() { return mpTextWindow; }
    ScrollBar*  GetHScrollBar() { return mpHScrollBar; }
    ScrollBar*  GetVScrollBar() { return mpVScrollBar; }
};

ImpVclMEdit::ImpVclMEdit( VclMultiLineEdit* pEdt, WinBits nWinStyle )
    :mpHScrollBar(NULL)
    ,mpVScrollBar(NULL)
    ,mpScrollBox(NULL)
{
    pVclMultiLineEdit = pEdt;
    mnTextWidth = 0;
    mpTextWindow = new TextWindow( pEdt );
    mpTextWindow->Show();
    InitFromStyle( nWinStyle );
    StartListening( *mpTextWindow->GetTextEngine() );
}

void ImpVclMEdit::ImpUpdateSrollBarVis( WinBits nWinStyle )
{
    const bool bHaveVScroll = (NULL != mpVScrollBar);
    const bool bHaveHScroll = (NULL != mpHScrollBar);
    const bool bHaveScrollBox = (NULL != mpScrollBox);

          bool bNeedVScroll = ( nWinStyle & WB_VSCROLL ) == WB_VSCROLL;
    const bool bNeedHScroll = ( nWinStyle & WB_HSCROLL ) == WB_HSCROLL;

    const bool bAutoVScroll = ( nWinStyle & WB_AUTOVSCROLL ) == WB_AUTOVSCROLL;
    if ( !bNeedVScroll && bAutoVScroll )
    {
        TextEngine& rEngine( *mpTextWindow->GetTextEngine() );
        sal_uLong nOverallTextHeight(0);
        for ( sal_uLong i=0; i<rEngine.GetParagraphCount(); ++i )
            nOverallTextHeight += rEngine.GetTextHeight( i );
        if ( nOverallTextHeight > (sal_uLong)mpTextWindow->GetOutputSizePixel().Height() )
            bNeedVScroll = true;
    }

    const bool bNeedScrollBox = bNeedVScroll && bNeedHScroll;

    bool bScrollbarsChanged = false;
    if ( bHaveVScroll != bNeedVScroll )
    {
        delete mpVScrollBar;
        mpVScrollBar = bNeedVScroll ? new ScrollBar( pVclMultiLineEdit, WB_VSCROLL|WB_DRAG ) : NULL;

        if ( bNeedVScroll )
        {
            mpVScrollBar->Show();
            mpVScrollBar->SetScrollHdl( LINK( this, ImpVclMEdit, ScrollHdl ) );
        }

        bScrollbarsChanged = true;
    }

    if ( bHaveHScroll != bNeedHScroll )
    {
        delete mpHScrollBar;
        mpHScrollBar = bNeedHScroll ? new ScrollBar( pVclMultiLineEdit, WB_HSCROLL|WB_DRAG ) : NULL;

        if ( bNeedHScroll )
        {
            mpHScrollBar->Show();
            mpHScrollBar->SetScrollHdl( LINK( this, ImpVclMEdit, ScrollHdl ) );
        }

        bScrollbarsChanged = true;
    }

    if ( bHaveScrollBox != bNeedScrollBox )
    {
        delete mpScrollBox;
        mpScrollBox = bNeedScrollBox ? new ScrollBarBox( pVclMultiLineEdit, WB_SIZEABLE ) : NULL;

        if ( bNeedScrollBox )
            mpScrollBox->Show();
    }

    if ( bScrollbarsChanged )
    {
        ImpInitScrollBars();
        Resize();
    }
}

void ImpVclMEdit::InitFromStyle( WinBits nWinStyle )
{
    ImpUpdateSrollBarVis( nWinStyle );
    SetAlign( nWinStyle );

    if ( nWinStyle & WB_NOHIDESELECTION )
        mpTextWindow->SetAutoFocusHide( sal_False );
    else
        mpTextWindow->SetAutoFocusHide( sal_True );

    if ( nWinStyle & WB_READONLY )
        mpTextWindow->GetTextView()->SetReadOnly( sal_True );
    else
        mpTextWindow->GetTextView()->SetReadOnly( sal_False );

    if ( nWinStyle & WB_IGNORETAB )
    {
        mpTextWindow->SetIgnoreTab( true );
    }
    else
    {
        mpTextWindow->SetIgnoreTab( false );
        // #103667# VclMultiLineEdit has the flag, but focusable window also needs this flag
        WinBits nStyle = mpTextWindow->GetStyle();
        nStyle |= WINDOW_DLGCTRL_MOD1TAB;
        mpTextWindow->SetStyle( nStyle );
    }
}

ImpVclMEdit::~ImpVclMEdit()
{
    EndListening( *mpTextWindow->GetTextEngine() );
    delete mpTextWindow;
    delete mpHScrollBar;
    delete mpVScrollBar;
    delete mpScrollBox;
}

void ImpVclMEdit::ImpSetScrollBarRanges()
{
    if ( mpVScrollBar )
    {
        sal_uLong nTextHeight = mpTextWindow->GetTextEngine()->GetTextHeight();
        mpVScrollBar->SetRange( Range( 0, (long)nTextHeight-1 ) );
    }
    if ( mpHScrollBar )
    {
//      sal_uLong nTextWidth = mpTextWindow->GetTextEngine()->CalcTextWidth();
        // Es gibt kein Notify bei Breiten-Aenderung...
//      sal_uLong nW = Max( (sal_uLong)mpTextWindow->GetOutputSizePixel().Width()*5, (sal_uLong)nTextWidth );
//      mpHScrollBar->SetRange( Range( 0, (long)nW ) );
        mpHScrollBar->SetRange( Range( 0, (long)mnTextWidth-1 ) );
    }
}

void ImpVclMEdit::ImpInitScrollBars()
{
    static const sal_Unicode sampleChar = { 'x' };
    if ( mpHScrollBar || mpVScrollBar )
    {
        ImpSetScrollBarRanges();
        Size aCharBox;
        aCharBox.Width() = mpTextWindow->GetTextWidth( OUString(sampleChar) );
        aCharBox.Height() = mpTextWindow->GetTextHeight();
        Size aOutSz = mpTextWindow->GetOutputSizePixel();
        if ( mpHScrollBar )
        {
            mpHScrollBar->SetVisibleSize( aOutSz.Width() );
            mpHScrollBar->SetPageSize( aOutSz.Width() * 8 / 10 );
            mpHScrollBar->SetLineSize( aCharBox.Width()*10 );
            ImpSetHScrollBarThumbPos();
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

void ImpVclMEdit::ImpSetHScrollBarThumbPos()
{
    long nX = mpTextWindow->GetTextView()->GetStartDocPos().X();
    if ( !mpTextWindow->GetTextEngine()->IsRightToLeft() )
        mpHScrollBar->SetThumbPos( nX );
    else
        mpHScrollBar->SetThumbPos( mnTextWidth - mpHScrollBar->GetVisibleSize() - nX );

}

IMPL_LINK( ImpVclMEdit, ScrollHdl, ScrollBar*, pCurScrollBar )
{
    long nDiffX = 0, nDiffY = 0;

    if ( pCurScrollBar == mpVScrollBar )
        nDiffY = mpTextWindow->GetTextView()->GetStartDocPos().Y() - pCurScrollBar->GetThumbPos();
    else if ( pCurScrollBar == mpHScrollBar )
        nDiffX = mpTextWindow->GetTextView()->GetStartDocPos().X() - pCurScrollBar->GetThumbPos();

    mpTextWindow->GetTextView()->Scroll( nDiffX, nDiffY );
    // mpTextWindow->GetTextView()->ShowCursor( sal_False, sal_True );

    return 0;
}

void ImpVclMEdit::SetAlign( WinBits nWinStyle )
{
    sal_Bool bRTL = Application::GetSettings().GetLayoutRTL();
    mpTextWindow->GetTextEngine()->SetRightToLeft( bRTL );

    if ( nWinStyle & WB_CENTER )
        mpTextWindow->GetTextEngine()->SetTextAlign( TXTALIGN_CENTER );
    else if ( nWinStyle & WB_RIGHT )
        mpTextWindow->GetTextEngine()->SetTextAlign( !bRTL ? TXTALIGN_RIGHT : TXTALIGN_LEFT );
    else if ( nWinStyle & WB_LEFT )
        mpTextWindow->GetTextEngine()->SetTextAlign( !bRTL ? TXTALIGN_LEFT : TXTALIGN_RIGHT );
}

void ImpVclMEdit::SetModified( sal_Bool bMod )
{
    mpTextWindow->GetTextEngine()->SetModified( bMod );
}

sal_Bool ImpVclMEdit::IsModified() const
{
    return mpTextWindow->GetTextEngine()->IsModified();
}

void ImpVclMEdit::SetReadOnly( sal_Bool bRdOnly )
{
    mpTextWindow->GetTextView()->SetReadOnly( bRdOnly );
    // TODO: Adjust color?
}

sal_Bool ImpVclMEdit::IsReadOnly() const
{
    return mpTextWindow->GetTextView()->IsReadOnly();
}

void ImpVclMEdit::SetMaxTextLen( xub_StrLen nLen )
{
    mpTextWindow->GetTextEngine()->SetMaxTextLen( nLen );
}

xub_StrLen ImpVclMEdit::GetMaxTextLen() const
{
    return sal::static_int_cast< xub_StrLen >(
        mpTextWindow->GetTextEngine()->GetMaxTextLen());
}

void ImpVclMEdit::InsertText( const OUString& rStr )
{
    mpTextWindow->GetTextView()->InsertText( rStr );
}

OUString ImpVclMEdit::GetSelected() const
{
    return mpTextWindow->GetTextView()->GetSelected();
}

OUString ImpVclMEdit::GetSelected( LineEnd aSeparator ) const
{
    return mpTextWindow->GetTextView()->GetSelected( aSeparator );
}

void ImpVclMEdit::SetMaxTextWidth( sal_uLong nMaxWidth )
{
    mpTextWindow->GetTextEngine()->SetMaxTextWidth( nMaxWidth );
}

void ImpVclMEdit::Resize()
{
    size_t nIteration = 1;
    do
    {
        WinBits nWinStyle( pVclMultiLineEdit->GetStyle() );
        if ( ( nWinStyle & WB_AUTOVSCROLL ) == WB_AUTOVSCROLL )
            ImpUpdateSrollBarVis( nWinStyle );

        Size aSz = pVclMultiLineEdit->GetOutputSizePixel();
        Size aEditSize = aSz;
        long nSBWidth = pVclMultiLineEdit->GetSettings().GetStyleSettings().GetScrollBarSize();
        nSBWidth = pVclMultiLineEdit->CalcZoom( nSBWidth );

        if ( mpHScrollBar )
            aSz.Height() -= nSBWidth+1;
        if ( mpVScrollBar )
            aSz.Width() -= nSBWidth+1;

        if ( !mpHScrollBar )
            mpTextWindow->GetTextEngine()->SetMaxTextWidth( aSz.Width() );
        else
            mpHScrollBar->setPosSizePixel( 0, aEditSize.Height()-nSBWidth, aSz.Width(), nSBWidth );

        Point aTextWindowPos( maTextWindowOffset );
        if ( mpVScrollBar )
        {
            if( Application::GetSettings().GetLayoutRTL() )
            {
                mpVScrollBar->setPosSizePixel( 0, 0, nSBWidth, aSz.Height() );
                aTextWindowPos.X() += nSBWidth;
            }
            else
                mpVScrollBar->setPosSizePixel( aEditSize.Width()-nSBWidth, 0, nSBWidth, aSz.Height() );
        }

        if ( mpScrollBox )
            mpScrollBox->setPosSizePixel( aSz.Width(), aSz.Height(), nSBWidth, nSBWidth );

        Size aTextWindowSize( aSz );
        aTextWindowSize.Width() -= maTextWindowOffset.X();
        aTextWindowSize.Height() -= maTextWindowOffset.Y();
        if ( aTextWindowSize.Width() < 0 )
            aTextWindowSize.Width() = 0;
        if ( aTextWindowSize.Height() < 0 )
            aTextWindowSize.Height() = 0;

        Size aOldTextWindowSize( mpTextWindow->GetSizePixel() );
        mpTextWindow->SetPosSizePixel( aTextWindowPos, aTextWindowSize );
        if ( aOldTextWindowSize == aTextWindowSize )
            break;

        // Changing the text window size might effectively have changed the need for
        // scrollbars, so do another iteration.
        ++nIteration;
        OSL_ENSURE( nIteration < 3, "ImpVclMEdit::Resize: isn't this expected to terminate with the second iteration?" );

    } while ( nIteration <= 3 );    // artificial break after four iterations

    ImpInitScrollBars();
}

void ImpVclMEdit::GetFocus()
{
    mpTextWindow->GrabFocus();
}

void ImpVclMEdit::Cut()
{
    if ( !mpTextWindow->GetTextView()->IsReadOnly() )
        mpTextWindow->GetTextView()->Cut();
}

void ImpVclMEdit::Copy()
{
    mpTextWindow->GetTextView()->Copy();
}

void ImpVclMEdit::Paste()
{
    if ( !mpTextWindow->GetTextView()->IsReadOnly() )
        mpTextWindow->GetTextView()->Paste();
}

void ImpVclMEdit::SetText( const OUString& rStr )
{
    sal_Bool bWasModified = mpTextWindow->GetTextEngine()->IsModified();
    mpTextWindow->GetTextEngine()->SetText( rStr );
    if ( !bWasModified )
        mpTextWindow->GetTextEngine()->SetModified( sal_False );

    mpTextWindow->GetTextView()->SetSelection( TextSelection() );

    WinBits nWinStyle( pVclMultiLineEdit->GetStyle() );
    if ( ( nWinStyle & WB_AUTOVSCROLL ) == WB_AUTOVSCROLL )
        ImpUpdateSrollBarVis( nWinStyle );
}

OUString ImpVclMEdit::GetText() const
{
    return mpTextWindow->GetTextEngine()->GetText();
}

OUString ImpVclMEdit::GetText( LineEnd aSeparator ) const
{
    return mpTextWindow->GetTextEngine()->GetText( aSeparator );
}

OUString ImpVclMEdit::GetTextLines( LineEnd aSeparator ) const
{
    return mpTextWindow->GetTextEngine()->GetTextLines( aSeparator );
}

void ImpVclMEdit::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( TextHint ) )
    {
        const TextHint& rTextHint = (const TextHint&)rHint;
        if( rTextHint.GetId() == TEXT_HINT_VIEWSCROLLED )
        {
            if ( mpHScrollBar )
                ImpSetHScrollBarThumbPos();
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
                sal_uLong nWidth = mpTextWindow->GetTextEngine()->CalcTextWidth();
                if ( nWidth != mnTextWidth )
                {
                    mnTextWidth = sal::static_int_cast< xub_StrLen >(nWidth);
                    mpHScrollBar->SetRange( Range( 0, (long)mnTextWidth-1 ) );
                    ImpSetHScrollBarThumbPos();
                }
            }
        }
        else if( rTextHint.GetId() == TEXT_HINT_MODIFIED )
        {
            ImpUpdateSrollBarVis(pVclMultiLineEdit->GetStyle());
            pVclMultiLineEdit->Modify();
        }
    }
}

void ImpVclMEdit::SetSelection( const Selection& rSelection )
{
    OUString aText = mpTextWindow->GetTextEngine()->GetText();

    Selection aNewSelection( rSelection );
    if ( aNewSelection.Min() < 0 )
        aNewSelection.Min() = 0;
    else if ( aNewSelection.Min() > aText.getLength() )
        aNewSelection.Min() = aText.getLength();
    if ( aNewSelection.Max() < 0 )
        aNewSelection.Max() = 0;
    else if ( aNewSelection.Max() > aText.getLength() )
        aNewSelection.Max() = aText.getLength();

    long nEnd = std::max( aNewSelection.Min(), aNewSelection.Max() );
    TextSelection aTextSel;
    sal_uLong nPara = 0;
    sal_uInt16 nChar = 0;
    sal_uInt16 x = 0;
    while ( x <= nEnd )
    {
        if ( x == aNewSelection.Min() )
            aTextSel.GetStart() = TextPaM( nPara, nChar );
        if ( x == aNewSelection.Max() )
            aTextSel.GetEnd() = TextPaM( nPara, nChar );

        if ( ( x < aText.getLength() ) && ( aText[ x ] == '\n' ) )
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

const Selection& ImpVclMEdit::GetSelection() const
{
    maSelection = Selection();
    TextSelection aTextSel( mpTextWindow->GetTextView()->GetSelection() );
    aTextSel.Justify();
    // flatten selection => every line-break a character

    ExtTextEngine* pExtTextEngine = mpTextWindow->GetTextEngine();
    // paragraphs before
    sal_uLong n;
    for ( n = 0; n < aTextSel.GetStart().GetPara(); n++ )
    {
        maSelection.Min() += pExtTextEngine->GetTextLen( n );
        maSelection.Min()++;
    }

    // first paragraph with selection
    maSelection.Max() = maSelection.Min();
    maSelection.Min() += aTextSel.GetStart().GetIndex();

    for ( n = aTextSel.GetStart().GetPara(); n < aTextSel.GetEnd().GetPara(); n++ )
    {
        maSelection.Max() += pExtTextEngine->GetTextLen( n );
        maSelection.Max()++;
    }

    maSelection.Max() += aTextSel.GetEnd().GetIndex();

    return maSelection;
}

Size ImpVclMEdit::CalcMinimumSize() const
{
    Size aSz(   mpTextWindow->GetTextEngine()->CalcTextWidth(),
                mpTextWindow->GetTextEngine()->GetTextHeight() );

    if ( mpHScrollBar )
        aSz.Height() += mpHScrollBar->GetSizePixel().Height();
    if ( mpVScrollBar )
        aSz.Width() += mpVScrollBar->GetSizePixel().Width();

    return aSz;
}

Size ImpVclMEdit::CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
{
    static const sal_Unicode sampleChar = 'X';

    Size aSz;
    Size aCharSz;
    aCharSz.Width() = mpTextWindow->GetTextWidth( OUString(sampleChar) );
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

void ImpVclMEdit::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    static const sal_Unicode sampleChar = { 'x' };
    Size aOutSz = mpTextWindow->GetOutputSizePixel();
    Size aCharSz( mpTextWindow->GetTextWidth( OUString(sampleChar) ), mpTextWindow->GetTextHeight() );
    rnCols = (sal_uInt16) (aOutSz.Width()/aCharSz.Width());
    rnLines = (sal_uInt16) (aOutSz.Height()/aCharSz.Height());
}

void ImpVclMEdit::Enable( sal_Bool bEnable )
{
    mpTextWindow->Enable( bEnable );
    if ( mpHScrollBar )
        mpHScrollBar->Enable( bEnable );
    if ( mpVScrollBar )
        mpVScrollBar->Enable( bEnable );
}

sal_Bool ImpVclMEdit::HandleCommand( const CommandEvent& rCEvt )
{
    sal_Bool bDone = sal_False;
    if ( ( rCEvt.GetCommand() == COMMAND_WHEEL ) ||
         ( rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL ) ||
         ( rCEvt.GetCommand() == COMMAND_AUTOSCROLL ) )
    {
        mpTextWindow->HandleScrollCommand( rCEvt, mpHScrollBar, mpVScrollBar );
        bDone = sal_True;
    }
    return bDone;
}


TextWindow::TextWindow( Window* pParent ) : Window( pParent )
{
    mbInMBDown = false;
    mbFocusSelectionHide = false;
    mbIgnoreTab = false;
    mbActivePopup = false;
    mbSelectOnTab = true;
    mbTextSelectable = true;

    SetPointer( Pointer( POINTER_TEXT ) );

    mpExtTextEngine = new ExtTextEngine;
    mpExtTextEngine->SetMaxTextLen( STRING_MAXLEN );
    if( pParent->GetStyle() & WB_BORDER )
        mpExtTextEngine->SetLeftMargin( 2 );
    mpExtTextEngine->SetLocale( GetSettings().GetLanguageTag().getLocale() );
    mpExtTextView = new ExtTextView( mpExtTextEngine, this );
    mpExtTextEngine->InsertView( mpExtTextView );
    mpExtTextEngine->EnableUndo( sal_True );
    mpExtTextView->ShowCursor();

    Color aBackgroundColor = GetSettings().GetStyleSettings().GetWorkspaceColor();
    SetBackground( aBackgroundColor );
    pParent->SetBackground( aBackgroundColor );
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
    if ( !mbTextSelectable )
        return;

    mbInMBDown = true;  // so that GetFocus does not select everything
    mpExtTextView->MouseButtonDown( rMEvt );
    Window::MouseButtonDown( rMEvt );
    GrabFocus();
    mbInMBDown = false;
}

void TextWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    mpExtTextView->MouseButtonUp( rMEvt );
    Window::MouseButtonUp( rMEvt );
}

void TextWindow::KeyInput( const KeyEvent& rKEvent )
{
    sal_Bool bDone = sal_False;
    sal_uInt16 nCode = rKEvent.GetKeyCode().GetCode();
    if ( nCode == com::sun::star::awt::Key::SELECT_ALL ||
         ( (nCode == KEY_A) && rKEvent.GetKeyCode().IsMod1() && !rKEvent.GetKeyCode().IsMod2() )
       )
    {
        mpExtTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFF, 0xFFFF ) ) );
        bDone = sal_True;
    }
    else if ( (nCode == KEY_S) && rKEvent.GetKeyCode().IsShift() && rKEvent.GetKeyCode().IsMod1() )
    {
        if ( Edit::GetGetSpecialCharsFunction() )
        {
            // to maintain the selection
            mbActivePopup = true;
            OUString aChars = Edit::GetGetSpecialCharsFunction()( this, GetFont() );
            if (!aChars.isEmpty())
            {
                mpExtTextView->InsertText( aChars );
                mpExtTextView->GetTextEngine()->SetModified( sal_True );
            }
            mbActivePopup = false;
            bDone = sal_True;
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
        if ( !mpExtTextView->HasSelection() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_COPY, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, sal_False );
        }
        if ( mpExtTextView->IsReadOnly() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_INSERTSYMBOL, sal_False );
        }
        if ( !mpExtTextView->GetTextEngine()->HasUndoManager() || !mpExtTextView->GetTextEngine()->GetUndoManager().GetUndoActionCount() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_UNDO, sal_False );
        }
//      if ( ( maSelection.Min() == 0 ) && ( maSelection.Max() == maText.Len() ) )
//      {
//          pPopup->EnableItem( SV_MENU_EDIT_SELECTALL, sal_False );
//      }
        if ( !Edit::GetGetSpecialCharsFunction() )
        {
            sal_uInt16 nPos = pPopup->GetItemPos( SV_MENU_EDIT_INSERTSYMBOL );
            pPopup->RemoveItem( nPos );
            pPopup->RemoveItem( nPos-1 );
        }

        mbActivePopup = true;
        Point aPos = rCEvt.GetMousePosPixel();
        if ( !rCEvt.IsMouseEvent() )
        {
            // Sometime do show Menu centered in the selection !!!
            Size aSize = GetOutputSizePixel();
            aPos = Point( aSize.Width()/2, aSize.Height()/2 );
        }
//      pPopup->RemoveDisabledEntries();
        sal_uInt16 n = pPopup->Execute( this, aPos );
        Edit::DeletePopupMenu( pPopup );
        switch ( n )
        {
            case SV_MENU_EDIT_UNDO:     mpExtTextView->Undo();
                                        mpExtTextEngine->SetModified( sal_True );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_CUT:      mpExtTextView->Cut();
                                        mpExtTextEngine->SetModified( sal_True );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_COPY:     mpExtTextView->Copy();
                                        break;
            case SV_MENU_EDIT_PASTE:    mpExtTextView->Paste();
                                        mpExtTextEngine->SetModified( sal_True );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_DELETE:   mpExtTextView->DeleteSelected();
                                        mpExtTextEngine->SetModified( sal_True );
                                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                                        break;
            case SV_MENU_EDIT_SELECTALL:    mpExtTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
                                            break;
            case SV_MENU_EDIT_INSERTSYMBOL:
                {
                    OUString aChars = Edit::GetGetSpecialCharsFunction()( this, GetFont() );
                    if (!aChars.isEmpty())
                    {
                        mpExtTextView->InsertText( aChars );
                        mpExtTextEngine->SetModified( sal_True );
                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                    }
                }
                break;
        }
        mbActivePopup = false;
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
        sal_Bool bGotoCursor = !mpExtTextView->IsReadOnly();
        if ( mbFocusSelectionHide && IsReallyVisible() && !mpExtTextView->IsReadOnly()
                && ( mbSelectOnTab &&
                    (!mbInMBDown || ( GetSettings().GetStyleSettings().GetSelectionOptions() & SELECTION_OPTION_FOCUS ) )) )
        {
            // select everything, but do not scroll
            sal_Bool bAutoScroll = mpExtTextView->IsAutoScroll();
            mpExtTextView->SetAutoScroll( sal_False );
            mpExtTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFF, 0xFFFF ) ) );
            mpExtTextView->SetAutoScroll( bAutoScroll );
            bGotoCursor = sal_False;
        }
        mpExtTextView->SetPaintSelection( sal_True );
        mpExtTextView->ShowCursor( bGotoCursor );
    }
}

void TextWindow::LoseFocus()
{
    Window::LoseFocus();

    if ( mbFocusSelectionHide && !mbActivePopup )
        mpExtTextView->SetPaintSelection( sal_False );
}

VclMultiLineEdit::VclMultiLineEdit( Window* pParent, WinBits nWinStyle )
    : Edit( pParent, nWinStyle )
{
    SetType( WINDOW_MULTILINEEDIT );
    pImpVclMEdit = new ImpVclMEdit( this, nWinStyle );
    ImplInitSettings( sal_True, sal_True, sal_True );
    pUpdateDataTimer = 0;

    SetCompoundControl( sal_True );
    SetStyle( ImplInitStyle( nWinStyle ) );
}

VclMultiLineEdit::VclMultiLineEdit( Window* pParent, const ResId& rResId )
    : Edit( pParent, rResId.SetRT( RSC_MULTILINEEDIT ) )
{
    SetType( WINDOW_MULTILINEEDIT );
    WinBits nWinStyle = rResId.GetWinBits();
    pImpVclMEdit = new ImpVclMEdit( this, nWinStyle );
    ImplInitSettings( sal_True, sal_True, sal_True );
    pUpdateDataTimer = 0;

    sal_uInt16 nMaxLen = Edit::GetMaxTextLen();
    if ( nMaxLen )
        SetMaxTextLen( nMaxLen );

    SetText( Edit::GetText() );

    if ( IsVisible() )
        pImpVclMEdit->Resize();

    SetCompoundControl( sal_True );
    SetStyle( ImplInitStyle( nWinStyle ) );

    // Base Edit ctor could call Show already, but that would cause problems
    // with accessibility, as Show might (indirectly) trigger a call to virtual
    // GetComponentInterface, which is the Edit's base version instead of the
    // VclMultiLineEdit's version while in the base Edit ctor:
    if ((GetStyle() & WB_HIDE) == 0)
        Show();

}

VclMultiLineEdit::~VclMultiLineEdit()
{
    {
        ::std::auto_ptr< ImpVclMEdit > pDelete( pImpVclMEdit );
        pImpVclMEdit = NULL;
    }
    delete pUpdateDataTimer;
}

WinBits VclMultiLineEdit::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    if ( !(nStyle & WB_IGNORETAB ))
        nStyle |= WINDOW_DLGCTRL_MOD1TAB;

    return nStyle;
}


void VclMultiLineEdit::ImplInitSettings( sal_Bool /*bFont*/, sal_Bool /*bForeground*/, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // The Font has to be adjusted, as the TextEngine does not take care of
    // TextColor/Background

    Color aTextColor = rStyleSettings.GetFieldTextColor();
    if ( IsControlForeground() )
        aTextColor = GetControlForeground();
    if ( !IsEnabled() )
        aTextColor = rStyleSettings.GetDisableColor();

    Font aFont = rStyleSettings.GetFieldFont();
    if ( IsControlFont() )
        aFont.Merge( GetControlFont() );
    aFont.SetTransparent( IsPaintTransparent() );
    SetZoomedPointFont( aFont );
    Font TheFont = GetFont();
    TheFont.SetColor( aTextColor );
    if( IsPaintTransparent() )
        TheFont.SetFillColor( Color( COL_TRANSPARENT ) );
    else
        TheFont.SetFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );
    pImpVclMEdit->GetTextWindow()->SetFont( TheFont );
    pImpVclMEdit->GetTextWindow()->GetTextEngine()->SetFont( TheFont );
    pImpVclMEdit->GetTextWindow()->SetTextColor( aTextColor );

    if ( bBackground )
    {
        if( IsPaintTransparent() )
        {
            pImpVclMEdit->GetTextWindow()->SetPaintTransparent( sal_True );
            pImpVclMEdit->GetTextWindow()->SetBackground();
            pImpVclMEdit->GetTextWindow()->SetControlBackground();
            SetBackground();
            SetControlBackground();
        }
        else
        {
            if( IsControlBackground() )
                pImpVclMEdit->GetTextWindow()->SetBackground( GetControlBackground() );
            else
                pImpVclMEdit->GetTextWindow()->SetBackground( rStyleSettings.GetFieldColor() );
            // also adjust for VclMultiLineEdit as the TextComponent might hide Scrollbars
            SetBackground( pImpVclMEdit->GetTextWindow()->GetBackground() );
        }
    }
}

void VclMultiLineEdit::Modify()
{
    aModifyHdlLink.Call( this );

    CallEventListeners( VCLEVENT_EDIT_MODIFY );

    if ( pUpdateDataTimer )
        pUpdateDataTimer->Start();
}

IMPL_LINK_NOARG(VclMultiLineEdit, ImpUpdateDataHdl)
{
    UpdateData();
    return 0;
}

void VclMultiLineEdit::UpdateData()
{
    aUpdateDataHdlLink.Call( this );
}

void VclMultiLineEdit::SetModifyFlag()
{
    pImpVclMEdit->SetModified( sal_True );
}

void VclMultiLineEdit::ClearModifyFlag()
{
    pImpVclMEdit->SetModified( sal_False );
}

sal_Bool VclMultiLineEdit::IsModified() const
{
    return pImpVclMEdit->IsModified();
}

void VclMultiLineEdit::EnableUpdateData( sal_uLong nTimeout )
{
    if ( !nTimeout )
        DisableUpdateData();
    else
    {
        if ( !pUpdateDataTimer )
        {
            pUpdateDataTimer = new Timer;
            pUpdateDataTimer->SetTimeoutHdl( LINK( this, VclMultiLineEdit, ImpUpdateDataHdl ) );
        }
        pUpdateDataTimer->SetTimeout( nTimeout );
    }
}

void VclMultiLineEdit::SetReadOnly( sal_Bool bReadOnly )
{
    pImpVclMEdit->SetReadOnly( bReadOnly );
    Edit::SetReadOnly( bReadOnly );

    // #94921# ReadOnly can be overwritten in InitFromStyle() when WB not set.
    WinBits nStyle = GetStyle();
    if ( bReadOnly )
        nStyle |= WB_READONLY;
    else
        nStyle &= ~WB_READONLY;
    SetStyle( nStyle );
}

sal_Bool VclMultiLineEdit::IsReadOnly() const
{
    return pImpVclMEdit->IsReadOnly();
}

void VclMultiLineEdit::SetMaxTextLen( xub_StrLen nMaxLen )
{
    pImpVclMEdit->SetMaxTextLen( nMaxLen );
}

void VclMultiLineEdit::SetMaxTextWidth( sal_uLong nMaxWidth )
{
    pImpVclMEdit->SetMaxTextWidth(nMaxWidth );
}

xub_StrLen VclMultiLineEdit::GetMaxTextLen() const
{
    return pImpVclMEdit->GetMaxTextLen();
}

void VclMultiLineEdit::ReplaceSelected( const OUString& rStr )
{
    pImpVclMEdit->InsertText( rStr );
}

void VclMultiLineEdit::DeleteSelected()
{
    pImpVclMEdit->InsertText( OUString() );
}

OUString VclMultiLineEdit::GetSelected() const
{
    return pImpVclMEdit->GetSelected();
}

OUString VclMultiLineEdit::GetSelected( LineEnd aSeparator ) const
{
    return pImpVclMEdit->GetSelected( aSeparator );
}

void VclMultiLineEdit::Cut()
{
    pImpVclMEdit->Cut();
}

void VclMultiLineEdit::Copy()
{
    pImpVclMEdit->Copy();
}

void VclMultiLineEdit::Paste()
{
    pImpVclMEdit->Paste();
}

void VclMultiLineEdit::SetText( const OUString& rStr )
{
    pImpVclMEdit->SetText( rStr );
}

OUString VclMultiLineEdit::GetText() const
{
    return pImpVclMEdit->GetText();
}

OUString VclMultiLineEdit::GetText( LineEnd aSeparator ) const
{
    return pImpVclMEdit->GetText( aSeparator );
}

OUString VclMultiLineEdit::GetTextLines(  LineEnd aSeparator ) const
{
    return pImpVclMEdit->GetTextLines( aSeparator );
}

void VclMultiLineEdit::Resize()
{
    pImpVclMEdit->Resize();
}

void VclMultiLineEdit::GetFocus()
{
    if ( !pImpVclMEdit )  // might be called from within the dtor, when pImpVclMEdit == NULL is a valid state
        return;

    Edit::GetFocus();
    pImpVclMEdit->GetFocus();
}

void VclMultiLineEdit::SetSelection( const Selection& rSelection )
{
    pImpVclMEdit->SetSelection( rSelection );
}

const Selection& VclMultiLineEdit::GetSelection() const
{
    return pImpVclMEdit->GetSelection();
}

Size VclMultiLineEdit::CalcMinimumSize() const
{
    Size aSz = pImpVclMEdit->CalcMinimumSize();

    sal_Int32 nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Width() += nLeft+nRight;
    aSz.Height() += nTop+nBottom;

    return aSz;
}

Size VclMultiLineEdit::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    sal_Int32 nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );

    // center vertically for whole lines

    long nHeight = aSz.Height() - nTop - nBottom;
    long nLineHeight = pImpVclMEdit->CalcSize( 1, 1 ).Height();
    long nLines = nHeight / nLineHeight;
    if ( nLines < 1 )
        nLines = 1;

    aSz.Height() = nLines * nLineHeight;
    aSz.Height() += nTop+nBottom;

    return aSz;
}

Size VclMultiLineEdit::CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
{
    Size aSz = pImpVclMEdit->CalcSize( nColumns, nLines );

    sal_Int32 nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Width() += nLeft+nRight;
    aSz.Height() += nTop+nBottom;
    return aSz;
}

void VclMultiLineEdit::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    pImpVclMEdit->GetMaxVisColumnsAndLines( rnCols, rnLines );
}

void VclMultiLineEdit::StateChanged( StateChangedType nType )
{
    if( nType == STATE_CHANGE_ENABLE )
    {
        pImpVclMEdit->Enable( IsEnabled() );
        ImplInitSettings( sal_True, sal_False, sal_False );
    }
    else if( nType == STATE_CHANGE_READONLY )
    {
        pImpVclMEdit->SetReadOnly( IsReadOnly() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        pImpVclMEdit->GetTextWindow()->SetZoom( GetZoom() );
        ImplInitSettings( sal_True, sal_False, sal_False );
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        ImplInitSettings( sal_True, sal_False, sal_False );
        Resize();
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        pImpVclMEdit->InitFromStyle( GetStyle() );
        SetStyle( ImplInitStyle( GetStyle() ) );
    }
    else if ( nType == STATE_CHANGE_INITSHOW )
    {
        if( IsPaintTransparent() )
        {
            pImpVclMEdit->GetTextWindow()->SetPaintTransparent( sal_True );
            pImpVclMEdit->GetTextWindow()->SetBackground();
            pImpVclMEdit->GetTextWindow()->SetControlBackground();
            SetBackground();
            SetControlBackground();
        }
    }

    Control::StateChanged( nType );
}

void VclMultiLineEdit::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Resize();
        Invalidate();
    }
    else
        Control::DataChanged( rDCEvt );
}

void VclMultiLineEdit::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    ImplInitSettings( sal_True, sal_True, sal_True );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = pImpVclMEdit->GetTextWindow()->GetDrawPixelFont( pDev );
    aFont.SetTransparent( sal_True );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
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

    // contents
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

    OUString aText = GetText();
    Size aTextSz( pDev->GetTextWidth( aText ), pDev->GetTextHeight() );
    sal_uLong nLines = (sal_uLong) (aSize.Height() / aTextSz.Height());
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
            aClip.Bottom() += aTextSz.Height() - aSize.Height() + 1;  // so that HP-printer does not 'optimize-away'
        pDev->IntersectClipRegion( aClip );
    }

    ExtTextEngine aTE;
    aTE.SetText( GetText() );
    aTE.SetMaxTextWidth( aSize.Width() );
    aTE.SetFont( aFont );
    aTE.SetTextAlign( pImpVclMEdit->GetTextWindow()->GetTextEngine()->GetTextAlign() );
    aTE.Draw( pDev, Point( aPos.X() + nOffX, aPos.Y() + nOffY ) );

    pDev->Pop();
}

long VclMultiLineEdit::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        nDone = pImpVclMEdit->HandleCommand( *rNEvt.GetCommandEvent() );
    }
    return nDone ? nDone : Edit::Notify( rNEvt );
}

long VclMultiLineEdit::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;

#if (OSL_DEBUG_LEVEL > 1) && defined(DBG_UTIL)
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent& rKEvent = *rNEvt.GetKeyEvent();
        if ( ( rKEvent.GetKeyCode().GetCode() == KEY_W ) && rKEvent.GetKeyCode().IsMod1() && rKEvent.GetKeyCode().IsMod2() )
        {
            SetRightToLeft( !IsRightToLeft() );
        }
    }
#endif

    if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && ( !GetTextView()->IsCursorEnabled() ) )
    {
        const KeyEvent& rKEvent = *rNEvt.GetKeyEvent();
        if ( !rKEvent.GetKeyCode().IsShift() && ( rKEvent.GetKeyCode().GetGroup() == KEYGROUP_CURSOR ) )
        {
            nDone = 1;
            TextSelection aSel = pImpVclMEdit->GetTextWindow()->GetTextView()->GetSelection();
            if ( aSel.HasRange() )
            {
                aSel.GetStart() = aSel.GetEnd();
                pImpVclMEdit->GetTextWindow()->GetTextView()->SetSelection( aSel );
            }
            else
            {
                switch ( rKEvent.GetKeyCode().GetCode() )
                {
                    case KEY_UP:
                    {
                        if ( pImpVclMEdit->GetVScrollBar() )
                            pImpVclMEdit->GetVScrollBar()->DoScrollAction( SCROLL_LINEUP );
                    }
                    break;
                    case KEY_DOWN:
                    {
                        if ( pImpVclMEdit->GetVScrollBar() )
                            pImpVclMEdit->GetVScrollBar()->DoScrollAction( SCROLL_LINEDOWN );
                    }
                    break;
                    case KEY_PAGEUP :
                    {
                        if ( pImpVclMEdit->GetVScrollBar() )
                            pImpVclMEdit->GetVScrollBar()->DoScrollAction( SCROLL_PAGEUP );
                    }
                    break;
                    case KEY_PAGEDOWN:
                    {
                        if ( pImpVclMEdit->GetVScrollBar() )
                            pImpVclMEdit->GetVScrollBar()->DoScrollAction( SCROLL_PAGEDOWN );
                    }
                    break;
                    case KEY_LEFT:
                    {
                        if ( pImpVclMEdit->GetHScrollBar() )
                            pImpVclMEdit->GetHScrollBar()->DoScrollAction( SCROLL_LINEUP );
                    }
                    break;
                    case KEY_RIGHT:
                    {
                        if ( pImpVclMEdit->GetHScrollBar() )
                            pImpVclMEdit->GetHScrollBar()->DoScrollAction( SCROLL_LINEDOWN );
                    }
                    break;
                    case KEY_HOME:
                    {
                        if ( rKEvent.GetKeyCode().IsMod1() )
                            pImpVclMEdit->GetTextWindow()->GetTextView()->
                                SetSelection( TextSelection( TextPaM( 0, 0 ) ) );
                    }
                    break;
                    case KEY_END:
                    {
                        if ( rKEvent.GetKeyCode().IsMod1() )
                            pImpVclMEdit->GetTextWindow()->GetTextView()->
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
// Internals for derived classes, e.g. TextComponent

ExtTextEngine* VclMultiLineEdit::GetTextEngine() const
{
    return pImpVclMEdit->GetTextWindow()->GetTextEngine();
}

ExtTextView* VclMultiLineEdit::GetTextView() const
{
    return pImpVclMEdit->GetTextWindow()->GetTextView();
}

ScrollBar* VclMultiLineEdit::GetVScrollBar() const
{
    return pImpVclMEdit->GetVScrollBar();
}

void VclMultiLineEdit::EnableFocusSelectionHide( sal_Bool bHide )
{
    pImpVclMEdit->GetTextWindow()->SetAutoFocusHide( bHide );
}

void VclMultiLineEdit::SetLeftMargin( sal_uInt16 n )
{
    if ( GetTextEngine() )
        GetTextEngine()->SetLeftMargin( n );
}

void VclMultiLineEdit::SetRightToLeft( sal_Bool bRightToLeft )
{
    if ( GetTextEngine() )
    {
        GetTextEngine()->SetRightToLeft( bRightToLeft );
        GetTextView()->ShowCursor();
    }
}

sal_Bool VclMultiLineEdit::IsRightToLeft() const
{
    sal_Bool bRightToLeft = sal_False;

    if ( GetTextEngine() )
        bRightToLeft = GetTextEngine()->IsRightToLeft();

    return bRightToLeft;
}

void VclMultiLineEdit::DisableSelectionOnFocus()
{
    pImpVclMEdit->GetTextWindow()->DisableSelectionOnFocus();
}

void VclMultiLineEdit::SetTextSelectable( bool bTextSelectable )
{
    pImpVclMEdit->GetTextWindow()->SetTextSelectable( bTextSelectable );
}

void VclMultiLineEdit::EnableCursor( sal_Bool bEnable )
{
    GetTextView()->EnableCursor( bEnable );
}

bool VclMultiLineEdit::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "cursor-visible")
        EnableCursor(toBool(rValue));
    else if (rKey == "accepts-tab")
        pImpVclMEdit->GetTextWindow()->SetIgnoreTab(!toBool(rValue));
    else
        return Edit::set_property(rKey, rValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
