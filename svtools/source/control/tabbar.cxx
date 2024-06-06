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
#include <tools/poly.hxx>
#include <utility>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svtaccessiblefactory.hxx>
#include <vcl/accessiblefactory.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/weldutils.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <limits>
#include <memory>
#include <vector>
#include <vcl/idle.hxx>
#include <bitmaps.hlst>

namespace
{

constexpr sal_uInt16 TABBAR_DRAG_SCROLLOFF = 5;
constexpr sal_uInt16 TABBAR_MINSIZE = 5;

constexpr sal_uInt16 ADDNEWPAGE_AREAWIDTH = 10;

class TabDrawer
{
private:
    vcl::RenderContext& mrRenderContext;
    const StyleSettings& mrStyleSettings;

    tools::Rectangle maRect;
    tools::Rectangle maLineRect;

    Color maSelectedColor;
    Color maCustomColor;

public:
    bool mbSelected:1;
    bool mbCustomColored:1;
    bool mbEnabled:1;
    bool mbProtect:1;

    explicit TabDrawer(vcl::RenderContext& rRenderContext)
        : mrRenderContext(rRenderContext)
        , mrStyleSettings(rRenderContext.GetSettings().GetStyleSettings())
        , mbSelected(false)
        , mbCustomColored(false)
        , mbEnabled(false)
        , mbProtect(false)
    {

    }

    void drawOuterFrame()
    {
        // set correct FillInBrush depending on status
        if (mbSelected)
        {
            // Currently selected Tab
            mrRenderContext.SetFillColor(maSelectedColor);
            mrRenderContext.SetLineColor(maSelectedColor);
            mrRenderContext.DrawRect(maRect);
            mrRenderContext.SetLineColor(mrStyleSettings.GetDarkShadowColor());
        }
        else if (mbCustomColored)
        {
            mrRenderContext.SetFillColor(maCustomColor);
            mrRenderContext.SetLineColor(maCustomColor);
            mrRenderContext.DrawRect(maRect);
            mrRenderContext.SetLineColor(mrStyleSettings.GetDarkShadowColor());
        }
    }

    void drawText(const OUString& aText)
    {
        tools::Rectangle aRect = maRect;
        tools::Long nTextWidth = mrRenderContext.GetTextWidth(aText);
        tools::Long nTextHeight = mrRenderContext.GetTextHeight();
        Point aPos = aRect.TopLeft();
        aPos.AdjustX((aRect.getOpenWidth()  - nTextWidth) / 2 );
        aPos.AdjustY((aRect.getOpenHeight() - nTextHeight) / 2 );

        if (mbEnabled)
            mrRenderContext.DrawText(aPos, aText);
        else
            mrRenderContext.DrawCtrlText(aPos, aText, 0, aText.getLength(), (DrawTextFlags::Disable | DrawTextFlags::Mnemonic));
    }

    void drawOverTopBorder()
    {
        Point aTopLeft  = maRect.TopLeft()  + Point(1, 0);
        Point aTopRight = maRect.TopRight() + Point(-1, 0);

        tools::Rectangle aDelRect(aTopLeft, aTopRight);
        mrRenderContext.DrawRect(aDelRect);
    }

    void drawColorLine()
    {
        if (!mbSelected)
            return;

        // tdf#141396: the color must be different from the rest of the selected tab
        Color aLineColor = (mbCustomColored && maCustomColor != maSelectedColor)
                               ? maCustomColor
                               : mrStyleSettings.GetDarkShadowColor();
        mrRenderContext.SetFillColor(aLineColor);
        mrRenderContext.SetLineColor(aLineColor);
        mrRenderContext.DrawRect(maLineRect);
    }

    void drawSeparator()
    {
        const tools::Long cMargin = 5;
        const tools::Long aRight( maRect.Right() - 1 );
        mrRenderContext.SetLineColor(mrStyleSettings.GetShadowColor());
        mrRenderContext.DrawLine(Point(aRight, maRect.Top() + cMargin),
                                 Point(aRight, maRect.Bottom() - cMargin));
    }

    void drawTab()
    {
        drawOuterFrame();
        drawColorLine();
        if (!mbSelected && !mbCustomColored)
            drawSeparator();
        if (mbProtect)
        {
            BitmapEx aBitmap(BMP_TAB_LOCK);
            Point aPosition = maRect.TopLeft();
            aPosition.AdjustX(2);
            aPosition.AdjustY((maRect.getOpenHeight() - aBitmap.GetSizePixel().Height()) / 2);
            mrRenderContext.DrawBitmapEx(aPosition, aBitmap);
        }
    }

    void setRect(const tools::Rectangle& rRect)
    {
        maLineRect = tools::Rectangle(rRect.BottomLeft(), rRect.BottomRight());
        maLineRect.AdjustTop(-2);
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

    void setEnabled(bool bEnabled)
    {
        mbEnabled = bEnabled;
    }

    void setSelectedFillColor(const Color& rColor)
    {
        maSelectedColor = rColor;
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
    OUString maAuxiliaryText; // used in LayerTabBar for real layer name
    tools::Rectangle maRect;
    tools::Long mnWidth;
    OString maHelpId;
    bool mbShort : 1;
    bool mbSelect : 1;
    bool mbProtect : 1;
    Color maTabBgColor;
    Color maTabTextColor;

    ImplTabBarItem(sal_uInt16 nItemId, OUString aText, TabBarPageBits nPageBits)
        : mnId(nItemId)
        , mnBits(nPageBits)
        , maText(std::move(aText))
        , mnWidth(0)
        , mbShort(false)
        , mbSelect(false)
        , mbProtect(false)
        , maTabBgColor(COL_AUTO)
        , maTabTextColor(COL_AUTO)
    {
    }

    bool IsDefaultTabBgColor() const
    {
        return maTabBgColor == COL_AUTO;
    }

    bool IsSelected(ImplTabBarItem const * pCurItem) const
    {
        return mbSelect || (pCurItem == this);
    }

    OUString const & GetRenderText() const
    {
        return maText;
    }
};

class ImplTabSizer : public vcl::Window
{
public:
                    ImplTabSizer( TabBar* pParent, WinBits nWinStyle );

    TabBar*         GetParent() const { return static_cast<TabBar*>(Window::GetParent()); }

private:
    void            ImplTrack( const Point& rScreenPos );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect ) override;

    Point           maStartPos;
    tools::Long            mnStartWidth;
};

ImplTabSizer::ImplTabSizer( TabBar* pParent, WinBits nWinStyle )
    : Window( pParent, nWinStyle & WB_3DLOOK )
    , mnStartWidth(0)
{
    SetPointer(PointerStyle::HSizeBar);
    SetSizePixel(Size(7 * GetDPIScaleFactor(), 0));
}

void ImplTabSizer::ImplTrack( const Point& rScreenPos )
{
    TabBar* pParent = GetParent();
    tools::Long nDiff = rScreenPos.X() - maStartPos.X();
    pParent->mnSplitSize = mnStartWidth + (pParent->IsMirrored() ? -nDiff : nDiff);
    if ( pParent->mnSplitSize < TABBAR_MINSIZE )
        pParent->mnSplitSize = TABBAR_MINSIZE;
    pParent->Split();
    pParent->PaintImmediately();
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

void ImplTabSizer::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    DecorationView aDecoView(&rRenderContext);
    tools::Rectangle aOutputRect(Point(0, 0), GetOutputSizePixel());
    aDecoView.DrawHandle(aOutputRect);
}

namespace {

// Is not named Impl. as it may be both instantiated and derived from
class TabBarEdit final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::Entry> m_xEntry;
    Idle            maLoseFocusIdle;
    bool            mbPostEvt;

