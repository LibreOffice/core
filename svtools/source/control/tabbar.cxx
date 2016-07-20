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
#include <vcl/settings.hxx>
#include "svtaccessiblefactory.hxx"
#include <filectrl.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <limits>
#include <utility>
#include <vcl/idle.hxx>

namespace
{

#define TABBAR_DRAG_SCROLLOFF   5
#define TABBAR_MINSIZE          5

const sal_uInt16 ADDNEWPAGE_AREAWIDTH = 10;
const sal_uInt16 BUTTON_MARGIN = 6;

class TabDrawer
{
private:
    TabBar& mrParent;
    vcl::RenderContext& mrRenderContext;
    const StyleSettings& mrStyleSettings;

    Rectangle maRect;

    Color maSelectedColor;
    Color maCustomColor;
    Color maUnselectedColor;

    bool mbSelected:1;
    bool mbCustomColored:1;
    bool mbSpecialTab:1;
    bool mbEnabled:1;

public:
    explicit TabDrawer(TabBar& rParent, vcl::RenderContext& rRenderContext)
        : mrParent(rParent)
        , mrRenderContext(rRenderContext)
        , mrStyleSettings(rRenderContext.GetSettings().GetStyleSettings())
        , mbSelected(false)
        , mbCustomColored(false)
        , mbSpecialTab(false)
        , mbEnabled(false)
    {
    }

    void drawOutputAreaBorder()
    {
        WinBits nWinStyle = mrParent.GetStyle();

        // draw extra line if above and below border
        if ((nWinStyle & WB_BORDER) || (nWinStyle & WB_TOPBORDER))
        {
            Size aOutputSize(mrParent.GetOutputSizePixel());
            Rectangle aOutRect = mrParent.GetPageArea();

            // also draw border in 3D for 3D-tabs
            if (nWinStyle & WB_3DTAB)
            {
                mrRenderContext.SetLineColor(mrStyleSettings.GetShadowColor());
                mrRenderContext.DrawLine(Point(aOutRect.Left(), 0), Point(aOutputSize.Width(), 0));
            }

            // draw border (line above and line below)
            mrRenderContext.SetLineColor(mrStyleSettings.GetDarkShadowColor());
            mrRenderContext.DrawLine(aOutRect.TopLeft(), Point(aOutputSize.Width() - 1, aOutRect.Top()));
        }
    }

    void drawOuterFrame()
    {
        mrRenderContext.SetLineColor(mrStyleSettings.GetDarkShadowColor());

        // set correct FillInBrush depending on status
        if (mbSelected)
        {
            // Currently selected Tab
            mrRenderContext.SetFillColor(maSelectedColor);
        }
        else if (mbCustomColored)
        {
            mrRenderContext.SetFillColor(maCustomColor);
        }
        else
        {
            mrRenderContext.SetFillColor(maUnselectedColor);
        }

        mrRenderContext.DrawRect(maRect);
    }

    void drawText(const OUString& aText)
    {
        Rectangle aRect = maRect;
        long nTextWidth = mrRenderContext.GetTextWidth(aText);
        long nTextHeight = mrRenderContext.GetTextHeight();
        Point aPos = aRect.TopLeft();
        aPos.X() += (aRect.getWidth()  - nTextWidth) / 2;
        aPos.Y() += (aRect.getHeight() - nTextHeight) / 2;

        if (mbEnabled)
            mrRenderContext.DrawText(aPos, aText);
        else
            mrRenderContext.DrawCtrlText(aPos, aText, 0, aText.getLength(), (DrawTextFlags::Disable | DrawTextFlags::Mnemonic));
    }

    void drawOverTopBorder()
    {
        Point aTopLeft  = maRect.TopLeft()  + Point(1, 0);
        Point aTopRight = maRect.TopRight() + Point(-1, 0);

        Rectangle aDelRect(aTopLeft, aTopRight);
        mrRenderContext.DrawRect(aDelRect);
    }

    void drawColorLine()
    {
        mrRenderContext.SetFillColor(maCustomColor);
        mrRenderContext.SetLineColor(maCustomColor);

        Rectangle aLineRect(maRect.BottomLeft(), maRect.BottomRight());
        aLineRect.Top() -= 3;

        mrRenderContext.DrawRect(aLineRect);
    }

    void drawTab()
    {
        drawOuterFrame();

        if (mbCustomColored && mbSelected)
        {
            drawColorLine();
        }
    }

    void setRect(const Rectangle& rRect)
    {
        maRect = rRect;
    }

    void setSelected(bool bSelected)
    {
        mbSelected = bSelected;
    }

    void setCustomColored(bool bCustomColored)
    {
        mbCustomColored = bCustomColored;
    }

    void setSpecialTab(bool bSpecialTab)
    {
        mbSpecialTab = bSpecialTab;
    }

    void setEnabled(bool bEnabled)
    {
        mbEnabled = bEnabled;
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
};

} // anonymous namespace

struct ImplTabBarItem
{
    sal_uInt16 mnId;
    TabBarPageBits mnBits;
    OUString maText;
    OUString maHelpText;
    Rectangle maRect;
    long mnWidth;
    OString maHelpId;
    bool mbShort : 1;
    bool mbSelect : 1;
    Color maTabBgColor;
    Color maTabTextColor;

    ImplTabBarItem(sal_uInt16 nItemId, const OUString& rText, TabBarPageBits nPageBits)
        : mnId(nItemId)
        , mnBits(nPageBits)
        , maText(rText)
        , mnWidth(0)
        , mbShort(false)
        , mbSelect(false)
        , maTabBgColor(Color(COL_AUTO))
        , maTabTextColor(Color(COL_AUTO))
    {
    }

    bool IsDefaultTabBgColor() const
    {
        return maTabBgColor == Color(COL_AUTO);
    }

    bool IsSelected(ImplTabBarItem* pCurItem) const
    {
        return mbSelect || (pCurItem == this);
    }
};

class ImplTabButton : public PushButton
{
    bool mbModKey : 1;

public:
    ImplTabButton(TabBar* pParent, WinBits nWinStyle = 0)
        : PushButton(pParent, nWinStyle | WB_FLATBUTTON | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOLIGHTBORDER | WB_NOPOINTERFOCUS)
        , mbModKey(false)
    {}

    TabBar* GetParent() const
    {
        return static_cast<TabBar*>(Window::GetParent());
    }

    bool isModKeyPressed()
    {
        return mbModKey;
    }

    virtual bool PreNotify(NotifyEvent& rNotifyEvent) override;
    virtual void MouseButtonDown(const MouseEvent& rMouseEvent) override;
    virtual void MouseButtonUp(const MouseEvent& rMouseEvent) override;
    virtual void Command(const CommandEvent& rCommandEvent) override;
};

void ImplTabButton::MouseButtonDown(const MouseEvent& rMouseEvent)
{
    mbModKey = rMouseEvent.IsMod1();
    PushButton::MouseButtonDown(rMouseEvent);
}

void ImplTabButton::MouseButtonUp(const MouseEvent& rMouseEvent)
{
    mbModKey = false;
    PushButton::MouseButtonUp(rMouseEvent);
}

void ImplTabButton::Command(const CommandEvent& rCommandEvent)
{
    if (rCommandEvent.GetCommand() == CommandEventId::ContextMenu)
    {
        TabBar* pParent = GetParent();
        pParent->maScrollAreaContextHdl.Call(rCommandEvent);
    }
    PushButton::Command(rCommandEvent);
}

bool ImplTabButton::PreNotify(NotifyEvent& rNotifyEvent)
{
    if (rNotifyEvent.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN)
    {
        if (GetParent()->IsInEditMode())
        {
            GetParent()->EndEditMode();
            return true;
        }
    }

    return PushButton::PreNotify(rNotifyEvent);
}

class ImplTabSizer : public vcl::Window
{
public:
                    ImplTabSizer( TabBar* pParent, WinBits nWinStyle = 0 );

    TabBar*         GetParent() const { return static_cast<TabBar*>(Window::GetParent()); }

private:
    void            ImplTrack( const Point& rScreenPos );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect ) override;

    Point           maStartPos;
    long            mnStartWidth;
};

ImplTabSizer::ImplTabSizer( TabBar* pParent, WinBits nWinStyle )
    : Window( pParent, nWinStyle & WB_3DLOOK )
    , mnStartWidth(0)
{
    sal_Int32 nScaleFactor = GetDPIScaleFactor();
    SetPointer(Pointer(PointerStyle::HSizeBar));
    SetSizePixel(Size(7 * nScaleFactor, 0));
}

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

void ImplTabSizer::Paint( vcl::RenderContext& rRenderContext, const Rectangle& )
{
    DecorationView aDecoView(&rRenderContext);
    Rectangle aOutputRect(Point(0, 0), GetOutputSizePixel());
    aDecoView.DrawHandle(aOutputRect);
}

