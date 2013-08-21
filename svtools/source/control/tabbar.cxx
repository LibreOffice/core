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


#include <svtools/tabbar.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <tools/poly.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/decoview.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/image.hxx>
#include "svtaccessiblefactory.hxx"
#include <filectrl.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <limits>

// =======================================================================

#define TABBAR_OFFSET_X         7
#define TABBAR_OFFSET_X2        2
#define TABBAR_DRAG_SCROLLOFF   5
#define TABBAR_MINSIZE          5

const sal_uInt16 ADDNEWPAGE_AREAWIDTH = 10;

// =======================================================================

struct ImplTabBarItem
{
    sal_uInt16      mnId;
    TabBarPageBits  mnBits;
    OUString        maText;
    OUString        maHelpText;
    Rectangle       maRect;
    long            mnWidth;
    OString    maHelpId;
    sal_Bool            mbShort;
    sal_Bool            mbSelect;
    sal_Bool            mbEnable;
    Color           maTabBgColor;
    Color           maTabTextColor;

                    ImplTabBarItem( sal_uInt16 nItemId, const OUString& rText,
                                    TabBarPageBits nPageBits ) :
                        maText( rText )
                    {
                        mnId     = nItemId;
                        mnBits   = nPageBits;
                        mnWidth  = 0;
                        mbShort  = sal_False;
                        mbSelect = sal_False;
                        mbEnable = sal_True;
                        maTabBgColor = Color( COL_AUTO );
                        maTabTextColor = Color( COL_AUTO );
                    }

    bool IsDefaultTabBgColor() const
    {
        return maTabBgColor == Color(COL_AUTO);
    }

    bool IsDefaultTabTextColor() const
    {
        return maTabTextColor == Color(COL_AUTO);
    }

    bool IsSelected(ImplTabBarItem* pCurItem) const
    {
        return mbSelect || (pCurItem == this);
    }
};

// =======================================================================

// -----------------
// - ImplTabButton -
// -----------------

class ImplTabButton : public PushButton
{
public:
                    ImplTabButton( TabBar* pParent, WinBits nWinStyle = 0 ) :
                        PushButton( pParent, nWinStyle | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOLIGHTBORDER | WB_NOPOINTERFOCUS  ) {}

    TabBar*         GetParent() const { return (TabBar*)Window::GetParent(); }

    virtual long    PreNotify( NotifyEvent& rNEvt );
};

// =======================================================================

long ImplTabButton::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        if ( GetParent()->IsInEditMode() )
        {
            GetParent()->EndEditMode();
            return sal_True;
        }
    }

    return PushButton::PreNotify( rNEvt );
}

// =======================================================================

// ----------------
// - ImplTabSizer -
// ----------------

class ImplTabSizer : public Window
{
public:
                    ImplTabSizer( TabBar* pParent, WinBits nWinStyle = 0 );

    TabBar*         GetParent() const { return (TabBar*)Window::GetParent(); }

private:
    void            ImplTrack( const Point& rScreenPos );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    Paint( const Rectangle& rRect );

    Point           maStartPos;
    long            mnStartWidth;
};

// -----------------------------------------------------------------------

ImplTabSizer::ImplTabSizer( TabBar* pParent, WinBits nWinStyle )
    : Window( pParent, nWinStyle & WB_3DLOOK )
    , mnStartWidth(0)
{
    SetPointer( Pointer( POINTER_HSIZEBAR ) );
    SetSizePixel( Size( 7, 0 ) );
}

// -----------------------------------------------------------------------

void ImplTabSizer::ImplTrack( const Point& rScreenPos )
{
    TabBar* pParent = GetParent();
    long nDiff = rScreenPos.X() - maStartPos.X();
    pParent->mnSplitSize = mnStartWidth + (pParent->IsMirrored() ? -nDiff : nDiff);
    if ( pParent->mnSplitSize < TABBAR_MINSIZE )
        pParent->mnSplitSize = TABBAR_MINSIZE;
    pParent->Split();
    pParent->Update();
}

// -----------------------------------------------------------------------

void ImplTabSizer::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( GetParent()->IsInEditMode() )
    {
        GetParent()->EndEditMode();
        return;
    }

    if ( rMEvt.IsLeft() )
    {
        maStartPos = OutputToScreenPixel( rMEvt.GetPosPixel() );
        mnStartWidth = GetParent()->GetSizePixel().Width();
        StartTracking();
    }
}

// -----------------------------------------------------------------------

void ImplTabSizer::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( rTEvt.IsTrackingCanceled() )
            ImplTrack( maStartPos );
        GetParent()->mnSplitSize = 0;
    }
    else
        ImplTrack( OutputToScreenPixel( rTEvt.GetMouseEvent().GetPosPixel() ) );
}

// -----------------------------------------------------------------------

void ImplTabSizer::Paint( const Rectangle& )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    DecorationView  aDecoView( this );
    long            nOffX = 0;
    Size            aOutputSize = GetOutputSizePixel();

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        SetLineColor( rStyleSettings.GetDarkShadowColor() );
        DrawLine( Point( 0, 0 ), Point( 0, aOutputSize.Height()-1 ) );
        nOffX++;
        aOutputSize.Width()--;
    }
    aDecoView.DrawButton( Rectangle( Point( nOffX, 0 ), aOutputSize ), BUTTON_DRAW_NOLIGHTBORDER );
}

// =======================================================================

// Heisst nicht Impl, da evtl. mal von aussen benutz- und ueberladbar

// --------------
// - TabBarEdit -
// --------------

class TabBarEdit : public Edit
{
private:
    Timer           maLoseFocusTimer;
    sal_Bool            mbPostEvt;

                    DECL_LINK( ImplEndEditHdl, void* );
                    DECL_LINK( ImplEndTimerHdl, void* );

public:
                    TabBarEdit( TabBar* pParent, WinBits nWinStyle = 0 );

    TabBar*         GetParent() const { return (TabBar*)Window::GetParent(); }

    void            SetPostEvent() { mbPostEvt = sal_True; }
    void            ResetPostEvent() { mbPostEvt = sal_False; }

    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual void    LoseFocus();
};

// -----------------------------------------------------------------------

TabBarEdit::TabBarEdit( TabBar* pParent, WinBits nWinStyle ) :
    Edit( pParent, nWinStyle )
{
    mbPostEvt = sal_False;
}

// -----------------------------------------------------------------------

long TabBarEdit::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        if ( !pKEvt->GetKeyCode().GetModifier() )
        {
            if ( pKEvt->GetKeyCode().GetCode() == KEY_RETURN )
            {
                if ( !mbPostEvt )
                {
                    if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), (void*)sal_False ) )
                        mbPostEvt = sal_True;
                }
                return sal_True;
            }
            else if ( pKEvt->GetKeyCode().GetCode() == KEY_ESCAPE )
            {
                if ( !mbPostEvt )
                {
                    if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), (void*)sal_True ) )
                        mbPostEvt = sal_True;
                }
                return sal_True;
            }
        }
    }

    return Edit::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void TabBarEdit::LoseFocus()
{
    if ( !mbPostEvt )
    {
        if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), (void*)sal_False ) )
            mbPostEvt = sal_True;
    }

    Edit::LoseFocus();
}

// -----------------------------------------------------------------------

IMPL_LINK( TabBarEdit, ImplEndEditHdl, void*, pCancel )
{
    ResetPostEvent();
    maLoseFocusTimer.Stop();

    // We need this query, because the edit get a losefous event,
    // when it shows the context menu or the insert symbol dialog
    if ( !HasFocus() && HasChildPathFocus( sal_True ) )
    {
        maLoseFocusTimer.SetTimeout( 30 );
        maLoseFocusTimer.SetTimeoutHdl( LINK( this, TabBarEdit, ImplEndTimerHdl ) );
        maLoseFocusTimer.Start();
    }
    else
        GetParent()->EndEditMode( pCancel != 0 );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(TabBarEdit, ImplEndTimerHdl)
{
    if ( HasFocus() )
        return 0;

    // We need this query, because the edit get a losefous event,
    // when it shows the context menu or the insert symbol dialog
    if ( HasChildPathFocus( sal_True ) )
        maLoseFocusTimer.Start();
    else
        GetParent()->EndEditMode( sal_True );

    return 0;
}

// =======================================================================
struct TabBar_Impl
{
    ImplTabSizer*                   mpSizer;
    ::svt::AccessibleFactoryAccess  maAccessibleFactory;

    TabBar_Impl()
        :mpSizer( NULL )
    {
    }
    ~TabBar_Impl()
    {
        delete mpSizer;
    }
};

// =======================================================================

const sal_uInt16 TabBar::APPEND         = ::std::numeric_limits<sal_uInt16>::max();
const sal_uInt16 TabBar::PAGE_NOT_FOUND = ::std::numeric_limits<sal_uInt16>::max();
const sal_uInt16 TabBar::INSERT_TAB_POS = ::std::numeric_limits<sal_uInt16>::max() - 1;

void TabBar::ImplInit( WinBits nWinStyle )
{
    mpItemList      = new ImplTabBarList;
    mpFirstBtn      = NULL;
    mpPrevBtn       = NULL;
    mpNextBtn       = NULL;
    mpLastBtn       = NULL;
    mpImpl          = new TabBar_Impl;
    mpEdit          = NULL;
    mnMaxPageWidth  = 0;
    mnCurMaxWidth   = 0;
    mnOffX          = 0;
    mnOffY          = 0;
    mnLastOffX      = 0;
    mnSplitSize     = 0;
    mnSwitchTime    = 0;
    mnWinStyle      = nWinStyle;
    mnCurPageId     = 0;
    mnFirstPos      = 0;
    mnDropPos       = 0;
    mnSwitchId      = 0;
    mnEditId        = 0;
    mbFormat        = sal_True;
    mbFirstFormat   = sal_True;
    mbSizeFormat    = sal_True;
    mbAutoMaxWidth  = sal_True;
    mbInSwitching   = sal_False;
    mbAutoEditMode  = sal_False;
    mbEditCanceled  = sal_False;
    mbDropPos       = sal_False;
    mbInSelect      = sal_False;
    mbSelColor      = sal_False;
    mbSelTextColor  = sal_False;
    mbMirrored      = sal_False;

    if ( nWinStyle & WB_3DTAB )
        mnOffY++;

    ImplInitControls();

    if(mpFirstBtn)
        mpFirstBtn->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVET0HOME));
    if(mpPrevBtn)
        mpPrevBtn->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVELEFT));
    if(mpNextBtn)
        mpNextBtn->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVERIGHT));
    if(mpLastBtn)
        mpLastBtn->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVETOEND));

    SetSizePixel( Size( 100, CalcWindowSizePixel().Height() ) );
    ImplInitSettings( sal_True, sal_True );
}

