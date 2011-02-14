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
#include "precompiled_svtools.hxx"


#include <memory>

#include "unoiface.hxx"

#include <tools/rc.h>

#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>

#include <svtools/svmedit.hxx>
#include <svtools/xtextedt.hxx>
#include <svl/brdcst.hxx>
#include <svl/undo.hxx>
#include <svtools/textwindowpeer.hxx>
#include <svl/lstner.hxx>
#include <svl/smplhint.hxx>


// IDs erstmal aus VCL geklaut, muss mal richtig delivert werden...
#define SV_MENU_EDIT_UNDO           1
#define SV_MENU_EDIT_CUT            2
#define SV_MENU_EDIT_COPY           3
#define SV_MENU_EDIT_PASTE          4
#define SV_MENU_EDIT_DELETE         5
#define SV_MENU_EDIT_SELECTALL      6
#define SV_MENU_EDIT_INSERTSYMBOL   7
#include <vcl/scrbar.hxx>

namespace css = ::com::sun::star;

class TextWindow : public Window
{
private:
    ExtTextEngine*  mpExtTextEngine;
    ExtTextView*    mpExtTextView;

    sal_Bool            mbInMBDown;
    sal_Bool            mbFocusSelectionHide;
    sal_Bool            mbIgnoreTab;
    sal_Bool            mbActivePopup;
    sal_Bool            mbSelectOnTab;

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

    sal_Bool            IsAutoFocusHide() const { return mbFocusSelectionHide; }
    void            SetAutoFocusHide( sal_Bool bAutoHide ) { mbFocusSelectionHide = bAutoHide; }

    sal_Bool            IsIgnoreTab() const { return mbIgnoreTab; }
    void            SetIgnoreTab( sal_Bool bIgnore ) { mbIgnoreTab = bIgnore; }

    void            DisableSelectionOnFocus() {mbSelectOnTab = sal_False;}

    virtual
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
    GetComponentInterface(sal_Bool bCreate = sal_True);
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
                ImpSvMEdit( MultiLineEdit* pSvMultiLineEdit, WinBits nWinStyle );
                ~ImpSvMEdit();

    void        SetModified( sal_Bool bMod );
    sal_Bool        IsModified() const;

    void        SetReadOnly( sal_Bool bRdOnly );
    sal_Bool        IsReadOnly() const;

    void        SetMaxTextLen( xub_StrLen nLen );
    xub_StrLen  GetMaxTextLen() const;

    void        SetInsertMode( sal_Bool bInsert );
    sal_Bool        IsInsertMode() const;

    void        InsertText( const String& rStr );
    String      GetSelected() const;
    String      GetSelected( LineEnd aSeparator ) const;

    void        SetSelection( const Selection& rSelection );
    const Selection& GetSelection() const;

    void        Cut();
    void        Copy();
    void        Paste();

    void        SetText( const String& rStr );
    String      GetText() const;
    String      GetText( LineEnd aSeparator ) const;
    String      GetTextLines() const;
    String      GetTextLines( LineEnd aSeparator ) const;

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

    void        SetTextWindowOffset( const Point& rOffset );
};

ImpSvMEdit::ImpSvMEdit( MultiLineEdit* pEdt, WinBits nWinStyle )
    :mpHScrollBar(NULL)
    ,mpVScrollBar(NULL)
    ,mpScrollBox(NULL)
{
    pSvMultiLineEdit = pEdt;
    mnTextWidth = 0;
    mpTextWindow = new TextWindow( pEdt );
    mpTextWindow->Show();
    InitFromStyle( nWinStyle );
    StartListening( *mpTextWindow->GetTextEngine() );
}