// Is not named Impl. as it may be both instantiated and derived from
class TabBarEdit : public Edit
{
private:
    Idle            maLoseFocusIdle;
    bool            mbPostEvt;

                    DECL_LINK_TYPED( ImplEndEditHdl, void*, void );
                    DECL_LINK_TYPED( ImplEndTimerHdl, Idle*, void );

public:
                    TabBarEdit( TabBar* pParent, WinBits nWinStyle = 0 );

    TabBar*         GetParent() const { return static_cast<TabBar*>(Window::GetParent()); }

    void            SetPostEvent() { mbPostEvt = true; }
    void            ResetPostEvent() { mbPostEvt = false; }

    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual void    LoseFocus() override;
};

TabBarEdit::TabBarEdit( TabBar* pParent, WinBits nWinStyle ) :
    Edit( pParent, nWinStyle )
{
    mbPostEvt = false;
    maLoseFocusIdle.SetPriority( SchedulerPriority::REPAINT );
    maLoseFocusIdle.SetIdleHdl( LINK( this, TabBarEdit, ImplEndTimerHdl ) );
    maLoseFocusIdle.SetDebugName( "svtools::TabBarEdit maLoseFocusIdle" );
}

bool TabBarEdit::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        if ( !pKEvt->GetKeyCode().GetModifier() )
        {
            if ( pKEvt->GetKeyCode().GetCode() == KEY_RETURN )
            {
                if ( !mbPostEvt )
                {
                    if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), reinterpret_cast<void*>(false), true ) )
                        mbPostEvt = true;
                }
                return true;
            }
            else if ( pKEvt->GetKeyCode().GetCode() == KEY_ESCAPE )
            {
                if ( !mbPostEvt )
                {
                    if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), reinterpret_cast<void*>(true), true ) )
                        mbPostEvt = true;
                }
                return true;
            }
        }
    }

    return Edit::PreNotify( rNEvt );
}

void TabBarEdit::LoseFocus()
{
    if ( !mbPostEvt )
    {
        if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), reinterpret_cast<void*>(false), true ) )
            mbPostEvt = true;
    }

    Edit::LoseFocus();
}

IMPL_LINK_TYPED( TabBarEdit, ImplEndEditHdl, void*, pCancel, void )
{
    ResetPostEvent();
    maLoseFocusIdle.Stop();

    // We need this query, because the edit gets a losefocus event,
    // when it shows the context menu or the insert symbol dialog
    if ( !HasFocus() && HasChildPathFocus( true ) )
    {
        maLoseFocusIdle.Start();
    }
    else
        GetParent()->EndEditMode( pCancel != nullptr );
}

IMPL_LINK_NOARG_TYPED(TabBarEdit, ImplEndTimerHdl, Idle *, void)
{
    if ( HasFocus() )
        return;

    // We need this query, because the edit gets a losefocus event,
    // when it shows the context menu or the insert symbol dialog
    if ( HasChildPathFocus( true ) )
        maLoseFocusIdle.Start();
    else
        GetParent()->EndEditMode( true );
}

struct TabBar_Impl
{
    ScopedVclPtr<ImplTabSizer>  mpSizer;
    ScopedVclPtr<ImplTabButton> mpFirstButton;
    ScopedVclPtr<ImplTabButton> mpPrevButton;
    ScopedVclPtr<ImplTabButton> mpNextButton;
    ScopedVclPtr<ImplTabButton> mpLastButton;
    ScopedVclPtr<ImplTabButton> mpAddButton;
    ScopedVclPtr<TabBarEdit>    mpEdit;
    ImplTabBarList              mpItemList;

    svt::AccessibleFactoryAccess  maAccessibleFactory;

    TabBar_Impl() {}

    ~TabBar_Impl()
    {
        for (ImplTabBarItem* i : mpItemList)
        {
            delete i;
        }
        mpItemList.clear();
    }

    sal_uInt16 getItemSize()
    {
        return static_cast<sal_uInt16>(mpItemList.size());
    }
};

TabBar::TabBar( vcl::Window* pParent, WinBits nWinStyle ) :
    Window( pParent, (nWinStyle & WB_3DLOOK) | WB_CLIPCHILDREN )
{
    ImplInit( nWinStyle );
    maCurrentItemList = 0;
}

TabBar::~TabBar()
{
    disposeOnce();
}

void TabBar::dispose()
{
    EndEditMode( true );
    mpImpl.reset();
    Window::dispose();
}

const sal_uInt16 TabBar::APPEND         = ::std::numeric_limits<sal_uInt16>::max();
const sal_uInt16 TabBar::PAGE_NOT_FOUND = ::std::numeric_limits<sal_uInt16>::max();

void TabBar::ImplInit( WinBits nWinStyle )
{
    mpImpl.reset(new TabBar_Impl);

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
    mbFormat        = true;
    mbFirstFormat   = true;
    mbSizeFormat    = true;
    mbInSwitching   = false;
    mbAutoEditMode  = false;
    mbEditCanceled  = false;
    mbDropPos       = false;
    mbInSelect      = false;
    mbMirrored      = false;
    mbScrollAlwaysEnabled = false;

    if ( nWinStyle & WB_3DTAB )
        mnOffY++;

    ImplInitControls();

    if (mpImpl->mpFirstButton)
        mpImpl->mpFirstButton->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVET0HOME));
    if (mpImpl->mpPrevButton)
        mpImpl->mpPrevButton->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVELEFT));
    if (mpImpl->mpNextButton)
        mpImpl->mpNextButton->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVERIGHT));
    if (mpImpl->mpLastButton)
        mpImpl->mpLastButton->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_MOVETOEND));

    if (mpImpl->mpAddButton)
        mpImpl->mpAddButton->SetAccessibleName(SVT_RESSTR(STR_TABBAR_PUSHBUTTON_ADDTAB));

    SetSizePixel( Size( 100, CalcWindowSizePixel().Height() ) );
    ImplInitSettings( true, true );
}

ImplTabBarItem* TabBar::seek( size_t i )
{
    if ( i < mpImpl->mpItemList.size() )
    {
        maCurrentItemList = i;
        return mpImpl->mpItemList[maCurrentItemList];
    }
    return nullptr;
}

ImplTabBarItem* TabBar::prev()
{
    if ( maCurrentItemList > 0 )
    {
        return mpImpl->mpItemList[--maCurrentItemList];
    }
    return nullptr;
}

ImplTabBarItem* TabBar::next()
{
    if ( maCurrentItemList + 1 < mpImpl->mpItemList.size() )
    {
        return mpImpl->mpItemList[++maCurrentItemList];
    }
    return nullptr;
}

void TabBar::ImplInitSettings( bool bFont, bool bBackground )
{
    // FIXME RenderContext

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if (bFont)
    {
        vcl::Font aToolFont;
        aToolFont = rStyleSettings.GetToolFont();
        aToolFont.SetWeight( WEIGHT_BOLD );
        ApplyControlFont(*this, aToolFont);

        // Adapt font size if window too small?
        while (GetTextHeight() > (GetOutputSizePixel().Height() - 1))
        {
            vcl::Font aFont = GetFont();
            if (aFont.GetFontHeight() <= 6)
                break;
            aFont.SetFontHeight(aFont.GetFontHeight() - 1);
            SetFont(aFont);
        }
    }

    if (bBackground)
    {
        ApplyControlBackground(*this, rStyleSettings.GetFaceColor());
    }
}

void TabBar::ImplGetColors(const StyleSettings& rStyleSettings,
                           Color& rFaceColor, Color& rFaceTextColor,
                           Color& rSelectColor, Color& rSelectTextColor)
{
    if (IsControlBackground())
        rFaceColor = GetControlBackground();
    else
        rFaceColor = rStyleSettings.GetInactiveTabColor();
    if (IsControlForeground())
        rFaceTextColor = GetControlForeground();
    else
        rFaceTextColor = rStyleSettings.GetButtonTextColor();
    rSelectColor = rStyleSettings.GetActiveTabColor();
    rSelectTextColor = rStyleSettings.GetWindowTextColor();

    // For 3D-tabs the selection- and face-colours are swapped,
    // as the selected tabs should appear in 3D
    if (mnWinStyle & WB_3DTAB)
    {
        using std::swap;
        swap(rFaceColor, rSelectColor);
        swap(rFaceTextColor, rSelectTextColor);
    }
}