    DECL_LINK( ImplEndEditHdl, void*, void );
    DECL_LINK( ImplEndTimerHdl, Timer*, void );
    DECL_LINK( ActivatedHdl, weld::Entry&, bool );
    DECL_LINK( KeyInputHdl, const KeyEvent&, bool );
    DECL_LINK( FocusOutHdl, weld::Widget&, void );

public:
    TabBarEdit(TabBar* pParent);
    virtual void dispose() override;

    TabBar*         GetParent() const { return static_cast<TabBar*>(Window::GetParent()); }

    weld::Entry&    get_widget() { return *m_xEntry; }

    void            SetPostEvent() { mbPostEvt = true; }
    void            ResetPostEvent() { mbPostEvt = false; }
};

}

TabBarEdit::TabBarEdit(TabBar* pParent)
    : InterimItemWindow(pParent, u"svt/ui/tabbaredit.ui"_ustr, u"TabBarEdit"_ustr)
    , m_xEntry(m_xBuilder->weld_entry(u"entry"_ustr))
    , maLoseFocusIdle( "svtools::TabBarEdit maLoseFocusIdle" )
{
    InitControlBase(m_xEntry.get());

    mbPostEvt = false;
    maLoseFocusIdle.SetPriority( TaskPriority::REPAINT );
    maLoseFocusIdle.SetInvokeHandler( LINK( this, TabBarEdit, ImplEndTimerHdl ) );

    m_xEntry->connect_activate(LINK(this, TabBarEdit, ActivatedHdl));
    m_xEntry->connect_key_press(LINK(this, TabBarEdit, KeyInputHdl));
    m_xEntry->connect_focus_out(LINK(this, TabBarEdit, FocusOutHdl));
}

void TabBarEdit::dispose()
{
    m_xEntry.reset();
    InterimItemWindow::dispose();
}

IMPL_LINK_NOARG(TabBarEdit, ActivatedHdl, weld::Entry&, bool)
{
    if ( !mbPostEvt )
    {
        if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), reinterpret_cast<void*>(false), true ) )
            mbPostEvt = true;
    }
    return true;
}

IMPL_LINK(TabBarEdit, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    if (!rKEvt.GetKeyCode().GetModifier() && rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        if ( !mbPostEvt )
        {
            if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), reinterpret_cast<void*>(true), true ) )
                mbPostEvt = true;
        }
        return true;
    }
    return false;
}

IMPL_LINK_NOARG(TabBarEdit, FocusOutHdl, weld::Widget&, void)
{
    if ( !mbPostEvt )
    {
        if ( PostUserEvent( LINK( this, TabBarEdit, ImplEndEditHdl ), reinterpret_cast<void*>(false), true ) )
            mbPostEvt = true;
    }
}

IMPL_LINK( TabBarEdit, ImplEndEditHdl, void*, pCancel, void )
{
    ResetPostEvent();
    maLoseFocusIdle.Stop();

    // tdf#156958: when renaming and clicking on canvas, LO goes into GetParent()->EndEditMode first time
    // then it calls TabBarEdit::dispose method which resets m_xEntry BUT, on the same thread, LO comes here again
    // so return if already disposed to avoid a crash
    if (isDisposed())
        return;

    // We need this query, because the edit gets a losefocus event,
    // when it shows the context menu or the insert symbol dialog
    if (!m_xEntry->has_focus() && m_xEntry->has_child_focus())
        maLoseFocusIdle.Start();
    else
        GetParent()->EndEditMode( pCancel != nullptr );
}

IMPL_LINK_NOARG(TabBarEdit, ImplEndTimerHdl, Timer *, void)
{
    if (m_xEntry->has_focus())
        return;

    // We need this query, because the edit gets a losefocus event,
    // when it shows the context menu or the insert symbol dialog
    if (m_xEntry->has_child_focus())
        maLoseFocusIdle.Start();
    else
        GetParent()->EndEditMode( true );
}

namespace {

class TabButtons final : public InterimItemWindow
{
public:
    std::unique_ptr<weld::Button> m_xFirstButton;
    std::unique_ptr<weld::Button> m_xPrevButton;
    std::unique_ptr<weld::Button> m_xNextButton;
    std::unique_ptr<weld::Button> m_xLastButton;
    std::unique_ptr<weld::Button> m_xAddButton;
    std::shared_ptr<weld::ButtonPressRepeater> m_xAddRepeater;
    std::shared_ptr<weld::ButtonPressRepeater> m_xPrevRepeater;
    std::shared_ptr<weld::ButtonPressRepeater> m_xNextRepeater;

    TabButtons(TabBar* pParent, bool bSheets)
        : InterimItemWindow(pParent,
                            pParent->IsMirrored() ? u"svt/ui/tabbuttonsmirrored.ui"_ustr
                                                  : u"svt/ui/tabbuttons.ui"_ustr,
                            u"TabButtons"_ustr)
        , m_xFirstButton(m_xBuilder->weld_button(u"first"_ustr))
        , m_xPrevButton(m_xBuilder->weld_button(u"prev"_ustr))
        , m_xNextButton(m_xBuilder->weld_button(u"next"_ustr))
        , m_xLastButton(m_xBuilder->weld_button(u"last"_ustr))
        , m_xAddButton(m_xBuilder->weld_button(u"add"_ustr))
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        SetPaintTransparent(false);
        SetBackground(rStyleSettings.GetFaceColor());

        m_xFirstButton->set_accessible_name(SvtResId(STR_TABBAR_PUSHBUTTON_MOVET0HOME));
        m_xPrevButton->set_accessible_name(SvtResId(STR_TABBAR_PUSHBUTTON_MOVELEFT));
        m_xNextButton->set_accessible_name(SvtResId(STR_TABBAR_PUSHBUTTON_MOVERIGHT));
        m_xLastButton->set_accessible_name(SvtResId(STR_TABBAR_PUSHBUTTON_MOVETOEND));
        m_xAddButton->set_accessible_name(SvtResId(STR_TABBAR_PUSHBUTTON_ADDTAB));