void ImpSvMEdit::ImpUpdateSrollBarVis( WinBits nWinStyle )
{
    const sal_Bool bHaveVScroll = (NULL != mpVScrollBar);
    const sal_Bool bHaveHScroll = (NULL != mpHScrollBar);
    const sal_Bool bHaveScrollBox = (NULL != mpScrollBox);

          sal_Bool bNeedVScroll = ( nWinStyle & WB_VSCROLL ) == WB_VSCROLL;
    const sal_Bool bNeedHScroll = ( nWinStyle & WB_HSCROLL ) == WB_HSCROLL;

    const sal_Bool bAutoVScroll = ( nWinStyle & WB_AUTOVSCROLL ) == WB_AUTOVSCROLL;
    if ( !bNeedVScroll && bAutoVScroll )
    {
        TextEngine& rEngine( *mpTextWindow->GetTextEngine() );
        sal_uLong nOverallTextHeight(0);
        for ( sal_uLong i=0; i<rEngine.GetParagraphCount(); ++i )
            nOverallTextHeight += rEngine.GetTextHeight( i );
        if ( nOverallTextHeight > (sal_uLong)mpTextWindow->GetOutputSizePixel().Height() )
            bNeedVScroll = true;
    }

    const sal_Bool bNeedScrollBox = bNeedVScroll && bNeedHScroll;

    sal_Bool bScrollbarsChanged = false;
    if ( bHaveVScroll != bNeedVScroll )
    {
        delete mpVScrollBar;
        mpVScrollBar = bNeedVScroll ? new ScrollBar( pSvMultiLineEdit, WB_VSCROLL|WB_DRAG ) : NULL;

        if ( bNeedVScroll )
        {
            mpVScrollBar->Show();
            mpVScrollBar->SetScrollHdl( LINK( this, ImpSvMEdit, ScrollHdl ) );
        }

        bScrollbarsChanged = sal_True;
    }

    if ( bHaveHScroll != bNeedHScroll )
    {
        delete mpHScrollBar;
        mpHScrollBar = bNeedHScroll ? new ScrollBar( pSvMultiLineEdit, WB_HSCROLL|WB_DRAG ) : NULL;

        if ( bNeedHScroll )
        {
            mpHScrollBar->Show();
            mpHScrollBar->SetScrollHdl( LINK( this, ImpSvMEdit, ScrollHdl ) );
        }

        bScrollbarsChanged = sal_True;
    }

    if ( bHaveScrollBox != bNeedScrollBox )
    {
        delete mpScrollBox;
        mpScrollBox = bNeedScrollBox ? new ScrollBarBox( pSvMultiLineEdit, WB_SIZEABLE ) : NULL;

        if ( bNeedScrollBox )
            mpScrollBox->Show();
    }

    if ( bScrollbarsChanged )
    {
        ImpInitScrollBars();
        Resize();
    }
}

void ImpSvMEdit::InitFromStyle( WinBits nWinStyle )
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
        mpTextWindow->SetIgnoreTab( sal_True );
    }
    else
    {
        mpTextWindow->SetIgnoreTab( sal_False );
        // #103667# MultiLineEdit has the flag, but focusable window also needs this flag
        WinBits nStyle = mpTextWindow->GetStyle();
        nStyle |= WINDOW_DLGCTRL_MOD1TAB;
        mpTextWindow->SetStyle( nStyle );
    }
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

void ImpSvMEdit::ImpSetHScrollBarThumbPos()
{
    long nX = mpTextWindow->GetTextView()->GetStartDocPos().X();
    if ( !mpTextWindow->GetTextEngine()->IsRightToLeft() )
        mpHScrollBar->SetThumbPos( nX );
    else
        mpHScrollBar->SetThumbPos( mnTextWidth - mpHScrollBar->GetVisibleSize() - nX );

}

IMPL_LINK( ImpSvMEdit, ScrollHdl, ScrollBar*, pCurScrollBar )
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


// void ImpSvMEdit::ImpModified()
// {
//  // Wann wird das gerufen ?????????????????????
//  pSvMultiLineEdit->Modify();
// }

void ImpSvMEdit::SetAlign( WinBits nWinStyle )
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

void ImpSvMEdit::SetTextWindowOffset( const Point& rOffset )
{
    maTextWindowOffset = rOffset;
    Resize();
}

void ImpSvMEdit::SetModified( sal_Bool bMod )
{
    mpTextWindow->GetTextEngine()->SetModified( bMod );
}

sal_Bool ImpSvMEdit::IsModified() const
{
    return mpTextWindow->GetTextEngine()->IsModified();
}

void ImpSvMEdit::SetInsertMode( sal_Bool bInsert )
{
    mpTextWindow->GetTextView()->SetInsertMode( bInsert );
}

void ImpSvMEdit::SetReadOnly( sal_Bool bRdOnly )
{
    mpTextWindow->GetTextView()->SetReadOnly( bRdOnly );
    // Farbe anpassen ???????????????????????????
}

sal_Bool ImpSvMEdit::IsReadOnly() const
{
    return mpTextWindow->GetTextView()->IsReadOnly();
}

void ImpSvMEdit::SetMaxTextLen( xub_StrLen nLen )
{
    mpTextWindow->GetTextEngine()->SetMaxTextLen( nLen );
}