bool TabBar::ImplCalcWidth()
{
    // Sizes should only be retrieved if the text or the font was changed
    if (!mbSizeFormat)
        return false;

    // retrieve width of tabs with bold font
    vcl::Font aFont = GetFont();
    if (aFont.GetWeight() != WEIGHT_BOLD)
    {
        aFont.SetWeight(WEIGHT_BOLD);
        SetFont(aFont);
    }

    if (mnMaxPageWidth)
        mnCurMaxWidth = mnMaxPageWidth;
    else
    {
        mnCurMaxWidth = mnLastOffX - mnOffX;
        if (mnCurMaxWidth < 1)
            mnCurMaxWidth = 1;
    }

    bool bChanged = false;
    for (ImplTabBarItem* pItem : mpImpl->mpItemList)
    {
        long nNewWidth = GetTextWidth(pItem->maText);
        if (mnCurMaxWidth && (nNewWidth > mnCurMaxWidth))
        {
            pItem->mbShort = true;
            nNewWidth = mnCurMaxWidth;
        }
        else
        {
            pItem->mbShort = false;
        }

        // Padding is dependent on font height - bigger font = bigger padding
        long nFontWidth = aFont.GetFontHeight();
        nNewWidth += nFontWidth * 2;

        if (pItem->mnWidth != nNewWidth)
        {
            pItem->mnWidth = nNewWidth;
            if (!pItem->maRect.IsEmpty())
                bChanged = true;
        }
    }
    mbSizeFormat = false;
    mbFormat = true;
    return bChanged;
}

void TabBar::ImplFormat()
{
    ImplCalcWidth();

    if (!mbFormat)
        return;

    sal_uInt16 n = 0;
    long x = mnOffX;
    for (ImplTabBarItem* pItem : mpImpl->mpItemList)
    {
        // At all non-visible tabs an empty rectangle is set
        if ((n + 1 < mnFirstPos) || (x > mnLastOffX))
            pItem->maRect.SetEmpty();
        else
        {
            // Slightly before the tab before the first visible page
            // should also be visible
            if (n + 1 == mnFirstPos)
                pItem->maRect.Left() = x-pItem->mnWidth;
            else
            {
                pItem->maRect.Left() = x;
                x += pItem->mnWidth;
            }
            pItem->maRect.Right() = x;
            pItem->maRect.Bottom() = maWinSize.Height() - 1;

            if (mbMirrored)
            {
                long nTmp = mnOffX + mnLastOffX - pItem->maRect.Right();
                pItem->maRect.Right() = mnOffX + mnLastOffX - pItem->maRect.Left();
                pItem->maRect.Left() = nTmp;
            }
        }

        n++;
    }

    mbFormat = false;

    //  enable/disable button
    ImplEnableControls();
}

sal_uInt16 TabBar::ImplGetLastFirstPos()
{
    sal_uInt16 nCount = mpImpl->getItemSize();
    if (!nCount || mbSizeFormat || mbFormat)
        return 0;

    sal_uInt16  nLastFirstPos = nCount - 1;
    long nWinWidth = mnLastOffX - mnOffX - ADDNEWPAGE_AREAWIDTH;
    long nWidth = mpImpl->mpItemList[nLastFirstPos]->mnWidth;

    while (nLastFirstPos && (nWidth < nWinWidth))
    {
        nLastFirstPos--;
        nWidth += mpImpl->mpItemList[nLastFirstPos]->mnWidth;
    }
    if ((nLastFirstPos != static_cast<sal_uInt16>(mpImpl->mpItemList.size() - 1)) && (nWidth > nWinWidth))
        nLastFirstPos++;
    return nLastFirstPos;
}

void TabBar::ImplInitControls()
{
    if (mnWinStyle & WB_SIZEABLE)
    {
        if (!mpImpl->mpSizer)
        {
            mpImpl->mpSizer.disposeAndReset(VclPtr<ImplTabSizer>::Create( this, mnWinStyle & (WB_DRAG | WB_3DLOOK)));
        }
        mpImpl->mpSizer->Show();
    }
    else
    {
        mpImpl->mpSizer.disposeAndClear();
    }

    if ((mnWinStyle & WB_INSERTTAB) && !mpImpl->mpAddButton)
    {
        Link<Button*,void> aLink = LINK(this, TabBar, ImplAddClickHandler);
        mpImpl->mpAddButton.disposeAndReset(VclPtr<ImplTabButton>::Create(this, WB_REPEAT));
        mpImpl->mpAddButton->SetClickHdl(aLink);
        mpImpl->mpAddButton->SetSymbol(SymbolType::PLUS);
        mpImpl->mpAddButton->Show();
    }

    Link<Button*,void> aLink = LINK( this, TabBar, ImplClickHdl );

    if (mnWinStyle & (WB_MINSCROLL | WB_SCROLL))
    {
        if (!mpImpl->mpPrevButton)
        {
            mpImpl->mpPrevButton.disposeAndReset(VclPtr<ImplTabButton>::Create(this, WB_REPEAT));
            mpImpl->mpPrevButton->SetClickHdl(aLink);
        }
        mpImpl->mpPrevButton->SetSymbol(mbMirrored ? SymbolType::NEXT : SymbolType::PREV);
        mpImpl->mpPrevButton->Show();

        if (!mpImpl->mpNextButton)
        {
            mpImpl->mpNextButton.disposeAndReset(VclPtr<ImplTabButton>::Create(this, WB_REPEAT));
            mpImpl->mpNextButton->SetClickHdl(aLink);
        }
        mpImpl->mpNextButton->SetSymbol(mbMirrored ? SymbolType::PREV : SymbolType::NEXT);
        mpImpl->mpNextButton->Show();
    }
    else
    {
        mpImpl->mpPrevButton.disposeAndClear();
        mpImpl->mpNextButton.disposeAndClear();
    }

    if (mnWinStyle & WB_SCROLL)
    {
        if (!mpImpl->mpFirstButton)
        {
            mpImpl->mpFirstButton.disposeAndReset(VclPtr<ImplTabButton>::Create(this));
            mpImpl->mpFirstButton->SetClickHdl(aLink);
        }
        mpImpl->mpFirstButton->SetSymbol(mbMirrored ? SymbolType::LAST : SymbolType::FIRST);
        mpImpl->mpFirstButton->Show();

        if (!mpImpl->mpLastButton)
        {
            mpImpl->mpLastButton.disposeAndReset(VclPtr<ImplTabButton>::Create(this));
            mpImpl->mpLastButton->SetClickHdl(aLink);
        }
        mpImpl->mpLastButton->SetSymbol(mbMirrored ? SymbolType::FIRST : SymbolType::LAST);
        mpImpl->mpLastButton->Show();
    }
    else
    {
        mpImpl->mpFirstButton.disposeAndClear();
        mpImpl->mpLastButton.disposeAndClear();
    }
}

void TabBar::ImplEnableControls()
{
    if (mbSizeFormat || mbFormat)
        return;

    // enable/disable buttons
    bool bEnableBtn = mbScrollAlwaysEnabled || mnFirstPos > 0;
    if (mpImpl->mpFirstButton)
        mpImpl->mpFirstButton->Enable(bEnableBtn);
    if (mpImpl->mpPrevButton)
        mpImpl->mpPrevButton->Enable(bEnableBtn);

    bEnableBtn = mbScrollAlwaysEnabled || mnFirstPos < ImplGetLastFirstPos();
    if (mpImpl->mpNextButton)
        mpImpl->mpNextButton->Enable(bEnableBtn);
    if (mpImpl->mpLastButton)
        mpImpl->mpLastButton->Enable(bEnableBtn);
}

void TabBar::SetScrollAlwaysEnabled(bool bScrollAlwaysEnabled)
{
    mbScrollAlwaysEnabled = bScrollAlwaysEnabled;
    ImplEnableControls();
}

void TabBar::ImplShowPage( sal_uInt16 nPos )
{
    if (nPos >= mpImpl->getItemSize())
        return;

    // calculate width
    long nWidth = GetOutputSizePixel().Width();

    ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];
    if (nPos < mnFirstPos)
        SetFirstPageId( pItem->mnId );
    else if (pItem->maRect.Right() > nWidth)
    {
        while (pItem->maRect.Right() > nWidth)
        {
            sal_uInt16 nNewPos = mnFirstPos + 1;
            SetFirstPageId(GetPageId(nNewPos));
            ImplFormat();
            if (nNewPos != mnFirstPos)
                break;
        }
    }
}

IMPL_LINK_TYPED( TabBar, ImplClickHdl, Button*, pButton, void )
{
    ImplTabButton* pBtn = static_cast<ImplTabButton*>(pButton);
    EndEditMode();

    sal_uInt16 nNewPos = mnFirstPos;

    if (pBtn == mpImpl->mpFirstButton.get() || (pBtn == mpImpl->mpPrevButton.get() && pBtn->isModKeyPressed()))
    {
        nNewPos = 0;
    }
    else if (pBtn == mpImpl->mpLastButton.get() || (pBtn == mpImpl->mpNextButton.get() && pBtn->isModKeyPressed()))
    {
        sal_uInt16 nCount = GetPageCount();
        if (nCount)
            nNewPos = nCount - 1;
    }
    else if (pBtn == mpImpl->mpPrevButton.get())
    {
        if (mnFirstPos)
            nNewPos = mnFirstPos - 1;
    }
    else if (pBtn == mpImpl->mpNextButton.get())
    {
        sal_uInt16 nCount = GetPageCount();
        if (mnFirstPos <  nCount)
            nNewPos = mnFirstPos+1;
    }
    else
    {
        return;
    }

    if (nNewPos != mnFirstPos)
        SetFirstPageId(GetPageId(nNewPos));
}