        if (bSheets)
        {
            m_xFirstButton->set_tooltip_text(SvtResId(STR_TABBAR_HINT_MOVETOHOME_SHEETS));
            m_xPrevButton->set_tooltip_text(SvtResId(STR_TABBAR_HINT_MOVELEFT_SHEETS));
            m_xNextButton->set_tooltip_text(SvtResId(STR_TABBAR_HINT_MOVERIGHT_SHEETS));
            m_xLastButton->set_tooltip_text(SvtResId(STR_TABBAR_HINT_MOVETOEND_SHEETS));
            m_xAddButton->set_tooltip_text(SvtResId(STR_TABBAR_HINT_ADDTAB_SHEETS));
        }
    }

    void AdaptToHeight(int nHeight)
    {
        if (m_xFirstButton->get_preferred_size() == Size(nHeight, nHeight))
            return;
        m_xFirstButton->set_size_request(nHeight, nHeight);
        m_xPrevButton->set_size_request(nHeight, nHeight);
        m_xNextButton->set_size_request(nHeight, nHeight);
        m_xLastButton->set_size_request(nHeight, nHeight);
        m_xAddButton->set_size_request(nHeight, nHeight);
        InvalidateChildSizeCache();
    }

    virtual void dispose() override
    {
        m_xNextRepeater.reset();
        m_xPrevRepeater.reset();
        m_xAddRepeater.reset();
        m_xAddButton.reset();
        m_xLastButton.reset();
        m_xNextButton.reset();
        m_xPrevButton.reset();
        m_xFirstButton.reset();
        InterimItemWindow::dispose();
    }
};

}

struct TabBar_Impl
{
    ScopedVclPtr<ImplTabSizer>  mpSizer;
    ScopedVclPtr<TabButtons>    mxButtonBox;
    ScopedVclPtr<TabBarEdit>    mxEdit;
    std::vector<ImplTabBarItem> maItemList;

    vcl::AccessibleFactoryAccess  maAccessibleFactory;

    sal_uInt16 getItemSize() const
    {
        return static_cast<sal_uInt16>(maItemList.size());
    }
};

TabBar::TabBar( vcl::Window* pParent, WinBits nWinStyle, bool bSheets ) :
    Window( pParent, (nWinStyle & WB_3DLOOK) | WB_CLIPCHILDREN )
{
    ImplInit( nWinStyle, bSheets );
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

void TabBar::ImplInit( WinBits nWinStyle, bool bSheets )
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
    mbAutoEditMode  = false;
    mbEditCanceled  = false;
    mbDropPos       = false;
    mbInSelect      = false;
    mbMirrored      = false;
    mbScrollAlwaysEnabled = false;
    mbSheets = bSheets;

    ImplInitControls();

    SetSizePixel( Size( 100, CalcWindowSizePixel().Height() ) );
    ImplInitSettings( true, true );
}

ImplTabBarItem* TabBar::seek( size_t i )
{
    if ( i < mpImpl->maItemList.size() )
    {
        maCurrentItemList = i;
        return &mpImpl->maItemList[maCurrentItemList];
    }
    return nullptr;
}

ImplTabBarItem* TabBar::prev()
{
    if ( maCurrentItemList > 0 )
    {
        return &mpImpl->maItemList[--maCurrentItemList];
    }
    return nullptr;
}

ImplTabBarItem* TabBar::next()
{
    if ( maCurrentItemList + 1 < mpImpl->maItemList.size() )
    {
        return &mpImpl->maItemList[++maCurrentItemList];
    }
    return nullptr;
}