xub_StrLen ImpSvMEdit::GetMaxTextLen() const
{
    return sal::static_int_cast< xub_StrLen >(
        mpTextWindow->GetTextEngine()->GetMaxTextLen());
}

void ImpSvMEdit::InsertText( const String& rStr )
{
    mpTextWindow->GetTextView()->InsertText( rStr );
}

String ImpSvMEdit::GetSelected() const
{
    return mpTextWindow->GetTextView()->GetSelected();
}

String ImpSvMEdit::GetSelected( LineEnd aSeparator ) const
{
    return mpTextWindow->GetTextView()->GetSelected( aSeparator );
}

void ImpSvMEdit::Resize()
{
    size_t nIteration = 1;
    do
    {
        WinBits nWinStyle( pSvMultiLineEdit->GetStyle() );
        if ( ( nWinStyle & WB_AUTOVSCROLL ) == WB_AUTOVSCROLL )
            ImpUpdateSrollBarVis( nWinStyle );

        Size aSz = pSvMultiLineEdit->GetOutputSizePixel();
        Size aEditSize = aSz;
        long nSBWidth = pSvMultiLineEdit->GetSettings().GetStyleSettings().GetScrollBarSize();
        nSBWidth = pSvMultiLineEdit->CalcZoom( nSBWidth );

        if ( mpHScrollBar )
            aSz.Height() -= nSBWidth+1;
        if ( mpVScrollBar )
            aSz.Width() -= nSBWidth+1;

        if ( !mpHScrollBar )
            mpTextWindow->GetTextEngine()->SetMaxTextWidth( aSz.Width() );
        else
            mpHScrollBar->SetPosSizePixel( 0, aEditSize.Height()-nSBWidth, aSz.Width(), nSBWidth );

        Point aTextWindowPos( maTextWindowOffset );
        if ( mpVScrollBar )
        {
            if( Application::GetSettings().GetLayoutRTL() )
            {
                mpVScrollBar->SetPosSizePixel( 0, 0, nSBWidth, aSz.Height() );
                aTextWindowPos.X() += nSBWidth;
            }
            else
                mpVScrollBar->SetPosSizePixel( aEditSize.Width()-nSBWidth, 0, nSBWidth, aSz.Height() );
        }

        if ( mpScrollBox )
            mpScrollBox->SetPosSizePixel( aSz.Width(), aSz.Height(), nSBWidth, nSBWidth );

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
        OSL_ENSURE( nIteration < 3, "ImpSvMEdit::Resize: isn't this expected to terminate with the second iteration?" );

    } while ( nIteration <= 3 );    // artificial break after four iterations

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
    sal_Bool bWasModified = mpTextWindow->GetTextEngine()->IsModified();
    mpTextWindow->GetTextEngine()->SetText( rStr );
    if ( !bWasModified )
        mpTextWindow->GetTextEngine()->SetModified( sal_False );

    mpTextWindow->GetTextView()->SetSelection( TextSelection() );

    WinBits nWinStyle( pSvMultiLineEdit->GetStyle() );
    if ( ( nWinStyle & WB_AUTOVSCROLL ) == WB_AUTOVSCROLL )
        ImpUpdateSrollBarVis( nWinStyle );
}

String ImpSvMEdit::GetText() const
{
    return mpTextWindow->GetTextEngine()->GetText();
}

String ImpSvMEdit::GetText( LineEnd aSeparator ) const
{
    return mpTextWindow->GetTextEngine()->GetText( aSeparator );
}

String ImpSvMEdit::GetTextLines() const
{
    return mpTextWindow->GetTextEngine()->GetTextLines();
}

String ImpSvMEdit::GetTextLines( LineEnd aSeparator ) const
{
    return mpTextWindow->GetTextEngine()->GetTextLines( aSeparator );
}

void ImpSvMEdit::Notify( SfxBroadcaster&, const SfxHint& rHint )
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
    sal_uLong nPara = 0;
    sal_uInt16 nChar = 0;
    sal_uInt16 x = 0;
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