// -----------------------------------------------------------------------

TabBar::TabBar( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, (nWinStyle & WB_3DLOOK) | WB_CLIPCHILDREN )
{
    ImplInit( nWinStyle );
    maCurrentItemList = 0;
}

// -----------------------------------------------------------------------

TabBar::~TabBar()
{
    EndEditMode( sal_True );

    // Controls loeschen
    if ( mpPrevBtn )
        delete mpPrevBtn;
    if ( mpNextBtn )
        delete mpNextBtn;
    if ( mpFirstBtn )
        delete mpFirstBtn;
    if ( mpLastBtn )
        delete mpLastBtn;
    delete mpImpl;

    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        delete (*mpItemList)[ i ];
    }
    delete mpItemList;
}

// -----------------------------------------------------------------------
ImplTabBarItem* TabBar::seek( size_t i )
{
    if ( i < mpItemList->size() )
    {
        maCurrentItemList = i;
        return (*mpItemList)[ maCurrentItemList ];
    }
    return NULL;
}

ImplTabBarItem* TabBar::prev()
{
    if ( maCurrentItemList > 0 ) {
        return (*mpItemList)[ --maCurrentItemList ];
    }
    return NULL;
}

ImplTabBarItem* TabBar::next()
{
    if ( maCurrentItemList+1 < mpItemList->size() ) {
        return (*mpItemList)[ ++maCurrentItemList ];
    }
    return NULL;
}

// -----------------------------------------------------------------------

void TabBar::ImplInitSettings( sal_Bool bFont, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aToolFont;
        aToolFont = rStyleSettings.GetToolFont();
        if ( IsControlFont() )
            aToolFont.Merge( GetControlFont() );
        aToolFont.SetWeight( WEIGHT_BOLD );
        SetZoomedPointFont( aToolFont );

        // Font in der groesse Anpassen, wenn Fenster zu klein?
        while ( GetTextHeight() > (GetOutputSizePixel().Height()-1) )
        {
            Font aFont = GetFont();
            if ( aFont.GetHeight() <= 6 )
                break;
            aFont.SetHeight( aFont.GetHeight()-1 );
            SetFont( aFont );
        }
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = rStyleSettings.GetFaceColor();
        SetBackground( aColor );
    }
}

// -----------------------------------------------------------------------

void TabBar::ImplGetColors( Color& rFaceColor, Color& rFaceTextColor,
                            Color& rSelectColor, Color& rSelectTextColor )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( IsControlBackground() )
        rFaceColor = GetControlBackground();
    else
        rFaceColor = rStyleSettings.GetInactiveTabColor();
    if ( IsControlForeground() )
        rFaceTextColor = GetControlForeground();
    else
        rFaceTextColor = rStyleSettings.GetButtonTextColor();
    if ( mbSelColor )
        rSelectColor = maSelColor;
    else
        rSelectColor = rStyleSettings.GetActiveTabColor();
    if ( mbSelTextColor )
        rSelectTextColor = maSelTextColor;
    else
        rSelectTextColor = rStyleSettings.GetWindowTextColor();

    // Bei 3D-Tabs wird Selektions- und Face-Farbe umgedreht, da die
    // selektierten Tabs in 3D erscheinen sollen
    if ( mnWinStyle & WB_3DTAB )
    {
        Color aTempColor = rFaceColor;
        rFaceColor = rSelectColor;
        rSelectColor = aTempColor;
        aTempColor = rFaceTextColor;
        rFaceTextColor = rSelectTextColor;
        rSelectTextColor = rFaceTextColor;
    }
}

// -----------------------------------------------------------------------

sal_Bool TabBar::ImplCalcWidth()
{
    // Groessen muessen nur ermittelt werden, wenn sich Text aendert oder
    // wenn der Font geaendert wurde
    if ( !mbSizeFormat )
        return sal_False;

    // Breiten der Tabs mit dem fetten Font ermitteln
    Font aFont = GetFont();
    if ( aFont.GetWeight() != WEIGHT_BOLD )
    {
        aFont.SetWeight( WEIGHT_BOLD );
        SetFont( aFont );
    }

    if ( mnMaxPageWidth )
        mnCurMaxWidth = mnMaxPageWidth;
    else if ( mbAutoMaxWidth )
    {
        mnCurMaxWidth = mnLastOffX-mnOffX-
                        TABBAR_OFFSET_X-TABBAR_OFFSET_X-
                        TABBAR_OFFSET_X2-TABBAR_OFFSET_X2-TABBAR_OFFSET_X2;
        if ( mnCurMaxWidth < 1 )
            mnCurMaxWidth = 1;
    }
    else
        mnCurMaxWidth = 0;

    sal_Bool            bChanged = sal_False;
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i )
    {
        ImplTabBarItem* pItem = (*mpItemList)[ i ];
        long nNewWidth = GetTextWidth( pItem->maText );
        if ( mnCurMaxWidth && (nNewWidth > mnCurMaxWidth) )
        {
            pItem->mbShort = sal_True;
            nNewWidth = mnCurMaxWidth;
        }
        else
            pItem->mbShort = sal_False;
        nNewWidth += TABBAR_OFFSET_X+TABBAR_OFFSET_X2;
        if ( pItem->mnWidth != nNewWidth )
        {
            pItem->mnWidth = nNewWidth;
            if ( !pItem->maRect.IsEmpty() )
                bChanged = sal_True;
        }
    }
    mbSizeFormat = sal_False;
    mbFormat = sal_True;
    return bChanged;
}

// -----------------------------------------------------------------------

void TabBar::ImplFormat()
{
    ImplCalcWidth();

    if ( !mbFormat )
        return;

    sal_uInt16 n = 0;
    long x = mnOffX;
    for ( size_t i = 0, nL = mpItemList->size(); i < nL; ++i )
    {
        ImplTabBarItem* pItem = (*mpItemList)[ i ];
        // Bei allen nicht sichtbaren Tabs, wird ein leeres Rechteck
        // gesetzt
        if ( (n+1 < mnFirstPos) || (x > mnLastOffX) )
            pItem->maRect.SetEmpty();
        else
        {
            // Etwas von der Tab vor der ersten sichtbaren Page
            // muss auch zu sehen sein
            if ( n+1 == mnFirstPos )
                pItem->maRect.Left() = x-pItem->mnWidth;
            else
            {
                pItem->maRect.Left() = x;
                x += pItem->mnWidth;
            }
            pItem->maRect.Right() = x+TABBAR_OFFSET_X+TABBAR_OFFSET_X2;
            pItem->maRect.Bottom() = maWinSize.Height()-1;

            if( mbMirrored )
            {
                long nTmp = mnOffX + mnLastOffX - pItem->maRect.Right();
                pItem->maRect.Right() = mnOffX + mnLastOffX - pItem->maRect.Left();
                pItem->maRect.Left() = nTmp;
            }
        }

        n++;
    }

    mbFormat = sal_False;

    // Button enablen/disablen
    ImplEnableControls();
}

// -----------------------------------------------------------------------

sal_uInt16 TabBar::ImplGetLastFirstPos()
{
    sal_uInt16 nCount = (sal_uInt16)(mpItemList->size());
    if ( !nCount || mbSizeFormat || mbFormat )
        return 0;

    sal_uInt16  nLastFirstPos = nCount-1;
    long    nWinWidth = mnLastOffX-mnOffX-TABBAR_OFFSET_X-ADDNEWPAGE_AREAWIDTH;
    long    nWidth = (*mpItemList)[ nLastFirstPos ]->mnWidth;
    while ( nLastFirstPos && (nWidth < nWinWidth) )
    {
        nLastFirstPos--;
        nWidth += (*mpItemList)[ nLastFirstPos ]->mnWidth;
    }
    if ( (nLastFirstPos != (sal_uInt16)(mpItemList->size()-1)) &&
         (nWidth > nWinWidth) )
        nLastFirstPos++;
    return nLastFirstPos;
}

// -----------------------------------------------------------------------