void TabBar::ImplInitSettings( bool bFont, bool bBackground )
{
    // FIXME RenderContext

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if (bFont)
    {
        vcl::Font aToolFont = rStyleSettings.GetToolFont();
        aToolFont.SetWeight( WEIGHT_BOLD );
        ApplyControlFont(*GetOutDev(), aToolFont);

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
        ApplyControlBackground(*GetOutDev(), rStyleSettings.GetFaceColor());
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
    for (auto& rItem : mpImpl->maItemList)
    {
        tools::Long nNewWidth = GetTextWidth(rItem.GetRenderText());
        if (mnCurMaxWidth && (nNewWidth > mnCurMaxWidth))
        {
            rItem.mbShort = true;
            nNewWidth = mnCurMaxWidth;
        }
        else
        {
            rItem.mbShort = false;
        }

        // Padding is dependent on font height - bigger font = bigger padding
        tools::Long nFontWidth = aFont.GetFontHeight();
        if (rItem.mbProtect)
            nNewWidth += 24;
        nNewWidth += nFontWidth * 2;

        if (rItem.mnWidth != nNewWidth)
        {
            rItem.mnWidth = nNewWidth;
            if (!rItem.maRect.IsEmpty())
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

    sal_uInt16 nItemIndex = 0;
    tools::Long x = mnOffX;
    for (auto & rItem : mpImpl->maItemList)
    {
        // At all non-visible tabs an empty rectangle is set
        if ((nItemIndex + 1 < mnFirstPos) || (x > mnLastOffX))
            rItem.maRect.SetEmpty();
        else
        {
            // Slightly before the tab before the first visible page
            // should also be visible
            if (nItemIndex + 1 == mnFirstPos)
            {
                rItem.maRect.SetLeft(x - rItem.mnWidth);
            }
            else
            {
                rItem.maRect.SetLeft(x);
                x += rItem.mnWidth;
            }
            rItem.maRect.SetRight(x);
            rItem.maRect.SetBottom(maWinSize.Height() - 1);

            if (mbMirrored)
            {
                tools::Long nNewLeft  = mnOffX + mnLastOffX - rItem.maRect.Right();
                tools::Long nNewRight = mnOffX + mnLastOffX - rItem.maRect.Left();
                rItem.maRect.SetRight(nNewRight);
                rItem.maRect.SetLeft(nNewLeft);
            }
        }

        nItemIndex++;
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
    tools::Long nWinWidth = mnLastOffX - mnOffX - ADDNEWPAGE_AREAWIDTH;
    tools::Long nWidth = mpImpl->maItemList[nLastFirstPos].mnWidth;

    while (nLastFirstPos && (nWidth < nWinWidth))
    {
        nLastFirstPos--;
        nWidth += mpImpl->maItemList[nLastFirstPos].mnWidth;
    }
    if ((nLastFirstPos != static_cast<sal_uInt16>(mpImpl->maItemList.size() - 1)) && (nWidth > nWinWidth))
        nLastFirstPos++;
    return nLastFirstPos;
}

IMPL_LINK(TabBar, ContextMenuHdl, const CommandEvent&, rCommandEvent, void)
{
    maScrollAreaContextHdl.Call(rCommandEvent);
}

IMPL_LINK(TabBar, MousePressHdl, const MouseEvent&, rMouseEvent, bool)
{
    if (rMouseEvent.IsRight())
        ContextMenuHdl(CommandEvent(rMouseEvent.GetPosPixel(), CommandEventId::ContextMenu, true));
    return false;
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

    mpImpl->mxButtonBox.disposeAndReset(VclPtr<TabButtons>::Create(this, mbSheets));

    Link<const CommandEvent&, void> aContextLink = LINK( this, TabBar, ContextMenuHdl );

    if (mnWinStyle & WB_INSERTTAB)
    {
        Link<weld::Button&,void> aLink = LINK(this, TabBar, ImplAddClickHandler);
        mpImpl->mxButtonBox->m_xAddRepeater = std::make_shared<weld::ButtonPressRepeater>(
                    *mpImpl->mxButtonBox->m_xAddButton, aLink, aContextLink);
        mpImpl->mxButtonBox->m_xAddButton->show();
    }

    Link<weld::Button&,void> aLink = LINK( this, TabBar, ImplClickHdl );

    if (mnWinStyle & (WB_MINSCROLL | WB_SCROLL))
    {
        mpImpl->mxButtonBox->m_xPrevRepeater = std::make_shared<weld::ButtonPressRepeater>(
                    *mpImpl->mxButtonBox->m_xPrevButton, aLink, aContextLink);
        mpImpl->mxButtonBox->m_xPrevButton->show();
        mpImpl->mxButtonBox->m_xNextRepeater = std::make_shared<weld::ButtonPressRepeater>(
                    *mpImpl->mxButtonBox->m_xNextButton, aLink, aContextLink);
        mpImpl->mxButtonBox->m_xNextButton->show();
    }

    if (mnWinStyle & WB_SCROLL)
    {
        Link<const MouseEvent&, bool> aBtnContextLink = LINK(this, TabBar, MousePressHdl);

        mpImpl->mxButtonBox->m_xFirstButton->connect_clicked(aLink);
        mpImpl->mxButtonBox->m_xFirstButton->connect_mouse_press(aBtnContextLink);
        mpImpl->mxButtonBox->m_xFirstButton->show();
        mpImpl->mxButtonBox->m_xLastButton->connect_clicked(aLink);
        mpImpl->mxButtonBox->m_xLastButton->connect_mouse_press(aBtnContextLink);
        mpImpl->mxButtonBox->m_xLastButton->show();
    }

    mpImpl->mxButtonBox->Show();
}

void TabBar::ImplEnableControls()
{
    if (mbSizeFormat || mbFormat)
        return;

    // enable/disable buttons
    bool bEnableBtn = mbScrollAlwaysEnabled || mnFirstPos > 0;
    mpImpl->mxButtonBox->m_xFirstButton->set_sensitive(bEnableBtn);
    mpImpl->mxButtonBox->m_xPrevButton->set_sensitive(bEnableBtn);
    if (!bEnableBtn && mpImpl->mxButtonBox->m_xPrevRepeater)
        mpImpl->mxButtonBox->m_xPrevRepeater->Stop();
    bEnableBtn = mbScrollAlwaysEnabled || mnFirstPos < ImplGetLastFirstPos();
    mpImpl->mxButtonBox->m_xLastButton->set_sensitive(bEnableBtn);
    mpImpl->mxButtonBox->m_xNextButton->set_sensitive(bEnableBtn);
    if (!bEnableBtn && mpImpl->mxButtonBox->m_xNextRepeater)
        mpImpl->mxButtonBox->m_xNextRepeater->Stop();
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
    tools::Long nWidth = GetOutputSizePixel().Width();

    auto& rItem = mpImpl->maItemList[nPos];
    if (nPos < mnFirstPos)
        SetFirstPageId( rItem.mnId );
    else if (rItem.maRect.Right() > nWidth)
    {
        while (rItem.maRect.Right() > nWidth)
        {
            sal_uInt16 nNewPos = mnFirstPos + 1;
            SetFirstPageId(GetPageId(nNewPos));
            ImplFormat();
            if (nNewPos != mnFirstPos)
                break;
        }
    }
}

IMPL_LINK( TabBar, ImplClickHdl, weld::Button&, rBtn, void )
{
    if (&rBtn != mpImpl->mxButtonBox->m_xFirstButton.get() && &rBtn != mpImpl->mxButtonBox->m_xLastButton.get())
    {
        if ((GetPointerState().mnState & (MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)) == 0)
        {
            // like tdf#149482 if we didn't see a mouse up, but find that the mouse is no
            // longer pressed at this point, then bail
            mpImpl->mxButtonBox->m_xPrevRepeater->Stop();
            mpImpl->mxButtonBox->m_xNextRepeater->Stop();
            return;
        }
    }

    EndEditMode();

    sal_uInt16 nNewPos = mnFirstPos;

    if (&rBtn == mpImpl->mxButtonBox->m_xFirstButton.get() || (&rBtn == mpImpl->mxButtonBox->m_xPrevButton.get() &&
                                                               mpImpl->mxButtonBox->m_xPrevRepeater->IsModKeyPressed()))
    {
        nNewPos = 0;
    }
    else if (&rBtn == mpImpl->mxButtonBox->m_xLastButton.get() || (&rBtn == mpImpl->mxButtonBox->m_xNextButton.get() &&
                                                                   mpImpl->mxButtonBox->m_xNextRepeater->IsModKeyPressed()))
    {
        sal_uInt16 nCount = GetPageCount();
        if (nCount)
            nNewPos = nCount - 1;
    }
    else if (&rBtn == mpImpl->mxButtonBox->m_xPrevButton.get())
    {
        if (mnFirstPos)
            nNewPos = mnFirstPos - 1;
    }
    else if (&rBtn == mpImpl->mxButtonBox->m_xNextButton.get())
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

IMPL_LINK_NOARG(TabBar, ImplAddClickHandler, weld::Button&, void)
{
    if ((GetPointerState().mnState & (MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)) == 0)
    {
        // tdf#149482 if we didn't see a mouse up, but find that the mouse is no
        // longer pressed at this point, then bail
        mpImpl->mxButtonBox->m_xAddRepeater->Stop();
        return;
    }

    EndEditMode();
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
    // Only terminate EditMode and do not execute click
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
                PaintImmediately();
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
                        auto& rItem = mpImpl->maItemList[n];
                        bSelect = n >= nPos;

                        if (rItem.mbSelect != bSelect)
                        {
                            rItem.mbSelect = bSelect;
                            if (!rItem.maRect.IsEmpty())
                                Invalidate(rItem.maRect);
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
                        auto& rItem = mpImpl->maItemList[n];

                        bSelect = n <= nPos;

                        if (rItem.mbSelect != bSelect)
                        {
                            rItem.mbSelect = bSelect;
                            if (!rItem.maRect.IsEmpty())
                                Invalidate(rItem.maRect);
                        }

                        n++;
                    }
                }
            }

            // scroll the selected tab if required
            if (bSelectTab)
            {
                ImplShowPage(nPos);
                PaintImmediately();
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
                auto& rItem = mpImpl->maItemList[nPos];

                if (!rItem.mbSelect)
                {
                    // make not valid
                    bool bUpdate = false;
                    if (IsReallyVisible() && IsUpdateMode())
                        bUpdate = true;

                    // deselect all selected items
                    for (auto& xItem : mpImpl->maItemList)
                    {
                        if (xItem.mbSelect || (xItem.mnId == mnCurPageId))
                        {
                            xItem.mbSelect = false;
                            if (bUpdate)
                                Invalidate(xItem.maRect);
                        }
                    }
                }

                if (ImplDeactivatePage())
                {
                    SetCurPageId(nSelId);
                    PaintImmediately();
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

void TabBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rect)
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

    ImplPrePaint();

    Color aFaceColor, aSelectColor, aFaceTextColor, aSelectTextColor;
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    ImplGetColors(rStyleSettings, aFaceColor, aFaceTextColor, aSelectColor, aSelectTextColor);

    rRenderContext.Push(vcl::PushFlags::FONT | vcl::PushFlags::CLIPREGION);
    rRenderContext.SetClipRegion(vcl::Region(GetPageArea()));

    // select font
    vcl::Font aFont = rRenderContext.GetFont();
    vcl::Font aLightFont = aFont;
    aLightFont.SetWeight(WEIGHT_NORMAL);

    TabDrawer aDrawer(rRenderContext);

    aDrawer.setSelectedFillColor(aSelectColor);

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
            tools::Rectangle aRect = pItem->maRect;
            bool bSelected = pItem->IsSelected(pCurItem);
            // We disable custom background color in high contrast mode.
            bool bCustomBgColor = !pItem->IsDefaultTabBgColor() && !rStyleSettings.GetHighContrastMode();
            OUString aText = pItem->mbShort ?
                rRenderContext.GetEllipsisString(pItem->GetRenderText(), mnCurMaxWidth) :
                pItem->GetRenderText();

            aDrawer.setRect(aRect);
            aDrawer.setSelected(bSelected);
            aDrawer.setCustomColored(bCustomBgColor);
            aDrawer.setEnabled(true);
            aDrawer.setCustomColor(pItem->maTabBgColor);
            aDrawer.mbProtect = pItem->mbProtect;
            aDrawer.drawTab();

            // currently visible sheet is drawn using a bold font
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

            // Special display of tab name depending on page bits

            if (pItem->mnBits & TabBarPageBits::Blue)
            {
                rRenderContext.SetTextColor(COL_LIGHTBLUE);
            }
            if (pItem->mnBits & TabBarPageBits::Italic)
            {
                vcl::Font aSpecialFont = rRenderContext.GetFont();
                aSpecialFont.SetItalic(FontItalic::ITALIC_NORMAL);
                rRenderContext.SetFont(aSpecialFont);
            }
            if (pItem->mnBits & TabBarPageBits::Underline)
            {
                vcl::Font aSpecialFont = rRenderContext.GetFont();
                aSpecialFont.SetUnderline(LINESTYLE_SINGLE);
                rRenderContext.SetFont(aSpecialFont);
            }

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

    tools::Long nSizerWidth = 0;

    // order the Sizer
    if ( mpImpl->mpSizer )
    {
        Size    aSizerSize = mpImpl->mpSizer->GetSizePixel();
        Point   aNewSizerPos( mbMirrored ? 0 : (aNewSize.Width()-aSizerSize.Width()), 0 );
        Size    aNewSizerSize( aSizerSize.Width(), aNewSize.Height() );
        mpImpl->mpSizer->SetPosSizePixel( aNewSizerPos, aNewSizerSize );
        nSizerWidth = aSizerSize.Width();
    }

    // order the scroll buttons
    tools::Long const nHeight = aNewSize.Height();
    // adapt font height?
    ImplInitSettings( true, false );

    mpImpl->mxButtonBox->AdaptToHeight(nHeight);
    Size const aBtnsSize(mpImpl->mxButtonBox->get_preferred_size().Width(), nHeight);
    Point aPos(mbMirrored ? (aNewSize.Width() - aBtnsSize.Width()) : 0, 0);
    mpImpl->mxButtonBox->SetPosSizePixel(aPos, aBtnsSize);
    auto nButtonWidth = aBtnsSize.Width();

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
    if (rNEvt.GetType() == NotifyEventType::COMMAND)
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
                tools::Rectangle aItemRect = GetPageRect(nItemId);
                Point aPt = OutputToScreenPixel(aItemRect.TopLeft());
                aItemRect.SetLeft( aPt.X() );
                aItemRect.SetTop( aPt.Y() );
                aPt = OutputToScreenPixel(aItemRect.BottomRight());
                aItemRect.SetRight( aPt.X() );
                aItemRect.SetBottom( aPt.Y() );
                Help::ShowBalloon(this, aItemRect.Center(), aItemRect, aStr);
                return;
            }
        }

        // show text for quick- or balloon-help
        // if this is isolated or not fully visible
        if (rHEvt.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON))
        {
            sal_uInt16 nPos = GetPagePos(nItemId);
            auto& rItem = mpImpl->maItemList[nPos];
            if (rItem.mbShort || (rItem.maRect.Right() - 5 > mnLastOffX))
            {
                tools::Rectangle aItemRect = GetPageRect(nItemId);
                Point aPt = OutputToScreenPixel(aItemRect.TopLeft());
                aItemRect.SetLeft( aPt.X() );
                aItemRect.SetTop( aPt.Y() );
                aPt = OutputToScreenPixel(aItemRect.BottomRight());
                aItemRect.SetRight( aPt.X() );
                aItemRect.SetBottom( aPt.Y() );
                OUString aStr = mpImpl->maItemList[nPos].maText;
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
        if ( (mbSizeFormat || mbFormat) && !mpImpl->maItemList.empty() )
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
        bool bIsRTLEnabled = IsRTLEnabled();
        // reacts on calls of EnableRTL, have to mirror all child controls
        if (mpImpl->mpSizer)
            mpImpl->mpSizer->EnableRTL(bIsRTLEnabled);
        if (mpImpl->mxButtonBox)
        {
            mpImpl->mxButtonBox->m_xFirstButton->set_direction(bIsRTLEnabled);
            mpImpl->mxButtonBox->m_xPrevButton->set_direction(bIsRTLEnabled);
            mpImpl->mxButtonBox->m_xNextButton->set_direction(bIsRTLEnabled);
            mpImpl->mxButtonBox->m_xLastButton->set_direction(bIsRTLEnabled);
            mpImpl->mxButtonBox->m_xAddButton->set_direction(bIsRTLEnabled);
        }
        if (mpImpl->mxEdit)
        {
            weld::Entry& rEntry = mpImpl->mxEdit->get_widget();
            rEntry.set_direction(bIsRTLEnabled);
        }
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
    CallEventListeners(VclEventId::TabbarPageSelected, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)));
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

    CallEventListeners(VclEventId::TabbarPageActivated, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)));
}