const Selection& ImpSvMEdit::GetSelection() const
{
    maSelection = Selection();
    TextSelection aTextSel( mpTextWindow->GetTextView()->GetSelection() );
    aTextSel.Justify();
    // Selektion flachklopfen => jeder Umbruch ein Zeichen...

    ExtTextEngine* pExtTextEngine = mpTextWindow->GetTextEngine();
    // Absaetze davor:
    sal_uLong n;
    for ( n = 0; n < aTextSel.GetStart().GetPara(); n++ )
    {
        maSelection.Min() += pExtTextEngine->GetTextLen( n );
        maSelection.Min()++;
    }

    // Erster Absatz mit Selektion:
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

Size ImpSvMEdit::CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
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

void ImpSvMEdit::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    static const sal_Unicode sampleText[] = { 'x', '\0' };
    Size aOutSz = mpTextWindow->GetOutputSizePixel();
    Size aCharSz( mpTextWindow->GetTextWidth( sampleText ), mpTextWindow->GetTextHeight() );
    rnCols = (sal_uInt16) (aOutSz.Width()/aCharSz.Width());
    rnLines = (sal_uInt16) (aOutSz.Height()/aCharSz.Height());
}

void ImpSvMEdit::Enable( sal_Bool bEnable )
{
    mpTextWindow->Enable( bEnable );
    if ( mpHScrollBar )
        mpHScrollBar->Enable( bEnable );
    if ( mpVScrollBar )
        mpVScrollBar->Enable( bEnable );
}

sal_Bool ImpSvMEdit::HandleCommand( const CommandEvent& rCEvt )
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
    mbInMBDown = sal_False;
    mbSelectOnTab = sal_True;
    mbFocusSelectionHide = sal_False;
    mbIgnoreTab = sal_False;
    mbActivePopup = sal_False;
    mbSelectOnTab = sal_True;

    SetPointer( Pointer( POINTER_TEXT ) );

    mpExtTextEngine = new ExtTextEngine;
    mpExtTextEngine->SetMaxTextLen( STRING_MAXLEN );
    if( pParent->GetStyle() & WB_BORDER )
        mpExtTextEngine->SetLeftMargin( 2 );
    mpExtTextEngine->SetLocale( GetSettings().GetLocale() );
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
    mbInMBDown = sal_True;  // Dann im GetFocus nicht alles selektieren wird
    mpExtTextView->MouseButtonDown( rMEvt );
    Window::MouseButtonDown( rMEvt );
    GrabFocus();
    mbInMBDown = sal_False;
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
            // Damit die Selektion erhalten bleibt
            mbActivePopup = sal_True;
            XubString aChars = Edit::GetGetSpecialCharsFunction()( this, GetFont() );
            if ( aChars.Len() )
            {
                mpExtTextView->InsertText( aChars );
                mpExtTextView->GetTextEngine()->SetModified( sal_True );
            }
            mbActivePopup = sal_False;
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
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_HIDEDISABLED )
            pPopup->SetMenuFlags( MENU_FLAG_HIDEDISABLEDENTRIES );
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

        mbActivePopup = sal_True;
        Point aPos = rCEvt.GetMousePosPixel();
        if ( !rCEvt.IsMouseEvent() )
        {
            // !!! Irgendwann einmal Menu zentriert in der Selektion anzeigen !!!
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
                    XubString aChars = Edit::GetGetSpecialCharsFunction()( this, GetFont() );
                    if ( aChars.Len() )
                    {
                        mpExtTextView->InsertText( aChars );
                        mpExtTextEngine->SetModified( sal_True );
                        mpExtTextEngine->Broadcast( TextHint( TEXT_HINT_MODIFIED ) );
                    }
                }
                break;
        }
        mbActivePopup = sal_False;
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
            // Alles selektieren, aber nicht scrollen
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

// virtual
::css::uno::Reference< ::css::awt::XWindowPeer >
TextWindow::GetComponentInterface(sal_Bool bCreate)
{
    ::css::uno::Reference< ::css::awt::XWindowPeer > xPeer(
        Window::GetComponentInterface(false));
    if (!xPeer.is() && bCreate)
    {
        xPeer = new ::svt::TextWindowPeer(*GetTextView(), true);
        SetComponentInterface(xPeer);
    }
    return xPeer;
}

MultiLineEdit::MultiLineEdit( Window* pParent, WinBits nWinStyle )
    : Edit( pParent, nWinStyle )
{
    SetType( WINDOW_MULTILINEEDIT );
    pImpSvMEdit = new ImpSvMEdit( this, nWinStyle );
    ImplInitSettings( sal_True, sal_True, sal_True );
    pUpdateDataTimer = 0;

    SetCompoundControl( sal_True );
    SetStyle( ImplInitStyle( nWinStyle ) );
}