void TabBar::ImplInitControls()
{
    if ( mnWinStyle & WB_SIZEABLE )
    {
        if ( !mpImpl->mpSizer )
            mpImpl->mpSizer = new ImplTabSizer( this, mnWinStyle & (WB_DRAG | WB_3DLOOK) );
        mpImpl->mpSizer->Show();
    }
    else
    {
        DELETEZ( mpImpl->mpSizer );
    }

    Link aLink = LINK( this, TabBar, ImplClickHdl );

    if ( mnWinStyle & (WB_MINSCROLL | WB_SCROLL) )
    {
        if ( !mpPrevBtn )
        {
            mpPrevBtn = new ImplTabButton( this, WB_REPEAT );
            mpPrevBtn->SetClickHdl( aLink );
        }
        mpPrevBtn->SetSymbol( mbMirrored ? SYMBOL_NEXT : SYMBOL_PREV );
        mpPrevBtn->Show();

        if ( !mpNextBtn )
        {
            mpNextBtn = new ImplTabButton( this, WB_REPEAT );
            mpNextBtn->SetClickHdl( aLink );
        }
        mpNextBtn->SetSymbol( mbMirrored ? SYMBOL_PREV : SYMBOL_NEXT );
        mpNextBtn->Show();
    }
    else
    {
        DELETEZ( mpPrevBtn );
        DELETEZ( mpNextBtn );
    }

    if ( mnWinStyle & WB_SCROLL )
    {
        if ( !mpFirstBtn )
        {
            mpFirstBtn = new ImplTabButton( this );
            mpFirstBtn->SetClickHdl( aLink );
        }
        mpFirstBtn->SetSymbol( mbMirrored ? SYMBOL_LAST : SYMBOL_FIRST );
        mpFirstBtn->Show();

        if ( !mpLastBtn )
        {
            mpLastBtn = new ImplTabButton( this );
            mpLastBtn->SetClickHdl( aLink );
        }
        mpLastBtn->SetSymbol( mbMirrored ? SYMBOL_FIRST : SYMBOL_LAST );
        mpLastBtn->Show();
    }
    else
    {
        DELETEZ( mpFirstBtn );
        DELETEZ( mpLastBtn );
    }

    mbHasInsertTab  = (mnWinStyle & WB_INSERTTAB);
}

// -----------------------------------------------------------------------

void TabBar::ImplEnableControls()
{
    if ( mbSizeFormat || mbFormat )
        return;

    // Buttons enablen/disblen
    sal_Bool bEnableBtn = mnFirstPos > 0;
    if ( mpFirstBtn )
        mpFirstBtn->Enable( bEnableBtn );
    if ( mpPrevBtn )
        mpPrevBtn->Enable( bEnableBtn );

    bEnableBtn = mnFirstPos < ImplGetLastFirstPos();
    if ( mpNextBtn )
        mpNextBtn->Enable( bEnableBtn );
    if ( mpLastBtn )
        mpLastBtn->Enable( bEnableBtn );
}

// -----------------------------------------------------------------------