void TabBar::ActivatePage()
{}

bool TabBar::ImplDeactivatePage()
{
    bool bRet = DeactivatePage();

    CallEventListeners(VclEventId::TabbarPageDeactivated, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(mnCurPageId)));

    return bRet;
}

void TabBar::ImplPrePaint()
{
    sal_uInt16 nItemCount = mpImpl->getItemSize();
    if (!nItemCount)
        return;

    // tabbar should be formatted
    ImplFormat();

    // assure the actual tabpage becomes visible at first format
    if (!mbFirstFormat)
        return;

    mbFirstFormat = false;

    if (!mnCurPageId || (mnFirstPos != 0) || mbDropPos)
        return;

    auto& rItem = mpImpl->maItemList[GetPagePos(mnCurPageId)];
    if (rItem.maRect.IsEmpty())
    {
        // set mbDropPos (or misuse) to prevent Invalidate()
        mbDropPos = true;
        SetFirstPageId(mnCurPageId);
        mbDropPos = false;
        if (mnFirstPos != 0)
            ImplFormat();
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
    assert (nPageId && "TabBar::InsertPage(): PageId must not be 0");
    assert ((GetPagePos(nPageId) == PAGE_NOT_FOUND) && "TabBar::InsertPage(): Page already exists");
    assert ((nBits <= TPB_DISPLAY_NAME_ALLFLAGS) && "TabBar::InsertPage(): Invalid flag set in nBits");

    // create PageItem and insert in the item list
    ImplTabBarItem aItem( nPageId, rText, nBits );
    if (nPos < mpImpl->maItemList.size())
    {
        auto it = mpImpl->maItemList.begin();
        it += nPos;
        mpImpl->maItemList.insert(it, aItem);
    }
    else
    {
        mpImpl->maItemList.push_back(aItem);
    }
    mbSizeFormat = true;

    // set CurPageId if required
    if (!mnCurPageId)
        mnCurPageId = nPageId;

    // redraw bar
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    CallEventListeners(VclEventId::TabbarPageInserted, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)));
}