IMPL_LINK_NOARG_TYPED(TabBar, ImplAddClickHandler, Button*, void)
{
    AddTabClick();
}

void TabBar::MouseMove(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeaveWindow())
        mbInSelect = false;

    Window::MouseMove(rMEvt);
}

void TabBar::MouseButtonDown(const MouseEvent& rMEvt)
{
    // Only terminate EditModus and do not execute click
    // if clicked inside our window,
    if (IsInEditMode())
    {
        EndEditMode();
        return;
    }

    sal_uInt16 nSelId = GetPageId(rMEvt.GetPosPixel());

    if (!rMEvt.IsLeft())
    {
        Window::MouseButtonDown(rMEvt);
        if (nSelId > 0 && nSelId != mnCurPageId)
        {
            if (ImplDeactivatePage())
            {
                SetCurPageId(nSelId);
                Update();
                ImplActivatePage();
                ImplSelect();
            }
            mbInSelect = true;
        }
        return;
    }

    if (rMEvt.IsMod2() && mbAutoEditMode && nSelId)
    {
        if (StartEditMode(nSelId))
            return;
    }

    if ((rMEvt.GetMode() & (MouseEventModifiers::MULTISELECT | MouseEventModifiers::RANGESELECT)) && (rMEvt.GetClicks() == 1))
    {
        if (nSelId)
        {
            sal_uInt16  nPos = GetPagePos(nSelId);

            bool bSelectTab = false;

            if ((rMEvt.GetMode() & MouseEventModifiers::MULTISELECT) && (mnWinStyle & WB_MULTISELECT))
            {
                if (nSelId != mnCurPageId)
                {
                    SelectPage(nSelId, !IsPageSelected(nSelId));
                    bSelectTab = true;
                }
            }
            else if (mnWinStyle & (WB_MULTISELECT | WB_RANGESELECT))
            {
                bSelectTab = true;
                sal_uInt16 n;
                bool bSelect;
                sal_uInt16 nCurPos = GetPagePos(mnCurPageId);
                if (nPos <= nCurPos)
                {
                    // Deselect all tabs till the clicked tab
                    // and select all tabs from the clicked tab
                    // 'till the actual position
                    n = 0;
                    while (n < nCurPos)
                    {
                        auto pItem = mpImpl->mpItemList[n];
                        if (n < nPos)
                            bSelect = false;
                        else
                            bSelect = true;

                        if (pItem->mbSelect != bSelect)
                        {
                            pItem->mbSelect = bSelect;
                            if (!pItem->maRect.IsEmpty())
                                Invalidate(pItem->maRect);
                        }

                        n++;
                    }
                }

                if (nPos >= nCurPos)
                {
                    // Select all tabs from the actual position till the clicked tab
                    // and deselect all tabs from the actual position
                    // till the last tab
                    sal_uInt16 nCount = mpImpl->getItemSize();
                    n = nCurPos;
                    while (n < nCount)
                    {
                        auto pItem = mpImpl->mpItemList[n];

                        if (n <= nPos)
                            bSelect = true;
                        else
                            bSelect = false;

                        if (pItem->mbSelect != bSelect)
                        {
                            pItem->mbSelect = bSelect;
                            if (!pItem->maRect.IsEmpty())
                                Invalidate(pItem->maRect);
                        }

                        n++;
                    }
                }
            }

            // scroll the selected tab if required
            if (bSelectTab)
            {
                ImplShowPage(nPos);
                Update();
                ImplSelect();
            }

            mbInSelect = true;

            return;
        }
    }
    else if (rMEvt.GetClicks() == 2)
    {
        // call double-click-handler if required
        if (!rMEvt.GetModifier() && (!nSelId || (nSelId == mnCurPageId)))
        {
            sal_uInt16 nOldCurId = mnCurPageId;
            mnCurPageId = nSelId;
            DoubleClick();
            // check, as actual page could be switched inside
            // the doubleclick-handler
            if (mnCurPageId == nSelId)
                mnCurPageId = nOldCurId;
        }

        return;
    }
    else
    {
        if (nSelId)
        {
            // execute Select if not actual page
            if (nSelId != mnCurPageId)
            {
                sal_uInt16 nPos = GetPagePos(nSelId);
                auto pItem = mpImpl->mpItemList[nPos];

                if (!pItem->mbSelect)
                {
                    // make not valid
                    bool bUpdate = false;
                    if (IsReallyVisible() && IsUpdateMode())
                        bUpdate = true;

                    // deselect all selected items
                    for (ImplTabBarItem* i : mpImpl->mpItemList)
                    {
                        pItem = i;
                        if (pItem->mbSelect || (pItem->mnId == mnCurPageId))
                        {
                            pItem->mbSelect = false;
                            if (bUpdate)
                                Invalidate(pItem->maRect);
                        }
                    }
                }

                if (ImplDeactivatePage())
                {
                    SetCurPageId(nSelId);
                    Update();
                    ImplActivatePage();
                    ImplSelect();
                }

                mbInSelect = true;
            }

            return;
        }
    }

    Window::MouseButtonDown(rMEvt);
}

void TabBar::MouseButtonUp(const MouseEvent& rMEvt)
{
    mbInSelect = false;
    Window::MouseButtonUp(rMEvt);
}

void TabBar::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rect)
{
    if (rRenderContext.IsNativeControlSupported(ControlType::WindowBackground,ControlPart::Entire))
    {
        rRenderContext.DrawNativeControl(ControlType::WindowBackground,ControlPart::Entire,rect,
                                         ControlState::ENABLED,ImplControlValue(0),OUString());
    }
    // calculate items and emit
    sal_uInt16 nItemCount = mpImpl->getItemSize();
    if (!nItemCount)
        return;

    ImplPrePaint(rRenderContext);

    Color aFaceColor, aSelectColor, aFaceTextColor, aSelectTextColor;
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    ImplGetColors(rStyleSettings, aFaceColor, aFaceTextColor, aSelectColor, aSelectTextColor);

    rRenderContext.Push(PushFlags::FONT | PushFlags::CLIPREGION);
    rRenderContext.SetClipRegion(vcl::Region(GetPageArea()));

    // select font
    vcl::Font aFont = rRenderContext.GetFont();
    vcl::Font aLightFont = aFont;
    aLightFont.SetWeight(WEIGHT_NORMAL);

    TabDrawer aDrawer(*this, rRenderContext);

    aDrawer.setSelectedFillColor(aSelectColor);
    aDrawer.setUnselectedFillColor(aFaceColor);

    aDrawer.drawOutputAreaBorder();

    // Now, start drawing the tabs.

    ImplTabBarItem* pItem = ImplGetLastTabBarItem(nItemCount);
    ImplTabBarItem* pCurItem = nullptr;
    while (pItem)
    {
        // emit CurrentItem last, as it covers all others
        if (!pCurItem && (pItem->mnId == mnCurPageId))
        {
            pCurItem = pItem;
            pItem = prev();
            if (!pItem)
                pItem = pCurItem;
            continue;
        }

        bool bCurrent = pItem == pCurItem;

        if (!pItem->maRect.IsEmpty())
        {
            Rectangle aRect = pItem->maRect;
            bool bSelected = pItem->IsSelected(pCurItem);
            // We disable custom background color in high contrast mode.
            bool bCustomBgColor = !pItem->IsDefaultTabBgColor() && !rStyleSettings.GetHighContrastMode();
            bool bSpecialTab = (pItem->mnBits & TPB_SPECIAL);
            OUString aText = pItem->mbShort ? rRenderContext.GetEllipsisString(pItem->maText, mnCurMaxWidth) : pItem->maText;

            aDrawer.setRect(aRect);
            aDrawer.setSelected(bSelected);
            aDrawer.setCustomColored(bCustomBgColor);
            aDrawer.setSpecialTab(bSpecialTab);
            aDrawer.setEnabled(true);
            aDrawer.setCustomColor(pItem->maTabBgColor);
            aDrawer.drawTab();

            // actual page is drawn using a bold font
            if (bCurrent)
                rRenderContext.SetFont(aFont);
            else
                rRenderContext.SetFont(aLightFont);

            // Set the correct FillInBrush depending on status

            if (bSelected)
                rRenderContext.SetTextColor(aSelectTextColor);
            else if (bCustomBgColor)
                rRenderContext.SetTextColor(pItem->maTabTextColor);
            else
                rRenderContext.SetTextColor(aFaceTextColor);

            // This tab is "special", and a special tab needs a blue text.
            if (bSpecialTab)
                rRenderContext.SetTextColor(Color(COL_LIGHTBLUE));

            aDrawer.drawText(aText);

            if (bCurrent)
            {
                rRenderContext.SetLineColor();
                rRenderContext.SetFillColor(aSelectColor);
                aDrawer.drawOverTopBorder();
                break;
            }

            pItem = prev();
        }
        else
        {
            if (bCurrent)
                break;

            pItem = nullptr;
        }

        if (!pItem)
            pItem = pCurItem;
    }
    rRenderContext.Pop();
}

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

    // order the scroll buttons
    long const nHeight = aNewSize.Height();
    // adapt font height?
    ImplInitSettings( true, false );

    long nButtonMargin = BUTTON_MARGIN * GetDPIScaleFactor();

    long nX = mbMirrored ? (aNewSize.Width() - nHeight - nButtonMargin) : nButtonMargin;
    long const nXDiff = mbMirrored ? -nHeight : nHeight;

    nButtonWidth += nButtonMargin;

    Size const aBtnSize( nHeight, nHeight );
    auto setButton = [aBtnSize, nXDiff, nHeight, &nX, &nButtonWidth](
        ScopedVclPtr<ImplTabButton> const & button)
    {
        if (button) {
            button->SetPosSizePixel(Point(nX, 0), aBtnSize);
            nX += nXDiff;
            nButtonWidth += nHeight;
        }
    };

    setButton(mpImpl->mpFirstButton);
    setButton(mpImpl->mpPrevButton);
    setButton(mpImpl->mpNextButton);
    setButton(mpImpl->mpLastButton);

    nButtonWidth += nButtonMargin;
    nX += mbMirrored ? -nButtonMargin : nButtonMargin;

    setButton(mpImpl->mpAddButton);

    nButtonWidth += nButtonMargin;

    // store size
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

    // reformat
    mbSizeFormat = true;
    if ( IsReallyVisible() )
    {
        if ( ImplCalcWidth() )
            Invalidate();

        ImplFormat();

        // Ensure as many tabs as possible are visible:
        sal_uInt16 nLastFirstPos = ImplGetLastFirstPos();
        if ( mnFirstPos > nLastFirstPos )
        {
            mnFirstPos = nLastFirstPos;
            mbFormat = true;
            Invalidate();
        }
        // Ensure the currently selected page is visible
        ImplShowPage(GetPagePos(mnCurPageId));

        ImplFormat();
    }

    // enable/disable button
    ImplEnableControls();
}