void TabBar::ImplShowPage( sal_uInt16 nPos )
{
    // Breite berechnen
    long nWidth = GetOutputSizePixel().Width();
    if ( nWidth >= TABBAR_OFFSET_X )
        nWidth -= TABBAR_OFFSET_X;
    ImplTabBarItem* pItem = (*mpItemList)[ nPos ];
    if ( nPos < mnFirstPos )
        SetFirstPageId( pItem->mnId );
    else if ( pItem->maRect.Right() > nWidth )
    {
        while ( pItem->maRect.Right() > nWidth )
        {
            sal_uInt16 nNewPos = mnFirstPos+1;
            SetFirstPageId( GetPageId( nNewPos ) );
            ImplFormat();
            if ( nNewPos != mnFirstPos )
                break;
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( TabBar, ImplClickHdl, ImplTabButton*, pBtn )
{
    EndEditMode();

    sal_uInt16 nNewPos = mnFirstPos;

    if ( pBtn == mpFirstBtn )
        nNewPos = 0;
    else if ( pBtn == mpPrevBtn )
    {
        if ( mnFirstPos )
            nNewPos = mnFirstPos-1;
    }
    else if ( pBtn == mpNextBtn )
    {
        sal_uInt16 nCount = GetPageCount();
        if ( mnFirstPos <  nCount )
            nNewPos = mnFirstPos+1;
    }
    else
    {
        sal_uInt16 nCount = GetPageCount();
        if ( nCount )
            nNewPos = nCount-1;
    }

    if ( nNewPos != mnFirstPos )
        SetFirstPageId( GetPageId( nNewPos ) );
    return 0;
}

// -----------------------------------------------------------------------

void TabBar::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeaveWindow() )
        mbInSelect = sal_False;

    Window::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void TabBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    // Bei Klick in unser Fenster EditModus nur beenden und Klick nicht
    // ausfuehren
    if ( IsInEditMode() )
    {
        EndEditMode();
        return;
    }

    ImplTabBarItem* pItem;
    sal_uInt16          nSelId = GetPageId( rMEvt.GetPosPixel() );

    if ( !rMEvt.IsLeft() )
    {
        Window::MouseButtonDown( rMEvt );
        if ( (nSelId > 0) && (nSelId != mnCurPageId) )
        {
            sal_uInt16 nPos = GetPagePos( nSelId );
            pItem = (*mpItemList)[ nPos ];

            if ( pItem->mbEnable )
            {
                if ( ImplDeactivatePage() )
                {
                    SetCurPageId( nSelId );
                    Update();
                    ImplActivatePage();
                    ImplSelect();
                }
                mbInSelect = sal_True;
            }
        }
        return;
    }

    if ( rMEvt.IsMod2() && mbAutoEditMode && nSelId )
    {
        if ( StartEditMode( nSelId ) )
            return;
    }

    if ( (rMEvt.GetMode() & (MOUSE_MULTISELECT | MOUSE_RANGESELECT)) && (rMEvt.GetClicks() == 1) )
    {
        if ( nSelId )
        {
            sal_uInt16      nPos = GetPagePos( nSelId );
            sal_Bool        bSelectTab = sal_False;
            pItem = (*mpItemList)[ nPos ];

            if ( pItem->mbEnable )
            {
                if ( (rMEvt.GetMode() & MOUSE_MULTISELECT) && (mnWinStyle & WB_MULTISELECT) )
                {
                    if ( nSelId != mnCurPageId )
                    {
                        SelectPage( nSelId, !IsPageSelected( nSelId ) );
                        bSelectTab = sal_True;
                    }
                }
                else if ( mnWinStyle & (WB_MULTISELECT | WB_RANGESELECT) )
                {
                    bSelectTab = sal_True;
                    sal_uInt16 n;
                    sal_Bool   bSelect;
                    sal_uInt16 nCurPos = GetPagePos( mnCurPageId );
                    if ( nPos <= nCurPos )
                    {
                        // Alle Tabs bis zur angeklickten Tab deselektieren
                        // und alle Tabs von der angeklickten Tab bis
                        // zur aktuellen Position selektieren
                        n = 0;
                        while ( n < nCurPos )
                        {
                            pItem = (*mpItemList)[ n ];
                            if ( n < nPos )
                                bSelect = sal_False;
                            else
                                bSelect = sal_True;

                            if ( pItem->mbSelect != bSelect )
                            {
                                pItem->mbSelect = bSelect;
                                if ( !pItem->maRect.IsEmpty() )
                                    Invalidate( pItem->maRect );
                            }

                            n++;
                        }
                    }

                    if ( nPos >= nCurPos )
                    {
                        // Alle Tabs von der aktuellen bis zur angeklickten
                        // Tab selektieren und alle Tabs von der angeklickten
                        // Tab bis zur letzten Tab deselektieren
                        sal_uInt16 nCount = (sal_uInt16)mpItemList->size();
                        n = nCurPos;
                        while ( n < nCount )
                        {
                            pItem = (*mpItemList)[ n ];

                            if ( n <= nPos )
                                bSelect = sal_True;
                            else
                                bSelect = sal_False;

                            if ( pItem->mbSelect != bSelect )
                            {
                                pItem->mbSelect = bSelect;
                                if ( !pItem->maRect.IsEmpty() )
                                    Invalidate( pItem->maRect );
                            }

                            n++;
                        }
                    }
                }

                // Gegebenenfalls muss die selektierte Tab gescrollt werden
                if ( bSelectTab )
                {
                    ImplShowPage( nPos );
                    Update();
                    ImplSelect();
                }
            }
            else
                ImplShowPage( nPos );
            mbInSelect = sal_True;

            return;
        }
    }
    else if ( rMEvt.GetClicks() == 2 )
    {
        // Gegebenenfalls den Double-Click-Handler rufen
        if ( !rMEvt.GetModifier() && (!nSelId || (nSelId == mnCurPageId)) )
        {
            sal_uInt16 nOldCurId = mnCurPageId;
            mnCurPageId = nSelId;
            DoubleClick();
            // Abfrage, da im DoubleClick-Handler die aktuelle Seite
            // umgeschaltet werden konnte
            if ( mnCurPageId == nSelId )
                mnCurPageId = nOldCurId;
        }

        return;
    }
    else
    {
        if ( nSelId )
        {
            // Nur Select ausfuehren, wenn noch nicht aktuelle Page
            if ( nSelId != mnCurPageId )
            {
                sal_uInt16 nPos = GetPagePos( nSelId );
                pItem = (*mpItemList)[ nPos ];

                if ( pItem->mbEnable )
                {
                    if ( !pItem->mbSelect )
                    {
                        // Muss invalidiert werden
                        sal_Bool bUpdate = sal_False;
                        if ( IsReallyVisible() && IsUpdateMode() )
                            bUpdate = sal_True;

                        // Alle selektierten Items deselektieren
                        for ( size_t i = 0, n = mpItemList->size(); i < n; ++i )
                        {
                            pItem = (*mpItemList)[ i ];
                            if ( pItem->mbSelect || (pItem->mnId == mnCurPageId) )
                            {
                                pItem->mbSelect = sal_False;
                                if ( bUpdate )
                                    Invalidate( pItem->maRect );
                            }
                        }
                    }

                    if ( ImplDeactivatePage() )
                    {
                        SetCurPageId( nSelId );
                        Update();
                        ImplActivatePage();
                        ImplSelect();
                    }
                }
                else
                    ImplShowPage( nPos );
                mbInSelect = sal_True;
            }

            return;
        }
    }

    Window::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void TabBar::MouseButtonUp( const MouseEvent& rMEvt )
{
    mbInSelect = sal_False;
    Window::MouseButtonUp( rMEvt );
}


// -----------------------------------------------------------------------

namespace {

class TabBarPaintGuard
{
public:
    explicit TabBarPaintGuard(TabBar& rParent) :
        mrParent(rParent),
        maFont(rParent.GetFont())
    {
        // #i36013# exclude push buttons from painting area
        mrParent.SetClipRegion( Region(mrParent.GetPageArea()) );
    }

    ~TabBarPaintGuard()
    {
        // Restore original font.
        mrParent.SetFont(maFont);
        // remove clip region
        mrParent.SetClipRegion();
    }
private:
    TabBar& mrParent;
    Font    maFont;
};

class TabDrawer
{
public:

    explicit TabDrawer(TabBar& rParent) :
        mrParent(rParent),
        mpStyleSettings(&mrParent.GetSettings().GetStyleSettings()),
        maPoly(4),
        mbSelected(false),
        mbCustomColored(false),
        mbSpecialTab(false),
        mbEnabled(false)
    {
    }

    void drawOutputAreaBorder()
    {
        WinBits nWinStyle = mrParent.GetStyle();

        // Bei Border oben und unten einen Strich extra malen
        if ( (nWinStyle & WB_BORDER) || (nWinStyle & WB_TOPBORDER) )
        {
            Size aOutputSize = mrParent.GetOutputSizePixel();
            Rectangle aOutRect = mrParent.GetPageArea();

            // Bei 3D-Tabs wird auch der Border in 3D gemalt
            if ( nWinStyle & WB_3DTAB )
            {
                mrParent.SetLineColor( mpStyleSettings->GetShadowColor() );
                mrParent.DrawLine( Point( aOutRect.Left(), 0 ), Point( aOutputSize.Width(), 0 ) );
            }

            // Border malen (Strich oben und Strich unten)
            mrParent.SetLineColor( mpStyleSettings->GetDarkShadowColor() );
            mrParent.DrawLine( aOutRect.TopLeft(), Point( aOutputSize.Width()-1, aOutRect.Top() ) );
        }
    }

    void drawOuterFrame()
    {
        mrParent.DrawPolygon(maPoly);
    }

    void drawLeftShadow()
    {
        Point p1 = maPoly[0], p2 = maPoly[1];
        p1.X()++;
        p2.X()++;
        p2.Y()--;
        mrParent.DrawLine(p1, p2);
    }

    void drawRightShadow()
    {
        Point p1 = maPoly[2];
        Point p2 = maPoly[3];
        p1.X()--;
        p2.X()--;
        mrParent.DrawLine(p1, p2);
    }

    void drawTopInnerShadow()
    {
        Point p1 = maPoly[0], p2 = maPoly[3];
        p1.Y()++;
        p2.Y()++;
        mrParent.DrawLine(p1, p2);
    }

    void drawBottomShadow(bool bColored)
    {
        Point p1 = maPoly[1], p2 = maPoly[2];
        p1.X() += 1;
        p1.Y() -= 1;
        p2.X() -= 1;
        p2.Y() -= 1;
        mrParent.DrawLine(p1, p2);
        if (bColored)
        {
            p1 += Point(-1, -1);
            p2 += Point(1, -1);
            mrParent.DrawLine(p1, p2);
        }
    }

    void drawText(const OUString& aText)
    {
        Rectangle aRect = maRect;
        long nTextWidth = mrParent.GetTextWidth(aText);
        long nTextHeight = mrParent.GetTextHeight();
        Point aPos = aRect.TopLeft();
        aPos.X() += (aRect.getWidth()  - nTextWidth) / 2;
        aPos.Y() += (aRect.getHeight() - nTextHeight) / 2;

        if (mbEnabled)
            mrParent.DrawText(aPos, aText);
        else
            mrParent.DrawCtrlText(
                aPos, aText, 0, STRING_LEN, (TEXT_DRAW_DISABLE | TEXT_DRAW_MNEMONIC));
    }

    void drawOverTopBorder(bool b3DTab)
    {
        Point p1 = maPoly[0], p2 = maPoly[3];
        p1.X() += 1;
        p2.X() -= 1;
        Rectangle aDelRect(p1, p2);
        mrParent.DrawRect(aDelRect);
        if (b3DTab)
        {
            aDelRect.Top()--;
            mrParent.DrawRect(aDelRect);
        }
    }

    void drawTab()
    {
        mrParent.SetLineColor(mpStyleSettings->GetDarkShadowColor());

        // Je nach Status die richtige FillInBrush setzen
        // Set the correct FillInBrush depending upon status
        if ( mbSelected )
        {
            // Currently selected Tab
            mrParent.SetFillColor( maSelectedColor );
        }
        else if ( mbCustomColored )
        {
            mrParent.SetFillColor( maCustomColor );
        }
        else
        {
            mrParent.SetFillColor( maUnselectedColor );
        }

        drawOuterFrame();

        // If this is the current tab, draw the left inner shadow the default color,
        // otherwise make it the same as the custom background color
        Color aColor = mpStyleSettings->GetLightColor();
        if (mbCustomColored && !mbSelected)
            aColor = maCustomColor;

        mrParent.SetLineColor(aColor);
        drawLeftShadow();

        if ( !mbSelected )
            drawTopInnerShadow();

        mrParent.SetLineColor( mpStyleSettings->GetShadowColor() );
        drawRightShadow();
        if ( mbCustomColored && mbSelected )
        {
            mrParent.SetLineColor(maCustomColor);
            drawBottomShadow(true);
        }
        else
            drawBottomShadow(false);

        // Draw the outer frame once more.  In some environments, the outer frame
        // gets overpainted.
        mrParent.SetLineColor( mpStyleSettings->GetDarkShadowColor() );
        mrParent.SetFillColor();
        drawOuterFrame();
    }

    void drawPlusImage()
    {
        SvtResId id( BMP_LIST_ADD );
        Image aPlusImg( id );
        // Center the image within the bounding rectangle.
        Size aSize = aPlusImg.GetSizePixel();
        Point pt = maRect.TopLeft();
        long nXOffSet = (maRect.GetWidth() - aSize.Width()) / 2;
        long nYOffset = (maRect.GetHeight() - aSize.Height()) / 2;
        pt += Point(nXOffSet, nYOffset);
        pt.X() += 1;
        mrParent.DrawImage(pt, aPlusImg);
    }

    void setRect(const Rectangle& rRect)
    {
        maRect = rRect;

        long nOffY = mrParent.GetPageArea().getY();

        // Zuerst geben wir das Polygon gefuellt aus
        maPoly[0] = Point( rRect.Left(), nOffY );
        maPoly[1] = Point( rRect.Left()+TABBAR_OFFSET_X, rRect.Bottom() );
        maPoly[2] = Point( rRect.Right()-TABBAR_OFFSET_X, rRect.Bottom() );
        maPoly[3] = Point( rRect.Right(), nOffY );
    }

    void setSelected(bool b)
    {
        mbSelected = b;
    }

    void setCustomColored(bool b)
    {
        mbCustomColored = b;
    }

    void setSpecialTab(bool b)
    {
        mbSpecialTab = b;
    }

    void setEnabled(bool b)
    {
        mbEnabled = b;
    }

    void setSelectedFillColor(const Color& rColor)
    {
        maSelectedColor = rColor;
    }

    void setUnselectedFillColor(const Color& rColor)
    {
        maUnselectedColor = rColor;
    }

    void setCustomColor(const Color& rColor)
    {
        maCustomColor = rColor;
    }

private:
    TabBar&         mrParent;
    const StyleSettings*  mpStyleSettings;

    Rectangle       maRect;
    Polygon         maPoly;

    Color       maSelectedColor;
    Color       maCustomColor;
    Color       maUnselectedColor;

    bool        mbSelected:1;
    bool        mbCustomColored:1;
    bool        mbSpecialTab:1;
    bool        mbEnabled:1;
};

}

void TabBar::Paint( const Rectangle& rect )
{
    if(IsNativeControlSupported(CTRL_WINDOW_BACKGROUND,PART_ENTIRE_CONTROL))
        DrawNativeControl(CTRL_WINDOW_BACKGROUND,PART_ENTIRE_CONTROL,rect,
                CTRL_STATE_ENABLED,ImplControlValue(0),OUString());

    // Items berechnen und ausgeben
    sal_uInt16 nItemCount = (sal_uInt16)mpItemList->size();
    if (!nItemCount)
        return;

    ImplPrePaint();

    Color aFaceColor, aSelectColor, aFaceTextColor, aSelectTextColor;
    ImplGetColors( aFaceColor, aFaceTextColor, aSelectColor, aSelectTextColor );

    // Font selektieren
    Font aFont = GetFont();
    Font aLightFont = aFont;
    aLightFont.SetWeight( WEIGHT_NORMAL );

    TabBarPaintGuard aGuard(*this);
    TabDrawer aDrawer(*this);
    aDrawer.setSelectedFillColor(aSelectColor);
    aDrawer.setUnselectedFillColor(aFaceColor);
    aDrawer.drawOutputAreaBorder();

    // Now, start drawing the tabs.

    ImplTabBarItem* pItem = ImplGetLastTabBarItem(nItemCount);

    if (pItem && mbHasInsertTab)
    {
        // Draw the insert tab at the right end.
        Rectangle aRect = ImplGetInsertTabRect(pItem);
        aDrawer.setRect(aRect);
        aDrawer.drawTab();
        aDrawer.drawPlusImage();
    }

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    ImplTabBarItem* pCurItem = NULL;
    while ( pItem )
    {
        // CurrentItem als letztes ausgeben, da es alle anderen ueberdeckt
        if ( !pCurItem && (pItem->mnId == mnCurPageId) )
        {
            pCurItem = pItem;
            pItem = prev();
            if ( !pItem )
                pItem = pCurItem;
            continue;
        }

        bool bCurrent = pItem == pCurItem;

        if ( !pItem->maRect.IsEmpty() )
        {
            Rectangle aRect = pItem->maRect;
            bool bSelected = pItem->IsSelected(pCurItem);
            // We disable custom background color in high contrast mode.
            bool bCustomBgColor = !pItem->IsDefaultTabBgColor() && !rStyleSettings.GetHighContrastMode();
            bool bSpecialTab = (pItem->mnBits & TPB_SPECIAL);
            bool bEnabled = pItem->mbEnable;
            OUString aText = pItem->mbShort ?
                GetEllipsisString(pItem->maText, mnCurMaxWidth, TEXT_DRAW_ENDELLIPSIS) : pItem->maText;

            aDrawer.setRect(aRect);
            aDrawer.setSelected(bSelected);
            aDrawer.setCustomColored(bCustomBgColor);
            aDrawer.setSpecialTab(bSpecialTab);
            aDrawer.setEnabled(bEnabled);
            aDrawer.setCustomColor(pItem->maTabBgColor);
            aDrawer.drawTab();

            // Aktuelle Page wird mit einem fetten Font ausgegeben
            if ( bCurrent )
                SetFont( aFont );
            else
                SetFont( aLightFont );

            // Je nach Status die richtige FillInBrush setzen
            // Set the correct FillInBrush depending upon status
            if ( bSelected )
                SetTextColor( aSelectTextColor );
            else if ( bCustomBgColor )
                SetTextColor( pItem->maTabTextColor );
            else
                SetTextColor( aFaceTextColor );

            // This tab is "special", and a special tab needs a blue text.
            if (bSpecialTab)
                SetTextColor(Color(COL_LIGHTBLUE));

            aDrawer.drawText(aText);

            if ( bCurrent )
            {
                SetLineColor();
                SetFillColor(aSelectColor);
                aDrawer.drawOverTopBorder(mnWinStyle & WB_3DTAB);
                return;
            }

            pItem = prev();
        }
        else
        {
            if ( bCurrent )
                return;

            pItem = NULL;
        }

        if ( !pItem )
            pItem = pCurItem;
    }
}

// -----------------------------------------------------------------------
void TabBar::Resize()
{
    Size aNewSize = GetOutputSizePixel();

    long nSizerWidth = 0;
    long nButtonWidth = 0;

    // Sizer anordnen
    if ( mpImpl->mpSizer )
    {
        Size    aSizerSize = mpImpl->mpSizer->GetSizePixel();
        Point   aNewSizerPos( mbMirrored ? 0 : (aNewSize.Width()-aSizerSize.Width()), 0 );
        Size    aNewSizerSize( aSizerSize.Width(), aNewSize.Height() );
        mpImpl->mpSizer->SetPosSizePixel( aNewSizerPos, aNewSizerSize );
        nSizerWidth = aSizerSize.Width();
    }

    // Scroll-Buttons anordnen
    long nHeight = aNewSize.Height();
    // Font in der groesse Anpassen?
    ImplInitSettings( sal_True, sal_False );

    long nX = mbMirrored ? (aNewSize.Width()-nHeight) : 0;
    long nXDiff = mbMirrored ? -nHeight : nHeight;

    Size aBtnSize( nHeight, nHeight );
    if ( mpFirstBtn )
    {
        mpFirstBtn->SetPosSizePixel( Point( nX, 0 ), aBtnSize );
        nX += nXDiff;
        nButtonWidth += nHeight;
    }
    if ( mpPrevBtn )
    {
        mpPrevBtn->SetPosSizePixel( Point( nX, 0 ), aBtnSize );
        nX += nXDiff;
        nButtonWidth += nHeight;
    }
    if ( mpNextBtn )
    {
        mpNextBtn->SetPosSizePixel( Point( nX, 0 ), aBtnSize );
        nX += nXDiff;
        nButtonWidth += nHeight;
    }
    if ( mpLastBtn )
    {
        mpLastBtn->SetPosSizePixel( Point( nX, 0 ), aBtnSize );
        nX += nXDiff;
        nButtonWidth += nHeight;
    }

    // Groesse merken
    maWinSize = aNewSize;

    if( mbMirrored )
    {
        mnOffX = nSizerWidth;
        mnLastOffX = maWinSize.Width() - nButtonWidth - 1;
    }
    else
    {
        mnOffX = nButtonWidth;
        mnLastOffX = maWinSize.Width() - nSizerWidth - 1;
    }

    // Neu formatieren
    mbSizeFormat = sal_True;
    if ( IsReallyVisible() )
    {
        if ( ImplCalcWidth() )
            Invalidate();
        ImplFormat();
    }

    // Button enablen/disablen
    ImplEnableControls();
}

// -----------------------------------------------------------------------

void TabBar::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = GetPageId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
    if ( nItemId )
    {
        if ( rHEvt.GetMode() & HELPMODE_BALLOON )
        {
            OUString aStr = GetHelpText( nItemId );
            if (!aStr.isEmpty())
            {
                Rectangle aItemRect = GetPageRect( nItemId );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                return;
            }
        }
        else if ( rHEvt.GetMode() & HELPMODE_EXTENDED )
        {
            OUString aHelpId( OStringToOUString( GetHelpId( nItemId ), RTL_TEXTENCODING_UTF8 ) );
            if ( !aHelpId.isEmpty() )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                    pHelp->Start( aHelpId, this );
                return;
            }
        }

        // Bei Quick- oder Ballloon-Help zeigen wir den Text an,
        // wenn dieser abgeschnitten oder nicht voll sichtbar ist
        if ( rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON) )
        {
            sal_uInt16 nPos = GetPagePos( nItemId );
            ImplTabBarItem* pItem = (*mpItemList)[ nPos ];
            if ( pItem->mbShort ||
                (pItem->maRect.Right()-TABBAR_OFFSET_X-5 > mnLastOffX) )
            {
                Rectangle aItemRect = GetPageRect( nItemId );
                Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                OUString aStr = (*mpItemList)[ nPos ]->maText;
                if (!aStr.isEmpty())
                {
                    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
                    else
                        Help::ShowQuickHelp( this, aItemRect, aStr );
                    return;
                }
            }
        }
    }

    Window::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void TabBar::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( (mbSizeFormat || mbFormat) && !mpItemList->empty() )
            ImplFormat();
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        Invalidate();
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_True );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_MIRRORING )
    {
        // reacts on calls of EnableRTL, have to mirror all child controls
        if( mpFirstBtn ) mpFirstBtn->EnableRTL( IsRTLEnabled() );
        if( mpPrevBtn ) mpPrevBtn->EnableRTL( IsRTLEnabled() );
        if( mpNextBtn ) mpNextBtn->EnableRTL( IsRTLEnabled() );
        if( mpLastBtn ) mpLastBtn->EnableRTL( IsRTLEnabled() );
        if( mpImpl->mpSizer ) mpImpl->mpSizer->EnableRTL( IsRTLEnabled() );
        if( mpEdit ) mpEdit->EnableRTL( IsRTLEnabled() );
    }
}