Color TabBar::GetTabBgColor(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->maItemList[nPos].maTabBgColor;
    else
        return COL_AUTO;
}

void TabBar::SetTabBgColor(sal_uInt16 nPageId, const Color& aTabBgColor)
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos == PAGE_NOT_FOUND)
        return;

    auto& rItem = mpImpl->maItemList[nPos];
    if (aTabBgColor != COL_AUTO)
    {
        rItem.maTabBgColor = aTabBgColor;
        if (aTabBgColor.GetLuminance() <= 128) //Do not use aTabBgColor.IsDark(), because that threshold is way too low...
            rItem.maTabTextColor = COL_WHITE;
        else
            rItem.maTabTextColor = COL_BLACK;
    }
    else
    {
        rItem.maTabBgColor = COL_AUTO;
        rItem.maTabTextColor = COL_AUTO;
    }
}

void TabBar::RemovePage(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    // does item exist
    if (nPos == PAGE_NOT_FOUND)
        return;

    if (mnCurPageId == nPageId)
        mnCurPageId = 0;

    // check if first visible page should be moved
    if (mnFirstPos > nPos)
        mnFirstPos--;

    // delete item data
    auto it = mpImpl->maItemList.begin();
    it += nPos;
    mpImpl->maItemList.erase(it);

    // redraw bar
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    CallEventListeners(VclEventId::TabbarPageRemoved, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)));
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
    if (nPos == PAGE_NOT_FOUND)
        return;

    // move tabbar item in the list
    auto it = mpImpl->maItemList.begin();
    it += nPos;
    ImplTabBarItem aItem = std::move(*it);
    mpImpl->maItemList.erase(it);
    if (nNewPos < mpImpl->maItemList.size())
    {
        it = mpImpl->maItemList.begin();
        it += nNewPos;
        mpImpl->maItemList.insert(it, aItem);
    }
    else
    {
        mpImpl->maItemList.push_back(aItem);
    }

    // redraw bar
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    CallEventListeners( VclEventId::TabbarPageMoved, static_cast<void*>(&aPair) );
}

void TabBar::Clear()
{
    // delete all items
    mpImpl->maItemList.clear();

    // remove items from the list
    mbSizeFormat = true;
    mnCurPageId = 0;
    mnFirstPos = 0;
    maCurrentItemList = 0;

    // redraw bar
    if (IsReallyVisible() && IsUpdateMode())
        Invalidate();

    CallEventListeners(VclEventId::TabbarPageRemoved, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(PAGE_NOT_FOUND)));
}

bool TabBar::IsPageEnabled(sal_uInt16 nPageId) const
{
    if (isDisposed())
        return false;
    sal_uInt16 nPos = GetPagePos(nPageId);

    return nPos != PAGE_NOT_FOUND;
}

void TabBar::SetPageBits(sal_uInt16 nPageId, TabBarPageBits nBits)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos == PAGE_NOT_FOUND)
        return;

    auto& rItem = mpImpl->maItemList[nPos];

    if (rItem.mnBits != nBits)
    {
        rItem.mnBits = nBits;

        // redraw bar
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate(rItem.maRect);
    }
}

TabBarPageBits TabBar::GetPageBits(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->maItemList[nPos].mnBits;
    else
        return TabBarPageBits::NONE;
}

sal_uInt16 TabBar::GetPageCount() const
{
    return mpImpl->getItemSize();
}

sal_uInt16 TabBar::GetPageId(sal_uInt16 nPos) const
{
    return nPos < mpImpl->maItemList.size() ? mpImpl->maItemList[nPos].mnId : 0;
}

sal_uInt16 TabBar::GetPagePos(sal_uInt16 nPageId) const
{
    for (size_t i = 0; i < mpImpl->maItemList.size(); ++i)
    {
        if (mpImpl->maItemList[i].mnId == nPageId)
        {
            return static_cast<sal_uInt16>(i);
        }
    }
    return PAGE_NOT_FOUND;
}

sal_uInt16 TabBar::GetPageId(const Point& rPos) const
{
    for (const auto& rItem : mpImpl->maItemList)
    {
        if (rItem.maRect.Contains(rPos))
            return rItem.mnId;
    }

    return 0;
}

tools::Rectangle TabBar::GetPageRect(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->maItemList[nPos].maRect;
    else
        return tools::Rectangle();
}