MultiLineEdit::MultiLineEdit( Window* pParent, const ResId& rResId )
    : Edit( pParent, rResId.SetRT( RSC_MULTILINEEDIT ) )
{
    SetType( WINDOW_MULTILINEEDIT );
    WinBits nWinStyle = rResId.GetWinBits();
    pImpSvMEdit = new ImpSvMEdit( this, nWinStyle );
    ImplInitSettings( sal_True, sal_True, sal_True );
    pUpdateDataTimer = 0;

    sal_uInt16 nMaxLen = Edit::GetMaxTextLen();
    if ( nMaxLen )
        SetMaxTextLen( nMaxLen );

    SetText( Edit::GetText() );

    if ( IsVisible() )
        pImpSvMEdit->Resize();

    SetCompoundControl( sal_True );
    SetStyle( ImplInitStyle( nWinStyle ) );

    // Base Edit ctor could call Show already, but that would cause problems
    // with accessibility, as Show might (indirectly) trigger a call to virtual
    // GetComponentInterface, which is the Edit's base version instead of the
    // MultiLineEdit's version while in the base Edit ctor:
    if ((GetStyle() & WB_HIDE) == 0)
        Show();
}

MultiLineEdit::~MultiLineEdit()
{
    {
        ::std::auto_ptr< ImpSvMEdit > pDelete( pImpSvMEdit );
        pImpSvMEdit = NULL;
    }
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


void MultiLineEdit::ImplInitSettings( sal_Bool /*bFont*/, sal_Bool /*bForeground*/, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // Der Font muss immer mit manipuliert werden, weil die TextEngine
    // sich nicht um TextColor/Background kuemmert

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
    pImpSvMEdit->GetTextWindow()->SetFont( TheFont );
    pImpSvMEdit->GetTextWindow()->GetTextEngine()->SetFont( TheFont );
    pImpSvMEdit->GetTextWindow()->SetTextColor( aTextColor );

    if ( bBackground )
    {
        if( IsPaintTransparent() )
        {
            pImpSvMEdit->GetTextWindow()->SetPaintTransparent( sal_True );
            pImpSvMEdit->GetTextWindow()->SetBackground();
            pImpSvMEdit->GetTextWindow()->SetControlBackground();
            SetBackground();
            SetControlBackground();
        }
        else
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
}

void MultiLineEdit::Modify()
{
    aModifyHdlLink.Call( this );

    CallEventListeners( VCLEVENT_EDIT_MODIFY );

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
    pImpSvMEdit->SetModified( sal_True );
}

void MultiLineEdit::ClearModifyFlag()
{
    pImpSvMEdit->SetModified( sal_False );
}

sal_Bool MultiLineEdit::IsModified() const
{
    return pImpSvMEdit->IsModified();
}

void MultiLineEdit::EnableUpdateData( sal_uLong nTimeout )
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

void MultiLineEdit::SetReadOnly( sal_Bool bReadOnly )
{
    pImpSvMEdit->SetReadOnly( bReadOnly );
    Edit::SetReadOnly( bReadOnly );

    // #94921# ReadOnly can be overwritten in InitFromStyle() when WB not set.
    WinBits nStyle = GetStyle();
    if ( bReadOnly )
        nStyle |= WB_READONLY;
    else
        nStyle &= ~WB_READONLY;
    SetStyle( nStyle );
}

sal_Bool MultiLineEdit::IsReadOnly() const
{
    return pImpSvMEdit->IsReadOnly();
}

void MultiLineEdit::SetMaxTextLen( xub_StrLen nMaxLen )
{
    pImpSvMEdit->SetMaxTextLen( nMaxLen );
}

xub_StrLen MultiLineEdit::GetMaxTextLen() const
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

String MultiLineEdit::GetSelected( LineEnd aSeparator ) const
{
    return pImpSvMEdit->GetSelected( aSeparator );
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

String MultiLineEdit::GetText( LineEnd aSeparator ) const
{
    return pImpSvMEdit->GetText( aSeparator );
}

String MultiLineEdit::GetTextLines() const
{
    return pImpSvMEdit->GetTextLines();
}

String MultiLineEdit::GetTextLines(  LineEnd aSeparator ) const
{
    return pImpSvMEdit->GetTextLines( aSeparator );
}

void MultiLineEdit::Resize()
{
    pImpSvMEdit->Resize();
}

void MultiLineEdit::GetFocus()
{
    if ( !pImpSvMEdit )  // might be called from within the dtor, when pImpSvMEdit == NULL is a valid state
        return;

    Edit::GetFocus();
    pImpSvMEdit->GetFocus();
}

void MultiLineEdit::SetSelection( const Selection& rSelection )
{
    pImpSvMEdit->SetSelection( rSelection );
}

const Selection& MultiLineEdit::GetSelection() const
{
    return pImpSvMEdit->GetSelection();
}

Size MultiLineEdit::CalcMinimumSize() const
{
    Size aSz = pImpSvMEdit->CalcMinimumSize();

    sal_Int32 nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Width() += nLeft+nRight;
    aSz.Height() += nTop+nBottom;

    return aSz;
}

Size MultiLineEdit::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    sal_Int32 nLeft, nTop, nRight, nBottom;
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

Size MultiLineEdit::CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
{
    Size aSz = pImpSvMEdit->CalcSize( nColumns, nLines );

    sal_Int32 nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Width() += nLeft+nRight;
    aSz.Height() += nTop+nBottom;
    return aSz;
}

void MultiLineEdit::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    pImpSvMEdit->GetMaxVisColumnsAndLines( rnCols, rnLines );
}

void MultiLineEdit::StateChanged( StateChangedType nType )
{
    if( nType == STATE_CHANGE_ENABLE )
    {
        pImpSvMEdit->Enable( IsEnabled() );
        ImplInitSettings( sal_True, sal_False, sal_False );
    }
    else if( nType == STATE_CHANGE_READONLY )
    {
        pImpSvMEdit->SetReadOnly( IsReadOnly() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        pImpSvMEdit->GetTextWindow()->SetZoom( GetZoom() );
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
        pImpSvMEdit->InitFromStyle( GetStyle() );
        SetStyle( ImplInitStyle( GetStyle() ) );
    }
    else if ( nType == STATE_CHANGE_INITSHOW )
    {
        if( IsPaintTransparent() )
        {
            pImpSvMEdit->GetTextWindow()->SetPaintTransparent( sal_True );
            pImpSvMEdit->GetTextWindow()->SetBackground();
            pImpSvMEdit->GetTextWindow()->SetControlBackground();
            SetBackground();
            SetControlBackground();
        }
    }

    Control::StateChanged( nType );
}

void MultiLineEdit::DataChanged( const DataChangedEvent& rDCEvt )
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

void MultiLineEdit::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    ImplInitSettings( sal_True, sal_True, sal_True );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = pImpSvMEdit->GetTextWindow()->GetDrawPixelFont( pDev );
    aFont.SetTransparent( sal_True );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    sal_Bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    sal_Bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
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

void MultiLineEdit::EnableFocusSelectionHide( sal_Bool bHide )
{
    pImpSvMEdit->GetTextWindow()->SetAutoFocusHide( bHide );
}

sal_Bool MultiLineEdit::IsFocusSelectionHideEnabled() const
{
    return pImpSvMEdit->GetTextWindow()->IsAutoFocusHide();
}


void MultiLineEdit::SetLeftMargin( sal_uInt16 n )
{
    if ( GetTextEngine() )
        GetTextEngine()->SetLeftMargin( n );
}

sal_uInt16 MultiLineEdit::GetLeftMargin() const
{
    if ( GetTextEngine() )
        return GetTextEngine()->GetLeftMargin();
    else
        return 0;
}

void MultiLineEdit::SetRightToLeft( sal_Bool bRightToLeft )
{
    if ( GetTextEngine() )
    {
        GetTextEngine()->SetRightToLeft( bRightToLeft );
        GetTextView()->ShowCursor();
    }
}

sal_Bool MultiLineEdit::IsRightToLeft() const
{
    sal_Bool bRightToLeft = sal_False;

    if ( GetTextEngine() )
        bRightToLeft = GetTextEngine()->IsRightToLeft();

    return bRightToLeft;
}

// virtual
::css::uno::Reference< ::css::awt::XWindowPeer >
MultiLineEdit::GetComponentInterface(sal_Bool bCreate)
{
    ::css::uno::Reference< ::css::awt::XWindowPeer > xPeer(
        Edit::GetComponentInterface(false));
    if (!xPeer.is() && bCreate)
    {
        ::std::auto_ptr< VCLXMultiLineEdit > xEdit(new VCLXMultiLineEdit());
        xEdit->SetWindow(this);
        xPeer = xEdit.release();
        SetComponentInterface(xPeer);
    }
    return xPeer;
}
/*-- 11.08.2004 11:29:23---------------------------------------------------

  -----------------------------------------------------------------------*/
void MultiLineEdit::DisableSelectionOnFocus()
{
    pImpSvMEdit->GetTextWindow()->DisableSelectionOnFocus();
}