bool TabBar::PreNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        if (rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel)
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            sal_uInt16 nNewPos = mnFirstPos;
            if (pData->GetNotchDelta() > 0)
            {
                if (mnFirstPos)
                    nNewPos = mnFirstPos - 1;
            }
            else if (pData->GetNotchDelta() < 0)
            {
                sal_uInt16 nCount = GetPageCount();
                if (mnFirstPos <  nCount)
                    nNewPos = mnFirstPos + 1;
            }
            if (nNewPos != mnFirstPos)
                SetFirstPageId(GetPageId(nNewPos));
        }
    }
    return Window::PreNotify(rNEvt);
}

void TabBar::RequestHelp(const HelpEvent& rHEvt)
{
    sal_uInt16 nItemId = GetPageId(ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
    if (nItemId)
    {
        if (rHEvt.GetMode() & HelpEventMode::BALLOON)
        {
            OUString aStr = GetHelpText(nItemId);
            if (!aStr.isEmpty())
            {
                Rectangle aItemRect = GetPageRect(nItemId);
                Point aPt = OutputToScreenPixel(aItemRect.TopLeft());
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel(aItemRect.BottomRight());
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                Help::ShowBalloon(this, aItemRect.Center(), aItemRect, aStr);
                return;
            }
        }
        else if (rHEvt.GetMode() & HelpEventMode::EXTENDED)
        {
            OUString aHelpId(OStringToOUString(GetHelpId(nItemId), RTL_TEXTENCODING_UTF8));
            if ( !aHelpId.isEmpty() )
            {
                // trigger Help if available
                Help* pHelp = Application::GetHelp();
                if (pHelp)
                    pHelp->Start(aHelpId, this);
                return;
            }
        }

        // show text for quick- or balloon-help
        // if this is isolated or not fully visible
        if (rHEvt.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON))
        {
            sal_uInt16 nPos = GetPagePos(nItemId);
            ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];
            if (pItem->mbShort || (pItem->maRect.Right() - 5 > mnLastOffX))
            {
                Rectangle aItemRect = GetPageRect(nItemId);
                Point aPt = OutputToScreenPixel(aItemRect.TopLeft());
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel(aItemRect.BottomRight());
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();
                OUString aStr = mpImpl->mpItemList[nPos]->maText;
                if (!aStr.isEmpty())
                {
                    if (rHEvt.GetMode() & HelpEventMode::BALLOON)
                        Help::ShowBalloon(this, aItemRect.Center(), aItemRect, aStr);
                    else
                        Help::ShowQuickHelp(this, aItemRect, aStr);
                    return;
                }
            }
        }
    }

    Window::RequestHelp(rHEvt);
}

void TabBar::StateChanged(StateChangedType nType)
{
    Window::StateChanged(nType);

    if (nType == StateChangedType::InitShow)
    {
        if ( (mbSizeFormat || mbFormat) && !mpImpl->mpItemList.empty() )
            ImplFormat();
    }
    else if (nType == StateChangedType::Zoom ||
             nType == StateChangedType::ControlFont)
    {
        ImplInitSettings(true, false);
        Invalidate();
    }
    else if (nType == StateChangedType::ControlForeground)
        Invalidate();
    else if (nType == StateChangedType::ControlBackground)
    {
        ImplInitSettings(false, true);
        Invalidate();
    }
    else if (nType == StateChangedType::Mirroring)
    {
        // reacts on calls of EnableRTL, have to mirror all child controls
        if (mpImpl->mpFirstButton)
            mpImpl->mpFirstButton->EnableRTL(IsRTLEnabled());
        if (mpImpl->mpPrevButton)
            mpImpl->mpPrevButton->EnableRTL(IsRTLEnabled());
        if (mpImpl->mpNextButton)
            mpImpl->mpNextButton->EnableRTL(IsRTLEnabled());
        if (mpImpl->mpLastButton)
            mpImpl->mpLastButton->EnableRTL(IsRTLEnabled());
        if (mpImpl->mpSizer)
            mpImpl->mpSizer->EnableRTL(IsRTLEnabled());
        if (mpImpl->mpAddButton)
            mpImpl->mpAddButton->EnableRTL(IsRTLEnabled());
        if (mpImpl->mpEdit)
            mpImpl->mpEdit->EnableRTL(IsRTLEnabled());
    }
}

void TabBar::DataChanged(const DataChangedEvent& rDCEvt)
{
    Window::DataChanged(rDCEvt);

    if (rDCEvt.GetType() == DataChangedEventType::FONTS
        || rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION
        || (rDCEvt.GetType() == DataChangedEventType::SETTINGS
            && rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        ImplInitSettings(true, true);
        Invalidate();
    }
}

void TabBar::ImplSelect()
{
    Select();
    CallEventListeners(VCLEVENT_TABBAR_PAGESELECTED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)));
}

void TabBar::Select()
{
    maSelectHdl.Call(this);
}

void TabBar::DoubleClick()
{
}

void TabBar::Split()
{
    maSplitHdl.Call(this);
}

void TabBar::ImplActivatePage()
{
    ActivatePage();

    CallEventListeners(VCLEVENT_TABBAR_PAGEACTIVATED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)));
}

void TabBar::ActivatePage()
{}

bool TabBar::ImplDeactivatePage()
{
    bool bRet = DeactivatePage();

    CallEventListeners(VCLEVENT_TABBAR_PAGEDEACTIVATED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)));

    return bRet;
}

void TabBar::ImplPrePaint(vcl::RenderContext& /*rRenderContext*/)
{
    sal_uInt16 nItemCount = mpImpl->getItemSize();
    if (!nItemCount)
        return;

    // tabbar should be formatted
    ImplFormat();

    // assure the actual tabpage becomes visible at first format
    if (mbFirstFormat)
    {
        mbFirstFormat = false;

        if (mnCurPageId && (mnFirstPos == 0) && !mbDropPos)
        {
            ImplTabBarItem* pItem = mpImpl->mpItemList[GetPagePos(mnCurPageId)];
            if (pItem->maRect.IsEmpty())
            {
                // set mbDropPos (or misuse) to prevent Invalidate()
                mbDropPos = true;
                SetFirstPageId(mnCurPageId);
                mbDropPos = false;
                if (mnFirstPos != 0)
                    ImplFormat();
            }
        }
    }
}