void TabBar::SetCurPageId(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    // do nothing if item does not exit
    if (nPos == PAGE_NOT_FOUND)
        return;

    // do nothing if the actual page did not change
    if (nPageId == mnCurPageId)
        return;

    // make invalid
    bool bUpdate = false;
    if (IsReallyVisible() && IsUpdateMode())
        bUpdate = true;

    auto& rItem = mpImpl->maItemList[nPos];
    ImplTabBarItem* pOldItem;

    if (mnCurPageId)
        pOldItem = &mpImpl->maItemList[GetPagePos(mnCurPageId)];
    else
        pOldItem = nullptr;

    // deselect previous page if page was not selected, if this is the
    // only selected page
    if (!rItem.mbSelect && pOldItem)
    {
        sal_uInt16 nSelPageCount = GetSelectPageCount();
        if (nSelPageCount == 1)
            pOldItem->mbSelect = false;
        rItem.mbSelect = true;
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
            tools::Long nWidth = mnLastOffX;
            if (nWidth > ADDNEWPAGE_AREAWIDTH)
                nWidth -= ADDNEWPAGE_AREAWIDTH;

            if (rItem.maRect.IsEmpty())
                ImplFormat();

            while ((mbMirrored ? (rItem.maRect.Left() < mnOffX) : (rItem.maRect.Right() > nWidth)) ||
                    rItem.maRect.IsEmpty())
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
        Invalidate(rItem.maRect);
        if (pOldItem)
            Invalidate(pOldItem->maRect);
    }
}

void TabBar::MakeVisible(sal_uInt16 nPageId)
{
    if (!IsReallyVisible())
        return;

    sal_uInt16 nPos = GetPagePos(nPageId);

    // do nothing if item does not exist
    if (nPos == PAGE_NOT_FOUND)
        return;

    if (nPos < mnFirstPos)
        SetFirstPageId(nPageId);
    else
    {
        auto& rItem = mpImpl->maItemList[nPos];

        // calculate visible area
        tools::Long nWidth = mnLastOffX;

        if (mbFormat || rItem.maRect.IsEmpty())
        {
            mbFormat = true;
            ImplFormat();
        }

        while ((rItem.maRect.Right() > nWidth) ||
                rItem.maRect.IsEmpty())
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

void TabBar::SetFirstPageId(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    // return false if item does not exist
    if (nPos == PAGE_NOT_FOUND)
        return;

    if (nPos == mnFirstPos)
        return;

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

void TabBar::SelectPage(sal_uInt16 nPageId, bool bSelect)
{
    sal_uInt16 nPos = GetPagePos(nPageId);

    if (nPos == PAGE_NOT_FOUND)
        return;

    auto& rItem = mpImpl->maItemList[nPos];

    if (rItem.mbSelect != bSelect)
    {
        rItem.mbSelect = bSelect;

        // redraw bar
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate(rItem.maRect);
    }
}

sal_uInt16 TabBar::GetSelectPageCount() const
{
    sal_uInt16 nSelected = 0;
    for (const auto& rItem : mpImpl->maItemList)
    {
        if (rItem.mbSelect)
            nSelected++;
    }

    return nSelected;
}

bool TabBar::IsPageSelected(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->maItemList[nPos].mbSelect;
    else
        return false;
}

void TabBar::SetProtectionSymbol(sal_uInt16 nPageId, bool bProtection)
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
    {
        if (mpImpl->maItemList[nPos].mbProtect != bProtection)
        {
            mpImpl->maItemList[nPos].mbProtect = bProtection;
            mbSizeFormat = true;    // render text width changes, thus bar width

            // redraw bar
            if (IsReallyVisible() && IsUpdateMode())
                Invalidate();
        }
    }
}

bool TabBar::StartEditMode(sal_uInt16 nPageId)
{
    sal_uInt16 nPos = GetPagePos( nPageId );
    if (mpImpl->mxEdit || (nPos == PAGE_NOT_FOUND) || (mnLastOffX < 8))
        return false;

    mnEditId = nPageId;
    if (StartRenaming())
    {
        ImplShowPage(nPos);
        ImplFormat();
        PaintImmediately();

        mpImpl->mxEdit.disposeAndReset(VclPtr<TabBarEdit>::Create(this));
        tools::Rectangle aRect = GetPageRect( mnEditId );
        tools::Long nX = aRect.Left();
        tools::Long nWidth = aRect.GetWidth();
        if (mnEditId != GetCurPageId())
            nX += 1;
        if (nX + nWidth > mnLastOffX)
            nWidth = mnLastOffX-nX;
        if (nWidth < 3)
        {
            nX = aRect.Left();
            nWidth = aRect.GetWidth();
        }
        weld::Entry& rEntry = mpImpl->mxEdit->get_widget();
        rEntry.set_text(GetPageText(mnEditId));
        mpImpl->mxEdit->SetPosSizePixel(Point(nX, aRect.Top() + mnOffY + 1), Size(nWidth, aRect.GetHeight() - 3));
        vcl::Font aFont = GetPointFont(*GetOutDev()); // FIXME RenderContext

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
        if (GetPageBits( mnEditId ) & TabBarPageBits::Blue)
        {
            aForegroundColor = COL_LIGHTBLUE;
        }
        rEntry.set_font(aFont);
        rEntry.set_font_color(aForegroundColor);
        mpImpl->mxEdit->SetControlBackground(aBackgroundColor);
        rEntry.grab_focus();
        rEntry.select_region(0, -1);
        mpImpl->mxEdit->Show();
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
    return bool(mpImpl->mxEdit);
}

void TabBar::EndEditMode(bool bCancel)
{
    if (!mpImpl->mxEdit)
        return;

    // call hdl
    bool bEnd = true;
    mbEditCanceled = bCancel;
    weld::Entry& rEntry = mpImpl->mxEdit->get_widget();
    maEditText = rEntry.get_text();
    mpImpl->mxEdit->SetPostEvent();
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

    // renaming not allowed, then reset edit data
    if (!bEnd)
    {
        mpImpl->mxEdit->ResetPostEvent();
        rEntry.grab_focus();
    }
    else
    {
        // close edit and call end hdl
        mpImpl->mxEdit.disposeAndClear();

        EndRenaming();
        mnEditId = 0;
    }

    // reset
    maEditText.clear();
    mbEditCanceled = false;
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

void TabBar::SetMaxPageWidth(tools::Long nMaxWidth)
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
        mpImpl->maItemList[nPos].maText = rText;
        mbSizeFormat = true;

        // redraw bar
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();

        CallEventListeners(VclEventId::TabbarPageTextChanged, reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(nPageId)));
    }
}

OUString TabBar::GetPageText(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->maItemList[nPos].maText;
    return OUString();
}

OUString TabBar::GetAuxiliaryText(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
        return mpImpl->maItemList[nPos].maAuxiliaryText;
    return OUString();
}

void TabBar::SetAuxiliaryText(sal_uInt16 nPageId, const OUString& rText )
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
    {
        mpImpl->maItemList[nPos].maAuxiliaryText = rText;
        // no redraw bar, no CallEventListener, internal use in LayerTabBar
    }
}