// -----------------------------------------------------------------------

void TabBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( sal_True, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void TabBar::ImplSelect()
{
    Select();

    CallEventListeners( VCLEVENT_TABBAR_PAGESELECTED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)) );
}

// -----------------------------------------------------------------------

void TabBar::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void TabBar::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void TabBar::Split()
{
    maSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void TabBar::ImplActivatePage()
{
    ActivatePage();

    CallEventListeners( VCLEVENT_TABBAR_PAGEACTIVATED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)) );
}

// -----------------------------------------------------------------------

void TabBar::ActivatePage()
{
    maActivatePageHdl.Call( this );
}

// -----------------------------------------------------------------------

long TabBar::ImplDeactivatePage()
{
    long nRet = DeactivatePage();

    CallEventListeners( VCLEVENT_TABBAR_PAGEDEACTIVATED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)) );

    return nRet;
}

void TabBar::ImplPrePaint()
{
    sal_uInt16 nItemCount = (sal_uInt16)mpItemList->size();
    if (!nItemCount)
        return;

    ImplTabBarItem* pItem;

    // TabBar muss formatiert sein
    ImplFormat();

    // Beim ersten Format auch dafuer sorgen, das aktuelle TabPage
    // sichtbar wird
    if ( mbFirstFormat )
    {
        mbFirstFormat = sal_False;

        if ( mnCurPageId && (mnFirstPos == 0) && !mbDropPos )
        {
            pItem = (*mpItemList)[ GetPagePos( mnCurPageId ) ];
            if ( pItem->maRect.IsEmpty() )
            {
                // mbDropPos setzen (bzw. misbrauchen) um Invalidate()
                // zu unterbinden
                mbDropPos = sal_True;
                SetFirstPageId( mnCurPageId );
                mbDropPos = sal_False;
                if ( mnFirstPos != 0 )
                    ImplFormat();
            }
        }
    }
}

ImplTabBarItem* TabBar::ImplGetLastTabBarItem( sal_uInt16 nItemCount )
{
    // letzten sichtbaren Eintrag suchen
    sal_uInt16 n = mnFirstPos+1;
    if ( n >= nItemCount )
        n = nItemCount-1;
    ImplTabBarItem* pItem = seek( n );
    while ( pItem )
    {
        if ( !pItem->maRect.IsEmpty() )
        {
            n++;
            pItem = next();
        }
        else
            break;
    }

    // Alle Tabs ausgeben (von hinten nach vorn und aktuellen zuletzt)
    if ( pItem )
        n--;
    else if ( n >= nItemCount )
        n = nItemCount-1;
    pItem = seek( n );
    return pItem;
}

Rectangle TabBar::ImplGetInsertTabRect(ImplTabBarItem* pItem) const
{
    if (mbHasInsertTab && pItem)
    {
        Rectangle aInsTabRect = pItem->maRect;
        if ( !mbMirrored )
            aInsTabRect.setX(
                aInsTabRect.getX() + aInsTabRect.getWidth() - TABBAR_OFFSET_X - TABBAR_OFFSET_X2);
        else
            aInsTabRect.setX(
                aInsTabRect.getX() - 3*TABBAR_OFFSET_X - TABBAR_OFFSET_X2);
        aInsTabRect.setWidth(32);
        return aInsTabRect;
    }
    return Rectangle();
}

// -----------------------------------------------------------------------