ImplTabBarItem* TabBar::ImplGetLastTabBarItem( sal_uInt16 nItemCount )
{
    // find last visible entry
    sal_uInt16 n = mnFirstPos + 1;
    if (n >= nItemCount)
        n = nItemCount-1;
    ImplTabBarItem* pItem = seek(n);
    while (pItem)
    {
        if (!pItem->maRect.IsEmpty())
        {
            n++;
            pItem = next();
        }
        else
            break;
    }

    // draw all tabs (from back to front and actual last)
    if (pItem)
        n--;
    else if (n >= nItemCount)
        n = nItemCount-1;
    pItem = seek(n);
    return pItem;
}

bool TabBar::DeactivatePage()
{
    return true;
}

bool TabBar::StartRenaming()
{
    return true;
}

TabBarAllowRenamingReturnCode TabBar::AllowRenaming()
{
    return TABBAR_RENAMING_YES;
}

void TabBar::EndRenaming()
{
}

void TabBar::Mirror()
{

}

void TabBar::AddTabClick()
{

}

void TabBar::InsertPage(sal_uInt16 nPageId, const OUString& rText,
                        TabBarPageBits nBits, sal_uInt16 nPos)
{
    DBG_ASSERT( nPageId, "TabBar::InsertPage(): PageId == 0" );
    DBG_ASSERT( GetPagePos( nPageId ) == PAGE_NOT_FOUND,
                "TabBar::InsertPage(): PageId already exists" );
    DBG_ASSERT( nBits <= TPB_SPECIAL, "TabBar::InsertPage(): nBits is wrong" );

    // create PageItem and insert in the item list
    ImplTabBarItem* pItem = new ImplTabBarItem( nPageId, rText, nBits );
    if (nPos < mpImpl->mpItemList.size())
    {
        ImplTabBarList::iterator it = mpImpl->mpItemList.begin();
        std::advance(it, nPos);
        mpImpl->mpItemList.insert(it, pItem);
    }
    else
    {
        mpImpl->mpItemList.push_back(pItem);
    }
    mbSizeFormat = true;

    // set CurPageId if required
    if (!mnCurPageId)
        mnCurPageId = nPageId;

    // redraw bar
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    CallEventListeners(VCLEVENT_TABBAR_PAGEINSERTED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)));
}

Color TabBar::GetTabBgColor(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->mpItemList[nPos]->maTabBgColor;
    else
        return Color(COL_AUTO);
}

void TabBar::SetTabBgColor(sal_uInt16 nPageId, const Color& aTabBgColor)
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
    {
        ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];
        if (aTabBgColor != Color(COL_AUTO))
        {
            pItem->maTabBgColor = aTabBgColor;
            if (aTabBgColor.GetLuminance() <= 128) //Do not use aTabBgColor.IsDark(), because that threshold is way too low...
                pItem->maTabTextColor = Color(COL_WHITE);
            else
                pItem->maTabTextColor = Color(COL_BLACK);
        }
        else
        {
            pItem->maTabBgColor = Color(COL_AUTO);
            pItem->maTabTextColor = Color(COL_AUTO);
        }
    }
}

void TabBar::RemovePage(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    // does item exist
    if (nPos != PAGE_NOT_FOUND)
    {
        if (mnCurPageId == nPageId)
            mnCurPageId = 0;

        // check if first visible page should be moved
        if (mnFirstPos > nPos)
            mnFirstPos--;

        // delete item data
        ImplTabBarList::iterator it = mpImpl->mpItemList.begin();
        std::advance(it, nPos);
        delete *it;
        mpImpl->mpItemList.erase(it);

        // redraw bar
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();

        CallEventListeners(VCLEVENT_TABBAR_PAGEREMOVED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)));
    }
}

void TabBar::MovePage(sal_uInt16 nPageId, sal_uInt16 nNewPos)
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    Pair aPair(nPos, nNewPos);

    if (nPos < nNewPos)
        nNewPos--;

    if (nPos == nNewPos)
        return;

    // does item exit
    if (nPos != PAGE_NOT_FOUND)
    {
        // move tabbar item in the list
        ImplTabBarList::iterator it = mpImpl->mpItemList.begin();
        std::advance(it, nPos);
        ImplTabBarItem* pItem = *it;
        mpImpl->mpItemList.erase(it);
        if (nNewPos < mpImpl->mpItemList.size())
        {
            it = mpImpl->mpItemList.begin();
            std::advance(it, nNewPos);
            mpImpl->mpItemList.insert(it, pItem);
        }
        else
        {
            mpImpl->mpItemList.push_back(pItem);
        }

        // redraw bar
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();

        CallEventListeners( VCLEVENT_TABBAR_PAGEMOVED, static_cast<void*>(&aPair) );
    }
}

void TabBar::Clear()
{
    // delete all items
    for (ImplTabBarItem* i : mpImpl->mpItemList)
    {
        delete i;
    }
    mpImpl->mpItemList.clear();

    // remove items from the list
    mbSizeFormat = true;
    mnCurPageId = 0;
    mnFirstPos = 0;
    maCurrentItemList = 0;

    // redraw bar
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    CallEventListeners(VCLEVENT_TABBAR_PAGEREMOVED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(PAGE_NOT_FOUND)));
}

bool TabBar::IsPageEnabled(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
        return true;
    else
        return false;
}

void TabBar::SetPageBits(sal_uInt16 nPageId, TabBarPageBits nBits)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
    {
        ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];

        if (pItem->mnBits != nBits)
        {
            pItem->mnBits = nBits;

            // redraw bar
            if (IsReallyVisible() && IsUpdateMode())
                Invalidate(pItem->maRect);
        }
    }
}

TabBarPageBits TabBar::GetPageBits(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->mpItemList[nPos]->mnBits;
    else
        return 0;
}

sal_uInt16 TabBar::GetPageCount() const
{
    return mpImpl->getItemSize();
}

sal_uInt16 TabBar::GetPageId(sal_uInt16 nPos) const
{
    return nPos < mpImpl->mpItemList.size() ? mpImpl->mpItemList[nPos]->mnId : 0;
}

sal_uInt16 TabBar::GetPagePos(sal_uInt16 nPageId) const
{
    for (size_t i = 0; i < mpImpl->mpItemList.size(); ++i)
    {
        if (mpImpl->mpItemList[i]->mnId == nPageId)
        {
            return static_cast<sal_uInt16>(i);
        }
    }
    return PAGE_NOT_FOUND;
}

sal_uInt16 TabBar::GetPageId(const Point& rPos) const
{
    for (ImplTabBarItem* pItem : mpImpl->mpItemList)
    {
        if (pItem->maRect.IsInside(rPos))
            return pItem->mnId;
    }

    return 0;
}

Rectangle TabBar::GetPageRect(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->mpItemList[nPos]->maRect;
    else
        return Rectangle();
}

void TabBar::SetCurPageId(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    // do nothing if item does not exit
    if (nPos != PAGE_NOT_FOUND)
    {
        // do nothing if the actual page did not change
        if (nPageId == mnCurPageId)
            return;

        // make invalide
        bool bUpdate = false;
        if (IsReallyVisible() && IsUpdateMode())
            bUpdate = true;

        ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];
        ImplTabBarItem* pOldItem;

        if (mnCurPageId)
            pOldItem = mpImpl->mpItemList[GetPagePos(mnCurPageId)];
        else
            pOldItem = nullptr;

        // deselect previous page if page was not selected, if this is the
        // only selected page
        if (!pItem->mbSelect && pOldItem)
        {
            sal_uInt16 nSelPageCount = GetSelectPageCount();
            if (nSelPageCount == 1)
                pOldItem->mbSelect = false;
            pItem->mbSelect = true;
        }

        mnCurPageId = nPageId;
        mbFormat = true;

        // assure the actual page becomes visible
        if (IsReallyVisible())
        {
            if (nPos < mnFirstPos)
                SetFirstPageId(nPageId);
            else
            {
                // calculate visible width
                long nWidth = mnLastOffX;
                if (nWidth > ADDNEWPAGE_AREAWIDTH)
                    nWidth -= ADDNEWPAGE_AREAWIDTH;

                if (pItem->maRect.IsEmpty())
                    ImplFormat();

                while ((mbMirrored ? (pItem->maRect.Left() < mnOffX) : (pItem->maRect.Right() > nWidth)) ||
                        pItem->maRect.IsEmpty())
                {
                    sal_uInt16 nNewPos = mnFirstPos + 1;
                    // assure at least the actual tabpages are visible as first tabpage
                    if (nNewPos >= nPos)
                    {
                        SetFirstPageId(nPageId);
                        break;
                    }
                    else
                        SetFirstPageId(GetPageId(nNewPos));
                    ImplFormat();
                    // abort if first page is not forwarded
                    if (nNewPos != mnFirstPos)
                        break;
                }
            }
        }

        // redraw bar
        if (bUpdate)
        {
            Invalidate(pItem->maRect);
            if (pOldItem)
                Invalidate(pOldItem->maRect);
        }
    }
}