OUString TabBar::GetHelpText(sal_uInt16 nPageId) const
{
    sal_uInt16 nPos = GetPagePos(nPageId);
    if (nPos != PAGE_NOT_FOUND)
    {
        auto& rItem = mpImpl->maItemList[nPos];
        if (rItem.maHelpText.isEmpty() && !rItem.maHelpId.isEmpty())
        {
            Help* pHelp = Application::GetHelp();
            if (pHelp)
                rItem.maHelpText = pHelp->GetHelpText(OStringToOUString(rItem.maHelpId, RTL_TEXTENCODING_UTF8));
        }

        return rItem.maHelpText;
    }
    return OUString();
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
                PaintImmediately();
                ImplActivatePage();
                ImplSelect();
            }
            else
                return false;
        }
    }
    mbInSelect = false;

    // assign region
    rRegion = vcl::Region();

    return true;
}

sal_uInt16 TabBar::ShowDropPos(const Point& rPos)
{
    sal_uInt16 nNewDropPos;
    sal_uInt16 nItemCount = mpImpl->getItemSize();
    sal_Int16 nScroll = 0;

    if (rPos.X() > mnLastOffX-TABBAR_DRAG_SCROLLOFF)
    {
        auto& rItem = mpImpl->maItemList[mpImpl->maItemList.size() - 1];
        if (!rItem.maRect.IsEmpty() && (rPos.X() > rItem.maRect.Right()))
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
        sal_uInt16 nDropId = GetPageId(rPos);
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
            tools::Rectangle aRect(mnOffX, 0, mnLastOffX, maWinSize.Height());
            GetOutDev()->SetFillColor(GetBackground().GetColor());
            GetOutDev()->DrawRect(aRect);
            Invalidate(aRect);
        }
    }

    // draw drop position arrows
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    const Color aTextColor = rStyles.GetLabelTextColor();
    tools::Long nX;
    tools::Long nY = (maWinSize.Height() / 2) - 1;
    sal_uInt16 nCurPos = GetPagePos(mnCurPageId);

    sal_Int32 nTriangleWidth = 3 * GetDPIScaleFactor();

    if (mnDropPos < nItemCount)
    {
        GetOutDev()->SetLineColor(aTextColor);
        GetOutDev()->SetFillColor(aTextColor);

        auto& rItem = mpImpl->maItemList[mnDropPos];
        nX = rItem.maRect.Left();
        if ( mnDropPos == nCurPos )
            nX--;
        else
            nX++;

        if (!rItem.IsDefaultTabBgColor() && !rItem.mbSelect)
        {
            GetOutDev()->SetLineColor(rItem.maTabTextColor);
            GetOutDev()->SetFillColor(rItem.maTabTextColor);
        }

        tools::Polygon aPoly(3);
        aPoly.SetPoint(Point(nX, nY), 0);
        aPoly.SetPoint(Point(nX + nTriangleWidth, nY - nTriangleWidth), 1);
        aPoly.SetPoint(Point(nX + nTriangleWidth, nY + nTriangleWidth), 2);
        GetOutDev()->DrawPolygon(aPoly);
    }
    if (mnDropPos > 0 && mnDropPos < nItemCount + 1)
    {
        GetOutDev()->SetLineColor(aTextColor);
        GetOutDev()->SetFillColor(aTextColor);

        auto& rItem = mpImpl->maItemList[mnDropPos - 1];
        nX = rItem.maRect.Right();
        if (mnDropPos == nCurPos)
            nX++;
        if (!rItem.IsDefaultTabBgColor() && !rItem.mbSelect)
        {
            GetOutDev()->SetLineColor(rItem.maTabTextColor);
            GetOutDev()->SetFillColor(rItem.maTabTextColor);
        }
        tools::Polygon aPoly(3);
        aPoly.SetPoint(Point(nX, nY), 0);
        aPoly.SetPoint(Point(nX - nTriangleWidth, nY - nTriangleWidth), 1);
        aPoly.SetPoint(Point(nX - nTriangleWidth, nY + nTriangleWidth), 2);
        GetOutDev()->DrawPolygon(aPoly);
    }

    return mnDropPos;
}

void TabBar::HideDropPos()
{
    if (!mbDropPos)
        return;

    tools::Long nX;
    tools::Long nY1 = (maWinSize.Height() / 2) - 3;
    tools::Long nY2 = nY1 + 5;
    sal_uInt16 nItemCount = mpImpl->getItemSize();

    if (mnDropPos < nItemCount)
    {
        auto& rItem = mpImpl->maItemList[mnDropPos];
        nX = rItem.maRect.Left();
        // immediately call Paint, as it is not possible during drag and drop
        tools::Rectangle aRect( nX-1, nY1, nX+3, nY2 );
        vcl::Region aRegion( aRect );
        GetOutDev()->SetClipRegion( aRegion );
        Invalidate(aRect);
        GetOutDev()->SetClipRegion();
    }
    if (mnDropPos > 0 && mnDropPos < nItemCount + 1)
    {
        auto& rItem = mpImpl->maItemList[mnDropPos - 1];
        nX = rItem.maRect.Right();
        // immediately call Paint, as it is not possible during drag and drop
        tools::Rectangle aRect(nX - 2, nY1, nX + 1, nY2);
        vcl::Region aRegion(aRect);
        GetOutDev()->SetClipRegion(aRegion);
        Invalidate(aRect);
        GetOutDev()->SetClipRegion();
    }

    mbDropPos = false;
    mnDropPos = 0;
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
                    if (ImplDeactivatePage())
                    {
                        SetCurPageId( mnSwitchId );
                        PaintImmediately();
                        ImplActivatePage();
                        ImplSelect();
                    }
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
    if (mnWinStyle == nStyle)
        return;
    mnWinStyle = nStyle;
    ImplInitControls();
    // order possible controls
    if (IsReallyVisible() && IsUpdateMode())
        Resize();
}

Size TabBar::CalcWindowSizePixel() const
{
    tools::Long nWidth = 0;

    if (!mpImpl->maItemList.empty())
    {
        const_cast<TabBar*>(this)->ImplCalcWidth();
        for (const auto& rItem : mpImpl->maItemList)
        {
            nWidth += rItem.mnWidth;
        }
    }

    return Size(nWidth, GetSettings().GetStyleSettings().GetScrollBarSize());
}

tools::Rectangle TabBar::GetPageArea() const
{
    return tools::Rectangle(Point(mnOffX, mnOffY),
                     Size(mnLastOffX - mnOffX + 1, GetSizePixel().Height() - mnOffY));
}

void TabBar::SetAddButtonEnabled(bool bAddButtonEnabled)
{
    mpImpl->mxButtonBox->m_xAddButton->set_sensitive(bAddButtonEnabled);
}

css::uno::Reference<css::accessibility::XAccessible> TabBar::CreateAccessible()
{
    return mpImpl->maAccessibleFactory.getFactory().createAccessibleTabBar(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