long TabBar::DeactivatePage()
{
    if ( maDeactivatePageHdl.IsSet() )
        return maDeactivatePageHdl.Call( this );
    else
        return sal_True;
}

// -----------------------------------------------------------------------

long TabBar::StartRenaming()
{
    if ( maStartRenamingHdl.IsSet() )
        return maStartRenamingHdl.Call( this );
    else
        return sal_True;
}

// -----------------------------------------------------------------------

long TabBar::AllowRenaming()
{
    if ( maAllowRenamingHdl.IsSet() )
        return maAllowRenamingHdl.Call( this );
    else
        return sal_True;
}

// -----------------------------------------------------------------------

void TabBar::EndRenaming()
{
    maEndRenamingHdl.Call( this );
}

// -----------------------------------------------------------------------

void TabBar::Mirror()
{

}

// -----------------------------------------------------------------------

void TabBar::InsertPage( sal_uInt16 nPageId, const OUString& rText,
                         TabBarPageBits nBits, sal_uInt16 nPos )
{
    DBG_ASSERT( nPageId, "TabBar::InsertPage(): PageId == 0" );
    DBG_ASSERT( GetPagePos( nPageId ) == PAGE_NOT_FOUND,
                "TabBar::InsertPage(): PageId already exists" );
    DBG_ASSERT( nBits <= TPB_SPECIAL, "TabBar::InsertPage(): nBits is wrong" );

    // PageItem anlegen und in die Item-Liste eintragen
    ImplTabBarItem* pItem = new ImplTabBarItem( nPageId, rText, nBits );
    if ( nPos < mpItemList->size() ) {
        ImplTabBarList::iterator it = mpItemList->begin();
        ::std::advance( it, nPos );
        mpItemList->insert( it, pItem );
    } else {
        mpItemList->push_back( pItem );
    }
    mbSizeFormat = sal_True;

    // CurPageId gegebenenfalls setzen
    if ( !mnCurPageId )
        mnCurPageId = nPageId;

    // Leiste neu ausgeben
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();

    CallEventListeners( VCLEVENT_TABBAR_PAGEINSERTED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)) );
}

// -----------------------------------------------------------------------

Color TabBar::GetTabBgColor( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    if ( nPos != PAGE_NOT_FOUND )
        return (*mpItemList)[ nPos ]->maTabBgColor;
    else
        return Color( COL_AUTO );
}

void TabBar::SetTabBgColor( sal_uInt16 nPageId, const Color& aTabBgColor )
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    ImplTabBarItem* pItem;
    if ( nPos != PAGE_NOT_FOUND )
    {
        pItem = (*mpItemList)[ nPos ];
        if ( aTabBgColor != Color( COL_AUTO )  )
        {
            pItem->maTabBgColor = aTabBgColor;
            if ( aTabBgColor.GetLuminance() <= 128 ) //Do not use aTabBgColor.IsDark(), because that threshold is way too low...
                pItem->maTabTextColor = Color( COL_WHITE );
            else
                pItem->maTabTextColor = Color( COL_BLACK );
        }
        else
        {
            pItem->maTabBgColor = Color( COL_AUTO );
            pItem->maTabTextColor = Color( COL_AUTO );
        }
    }
}

// -----------------------------------------------------------------------

void TabBar::RemovePage( sal_uInt16 nPageId )
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    // Existiert Item
    if ( nPos != PAGE_NOT_FOUND )
    {
        if ( mnCurPageId == nPageId )
            mnCurPageId = 0;

        // Testen, ob erste sichtbare Seite verschoben werden muss
        if ( mnFirstPos > nPos )
            mnFirstPos--;

        // Item-Daten loeschen
        ImplTabBarList::iterator it = mpItemList->begin();
        ::std::advance( it, nPos );
        delete *it;
        mpItemList->erase( it );

        // Leiste neu ausgeben
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();

        CallEventListeners( VCLEVENT_TABBAR_PAGEREMOVED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)) );
    }
}

// -----------------------------------------------------------------------

void TabBar::MovePage( sal_uInt16 nPageId, sal_uInt16 nNewPos )
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    Pair aPair( nPos, nNewPos );

    if ( nPos < nNewPos )
        nNewPos--;

    if ( nPos == nNewPos )
        return;

    // Existiert Item
    if ( nPos != PAGE_NOT_FOUND )
    {
        // TabBar-Item in der Liste verschieben
        ImplTabBarList::iterator it = mpItemList->begin();
        ::std::advance( it, nPos );
        ImplTabBarItem* pItem = *it;
        mpItemList->erase( it );
        if ( nNewPos < mpItemList->size() ) {
            it = mpItemList->begin();
            ::std::advance( it, nNewPos );
            mpItemList->insert( it, pItem );
        } else {
            mpItemList->push_back( pItem );
        }

        // Leiste neu ausgeben
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();

        CallEventListeners( VCLEVENT_TABBAR_PAGEMOVED, (void*) &aPair );
    }
}

// -----------------------------------------------------------------------

void TabBar::Clear()
{
    // Alle Items loeschen
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        delete (*mpItemList)[ i ];
    }
    mpItemList->clear();

    // Items aus der Liste loeschen
    mbSizeFormat = sal_True;
    mnCurPageId = 0;
    mnFirstPos = 0;
    maCurrentItemList = 0;

    // Leiste neu ausgeben
    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();

    CallEventListeners( VCLEVENT_TABBAR_PAGEREMOVED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(PAGE_NOT_FOUND)) );
}

// -----------------------------------------------------------------------

sal_Bool TabBar::IsPageEnabled( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    if ( nPos != PAGE_NOT_FOUND )
        return (*mpItemList)[ nPos ]->mbEnable;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void TabBar::SetPageBits( sal_uInt16 nPageId, TabBarPageBits nBits )
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    if ( nPos != PAGE_NOT_FOUND )
    {
        ImplTabBarItem* pItem = (*mpItemList)[ nPos ];

        if ( pItem->mnBits != nBits )
        {
            pItem->mnBits = nBits;

            // Leiste neu ausgeben
            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate( pItem->maRect );
        }
    }
}

// -----------------------------------------------------------------------

TabBarPageBits TabBar::GetPageBits( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    if ( nPos != PAGE_NOT_FOUND )
        return (*mpItemList)[ nPos ]->mnBits;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_uInt16 TabBar::GetPageCount() const
{
    return (sal_uInt16)mpItemList->size();
}

// -----------------------------------------------------------------------

sal_uInt16 TabBar::GetPageId( sal_uInt16 nPos ) const
{
    return ( nPos < mpItemList->size() ) ? (*mpItemList)[ nPos ]->mnId : 0;
}

// -----------------------------------------------------------------------

sal_uInt16 TabBar::GetPagePos( sal_uInt16 nPageId ) const
{
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i ) {
        if ( (*mpItemList)[ i ]->mnId == nPageId ) {
            return sal_uInt16( i );
        }
    }
    return PAGE_NOT_FOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 TabBar::GetPageId( const Point& rPos, bool bCheckInsTab ) const
{
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i )
    {
        ImplTabBarItem* pItem = (*mpItemList)[ i ];
        if ( pItem->maRect.IsInside( rPos ) )
            return pItem->mnId;
    }

    if (bCheckInsTab && mbHasInsertTab && !mpItemList->empty())
    {
        ImplTabBarItem* pItem = mpItemList->back();
        if (ImplGetInsertTabRect(pItem).IsInside(rPos))
            return INSERT_TAB_POS;
    }

    return 0;
}

// -----------------------------------------------------------------------

Rectangle TabBar::GetPageRect( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    if ( nPos != PAGE_NOT_FOUND )
        return (*mpItemList)[ nPos ]->maRect;
    else
        return Rectangle();
}

// -----------------------------------------------------------------------

void TabBar::SetCurPageId( sal_uInt16 nPageId )
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    // Wenn Item nicht existiert, dann nichts machen
    if ( nPos != PAGE_NOT_FOUND )
    {
        // Wenn sich aktuelle Page nicht geaendert hat, dann muessen wir
        // jetzt nichts mehr machen
        if ( nPageId == mnCurPageId )
            return;

        // Muss invalidiert werden
        sal_Bool bUpdate = sal_False;
        if ( IsReallyVisible() && IsUpdateMode() )
            bUpdate = sal_True;

        ImplTabBarItem* pItem = (*mpItemList)[ nPos ];
        ImplTabBarItem* pOldItem;

        if ( mnCurPageId )
            pOldItem = (*mpItemList)[ GetPagePos( mnCurPageId ) ];
        else
            pOldItem = NULL;

        // Wenn Page nicht selektiert, dann vorher selektierte Seite
        // deselktieren, wenn dies die einzige selektierte Seite ist
        if ( !pItem->mbSelect && pOldItem )
        {
            sal_uInt16 nSelPageCount = GetSelectPageCount();
            if ( nSelPageCount == 1 )
                pOldItem->mbSelect = sal_False;
            pItem->mbSelect = sal_True;
        }

        mnCurPageId = nPageId;
        mbFormat = sal_True;

        // Dafuer sorgen, das aktuelle Page sichtbar wird
        if ( IsReallyVisible() )
        {
            if ( nPos < mnFirstPos )
                SetFirstPageId( nPageId );
            else
            {
                // sichtbare Breite berechnen
                long nWidth = mnLastOffX;
                if ( nWidth > TABBAR_OFFSET_X )
                    nWidth -= TABBAR_OFFSET_X;
                if ( nWidth > ADDNEWPAGE_AREAWIDTH )
                    nWidth -= ADDNEWPAGE_AREAWIDTH;

                if ( pItem->maRect.IsEmpty() )
                    ImplFormat();

                while ( (mbMirrored ? (pItem->maRect.Left() < mnOffX) : (pItem->maRect.Right() > nWidth)) ||
                        pItem->maRect.IsEmpty() )
                {
                    sal_uInt16 nNewPos = mnFirstPos+1;
                    // Dafuer sorgen, das min. die aktuelle TabPages als
                    // erste TabPage sichtbar ist
                    if ( nNewPos >= nPos )
                    {
                        SetFirstPageId( nPageId );
                        break;
                    }
                    else
                        SetFirstPageId( GetPageId( nNewPos ) );
                    ImplFormat();
                    // Falls erste Seite nicht weitergeschaltet wird, dann
                    // koennen wir abbrechen
                    if ( nNewPos != mnFirstPos )
                        break;
                }
            }
        }

        // Leiste neu ausgeben
        if ( bUpdate )
        {
            Invalidate( pItem->maRect );
            if ( pOldItem )
                Invalidate( pOldItem->maRect );
        }
    }
}