void TabBar::MakeVisible(sal_uInt16 nPageId)
{
    if (!IsReallyVisible())
        return;

    sal_uInt16 nPos = GetPagePos(nPageId);

    // do nothing if item does not exist
    if (nPos != PAGE_NOT_FOUND)
    {
        if (nPos < mnFirstPos)
            SetFirstPageId(nPageId);
        else
        {
            ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];

            // calculate visible area
            long nWidth = mnLastOffX;

            if (mbFormat || pItem->maRect.IsEmpty())
            {
                mbFormat = true;
                ImplFormat();
            }

            while ((pItem->maRect.Right() > nWidth) ||
                    pItem->maRect.IsEmpty())
            {
                sal_uInt16 nNewPos = mnFirstPos+1;
                // assure at least the actual tabpages are visible as first tabpage
                if (nNewPos >= nPos)
                {
                    SetFirstPageId(nPageId);
                    break;
                }
                else
                    SetFirstPageId(GetPageId(nNewPos));
                ImplFormat();
                // abort if first page is not forwarded
                if (nNewPos != mnFirstPos)
                    break;
            }
        }
    }
}

void TabBar::SetFirstPageId(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    // return false if item does not exist
    if (nPos != PAGE_NOT_FOUND)
    {
        if (nPos != mnFirstPos)
        {
            // assure as much pages are visible as possible
            ImplFormat();
            sal_uInt16 nLastFirstPos = ImplGetLastFirstPos();
            sal_uInt16 nNewPos;
            if (nPos > nLastFirstPos)
                nNewPos = nLastFirstPos;
            else
                nNewPos = nPos;

            if (nNewPos != mnFirstPos)
            {
                mnFirstPos = nNewPos;
                mbFormat = true;

                // redraw bar (attention: check mbDropPos,
                // as if this flag was set, we do not re-paint immediately
                if (IsReallyVisible() && IsUpdateMode() && !mbDropPos)
                    Invalidate();
            }
        }
    }
}

void TabBar::SelectPage(sal_uInt16 nPageId, bool bSelect)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
    {
        ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];

        if (pItem->mbSelect != bSelect)
        {
            pItem->mbSelect = bSelect;

            // redraw bar
            if (IsReallyVisible() && IsUpdateMode())
                Invalidate(pItem->maRect);
        }
    }
}

sal_uInt16 TabBar::GetSelectPageCount() const
{
    sal_uInt16 nSelected = 0;
    for (ImplTabBarItem* pItem : mpImpl->mpItemList)
    {
        if (pItem->mbSelect)
            nSelected++;
    }

    return nSelected;
}

bool TabBar::IsPageSelected(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->mpItemList[nPos]->mbSelect;
    else
        return false;
}

bool TabBar::StartEditMode(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    if (mpImpl->mpEdit || (nPos == PAGE_NOT_FOUND) || (mnLastOffX < 8))
        return false;

    mnEditId = nPageId;
    if (StartRenaming())
    {
        ImplShowPage(nPos);
        ImplFormat();
        Update();

        mpImpl->mpEdit.disposeAndReset(VclPtr<TabBarEdit>::Create(this, WB_CENTER));
        Rectangle aRect = GetPageRect( mnEditId );
        long nX = aRect.Left();
        long nWidth = aRect.GetWidth();
        if (mnEditId != GetCurPageId())
            nX += 1;
        if (nX + nWidth > mnLastOffX)
            nWidth = mnLastOffX-nX;
        if (nWidth < 3)
        {
            nX = aRect.Left();
            nWidth = aRect.GetWidth();
        }
        mpImpl->mpEdit->SetText(GetPageText(mnEditId));
        mpImpl->mpEdit->setPosSizePixel(nX, aRect.Top() + mnOffY + 1, nWidth, aRect.GetHeight() - 3);
        vcl::Font aFont = GetPointFont(*this); // FIXME RenderContext

        Color   aForegroundColor;
        Color   aBackgroundColor;
        Color   aFaceColor;
        Color   aSelectColor;
        Color   aFaceTextColor;
        Color   aSelectTextColor;

        ImplGetColors(Application::GetSettings().GetStyleSettings(), aFaceColor, aFaceTextColor, aSelectColor, aSelectTextColor);

        if (mnEditId != GetCurPageId())
        {
            aFont.SetWeight(WEIGHT_LIGHT);
        }
        if (IsPageSelected(mnEditId) || mnEditId == GetCurPageId())
        {
            aForegroundColor = aSelectTextColor;
            aBackgroundColor = aSelectColor;
        }
        else
        {
            aForegroundColor = aFaceTextColor;
            aBackgroundColor = aFaceColor;
        }
        if (GetPageBits( mnEditId ) & TPB_SPECIAL)
        {
            aForegroundColor = Color(COL_LIGHTBLUE);
        }
        mpImpl->mpEdit->SetControlFont(aFont);
        mpImpl->mpEdit->SetControlForeground(aForegroundColor);
        mpImpl->mpEdit->SetControlBackground(aBackgroundColor);
        mpImpl->mpEdit->GrabFocus();
        mpImpl->mpEdit->SetSelection(Selection(0, mpImpl->mpEdit->GetText().getLength()));
        mpImpl->mpEdit->Show();
        return true;
    }
    else
    {
        mnEditId = 0;
        return false;
    }
}

bool TabBar::IsInEditMode() const
{
    return mpImpl->mpEdit.get() != nullptr;
}

void TabBar::EndEditMode(bool bCancel)
{
    if (mpImpl->mpEdit)
    {
        // call hdl
        bool bEnd = true;
        mbEditCanceled = bCancel;
        maEditText = mpImpl->mpEdit->GetText();
        mpImpl->mpEdit->SetPostEvent();
        if (!bCancel)
        {
            TabBarAllowRenamingReturnCode nAllowRenaming = AllowRenaming();
            if (nAllowRenaming == TABBAR_RENAMING_YES)
                SetPageText(mnEditId, maEditText);
            else if (nAllowRenaming == TABBAR_RENAMING_NO)
                bEnd = false;
            else // nAllowRenaming == TABBAR_RENAMING_CANCEL
                mbEditCanceled = true;
        }

        // renaming not allowed, than reset edit data
        if (!bEnd)
        {
            mpImpl->mpEdit->ResetPostEvent();
            mpImpl->mpEdit->GrabFocus();
        }
        else
        {
            // close edit and call end hdl
            mpImpl->mpEdit.disposeAndClear();

            EndRenaming();
            mnEditId = 0;
        }

        // reset
        maEditText.clear();
        mbEditCanceled = false;
    }
}

void TabBar::SetMirrored(bool bMirrored)
{
    if (mbMirrored != bMirrored)
    {
        mbMirrored = bMirrored;
        mbSizeFormat = true;
        ImplInitControls();     // for button images
        Resize();               // recalculates control positions
        Mirror();
    }
}

void TabBar::SetEffectiveRTL(bool bRTL)
{
    SetMirrored( bRTL != AllSettings::GetLayoutRTL() );
}

bool TabBar::IsEffectiveRTL() const
{
    return IsMirrored() != AllSettings::GetLayoutRTL();
}

void TabBar::SetMaxPageWidth(long nMaxWidth)
{
    if (mnMaxPageWidth != nMaxWidth)
    {
        mnMaxPageWidth = nMaxWidth;
        mbSizeFormat = true;

        // redraw bar
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
    }
}

void TabBar::SetPageText(sal_uInt16 nPageId, const OUString& rText)
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
    {
        mpImpl->mpItemList[nPos]->maText = rText;
        mbSizeFormat = true;

        // redraw bar
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();

        CallEventListeners(VCLEVENT_TABBAR_PAGETEXTCHANGED, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)));
    }
}

OUString TabBar::GetPageText(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->mpItemList[nPos]->maText;
    return OUString();
}

OUString TabBar::GetHelpText(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
    {
        ImplTabBarItem* pItem = mpImpl->mpItemList[nPos];
        if (pItem->maHelpText.isEmpty() && !pItem->maHelpId.isEmpty())
        {
            Help* pHelp = Application::GetHelp();
            if (pHelp)
                pItem->maHelpText = pHelp->GetHelpText(OStringToOUString(pItem->maHelpId, RTL_TEXTENCODING_UTF8), this);
        }

        return pItem->maHelpText;
    }
    return OUString();
}