// -----------------------------------------------------------------------

void TabBar::MakeVisible( sal_uInt16 nPageId )
{
    if ( !IsReallyVisible() )
        return;

    sal_uInt16 nPos = GetPagePos( nPageId );

    // Wenn Item nicht existiert, dann nichts machen
    if ( nPos != PAGE_NOT_FOUND )
    {
        if ( nPos < mnFirstPos )
            SetFirstPageId( nPageId );
        else
        {
            ImplTabBarItem* pItem = (*mpItemList)[ nPos ];

            // sichtbare Breite berechnen
            long nWidth = mnLastOffX;
            if ( nWidth > TABBAR_OFFSET_X )
                nWidth -= TABBAR_OFFSET_X;

            if ( mbFormat || pItem->maRect.IsEmpty() )
            {
                mbFormat = sal_True;
                ImplFormat();
            }

            while ( (pItem->maRect.Right() > nWidth) ||
                    pItem->maRect.IsEmpty() )
            {
                sal_uInt16 nNewPos = mnFirstPos+1;
                // Dafuer sorgen, das min. die aktuelle TabPages als
                // erste TabPage sichtbar ist
                if ( nNewPos >= nPos )
                {
                    SetFirstPageId( nPageId );
                    break;
                }
                else
                    SetFirstPageId( GetPageId( nNewPos ) );
                ImplFormat();
                // Falls erste Seite nicht weitergeschaltet wird, dann
                // koennen wir abbrechen
                if ( nNewPos != mnFirstPos )
                    break;
            }
        }
    }
}

// -----------------------------------------------------------------------

void TabBar::SetFirstPageId( sal_uInt16 nPageId )
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    // Wenn Item nicht existiert, dann sal_False zurueckgeben
    if ( nPos != PAGE_NOT_FOUND )
    {
        if ( nPos != mnFirstPos )
        {
            // Dafuer sorgen, das nach Moeglichkteit soviele Pages wie
            // moeglich sichtbar sind
            ImplFormat();
            sal_uInt16 nLastFirstPos = ImplGetLastFirstPos();
            sal_uInt16 nNewPos;
            if ( nPos > nLastFirstPos )
                nNewPos = nLastFirstPos;
            else
                nNewPos = nPos;

            if ( nNewPos != mnFirstPos )
            {
                mnFirstPos = nNewPos;
                mbFormat = sal_True;

                // Leiste neu ausgeben (Achtung: mbDropPos beachten, da wenn
                // dieses Flag gesetzt ist, wird direkt gepaintet)
                if ( IsReallyVisible() && IsUpdateMode() && !mbDropPos )
                    Invalidate();
            }
        }
    }
}

// -----------------------------------------------------------------------

void TabBar::SelectPage( sal_uInt16 nPageId, sal_Bool bSelect )
{
    sal_uInt16 nPos = GetPagePos( nPageId );

    if ( nPos != PAGE_NOT_FOUND )
    {
        ImplTabBarItem* pItem = (*mpItemList)[ nPos ];

        if ( pItem->mbSelect != bSelect )
        {
            pItem->mbSelect = bSelect;

            // Leiste neu ausgeben
            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate( pItem->maRect );
        }
    }
}

// -----------------------------------------------------------------------

sal_uInt16 TabBar::GetSelectPageCount() const
{
    sal_uInt16          nSelected = 0;
    for ( size_t i = 0, n = mpItemList->size(); i < n; ++i )
    {
        ImplTabBarItem* pItem = (*mpItemList)[ i ];
        if ( pItem->mbSelect )
            nSelected++;
    }

    return nSelected;
}

// -----------------------------------------------------------------------

sal_Bool TabBar::IsPageSelected( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    if ( nPos != PAGE_NOT_FOUND )
        return (*mpItemList)[ nPos ]->mbSelect;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool TabBar::StartEditMode( sal_uInt16 nPageId )
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    if ( mpEdit || (nPos == PAGE_NOT_FOUND) || (mnLastOffX < 8) )
        return sal_False;

    mnEditId = nPageId;
    if ( StartRenaming() )
    {
        ImplShowPage( nPos );
        ImplFormat();
        Update();

        mpEdit = new TabBarEdit( this, WB_CENTER );
        Rectangle aRect = GetPageRect( mnEditId );
        long nX = aRect.Left()+TABBAR_OFFSET_X+(TABBAR_OFFSET_X2/2);
        long nWidth = aRect.GetWidth()-(TABBAR_OFFSET_X*2)-TABBAR_OFFSET_X2;
        if ( mnEditId != GetCurPageId() )
            nX += 1;
        if ( nX+nWidth > mnLastOffX )
            nWidth = mnLastOffX-nX;
        if ( nWidth < 3 )
        {
            nX = aRect.Left();
            nWidth = aRect.GetWidth();
        }
        mpEdit->SetText( GetPageText( mnEditId ) );
        mpEdit->setPosSizePixel( nX, aRect.Top()+mnOffY+1, nWidth, aRect.GetHeight()-3 );
        Font    aFont = GetPointFont();
        Color   aForegroundColor;
        Color   aBackgroundColor;
        Color   aFaceColor;
        Color   aSelectColor;
        Color   aFaceTextColor;
        Color   aSelectTextColor;
        ImplGetColors( aFaceColor, aFaceTextColor, aSelectColor, aSelectTextColor );
        if ( mnEditId != GetCurPageId() )
            aFont.SetWeight( WEIGHT_LIGHT );
        if ( IsPageSelected( mnEditId ) || (mnEditId == GetCurPageId()) )
        {
            aForegroundColor = aSelectTextColor;
            aBackgroundColor = aSelectColor;
        }
        else
        {
            aForegroundColor = aFaceTextColor;
            aBackgroundColor = aFaceColor;
        }
        if ( GetPageBits( mnEditId ) & TPB_SPECIAL )
            aForegroundColor = Color( COL_LIGHTBLUE );
        mpEdit->SetControlFont( aFont );
        mpEdit->SetControlForeground( aForegroundColor );
        mpEdit->SetControlBackground( aBackgroundColor );
        mpEdit->GrabFocus();
        mpEdit->SetSelection( Selection( 0, mpEdit->GetText().getLength() ) );
        mpEdit->Show();
        return sal_True;
    }
    else
    {
        mnEditId = 0;
        return sal_False;
    }
}

// -----------------------------------------------------------------------

void TabBar::EndEditMode( sal_Bool bCancel )
{
    if ( mpEdit )
    {
        // call hdl
        sal_Bool bEnd = sal_True;
        mbEditCanceled = bCancel;
        maEditText = mpEdit->GetText();
        mpEdit->SetPostEvent();
        if ( !bCancel )
        {
            long nAllowRenaming = AllowRenaming();
            if ( nAllowRenaming == TABBAR_RENAMING_YES )
                SetPageText( mnEditId, maEditText );
            else if ( nAllowRenaming == TABBAR_RENAMING_NO )
                bEnd = sal_False;
            else // nAllowRenaming == TABBAR_RENAMING_CANCEL
                mbEditCanceled = sal_True;
        }

        // renaming not allowed, than reset edit data
        if ( !bEnd )
        {
            mpEdit->ResetPostEvent();
            mpEdit->GrabFocus();
        }
        else
        {
            // close edit and call end hdl
            delete mpEdit;
            mpEdit = NULL;
            EndRenaming();
            mnEditId = 0;
        }

        // reset
        maEditText = OUString();
        mbEditCanceled = false;
    }
}

// -----------------------------------------------------------------------

void TabBar::SetMirrored(bool bMirrored)
{
    if( mbMirrored != bMirrored )
    {
        mbMirrored = bMirrored;
        mbSizeFormat = true;
        ImplInitControls();     // for button images
        Resize();               // recalculates control positions
        Mirror();
    }
}

void TabBar::SetEffectiveRTL( bool bRTL )
{
    SetMirrored( bRTL != Application::GetSettings().GetLayoutRTL() );
}

bool TabBar::IsEffectiveRTL() const
{
    return IsMirrored() != Application::GetSettings().GetLayoutRTL();
}

// -----------------------------------------------------------------------

void TabBar::SetMaxPageWidth( long nMaxWidth )
{
    if ( mnMaxPageWidth != nMaxWidth )
    {
        mnMaxPageWidth = nMaxWidth;
        mbSizeFormat = sal_True;

        // Leiste neu ausgeben
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void TabBar::SetPageText( sal_uInt16 nPageId, const OUString& rText )
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    if ( nPos != PAGE_NOT_FOUND )
    {
        (*mpItemList)[ nPos ]->maText = rText;
        mbSizeFormat = sal_True;

        // Leiste neu ausgeben
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();

        CallEventListeners( VCLEVENT_TABBAR_PAGETEXTCHANGED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)) );
    }
}

// -----------------------------------------------------------------------

OUString TabBar::GetPageText( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    if ( nPos != PAGE_NOT_FOUND )
        return (*mpItemList)[ nPos ]->maText;
    return OUString();
}

// -----------------------------------------------------------------------

OUString TabBar::GetHelpText( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    if ( nPos != PAGE_NOT_FOUND )
    {
        ImplTabBarItem* pItem = (*mpItemList)[ nPos ];
        if (pItem->maHelpText.isEmpty() && !pItem->maHelpId.isEmpty())
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pItem->maHelpText = pHelp->GetHelpText( OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
        }

        return pItem->maHelpText;
    }
    return OUString();
}