OString TabBar::GetHelpId(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    OString aRet;
    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->mpItemList[nPos]->maHelpId;
    return aRet;
}

bool TabBar::StartDrag(const CommandEvent& rCEvt, vcl::Region& rRegion)
{
    if (!(mnWinStyle & WB_DRAG) || (rCEvt.GetCommand() != CommandEventId::StartDrag))
        return false;

    // Check if the clicked page was selected. If this is not the case
    // set it as actual entry. We check for this only at a mouse action
    // if Drag and Drop can be triggered from the keyboard.
    // We only do this, if Select() was not triggered, as the Select()
    // could have scrolled the area
    if (rCEvt.IsMouseEvent() && !mbInSelect)
    {
        sal_uInt16 nSelId = GetPageId(rCEvt.GetMousePosPixel());

        // do not start dragging if no entry was clicked
        if (!nSelId)
            return false;

        // check if page was selected. If not set it as actual
        // page and call Select()
        if (!IsPageSelected(nSelId))
        {
            if (ImplDeactivatePage())
            {
                SetCurPageId(nSelId);
                Update();
                ImplActivatePage();
                ImplSelect();
            }
            else
                return false;
        }
    }
    mbInSelect = false;

    vcl::Region aRegion;

    // assign region
    rRegion = aRegion;

    return true;
}

sal_uInt16 TabBar::ShowDropPos(const Point& rPos)
{
    ImplTabBarItem* pItem;
    sal_uInt16 nDropId;
    sal_uInt16 nNewDropPos;
    sal_uInt16 nItemCount = mpImpl->getItemSize();
    sal_Int16 nScroll = 0;

    if (rPos.X() > mnLastOffX-TABBAR_DRAG_SCROLLOFF)
    {
        pItem = mpImpl->mpItemList[mpImpl->mpItemList.size() - 1];
        if (!pItem->maRect.IsEmpty() && (rPos.X() > pItem->maRect.Right()))
            nNewDropPos = mpImpl->getItemSize();
        else
        {
            nNewDropPos = mnFirstPos + 1;
            nScroll = 1;
        }
    }
    else if ((rPos.X() <= mnOffX) ||
             (!mnOffX && (rPos.X() <= TABBAR_DRAG_SCROLLOFF)))
    {
        if (mnFirstPos)
        {
            nNewDropPos = mnFirstPos;
            nScroll = -1;
        }
        else
            nNewDropPos = 0;
    }
    else
    {
        nDropId = GetPageId(rPos);
        if (nDropId)
        {
            nNewDropPos = GetPagePos(nDropId);
            if (mnFirstPos && (nNewDropPos == mnFirstPos - 1))
                nScroll = -1;
        }
        else
            nNewDropPos = nItemCount;
    }

    if (mbDropPos && (nNewDropPos == mnDropPos) && !nScroll)
        return mnDropPos;

    if (mbDropPos)
        HideDropPos();
    mbDropPos = true;
    mnDropPos = nNewDropPos;

    if (nScroll)
    {
        sal_uInt16 nOldFirstPos = mnFirstPos;
        SetFirstPageId(GetPageId(mnFirstPos + nScroll));

        // draw immediately, as Paint not possible during Drag and Drop
        if (nOldFirstPos != mnFirstPos)
        {
            Rectangle aRect(mnOffX, 0, mnLastOffX, maWinSize.Height());
            SetFillColor(GetBackground().GetColor());
            DrawRect(aRect);
            Invalidate(aRect);
        }
    }

    // draw drop position arrows
    Color aBlackColor(COL_BLACK);
    long nX;
    long nY = (maWinSize.Height() / 2) - 1;
    sal_uInt16 nCurPos = GetPagePos(mnCurPageId);

    sal_Int32 nTriangleWidth = 3 * GetDPIScaleFactor();

    if (mnDropPos < nItemCount)
    {
        SetLineColor(aBlackColor);
        SetFillColor(aBlackColor);

        pItem = mpImpl->mpItemList[mnDropPos];
        nX = pItem->maRect.Left();
        if ( mnDropPos == nCurPos )
            nX--;
        else
            nX++;

        if (!pItem->IsDefaultTabBgColor() && !pItem->mbSelect)
        {
            SetLineColor(pItem->maTabTextColor);
            SetFillColor(pItem->maTabTextColor);
        }

        tools::Polygon aPoly(3);
        aPoly.SetPoint(Point(nX, nY), 0);
        aPoly.SetPoint(Point(nX + nTriangleWidth, nY - nTriangleWidth), 1);
        aPoly.SetPoint(Point(nX + nTriangleWidth, nY + nTriangleWidth), 2);
        DrawPolygon(aPoly);
    }
    if (mnDropPos > 0 && mnDropPos < nItemCount + 1)
    {
        SetLineColor(aBlackColor);
        SetFillColor(aBlackColor);

        pItem = mpImpl->mpItemList[mnDropPos - 1];
        nX = pItem->maRect.Right();
        if (mnDropPos == nCurPos)
            nX++;
        if (!pItem->IsDefaultTabBgColor() && !pItem->mbSelect)
        {
            SetLineColor(pItem->maTabTextColor);
            SetFillColor(pItem->maTabTextColor);
        }
        tools::Polygon aPoly(3);
        aPoly.SetPoint(Point(nX, nY), 0);
        aPoly.SetPoint(Point(nX - nTriangleWidth, nY - nTriangleWidth), 1);
        aPoly.SetPoint(Point(nX - nTriangleWidth, nY + nTriangleWidth), 2);
        DrawPolygon(aPoly);
    }

    return mnDropPos;
}

void TabBar::HideDropPos()
{
    if (mbDropPos)
    {
        ImplTabBarItem* pItem;
        long nX;
        long nY1 = (maWinSize.Height() / 2) - 3;
        long nY2 = nY1 + 5;
        sal_uInt16 nItemCount = mpImpl->getItemSize();

        if (mnDropPos < nItemCount)
        {
            pItem = mpImpl->mpItemList[mnDropPos];
            nX = pItem->maRect.Left();
            // immediately call Paint, as it is not possible during drag and drop
            Rectangle aRect( nX-1, nY1, nX+3, nY2 );
            vcl::Region aRegion( aRect );
            SetClipRegion( aRegion );
            Invalidate(aRect);
            SetClipRegion();
        }
        if (mnDropPos > 0 && mnDropPos < nItemCount + 1)
        {
            pItem = mpImpl->mpItemList[mnDropPos - 1];
            nX = pItem->maRect.Right();
            // immediately call Paint, as it is not possible during drag and drop
            Rectangle aRect(nX - 2, nY1, nX + 1, nY2);
            vcl::Region aRegion(aRect);
            SetClipRegion(aRegion);
            Invalidate(aRect);
            SetClipRegion();
        }

        mbDropPos = false;
        mnDropPos = 0;
    }
}

void TabBar::SwitchPage(const Point& rPos)
{
    sal_uInt16 nSwitchId = GetPageId(rPos);
    if (!nSwitchId)
        EndSwitchPage();
    else
    {
        if (nSwitchId != mnSwitchId)
        {
            mnSwitchId = nSwitchId;
            mnSwitchTime = tools::Time::GetSystemTicks();
        }
        else
        {
            // change only after 500 ms
            if (mnSwitchId != GetCurPageId())
            {
                if (tools::Time::GetSystemTicks() > mnSwitchTime + 500)
                {
                    mbInSwitching = true;
                    if (ImplDeactivatePage())
                    {
                        SetCurPageId( mnSwitchId );
                        Update();
                        ImplActivatePage();
                        ImplSelect();
                    }
                    mbInSwitching = false;
                }
            }
        }
    }
}

void TabBar::EndSwitchPage()
{
    mnSwitchTime = 0;
    mnSwitchId = 0;
}

void TabBar::SetStyle(WinBits nStyle)
{
    mnWinStyle = nStyle;
    ImplInitControls();
    // order possible controls
    if (IsReallyVisible() && IsUpdateMode())
        Resize();
}

Size TabBar::CalcWindowSizePixel() const
{
    long nWidth = 0;

    if (mpImpl->mpItemList.size() > 0)
    {
        const_cast<TabBar*>(this)->ImplCalcWidth();
        for (ImplTabBarItem* pItem : mpImpl->mpItemList)
        {
            nWidth += pItem->mnWidth;
        }
    }

    return Size(nWidth, GetSettings().GetStyleSettings().GetScrollBarSize());
}

Rectangle TabBar::GetPageArea() const
{
    return Rectangle(Point(mnOffX, mnOffY),
                     Size(mnLastOffX - mnOffX + 1, GetSizePixel().Height() - mnOffY));
}

css::uno::Reference<css::accessibility::XAccessible> TabBar::CreateAccessible()
{
    return mpImpl->maAccessibleFactory.getFactory().createAccessibleTabBar(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