// -----------------------------------------------------------------------

OString TabBar::GetHelpId( sal_uInt16 nPageId ) const
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    OString aRet;
    if ( nPos != PAGE_NOT_FOUND )
        return (*mpItemList)[ nPos ]->maHelpId;
    return aRet;
}

// -----------------------------------------------------------------------

sal_Bool TabBar::StartDrag( const CommandEvent& rCEvt, Region& rRegion )
{
    if ( !(mnWinStyle & WB_DRAG) || (rCEvt.GetCommand() != COMMAND_STARTDRAG) )
        return sal_False;

    // Testen, ob angeklickte Seite selektiert ist. Falls dies nicht
    // der Fall ist, setzen wir ihn als aktuellen Eintrag. Falls Drag and
    // Drop auch mal ueber Tastatur ausgeloest werden kann, testen wir
    // dies nur bei einer Mausaktion.
    // Ausserdem machen wir das nur, wenn kein Select() ausgeloest wurde,
    // da der Select schon den Bereich gescrollt haben kann
    if ( rCEvt.IsMouseEvent() && !mbInSelect )
    {
        sal_uInt16 nSelId = GetPageId( rCEvt.GetMousePosPixel() );

        // Falls kein Eintrag angeklickt wurde, starten wir kein Dragging
        if ( !nSelId )
            return sal_False;

        // Testen, ob Seite selektiertiert ist. Falls nicht, als aktuelle
        // Seite setzen und Select rufen.
        if ( !IsPageSelected( nSelId ) )
        {
            if ( ImplDeactivatePage() )
            {
                SetCurPageId( nSelId );
                Update();
                ImplActivatePage();
                ImplSelect();
            }
            else
                return sal_False;
        }
    }
    mbInSelect = sal_False;

    Region aRegion;

    // Region zuweisen
    rRegion = aRegion;

    return sal_True;
}

// -----------------------------------------------------------------------

sal_uInt16 TabBar::ShowDropPos( const Point& rPos )
{
    ImplTabBarItem* pItem;
    sal_uInt16      nDropId;
    sal_uInt16      nNewDropPos;
    sal_uInt16      nItemCount = (sal_uInt16)mpItemList->size();
    short       nScroll = 0;

    if ( rPos.X() > mnLastOffX-TABBAR_DRAG_SCROLLOFF )
    {
        pItem = (*mpItemList)[ mpItemList->size()-1 ];
        if ( !pItem->maRect.IsEmpty() && (rPos.X() > pItem->maRect.Right()) )
            nNewDropPos = (sal_uInt16)mpItemList->size();
        else
        {
            nNewDropPos = mnFirstPos+1;
            nScroll = 1;
        }
    }
    else if ( (rPos.X() <= mnOffX) ||
              (!mnOffX && (rPos.X() <= TABBAR_DRAG_SCROLLOFF)) )
    {
        if ( mnFirstPos )
        {
            nNewDropPos = mnFirstPos;
            nScroll = -1;
        }
        else
            nNewDropPos = 0;
    }
    else
    {
        nDropId = GetPageId( rPos );
        if ( nDropId )
        {
            nNewDropPos = GetPagePos( nDropId );
            if ( mnFirstPos && (nNewDropPos == mnFirstPos-1) )
                nScroll = -1;
        }
        else
            nNewDropPos = nItemCount;
    }

    if ( mbDropPos && (nNewDropPos == mnDropPos) && !nScroll )
        return mnDropPos;

    if ( mbDropPos )
        HideDropPos();
    mbDropPos = sal_True;
    mnDropPos = nNewDropPos;

    if ( nScroll )
    {
        sal_uInt16 nOldFirstPos = mnFirstPos;
        SetFirstPageId( GetPageId( mnFirstPos+nScroll ) );

        // Direkt ausgeben, da kein Paint bei Drag and Drop moeglich
        if ( nOldFirstPos != mnFirstPos )
        {
            Rectangle aRect( mnOffX, 0, mnLastOffX, maWinSize.Height() );
            SetFillColor( GetBackground().GetColor() );
            DrawRect( aRect );
            Paint( aRect );
        }
    }

    // Drop-Position-Pfeile ausgeben
    Color       aBlackColor( COL_BLACK );
    long        nX;
    long        nY = (maWinSize.Height()/2)-1;
    sal_uInt16      nCurPos = GetPagePos( mnCurPageId );

    SetLineColor( aBlackColor );
    if ( mnDropPos < nItemCount )
    {
        pItem = (*mpItemList)[ mnDropPos ];
        nX = pItem->maRect.Left()+TABBAR_OFFSET_X;
        if ( mnDropPos == nCurPos )
            nX--;
        else
            nX++;
        if ( !pItem->IsDefaultTabBgColor() && !pItem->mbSelect)
            SetLineColor( pItem->maTabTextColor );
        DrawLine( Point( nX, nY ), Point( nX, nY ) );
        DrawLine( Point( nX+1, nY-1 ), Point( nX+1, nY+1 ) );
        DrawLine( Point( nX+2, nY-2 ), Point( nX+2, nY+2 ) );
        SetLineColor( aBlackColor );
    }
    if ( (mnDropPos > 0) && (mnDropPos < nItemCount+1) )
    {
        pItem = (*mpItemList)[ mnDropPos-1 ];
        nX = pItem->maRect.Right()-TABBAR_OFFSET_X;
        if ( mnDropPos == nCurPos )
            nX++;
        if ( !pItem->IsDefaultTabBgColor() && !pItem->mbSelect)
            SetLineColor( pItem->maTabTextColor );
        DrawLine( Point( nX, nY ), Point( nX, nY ) );
        DrawLine( Point( nX-1, nY-1 ), Point( nX-1, nY+1 ) );
        DrawLine( Point( nX-2, nY-2 ), Point( nX-2, nY+2 ) );
    }

    return mnDropPos;
}

// -----------------------------------------------------------------------

void TabBar::HideDropPos()
{
    if ( mbDropPos )
    {
        ImplTabBarItem* pItem;
        long        nX;
        long        nY1 = (maWinSize.Height()/2)-3;
        long        nY2 = nY1 + 5;
        sal_uInt16      nItemCount = (sal_uInt16)mpItemList->size();

        if ( mnDropPos < nItemCount )
        {
            pItem = (*mpItemList)[ mnDropPos ];
            nX = pItem->maRect.Left()+TABBAR_OFFSET_X;
            // Paint direkt aufrufen, da bei Drag and Drop kein Paint
            // moeglich
            Rectangle aRect( nX-1, nY1, nX+3, nY2 );
            Region aRegion( aRect );
            SetClipRegion( aRegion );
            Paint( aRect );
            SetClipRegion();
        }
        if ( (mnDropPos > 0) && (mnDropPos < nItemCount+1) )
        {
            pItem = (*mpItemList)[ mnDropPos-1 ];
            nX = pItem->maRect.Right()-TABBAR_OFFSET_X;
            // Paint direkt aufrufen, da bei Drag and Drop kein Paint
            // moeglich
            Rectangle aRect( nX-2, nY1, nX+1, nY2 );
            Region aRegion( aRect );
            SetClipRegion( aRegion );
            Paint( aRect );
            SetClipRegion();
        }

        mbDropPos = sal_False;
        mnDropPos = 0;
    }
}

// -----------------------------------------------------------------------

sal_Bool TabBar::SwitchPage( const Point& rPos )
{
    sal_Bool    bSwitch = sal_False;
    sal_uInt16  nSwitchId = GetPageId( rPos );
    if ( !nSwitchId )
        EndSwitchPage();
    else
    {
        if ( nSwitchId != mnSwitchId )
        {
            mnSwitchId = nSwitchId;
            mnSwitchTime = Time::GetSystemTicks();
        }
        else
        {
            // Erst nach 500 ms umschalten
            if ( mnSwitchId != GetCurPageId() )
            {
                if ( Time::GetSystemTicks() > mnSwitchTime+500 )
                {
                    mbInSwitching = sal_True;
                    if ( ImplDeactivatePage() )
                    {
                        SetCurPageId( mnSwitchId );
                        Update();
                        ImplActivatePage();
                        ImplSelect();
                        bSwitch = sal_True;
                    }
                    mbInSwitching = sal_False;
                }
            }
        }
    }

    return bSwitch;
}

// -----------------------------------------------------------------------

void TabBar::EndSwitchPage()
{
    mnSwitchTime    = 0;
    mnSwitchId      = 0;
}

// -----------------------------------------------------------------------

void TabBar::SetStyle( WinBits nStyle )
{
    mnWinStyle = nStyle;
    ImplInitControls();
    // Evt. Controls neu anordnen
    if ( IsReallyVisible() && IsUpdateMode() )
        Resize();
}

// -----------------------------------------------------------------------

Size TabBar::CalcWindowSizePixel() const
{
    long nWidth = 0;

    if ( mpItemList->size() )
    {
        ((TabBar*)this)->ImplCalcWidth();
        for ( size_t i = 0, n = mpItemList->size(); i < n; ++i )
        {
            ImplTabBarItem* pItem = (*mpItemList)[ i ];
            nWidth += pItem->mnWidth;
        }
        nWidth += TABBAR_OFFSET_X+TABBAR_OFFSET_X2;
    }

    return Size( nWidth, GetSettings().GetStyleSettings().GetScrollBarSize() );
}
// -----------------------------------------------------------------------

Rectangle TabBar::GetPageArea() const
{
    return Rectangle( Point( mnOffX, mnOffY ), Size( mnLastOffX-mnOffX+1, GetSizePixel().Height()-mnOffY ) );
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > TabBar::CreateAccessible()
{
    return mpImpl->maAccessibleFactory.getFactory().createAccessibleTabBar( *this );
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
