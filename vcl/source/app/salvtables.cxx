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

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <o3tl/safeint.hxx>
#include <o3tl/sorted_vector.hxx>
#include <officecfg/Office/Common.hxx>
#include <salframe.hxx>
#include <salinst.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <saltimer.hxx>
#include <salsession.hxx>
#include <salsys.hxx>
#include <salbmp.hxx>
#include <salobj.hxx>
#include <salmenu.hxx>
#include <strings.hrc>
#include <svdata.hxx>
#include <svimpbox.hxx>
#include <messagedialog.hxx>
#include <treeglue.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/configmgr.hxx>
#include <utility>
#include <tools/helpers.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/builder.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/fixedhyper.hxx>
#include <vcl/toolkit/fmtfield.hxx>
#include <vcl/headbar.hxx>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/layout.hxx>
#include <vcl/toolkit/menubtn.hxx>
#include <vcl/toolkit/prgsbar.hxx>
#include <vcl/ptrstyle.hxx>
#include <slider.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/svtabbx.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/throbber.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/toolkit/vclmedit.hxx>
#include <vcl/toolkit/viewdataentry.hxx>
#include <vcl/virdev.hxx>
#include <bitmaps.hlst>
#include <calendar.hxx>
#include <menutogglebutton.hxx>
#include <verticaltabctrl.hxx>
#include <window.h>
#include <wizdlg.hxx>
#include <salvtables.hxx>
#include <comphelper/lok.hxx>

SalFrame::SalFrame()
    : m_pWindow(nullptr)
    , m_pProc(nullptr)
{
}

// this file contains the virtual destructors of the sal interface
// compilers usually put their vtables where the destructor is

SalFrame::~SalFrame() {}

void SalFrame::SetCallback(vcl::Window* pWindow, SALFRAMEPROC pProc)
{
    m_pWindow = pWindow;
    m_pProc = pProc;
}

// default to full-frame flushes
// on ports where partial-flushes are much cheaper this method should be overridden
void SalFrame::Flush(const tools::Rectangle&) { Flush(); }

void SalFrame::SetRepresentedURL(const OUString&)
{
    // currently this is Mac only functionality
}

SalInstance::SalInstance(std::unique_ptr<comphelper::SolarMutex> pMutex)
    : m_pYieldMutex(std::move(pMutex))
{
}

SalInstance::~SalInstance() {}

comphelper::SolarMutex* SalInstance::GetYieldMutex() { return m_pYieldMutex.get(); }

sal_uInt32 SalInstance::ReleaseYieldMutexAll() { return m_pYieldMutex->release(true); }

void SalInstance::AcquireYieldMutex(sal_uInt32 nCount) { m_pYieldMutex->acquire(nCount); }

std::unique_ptr<SalSession> SalInstance::CreateSalSession() { return nullptr; }

OpenGLContext* SalInstance::CreateOpenGLContext()
{
    assert(!m_bSupportsOpenGL);
    std::abort();
}

std::unique_ptr<SalMenu> SalInstance::CreateMenu(bool, Menu*)
{
    // default: no native menus
    return nullptr;
}

std::unique_ptr<SalMenuItem> SalInstance::CreateMenuItem(const SalItemParams&) { return nullptr; }

bool SalInstance::CallEventCallback(void const* pEvent, int nBytes)
{
    return m_pEventInst.is() && m_pEventInst->dispatchEvent(pEvent, nBytes);
}

SalTimer::~SalTimer() COVERITY_NOEXCEPT_FALSE {}

void SalBitmap::DropScaledCache()
{
    if (ImplSVData* pSVData = ImplGetSVData())
    {
        auto& rCache = pSVData->maGDIData.maScaleCache;

        rCache.remove_if([this](const lru_scale_cache::key_value_pair_t& rKeyValuePair) {
            return rKeyValuePair.first.mpBitmap == this;
        });
    }
}

SalBitmap::~SalBitmap() { DropScaledCache(); }

SalSystem::~SalSystem() {}

SalPrinter::~SalPrinter() {}

bool SalPrinter::StartJob(const OUString*, const OUString&, const OUString&, ImplJobSetup*,
                          vcl::PrinterController&)
{
    return false;
}

SalInfoPrinter::~SalInfoPrinter() {}

SalVirtualDevice::~SalVirtualDevice() {}

SalObject::~SalObject() {}

SalMenu::~SalMenu() {}

bool SalMenu::ShowNativePopupMenu(FloatingWindow*, const tools::Rectangle&, FloatWinPopupFlags)
{
    return false;
}

void SalMenu::ShowCloseButton(bool) {}

bool SalMenu::AddMenuBarButton(const SalMenuButtonItem&) { return false; }

void SalMenu::RemoveMenuBarButton(sal_uInt16) {}

tools::Rectangle SalMenu::GetMenuBarButtonRectPixel(sal_uInt16, SalFrame*)
{
    return tools::Rectangle();
}

int SalMenu::GetMenuBarHeight() const { return 0; }

void SalMenu::ApplyPersona() {}

SalMenuItem::~SalMenuItem() {}

void SalInstanceWidget::ensure_event_listener()
{
    if (!m_bEventListener)
    {
        m_xWidget->AddEventListener(LINK(this, SalInstanceWidget, EventListener));
        m_bEventListener = true;
    }
}

// we want the ability to mark key events as handled, so use this variant
// for those, we get all keystrokes in this case, so we will need to filter
// them later
void SalInstanceWidget::ensure_key_listener()
{
    if (!m_bKeyEventListener)
    {
        Application::AddKeyListener(LINK(this, SalInstanceWidget, KeyEventListener));
        m_bKeyEventListener = true;
    }
}

// we want the ability to know about mouse events that happen in our children
// so use this variant, we will need to filter them later
void SalInstanceWidget::ensure_mouse_listener()
{
    if (!m_bMouseEventListener)
    {
        m_xWidget->AddChildEventListener(LINK(this, SalInstanceWidget, MouseEventListener));
        m_bMouseEventListener = true;
    }
}

void SalInstanceWidget::set_background(const Color& rColor)
{
    m_xWidget->SetControlBackground(rColor);
    m_xWidget->SetBackground(m_xWidget->GetControlBackground());
    if (m_xWidget->GetStyle() & WB_CLIPCHILDREN)
    {
        // turn off WB_CLIPCHILDREN otherwise the bg won't extend "under"
        // transparent children of the widget e.g. expander in sidebar panel header
        m_xWidget->SetStyle(m_xWidget->GetStyle() & ~WB_CLIPCHILDREN);
        // and toggle mbClipChildren on instead otherwise the bg won't fill e.g.
        // deck titlebar header when its width is stretched
        WindowImpl* pImpl = m_xWidget->ImplGetWindowImpl();
        pImpl->mbClipChildren = true;
    }
}

SalInstanceWidget::SalInstanceWidget(vcl::Window* pWidget, SalInstanceBuilder* pBuilder,
                                     bool bTakeOwnership)
    : m_xWidget(pWidget)
    , m_pBuilder(pBuilder)
    , m_bTakeOwnership(bTakeOwnership)
    , m_bEventListener(false)
    , m_bKeyEventListener(false)
    , m_bMouseEventListener(false)
    , m_nBlockNotify(0)
    , m_nFreezeCount(0)
{
}

void SalInstanceWidget::set_sensitive(bool sensitive) { m_xWidget->Enable(sensitive); }

bool SalInstanceWidget::get_sensitive() const { return m_xWidget->IsEnabled(); }

bool SalInstanceWidget::get_visible() const { return m_xWidget->IsVisible(); }

bool SalInstanceWidget::is_visible() const { return m_xWidget->IsReallyVisible(); }

void SalInstanceWidget::set_can_focus(bool bCanFocus)
{
    auto nStyle = m_xWidget->GetStyle() & ~(WB_TABSTOP | WB_NOTABSTOP);
    if (bCanFocus)
        nStyle |= WB_TABSTOP;
    else
        nStyle |= WB_NOTABSTOP;
    m_xWidget->SetStyle(nStyle);
}

void SalInstanceWidget::grab_focus()
{
    if (has_focus())
        return;
    m_xWidget->GrabFocus();
}

bool SalInstanceWidget::has_focus() const { return m_xWidget->HasFocus(); }

bool SalInstanceWidget::is_active() const { return m_xWidget->IsActive(); }

bool SalInstanceWidget::has_child_focus() const { return m_xWidget->HasChildPathFocus(true); }

void SalInstanceWidget::set_has_default(bool has_default)
{
    m_xWidget->set_property("has-default", OUString::boolean(has_default));
}

bool SalInstanceWidget::get_has_default() const { return m_xWidget->GetStyle() & WB_DEFBUTTON; }

void SalInstanceWidget::show() { m_xWidget->Show(); }

void SalInstanceWidget::hide() { m_xWidget->Hide(); }

void SalInstanceWidget::set_size_request(int nWidth, int nHeight)
{
    m_xWidget->set_width_request(nWidth);
    m_xWidget->set_height_request(nHeight);
}

Size SalInstanceWidget::get_size_request() const
{
    return Size(m_xWidget->get_width_request(), m_xWidget->get_height_request());
}

Size SalInstanceWidget::get_preferred_size() const { return m_xWidget->get_preferred_size(); }

float SalInstanceWidget::get_approximate_digit_width() const
{
    return m_xWidget->approximate_digit_width();
}

int SalInstanceWidget::get_text_height() const { return m_xWidget->GetTextHeight(); }

Size SalInstanceWidget::get_pixel_size(const OUString& rText) const
{
    //TODO, or do I want GetTextBoundRect ?, just using width at the moment anyway
    return Size(m_xWidget->GetTextWidth(rText), m_xWidget->GetTextHeight());
}

vcl::Font SalInstanceWidget::get_font() { return m_xWidget->GetPointFont(*m_xWidget->GetOutDev()); }

OString SalInstanceWidget::get_buildable_name() const { return m_xWidget->get_id().toUtf8(); }

void SalInstanceWidget::set_buildable_name(const OString& rId)
{
    return m_xWidget->set_id(OUString::fromUtf8(rId));
}

void SalInstanceWidget::set_help_id(const OString& rId) { return m_xWidget->SetHelpId(rId); }

OString SalInstanceWidget::get_help_id() const { return m_xWidget->GetHelpId(); }

void SalInstanceWidget::set_grid_left_attach(int nAttach)
{
    m_xWidget->set_grid_left_attach(nAttach);
}

int SalInstanceWidget::get_grid_left_attach() const { return m_xWidget->get_grid_left_attach(); }

void SalInstanceWidget::set_grid_width(int nCols) { m_xWidget->set_grid_width(nCols); }

void SalInstanceWidget::set_grid_top_attach(int nAttach)
{
    m_xWidget->set_grid_top_attach(nAttach);
}

int SalInstanceWidget::get_grid_top_attach() const { return m_xWidget->get_grid_top_attach(); }

void SalInstanceWidget::set_hexpand(bool bExpand) { m_xWidget->set_hexpand(bExpand); }

bool SalInstanceWidget::get_hexpand() const { return m_xWidget->get_hexpand(); }

void SalInstanceWidget::set_vexpand(bool bExpand) { m_xWidget->set_vexpand(bExpand); }

bool SalInstanceWidget::get_vexpand() const { return m_xWidget->get_vexpand(); }

void SalInstanceWidget::set_margin_top(int nMargin) { m_xWidget->set_margin_top(nMargin); }

void SalInstanceWidget::set_margin_bottom(int nMargin) { m_xWidget->set_margin_bottom(nMargin); }

void SalInstanceWidget::set_margin_start(int nMargin) { m_xWidget->set_margin_start(nMargin); }

void SalInstanceWidget::set_margin_end(int nMargin) { m_xWidget->set_margin_bottom(nMargin); }

int SalInstanceWidget::get_margin_top() const { return m_xWidget->get_margin_top(); }

int SalInstanceWidget::get_margin_bottom() const { return m_xWidget->get_margin_bottom(); }

int SalInstanceWidget::get_margin_start() const { return m_xWidget->get_margin_start(); }

int SalInstanceWidget::get_margin_end() const { return m_xWidget->get_margin_bottom(); }

void SalInstanceWidget::set_accessible_name(const OUString& rName)
{
    m_xWidget->SetAccessibleName(rName);
}

void SalInstanceWidget::set_accessible_description(const OUString& rDescription)
{
    m_xWidget->SetAccessibleDescription(rDescription);
}

OUString SalInstanceWidget::get_accessible_name() const { return m_xWidget->GetAccessibleName(); }

OUString SalInstanceWidget::get_accessible_description() const
{
    return m_xWidget->GetAccessibleDescription();
}

void SalInstanceWidget::set_accessible_relation_labeled_by(weld::Widget* pLabel)
{
    vcl::Window* pAtkLabel
        = pLabel ? dynamic_cast<SalInstanceWidget&>(*pLabel).getWidget() : nullptr;
    m_xWidget->SetAccessibleRelationLabeledBy(pAtkLabel);
}

void SalInstanceWidget::set_accessible_relation_label_for(weld::Widget* pLabeled)
{
    vcl::Window* pAtkLabeled
        = pLabeled ? dynamic_cast<SalInstanceWidget&>(*pLabeled).getWidget() : nullptr;
    m_xWidget->SetAccessibleRelationLabelFor(pAtkLabeled);
}

void SalInstanceWidget::set_tooltip_text(const OUString& rTip)
{
    m_xWidget->SetQuickHelpText(rTip);
}

OUString SalInstanceWidget::get_tooltip_text() const { return m_xWidget->GetQuickHelpText(); }

void SalInstanceWidget::connect_focus_in(const Link<Widget&, void>& rLink)
{
    ensure_event_listener();
    weld::Widget::connect_focus_in(rLink);
}

void SalInstanceWidget::connect_mnemonic_activate(const Link<Widget&, bool>& rLink)
{
    m_xWidget->SetMnemonicActivateHdl(LINK(this, SalInstanceWidget, MnemonicActivateHdl));
    weld::Widget::connect_mnemonic_activate(rLink);
}

void SalInstanceWidget::connect_focus_out(const Link<Widget&, void>& rLink)
{
    ensure_event_listener();
    weld::Widget::connect_focus_out(rLink);
}

void SalInstanceWidget::connect_size_allocate(const Link<const Size&, void>& rLink)
{
    ensure_event_listener();
    weld::Widget::connect_size_allocate(rLink);
}

void SalInstanceWidget::connect_mouse_press(const Link<const MouseEvent&, bool>& rLink)
{
    ensure_mouse_listener();
    weld::Widget::connect_mouse_press(rLink);
}

void SalInstanceWidget::connect_mouse_move(const Link<const MouseEvent&, bool>& rLink)
{
    ensure_mouse_listener();
    weld::Widget::connect_mouse_move(rLink);
}

void SalInstanceWidget::connect_mouse_release(const Link<const MouseEvent&, bool>& rLink)
{
    ensure_mouse_listener();
    weld::Widget::connect_mouse_release(rLink);
}

void SalInstanceWidget::connect_key_press(const Link<const KeyEvent&, bool>& rLink)
{
    ensure_key_listener();
    weld::Widget::connect_key_press(rLink);
}

void SalInstanceWidget::connect_key_release(const Link<const KeyEvent&, bool>& rLink)
{
    ensure_key_listener();
    weld::Widget::connect_key_release(rLink);
}

bool SalInstanceWidget::get_extents_relative_to(const Widget& rRelative, int& x, int& y, int& width,
                                                int& height) const
{
    tools::Rectangle aRect(m_xWidget->GetWindowExtentsRelative(
        dynamic_cast<const SalInstanceWidget&>(rRelative).getWidget()));
    x = aRect.Left();
    y = aRect.Top();
    width = aRect.GetWidth();
    height = aRect.GetHeight();
    return true;
}

void SalInstanceWidget::grab_add() { m_xWidget->CaptureMouse(); }

bool SalInstanceWidget::has_grab() const { return m_xWidget->IsMouseCaptured(); }

void SalInstanceWidget::grab_remove() { m_xWidget->ReleaseMouse(); }

bool SalInstanceWidget::get_direction() const { return m_xWidget->IsRTLEnabled(); }

void SalInstanceWidget::set_direction(bool bRTL) { m_xWidget->EnableRTL(bRTL); }

void SalInstanceWidget::freeze()
{
    if (m_nFreezeCount == 0)
        m_xWidget->SetUpdateMode(false);
    ++m_nFreezeCount;
}

void SalInstanceWidget::thaw()
{
    --m_nFreezeCount;
    if (m_nFreezeCount == 0)
        m_xWidget->SetUpdateMode(true);
}

void SalInstanceWidget::set_busy_cursor(bool bBusy)
{
    if (bBusy)
        m_xWidget->EnterWait();
    else
        m_xWidget->LeaveWait();
}

void SalInstanceWidget::queue_resize() { m_xWidget->queue_resize(); }

SalInstanceWidget::~SalInstanceWidget()
{
    if (m_aMnemonicActivateHdl.IsSet())
        m_xWidget->SetMnemonicActivateHdl(Link<vcl::Window&, bool>());
    if (m_bMouseEventListener)
        m_xWidget->RemoveChildEventListener(LINK(this, SalInstanceWidget, MouseEventListener));
    if (m_bKeyEventListener)
        Application::RemoveKeyListener(LINK(this, SalInstanceWidget, KeyEventListener));
    if (m_bEventListener)
        m_xWidget->RemoveEventListener(LINK(this, SalInstanceWidget, EventListener));
    if (m_bTakeOwnership)
        m_xWidget.disposeAndClear();
}

vcl::Window* SalInstanceWidget::getWidget() const { return m_xWidget; }

void SalInstanceWidget::disable_notify_events() { ++m_nBlockNotify; }

bool SalInstanceWidget::notify_events_disabled() const { return m_nBlockNotify != 0; }

void SalInstanceWidget::enable_notify_events() { --m_nBlockNotify; }

OUString SalInstanceWidget::strip_mnemonic(const OUString& rLabel) const
{
    return rLabel.replaceFirst("~", "");
}

VclPtr<VirtualDevice> SalInstanceWidget::create_virtual_device() const
{
    // create with (annoying) separate alpha layer that LibreOffice itself uses
    return VclPtr<VirtualDevice>::Create(*Application::GetDefaultDevice(), DeviceFormat::DEFAULT,
                                         DeviceFormat::DEFAULT);
}

css::uno::Reference<css::datatransfer::dnd::XDropTarget> SalInstanceWidget::get_drop_target()
{
    return m_xWidget->GetDropTarget();
}

css::uno::Reference<css::datatransfer::clipboard::XClipboard>
SalInstanceWidget::get_clipboard() const
{
    return m_xWidget->GetClipboard();
}

void SalInstanceWidget::connect_get_property_tree(const Link<tools::JsonWriter&, void>& rLink)
{
    m_xWidget->SetDumpAsPropertyTreeHdl(rLink);
}

void SalInstanceWidget::get_property_tree(tools::JsonWriter& rJsonWriter)
{
    m_xWidget->DumpAsPropertyTree(rJsonWriter);
}

void SalInstanceWidget::set_stack_background()
{
    set_background(m_xWidget->GetSettings().GetStyleSettings().GetWindowColor());
}

void SalInstanceWidget::set_title_background()
{
    set_background(m_xWidget->GetSettings().GetStyleSettings().GetShadowColor());
}

void SalInstanceWidget::set_toolbar_background()
{
    m_xWidget->SetBackground();
    m_xWidget->SetPaintTransparent(true);
}

void SalInstanceWidget::set_highlight_background()
{
    set_background(m_xWidget->GetSettings().GetStyleSettings().GetHighlightColor());
}

SystemWindow* SalInstanceWidget::getSystemWindow() { return m_xWidget->GetSystemWindow(); }

void SalInstanceWidget::HandleEventListener(VclWindowEvent& rEvent)
{
    if (rEvent.GetId() == VclEventId::WindowGetFocus)
        m_aFocusInHdl.Call(*this);
    else if (rEvent.GetId() == VclEventId::WindowLoseFocus)
        m_aFocusOutHdl.Call(*this);
    else if (rEvent.GetId() == VclEventId::WindowResize)
        m_aSizeAllocateHdl.Call(m_xWidget->GetSizePixel());
}

namespace
{
MouseEvent TransformEvent(const MouseEvent& rEvent, const vcl::Window* pParent,
                          const vcl::Window* pChild)
{
    return MouseEvent(
        pParent->ScreenToOutputPixel(pChild->OutputToScreenPixel(rEvent.GetPosPixel())),
        rEvent.GetClicks(), rEvent.GetMode(), rEvent.GetButtons(), rEvent.GetModifier());
}
}

void SalInstanceWidget::HandleMouseEventListener(VclWindowEvent& rWinEvent)
{
    if (rWinEvent.GetId() == VclEventId::WindowMouseButtonDown)
    {
        if (m_xWidget == rWinEvent.GetWindow())
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            m_aMousePressHdl.Call(*pMouseEvent);
        }
        else if (m_xWidget->ImplIsChild(rWinEvent.GetWindow()))
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            const MouseEvent aTransformedEvent(
                TransformEvent(*pMouseEvent, m_xWidget, rWinEvent.GetWindow()));
            m_aMousePressHdl.Call(aTransformedEvent);
        }
    }
    else if (rWinEvent.GetId() == VclEventId::WindowMouseButtonUp)
    {
        if (m_xWidget == rWinEvent.GetWindow())
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            m_aMouseReleaseHdl.Call(*pMouseEvent);
        }
        else if (m_xWidget->ImplIsChild(rWinEvent.GetWindow()))
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            const MouseEvent aTransformedEvent(
                TransformEvent(*pMouseEvent, m_xWidget, rWinEvent.GetWindow()));
            m_aMouseReleaseHdl.Call(aTransformedEvent);
        }
    }
    else if (rWinEvent.GetId() == VclEventId::WindowMouseMove)
    {
        if (m_xWidget == rWinEvent.GetWindow())
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            m_aMouseMotionHdl.Call(*pMouseEvent);
        }
        else if (m_xWidget->ImplIsChild(rWinEvent.GetWindow()))
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            const MouseEvent aTransformedEvent(
                TransformEvent(*pMouseEvent, m_xWidget, rWinEvent.GetWindow()));
            m_aMouseMotionHdl.Call(aTransformedEvent);
        }
    }
}

bool SalInstanceWidget::HandleKeyEventListener(VclWindowEvent& rEvent)
{
    // we get all key events here, ignore them unless we have focus
    if (!m_xWidget->HasChildPathFocus())
        return false;
    if (rEvent.GetId() == VclEventId::WindowKeyInput)
    {
        const KeyEvent* pKeyEvent = static_cast<const KeyEvent*>(rEvent.GetData());
        return m_aKeyPressHdl.Call(*pKeyEvent);
    }
    else if (rEvent.GetId() == VclEventId::WindowKeyUp)
    {
        const KeyEvent* pKeyEvent = static_cast<const KeyEvent*>(rEvent.GetData());
        return m_aKeyReleaseHdl.Call(*pKeyEvent);
    }
    return false;
}

IMPL_LINK(SalInstanceWidget, EventListener, VclWindowEvent&, rEvent, void)
{
    HandleEventListener(rEvent);
}

IMPL_LINK(SalInstanceWidget, KeyEventListener, VclWindowEvent&, rEvent, bool)
{
    return HandleKeyEventListener(rEvent);
}

IMPL_LINK(SalInstanceWidget, MouseEventListener, VclWindowEvent&, rEvent, void)
{
    HandleMouseEventListener(rEvent);
}

IMPL_LINK_NOARG(SalInstanceWidget, MnemonicActivateHdl, vcl::Window&, bool)
{
    return m_aMnemonicActivateHdl.Call(*this);
}

namespace
{
Image createImage(const OUString& rImage)
{
    if (rImage.isEmpty())
        return Image();
    if (rImage.lastIndexOf('.') != rImage.getLength() - 4)
    {
        assert((rImage == "dialog-warning" || rImage == "dialog-error"
                || rImage == "dialog-information")
               && "unknown stock image");
        if (rImage == "dialog-warning")
            return Image(StockImage::Yes, IMG_WARN);
        else if (rImage == "dialog-error")
            return Image(StockImage::Yes, IMG_ERROR);
        else if (rImage == "dialog-information")
            return Image(StockImage::Yes, IMG_INFO);
    }
    return Image(StockImage::Yes, rImage);
}

Image createImage(const VirtualDevice& rDevice)
{
    return Image(rDevice.GetBitmapEx(Point(), rDevice.GetOutputSizePixel()));
}

sal_uInt16 insert_to_menu(sal_uInt16 nLastId, PopupMenu* pMenu, int pos, std::u16string_view rId,
                          const OUString& rStr, const OUString* pIconName,
                          const VirtualDevice* pImageSurface,
                          const css::uno::Reference<css::graphic::XGraphic>& rImage,
                          TriState eCheckRadioFalse)
{
    const sal_uInt16 nNewid = nLastId + 1;

    MenuItemBits nBits;
    if (eCheckRadioFalse == TRISTATE_TRUE)
        nBits = MenuItemBits::CHECKABLE;
    else if (eCheckRadioFalse == TRISTATE_FALSE)
        nBits = MenuItemBits::CHECKABLE | MenuItemBits::RADIOCHECK;
    else
        nBits = MenuItemBits::NONE;

    pMenu->InsertItem(nNewid, rStr, nBits, OUStringToOString(rId, RTL_TEXTENCODING_UTF8),
                      pos == -1 ? MENU_APPEND : pos);
    if (pIconName)
    {
        pMenu->SetItemImage(nNewid, createImage(*pIconName));
    }
    else if (pImageSurface)
    {
        pMenu->SetItemImage(nNewid, createImage(*pImageSurface));
    }
    else if (rImage)
    {
        pMenu->SetItemImage(nNewid, Image(rImage));
    }
    return nNewid;
}
}

SalInstanceMenu::SalInstanceMenu(PopupMenu* pMenu, bool bTakeOwnership)
    : m_xMenu(pMenu)
    , m_bTakeOwnership(bTakeOwnership)
{
    const auto nCount = m_xMenu->GetItemCount();
    m_nLastId = nCount ? pMenu->GetItemId(nCount - 1) : 0;
    m_xMenu->SetSelectHdl(LINK(this, SalInstanceMenu, SelectMenuHdl));
}
OString SalInstanceMenu::popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                                       weld::Placement ePlace)
{
    SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pParent);
    assert(pVclWidget);
    PopupMenuFlags eFlags = PopupMenuFlags::NoMouseUpClose;
    if (ePlace == weld::Placement::Under)
        eFlags = eFlags | PopupMenuFlags::ExecuteDown;
    else
        eFlags = eFlags | PopupMenuFlags::ExecuteRight;
    m_xMenu->Execute(pVclWidget->getWidget(), rRect, eFlags);
    return m_xMenu->GetCurItemIdent();
}
void SalInstanceMenu::set_sensitive(const OString& rIdent, bool bSensitive)
{
    m_xMenu->EnableItem(rIdent, bSensitive);
}
bool SalInstanceMenu::get_sensitive(const OString& rIdent) const
{
    return m_xMenu->IsItemEnabled(m_xMenu->GetItemId(rIdent));
}
void SalInstanceMenu::set_active(const OString& rIdent, bool bActive)
{
    m_xMenu->CheckItem(rIdent, bActive);
}
bool SalInstanceMenu::get_active(const OString& rIdent) const
{
    return m_xMenu->IsItemChecked(m_xMenu->GetItemId(rIdent));
}
void SalInstanceMenu::set_label(const OString& rIdent, const OUString& rLabel)
{
    m_xMenu->SetItemText(m_xMenu->GetItemId(rIdent), rLabel);
}
OUString SalInstanceMenu::get_label(const OString& rIdent) const
{
    return m_xMenu->GetItemText(m_xMenu->GetItemId(rIdent));
}
void SalInstanceMenu::set_visible(const OString& rIdent, bool bShow)
{
    m_xMenu->ShowItem(m_xMenu->GetItemId(rIdent), bShow);
}
void SalInstanceMenu::clear() { m_xMenu->Clear(); }
void SalInstanceMenu::insert(int pos, const OUString& rId, const OUString& rStr,
                             const OUString* pIconName, VirtualDevice* pImageSurface,
                             const css::uno::Reference<css::graphic::XGraphic>& rImage,
                             TriState eCheckRadioFalse)
{
    m_nLastId = insert_to_menu(m_nLastId, m_xMenu, pos, rId, rStr, pIconName, pImageSurface, rImage,
                               eCheckRadioFalse);
}
void SalInstanceMenu::insert_separator(int pos, const OUString& rId)
{
    auto nInsertPos = pos == -1 ? MENU_APPEND : pos;
    m_xMenu->InsertSeparator(rId.toUtf8(), nInsertPos);
}
void SalInstanceMenu::remove(const OString& rId)
{
    m_xMenu->RemoveItem(m_xMenu->GetItemPos(m_xMenu->GetItemId(rId)));
}
int SalInstanceMenu::n_children() const { return m_xMenu->GetItemCount(); }
OString SalInstanceMenu::get_id(int pos) const
{
    return m_xMenu->GetItemIdent(m_xMenu->GetItemId(pos));
}
PopupMenu* SalInstanceMenu::getMenu() const { return m_xMenu.get(); }
SalInstanceMenu::~SalInstanceMenu()
{
    m_xMenu->SetSelectHdl(Link<::Menu*, bool>());
    if (m_bTakeOwnership)
        m_xMenu.disposeAndClear();
}

IMPL_LINK_NOARG(SalInstanceMenu, SelectMenuHdl, ::Menu*, bool)
{
    signal_activate(m_xMenu->GetCurItemIdent());
    /* tdf#131333 Menu::Select depends on a false here to allow
       propagating a submens's selected id to its parent menu to become its
       selected id.

       without this, while gen menus already have propagated this to its parent
       in MenuFloatingWindow::EndExecute, SalMenus as used under kf5/macOS
       won't propagate the selected id
    */
    return false;
}

SalInstanceToolbar::SalInstanceToolbar(ToolBox* pToolBox, SalInstanceBuilder* pBuilder,
                                       bool bTakeOwnership)
    : SalInstanceWidget(pToolBox, pBuilder, bTakeOwnership)
    , m_xToolBox(pToolBox)
{
    m_xToolBox->SetSelectHdl(LINK(this, SalInstanceToolbar, ClickHdl));
    m_xToolBox->SetDropdownClickHdl(LINK(this, SalInstanceToolbar, DropdownClick));
}

void SalInstanceToolbar::set_item_sensitive(const OString& rIdent, bool bSensitive)
{
    m_xToolBox->EnableItem(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), bSensitive);
}

bool SalInstanceToolbar::get_item_sensitive(const OString& rIdent) const
{
    return m_xToolBox->IsItemEnabled(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)));
}

void SalInstanceToolbar::set_item_visible(const OString& rIdent, bool bVisible)
{
    m_xToolBox->ShowItem(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), bVisible);
}

void SalInstanceToolbar::set_item_help_id(const OString& rIdent, const OString& rHelpId)
{
    m_xToolBox->SetHelpId(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), rHelpId);
}

bool SalInstanceToolbar::get_item_visible(const OString& rIdent) const
{
    return m_xToolBox->IsItemVisible(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)));
}

void SalInstanceToolbar::set_item_active(const OString& rIdent, bool bActive)
{
    ToolBoxItemId nItemId = m_xToolBox->GetItemId(OUString::fromUtf8(rIdent));
    m_xToolBox->CheckItem(nItemId, bActive);
}

bool SalInstanceToolbar::get_item_active(const OString& rIdent) const
{
    return m_xToolBox->IsItemChecked(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)));
}

void SalInstanceToolbar::set_menu_item_active(const OString& rIdent, bool bActive)
{
    ToolBoxItemId nItemId = m_xToolBox->GetItemId(OUString::fromUtf8(rIdent));
    assert(m_xToolBox->GetItemBits(nItemId) & ToolBoxItemBits::DROPDOWN);

    if (bActive)
    {
        m_sStartShowIdent = m_xToolBox->GetItemCommand(nItemId).toUtf8();
        signal_toggle_menu(m_sStartShowIdent);
    }

    auto pFloat = m_aFloats[nItemId];
    if (pFloat)
    {
        if (bActive)
            vcl::Window::GetDockingManager()->StartPopupMode(m_xToolBox, pFloat,
                                                             FloatWinPopupFlags::GrabFocus);
        else
            vcl::Window::GetDockingManager()->EndPopupMode(pFloat);
    }
    auto pPopup = m_aMenus[nItemId];
    if (pPopup)
    {
        if (bActive)
        {
            tools::Rectangle aRect = m_xToolBox->GetItemRect(nItemId);
            pPopup->Execute(m_xToolBox, aRect, PopupMenuFlags::ExecuteDown);
        }
        else
            pPopup->EndExecute();
    }

    m_sStartShowIdent.clear();
}

bool SalInstanceToolbar::get_menu_item_active(const OString& rIdent) const
{
    ToolBoxItemId nItemId = m_xToolBox->GetItemId(OUString::fromUtf8(rIdent));
    assert(m_xToolBox->GetItemBits(nItemId) & ToolBoxItemBits::DROPDOWN);

    if (rIdent == m_sStartShowIdent)
        return true;

    auto aFloat = m_aFloats.find(nItemId);
    if (aFloat != m_aFloats.end())
    {
        return vcl::Window::GetDockingManager()->IsInPopupMode(aFloat->second);
    }

    auto aPopup = m_aMenus.find(nItemId);
    if (aPopup != m_aMenus.end())
    {
        return PopupMenu::GetActivePopupMenu() == aPopup->second;
    }

    return false;
}

void SalInstanceToolbar::set_item_popover(const OString& rIdent, weld::Widget* pPopover)
{
    SalInstanceWidget* pPopoverWidget = dynamic_cast<SalInstanceWidget*>(pPopover);

    vcl::Window* pFloat = pPopoverWidget ? pPopoverWidget->getWidget() : nullptr;
    if (pFloat)
    {
        pFloat->AddEventListener(LINK(this, SalInstanceToolbar, MenuToggleListener));
        pFloat->EnableDocking();
    }

    ToolBoxItemId nId = m_xToolBox->GetItemId(OUString::fromUtf8(rIdent));
    auto xOldFloat = m_aFloats[nId];
    if (xOldFloat)
    {
        xOldFloat->RemoveEventListener(LINK(this, SalInstanceToolbar, MenuToggleListener));
    }
    m_aFloats[nId] = pFloat;
    m_aMenus[nId] = nullptr;
}

void SalInstanceToolbar::set_item_menu(const OString& rIdent, weld::Menu* pMenu)
{
    SalInstanceMenu* pInstanceMenu = dynamic_cast<SalInstanceMenu*>(pMenu);

    PopupMenu* pPopup = pInstanceMenu ? pInstanceMenu->getMenu() : nullptr;

    ToolBoxItemId nId = m_xToolBox->GetItemId(OUString::fromUtf8(rIdent));
    m_aMenus[nId] = pPopup;
    m_aFloats[nId] = nullptr;
}

void SalInstanceToolbar::insert_item(int pos, const OUString& rId)
{
    ToolBoxItemId nId(pos);
    m_xToolBox->InsertItem(nId, rId, ToolBoxItemBits::ICON_ONLY);
    m_xToolBox->SetItemCommand(nId, rId);
}

void SalInstanceToolbar::insert_separator(int pos, const OUString& /*rId*/)
{
    auto nInsertPos = pos == -1 ? ToolBox::APPEND : pos;
    m_xToolBox->InsertSeparator(nInsertPos, 5);
}

int SalInstanceToolbar::get_n_items() const { return m_xToolBox->GetItemCount(); }

OString SalInstanceToolbar::get_item_ident(int nIndex) const
{
    return m_xToolBox->GetItemCommand(m_xToolBox->GetItemId(nIndex)).toUtf8();
}

void SalInstanceToolbar::set_item_ident(int nIndex, const OString& rIdent)
{
    return m_xToolBox->SetItemCommand(m_xToolBox->GetItemId(nIndex), OUString::fromUtf8(rIdent));
}

void SalInstanceToolbar::set_item_label(int nIndex, const OUString& rLabel)
{
    m_xToolBox->SetItemText(m_xToolBox->GetItemId(nIndex), rLabel);
}

OUString SalInstanceToolbar::get_item_label(const OString& rIdent) const
{
    return m_xToolBox->GetItemText(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)));
}

void SalInstanceToolbar::set_item_label(const OString& rIdent, const OUString& rLabel)
{
    m_xToolBox->SetItemText(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), rLabel);
}

void SalInstanceToolbar::set_item_icon_name(const OString& rIdent, const OUString& rIconName)
{
    m_xToolBox->SetItemImage(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)),
                             Image(StockImage::Yes, rIconName));
}

void SalInstanceToolbar::set_item_image(const OString& rIdent,
                                        const css::uno::Reference<css::graphic::XGraphic>& rIcon)
{
    m_xToolBox->SetItemImage(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), Image(rIcon));
}

void SalInstanceToolbar::set_item_image(const OString& rIdent, VirtualDevice* pDevice)
{
    if (pDevice)
        m_xToolBox->SetItemImage(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)),
                                 createImage(*pDevice));
    else
        m_xToolBox->SetItemImage(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), Image());
}

void SalInstanceToolbar::set_item_image(int nIndex,
                                        const css::uno::Reference<css::graphic::XGraphic>& rIcon)
{
    m_xToolBox->SetItemImage(m_xToolBox->GetItemId(nIndex), Image(rIcon));
}

void SalInstanceToolbar::set_item_tooltip_text(int nIndex, const OUString& rTip)
{
    m_xToolBox->SetQuickHelpText(m_xToolBox->GetItemId(nIndex), rTip);
}

void SalInstanceToolbar::set_item_tooltip_text(const OString& rIdent, const OUString& rTip)
{
    m_xToolBox->SetQuickHelpText(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), rTip);
}

OUString SalInstanceToolbar::get_item_tooltip_text(const OString& rIdent) const
{
    return m_xToolBox->GetQuickHelpText(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)));
}

vcl::ImageType SalInstanceToolbar::get_icon_size() const { return m_xToolBox->GetImageSize(); }

void SalInstanceToolbar::set_icon_size(vcl::ImageType eType)
{
    ToolBoxButtonSize eButtonSize = ToolBoxButtonSize::DontCare;
    switch (eType)
    {
        case vcl::ImageType::Size16:
            eButtonSize = ToolBoxButtonSize::Small;
            break;
        case vcl::ImageType::Size26:
            eButtonSize = ToolBoxButtonSize::Large;
            break;
        case vcl::ImageType::Size32:
            eButtonSize = ToolBoxButtonSize::Size32;
            break;
    }
    if (m_xToolBox->GetToolboxButtonSize() != eButtonSize)
    {
        m_xToolBox->SetToolboxButtonSize(eButtonSize);
        m_xToolBox->queue_resize();
    }
}

sal_uInt16 SalInstanceToolbar::get_modifier_state() const { return m_xToolBox->GetModifier(); }

int SalInstanceToolbar::get_drop_index(const Point& rPoint) const
{
    auto nRet = m_xToolBox->GetItemPos(rPoint);
    if (nRet == ToolBox::ITEM_NOTFOUND)
        return 0;
    return nRet;
}

SalInstanceToolbar::~SalInstanceToolbar()
{
    m_xToolBox->SetDropdownClickHdl(Link<ToolBox*, void>());
    m_xToolBox->SetSelectHdl(Link<ToolBox*, void>());
}

IMPL_LINK_NOARG(SalInstanceToolbar, ClickHdl, ToolBox*, void)
{
    ToolBoxItemId nItemId = m_xToolBox->GetCurItemId();
    signal_clicked(m_xToolBox->GetItemCommand(nItemId).toUtf8());
}

IMPL_LINK_NOARG(SalInstanceToolbar, DropdownClick, ToolBox*, void)
{
    ToolBoxItemId nItemId = m_xToolBox->GetCurItemId();
    set_menu_item_active(m_xToolBox->GetItemCommand(nItemId).toUtf8(), true);
}

IMPL_LINK(SalInstanceToolbar, MenuToggleListener, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() == VclEventId::WindowEndPopupMode)
    {
        for (const auto& rFloat : m_aFloats)
        {
            if (rEvent.GetWindow() == rFloat.second)
            {
                ToolBoxItemId nItemId = rFloat.first;
                signal_toggle_menu(m_xToolBox->GetItemCommand(nItemId).toUtf8());
                break;
            }
        }
    }
}

namespace
{
class SalInstanceSizeGroup : public weld::SizeGroup
{
private:
    std::shared_ptr<VclSizeGroup> m_xGroup;

public:
    SalInstanceSizeGroup()
        : m_xGroup(std::make_shared<VclSizeGroup>())
    {
    }
    virtual void add_widget(weld::Widget* pWidget) override
    {
        SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
        assert(pVclWidget && pVclWidget->getWidget());
        pVclWidget->getWidget()->add_to_size_group(m_xGroup);
    }
    virtual void set_mode(VclSizeGroupMode eMode) override { m_xGroup->set_mode(eMode); }
};
}

void SalInstanceContainer::implResetDefault(const vcl::Window* _pWindow)
{
    vcl::Window* pChildLoop = _pWindow->GetWindow(GetWindowType::FirstChild);
    while (pChildLoop)
    {
        // does the window participate in the tabbing order?
        if (pChildLoop->GetStyle() & WB_DIALOGCONTROL)
            implResetDefault(pChildLoop);

        // is it a button?
        WindowType eType = pChildLoop->GetType();
        if ((WindowType::PUSHBUTTON == eType) || (WindowType::OKBUTTON == eType)
            || (WindowType::CANCELBUTTON == eType) || (WindowType::HELPBUTTON == eType)
            || (WindowType::IMAGEBUTTON == eType) || (WindowType::MENUBUTTON == eType)
            || (WindowType::MOREBUTTON == eType))
        {
            pChildLoop->SetStyle(pChildLoop->GetStyle() & ~WB_DEFBUTTON);
        }

        // the next one ...
        pChildLoop = pChildLoop->GetWindow(GetWindowType::Next);
    }
}

void SalInstanceContainer::connect_container_focus_changed(const Link<Container&, void>& rLink)
{
    ensure_event_listener();
    weld::Container::connect_container_focus_changed(rLink);
}

void SalInstanceContainer::HandleEventListener(VclWindowEvent& rEvent)
{
    if (rEvent.GetId() == VclEventId::WindowActivate
        || rEvent.GetId() == VclEventId::WindowDeactivate)
    {
        signal_container_focus_changed();
        return;
    }
    SalInstanceWidget::HandleEventListener(rEvent);
}

SalInstanceContainer::SalInstanceContainer(vcl::Window* pContainer, SalInstanceBuilder* pBuilder,
                                           bool bTakeOwnership)
    : SalInstanceWidget(pContainer, pBuilder, bTakeOwnership)
    , m_xContainer(pContainer)
{
}

void SalInstanceContainer::move(weld::Widget* pWidget, weld::Container* pNewParent)
{
    SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
    assert(pVclWidget);
    SalInstanceContainer* pNewVclParent = dynamic_cast<SalInstanceContainer*>(pNewParent);
    assert(!pNewParent || pNewVclParent);
    vcl::Window* pVclWindow = pVclWidget->getWidget();
    if (pNewVclParent)
        pVclWindow->SetParent(pNewVclParent->getWidget());
    else
    {
        pVclWindow->Hide();
        pVclWindow->SetParent(ImplGetDefaultWindow());
    }
}

void SalInstanceContainer::child_grab_focus()
{
    m_xContainer->GrabFocus();
    if (vcl::Window* pFirstChild = m_xContainer->ImplGetDlgWindow(0, GetDlgWindowType::First))
        pFirstChild->ImplControlFocus();
}

void SalInstanceContainer::recursively_unset_default_buttons()
{
    implResetDefault(m_xContainer.get());
}

css::uno::Reference<css::awt::XWindow> SalInstanceContainer::CreateChildFrame()
{
    auto xPage = VclPtr<VclBin>::Create(m_xContainer.get());
    xPage->set_expand(true);
    xPage->Show();
    return css::uno::Reference<css::awt::XWindow>(xPage->GetComponentInterface(),
                                                  css::uno::UNO_QUERY);
}

std::unique_ptr<weld::Container> SalInstanceWidget::weld_parent() const
{
    vcl::Window* pParent = m_xWidget->GetParent();
    if (!pParent)
        return nullptr;
    return std::make_unique<SalInstanceContainer>(pParent, m_pBuilder, false);
}

void SalInstanceWidget::DoRecursivePaint(vcl::Window* pWindow, const Point& rRenderLogicPos,
                                         OutputDevice& rOutput)
{
    rOutput.Push();
    bool bOldMapModeEnabled = pWindow->IsMapModeEnabled();

    if (pWindow->GetMapMode().GetMapUnit() != rOutput.GetMapMode().GetMapUnit())
    {
        // This is needed for e.g. the scrollbar in writer comments in margins that has its map unit in pixels
        // as seen with bin/run gtktiledviewer --enable-tiled-annotations on a document containing a comment
        // long enough to need a scrollbar
        pWindow->EnableMapMode();
        MapMode aMapMode = pWindow->GetMapMode();
        aMapMode.SetMapUnit(rOutput.GetMapMode().GetMapUnit());
        aMapMode.SetScaleX(rOutput.GetMapMode().GetScaleX());
        aMapMode.SetScaleY(rOutput.GetMapMode().GetScaleY());
        pWindow->SetMapMode(aMapMode);
    }

    VclPtr<VirtualDevice> xOutput(VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT));
    Size aChildSizePixel(pWindow->GetSizePixel());
    xOutput->SetOutputSizePixel(aChildSizePixel);

    MapMode aMapMode(xOutput->GetMapMode());
    aMapMode.SetMapUnit(rOutput.GetMapMode().GetMapUnit());
    aMapMode.SetScaleX(rOutput.GetMapMode().GetScaleX());
    aMapMode.SetScaleY(rOutput.GetMapMode().GetScaleY());
    xOutput->SetMapMode(aMapMode);

    Size aTempLogicSize(xOutput->PixelToLogic(aChildSizePixel));
    Size aRenderLogicSize(rOutput.PixelToLogic(aChildSizePixel));

    switch (rOutput.GetOutDevType())
    {
        case OUTDEV_WINDOW:
        case OUTDEV_VIRDEV:
            xOutput->DrawOutDev(Point(), aTempLogicSize, rRenderLogicPos, aRenderLogicSize,
                                rOutput);
            break;
        case OUTDEV_PRINTER:
        case OUTDEV_PDF:
            xOutput->SetBackground(rOutput.GetBackground());
            xOutput->Erase();
            break;
    }

    //set ReallyVisible to match Visible, we restore the original state after Paint
    WindowImpl* pImpl = pWindow->ImplGetWindowImpl();
    bool bRVisible = pImpl->mbReallyVisible;
    pImpl->mbReallyVisible = pWindow->IsVisible();

    pWindow->ApplySettings(*xOutput);
    pWindow->Paint(*xOutput, tools::Rectangle(Point(), pWindow->PixelToLogic(aChildSizePixel)));

    pImpl->mbReallyVisible = bRVisible;

    switch (rOutput.GetOutDevType())
    {
        case OUTDEV_WINDOW:
        case OUTDEV_VIRDEV:
            rOutput.DrawOutDev(rRenderLogicPos, aRenderLogicSize, Point(), aTempLogicSize,
                               *xOutput);
            break;
        case OUTDEV_PRINTER:
        case OUTDEV_PDF:
            rOutput.DrawBitmapEx(rRenderLogicPos, aRenderLogicSize,
                                 xOutput->GetBitmapEx(Point(), aTempLogicSize));
            break;
    }

    xOutput.disposeAndClear();

    pWindow->EnableMapMode(bOldMapModeEnabled);
    rOutput.Pop();

    for (vcl::Window* pChild = pWindow->GetWindow(GetWindowType::FirstChild); pChild;
         pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        Point aRelPos(pChild->GetPosPixel());
        Size aRelLogicOffset(rOutput.PixelToLogic(Size(aRelPos.X(), aRelPos.Y())));
        DoRecursivePaint(pChild,
                         rRenderLogicPos + Point(aRelLogicOffset.Width(), aRelLogicOffset.Height()),
                         rOutput);
    }
}

void SalInstanceWidget::draw(OutputDevice& rOutput, const Point& rPos, const Size& rSizePixel)
{
    Size aOrigSize(m_xWidget->GetSizePixel());
    bool bChangeSize = aOrigSize != rSizePixel;
    if (bChangeSize)
        m_xWidget->SetSizePixel(rSizePixel);

    DoRecursivePaint(m_xWidget, rPos, rOutput);

    if (bChangeSize)
        m_xWidget->SetSizePixel(aOrigSize);
}

SalInstanceBox::SalInstanceBox(VclBox* pContainer, SalInstanceBuilder* pBuilder,
                               bool bTakeOwnership)
    : SalInstanceContainer(pContainer, pBuilder, bTakeOwnership)
    , m_xBox(pContainer)
{
}
void SalInstanceBox::reorder_child(weld::Widget* pWidget, int nNewPosition)
{
    SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
    assert(pVclWidget);
    pVclWidget->getWidget()->reorderWithinParent(nNewPosition);
}

void SalInstanceBox::sort_native_button_order() { ::sort_native_button_order(*m_xBox); }

namespace
{
void CollectChildren(const vcl::Window& rCurrent, const basegfx::B2IPoint& rTopLeft,
                     weld::ScreenShotCollection& rControlDataCollection)
{
    if (!rCurrent.IsVisible())
        return;

    const Point aCurrentPos(rCurrent.GetPosPixel());
    const Size aCurrentSize(rCurrent.GetSizePixel());
    const basegfx::B2IPoint aCurrentTopLeft(rTopLeft.getX() + aCurrentPos.X(),
                                            rTopLeft.getY() + aCurrentPos.Y());
    const basegfx::B2IRange aCurrentRange(
        aCurrentTopLeft,
        aCurrentTopLeft + basegfx::B2IPoint(aCurrentSize.Width(), aCurrentSize.Height()));

    if (!aCurrentRange.isEmpty())
    {
        rControlDataCollection.emplace_back(rCurrent.GetHelpId(), aCurrentRange);
    }

    for (sal_uInt16 a(0); a < rCurrent.GetChildCount(); a++)
    {
        vcl::Window* pChild = rCurrent.GetChild(a);
        if (nullptr != pChild)
        {
            CollectChildren(*pChild, aCurrentTopLeft, rControlDataCollection);
        }
    }
}
}

void SalInstanceWindow::override_child_help(vcl::Window* pParent)
{
    for (vcl::Window* pChild = pParent->GetWindow(GetWindowType::FirstChild); pChild;
         pChild = pChild->GetWindow(GetWindowType::Next))
        override_child_help(pChild);
    pParent->SetHelpHdl(LINK(this, SalInstanceWindow, HelpHdl));
}

void SalInstanceWindow::clear_child_help(vcl::Window* pParent)
{
    for (vcl::Window* pChild = pParent->GetWindow(GetWindowType::FirstChild); pChild;
         pChild = pChild->GetWindow(GetWindowType::Next))
        clear_child_help(pChild);
    pParent->SetHelpHdl(Link<vcl::Window&, bool>());
}

SalInstanceWindow::SalInstanceWindow(vcl::Window* pWindow, SalInstanceBuilder* pBuilder,
                                     bool bTakeOwnership)
    : SalInstanceContainer(pWindow, pBuilder, bTakeOwnership)
    , m_xWindow(pWindow)
{
    override_child_help(m_xWindow);
}

void SalInstanceWindow::set_title(const OUString& rTitle) { m_xWindow->SetText(rTitle); }

OUString SalInstanceWindow::get_title() const { return m_xWindow->GetText(); }

css::uno::Reference<css::awt::XWindow> SalInstanceWindow::GetXWindow()
{
    css::uno::Reference<css::awt::XWindow> xWindow(m_xWindow->GetComponentInterface(),
                                                   css::uno::UNO_QUERY);
    return xWindow;
}

namespace
{
void resize_to_request(vcl::Window* pWindow)
{
    if (SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(pWindow))
    {
        pSysWin->setOptimalLayoutSize();
        return;
    }
    if (DockingWindow* pDockWin = dynamic_cast<DockingWindow*>(pWindow))
    {
        pDockWin->setOptimalLayoutSize();
        return;
    }
    assert(false && "must be system or docking window");
}
}

void SalInstanceWindow::resize_to_request() { ::resize_to_request(m_xWindow.get()); }

void SalInstanceWindow::set_modal(bool bModal) { m_xWindow->ImplGetFrame()->SetModal(bModal); }

bool SalInstanceWindow::get_modal() const { return m_xWindow->ImplGetFrame()->GetModal(); }

void SalInstanceWindow::window_move(int x, int y) { m_xWindow->SetPosPixel(Point(x, y)); }

Size SalInstanceWindow::get_size() const { return m_xWindow->GetSizePixel(); }

Point SalInstanceWindow::get_position() const { return m_xWindow->GetPosPixel(); }

tools::Rectangle SalInstanceWindow::get_monitor_workarea() const
{
    return m_xWindow->GetDesktopRectPixel();
}

void SalInstanceWindow::set_centered_on_parent(bool /*bTrackGeometryRequests*/)
{
    if (vcl::Window* pParent = m_xWidget->GetParent())
    {
        Size aParentGeometry(pParent->GetSizePixel());
        Size aGeometry(m_xWidget->get_preferred_size());
        auto nX = (aParentGeometry.Width() - aGeometry.Width()) / 2;
        auto nY = (aParentGeometry.Height() - aGeometry.Height()) / 2;
        m_xWidget->SetPosPixel(Point(nX, nY));
    }
}

bool SalInstanceWindow::get_resizable() const { return m_xWindow->GetStyle() & WB_SIZEABLE; }

bool SalInstanceWindow::has_toplevel_focus() const { return m_xWindow->HasChildPathFocus(); }

void SalInstanceWindow::present()
{
    m_xWindow->ToTop(ToTopFlags::RestoreWhenMin | ToTopFlags::ForegroundTask);
}

void SalInstanceWindow::set_window_state(const OString& rStr)
{
    SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(m_xWindow.get());
    assert(pSysWin);
    pSysWin->SetWindowState(rStr);
}

OString SalInstanceWindow::get_window_state(WindowStateMask nMask) const
{
    SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(m_xWindow.get());
    assert(pSysWin);
    return pSysWin->GetWindowState(nMask);
}

SystemEnvData SalInstanceWindow::get_system_data() const { return *m_xWindow->GetSystemData(); }

VclPtr<VirtualDevice> SalInstanceWindow::screenshot()
{
    SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(m_xWindow.get());
    assert(pSysWin);
    return pSysWin->createScreenshot();
}

weld::ScreenShotCollection SalInstanceWindow::collect_screenshot_data()
{
    weld::ScreenShotCollection aRet;

    // collect all children. Choose start pos to be negative
    // of target dialog's position to get all positions relative to (0,0)
    const Point aParentPos(m_xWindow->GetPosPixel());
    const basegfx::B2IPoint aTopLeft(-aParentPos.X(), -aParentPos.Y());
    CollectChildren(*m_xWindow, aTopLeft, aRet);

    return aRet;
}

SalInstanceWindow::~SalInstanceWindow() { clear_child_help(m_xWindow); }

IMPL_LINK_NOARG(SalInstanceWindow, HelpHdl, vcl::Window&, bool)
{
    help();
    return false;
}

typedef std::set<VclPtr<vcl::Window>> winset;

namespace
{
void hideUnless(const vcl::Window* pTop, const winset& rVisibleWidgets,
                std::vector<VclPtr<vcl::Window>>& rWasVisibleWidgets)
{
    for (vcl::Window* pChild = pTop->GetWindow(GetWindowType::FirstChild); pChild;
         pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        if (rVisibleWidgets.find(pChild) == rVisibleWidgets.end())
        {
            rWasVisibleWidgets.emplace_back(pChild);
            pChild->Hide();
        }
        else if (isContainerWindow(pChild))
        {
            hideUnless(pChild, rVisibleWidgets, rWasVisibleWidgets);
        }
    }
}
}

SalInstanceDialog::SalInstanceDialog(::Dialog* pDialog, SalInstanceBuilder* pBuilder,
                                     bool bTakeOwnership)
    : SalInstanceWindow(pDialog, pBuilder, bTakeOwnership)
    , m_xDialog(pDialog)
    , m_nOldEditWidthReq(0)
    , m_nOldBorderWidth(0)
{
    const bool bScreenshotMode(officecfg::Office::Common::Misc::ScreenshotMode::get());
    if (bScreenshotMode)
    {
        m_xDialog->SetPopupMenuHdl(LINK(this, SalInstanceDialog, PopupScreenShotMenuHdl));
    }
}

bool SalInstanceDialog::runAsync(std::shared_ptr<weld::DialogController> aOwner,
                                 const std::function<void(sal_Int32)>& rEndDialogFn)
{
    VclAbstractDialog::AsyncContext aCtx;
    aCtx.mxOwnerDialogController = aOwner;
    aCtx.maEndDialogFn = rEndDialogFn;
    VclButtonBox* pActionArea = m_xDialog->get_action_area();
    if (pActionArea)
        sort_native_button_order(*pActionArea);
    return m_xDialog->StartExecuteAsync(aCtx);
}

bool SalInstanceDialog::runAsync(std::shared_ptr<Dialog> const& rxSelf,
                                 const std::function<void(sal_Int32)>& rEndDialogFn)
{
    assert(rxSelf.get() == this);
    VclAbstractDialog::AsyncContext aCtx;
    // In order to store a shared_ptr to ourself, we have to have been constructed by make_shared,
    // which is that rxSelf enforces.
    aCtx.mxOwnerSelf = rxSelf;
    aCtx.maEndDialogFn = rEndDialogFn;
    VclButtonBox* pActionArea = m_xDialog->get_action_area();
    if (pActionArea)
        sort_native_button_order(*pActionArea);
    return m_xDialog->StartExecuteAsync(aCtx);
}

void SalInstanceDialog::collapse(weld::Widget* pEdit, weld::Widget* pButton)
{
    SalInstanceWidget* pVclEdit = dynamic_cast<SalInstanceWidget*>(pEdit);
    assert(pVclEdit);
    SalInstanceWidget* pVclButton = dynamic_cast<SalInstanceWidget*>(pButton);

    vcl::Window* pRefEdit = pVclEdit->getWidget();
    vcl::Window* pRefBtn = pVclButton ? pVclButton->getWidget() : nullptr;

    auto nOldEditWidth = pRefEdit->GetSizePixel().Width();
    m_nOldEditWidthReq = pRefEdit->get_width_request();

    //We want just pRefBtn and pRefEdit to be shown
    //mark widgets we want to be visible, starting with pRefEdit
    //and all its direct parents.
    winset aVisibleWidgets;
    vcl::Window* pContentArea = m_xDialog->get_content_area();
    for (vcl::Window* pCandidate = pRefEdit;
         pCandidate && (pCandidate != pContentArea && pCandidate->IsVisible());
         pCandidate = pCandidate->GetWindow(GetWindowType::RealParent))
    {
        aVisibleWidgets.insert(pCandidate);
    }
    //same again with pRefBtn, except stop if there's a
    //shared parent in the existing widgets
    for (vcl::Window* pCandidate = pRefBtn;
         pCandidate && (pCandidate != pContentArea && pCandidate->IsVisible());
         pCandidate = pCandidate->GetWindow(GetWindowType::RealParent))
    {
        if (aVisibleWidgets.insert(pCandidate).second)
            break;
    }

    //hide everything except the aVisibleWidgets
    hideUnless(pContentArea, aVisibleWidgets, m_aHiddenWidgets);

    // the insert function case has an initially hidden edit widget, so it has
    // not start size, so take larger of actual size and size request
    pRefEdit->set_width_request(std::max(nOldEditWidth, m_nOldEditWidthReq));
    m_nOldBorderWidth = m_xDialog->get_border_width();
    m_xDialog->set_border_width(0);
    if (vcl::Window* pActionArea = m_xDialog->get_action_area())
        pActionArea->Hide();
    m_xDialog->setOptimalLayoutSize();
    m_xRefEdit = pRefEdit;
}

void SalInstanceDialog::undo_collapse()
{
    // All others: Show();
    for (VclPtr<vcl::Window> const& pWindow : m_aHiddenWidgets)
    {
        pWindow->Show();
    }
    m_aHiddenWidgets.clear();

    m_xRefEdit->set_width_request(m_nOldEditWidthReq);
    m_xRefEdit.clear();
    m_xDialog->set_border_width(m_nOldBorderWidth);
    if (vcl::Window* pActionArea = m_xDialog->get_action_area())
        pActionArea->Show();
    m_xDialog->setOptimalLayoutSize();
}

void SalInstanceDialog::SetInstallLOKNotifierHdl(
    const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink)
{
    m_xDialog->SetInstallLOKNotifierHdl(rLink);
}

int SalInstanceDialog::run()
{
    VclButtonBox* pActionArea = m_xDialog->get_action_area();
    if (pActionArea)
        sort_native_button_order(*pActionArea);
    return m_xDialog->Execute();
}

void SalInstanceDialog::response(int nResponse) { m_xDialog->EndDialog(nResponse); }

void SalInstanceDialog::add_button(const OUString& rText, int nResponse, const OString& rHelpId)
{
    VclButtonBox* pBox = m_xDialog->get_action_area();
    VclPtr<PushButton> xButton(
        VclPtr<PushButton>::Create(pBox, WB_CLIPCHILDREN | WB_CENTER | WB_VCENTER));
    xButton->SetText(rText);
    xButton->SetHelpId(rHelpId);

    switch (nResponse)
    {
        case RET_OK:
            xButton->set_id("ok");
            break;
        case RET_CLOSE:
            xButton->set_id("close");
            break;
        case RET_CANCEL:
            xButton->set_id("cancel");
            break;
        case RET_YES:
            xButton->set_id("yes");
            break;
        case RET_NO:
            xButton->set_id("no");
            break;
    }

    xButton->Show();
    m_xDialog->add_button(xButton, nResponse, true);
}

void SalInstanceDialog::set_modal(bool bModal)
{
    if (get_modal() == bModal)
        return;
    m_xDialog->SetModalInputMode(bModal);
}

bool SalInstanceDialog::get_modal() const { return m_xDialog->IsModalInputMode(); }

void SalInstanceDialog::set_default_response(int nResponse)
{
    m_xDialog->set_default_response(nResponse);
}

weld::Container* SalInstanceDialog::weld_content_area()
{
    return new SalInstanceContainer(m_xDialog->get_content_area(), m_pBuilder, false);
}

IMPL_LINK(SalInstanceDialog, PopupScreenShotMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (CommandEventId::ContextMenu == rCEvt.GetCommand())
    {
        const Point aMenuPos(rCEvt.GetMousePosPixel());
        ScopedVclPtrInstance<PopupMenu> aMenu;
        sal_uInt16 nLocalID(1);

        aMenu->InsertItem(nLocalID, VclResId(SV_BUTTONTEXT_SCREENSHOT));
        aMenu->SetHelpText(nLocalID, VclResId(SV_HELPTEXT_SCREENSHOT));
        aMenu->SetHelpId(nLocalID, "InteractiveScreenshotMode");
        aMenu->EnableItem(nLocalID);

        const sal_uInt16 nId(aMenu->Execute(m_xDialog, aMenuPos));

        // 0 == no selection (so not usable as ID)
        if (0 != nId)
        {
            // open screenshot annotation dialog
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            VclPtr<AbstractScreenshotAnnotationDlg> pTmp
                = pFact->CreateScreenshotAnnotationDlg(*this);
            ScopedVclPtr<AbstractScreenshotAnnotationDlg> pDialog(pTmp);

            if (pDialog)
            {
                // currently just execute the dialog, no need to do
                // different things for ok/cancel. This may change later,
                // for that case use 'if (pDlg->Execute() == RET_OK)'
                pDialog->Execute();
            }
        }

        // consume event when:
        // - CommandEventId::ContextMenu
        // - bScreenshotMode
        return true;
    }

    return false;
}

SalInstanceMessageDialog::SalInstanceMessageDialog(::MessageDialog* pDialog,
                                                   SalInstanceBuilder* pBuilder,
                                                   bool bTakeOwnership)
    : SalInstanceDialog(pDialog, pBuilder, bTakeOwnership)
    , m_xMessageDialog(pDialog)
{
}

void SalInstanceMessageDialog::set_primary_text(const OUString& rText)
{
    m_xMessageDialog->set_primary_text(rText);
}

OUString SalInstanceMessageDialog::get_primary_text() const
{
    return m_xMessageDialog->get_primary_text();
}

void SalInstanceMessageDialog::set_secondary_text(const OUString& rText)
{
    m_xMessageDialog->set_secondary_text(rText);
}

OUString SalInstanceMessageDialog::get_secondary_text() const
{
    return m_xMessageDialog->get_secondary_text();
}

weld::Container* SalInstanceMessageDialog::weld_message_area()
{
    return new SalInstanceContainer(m_xMessageDialog->get_message_area(), m_pBuilder, false);
}

namespace
{
class SalInstanceAssistant : public SalInstanceDialog, public virtual weld::Assistant
{
private:
    VclPtr<vcl::RoadmapWizard> m_xWizard;
    std::vector<std::unique_ptr<SalInstanceContainer>> m_aPages;
    std::vector<VclPtr<TabPage>> m_aAddedPages;
    std::vector<int> m_aIds;
    std::vector<VclPtr<VclGrid>> m_aAddedGrids;
    Idle m_aUpdateRoadmapIdle;

    int find_page(std::string_view rIdent) const
    {
        for (size_t i = 0; i < m_aAddedPages.size(); ++i)
        {
            if (m_aAddedPages[i]->get_id().toUtf8() == rIdent)
                return i;
        }
        return -1;
    }

    int find_id(int nId) const
    {
        for (size_t i = 0; i < m_aIds.size(); ++i)
        {
            if (nId == m_aIds[i])
                return i;
        }
        return -1;
    }

    DECL_LINK(OnRoadmapItemSelected, LinkParamNone*, void);
    DECL_LINK(UpdateRoadmap_Hdl, Timer*, void);

public:
    SalInstanceAssistant(vcl::RoadmapWizard* pDialog, SalInstanceBuilder* pBuilder,
                         bool bTakeOwnership)
        : SalInstanceDialog(pDialog, pBuilder, bTakeOwnership)
        , m_xWizard(pDialog)
        , m_aUpdateRoadmapIdle("SalInstanceAssistant m_aUpdateRoadmapIdle")
    {
        m_xWizard->SetItemSelectHdl(LINK(this, SalInstanceAssistant, OnRoadmapItemSelected));

        m_aUpdateRoadmapIdle.SetInvokeHandler(LINK(this, SalInstanceAssistant, UpdateRoadmap_Hdl));
        m_aUpdateRoadmapIdle.SetPriority(TaskPriority::HIGHEST);
    }

    virtual int get_current_page() const override { return find_id(m_xWizard->GetCurLevel()); }

    virtual int get_n_pages() const override { return m_aAddedPages.size(); }

    virtual OString get_page_ident(int nPage) const override
    {
        return m_aAddedPages[nPage]->get_id().toUtf8();
    }

    virtual OString get_current_page_ident() const override
    {
        return get_page_ident(get_current_page());
    }

    virtual void set_current_page(int nPage) override
    {
        disable_notify_events();

        // take the first shown page as the size for all pages
        if (m_xWizard->GetPageSizePixel().Width() == 0)
        {
            Size aFinalSize;
            for (int i = 0, nPages = get_n_pages(); i < nPages; ++i)
            {
                TabPage* pPage = m_xWizard->GetPage(m_aIds[i]);
                assert(pPage);
                Size aPageSize(pPage->get_preferred_size());
                if (aPageSize.Width() > aFinalSize.Width())
                    aFinalSize.setWidth(aPageSize.Width());
                if (aPageSize.Height() > aFinalSize.Height())
                    aFinalSize.setHeight(aPageSize.Height());
            }
            m_xWizard->SetPageSizePixel(aFinalSize);
        }

        (void)m_xWizard->ShowPage(m_aIds[nPage]);
        enable_notify_events();
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        int nIndex = find_page(rIdent);
        if (nIndex == -1)
            return;
        set_current_page(nIndex);
    }

    virtual void set_page_index(const OString& rIdent, int nNewIndex) override
    {
        int nOldIndex = find_page(rIdent);

        if (nOldIndex == -1)
            return;

        if (nOldIndex == nNewIndex)
            return;

        disable_notify_events();

        auto entry = std::move(m_aAddedPages[nOldIndex]);
        m_aAddedPages.erase(m_aAddedPages.begin() + nOldIndex);
        m_aAddedPages.insert(m_aAddedPages.begin() + nNewIndex, std::move(entry));

        int nId = m_aIds[nOldIndex];
        m_aIds.erase(m_aIds.begin() + nOldIndex);
        m_aIds.insert(m_aIds.begin() + nNewIndex, nId);

        m_aUpdateRoadmapIdle.Start();

        enable_notify_events();
    }

    virtual weld::Container* append_page(const OString& rIdent) override
    {
        VclPtrInstance<TabPage> xPage(m_xWizard);
        VclPtrInstance<VclGrid> xGrid(xPage);
        xPage->set_id(OUString::fromUtf8(rIdent));
        xPage->Show();
        xGrid->set_hexpand(true);
        xGrid->set_vexpand(true);
        xGrid->Show();
        m_xWizard->AddPage(xPage);
        m_aIds.push_back(m_aAddedPages.size());
        m_xWizard->SetPage(m_aIds.back(), xPage);
        m_aAddedPages.push_back(xPage);
        m_aAddedGrids.push_back(xGrid);

        m_aUpdateRoadmapIdle.Start();

        m_aPages.emplace_back(new SalInstanceContainer(xGrid, m_pBuilder, false));
        return m_aPages.back().get();
    }

    virtual OUString get_page_title(const OString& rIdent) const override
    {
        int nIndex = find_page(rIdent);
        if (nIndex == -1)
            return OUString();
        return m_aAddedPages[nIndex]->GetText();
    }

    virtual void set_page_title(const OString& rIdent, const OUString& rTitle) override
    {
        int nIndex = find_page(rIdent);
        if (nIndex == -1)
            return;
        if (m_aAddedPages[nIndex]->GetText() != rTitle)
        {
            disable_notify_events();
            m_aAddedPages[nIndex]->SetText(rTitle);
            m_aUpdateRoadmapIdle.Start();
            enable_notify_events();
        }
    }

    virtual void set_page_sensitive(const OString& rIdent, bool bSensitive) override
    {
        int nIndex = find_page(rIdent);
        if (nIndex == -1)
            return;
        if (m_aAddedPages[nIndex]->IsEnabled() != bSensitive)
        {
            disable_notify_events();
            m_aAddedPages[nIndex]->Enable(bSensitive);
            m_aUpdateRoadmapIdle.Start();
            enable_notify_events();
        }
    }

    virtual void set_page_side_help_id(const OString& rHelpId) override
    {
        m_xWizard->SetRoadmapHelpId(rHelpId);
    }

    weld::Button* weld_widget_for_response(int nResponse) override;

    virtual ~SalInstanceAssistant() override
    {
        for (auto& rGrid : m_aAddedGrids)
            rGrid.disposeAndClear();
        for (auto& rPage : m_aAddedPages)
            rPage.disposeAndClear();
    }
};
}

IMPL_LINK_NOARG(SalInstanceAssistant, OnRoadmapItemSelected, LinkParamNone*, void)
{
    if (notify_events_disabled())
        return;
    auto nCurItemId = m_xWizard->GetCurrentRoadmapItemID();
    int nPageIndex(find_id(nCurItemId));
    if (!signal_jump_page(get_page_ident(nPageIndex)) && nCurItemId != m_xWizard->GetCurLevel())
        m_xWizard->SelectRoadmapItemByID(m_xWizard->GetCurLevel());
}

IMPL_LINK_NOARG(SalInstanceAssistant, UpdateRoadmap_Hdl, Timer*, void)
{
    disable_notify_events();

    m_xWizard->DeleteRoadmapItems();

    int nPos = 0;
    for (size_t i = 0; i < m_aAddedPages.size(); ++i)
    {
        const OUString& rLabel = m_aAddedPages[i]->GetText();
        bool bSensitive = m_aAddedPages[i]->IsEnabled();
        if (rLabel.isEmpty())
            continue;
        m_xWizard->InsertRoadmapItem(nPos++, rLabel, m_aIds[i], bSensitive);
    }

    m_xWizard->SelectRoadmapItemByID(m_aIds[get_current_page()], false);

    m_xWizard->ShowRoadmap(nPos != 0);

    enable_notify_events();
}

SalInstanceFrame::SalInstanceFrame(VclFrame* pFrame, SalInstanceBuilder* pBuilder,
                                   bool bTakeOwnership)
    : SalInstanceContainer(pFrame, pBuilder, bTakeOwnership)
    , m_xFrame(pFrame)
{
}

void SalInstanceFrame::set_label(const OUString& rText) { m_xFrame->set_label(rText); }

OUString SalInstanceFrame::get_label() const { return m_xFrame->get_label(); }

namespace
{
class SalInstancePaned : public SalInstanceContainer, public virtual weld::Paned
{
private:
    VclPtr<VclPaned> m_xPaned;

public:
    SalInstancePaned(VclPaned* pPaned, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pPaned, pBuilder, bTakeOwnership)
        , m_xPaned(pPaned)
    {
    }

    virtual void set_position(int nPos) override { m_xPaned->set_position(nPos); }

    virtual int get_position() const override { return m_xPaned->get_position(); }
};

class SalInstanceScrolledWindow : public SalInstanceContainer, public virtual weld::ScrolledWindow
{
private:
    VclPtr<VclScrolledWindow> m_xScrolledWindow;
    Link<ScrollBar*, void> m_aOrigVScrollHdl;
    Link<ScrollBar*, void> m_aOrigHScrollHdl;
    bool m_bUserManagedScrolling;

    DECL_LINK(VscrollHdl, ScrollBar*, void);
    DECL_LINK(HscrollHdl, ScrollBar*, void);

    static void customize_scrollbars(ScrollBar& rScrollBar, const Color& rButtonTextColor,
                                     const Color& rBackgroundColor, const Color& rShadowColor,
                                     const Color& rFaceColor)
    {
        rScrollBar.EnableNativeWidget(false);
        AllSettings aSettings = rScrollBar.GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        aStyleSettings.SetButtonTextColor(rButtonTextColor);
        aStyleSettings.SetCheckedColor(rBackgroundColor); // background
        aStyleSettings.SetShadowColor(rShadowColor);
        aStyleSettings.SetFaceColor(rFaceColor);
        aSettings.SetStyleSettings(aStyleSettings);
        rScrollBar.SetSettings(aSettings);
    }

public:
    SalInstanceScrolledWindow(VclScrolledWindow* pScrolledWindow, SalInstanceBuilder* pBuilder,
                              bool bTakeOwnership, bool bUserManagedScrolling)
        : SalInstanceContainer(pScrolledWindow, pBuilder, bTakeOwnership)
        , m_xScrolledWindow(pScrolledWindow)
        , m_bUserManagedScrolling(bUserManagedScrolling)
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        m_aOrigVScrollHdl = rVertScrollBar.GetScrollHdl();
        rVertScrollBar.SetScrollHdl(LINK(this, SalInstanceScrolledWindow, VscrollHdl));
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        m_aOrigHScrollHdl = rHorzScrollBar.GetScrollHdl();
        rHorzScrollBar.SetScrollHdl(LINK(this, SalInstanceScrolledWindow, HscrollHdl));
        m_xScrolledWindow->setUserManagedScrolling(m_bUserManagedScrolling);
    }

    virtual void hadjustment_configure(int value, int lower, int upper, int step_increment,
                                       int page_increment, int page_size) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        rHorzScrollBar.SetRangeMin(lower);
        rHorzScrollBar.SetRangeMax(upper);
        rHorzScrollBar.SetLineSize(step_increment);
        rHorzScrollBar.SetPageSize(page_increment);
        rHorzScrollBar.SetThumbPos(value);
        rHorzScrollBar.SetVisibleSize(page_size);
    }

    virtual int hadjustment_get_value() const override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        return rHorzScrollBar.GetThumbPos();
    }

    virtual void hadjustment_set_value(int value) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        rHorzScrollBar.SetThumbPos(value);
        if (!m_bUserManagedScrolling)
            m_aOrigHScrollHdl.Call(&rHorzScrollBar);
    }

    virtual int hadjustment_get_upper() const override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        return rHorzScrollBar.GetRangeMax();
    }

    virtual void hadjustment_set_upper(int upper) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        rHorzScrollBar.SetRangeMax(upper);
    }

    virtual int hadjustment_get_page_size() const override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        return rHorzScrollBar.GetVisibleSize();
    }

    virtual void hadjustment_set_page_size(int size) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        return rHorzScrollBar.SetVisibleSize(size);
    }

    virtual void hadjustment_set_page_increment(int size) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        return rHorzScrollBar.SetPageSize(size);
    }

    virtual void hadjustment_set_step_increment(int size) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        return rHorzScrollBar.SetLineSize(size);
    }

    virtual void set_hpolicy(VclPolicyType eHPolicy) override
    {
        WinBits nWinBits = m_xScrolledWindow->GetStyle() & ~(WB_AUTOHSCROLL | WB_HSCROLL);
        if (eHPolicy == VclPolicyType::ALWAYS)
            nWinBits |= WB_HSCROLL;
        else if (eHPolicy == VclPolicyType::AUTOMATIC)
            nWinBits |= WB_AUTOHSCROLL;
        m_xScrolledWindow->SetStyle(nWinBits);
        m_xScrolledWindow->queue_resize();
    }

    virtual VclPolicyType get_hpolicy() const override
    {
        WinBits nWinBits = m_xScrolledWindow->GetStyle();
        if (nWinBits & WB_AUTOHSCROLL)
            return VclPolicyType::AUTOMATIC;
        else if (nWinBits & WB_HSCROLL)
            return VclPolicyType::ALWAYS;
        return VclPolicyType::NEVER;
    }

    virtual void vadjustment_configure(int value, int lower, int upper, int step_increment,
                                       int page_increment, int page_size) override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        rVertScrollBar.SetRangeMin(lower);
        rVertScrollBar.SetRangeMax(upper);
        rVertScrollBar.SetLineSize(step_increment);
        rVertScrollBar.SetPageSize(page_increment);
        rVertScrollBar.SetThumbPos(value);
        rVertScrollBar.SetVisibleSize(page_size);
    }

    virtual int vadjustment_get_value() const override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.GetThumbPos();
    }

    virtual void vadjustment_set_value(int value) override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        rVertScrollBar.SetThumbPos(value);
        if (!m_bUserManagedScrolling)
            m_aOrigVScrollHdl.Call(&rVertScrollBar);
    }

    virtual int vadjustment_get_upper() const override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.GetRangeMax();
    }

    virtual void vadjustment_set_upper(int upper) override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        rVertScrollBar.SetRangeMax(upper);
    }

    virtual int vadjustment_get_lower() const override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.GetRangeMin();
    }

    virtual void vadjustment_set_lower(int lower) override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        rVertScrollBar.SetRangeMin(lower);
    }

    virtual int vadjustment_get_page_size() const override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.GetVisibleSize();
    }

    virtual void vadjustment_set_page_size(int size) override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.SetVisibleSize(size);
    }

    virtual void vadjustment_set_page_increment(int size) override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.SetPageSize(size);
    }

    virtual void vadjustment_set_step_increment(int size) override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.SetLineSize(size);
    }

    virtual void set_vpolicy(VclPolicyType eVPolicy) override
    {
        WinBits nWinBits = m_xScrolledWindow->GetStyle() & ~(WB_AUTOVSCROLL | WB_VSCROLL);
        if (eVPolicy == VclPolicyType::ALWAYS)
            nWinBits |= WB_VSCROLL;
        else if (eVPolicy == VclPolicyType::AUTOMATIC)
            nWinBits |= WB_AUTOVSCROLL;
        m_xScrolledWindow->SetStyle(nWinBits);
        m_xScrolledWindow->queue_resize();
    }

    virtual VclPolicyType get_vpolicy() const override
    {
        WinBits nWinBits = m_xScrolledWindow->GetStyle();
        if (nWinBits & WB_AUTOVSCROLL)
            return VclPolicyType::AUTOMATIC;
        else if (nWinBits & WB_VSCROLL)
            return VclPolicyType::ALWAYS;
        return VclPolicyType::NEVER;
    }

    virtual int get_scroll_thickness() const override
    {
        return m_xScrolledWindow->getVertScrollBar().get_preferred_size().Width();
    }

    virtual void set_scroll_thickness(int nThickness) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        rHorzScrollBar.set_height_request(nThickness);
        rVertScrollBar.set_width_request(nThickness);
    }

    virtual void customize_scrollbars(const Color& rBackgroundColor, const Color& rShadowColor,
                                      const Color& rFaceColor) override
    {
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        customize_scrollbars(rHorzScrollBar, Color(0, 0, 0), rBackgroundColor, rShadowColor,
                             rFaceColor);
        customize_scrollbars(rVertScrollBar, Color(0, 0, 0), rBackgroundColor, rShadowColor,
                             rFaceColor);
    }

    virtual ~SalInstanceScrolledWindow() override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        rVertScrollBar.SetScrollHdl(m_aOrigVScrollHdl);
    }
};
}

IMPL_LINK(SalInstanceScrolledWindow, VscrollHdl, ScrollBar*, pScrollBar, void)
{
    signal_vadjustment_changed();
    if (!m_bUserManagedScrolling)
        m_aOrigVScrollHdl.Call(pScrollBar);
}

IMPL_LINK_NOARG(SalInstanceScrolledWindow, HscrollHdl, ScrollBar*, void)
{
    signal_hadjustment_changed();
    if (!m_bUserManagedScrolling)
        m_aOrigHScrollHdl.Call(&m_xScrolledWindow->getHorzScrollBar());
}

SalInstanceNotebook::SalInstanceNotebook(TabControl* pNotebook, SalInstanceBuilder* pBuilder,
                                         bool bTakeOwnership)
    : SalInstanceWidget(pNotebook, pBuilder, bTakeOwnership)
    , m_xNotebook(pNotebook)
{
    m_xNotebook->SetActivatePageHdl(LINK(this, SalInstanceNotebook, ActivatePageHdl));
    m_xNotebook->SetDeactivatePageHdl(LINK(this, SalInstanceNotebook, DeactivatePageHdl));
}

int SalInstanceNotebook::get_current_page() const
{
    return m_xNotebook->GetPagePos(m_xNotebook->GetCurPageId());
}

OString SalInstanceNotebook::get_page_ident(int nPage) const
{
    return m_xNotebook->GetPageName(m_xNotebook->GetPageId(nPage));
}

OString SalInstanceNotebook::get_current_page_ident() const
{
    return m_xNotebook->GetPageName(m_xNotebook->GetCurPageId());
}

int SalInstanceNotebook::get_page_index(const OString& rIdent) const
{
    sal_uInt16 nPageId = m_xNotebook->GetPageId(rIdent);
    sal_uInt16 nPageIndex = m_xNotebook->GetPagePos(nPageId);
    if (nPageIndex == TAB_PAGE_NOTFOUND)
        return -1;
    return nPageIndex;
}

weld::Container* SalInstanceNotebook::get_page(const OString& rIdent) const
{
    int nPageIndex = get_page_index(rIdent);
    if (nPageIndex == -1)
        return nullptr;
    sal_uInt16 nPageId = m_xNotebook->GetPageId(rIdent);
    TabPage* pPage = m_xNotebook->GetTabPage(nPageId);
    vcl::Window* pChild = pPage->GetChild(0);
    if (m_aPages.size() < nPageIndex + 1U)
        m_aPages.resize(nPageIndex + 1U);
    if (!m_aPages[nPageIndex])
        m_aPages[nPageIndex] = std::make_shared<SalInstanceContainer>(pChild, m_pBuilder, false);
    return m_aPages[nPageIndex].get();
}

void SalInstanceNotebook::set_current_page(int nPage)
{
    m_xNotebook->SetCurPageId(m_xNotebook->GetPageId(nPage));
}

void SalInstanceNotebook::set_current_page(const OString& rIdent)
{
    m_xNotebook->SetCurPageId(m_xNotebook->GetPageId(rIdent));
}

void SalInstanceNotebook::remove_page(const OString& rIdent)
{
    sal_uInt16 nPageId = m_xNotebook->GetPageId(rIdent);
    sal_uInt16 nPageIndex = m_xNotebook->GetPagePos(nPageId);
    if (nPageIndex == TAB_PAGE_NOTFOUND)
        return;

    m_xNotebook->RemovePage(nPageId);
    if (nPageIndex < m_aPages.size())
        m_aPages.erase(m_aPages.begin() + nPageIndex);

    auto iter = m_aAddedPages.find(rIdent);
    if (iter != m_aAddedPages.end())
    {
        iter->second.second.disposeAndClear();
        iter->second.first.disposeAndClear();
        m_aAddedPages.erase(iter);
    }
}

void SalInstanceNotebook::insert_page(const OString& rIdent, const OUString& rLabel, int nPos)
{
    sal_uInt16 nPageCount = m_xNotebook->GetPageCount();
    sal_uInt16 nLastPageId = nPageCount ? m_xNotebook->GetPageId(nPageCount - 1) : 0;
    sal_uInt16 nNewPageId = nLastPageId + 1;
    while (m_xNotebook->GetPagePos(nNewPageId) != TAB_PAGE_NOTFOUND)
        ++nNewPageId;
    m_xNotebook->InsertPage(nNewPageId, rLabel, nPos == -1 ? TAB_APPEND : nPos);
    VclPtrInstance<TabPage> xPage(m_xNotebook);
    VclPtrInstance<VclGrid> xGrid(xPage);
    xPage->Show();
    xGrid->set_hexpand(true);
    xGrid->set_vexpand(true);
    xGrid->Show();
    m_xNotebook->SetTabPage(nNewPageId, xPage);
    m_xNotebook->SetPageName(nNewPageId, rIdent);
    m_aAddedPages.try_emplace(rIdent, xPage, xGrid);

    if (nPos != -1)
    {
        unsigned int nPageIndex = static_cast<unsigned int>(nPos);
        if (nPageIndex < m_aPages.size())
            m_aPages.insert(m_aPages.begin() + nPageIndex, nullptr);
    }
}

int SalInstanceNotebook::get_n_pages() const { return m_xNotebook->GetPageCount(); }

OUString SalInstanceNotebook::get_tab_label_text(const OString& rIdent) const
{
    return m_xNotebook->GetPageText(m_xNotebook->GetPageId(rIdent));
}

void SalInstanceNotebook::set_tab_label_text(const OString& rIdent, const OUString& rText)
{
    return m_xNotebook->SetPageText(m_xNotebook->GetPageId(rIdent), rText);
}

SalInstanceNotebook::~SalInstanceNotebook()
{
    for (auto& rItem : m_aAddedPages)
    {
        rItem.second.second.disposeAndClear();
        rItem.second.first.disposeAndClear();
    }
    m_xNotebook->SetActivatePageHdl(Link<TabControl*, void>());
    m_xNotebook->SetDeactivatePageHdl(Link<TabControl*, bool>());
}

IMPL_LINK_NOARG(SalInstanceNotebook, DeactivatePageHdl, TabControl*, bool)
{
    return !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
}

IMPL_LINK_NOARG(SalInstanceNotebook, ActivatePageHdl, TabControl*, void)
{
    m_aEnterPageHdl.Call(get_current_page_ident());
}

namespace
{
class SalInstanceVerticalNotebook : public SalInstanceWidget, public virtual weld::Notebook
{
private:
    VclPtr<VerticalTabControl> m_xNotebook;
    mutable std::vector<std::unique_ptr<SalInstanceContainer>> m_aPages;

    DECL_LINK(DeactivatePageHdl, VerticalTabControl*, bool);
    DECL_LINK(ActivatePageHdl, VerticalTabControl*, void);

public:
    SalInstanceVerticalNotebook(VerticalTabControl* pNotebook, SalInstanceBuilder* pBuilder,
                                bool bTakeOwnership)
        : SalInstanceWidget(pNotebook, pBuilder, bTakeOwnership)
        , m_xNotebook(pNotebook)
    {
        m_xNotebook->SetActivatePageHdl(LINK(this, SalInstanceVerticalNotebook, ActivatePageHdl));
        m_xNotebook->SetDeactivatePageHdl(
            LINK(this, SalInstanceVerticalNotebook, DeactivatePageHdl));
    }

    virtual int get_current_page() const override
    {
        return m_xNotebook->GetPagePos(m_xNotebook->GetCurPageId());
    }

    virtual OString get_page_ident(int nPage) const override
    {
        return m_xNotebook->GetPageId(nPage);
    }

    virtual OString get_current_page_ident() const override { return m_xNotebook->GetCurPageId(); }

    virtual int get_page_index(const OString& rIdent) const override
    {
        sal_uInt16 nPageIndex = m_xNotebook->GetPagePos(rIdent);
        if (nPageIndex == TAB_PAGE_NOTFOUND)
            return -1;
        return nPageIndex;
    }

    virtual weld::Container* get_page(const OString& rIdent) const override
    {
        int nPageIndex = get_page_index(rIdent);
        if (nPageIndex == -1)
            return nullptr;
        auto pChild = m_xNotebook->GetPage(rIdent);
        if (m_aPages.size() < nPageIndex + 1U)
            m_aPages.resize(nPageIndex + 1U);
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new SalInstanceContainer(pChild, m_pBuilder, false));
        return m_aPages[nPageIndex].get();
    }

    virtual void set_current_page(int nPage) override
    {
        m_xNotebook->SetCurPageId(m_xNotebook->GetPageId(nPage));
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        m_xNotebook->SetCurPageId(rIdent);
    }

    virtual void remove_page(const OString& rIdent) override
    {
        sal_uInt16 nPageIndex = m_xNotebook->GetPagePos(rIdent);
        if (nPageIndex == TAB_PAGE_NOTFOUND)
            return;
        m_xNotebook->RemovePage(rIdent);
        if (nPageIndex < m_aPages.size())
            m_aPages.erase(m_aPages.begin() + nPageIndex);
    }

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override
    {
        VclPtrInstance<VclGrid> xGrid(m_xNotebook->GetPageParent());
        xGrid->set_hexpand(true);
        xGrid->set_vexpand(true);
        m_xNotebook->InsertPage(rIdent, rLabel, Image(), "", xGrid, nPos);

        if (nPos != -1)
        {
            unsigned int nPageIndex = static_cast<unsigned int>(nPos);
            if (nPageIndex < m_aPages.size())
                m_aPages.insert(m_aPages.begin() + nPageIndex, nullptr);
        }
    }

    virtual int get_n_pages() const override { return m_xNotebook->GetPageCount(); }

    virtual void set_tab_label_text(const OString& rIdent, const OUString& rText) override
    {
        return m_xNotebook->SetPageText(rIdent, rText);
    }

    virtual OUString get_tab_label_text(const OString& rIdent) const override
    {
        return m_xNotebook->GetPageText(rIdent);
    }

    virtual ~SalInstanceVerticalNotebook() override
    {
        m_xNotebook->SetActivatePageHdl(Link<VerticalTabControl*, void>());
        m_xNotebook->SetDeactivatePageHdl(Link<VerticalTabControl*, bool>());
    }
};
}

IMPL_LINK_NOARG(SalInstanceVerticalNotebook, DeactivatePageHdl, VerticalTabControl*, bool)
{
    return !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
}

IMPL_LINK_NOARG(SalInstanceVerticalNotebook, ActivatePageHdl, VerticalTabControl*, void)
{
    m_aEnterPageHdl.Call(get_current_page_ident());
}

SalInstanceButton::SalInstanceButton(::Button* pButton, SalInstanceBuilder* pBuilder,
                                     bool bTakeOwnership)
    : SalInstanceWidget(pButton, pBuilder, bTakeOwnership)
    , m_xButton(pButton)
    , m_aOldClickHdl(pButton->GetClickHdl())
{
    m_xButton->SetClickHdl(LINK(this, SalInstanceButton, ClickHdl));
}

void SalInstanceButton::set_label(const OUString& rText) { m_xButton->SetText(rText); }

void SalInstanceButton::set_image(VirtualDevice* pDevice)
{
    m_xButton->SetImageAlign(ImageAlign::Left);
    if (pDevice)
        m_xButton->SetModeImage(createImage(*pDevice));
    else
        m_xButton->SetModeImage(Image());
}

void SalInstanceButton::set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage)
{
    m_xButton->SetImageAlign(ImageAlign::Left);
    m_xButton->SetModeImage(Image(rImage));
}

void SalInstanceButton::set_from_icon_name(const OUString& rIconName)
{
    m_xButton->SetModeImage(Image(StockImage::Yes, rIconName));
}

void SalInstanceButton::set_label_wrap(bool wrap)
{
    WinBits nBits = m_xButton->GetStyle();
    nBits &= ~WB_WORDBREAK;
    if (wrap)
        nBits |= WB_WORDBREAK;
    m_xButton->SetStyle(nBits);
    m_xButton->queue_resize();
}

void SalInstanceButton::set_font(const vcl::Font& rFont)
{
    m_xButton->SetControlFont(rFont);
    m_xButton->Invalidate();
}

void SalInstanceButton::set_custom_button(VirtualDevice* pDevice)
{
    if (pDevice)
        m_xButton->SetCustomButtonImage(createImage(*pDevice));
    else
        m_xButton->SetCustomButtonImage(Image());
    m_xButton->Invalidate();
}

OUString SalInstanceButton::get_label() const { return m_xButton->GetText(); }

SalInstanceButton::~SalInstanceButton() { m_xButton->SetClickHdl(Link<::Button*, void>()); }

IMPL_LINK(SalInstanceButton, ClickHdl, ::Button*, pButton, void)
{
    //if there's no handler set, disengage our intercept and
    //run the click again to get default behaviour for cancel/ok
    //etc buttons.
    if (!m_aClickHdl.IsSet())
    {
        pButton->SetClickHdl(m_aOldClickHdl);
        pButton->Click();
        pButton->SetClickHdl(LINK(this, SalInstanceButton, ClickHdl));
        return;
    }
    signal_clicked();
}

weld::Button* SalInstanceDialog::weld_widget_for_response(int nResponse)
{
    PushButton* pButton = dynamic_cast<PushButton*>(m_xDialog->get_widget_for_response(nResponse));
    return pButton ? new SalInstanceButton(pButton, nullptr, false) : nullptr;
}

weld::Button* SalInstanceAssistant::weld_widget_for_response(int nResponse)
{
    PushButton* pButton = nullptr;
    if (nResponse == RET_YES)
        pButton = m_xWizard->m_pNextPage;
    else if (nResponse == RET_NO)
        pButton = m_xWizard->m_pPrevPage;
    else if (nResponse == RET_OK)
        pButton = m_xWizard->m_pFinish;
    else if (nResponse == RET_CANCEL)
        pButton = m_xWizard->m_pCancel;
    else if (nResponse == RET_HELP)
        pButton = m_xWizard->m_pHelp;
    if (pButton)
        return new SalInstanceButton(pButton, nullptr, false);
    return nullptr;
}

SalInstanceMenuButton::SalInstanceMenuButton(::MenuButton* pButton, SalInstanceBuilder* pBuilder,
                                             bool bTakeOwnership)
    : SalInstanceButton(pButton, pBuilder, bTakeOwnership)
    , m_xMenuButton(pButton)
    , m_nLastId(0)
{
    m_xMenuButton->SetActivateHdl(LINK(this, SalInstanceMenuButton, ActivateHdl));
    m_xMenuButton->SetSelectHdl(LINK(this, SalInstanceMenuButton, MenuSelectHdl));
    if (PopupMenu* pMenu = m_xMenuButton->GetPopupMenu())
    {
        pMenu->SetMenuFlags(MenuFlags::NoAutoMnemonics);
        const auto nCount = pMenu->GetItemCount();
        m_nLastId = nCount ? pMenu->GetItemId(nCount - 1) : 0;
    }
}

void SalInstanceMenuButton::set_active(bool active)
{
    if (active == get_active())
        return;
    if (active)
        m_xMenuButton->ExecuteMenu();
    else
        m_xMenuButton->CancelMenu();
}

bool SalInstanceMenuButton::get_active() const { return m_xMenuButton->InPopupMode(); }

void SalInstanceMenuButton::set_inconsistent(bool /*inconsistent*/)
{
    //not available
}

bool SalInstanceMenuButton::get_inconsistent() const { return false; }

void SalInstanceMenuButton::insert_item(int pos, const OUString& rId, const OUString& rStr,
                                        const OUString* pIconName, VirtualDevice* pImageSurface,
                                        TriState eCheckRadioFalse)
{
    m_nLastId = insert_to_menu(m_nLastId, m_xMenuButton->GetPopupMenu(), pos, rId, rStr, pIconName,
                               pImageSurface, nullptr, eCheckRadioFalse);
}

void SalInstanceMenuButton::insert_separator(int pos, const OUString& rId)
{
    auto nInsertPos = pos == -1 ? MENU_APPEND : pos;
    m_xMenuButton->GetPopupMenu()->InsertSeparator(rId.toUtf8(), nInsertPos);
}

void SalInstanceMenuButton::set_item_sensitive(const OString& rIdent, bool bSensitive)
{
    PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
    pMenu->EnableItem(rIdent, bSensitive);
}

void SalInstanceMenuButton::remove_item(const OString& rId)
{
    PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
    pMenu->RemoveItem(pMenu->GetItemPos(pMenu->GetItemId(rId)));
}

void SalInstanceMenuButton::clear()
{
    PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
    pMenu->Clear();
}

void SalInstanceMenuButton::set_item_active(const OString& rIdent, bool bActive)
{
    PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
    pMenu->CheckItem(rIdent, bActive);
}

void SalInstanceMenuButton::set_item_label(const OString& rIdent, const OUString& rText)
{
    PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
    pMenu->SetItemText(pMenu->GetItemId(rIdent), rText);
}

OUString SalInstanceMenuButton::get_item_label(const OString& rIdent) const
{
    PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
    return pMenu->GetItemText(pMenu->GetItemId(rIdent));
}

void SalInstanceMenuButton::set_item_visible(const OString& rIdent, bool bShow)
{
    PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
    pMenu->ShowItem(pMenu->GetItemId(rIdent), bShow);
}

void SalInstanceMenuButton::set_popover(weld::Widget* pPopover)
{
    SalInstanceWidget* pPopoverWidget = dynamic_cast<SalInstanceWidget*>(pPopover);
    m_xMenuButton->SetPopover(pPopoverWidget ? pPopoverWidget->getWidget() : nullptr);
}

SalInstanceMenuButton::~SalInstanceMenuButton()
{
    m_xMenuButton->SetSelectHdl(Link<::MenuButton*, void>());
    m_xMenuButton->SetActivateHdl(Link<::MenuButton*, void>());
}

IMPL_LINK_NOARG(SalInstanceMenuButton, MenuSelectHdl, ::MenuButton*, void)
{
    signal_selected(m_xMenuButton->GetCurItemIdent());
}

IMPL_LINK_NOARG(SalInstanceMenuButton, ActivateHdl, ::MenuButton*, void)
{
    if (notify_events_disabled())
        return;
    signal_toggled();
}

namespace
{
class SalInstanceMenuToggleButton : public SalInstanceMenuButton,
                                    public virtual weld::MenuToggleButton
{
private:
    VclPtr<::MenuToggleButton> m_xMenuToggleButton;

public:
    SalInstanceMenuToggleButton(::MenuToggleButton* pButton, SalInstanceBuilder* pBuilder,
                                bool bTakeOwnership)
        : SalInstanceMenuButton(pButton, pBuilder, bTakeOwnership)
        , m_xMenuToggleButton(pButton)
    {
        m_xMenuToggleButton->SetDelayMenu(true);
        m_xMenuToggleButton->SetDropDown(PushButtonDropdownStyle::SplitMenuButton);
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
        m_xMenuToggleButton->SetActive(active);
        enable_notify_events();
    }

    virtual bool get_active() const override { return m_xMenuToggleButton->GetActive(); }
};
}

namespace
{
class SalInstanceLinkButton : public SalInstanceWidget, public virtual weld::LinkButton
{
private:
    VclPtr<FixedHyperlink> m_xButton;
    Link<FixedHyperlink&, void> m_aOrigClickHdl;

    DECL_LINK(ClickHdl, FixedHyperlink&, void);

public:
    SalInstanceLinkButton(FixedHyperlink* pButton, SalInstanceBuilder* pBuilder,
                          bool bTakeOwnership)
        : SalInstanceWidget(pButton, pBuilder, bTakeOwnership)
        , m_xButton(pButton)
    {
        m_aOrigClickHdl = m_xButton->GetClickHdl();
        m_xButton->SetClickHdl(LINK(this, SalInstanceLinkButton, ClickHdl));
    }

    virtual void set_label(const OUString& rText) override { m_xButton->SetText(rText); }

    virtual OUString get_label() const override { return m_xButton->GetText(); }

    virtual void set_uri(const OUString& rUri) override { m_xButton->SetURL(rUri); }

    virtual OUString get_uri() const override { return m_xButton->GetURL(); }

    virtual ~SalInstanceLinkButton() override { m_xButton->SetClickHdl(m_aOrigClickHdl); }
};
}

IMPL_LINK(SalInstanceLinkButton, ClickHdl, FixedHyperlink&, rButton, void)
{
    bool bConsumed = signal_activate_link();
    if (!bConsumed)
        m_aOrigClickHdl.Call(rButton);
}

SalInstanceRadioButton::SalInstanceRadioButton(::RadioButton* pButton, SalInstanceBuilder* pBuilder,
                                               bool bTakeOwnership)
    : SalInstanceButton(pButton, pBuilder, bTakeOwnership)
    , m_xRadioButton(pButton)
{
    m_xRadioButton->SetToggleHdl(LINK(this, SalInstanceRadioButton, ToggleHdl));
}

void SalInstanceRadioButton::set_active(bool active)
{
    disable_notify_events();
    m_xRadioButton->Check(active);
    enable_notify_events();
}

bool SalInstanceRadioButton::get_active() const { return m_xRadioButton->IsChecked(); }

void SalInstanceRadioButton::set_image(VirtualDevice* pDevice)
{
    m_xRadioButton->SetImageAlign(ImageAlign::Center);
    if (pDevice)
        m_xRadioButton->SetModeImage(createImage(*pDevice));
    else
        m_xRadioButton->SetModeImage(Image());
}

void SalInstanceRadioButton::set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage)
{
    m_xRadioButton->SetImageAlign(ImageAlign::Center);
    m_xRadioButton->SetModeImage(Image(rImage));
}

void SalInstanceRadioButton::set_from_icon_name(const OUString& rIconName)
{
    m_xRadioButton->SetModeRadioImage(Image(StockImage::Yes, rIconName));
}

void SalInstanceRadioButton::set_inconsistent(bool /*inconsistent*/)
{
    //not available
}

bool SalInstanceRadioButton::get_inconsistent() const { return false; }

SalInstanceRadioButton::~SalInstanceRadioButton()
{
    m_xRadioButton->SetToggleHdl(Link<::RadioButton&, void>());
}

IMPL_LINK_NOARG(SalInstanceRadioButton, ToggleHdl, ::RadioButton&, void)
{
    if (notify_events_disabled())
        return;
    signal_toggled();
}

namespace
{
class SalInstanceToggleButton : public SalInstanceButton, public virtual weld::ToggleButton
{
private:
    VclPtr<PushButton> m_xToggleButton;

    DECL_LINK(ToggleListener, VclWindowEvent&, void);

public:
    SalInstanceToggleButton(PushButton* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceButton(pButton, pBuilder, bTakeOwnership)
        , m_xToggleButton(pButton)
    {
    }

    virtual void connect_toggled(const Link<Toggleable&, void>& rLink) override
    {
        assert(!m_aToggleHdl.IsSet());
        m_xToggleButton->AddEventListener(LINK(this, SalInstanceToggleButton, ToggleListener));
        weld::ToggleButton::connect_toggled(rLink);
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
        m_xToggleButton->Check(active);
        enable_notify_events();
    }

    virtual bool get_active() const override { return m_xToggleButton->IsChecked(); }

    virtual void set_inconsistent(bool inconsistent) override
    {
        disable_notify_events();
        m_xToggleButton->SetState(inconsistent ? TRISTATE_INDET : TRISTATE_FALSE);
        enable_notify_events();
    }

    virtual bool get_inconsistent() const override
    {
        return m_xToggleButton->GetState() == TRISTATE_INDET;
    }

    virtual ~SalInstanceToggleButton() override
    {
        if (m_aToggleHdl.IsSet())
            m_xToggleButton->RemoveEventListener(
                LINK(this, SalInstanceToggleButton, ToggleListener));
    }
};
}

IMPL_LINK(SalInstanceToggleButton, ToggleListener, VclWindowEvent&, rEvent, void)
{
    if (notify_events_disabled())
        return;
    if (rEvent.GetId() == VclEventId::PushbuttonToggle)
        signal_toggled();
}

SalInstanceCheckButton::SalInstanceCheckButton(CheckBox* pButton, SalInstanceBuilder* pBuilder,
                                               bool bTakeOwnership)
    : SalInstanceButton(pButton, pBuilder, bTakeOwnership)
    , m_xCheckButton(pButton)
{
    m_xCheckButton->SetToggleHdl(LINK(this, SalInstanceCheckButton, ToggleHdl));
}

void SalInstanceCheckButton::set_active(bool active)
{
    disable_notify_events();
    m_xCheckButton->EnableTriState(false);
    m_xCheckButton->Check(active);
    enable_notify_events();
}

bool SalInstanceCheckButton::get_active() const { return m_xCheckButton->IsChecked(); }

void SalInstanceCheckButton::set_inconsistent(bool inconsistent)
{
    disable_notify_events();
    m_xCheckButton->EnableTriState(true);
    m_xCheckButton->SetState(inconsistent ? TRISTATE_INDET : TRISTATE_FALSE);
    enable_notify_events();
}

bool SalInstanceCheckButton::get_inconsistent() const
{
    return m_xCheckButton->GetState() == TRISTATE_INDET;
}

SalInstanceCheckButton::~SalInstanceCheckButton()
{
    m_xCheckButton->SetToggleHdl(Link<CheckBox&, void>());
}

IMPL_LINK_NOARG(SalInstanceCheckButton, ToggleHdl, CheckBox&, void)
{
    if (notify_events_disabled())
        return;
    m_xCheckButton->EnableTriState(false);
    signal_toggled();
}

namespace
{
class SalInstanceScale : public SalInstanceWidget, public virtual weld::Scale
{
private:
    VclPtr<Slider> m_xScale;

    DECL_LINK(SlideHdl, Slider*, void);

public:
    SalInstanceScale(Slider* pScale, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pScale, pBuilder, bTakeOwnership)
        , m_xScale(pScale)
    {
        m_xScale->SetSlideHdl(LINK(this, SalInstanceScale, SlideHdl));
    }

    virtual void set_value(int value) override { m_xScale->SetThumbPos(value); }

    virtual void set_range(int min, int max) override
    {
        m_xScale->SetRangeMin(min);
        m_xScale->SetRangeMax(max);
    }

    virtual int get_value() const override { return m_xScale->GetThumbPos(); }

    virtual void set_increments(int step, int page) override
    {
        m_xScale->SetLineSize(step);
        m_xScale->SetPageSize(page);
    }

    virtual void get_increments(int& step, int& page) const override
    {
        step = m_xScale->GetLineSize();
        page = m_xScale->GetPageSize();
    }

    virtual ~SalInstanceScale() override { m_xScale->SetSlideHdl(Link<Slider*, void>()); }
};
}

IMPL_LINK_NOARG(SalInstanceScale, SlideHdl, Slider*, void) { signal_value_changed(); }

namespace
{
class SalInstanceSpinner : public SalInstanceWidget, public virtual weld::Spinner
{
private:
    VclPtr<Throbber> m_xThrobber;

public:
    SalInstanceSpinner(Throbber* pThrobber, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pThrobber, pBuilder, bTakeOwnership)
        , m_xThrobber(pThrobber)
    {
    }

    virtual void start() override { m_xThrobber->start(); }

    virtual void stop() override { m_xThrobber->stop(); }
};

class SalInstanceProgressBar : public SalInstanceWidget, public virtual weld::ProgressBar
{
private:
    VclPtr<::ProgressBar> m_xProgressBar;

public:
    SalInstanceProgressBar(::ProgressBar* pProgressBar, SalInstanceBuilder* pBuilder,
                           bool bTakeOwnership)
        : SalInstanceWidget(pProgressBar, pBuilder, bTakeOwnership)
        , m_xProgressBar(pProgressBar)
    {
    }

    virtual void set_percentage(int value) override { m_xProgressBar->SetValue(value); }

    virtual OUString get_text() const override { return m_xProgressBar->GetText(); }

    virtual void set_text(const OUString& rText) override { m_xProgressBar->SetText(rText); }
};

class SalInstanceImage : public SalInstanceWidget, public virtual weld::Image
{
private:
    VclPtr<FixedImage> m_xImage;

public:
    SalInstanceImage(FixedImage* pImage, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pImage, pBuilder, bTakeOwnership)
        , m_xImage(pImage)
    {
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        m_xImage->SetImage(::Image(StockImage::Yes, rIconName));
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        if (pDevice)
            m_xImage->SetImage(createImage(*pDevice));
        else
            m_xImage->SetImage(::Image());
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        m_xImage->SetImage(::Image(rImage));
    }
};

class SalInstanceCalendar : public SalInstanceWidget, public virtual weld::Calendar
{
private:
    VclPtr<::Calendar> m_xCalendar;

    DECL_LINK(SelectHdl, ::Calendar*, void);
    DECL_LINK(ActivateHdl, ::Calendar*, void);

public:
    SalInstanceCalendar(::Calendar* pCalendar, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pCalendar, pBuilder, bTakeOwnership)
        , m_xCalendar(pCalendar)
    {
        m_xCalendar->SetSelectHdl(LINK(this, SalInstanceCalendar, SelectHdl));
        m_xCalendar->SetActivateHdl(LINK(this, SalInstanceCalendar, ActivateHdl));
    }

    virtual void set_date(const Date& rDate) override { m_xCalendar->SetCurDate(rDate); }

    virtual Date get_date() const override { return m_xCalendar->GetFirstSelectedDate(); }

    virtual ~SalInstanceCalendar() override
    {
        m_xCalendar->SetSelectHdl(Link<::Calendar*, void>());
        m_xCalendar->SetActivateHdl(Link<::Calendar*, void>());
    }
};
}

IMPL_LINK_NOARG(SalInstanceCalendar, SelectHdl, ::Calendar*, void)
{
    if (notify_events_disabled())
        return;
    signal_selected();
}

IMPL_LINK_NOARG(SalInstanceCalendar, ActivateHdl, ::Calendar*, void)
{
    if (notify_events_disabled())
        return;
    signal_activated();
}

WeldTextFilter::WeldTextFilter(Link<OUString&, bool>& rInsertTextHdl)
    : TextFilter(OUString())
    , m_rInsertTextHdl(rInsertTextHdl)
{
}

OUString WeldTextFilter::filter(const OUString& rText)
{
    if (!m_rInsertTextHdl.IsSet())
        return rText;
    OUString sText(rText);
    const bool bContinue = m_rInsertTextHdl.Call(sText);
    if (!bContinue)
        return OUString();
    return sText;
}

SalInstanceEntry::SalInstanceEntry(Edit* pEntry, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
    : SalInstanceWidget(pEntry, pBuilder, bTakeOwnership)
    , m_xEntry(pEntry)
    , m_aTextFilter(m_aInsertTextHdl)
{
    m_xEntry->SetModifyHdl(LINK(this, SalInstanceEntry, ChangeHdl));
    m_xEntry->SetActivateHdl(LINK(this, SalInstanceEntry, ActivateHdl));
    m_xEntry->SetTextFilter(&m_aTextFilter);
}

void SalInstanceEntry::set_text(const OUString& rText)
{
    disable_notify_events();
    m_xEntry->SetText(rText);
    enable_notify_events();
}

OUString SalInstanceEntry::get_text() const { return m_xEntry->GetText(); }

void SalInstanceEntry::set_width_chars(int nChars) { m_xEntry->SetWidthInChars(nChars); }

int SalInstanceEntry::get_width_chars() const { return m_xEntry->GetWidthInChars(); }

void SalInstanceEntry::set_max_length(int nChars) { m_xEntry->SetMaxTextLen(nChars); }

void SalInstanceEntry::select_region(int nStartPos, int nEndPos)
{
    disable_notify_events();
    tools::Long nStart = nStartPos < 0 ? SELECTION_MAX : nStartPos;
    tools::Long nEnd = nEndPos < 0 ? SELECTION_MAX : nEndPos;
    m_xEntry->SetSelection(Selection(nStart, nEnd));
    enable_notify_events();
}

bool SalInstanceEntry::get_selection_bounds(int& rStartPos, int& rEndPos)
{
    const Selection& rSelection = m_xEntry->GetSelection();
    rStartPos = rSelection.Min();
    rEndPos = rSelection.Max();
    return rSelection.Len();
}

void SalInstanceEntry::replace_selection(const OUString& rText)
{
    m_xEntry->ReplaceSelected(rText);
}

void SalInstanceEntry::set_position(int nCursorPos)
{
    disable_notify_events();
    if (nCursorPos < 0)
        m_xEntry->SetCursorAtLast();
    else
        m_xEntry->SetSelection(Selection(nCursorPos, nCursorPos));
    enable_notify_events();
}

int SalInstanceEntry::get_position() const { return m_xEntry->GetSelection().Max(); }

void SalInstanceEntry::set_editable(bool bEditable) { m_xEntry->SetReadOnly(!bEditable); }

bool SalInstanceEntry::get_editable() const { return !m_xEntry->IsReadOnly(); }

void SalInstanceEntry::set_overwrite_mode(bool bOn) { m_xEntry->SetInsertMode(!bOn); }

bool SalInstanceEntry::get_overwrite_mode() const { return !m_xEntry->IsInsertMode(); }

void SalInstanceEntry::set_message_type(weld::EntryMessageType eType)
{
    switch (eType)
    {
        case weld::EntryMessageType::Normal:
            m_xEntry->SetForceControlBackground(false);
            m_xEntry->SetControlForeground();
            m_xEntry->SetControlBackground();
            break;
        case weld::EntryMessageType::Warning:
            // tdf#114603: enable setting the background to a different color;
            // relevant for GTK; see also #i75179#
            m_xEntry->SetForceControlBackground(true);
            m_xEntry->SetControlForeground(COL_BLACK);
            m_xEntry->SetControlBackground(COL_YELLOW);
            break;
        case weld::EntryMessageType::Error:
            // tdf#114603: enable setting the background to a different color;
            // relevant for GTK; see also #i75179#
            m_xEntry->SetForceControlBackground(true);
            m_xEntry->SetControlForeground(COL_WHITE);
            m_xEntry->SetControlBackground(0xff6563);
            break;
    }
}

void SalInstanceEntry::set_font(const vcl::Font& rFont)
{
    m_xEntry->SetControlFont(rFont);
    m_xEntry->Invalidate();
}

void SalInstanceEntry::set_font_color(const Color& rColor)
{
    if (rColor == COL_AUTO)
        m_xEntry->SetControlForeground();
    else
        m_xEntry->SetControlForeground(rColor);
}

void SalInstanceEntry::connect_cursor_position(const Link<Entry&, void>& rLink)
{
    assert(!m_aCursorPositionHdl.IsSet());
    m_xEntry->AddEventListener(LINK(this, SalInstanceEntry, CursorListener));
    weld::Entry::connect_cursor_position(rLink);
}

void SalInstanceEntry::set_placeholder_text(const OUString& rText)
{
    m_xEntry->SetPlaceholderText(rText);
}

Edit& SalInstanceEntry::getEntry() { return *m_xEntry; }

void SalInstanceEntry::fire_signal_changed() { signal_changed(); }

void SalInstanceEntry::cut_clipboard()
{
    m_xEntry->Cut();
    m_xEntry->Modify();
}

void SalInstanceEntry::copy_clipboard() { m_xEntry->Copy(); }

void SalInstanceEntry::paste_clipboard()
{
    m_xEntry->Paste();
    m_xEntry->Modify();
}

namespace
{
void set_alignment(Edit& rEntry, TxtAlign eXAlign)
{
    WinBits nAlign(0);
    switch (eXAlign)
    {
        case TxtAlign::Left:
            nAlign = WB_LEFT;
            break;
        case TxtAlign::Center:
            nAlign = WB_CENTER;
            break;
        case TxtAlign::Right:
            nAlign = WB_RIGHT;
            break;
    }
    WinBits nBits = rEntry.GetStyle();
    nBits &= ~(WB_LEFT | WB_CENTER | WB_RIGHT);
    rEntry.SetStyle(nBits | nAlign);
}
}

void SalInstanceEntry::set_alignment(TxtAlign eXAlign) { ::set_alignment(*m_xEntry, eXAlign); }

SalInstanceEntry::~SalInstanceEntry()
{
    if (m_aCursorPositionHdl.IsSet())
        m_xEntry->RemoveEventListener(LINK(this, SalInstanceEntry, CursorListener));
    m_xEntry->SetTextFilter(nullptr);
    m_xEntry->SetActivateHdl(Link<Edit&, bool>());
    m_xEntry->SetModifyHdl(Link<Edit&, void>());
}

IMPL_LINK_NOARG(SalInstanceEntry, ChangeHdl, Edit&, void) { signal_changed(); }

IMPL_LINK(SalInstanceEntry, CursorListener, VclWindowEvent&, rEvent, void)
{
    if (notify_events_disabled())
        return;
    if (rEvent.GetId() == VclEventId::EditSelectionChanged
        || rEvent.GetId() == VclEventId::EditCaretChanged)
        signal_cursor_position();
}

IMPL_LINK_NOARG(SalInstanceEntry, ActivateHdl, Edit&, bool) { return m_aActivateHdl.Call(*this); }

class SalInstanceTreeView;

static SalInstanceTreeView* g_DragSource;

namespace
{
// tdf#131581 if the TreeView is hidden then there are possibly additional
// optimizations available
class UpdateGuardIfHidden
{
private:
    SvTabListBox& m_rTreeView;
    bool m_bOrigUpdate;
    bool m_bOrigEnableInvalidate;

public:
    UpdateGuardIfHidden(SvTabListBox& rTreeView)
        : m_rTreeView(rTreeView)
        // tdf#136962 only do SetUpdateMode(false) optimization if the widget is currently hidden
        , m_bOrigUpdate(!m_rTreeView.IsVisible() && m_rTreeView.IsUpdateMode())
        // tdf#137432 only do EnableInvalidate(false) optimization if the widget is currently hidden
        , m_bOrigEnableInvalidate(!m_rTreeView.IsVisible()
                                  && m_rTreeView.GetModel()->IsEnableInvalidate())
    {
        if (m_bOrigUpdate)
            m_rTreeView.SetUpdateMode(false);
        if (m_bOrigEnableInvalidate)
            m_rTreeView.GetModel()->EnableInvalidate(false);
    }

    ~UpdateGuardIfHidden()
    {
        if (m_bOrigEnableInvalidate)
            m_rTreeView.GetModel()->EnableInvalidate(true);
        if (m_bOrigUpdate)
            m_rTreeView.SetUpdateMode(true);
    }
};
}

// Each row has a cell for the expander image, (and an optional cell for a
// checkbutton if enable_toggle_buttons has been called) which precede
// index 0
int SalInstanceTreeView::to_internal_model(int col) const
{
    if (m_xTreeView->nTreeFlags & SvTreeFlags::CHKBTN)
        ++col; // skip checkbutton column
    ++col; //skip expander column
    return col;
}

int SalInstanceTreeView::to_external_model(int col) const
{
    if (m_xTreeView->nTreeFlags & SvTreeFlags::CHKBTN)
        --col; // skip checkbutton column
    --col; //skip expander column
    return col;
}

bool SalInstanceTreeView::IsDummyEntry(SvTreeListEntry* pEntry) const
{
    return m_xTreeView->GetEntryText(pEntry).trim() == "<dummy>";
}

SvTreeListEntry* SalInstanceTreeView::GetPlaceHolderChild(SvTreeListEntry* pEntry) const
{
    if (pEntry->HasChildren())
    {
        auto pChild = m_xTreeView->FirstChild(pEntry);
        assert(pChild);
        if (IsDummyEntry(pChild))
            return pChild;
    }
    return nullptr;
}

void SalInstanceTreeView::set_font_color(SvTreeListEntry* pEntry, const Color& rColor)
{
    if (rColor == COL_AUTO)
        pEntry->SetTextColor(std::optional<Color>());
    else
        pEntry->SetTextColor(rColor);
}

void SalInstanceTreeView::AddStringItem(SvTreeListEntry* pEntry, const OUString& rStr, int nCol)
{
    auto xCell = std::make_unique<SvLBoxString>(rStr);
    if (m_aCustomRenders.count(nCol))
        xCell->SetCustomRender();
    pEntry->AddItem(std::move(xCell));
}

void SalInstanceTreeView::do_insert(const weld::TreeIter* pParent, int pos, const OUString* pStr,
                                    const OUString* pId, const OUString* pIconName,
                                    const VirtualDevice* pImageSurface, bool bChildrenOnDemand,
                                    weld::TreeIter* pRet, bool bIsSeparator)
{
    disable_notify_events();
    const SalInstanceTreeIter* pVclIter = static_cast<const SalInstanceTreeIter*>(pParent);
    SvTreeListEntry* iter = pVclIter ? pVclIter->iter : nullptr;
    auto nInsertPos = pos == -1 ? TREELIST_APPEND : pos;
    void* pUserData;
    if (pId)
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(*pId));
        pUserData = m_aUserData.back().get();
    }
    else
        pUserData = nullptr;

    SvTreeListEntry* pEntry = new SvTreeListEntry;
    if (bIsSeparator)
        pEntry->SetFlags(pEntry->GetFlags() | SvTLEntryFlags::IS_SEPARATOR);

    if (m_xTreeView->nTreeFlags & SvTreeFlags::CHKBTN)
        AddStringItem(pEntry, "", -1);

    if (pIconName || pImageSurface)
    {
        Image aImage(pIconName ? createImage(*pIconName) : createImage(*pImageSurface));
        pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(aImage, aImage, false));
    }
    else
    {
        Image aDummy;
        pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(aDummy, aDummy, false));
    }
    if (pStr)
        AddStringItem(pEntry, *pStr, 0);
    pEntry->SetUserData(pUserData);
    m_xTreeView->Insert(pEntry, iter, nInsertPos);

    if (pRet)
    {
        SalInstanceTreeIter* pVclRetIter = static_cast<SalInstanceTreeIter*>(pRet);
        pVclRetIter->iter = pEntry;
    }

    if (bChildrenOnDemand)
    {
        SvTreeListEntry* pPlaceHolder
            = m_xTreeView->InsertEntry("<dummy>", pEntry, false, 0, nullptr);
        SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pPlaceHolder);
        pViewData->SetSelectable(false);
    }

    if (bIsSeparator)
    {
        SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
        pViewData->SetSelectable(false);
    }

    enable_notify_events();
}

void SalInstanceTreeView::update_checkbutton_column_width(SvTreeListEntry* pEntry)
{
    SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
    m_xTreeView->InitViewData(pViewData, pEntry);
    m_xTreeView->CheckBoxInserted(pEntry);
}

void SalInstanceTreeView::InvalidateModelEntry(SvTreeListEntry* pEntry)
{
    if (!m_xTreeView->GetModel()->IsEnableInvalidate())
        return;
    m_xTreeView->ModelHasEntryInvalidated(pEntry);
}

void SalInstanceTreeView::do_set_toggle(SvTreeListEntry* pEntry, TriState eState, int col)
{
    assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
    // if it's the placeholder to allow a blank column, replace it now
    if (pEntry->GetItem(col).GetType() != SvLBoxItemType::Button)
    {
        SvLBoxButtonData* pData = m_bTogglesAsRadio ? &m_aRadioButtonData : &m_aCheckButtonData;
        pEntry->ReplaceItem(std::make_unique<SvLBoxButton>(pData), 0);
        update_checkbutton_column_width(pEntry);
    }
    SvLBoxItem& rItem = pEntry->GetItem(col);
    assert(dynamic_cast<SvLBoxButton*>(&rItem));
    switch (eState)
    {
        case TRISTATE_TRUE:
            static_cast<SvLBoxButton&>(rItem).SetStateChecked();
            break;
        case TRISTATE_FALSE:
            static_cast<SvLBoxButton&>(rItem).SetStateUnchecked();
            break;
        case TRISTATE_INDET:
            static_cast<SvLBoxButton&>(rItem).SetStateTristate();
            break;
    }

    InvalidateModelEntry(pEntry);
}

TriState SalInstanceTreeView::do_get_toggle(SvTreeListEntry* pEntry, int col)
{
    if (static_cast<size_t>(col) == pEntry->ItemCount())
        return TRISTATE_FALSE;

    assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
    SvLBoxItem& rItem = pEntry->GetItem(col);
    assert(dynamic_cast<SvLBoxButton*>(&rItem));
    SvLBoxButton& rToggle = static_cast<SvLBoxButton&>(rItem);
    if (rToggle.IsStateTristate())
        return TRISTATE_INDET;
    else if (rToggle.IsStateChecked())
        return TRISTATE_TRUE;
    return TRISTATE_FALSE;
}

TriState SalInstanceTreeView::get_toggle(SvTreeListEntry* pEntry, int col) const
{
    if (col == -1)
    {
        assert(m_xTreeView->nTreeFlags & SvTreeFlags::CHKBTN);
        return do_get_toggle(pEntry, 0);
    }
    col = to_internal_model(col);
    return do_get_toggle(pEntry, col);
}

void SalInstanceTreeView::set_toggle(SvTreeListEntry* pEntry, TriState eState, int col)
{
    if (col == -1)
    {
        assert(m_xTreeView->nTreeFlags & SvTreeFlags::CHKBTN);
        do_set_toggle(pEntry, eState, 0);
        return;
    }

    col = to_internal_model(col);

    // blank out missing entries
    for (int i = pEntry->ItemCount(); i < col; ++i)
        AddStringItem(pEntry, "", i - 1);

    if (static_cast<size_t>(col) == pEntry->ItemCount())
    {
        SvLBoxButtonData* pData = m_bTogglesAsRadio ? &m_aRadioButtonData : &m_aCheckButtonData;
        pEntry->AddItem(std::make_unique<SvLBoxButton>(pData));
        update_checkbutton_column_width(pEntry);
    }

    do_set_toggle(pEntry, eState, col);
}

bool SalInstanceTreeView::get_text_emphasis(SvTreeListEntry* pEntry, int col) const
{
    col = to_internal_model(col);

    assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
    SvLBoxItem& rItem = pEntry->GetItem(col);
    assert(dynamic_cast<SvLBoxString*>(&rItem));
    return static_cast<SvLBoxString&>(rItem).IsEmphasized();
}

void SalInstanceTreeView::set_header_item_width(const std::vector<int>& rWidths)
{
    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
    if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
    {
        for (size_t i = 0; i < rWidths.size(); ++i)
            pHeaderBar->SetItemSize(pHeaderBar->GetItemId(i), rWidths[i]);
    }
}

SalInstanceTreeView::SalInstanceTreeView(SvTabListBox* pTreeView, SalInstanceBuilder* pBuilder,
                                         bool bTakeOwnership)
    : SalInstanceWidget(pTreeView, pBuilder, bTakeOwnership)
    , m_xTreeView(pTreeView)
    , m_aCheckButtonData(pTreeView, false)
    , m_aRadioButtonData(pTreeView, true)
    , m_bTogglesAsRadio(false)
    , m_nSortColumn(-1)
{
    m_xTreeView->SetNodeDefaultImages();
    m_xTreeView->SetForceMakeVisible(true);
    m_xTreeView->SetSelectHdl(LINK(this, SalInstanceTreeView, SelectHdl));
    m_xTreeView->SetDeselectHdl(LINK(this, SalInstanceTreeView, DeSelectHdl));
    m_xTreeView->SetDoubleClickHdl(LINK(this, SalInstanceTreeView, DoubleClickHdl));
    m_xTreeView->SetExpandingHdl(LINK(this, SalInstanceTreeView, ExpandingHdl));
    m_xTreeView->SetPopupMenuHdl(LINK(this, SalInstanceTreeView, PopupMenuHdl));
    m_xTreeView->SetCustomRenderHdl(LINK(this, SalInstanceTreeView, CustomRenderHdl));
    m_xTreeView->SetCustomMeasureHdl(LINK(this, SalInstanceTreeView, CustomMeasureHdl));
    const tools::Long aTabPositions[] = { 0 };
    m_xTreeView->SetTabs(SAL_N_ELEMENTS(aTabPositions), aTabPositions);
    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());

    if (pHeaderBox)
    {
        if (HeaderBar* pHeaderBar = pHeaderBox->GetHeaderBar())
        {
            //make the last entry fill available space
            pHeaderBar->SetItemSize(pHeaderBar->GetItemId(pHeaderBar->GetItemCount() - 1),
                                    HEADERBAR_FULLSIZE);
            pHeaderBar->SetEndDragHdl(LINK(this, SalInstanceTreeView, EndDragHdl));
            pHeaderBar->SetSelectHdl(LINK(this, SalInstanceTreeView, HeaderBarClickedHdl));
        }
        pHeaderBox->SetEditingEntryHdl(LINK(this, SalInstanceTreeView, EditingEntryHdl));
        pHeaderBox->SetEditedEntryHdl(LINK(this, SalInstanceTreeView, EditedEntryHdl));
    }
    else
    {
        static_cast<LclTabListBox&>(*m_xTreeView)
            .SetModelChangedHdl(LINK(this, SalInstanceTreeView, ModelChangedHdl));
        static_cast<LclTabListBox&>(*m_xTreeView)
            .SetStartDragHdl(LINK(this, SalInstanceTreeView, StartDragHdl));
        static_cast<LclTabListBox&>(*m_xTreeView)
            .SetEndDragHdl(LINK(this, SalInstanceTreeView, FinishDragHdl));
        static_cast<LclTabListBox&>(*m_xTreeView)
            .SetEditingEntryHdl(LINK(this, SalInstanceTreeView, EditingEntryHdl));
        static_cast<LclTabListBox&>(*m_xTreeView)
            .SetEditedEntryHdl(LINK(this, SalInstanceTreeView, EditedEntryHdl));
    }
    m_aCheckButtonData.SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
    m_aRadioButtonData.SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
}

void SalInstanceTreeView::connect_query_tooltip(const Link<const weld::TreeIter&, OUString>& rLink)
{
    weld::TreeView::connect_query_tooltip(rLink);
    m_xTreeView->SetTooltipHdl(LINK(this, SalInstanceTreeView, TooltipHdl));
}

void SalInstanceTreeView::columns_autosize()
{
    std::vector<tools::Long> aWidths;
    m_xTreeView->getPreferredDimensions(aWidths);
    if (aWidths.size() > 2)
    {
        std::vector<int> aColWidths;
        for (size_t i = 1; i < aWidths.size() - 1; ++i)
            aColWidths.push_back(aWidths[i] - aWidths[i - 1]);
        set_column_fixed_widths(aColWidths);
    }
}

void SalInstanceTreeView::freeze()
{
    bool bIsFirstFreeze = IsFirstFreeze();
    SalInstanceWidget::freeze();
    if (bIsFirstFreeze)
    {
        m_xTreeView->SetUpdateMode(false);
        m_xTreeView->GetModel()->EnableInvalidate(false);
    }
}

void SalInstanceTreeView::thaw()
{
    bool bIsLastThaw = IsLastThaw();
    if (bIsLastThaw)
    {
        m_xTreeView->GetModel()->EnableInvalidate(true);
        m_xTreeView->SetUpdateMode(true);
    }
    SalInstanceWidget::thaw();
}

void SalInstanceTreeView::set_column_fixed_widths(const std::vector<int>& rWidths)
{
    std::vector<tools::Long> aTabPositions{ 0 };
    for (size_t i = 0; i < rWidths.size(); ++i)
        aTabPositions.push_back(aTabPositions[i] + rWidths[i]);
    m_xTreeView->SetTabs(aTabPositions.size(), aTabPositions.data(), MapUnit::MapPixel);
    set_header_item_width(rWidths);
    // call Resize to recalculate based on the new tabs
    m_xTreeView->Resize();
}

void SalInstanceTreeView::set_column_editables(const std::vector<bool>& rEditables)
{
    size_t nTabCount = rEditables.size();
    for (size_t i = 0; i < nTabCount; ++i)
        m_xTreeView->SetTabEditable(i, rEditables[i]);
}

void SalInstanceTreeView::set_centered_column(int nCol)
{
    m_xTreeView->SetTabJustify(nCol, SvTabJustify::AdjustCenter);
}

int SalInstanceTreeView::get_column_width(int nColumn) const
{
    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
    if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        return pHeaderBar->GetItemSize(pHeaderBar->GetItemId(nColumn));
    // GetTab(0) gives the position of the bitmap which is automatically inserted by the TabListBox.
    // So the first text column's width is Tab(2)-Tab(1).
    auto nWidthPixel
        = m_xTreeView->GetLogicTab(nColumn + 2) - m_xTreeView->GetLogicTab(nColumn + 1);
    nWidthPixel -= SV_TAB_BORDER;
    return nWidthPixel;
}

OUString SalInstanceTreeView::get_column_title(int nColumn) const
{
    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
    if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
    {
        return pHeaderBar->GetItemText(pHeaderBar->GetItemId(nColumn));
    }
    return OUString();
}

void SalInstanceTreeView::set_column_title(int nColumn, const OUString& rTitle)
{
    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
    if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
    {
        return pHeaderBar->SetItemText(pHeaderBar->GetItemId(nColumn), rTitle);
    }
}

void SalInstanceTreeView::set_column_custom_renderer(int nColumn, bool bEnable)
{
    assert(n_children() == 0 && "tree must be empty");
    if (bEnable)
        m_aCustomRenders.insert(nColumn);
    else
        m_aCustomRenders.erase(nColumn);
}

void SalInstanceTreeView::queue_draw()
{
    // invalidate the entries
    SvTreeList* pModel = m_xTreeView->GetModel();
    for (SvTreeListEntry* pEntry = m_xTreeView->First(); pEntry; pEntry = m_xTreeView->Next(pEntry))
        pModel->InvalidateEntry(pEntry);
}

void SalInstanceTreeView::show()
{
    if (LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get()))
        pHeaderBox->GetParent()->Show();
    SalInstanceWidget::show();
}

void SalInstanceTreeView::hide()
{
    if (LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get()))
        pHeaderBox->GetParent()->Hide();
    SalInstanceWidget::hide();
}

void SalInstanceTreeView::insert(const weld::TreeIter* pParent, int pos, const OUString* pStr,
                                 const OUString* pId, const OUString* pIconName,
                                 VirtualDevice* pImageSurface, bool bChildrenOnDemand,
                                 weld::TreeIter* pRet)
{
    do_insert(pParent, pos, pStr, pId, pIconName, pImageSurface, bChildrenOnDemand, pRet, false);
}

void SalInstanceTreeView::insert_separator(int pos, const OUString& /*rId*/)
{
    OUString sSep(VclResId(STR_SEPARATOR));
    do_insert(nullptr, pos, &sSep, nullptr, nullptr, nullptr, false, nullptr, true);
}

void SalInstanceTreeView::bulk_insert_for_each(
    int nSourceCount, const std::function<void(weld::TreeIter&, int nSourceIndex)>& func,
    const weld::TreeIter* pParent, const std::vector<int>* pFixedWidths)
{
    const SalInstanceTreeIter* pVclIter = static_cast<const SalInstanceTreeIter*>(pParent);
    SvTreeListEntry* pVclParent = pVclIter ? pVclIter->iter : nullptr;

    freeze();
    if (!pVclParent)
        clear();
    else
    {
        while (SvTreeListEntry* pChild = m_xTreeView->FirstChild(pVclParent))
            m_xTreeView->RemoveEntry(pChild);
    }
    SalInstanceTreeIter aVclIter(static_cast<SvTreeListEntry*>(nullptr));

    m_xTreeView->nTreeFlags |= SvTreeFlags::MANINS;

    if (pFixedWidths)
        set_header_item_width(*pFixedWidths);

    bool bHasAutoCheckButton(m_xTreeView->nTreeFlags & SvTreeFlags::CHKBTN);
    size_t nExtraCols = bHasAutoCheckButton ? 2 : 1;

    Image aDummy;
    for (int i = 0; i < nSourceCount; ++i)
    {
        aVclIter.iter = new SvTreeListEntry;
        if (bHasAutoCheckButton)
            AddStringItem(aVclIter.iter, "", -1);
        aVclIter.iter->AddItem(std::make_unique<SvLBoxContextBmp>(aDummy, aDummy, false));
        m_xTreeView->Insert(aVclIter.iter, pVclParent, TREELIST_APPEND);
        func(aVclIter, i);

        if (!pFixedWidths)
            continue;

        size_t nFixedWidths = std::min(pFixedWidths->size(), aVclIter.iter->ItemCount());
        for (size_t j = 0; j < nFixedWidths; ++j)
        {
            SvLBoxItem& rItem = aVclIter.iter->GetItem(j + nExtraCols);
            SvViewDataItem* pViewDataItem = m_xTreeView->GetViewDataItem(aVclIter.iter, &rItem);
            pViewDataItem->mnWidth = (*pFixedWidths)[j];
        }
    }

    m_xTreeView->nTreeFlags &= ~SvTreeFlags::MANINS;

    thaw();
}

void SalInstanceTreeView::set_font_color(int pos, const Color& rColor)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    set_font_color(pEntry, rColor);
}

void SalInstanceTreeView::set_font_color(const weld::TreeIter& rIter, const Color& rColor)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_font_color(rVclIter.iter, rColor);
}

void SalInstanceTreeView::remove(int pos)
{
    disable_notify_events();
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    m_xTreeView->RemoveEntry(pEntry);
    enable_notify_events();
}

int SalInstanceTreeView::find_text(const OUString& rText) const
{
    for (SvTreeListEntry* pEntry = m_xTreeView->First(); pEntry; pEntry = m_xTreeView->Next(pEntry))
    {
        if (SvTabListBox::GetEntryText(pEntry, 0) == rText)
            return SvTreeList::GetRelPos(pEntry);
    }
    return -1;
}

int SalInstanceTreeView::find_id(const OUString& rId) const
{
    for (SvTreeListEntry* pEntry = m_xTreeView->First(); pEntry; pEntry = m_xTreeView->Next(pEntry))
    {
        const OUString* pId = static_cast<const OUString*>(pEntry->GetUserData());
        if (!pId)
            continue;
        if (rId == *pId)
            return SvTreeList::GetRelPos(pEntry);
    }
    return -1;
}

void SalInstanceTreeView::swap(int pos1, int pos2)
{
    int min = std::min(pos1, pos2);
    int max = std::max(pos1, pos2);
    SvTreeList* pModel = m_xTreeView->GetModel();
    SvTreeListEntry* pEntry1 = pModel->GetEntry(nullptr, min);
    SvTreeListEntry* pEntry2 = pModel->GetEntry(nullptr, max);
    pModel->Move(pEntry1, pEntry2);
}

void SalInstanceTreeView::clear()
{
    disable_notify_events();
    m_xTreeView->Clear();
    m_aUserData.clear();
    enable_notify_events();
}

int SalInstanceTreeView::n_children() const
{
    return m_xTreeView->GetModel()->GetChildList(nullptr).size();
}

int SalInstanceTreeView::iter_n_children(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return m_xTreeView->GetModel()->GetChildList(rVclIter.iter).size();
}

void SalInstanceTreeView::select(int pos)
{
    assert(m_xTreeView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    if (pos == -1 || (pos == 0 && n_children() == 0))
        m_xTreeView->SelectAll(false);
    else
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        assert(pEntry && "bad pos?");
        m_xTreeView->Select(pEntry, true);
        m_xTreeView->MakeVisible(pEntry);
    }
    enable_notify_events();
}

int SalInstanceTreeView::get_cursor_index() const
{
    SvTreeListEntry* pEntry = m_xTreeView->GetCurEntry();
    if (!pEntry)
        return -1;
    return SvTreeList::GetRelPos(pEntry);
}

void SalInstanceTreeView::set_cursor(int pos)
{
    disable_notify_events();
    if (pos == -1)
        m_xTreeView->SetCurEntry(nullptr);
    else
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        m_xTreeView->SetCurEntry(pEntry);
    }
    enable_notify_events();
}

void SalInstanceTreeView::scroll_to_row(int pos)
{
    assert(m_xTreeView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    m_xTreeView->MakeVisible(pEntry);
    enable_notify_events();
}

bool SalInstanceTreeView::is_selected(int pos) const
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    return m_xTreeView->IsSelected(pEntry);
}

void SalInstanceTreeView::unselect(int pos)
{
    assert(m_xTreeView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    if (pos == -1)
        m_xTreeView->SelectAll(true);
    else
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        m_xTreeView->Select(pEntry, false);
    }
    enable_notify_events();
}

std::vector<int> SalInstanceTreeView::get_selected_rows() const
{
    std::vector<int> aRows;

    aRows.reserve(m_xTreeView->GetSelectionCount());
    for (SvTreeListEntry* pEntry = m_xTreeView->FirstSelected(); pEntry;
         pEntry = m_xTreeView->NextSelected(pEntry))
        aRows.push_back(SvTreeList::GetRelPos(pEntry));

    return aRows;
}

OUString SalInstanceTreeView::get_text(SvTreeListEntry* pEntry, int col) const
{
    if (col == -1)
        return SvTabListBox::GetEntryText(pEntry, 0);

    col = to_internal_model(col);

    if (static_cast<size_t>(col) == pEntry->ItemCount())
        return OUString();

    assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
    SvLBoxItem& rItem = pEntry->GetItem(col);
    assert(dynamic_cast<SvLBoxString*>(&rItem));
    return static_cast<SvLBoxString&>(rItem).GetText();
}

OUString SalInstanceTreeView::get_text(int pos, int col) const
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    return get_text(pEntry, col);
}

void SalInstanceTreeView::set_text(SvTreeListEntry* pEntry, const OUString& rText, int col)
{
    if (col == -1)
    {
        m_xTreeView->SetEntryText(pEntry, rText);
        return;
    }

    col = to_internal_model(col);

    // blank out missing entries
    for (int i = pEntry->ItemCount(); i < col; ++i)
        AddStringItem(pEntry, "", i - 1);

    if (static_cast<size_t>(col) == pEntry->ItemCount())
    {
        AddStringItem(pEntry, rText, col - 1);
        SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
        m_xTreeView->InitViewData(pViewData, pEntry);
    }
    else
    {
        assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxString*>(&rItem));
        static_cast<SvLBoxString&>(rItem).SetText(rText);
    }

    InvalidateModelEntry(pEntry);
}

void SalInstanceTreeView::set_text(int pos, const OUString& rText, int col)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    set_text(pEntry, rText, col);
}

void SalInstanceTreeView::set_sensitive(SvTreeListEntry* pEntry, bool bSensitive, int col)
{
    if (col == -1)
    {
        auto nFlags = pEntry->GetFlags() & ~SvTLEntryFlags::SEMITRANSPARENT;
        if (!bSensitive)
            nFlags = nFlags | SvTLEntryFlags::SEMITRANSPARENT;
        pEntry->SetFlags(nFlags);
        const sal_uInt16 nCount = pEntry->ItemCount();
        for (sal_uInt16 nCur = 0; nCur < nCount; ++nCur)
        {
            SvLBoxItem& rItem = pEntry->GetItem(nCur);
            if (rItem.GetType() == SvLBoxItemType::String)
            {
                rItem.Enable(bSensitive);
                InvalidateModelEntry(pEntry);
                break;
            }
        }
        return;
    }

    col = to_internal_model(col);

    assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
    SvLBoxItem& rItem = pEntry->GetItem(col);
    rItem.Enable(bSensitive);

    InvalidateModelEntry(pEntry);
}

void SalInstanceTreeView::set_sensitive(int pos, bool bSensitive, int col)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    set_sensitive(pEntry, bSensitive, col);
}

void SalInstanceTreeView::set_sensitive(const weld::TreeIter& rIter, bool bSensitive, int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_sensitive(rVclIter.iter, bSensitive, col);
}

TriState SalInstanceTreeView::get_toggle(int pos, int col) const
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    return get_toggle(pEntry, col);
}

TriState SalInstanceTreeView::get_toggle(const weld::TreeIter& rIter, int col) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return get_toggle(rVclIter.iter, col);
}

void SalInstanceTreeView::enable_toggle_buttons(weld::ColumnToggleType eType)
{
    assert(n_children() == 0 && "tree must be empty");
    m_bTogglesAsRadio = eType == weld::ColumnToggleType::Radio;

    SvLBoxButtonData* pData = m_bTogglesAsRadio ? &m_aRadioButtonData : &m_aCheckButtonData;
    m_xTreeView->EnableCheckButton(pData);
    // EnableCheckButton clobbered this, restore it
    pData->SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
}

void SalInstanceTreeView::set_toggle(int pos, TriState eState, int col)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    set_toggle(pEntry, eState, col);
}

void SalInstanceTreeView::set_toggle(const weld::TreeIter& rIter, TriState eState, int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_toggle(rVclIter.iter, eState, col);
}

void SalInstanceTreeView::set_clicks_to_toggle(int nToggleBehavior)
{
    m_xTreeView->SetClicksToToggle(nToggleBehavior);
}

void SalInstanceTreeView::set_extra_row_indent(const weld::TreeIter& rIter, int nIndentLevel)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    rVclIter.iter->SetExtraIndent(nIndentLevel);
}

void SalInstanceTreeView::set_text_emphasis(SvTreeListEntry* pEntry, bool bOn, int col)
{
    col = to_internal_model(col);

    assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
    SvLBoxItem& rItem = pEntry->GetItem(col);
    assert(dynamic_cast<SvLBoxString*>(&rItem));
    static_cast<SvLBoxString&>(rItem).Emphasize(bOn);

    InvalidateModelEntry(pEntry);
}

void SalInstanceTreeView::set_text_emphasis(const weld::TreeIter& rIter, bool bOn, int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_text_emphasis(rVclIter.iter, bOn, col);
}

void SalInstanceTreeView::set_text_emphasis(int pos, bool bOn, int col)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    set_text_emphasis(pEntry, bOn, col);
}

bool SalInstanceTreeView::get_text_emphasis(const weld::TreeIter& rIter, int col) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return get_text_emphasis(rVclIter.iter, col);
}

bool SalInstanceTreeView::get_text_emphasis(int pos, int col) const
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    return get_text_emphasis(pEntry, col);
}

void SalInstanceTreeView::set_text_align(SvTreeListEntry* pEntry, double fAlign, int col)
{
    col = to_internal_model(col);

    assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
    SvLBoxItem& rItem = pEntry->GetItem(col);
    assert(dynamic_cast<SvLBoxString*>(&rItem));
    static_cast<SvLBoxString&>(rItem).Align(fAlign);

    InvalidateModelEntry(pEntry);
}

void SalInstanceTreeView::set_text_align(const weld::TreeIter& rIter, double fAlign, int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_text_align(rVclIter.iter, fAlign, col);
}

void SalInstanceTreeView::set_text_align(int pos, double fAlign, int col)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    set_text_align(pEntry, fAlign, col);
}

void SalInstanceTreeView::connect_editing(const Link<const weld::TreeIter&, bool>& rStartLink,
                                          const Link<const iter_string&, bool>& rEndLink)
{
    m_xTreeView->EnableInplaceEditing(rStartLink.IsSet() || rEndLink.IsSet());
    weld::TreeView::connect_editing(rStartLink, rEndLink);
}

void SalInstanceTreeView::start_editing(const weld::TreeIter& rIter)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    m_xTreeView->EditEntry(rVclIter.iter);
}

void SalInstanceTreeView::end_editing() { m_xTreeView->EndEditing(); }

void SalInstanceTreeView::set_image(SvTreeListEntry* pEntry, const Image& rImage, int col)
{
    if (col == -1)
    {
        m_xTreeView->SetExpandedEntryBmp(pEntry, rImage);
        m_xTreeView->SetCollapsedEntryBmp(pEntry, rImage);
        return;
    }

    col = to_internal_model(col);

    // blank out missing entries
    for (int i = pEntry->ItemCount(); i < col; ++i)
        AddStringItem(pEntry, "", i - 1);

    if (static_cast<size_t>(col) == pEntry->ItemCount())
    {
        pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(rImage, rImage, false));
        SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
        m_xTreeView->InitViewData(pViewData, pEntry);
    }
    else
    {
        assert(col >= 0 && o3tl::make_unsigned(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxContextBmp*>(&rItem));
        static_cast<SvLBoxContextBmp&>(rItem).SetBitmap1(rImage);
        static_cast<SvLBoxContextBmp&>(rItem).SetBitmap2(rImage);
    }

    m_xTreeView->SetEntryHeight(pEntry);
    InvalidateModelEntry(pEntry);
}

void SalInstanceTreeView::set_image(int pos, const OUString& rImage, int col)
{
    set_image(m_xTreeView->GetEntry(nullptr, pos), createImage(rImage), col);
}

void SalInstanceTreeView::set_image(int pos,
                                    const css::uno::Reference<css::graphic::XGraphic>& rImage,
                                    int col)
{
    set_image(m_xTreeView->GetEntry(nullptr, pos), Image(rImage), col);
}

void SalInstanceTreeView::set_image(int pos, VirtualDevice& rImage, int col)
{
    set_image(m_xTreeView->GetEntry(nullptr, pos), createImage(rImage), col);
}

void SalInstanceTreeView::set_image(const weld::TreeIter& rIter, const OUString& rImage, int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_image(rVclIter.iter, createImage(rImage), col);
}

void SalInstanceTreeView::set_image(const weld::TreeIter& rIter,
                                    const css::uno::Reference<css::graphic::XGraphic>& rImage,
                                    int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_image(rVclIter.iter, Image(rImage), col);
}

void SalInstanceTreeView::set_image(const weld::TreeIter& rIter, VirtualDevice& rImage, int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_image(rVclIter.iter, createImage(rImage), col);
}

const OUString* SalInstanceTreeView::getEntryData(int index) const
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, index);
    return pEntry ? static_cast<const OUString*>(pEntry->GetUserData()) : nullptr;
}

OUString SalInstanceTreeView::get_id(int pos) const
{
    const OUString* pRet = getEntryData(pos);
    if (!pRet)
        return OUString();
    return *pRet;
}

void SalInstanceTreeView::set_id(SvTreeListEntry* pEntry, const OUString& rId)
{
    m_aUserData.emplace_back(std::make_unique<OUString>(rId));
    pEntry->SetUserData(m_aUserData.back().get());
}

void SalInstanceTreeView::set_id(int pos, const OUString& rId)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
    set_id(pEntry, rId);
}

int SalInstanceTreeView::get_selected_index() const
{
    assert(m_xTreeView->IsUpdateMode() && "don't request selection when frozen");
    SvTreeListEntry* pEntry = m_xTreeView->FirstSelected();
    if (!pEntry)
        return -1;
    return SvTreeList::GetRelPos(pEntry);
}

OUString SalInstanceTreeView::get_selected_text() const
{
    assert(m_xTreeView->IsUpdateMode() && "don't request selection when frozen");
    if (SvTreeListEntry* pEntry = m_xTreeView->FirstSelected())
        return SvTabListBox::GetEntryText(pEntry, 0);
    return OUString();
}

OUString SalInstanceTreeView::get_selected_id() const
{
    assert(m_xTreeView->IsUpdateMode() && "don't request selection when frozen");
    if (SvTreeListEntry* pEntry = m_xTreeView->FirstSelected())
    {
        if (const OUString* pStr = static_cast<const OUString*>(pEntry->GetUserData()))
            return *pStr;
    }
    return OUString();
}

std::unique_ptr<weld::TreeIter>
SalInstanceTreeView::make_iterator(const weld::TreeIter* pOrig) const
{
    return std::unique_ptr<weld::TreeIter>(
        new SalInstanceTreeIter(static_cast<const SalInstanceTreeIter*>(pOrig)));
}

void SalInstanceTreeView::copy_iterator(const weld::TreeIter& rSource, weld::TreeIter& rDest) const
{
    const SalInstanceTreeIter& rVclSource(static_cast<const SalInstanceTreeIter&>(rSource));
    SalInstanceTreeIter& rVclDest(static_cast<SalInstanceTreeIter&>(rDest));
    rVclDest.iter = rVclSource.iter;
}

bool SalInstanceTreeView::get_selected(weld::TreeIter* pIter) const
{
    SvTreeListEntry* pEntry = m_xTreeView->FirstSelected();
    auto pVclIter = static_cast<SalInstanceTreeIter*>(pIter);
    if (pVclIter)
        pVclIter->iter = pEntry;
    return pEntry != nullptr;
}

bool SalInstanceTreeView::get_cursor(weld::TreeIter* pIter) const
{
    SvTreeListEntry* pEntry = m_xTreeView->GetCurEntry();
    auto pVclIter = static_cast<SalInstanceTreeIter*>(pIter);
    if (pVclIter)
        pVclIter->iter = pEntry;
    return pEntry != nullptr;
}

void SalInstanceTreeView::set_cursor(const weld::TreeIter& rIter)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    disable_notify_events();
    m_xTreeView->SetCurEntry(rVclIter.iter);
    enable_notify_events();
}

bool SalInstanceTreeView::get_iter_first(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = m_xTreeView->GetEntry(0);
    return rVclIter.iter != nullptr;
}

bool SalInstanceTreeView::get_iter_abs_pos(weld::TreeIter& rIter, int nAbsPos) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = m_xTreeView->GetEntryAtAbsPos(nAbsPos);
    return rVclIter.iter != nullptr;
}

bool SalInstanceTreeView::iter_next_sibling(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = rVclIter.iter->NextSibling();
    return rVclIter.iter != nullptr;
}

bool SalInstanceTreeView::iter_previous_sibling(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = rVclIter.iter->PrevSibling();
    return rVclIter.iter != nullptr;
}

bool SalInstanceTreeView::iter_next(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = m_xTreeView->Next(rVclIter.iter);
    if (rVclIter.iter && IsDummyEntry(rVclIter.iter))
        return iter_next(rVclIter);
    return rVclIter.iter != nullptr;
}

bool SalInstanceTreeView::iter_previous(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = m_xTreeView->Prev(rVclIter.iter);
    if (rVclIter.iter && IsDummyEntry(rVclIter.iter))
        return iter_previous(rVclIter);
    return rVclIter.iter != nullptr;
}

bool SalInstanceTreeView::iter_children(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = m_xTreeView->FirstChild(rVclIter.iter);
    bool bRet = rVclIter.iter != nullptr;
    if (bRet)
    {
        //on-demand dummy entry doesn't count
        return !IsDummyEntry(rVclIter.iter);
    }
    return bRet;
}

bool SalInstanceTreeView::iter_parent(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = m_xTreeView->GetParent(rVclIter.iter);
    return rVclIter.iter != nullptr;
}

void SalInstanceTreeView::remove(const weld::TreeIter& rIter)
{
    disable_notify_events();
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    m_xTreeView->RemoveEntry(rVclIter.iter);
    enable_notify_events();
}

void SalInstanceTreeView::select(const weld::TreeIter& rIter)
{
    assert(m_xTreeView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    m_xTreeView->Select(rVclIter.iter, true);
    enable_notify_events();
}

void SalInstanceTreeView::scroll_to_row(const weld::TreeIter& rIter)
{
    assert(m_xTreeView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    m_xTreeView->MakeVisible(rVclIter.iter);
    enable_notify_events();
}

void SalInstanceTreeView::unselect(const weld::TreeIter& rIter)
{
    assert(m_xTreeView->IsUpdateMode() && "don't unselect when frozen");
    disable_notify_events();
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    m_xTreeView->Select(rVclIter.iter, false);
    enable_notify_events();
}

int SalInstanceTreeView::get_iter_depth(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return m_xTreeView->GetModel()->GetDepth(rVclIter.iter);
}

bool SalInstanceTreeView::iter_has_child(const weld::TreeIter& rIter) const
{
    SalInstanceTreeIter aTempCopy(static_cast<const SalInstanceTreeIter*>(&rIter));
    return iter_children(aTempCopy);
}

bool SalInstanceTreeView::get_row_expanded(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return m_xTreeView->IsExpanded(rVclIter.iter);
}

bool SalInstanceTreeView::get_children_on_demand(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    if (m_aExpandingPlaceHolderParents.count(rVclIter.iter))
        return true;
    return GetPlaceHolderChild(rVclIter.iter) != nullptr;
}

void SalInstanceTreeView::set_children_on_demand(const weld::TreeIter& rIter,
                                                 bool bChildrenOnDemand)
{
    disable_notify_events();

    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);

    SvTreeListEntry* pPlaceHolder = GetPlaceHolderChild(rVclIter.iter);

    if (bChildrenOnDemand && !pPlaceHolder)
    {
        pPlaceHolder = m_xTreeView->InsertEntry("<dummy>", rVclIter.iter, false, 0, nullptr);
        SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pPlaceHolder);
        pViewData->SetSelectable(false);
    }
    else if (!bChildrenOnDemand && pPlaceHolder)
        m_xTreeView->RemoveEntry(pPlaceHolder);

    enable_notify_events();
}

void SalInstanceTreeView::expand_row(const weld::TreeIter& rIter)
{
    assert(m_xTreeView->IsUpdateMode() && "don't expand when frozen");
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    if (!m_xTreeView->IsExpanded(rVclIter.iter) && signal_expanding(rIter))
        m_xTreeView->Expand(rVclIter.iter);
}

void SalInstanceTreeView::collapse_row(const weld::TreeIter& rIter)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    if (m_xTreeView->IsExpanded(rVclIter.iter) && signal_collapsing(rIter))
        m_xTreeView->Collapse(rVclIter.iter);
}

OUString SalInstanceTreeView::get_text(const weld::TreeIter& rIter, int col) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return get_text(rVclIter.iter, col);
}

void SalInstanceTreeView::set_text(const weld::TreeIter& rIter, const OUString& rText, int col)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_text(rVclIter.iter, rText, col);
}

OUString SalInstanceTreeView::get_id(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    const OUString* pStr = static_cast<const OUString*>(rVclIter.iter->GetUserData());
    if (pStr)
        return *pStr;
    return OUString();
}

void SalInstanceTreeView::set_id(const weld::TreeIter& rIter, const OUString& rId)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    set_id(rVclIter.iter, rId);
}

void SalInstanceTreeView::enable_drag_source(rtl::Reference<TransferDataContainer>& rHelper,
                                             sal_uInt8 eDNDConstants)
{
    m_xTreeView->SetDragHelper(rHelper, eDNDConstants);
}

void SalInstanceTreeView::set_selection_mode(SelectionMode eMode)
{
    m_xTreeView->SetSelectionMode(eMode);
}

void SalInstanceTreeView::all_foreach(const std::function<bool(weld::TreeIter&)>& func)
{
    UpdateGuardIfHidden aGuard(*m_xTreeView);

    SalInstanceTreeIter aVclIter(m_xTreeView->First());
    while (aVclIter.iter)
    {
        if (func(aVclIter))
            return;
        iter_next(aVclIter);
    }
}

void SalInstanceTreeView::selected_foreach(const std::function<bool(weld::TreeIter&)>& func)
{
    UpdateGuardIfHidden aGuard(*m_xTreeView);

    SalInstanceTreeIter aVclIter(m_xTreeView->FirstSelected());
    while (aVclIter.iter)
    {
        if (func(aVclIter))
            return;
        aVclIter.iter = m_xTreeView->NextSelected(aVclIter.iter);
    }
}

void SalInstanceTreeView::visible_foreach(const std::function<bool(weld::TreeIter&)>& func)
{
    UpdateGuardIfHidden aGuard(*m_xTreeView);

    SalInstanceTreeIter aVclIter(m_xTreeView->GetFirstEntryInView());
    while (aVclIter.iter)
    {
        if (func(aVclIter))
            return;
        aVclIter.iter = m_xTreeView->GetNextEntryInView(aVclIter.iter);
    }
}

void SalInstanceTreeView::connect_visible_range_changed(const Link<weld::TreeView&, void>& rLink)
{
    weld::TreeView::connect_visible_range_changed(rLink);
    m_xTreeView->SetScrolledHdl(LINK(this, SalInstanceTreeView, VisibleRangeChangedHdl));
}

void SalInstanceTreeView::remove_selection()
{
    disable_notify_events();
    SvTreeListEntry* pSelected = m_xTreeView->FirstSelected();
    while (pSelected)
    {
        SvTreeListEntry* pNextSelected = m_xTreeView->NextSelected(pSelected);
        m_xTreeView->RemoveEntry(pSelected);
        pSelected = pNextSelected;
    }
    enable_notify_events();
}

bool SalInstanceTreeView::is_selected(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return m_xTreeView->IsSelected(rVclIter.iter);
}

int SalInstanceTreeView::get_iter_index_in_parent(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    return SvTreeList::GetRelPos(rVclIter.iter);
}

int SalInstanceTreeView::iter_compare(const weld::TreeIter& a, const weld::TreeIter& b) const
{
    const SalInstanceTreeIter& rVclIterA = static_cast<const SalInstanceTreeIter&>(a);
    const SalInstanceTreeIter& rVclIterB = static_cast<const SalInstanceTreeIter&>(b);
    const SvTreeList* pModel = m_xTreeView->GetModel();
    auto nAbsPosA = pModel->GetAbsPos(rVclIterA.iter);
    auto nAbsPosB = pModel->GetAbsPos(rVclIterB.iter);
    if (nAbsPosA < nAbsPosB)
        return -1;
    if (nAbsPosA > nAbsPosB)
        return 1;
    return 0;
}

void SalInstanceTreeView::move_subtree(weld::TreeIter& rNode, const weld::TreeIter* pNewParent,
                                       int nIndexInNewParent)
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rNode);
    const SalInstanceTreeIter* pVclParentIter = static_cast<const SalInstanceTreeIter*>(pNewParent);
    m_xTreeView->GetModel()->Move(rVclIter.iter, pVclParentIter ? pVclParentIter->iter : nullptr,
                                  nIndexInNewParent);
}

int SalInstanceTreeView::count_selected_rows() const { return m_xTreeView->GetSelectionCount(); }

int SalInstanceTreeView::get_height_rows(int nRows) const
{
    int nHeight = m_xTreeView->GetEntryHeight() * nRows;

    sal_Int32 nLeftBorder(0), nTopBorder(0), nRightBorder(0), nBottomBorder(0);
    m_xTreeView->GetBorder(nLeftBorder, nTopBorder, nRightBorder, nBottomBorder);
    nHeight += nTopBorder + nBottomBorder;

    return nHeight;
}

void SalInstanceTreeView::make_sorted()
{
    assert(m_xTreeView->IsUpdateMode() && "don't sort when frozen");
    m_xTreeView->SetStyle(m_xTreeView->GetStyle() | WB_SORT);
    m_xTreeView->GetModel()->SetCompareHdl(LINK(this, SalInstanceTreeView, CompareHdl));
    set_sort_order(true);
}

void SalInstanceTreeView::set_sort_func(
    const std::function<int(const weld::TreeIter&, const weld::TreeIter&)>& func)
{
    weld::TreeView::set_sort_func(func);
    SvTreeList* pListModel = m_xTreeView->GetModel();
    pListModel->Resort();
}

void SalInstanceTreeView::make_unsorted()
{
    m_xTreeView->SetStyle(m_xTreeView->GetStyle() & ~WB_SORT);
}

void SalInstanceTreeView::set_sort_order(bool bAscending)
{
    SvTreeList* pListModel = m_xTreeView->GetModel();
    pListModel->SetSortMode(bAscending ? SvSortMode::Ascending : SvSortMode::Descending);
    pListModel->Resort();
}

bool SalInstanceTreeView::get_sort_order() const
{
    return m_xTreeView->GetModel()->GetSortMode() == SvSortMode::Ascending;
}

void SalInstanceTreeView::set_sort_indicator(TriState eState, int col)
{
    assert(col >= 0 && "cannot sort on expander column");

    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
    HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr;
    if (!pHeaderBar)
        return;

    sal_uInt16 nTextId = pHeaderBar->GetItemId(col);
    HeaderBarItemBits nBits = pHeaderBar->GetItemBits(nTextId);
    nBits &= ~HeaderBarItemBits::UPARROW;
    nBits &= ~HeaderBarItemBits::DOWNARROW;
    if (eState != TRISTATE_INDET)
    {
        if (eState == TRISTATE_TRUE)
            nBits |= HeaderBarItemBits::DOWNARROW;
        else
            nBits |= HeaderBarItemBits::UPARROW;
    }
    pHeaderBar->SetItemBits(nTextId, nBits);
}

TriState SalInstanceTreeView::get_sort_indicator(int col) const
{
    assert(col >= 0 && "cannot sort on expander column");

    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
    if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
    {
        sal_uInt16 nTextId = pHeaderBar->GetItemId(col);
        HeaderBarItemBits nBits = pHeaderBar->GetItemBits(nTextId);
        if (nBits & HeaderBarItemBits::DOWNARROW)
            return TRISTATE_TRUE;
        if (nBits & HeaderBarItemBits::UPARROW)
            return TRISTATE_FALSE;
    }

    return TRISTATE_INDET;
}

int SalInstanceTreeView::get_sort_column() const { return m_nSortColumn; }

void SalInstanceTreeView::set_sort_column(int nColumn)
{
    if (nColumn == -1)
    {
        make_unsorted();
        m_nSortColumn = -1;
        return;
    }

    if (nColumn != m_nSortColumn)
    {
        m_nSortColumn = nColumn;
        m_xTreeView->GetModel()->Resort();
    }
}

SvTabListBox& SalInstanceTreeView::getTreeView() { return *m_xTreeView; }

bool SalInstanceTreeView::get_dest_row_at_pos(const Point& rPos, weld::TreeIter* pResult,
                                              bool bDnDMode)
{
    LclTabListBox* pTreeView
        = !bDnDMode ? dynamic_cast<LclTabListBox*>(m_xTreeView.get()) : nullptr;
    SvTreeListEntry* pTarget
        = pTreeView ? pTreeView->GetTargetAtPoint(rPos, false) : m_xTreeView->GetDropTarget(rPos);

    if (pTarget && pResult)
    {
        SalInstanceTreeIter& rSalIter = static_cast<SalInstanceTreeIter&>(*pResult);
        rSalIter.iter = pTarget;
    }

    return pTarget != nullptr;
}

void SalInstanceTreeView::unset_drag_dest_row() { m_xTreeView->UnsetDropTarget(); }

tools::Rectangle SalInstanceTreeView::get_row_area(const weld::TreeIter& rIter) const
{
    return m_xTreeView->GetBoundingRect(static_cast<const SalInstanceTreeIter&>(rIter).iter);
}

weld::TreeView* SalInstanceTreeView::get_drag_source() const { return g_DragSource; }

int SalInstanceTreeView::vadjustment_get_value() const
{
    int nValue = -1;
    const SvTreeListEntry* pEntry = m_xTreeView->GetFirstEntryInView();
    if (pEntry)
        nValue = m_xTreeView->GetAbsPos(pEntry);
    return nValue;
}

void SalInstanceTreeView::vadjustment_set_value(int nValue)
{
    if (nValue == -1)
        return;
    bool bUpdate = m_xTreeView->IsUpdateMode();
    if (bUpdate)
        m_xTreeView->SetUpdateMode(false);
    m_xTreeView->ScrollToAbsPos(nValue);
    if (bUpdate)
        m_xTreeView->SetUpdateMode(true);
}

void SalInstanceTreeView::set_show_expanders(bool bShow)
{
    m_xTreeView->set_property("show-expanders", OUString::boolean(bShow));
}

bool SalInstanceTreeView::changed_by_hover() const { return m_xTreeView->IsSelectDueToHover(); }

SalInstanceTreeView::~SalInstanceTreeView()
{
    LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
    if (pHeaderBox)
    {
        if (HeaderBar* pHeaderBar = pHeaderBox->GetHeaderBar())
        {
            pHeaderBar->SetSelectHdl(Link<HeaderBar*, void>());
            pHeaderBar->SetEndDragHdl(Link<HeaderBar*, void>());
        }
    }
    else
    {
        static_cast<LclTabListBox&>(*m_xTreeView).SetEndDragHdl(Link<SvTreeListBox*, void>());
        static_cast<LclTabListBox&>(*m_xTreeView).SetStartDragHdl(Link<SvTreeListBox*, bool>());
        static_cast<LclTabListBox&>(*m_xTreeView).SetModelChangedHdl(Link<SvTreeListBox*, void>());
    }
    m_xTreeView->SetPopupMenuHdl(Link<const CommandEvent&, bool>());
    m_xTreeView->SetExpandingHdl(Link<SvTreeListBox*, bool>());
    m_xTreeView->SetDoubleClickHdl(Link<SvTreeListBox*, bool>());
    m_xTreeView->SetSelectHdl(Link<SvTreeListBox*, void>());
    m_xTreeView->SetDeselectHdl(Link<SvTreeListBox*, void>());
    m_xTreeView->SetScrolledHdl(Link<SvTreeListBox*, void>());
    m_xTreeView->SetTooltipHdl(Link<const HelpEvent&, bool>());
    m_xTreeView->SetCustomRenderHdl(Link<svtree_render_args, void>());
    m_xTreeView->SetCustomMeasureHdl(Link<svtree_measure_args, Size>());
}

IMPL_LINK(SalInstanceTreeView, TooltipHdl, const HelpEvent&, rHEvt, bool)
{
    if (notify_events_disabled())
        return false;
    Point aPos(m_xTreeView->ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
    SvTreeListEntry* pEntry = m_xTreeView->GetEntry(aPos);
    if (pEntry)
    {
        SalInstanceTreeIter aIter(pEntry);
        OUString aTooltip = signal_query_tooltip(aIter);
        if (aTooltip.isEmpty())
            return false;
        Size aSize(m_xTreeView->GetOutputSizePixel().Width(), m_xTreeView->GetEntryHeight());
        tools::Rectangle aScreenRect(
            m_xTreeView->OutputToScreenPixel(m_xTreeView->GetEntryPosition(pEntry)), aSize);
        Help::ShowQuickHelp(m_xTreeView, aScreenRect, aTooltip);
    }
    return true;
}

IMPL_LINK(SalInstanceTreeView, CustomRenderHdl, svtree_render_args, payload, void)
{
    vcl::RenderContext& rRenderDevice = std::get<0>(payload);
    const tools::Rectangle& rRect = std::get<1>(payload);
    const SvTreeListEntry& rEntry = std::get<2>(payload);
    const OUString* pId = static_cast<const OUString*>(rEntry.GetUserData());
    if (!pId)
        return;
    signal_custom_render(rRenderDevice, rRect, m_xTreeView->IsSelected(&rEntry), *pId);
}

IMPL_LINK(SalInstanceTreeView, CustomMeasureHdl, svtree_measure_args, payload, Size)
{
    vcl::RenderContext& rRenderDevice = payload.first;
    const SvTreeListEntry& rEntry = payload.second;
    const OUString* pId = static_cast<const OUString*>(rEntry.GetUserData());
    if (!pId)
        return Size();
    return signal_custom_get_size(rRenderDevice, *pId);
}

IMPL_LINK(SalInstanceTreeView, CompareHdl, const SvSortData&, rSortData, sal_Int32)
{
    const SvTreeListEntry* pLHS = rSortData.pLeft;
    const SvTreeListEntry* pRHS = rSortData.pRight;
    assert(pLHS && pRHS);

    if (m_aCustomSort)
        return m_aCustomSort(SalInstanceTreeIter(const_cast<SvTreeListEntry*>(pLHS)),
                             SalInstanceTreeIter(const_cast<SvTreeListEntry*>(pRHS)));

    const SvLBoxString* pLeftTextItem;
    const SvLBoxString* pRightTextItem;

    if (m_nSortColumn != -1)
    {
        size_t col = to_internal_model(m_nSortColumn);

        if (col < pLHS->ItemCount())
        {
            const SvLBoxString& rLeftTextItem
                = static_cast<const SvLBoxString&>(pLHS->GetItem(col));
            pLeftTextItem = &rLeftTextItem;
        }
        else
            pLeftTextItem = nullptr;
        if (col < pRHS->ItemCount())
        {
            const SvLBoxString& rRightTextItem
                = static_cast<const SvLBoxString&>(pRHS->GetItem(col));
            pRightTextItem = &rRightTextItem;
        }
        else
            pRightTextItem = nullptr;
    }
    else
    {
        pLeftTextItem
            = static_cast<const SvLBoxString*>(pLHS->GetFirstItem(SvLBoxItemType::String));
        pRightTextItem
            = static_cast<const SvLBoxString*>(pRHS->GetFirstItem(SvLBoxItemType::String));
    }

    return m_xTreeView->DefaultCompare(pLeftTextItem, pRightTextItem);
}

IMPL_LINK_NOARG(SalInstanceTreeView, VisibleRangeChangedHdl, SvTreeListBox*, void)
{
    if (notify_events_disabled())
        return;
    signal_visible_range_changed();
}

IMPL_LINK_NOARG(SalInstanceTreeView, ModelChangedHdl, SvTreeListBox*, void)
{
    if (notify_events_disabled())
        return;
    signal_model_changed();
}

IMPL_LINK_NOARG(SalInstanceTreeView, StartDragHdl, SvTreeListBox*, bool)
{
    bool bUnsetDragIcon(false); // ignored for vcl
    if (m_aDragBeginHdl.Call(bUnsetDragIcon))
        return true;
    g_DragSource = this;
    return false;
}

IMPL_STATIC_LINK_NOARG(SalInstanceTreeView, FinishDragHdl, SvTreeListBox*, void)
{
    g_DragSource = nullptr;
}

IMPL_LINK(SalInstanceTreeView, ToggleHdl, SvLBoxButtonData*, pData, void)
{
    SvTreeListEntry* pEntry = pData->GetActEntry();
    SvLBoxButton* pBox = pData->GetActBox();

    // tdf#122874 Select the row, calling SelectHdl, before handling
    // the toggle
    if (!m_xTreeView->IsSelected(pEntry))
    {
        m_xTreeView->SelectAll(false);
        m_xTreeView->Select(pEntry, true);
    }

    // additionally set the cursor into the row the toggled element is in
    m_xTreeView->pImpl->m_pCursor = pEntry;

    for (int i = 0, nCount = pEntry->ItemCount(); i < nCount; ++i)
    {
        SvLBoxItem& rItem = pEntry->GetItem(i);
        if (&rItem == pBox)
        {
            int nCol = to_external_model(i);
            signal_toggled(iter_col(SalInstanceTreeIter(pEntry), nCol));
            break;
        }
    }
}

IMPL_LINK_NOARG(SalInstanceTreeView, SelectHdl, SvTreeListBox*, void)
{
    if (notify_events_disabled())
        return;
    signal_changed();
}

IMPL_LINK_NOARG(SalInstanceTreeView, DeSelectHdl, SvTreeListBox*, void)
{
    if (notify_events_disabled())
        return;
    if (m_xTreeView->GetSelectionMode() == SelectionMode::Single
        && !m_xTreeView->GetHoverSelection())
        return;
    signal_changed();
}

IMPL_LINK_NOARG(SalInstanceTreeView, DoubleClickHdl, SvTreeListBox*, bool)
{
    if (notify_events_disabled())
        return false;
    return !signal_row_activated();
}

IMPL_LINK(SalInstanceTreeView, EndDragHdl, HeaderBar*, pHeaderBar, void)
{
    std::vector<tools::Long> aTabPositions{ 0 };
    for (int i = 0; i < pHeaderBar->GetItemCount() - 1; ++i)
        aTabPositions.push_back(aTabPositions[i]
                                + pHeaderBar->GetItemSize(pHeaderBar->GetItemId(i)));
    m_xTreeView->SetTabs(aTabPositions.size(), aTabPositions.data(), MapUnit::MapPixel);
}

IMPL_LINK(SalInstanceTreeView, HeaderBarClickedHdl, HeaderBar*, pHeaderBar, void)
{
    sal_uInt16 nId = pHeaderBar->GetCurItemId();
    if (!(pHeaderBar->GetItemBits(nId) & HeaderBarItemBits::CLICKABLE))
        return;
    signal_column_clicked(pHeaderBar->GetItemPos(nId));
}

IMPL_LINK_NOARG(SalInstanceTreeView, ExpandingHdl, SvTreeListBox*, bool)
{
    SvTreeListEntry* pEntry = m_xTreeView->GetHdlEntry();
    SalInstanceTreeIter aIter(pEntry);

    if (m_xTreeView->IsExpanded(pEntry))
    {
        //collapsing;
        return signal_collapsing(aIter);
    }

    // expanding

    // if there's a preexisting placeholder child, required to make this
    // potentially expandable in the first place, now we remove it
    SvTreeListEntry* pPlaceHolder = GetPlaceHolderChild(pEntry);
    if (pPlaceHolder)
    {
        m_aExpandingPlaceHolderParents.insert(pEntry);
        m_xTreeView->RemoveEntry(pPlaceHolder);
    }

    bool bRet = signal_expanding(aIter);

    if (pPlaceHolder)
    {
        //expand disallowed, restore placeholder
        if (!bRet)
        {
            pPlaceHolder = m_xTreeView->InsertEntry("<dummy>", pEntry, false, 0, nullptr);
            SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pPlaceHolder);
            pViewData->SetSelectable(false);
        }
        m_aExpandingPlaceHolderParents.erase(pEntry);
    }

    return bRet;
}

IMPL_LINK(SalInstanceTreeView, PopupMenuHdl, const CommandEvent&, rEvent, bool)
{
    return m_aPopupMenuHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceTreeView, EditingEntryHdl, SvTreeListEntry*, pEntry, bool)
{
    return signal_editing_started(SalInstanceTreeIter(pEntry));
}

IMPL_LINK(SalInstanceTreeView, EditedEntryHdl, IterString, rIterString, bool)
{
    return signal_editing_done(
        iter_string(SalInstanceTreeIter(rIterString.first), rIterString.second));
}

SalInstanceIconView::SalInstanceIconView(::IconView* pIconView, SalInstanceBuilder* pBuilder,
                                         bool bTakeOwnership)
    : SalInstanceWidget(pIconView, pBuilder, bTakeOwnership)
    , m_xIconView(pIconView)
{
    m_xIconView->SetSelectHdl(LINK(this, SalInstanceIconView, SelectHdl));
    m_xIconView->SetDeselectHdl(LINK(this, SalInstanceIconView, DeSelectHdl));
    m_xIconView->SetDoubleClickHdl(LINK(this, SalInstanceIconView, DoubleClickHdl));
    m_xIconView->SetPopupMenuHdl(LINK(this, SalInstanceIconView, CommandHdl));
}

void SalInstanceIconView::freeze()
{
    bool bIsFirstFreeze = IsFirstFreeze();
    SalInstanceWidget::freeze();
    if (bIsFirstFreeze)
        m_xIconView->SetUpdateMode(false);
}

void SalInstanceIconView::thaw()
{
    bool bIsLastThaw = IsLastThaw();
    if (bIsLastThaw)
        m_xIconView->SetUpdateMode(true);
    SalInstanceWidget::thaw();
}

void SalInstanceIconView::insert(int pos, const OUString* pStr, const OUString* pId,
                                 const OUString* pIconName, weld::TreeIter* pRet)
{
    disable_notify_events();
    auto nInsertPos = pos == -1 ? TREELIST_APPEND : pos;
    void* pUserData;
    if (pId)
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(*pId));
        pUserData = m_aUserData.back().get();
    }
    else
        pUserData = nullptr;

    SvTreeListEntry* pEntry = new SvTreeListEntry;
    if (pIconName)
    {
        Image aImage(createImage(*pIconName));
        pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(aImage, aImage, false));
    }
    else
    {
        Image aDummy;
        pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(aDummy, aDummy, false));
    }
    if (pStr)
        pEntry->AddItem(std::make_unique<SvLBoxString>(*pStr));
    pEntry->SetUserData(pUserData);
    m_xIconView->Insert(pEntry, nullptr, nInsertPos);

    if (pRet)
    {
        SalInstanceTreeIter* pVclRetIter = static_cast<SalInstanceTreeIter*>(pRet);
        pVclRetIter->iter = pEntry;
    }

    enable_notify_events();
}

void SalInstanceIconView::insert(int pos, const OUString* pStr, const OUString* pId,
                                 const VirtualDevice* pIcon, weld::TreeIter* pRet)
{
    disable_notify_events();
    auto nInsertPos = pos == -1 ? TREELIST_APPEND : pos;
    void* pUserData;
    if (pId)
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(*pId));
        pUserData = m_aUserData.back().get();
    }
    else
        pUserData = nullptr;

    SvTreeListEntry* pEntry = new SvTreeListEntry;
    if (pIcon)
    {
        const Point aNull(0, 0);
        const Size aSize = pIcon->GetOutputSizePixel();
        Image aImage(pIcon->GetBitmapEx(aNull, aSize));
        pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(aImage, aImage, false));
    }
    else
    {
        Image aDummy;
        pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(aDummy, aDummy, false));
    }
    if (pStr)
        pEntry->AddItem(std::make_unique<SvLBoxString>(*pStr));
    pEntry->SetUserData(pUserData);
    m_xIconView->Insert(pEntry, nullptr, nInsertPos);

    if (pRet)
    {
        SalInstanceTreeIter* pVclRetIter = static_cast<SalInstanceTreeIter*>(pRet);
        pVclRetIter->iter = pEntry;
    }

    enable_notify_events();
}

OUString SalInstanceIconView::get_selected_id() const
{
    assert(m_xIconView->IsUpdateMode() && "don't request selection when frozen");
    if (SvTreeListEntry* pEntry = m_xIconView->FirstSelected())
    {
        if (const OUString* pStr = static_cast<const OUString*>(pEntry->GetUserData()))
            return *pStr;
    }
    return OUString();
}

OUString SalInstanceIconView::get_selected_text() const
{
    assert(m_xIconView->IsUpdateMode() && "don't request selection when frozen");
    if (SvTreeListEntry* pEntry = m_xIconView->FirstSelected())
        return m_xIconView->GetEntryText(pEntry);
    return OUString();
}

int SalInstanceIconView::count_selected_items() const { return m_xIconView->GetSelectionCount(); }

void SalInstanceIconView::select(int pos)
{
    assert(m_xIconView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    if (pos == -1 || (pos == 0 && n_children() == 0))
        m_xIconView->SelectAll(false);
    else
    {
        SvTreeListEntry* pEntry = m_xIconView->GetEntry(nullptr, pos);
        m_xIconView->Select(pEntry, true);
        m_xIconView->MakeVisible(pEntry);
    }
    enable_notify_events();
}

void SalInstanceIconView::unselect(int pos)
{
    assert(m_xIconView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    if (pos == -1)
        m_xIconView->SelectAll(true);
    else
    {
        SvTreeListEntry* pEntry = m_xIconView->GetEntry(nullptr, pos);
        m_xIconView->Select(pEntry, false);
    }
    enable_notify_events();
}

int SalInstanceIconView::n_children() const
{
    return m_xIconView->GetModel()->GetChildList(nullptr).size();
}

std::unique_ptr<weld::TreeIter>
SalInstanceIconView::make_iterator(const weld::TreeIter* pOrig) const
{
    return std::unique_ptr<weld::TreeIter>(
        new SalInstanceTreeIter(static_cast<const SalInstanceTreeIter*>(pOrig)));
}

bool SalInstanceIconView::get_selected(weld::TreeIter* pIter) const
{
    SvTreeListEntry* pEntry = m_xIconView->FirstSelected();
    auto pVclIter = static_cast<SalInstanceTreeIter*>(pIter);
    if (pVclIter)
        pVclIter->iter = pEntry;
    return pEntry != nullptr;
}

bool SalInstanceIconView::get_cursor(weld::TreeIter* pIter) const
{
    SvTreeListEntry* pEntry = m_xIconView->GetCurEntry();
    auto pVclIter = static_cast<SalInstanceTreeIter*>(pIter);
    if (pVclIter)
        pVclIter->iter = pEntry;
    return pEntry != nullptr;
}

void SalInstanceIconView::set_cursor(const weld::TreeIter& rIter)
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    disable_notify_events();
    m_xIconView->SetCurEntry(rVclIter.iter);
    enable_notify_events();
}

bool SalInstanceIconView::get_iter_first(weld::TreeIter& rIter) const
{
    SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
    rVclIter.iter = m_xIconView->GetEntry(0);
    return rVclIter.iter != nullptr;
}

void SalInstanceIconView::scroll_to_item(const weld::TreeIter& rIter)
{
    assert(m_xIconView->IsUpdateMode()
           && "don't select when frozen, select after thaw. Note selection doesn't survive a "
              "freeze");
    disable_notify_events();
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    m_xIconView->MakeVisible(rVclIter.iter);
    enable_notify_events();
}

void SalInstanceIconView::selected_foreach(const std::function<bool(weld::TreeIter&)>& func)
{
    SalInstanceTreeIter aVclIter(m_xIconView->FirstSelected());
    while (aVclIter.iter)
    {
        if (func(aVclIter))
            return;
        aVclIter.iter = m_xIconView->NextSelected(aVclIter.iter);
    }
}

OUString SalInstanceIconView::get_id(const weld::TreeIter& rIter) const
{
    const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
    const OUString* pStr = static_cast<const OUString*>(rVclIter.iter->GetUserData());
    if (pStr)
        return *pStr;
    return OUString();
}

void SalInstanceIconView::clear()
{
    disable_notify_events();
    m_xIconView->Clear();
    m_aUserData.clear();
    enable_notify_events();
}

SalInstanceIconView::~SalInstanceIconView()
{
    m_xIconView->SetDoubleClickHdl(Link<SvTreeListBox*, bool>());
    m_xIconView->SetSelectHdl(Link<SvTreeListBox*, void>());
    m_xIconView->SetDeselectHdl(Link<SvTreeListBox*, void>());
}

IMPL_LINK_NOARG(SalInstanceIconView, SelectHdl, SvTreeListBox*, void)
{
    if (notify_events_disabled())
        return;
    signal_selection_changed();
}

IMPL_LINK_NOARG(SalInstanceIconView, DeSelectHdl, SvTreeListBox*, void)
{
    if (notify_events_disabled())
        return;
    if (m_xIconView->GetSelectionMode() == SelectionMode::Single)
        return;
    signal_selection_changed();
}

IMPL_LINK_NOARG(SalInstanceIconView, DoubleClickHdl, SvTreeListBox*, bool)
{
    if (notify_events_disabled())
        return false;
    return !signal_item_activated();
}

IMPL_LINK(SalInstanceIconView, CommandHdl, const CommandEvent&, rEvent, bool)
{
    return m_aCommandHdl.Call(rEvent);
}

double SalInstanceSpinButton::toField(int nValue) const
{
    return static_cast<double>(nValue) / Power10(get_digits());
}

int SalInstanceSpinButton::fromField(double fValue) const
{
    return FRound(fValue * Power10(get_digits()));
}

SalInstanceSpinButton::SalInstanceSpinButton(FormattedField* pButton, SalInstanceBuilder* pBuilder,
                                             bool bTakeOwnership)
    : SalInstanceEntry(pButton, pBuilder, bTakeOwnership)
    , m_xButton(pButton)
    , m_rFormatter(m_xButton->GetFormatter())
{
    m_rFormatter.SetThousandsSep(false); //off by default, MetricSpinButton enables it
    m_xButton->SetUpHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
    m_xButton->SetDownHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
    m_xButton->SetLoseFocusHdl(LINK(this, SalInstanceSpinButton, LoseFocusHdl));
    m_rFormatter.SetOutputHdl(LINK(this, SalInstanceSpinButton, OutputHdl));
    m_rFormatter.SetInputHdl(LINK(this, SalInstanceSpinButton, InputHdl));
    if (Edit* pEdit = m_xButton->GetSubEdit())
        pEdit->SetActivateHdl(LINK(this, SalInstanceSpinButton, ActivateHdl));
    else
        m_xButton->SetActivateHdl(LINK(this, SalInstanceSpinButton, ActivateHdl));
}

int SalInstanceSpinButton::get_value() const { return fromField(m_rFormatter.GetValue()); }

void SalInstanceSpinButton::set_value(int value) { m_rFormatter.SetValue(toField(value)); }

void SalInstanceSpinButton::set_range(int min, int max)
{
    m_rFormatter.SetMinValue(toField(min));
    m_rFormatter.SetMaxValue(toField(max));
}

void SalInstanceSpinButton::get_range(int& min, int& max) const
{
    min = fromField(m_rFormatter.GetMinValue());
    max = fromField(m_rFormatter.GetMaxValue());
}

void SalInstanceSpinButton::set_increments(int step, int /*page*/)
{
    m_rFormatter.SetSpinSize(toField(step));
}

void SalInstanceSpinButton::get_increments(int& step, int& page) const
{
    step = fromField(m_rFormatter.GetSpinSize());
    page = fromField(m_rFormatter.GetSpinSize());
}

void SalInstanceSpinButton::set_digits(unsigned int digits)
{
    m_rFormatter.SetDecimalDigits(digits);
}

// SpinButton may be comprised of multiple subwidgets, consider the lot as
// one thing for focus
bool SalInstanceSpinButton::has_focus() const { return m_xWidget->HasChildPathFocus(); }

//off by default for direct SpinButtons, MetricSpinButton enables it
void SalInstanceSpinButton::SetUseThousandSep() { m_rFormatter.SetThousandsSep(true); }

unsigned int SalInstanceSpinButton::get_digits() const { return m_rFormatter.GetDecimalDigits(); }

SalInstanceSpinButton::~SalInstanceSpinButton()
{
    if (Edit* pEdit = m_xButton->GetSubEdit())
        pEdit->SetActivateHdl(Link<Edit&, bool>());
    else
        m_xButton->SetActivateHdl(Link<Edit&, bool>());
    m_rFormatter.SetInputHdl(Link<sal_Int64*, TriState>());
    m_rFormatter.SetOutputHdl(Link<LinkParamNone*, bool>());
    m_xButton->SetLoseFocusHdl(Link<Control&, void>());
    m_xButton->SetDownHdl(Link<SpinField&, void>());
    m_xButton->SetUpHdl(Link<SpinField&, void>());
}

IMPL_LINK_NOARG(SalInstanceSpinButton, ActivateHdl, Edit&, bool)
{
    // tdf#122348 return pressed to end dialog
    signal_value_changed();
    return m_aActivateHdl.Call(*this);
}

IMPL_LINK_NOARG(SalInstanceSpinButton, UpDownHdl, SpinField&, void) { signal_value_changed(); }

IMPL_LINK_NOARG(SalInstanceSpinButton, LoseFocusHdl, Control&, void) { signal_value_changed(); }

IMPL_LINK_NOARG(SalInstanceSpinButton, OutputHdl, LinkParamNone*, bool) { return signal_output(); }

IMPL_LINK(SalInstanceSpinButton, InputHdl, sal_Int64*, pResult, TriState)
{
    int nResult;
    TriState eRet = signal_input(&nResult);
    if (eRet == TRISTATE_TRUE)
        *pResult = nResult;
    return eRet;
}

namespace
{
class SalInstanceFormattedSpinButton : public SalInstanceEntry,
                                       public virtual weld::FormattedSpinButton
{
private:
    VclPtr<FormattedField> m_xButton;
    weld::EntryFormatter* m_pFormatter;
    Link<weld::Widget&, void> m_aLoseFocusHdl;

    DECL_LINK(UpDownHdl, SpinField&, void);
    DECL_LINK(LoseFocusHdl, Control&, void);

public:
    SalInstanceFormattedSpinButton(FormattedField* pButton, SalInstanceBuilder* pBuilder,
                                   bool bTakeOwnership)
        : SalInstanceEntry(pButton, pBuilder, bTakeOwnership)
        , m_xButton(pButton)
        , m_pFormatter(nullptr)
    {
        m_xButton->SetUpHdl(LINK(this, SalInstanceFormattedSpinButton, UpDownHdl));
        m_xButton->SetDownHdl(LINK(this, SalInstanceFormattedSpinButton, UpDownHdl));
        m_xButton->SetLoseFocusHdl(LINK(this, SalInstanceFormattedSpinButton, LoseFocusHdl));
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
        m_xButton->SpinField::SetText(rText);
        enable_notify_events();
    }

    virtual void connect_changed(const Link<weld::Entry&, void>& rLink) override
    {
        if (!m_pFormatter) // once a formatter is set, it takes over "changed"
        {
            SalInstanceEntry::connect_changed(rLink);
            return;
        }
        m_pFormatter->connect_changed(rLink);
    }

    virtual void connect_focus_out(const Link<weld::Widget&, void>& rLink) override
    {
        if (!m_pFormatter) // once a formatter is set, it takes over "focus-out"
        {
            m_aLoseFocusHdl = rLink;
            return;
        }
        m_pFormatter->connect_focus_out(rLink);
    }

    virtual void SetFormatter(weld::EntryFormatter* pFormatter) override
    {
        m_pFormatter = pFormatter;
        m_xButton->SetFormatter(pFormatter);
    }

    virtual void sync_value_from_formatter() override
    {
        // no-op for gen
    }

    virtual void sync_range_from_formatter() override
    {
        // no-op for gen
    }

    virtual void sync_increments_from_formatter() override
    {
        // no-op for gen
    }

    virtual Formatter& GetFormatter() override { return m_xButton->GetFormatter(); }

    virtual ~SalInstanceFormattedSpinButton() override
    {
        m_xButton->SetLoseFocusHdl(Link<Control&, void>());
        m_xButton->SetDownHdl(Link<SpinField&, void>());
        m_xButton->SetUpHdl(Link<SpinField&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceFormattedSpinButton, UpDownHdl, SpinField&, void)
{
    signal_value_changed();
}

IMPL_LINK_NOARG(SalInstanceFormattedSpinButton, LoseFocusHdl, Control&, void)
{
    signal_value_changed();
    m_aLoseFocusHdl.Call(*this);
}
}

SalInstanceLabel::SalInstanceLabel(Control* pLabel, SalInstanceBuilder* pBuilder,
                                   bool bTakeOwnership)
    : SalInstanceWidget(pLabel, pBuilder, bTakeOwnership)
    , m_xLabel(pLabel)
{
}

void SalInstanceLabel::set_label(const OUString& rText) { m_xLabel->SetText(rText); }

OUString SalInstanceLabel::get_label() const { return m_xLabel->GetText(); }

void SalInstanceLabel::set_mnemonic_widget(Widget* pTarget)
{
    FixedText* pLabel = dynamic_cast<FixedText*>(m_xLabel.get());
    assert(pLabel && "can't use set_mnemonic_widget on SelectableFixedText");
    SalInstanceWidget* pTargetWidget = dynamic_cast<SalInstanceWidget*>(pTarget);
    pLabel->set_mnemonic_widget(pTargetWidget ? pTargetWidget->getWidget() : nullptr);
}

void SalInstanceLabel::set_label_type(weld::LabelType eType)
{
    switch (eType)
    {
        case weld::LabelType::Normal:
            m_xLabel->SetControlForeground();
            m_xLabel->SetControlBackground();
            break;
        case weld::LabelType::Warning:
            m_xLabel->SetControlForeground();
            m_xLabel->SetControlBackground(COL_YELLOW);
            break;
        case weld::LabelType::Error:
            m_xLabel->SetControlForeground();
            m_xLabel->SetControlBackground(
                m_xLabel->GetSettings().GetStyleSettings().GetHighlightColor());
            break;
        case weld::LabelType::Title:
            m_xLabel->SetControlForeground(
                m_xLabel->GetSettings().GetStyleSettings().GetLightColor());
            m_xLabel->SetControlBackground();
            break;
    }
}

void SalInstanceLabel::set_font_color(const Color& rColor)
{
    if (rColor != COL_AUTO)
        m_xLabel->SetControlForeground(rColor);
    else
        m_xLabel->SetControlForeground();
}

void SalInstanceLabel::set_font(const vcl::Font& rFont)
{
    m_xLabel->SetControlFont(rFont);
    m_xLabel->Invalidate();
}

std::unique_ptr<weld::Label> SalInstanceFrame::weld_label_widget() const
{
    FixedText* pLabel = dynamic_cast<FixedText*>(m_xFrame->get_label_widget());
    if (!pLabel)
        return nullptr;
    return std::make_unique<SalInstanceLabel>(pLabel, m_pBuilder, false);
}

SalInstanceTextView::SalInstanceTextView(VclMultiLineEdit* pTextView, SalInstanceBuilder* pBuilder,
                                         bool bTakeOwnership)
    : SalInstanceWidget(pTextView, pBuilder, bTakeOwnership)
    , m_xTextView(pTextView)
{
    m_xTextView->SetModifyHdl(LINK(this, SalInstanceTextView, ChangeHdl));
    ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
    m_aOrigVScrollHdl = rVertScrollBar.GetScrollHdl();
    rVertScrollBar.SetScrollHdl(LINK(this, SalInstanceTextView, VscrollHdl));
}

void SalInstanceTextView::set_text(const OUString& rText)
{
    disable_notify_events();
    m_xTextView->SetText(rText);
    enable_notify_events();
}

void SalInstanceTextView::replace_selection(const OUString& rText)
{
    disable_notify_events();
    m_xTextView->ReplaceSelected(rText);
    enable_notify_events();
}

OUString SalInstanceTextView::get_text() const { return m_xTextView->GetText(); }

bool SalInstanceTextView::get_selection_bounds(int& rStartPos, int& rEndPos)
{
    const Selection& rSelection = m_xTextView->GetSelection();
    rStartPos = rSelection.Min();
    rEndPos = rSelection.Max();
    return rSelection.Len();
}

void SalInstanceTextView::select_region(int nStartPos, int nEndPos)
{
    disable_notify_events();
    tools::Long nStart = nStartPos < 0 ? SELECTION_MAX : nStartPos;
    tools::Long nEnd = nEndPos < 0 ? SELECTION_MAX : nEndPos;
    m_xTextView->SetSelection(Selection(nStart, nEnd));
    enable_notify_events();
}

void SalInstanceTextView::set_editable(bool bEditable) { m_xTextView->SetReadOnly(!bEditable); }
bool SalInstanceTextView::get_editable() const { return !m_xTextView->IsReadOnly(); }
void SalInstanceTextView::set_max_length(int nChars) { m_xTextView->SetMaxTextLen(nChars); }

void SalInstanceTextView::set_monospace(bool bMonospace)
{
    vcl::Font aOrigFont = m_xTextView->GetControlFont();
    vcl::Font aFont;
    if (bMonospace)
        aFont
            = OutputDevice::GetDefaultFont(DefaultFontType::UI_FIXED, LANGUAGE_DONTKNOW,
                                           GetDefaultFontFlags::OnlyOne, m_xTextView->GetOutDev());
    else
        aFont = Application::GetSettings().GetStyleSettings().GetFieldFont();
    aFont.SetFontHeight(aOrigFont.GetFontHeight());
    set_font(aFont);
}

void SalInstanceTextView::set_font_color(const Color& rColor)
{
    if (rColor != COL_AUTO)
        m_xTextView->SetControlForeground(rColor);
    else
        m_xTextView->SetControlForeground();
}

void SalInstanceTextView::set_font(const vcl::Font& rFont)
{
    m_xTextView->SetFont(rFont);
    m_xTextView->SetControlFont(rFont);
    m_xTextView->Invalidate();
}

void SalInstanceTextView::connect_cursor_position(const Link<TextView&, void>& rLink)
{
    assert(!m_aCursorPositionHdl.IsSet());
    m_xTextView->AddEventListener(LINK(this, SalInstanceTextView, CursorListener));
    weld::TextView::connect_cursor_position(rLink);
}

bool SalInstanceTextView::can_move_cursor_with_up() const
{
    bool bNoSelection = !m_xTextView->GetSelection();
    return !bNoSelection || m_xTextView->CanUp();
}

bool SalInstanceTextView::can_move_cursor_with_down() const
{
    bool bNoSelection = !m_xTextView->GetSelection();
    return !bNoSelection || m_xTextView->CanDown();
}

void SalInstanceTextView::cut_clipboard() { m_xTextView->Cut(); }

void SalInstanceTextView::copy_clipboard() { m_xTextView->Copy(); }

void SalInstanceTextView::paste_clipboard() { m_xTextView->Paste(); }

void SalInstanceTextView::set_alignment(TxtAlign eXAlign)
{
    ::set_alignment(*m_xTextView, eXAlign);
}

int SalInstanceTextView::vadjustment_get_value() const
{
    ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
    return rVertScrollBar.GetThumbPos();
}

void SalInstanceTextView::vadjustment_set_value(int value)
{
    ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
    rVertScrollBar.SetThumbPos(value);
    m_aOrigVScrollHdl.Call(&rVertScrollBar);
}

int SalInstanceTextView::vadjustment_get_upper() const
{
    ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
    return rVertScrollBar.GetRangeMax();
}

int SalInstanceTextView::vadjustment_get_lower() const
{
    ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
    return rVertScrollBar.GetRangeMin();
}

int SalInstanceTextView::vadjustment_get_page_size() const
{
    ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
    return rVertScrollBar.GetVisibleSize();
}

bool SalInstanceTextView::has_focus() const { return m_xTextView->HasChildPathFocus(); }

SalInstanceTextView::~SalInstanceTextView()
{
    if (!m_xTextView->isDisposed())
    {
        if (m_aCursorPositionHdl.IsSet())
            m_xTextView->RemoveEventListener(LINK(this, SalInstanceTextView, CursorListener));
        m_xTextView->SetModifyHdl(Link<Edit&, void>());
        ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
        rVertScrollBar.SetScrollHdl(m_aOrigVScrollHdl);
    }
}

IMPL_LINK(SalInstanceTextView, VscrollHdl, ScrollBar*, pScrollBar, void)
{
    signal_vadjustment_changed();
    m_aOrigVScrollHdl.Call(pScrollBar);
}

IMPL_LINK_NOARG(SalInstanceTextView, ChangeHdl, Edit&, void) { signal_changed(); }

IMPL_LINK(SalInstanceTextView, CursorListener, VclWindowEvent&, rEvent, void)
{
    if (notify_events_disabled())
        return;
    if (rEvent.GetId() == VclEventId::EditSelectionChanged
        || rEvent.GetId() == VclEventId::EditCaretChanged)
        signal_cursor_position();
}

SalInstanceExpander::SalInstanceExpander(VclExpander* pExpander, SalInstanceBuilder* pBuilder,
                                         bool bTakeOwnership)
    : SalInstanceWidget(pExpander, pBuilder, bTakeOwnership)
    , m_xExpander(pExpander)
{
    m_xExpander->SetExpandedHdl(LINK(this, SalInstanceExpander, ExpandedHdl));
}

void SalInstanceExpander::set_label(const OUString& rText) { m_xExpander->set_label(rText); }

OUString SalInstanceExpander::get_label() const { return m_xExpander->get_label(); }

bool SalInstanceExpander::get_expanded() const { return m_xExpander->get_expanded(); }

void SalInstanceExpander::set_expanded(bool bExpand) { m_xExpander->set_expanded(bExpand); }

bool SalInstanceExpander::has_focus() const
{
    return m_xExpander->get_label_widget()->HasFocus() || SalInstanceWidget::has_focus();
}

void SalInstanceExpander::grab_focus() { return m_xExpander->get_label_widget()->GrabFocus(); }

SalInstanceExpander::~SalInstanceExpander()
{
    m_xExpander->SetExpandedHdl(Link<VclExpander&, void>());
}

IMPL_LINK_NOARG(SalInstanceExpander, ExpandedHdl, VclExpander&, void) { signal_expanded(); }

// SalInstanceWidget has a generic listener for all these
// events, ignore the ones we have specializations for
// in VclDrawingArea
void SalInstanceDrawingArea::HandleEventListener(VclWindowEvent& rEvent)
{
    if (rEvent.GetId() == VclEventId::WindowResize)
        return;
    SalInstanceWidget::HandleEventListener(rEvent);
}

void SalInstanceDrawingArea::HandleMouseEventListener(VclWindowEvent& rEvent)
{
    if (rEvent.GetId() == VclEventId::WindowMouseButtonDown
        || rEvent.GetId() == VclEventId::WindowMouseButtonUp
        || rEvent.GetId() == VclEventId::WindowMouseMove)
    {
        return;
    }
    SalInstanceWidget::HandleMouseEventListener(rEvent);
}

bool SalInstanceDrawingArea::HandleKeyEventListener(VclWindowEvent& /*rEvent*/) { return false; }

SalInstanceDrawingArea::SalInstanceDrawingArea(VclDrawingArea* pDrawingArea,
                                               SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                                               FactoryFunction pUITestFactoryFunction,
                                               void* pUserData, bool bTakeOwnership)
    : SalInstanceWidget(pDrawingArea, pBuilder, bTakeOwnership)
    , m_xDrawingArea(pDrawingArea)
{
    m_xDrawingArea->SetAccessible(rAlly);
    m_xDrawingArea->SetUITestFactory(std::move(pUITestFactoryFunction), pUserData);
    m_xDrawingArea->SetPaintHdl(LINK(this, SalInstanceDrawingArea, PaintHdl));
    m_xDrawingArea->SetResizeHdl(LINK(this, SalInstanceDrawingArea, ResizeHdl));
    m_xDrawingArea->SetMousePressHdl(LINK(this, SalInstanceDrawingArea, MousePressHdl));
    m_xDrawingArea->SetMouseMoveHdl(LINK(this, SalInstanceDrawingArea, MouseMoveHdl));
    m_xDrawingArea->SetMouseReleaseHdl(LINK(this, SalInstanceDrawingArea, MouseReleaseHdl));
    m_xDrawingArea->SetKeyPressHdl(LINK(this, SalInstanceDrawingArea, KeyPressHdl));
    m_xDrawingArea->SetKeyReleaseHdl(LINK(this, SalInstanceDrawingArea, KeyReleaseHdl));
    m_xDrawingArea->SetStyleUpdatedHdl(LINK(this, SalInstanceDrawingArea, StyleUpdatedHdl));
    m_xDrawingArea->SetCommandHdl(LINK(this, SalInstanceDrawingArea, CommandHdl));
    m_xDrawingArea->SetQueryTooltipHdl(LINK(this, SalInstanceDrawingArea, QueryTooltipHdl));
    m_xDrawingArea->SetGetSurroundingHdl(LINK(this, SalInstanceDrawingArea, GetSurroundingHdl));
    m_xDrawingArea->SetDeleteSurroundingHdl(
        LINK(this, SalInstanceDrawingArea, DeleteSurroundingHdl));
    m_xDrawingArea->SetStartDragHdl(LINK(this, SalInstanceDrawingArea, StartDragHdl));
}

void SalInstanceDrawingArea::queue_draw() { m_xDrawingArea->Invalidate(); }

void SalInstanceDrawingArea::queue_draw_area(int x, int y, int width, int height)
{
    m_xDrawingArea->Invalidate(tools::Rectangle(Point(x, y), Size(width, height)));
}

void SalInstanceDrawingArea::connect_size_allocate(const Link<const Size&, void>& rLink)
{
    weld::Widget::connect_size_allocate(rLink);
}

void SalInstanceDrawingArea::connect_key_press(const Link<const KeyEvent&, bool>& rLink)
{
    weld::Widget::connect_key_press(rLink);
}

void SalInstanceDrawingArea::connect_key_release(const Link<const KeyEvent&, bool>& rLink)
{
    weld::Widget::connect_key_release(rLink);
}

void SalInstanceDrawingArea::set_cursor(PointerStyle ePointerStyle)
{
    m_xDrawingArea->SetPointer(ePointerStyle);
}

void SalInstanceDrawingArea::set_input_context(const InputContext& rInputContext)
{
    m_xDrawingArea->SetInputContext(rInputContext);
}

void SalInstanceDrawingArea::im_context_set_cursor_location(const tools::Rectangle& rCursorRect,
                                                            int nExtTextInputWidth)
{
    tools::Rectangle aCursorRect = m_xDrawingArea->PixelToLogic(rCursorRect);
    m_xDrawingArea->SetCursorRect(
        &aCursorRect, m_xDrawingArea->PixelToLogic(Size(nExtTextInputWidth, 0)).Width());
}

a11yref SalInstanceDrawingArea::get_accessible_parent()
{
    vcl::Window* pParent = m_xDrawingArea->GetParent();
    if (pParent)
        return pParent->GetAccessible();
    return css::uno::Reference<css::accessibility::XAccessible>();
}

a11yrelationset SalInstanceDrawingArea::get_accessible_relation_set()
{
    rtl::Reference<utl::AccessibleRelationSetHelper> pRelationSetHelper
        = new utl::AccessibleRelationSetHelper;
    vcl::Window* pWindow = m_xDrawingArea.get();
    if (pWindow)
    {
        vcl::Window* pLabeledBy = pWindow->GetAccessibleRelationLabeledBy();
        if (pLabeledBy && pLabeledBy != pWindow)
        {
            css::uno::Sequence<css::uno::Reference<css::uno::XInterface>> aSequence{
                pLabeledBy->GetAccessible()
            };
            pRelationSetHelper->AddRelation(css::accessibility::AccessibleRelation(
                css::accessibility::AccessibleRelationType::LABELED_BY, aSequence));
        }
        vcl::Window* pMemberOf = pWindow->GetAccessibleRelationMemberOf();
        if (pMemberOf && pMemberOf != pWindow)
        {
            css::uno::Sequence<css::uno::Reference<css::uno::XInterface>> aSequence{
                pMemberOf->GetAccessible()
            };
            pRelationSetHelper->AddRelation(css::accessibility::AccessibleRelation(
                css::accessibility::AccessibleRelationType::MEMBER_OF, aSequence));
        }
    }
    return pRelationSetHelper;
}

Point SalInstanceDrawingArea::get_accessible_location_on_screen()
{
    return m_xDrawingArea->OutputToAbsoluteScreenPixel(Point());
}

Point SalInstanceDrawingArea::get_pointer_position() const
{
    return m_xDrawingArea->GetPointerPosPixel();
}

void SalInstanceDrawingArea::enable_drag_source(rtl::Reference<TransferDataContainer>& rHelper,
                                                sal_uInt8 eDNDConstants)
{
    m_xDrawingArea->SetDragHelper(rHelper, eDNDConstants);
}

SalInstanceDrawingArea::~SalInstanceDrawingArea()
{
    m_xDrawingArea->SetDeleteSurroundingHdl(Link<const Selection&, bool>());
    m_xDrawingArea->SetGetSurroundingHdl(Link<OUString&, int>());
    m_xDrawingArea->SetQueryTooltipHdl(Link<tools::Rectangle&, OUString>());
    m_xDrawingArea->SetCommandHdl(Link<const CommandEvent&, bool>());
    m_xDrawingArea->SetStyleUpdatedHdl(Link<VclDrawingArea&, void>());
    m_xDrawingArea->SetMousePressHdl(Link<const MouseEvent&, bool>());
    m_xDrawingArea->SetMouseMoveHdl(Link<const MouseEvent&, bool>());
    m_xDrawingArea->SetMouseReleaseHdl(Link<const MouseEvent&, bool>());
    m_xDrawingArea->SetKeyPressHdl(Link<const KeyEvent&, bool>());
    m_xDrawingArea->SetKeyReleaseHdl(Link<const KeyEvent&, bool>());
    m_xDrawingArea->SetResizeHdl(Link<const Size&, void>());
    m_xDrawingArea->SetPaintHdl(
        Link<std::pair<vcl::RenderContext&, const tools::Rectangle&>, void>());
}

OutputDevice& SalInstanceDrawingArea::get_ref_device() { return *m_xDrawingArea->GetOutDev(); }

void SalInstanceDrawingArea::click(const Point& rPos)
{
    MouseEvent aEvent(rPos, 1, MouseEventModifiers::NONE, MOUSE_LEFT, 0);
    m_xDrawingArea->MouseButtonDown(aEvent);
    m_xDrawingArea->MouseButtonUp(aEvent);
}

IMPL_LINK(SalInstanceDrawingArea, PaintHdl, target_and_area, aPayload, void)
{
    m_aDrawHdl.Call(aPayload);
    tools::Rectangle aFocusRect(m_aGetFocusRectHdl.Call(*this));
    if (!aFocusRect.IsEmpty())
        InvertFocusRect(aPayload.first, aFocusRect);
}

IMPL_LINK(SalInstanceDrawingArea, ResizeHdl, const Size&, rSize, void)
{
    m_aSizeAllocateHdl.Call(rSize);
}

IMPL_LINK(SalInstanceDrawingArea, MousePressHdl, const MouseEvent&, rEvent, bool)
{
    return m_aMousePressHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, MouseMoveHdl, const MouseEvent&, rEvent, bool)
{
    return m_aMouseMotionHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, MouseReleaseHdl, const MouseEvent&, rEvent, bool)
{
    return m_aMouseReleaseHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, KeyPressHdl, const KeyEvent&, rEvent, bool)
{
    return m_aKeyPressHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, KeyReleaseHdl, const KeyEvent&, rEvent, bool)
{
    return m_aKeyReleaseHdl.Call(rEvent);
}

IMPL_LINK_NOARG(SalInstanceDrawingArea, StyleUpdatedHdl, VclDrawingArea&, void)
{
    m_aStyleUpdatedHdl.Call(*this);
}

IMPL_LINK(SalInstanceDrawingArea, CommandHdl, const CommandEvent&, rEvent, bool)
{
    return m_aCommandHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, GetSurroundingHdl, OUString&, rSurrounding, int)
{
    return m_aGetSurroundingHdl.Call(rSurrounding);
}

IMPL_LINK(SalInstanceDrawingArea, DeleteSurroundingHdl, const Selection&, rSelection, bool)
{
    return m_aDeleteSurroundingHdl.Call(rSelection);
}

IMPL_LINK(SalInstanceDrawingArea, QueryTooltipHdl, tools::Rectangle&, rHelpArea, OUString)
{
    return m_aQueryTooltipHdl.Call(rHelpArea);
}

IMPL_LINK_NOARG(SalInstanceDrawingArea, StartDragHdl, VclDrawingArea*, bool)
{
    if (m_aDragBeginHdl.Call(*this))
        return true;
    return false;
}

SalInstanceComboBoxWithoutEdit::SalInstanceComboBoxWithoutEdit(ListBox* pListBox,
                                                               SalInstanceBuilder* pBuilder,
                                                               bool bTakeOwnership)
    : SalInstanceComboBox<ListBox>(pListBox, pBuilder, bTakeOwnership)
{
    m_xComboBox->SetSelectHdl(LINK(this, SalInstanceComboBoxWithoutEdit, SelectHdl));
}

OUString SalInstanceComboBoxWithoutEdit::get_active_text() const
{
    return m_xComboBox->GetSelectedEntry();
}

void SalInstanceComboBoxWithoutEdit::remove(int pos) { m_xComboBox->RemoveEntry(pos); }

void SalInstanceComboBoxWithoutEdit::insert(int pos, const OUString& rStr, const OUString* pId,
                                            const OUString* pIconName, VirtualDevice* pImageSurface)
{
    auto nInsertPos = pos == -1 ? COMBOBOX_APPEND : pos;
    sal_Int32 nInsertedAt;
    if (!pIconName && !pImageSurface)
        nInsertedAt = m_xComboBox->InsertEntry(rStr, nInsertPos);
    else if (pIconName)
        nInsertedAt = m_xComboBox->InsertEntry(rStr, createImage(*pIconName), nInsertPos);
    else
        nInsertedAt = m_xComboBox->InsertEntry(rStr, createImage(*pImageSurface), nInsertPos);
    if (pId)
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(*pId));
        m_xComboBox->SetEntryData(nInsertedAt, m_aUserData.back().get());
    }
}

void SalInstanceComboBoxWithoutEdit::insert_separator(int pos, const OUString& /*rId*/)
{
    auto nInsertPos = pos == -1 ? m_xComboBox->GetEntryCount() : pos;
    m_xComboBox->AddSeparator(nInsertPos - 1);
}

bool SalInstanceComboBoxWithoutEdit::has_entry() const { return false; }

bool SalInstanceComboBoxWithoutEdit::changed_by_direct_pick() const { return true; }

void SalInstanceComboBoxWithoutEdit::set_entry_message_type(weld::EntryMessageType /*eType*/)
{
    assert(false);
}

void SalInstanceComboBoxWithoutEdit::set_entry_text(const OUString& /*rText*/) { assert(false); }

void SalInstanceComboBoxWithoutEdit::select_entry_region(int /*nStartPos*/, int /*nEndPos*/)
{
    assert(false);
}

bool SalInstanceComboBoxWithoutEdit::get_entry_selection_bounds(int& /*rStartPos*/,
                                                                int& /*rEndPos*/)
{
    assert(false);
    return false;
}

void SalInstanceComboBoxWithoutEdit::set_entry_width_chars(int /*nChars*/) { assert(false); }

void SalInstanceComboBoxWithoutEdit::set_entry_max_length(int /*nChars*/) { assert(false); }

void SalInstanceComboBoxWithoutEdit::set_entry_completion(bool, bool) { assert(false); }

void SalInstanceComboBoxWithoutEdit::set_entry_placeholder_text(const OUString&) { assert(false); }

void SalInstanceComboBoxWithoutEdit::set_entry_editable(bool /*bEditable*/) { assert(false); }

void SalInstanceComboBoxWithoutEdit::cut_entry_clipboard() { assert(false); }

void SalInstanceComboBoxWithoutEdit::copy_entry_clipboard() { assert(false); }

void SalInstanceComboBoxWithoutEdit::paste_entry_clipboard() { assert(false); }

void SalInstanceComboBoxWithoutEdit::set_font(const vcl::Font& rFont)
{
    m_xComboBox->SetControlFont(rFont);
    m_xComboBox->Invalidate();
}

void SalInstanceComboBoxWithoutEdit::set_entry_font(const vcl::Font&) { assert(false); }

vcl::Font SalInstanceComboBoxWithoutEdit::get_entry_font()
{
    assert(false);
    return vcl::Font();
}

void SalInstanceComboBoxWithoutEdit::set_custom_renderer(bool /*bOn*/)
{
    assert(false && "not implemented");
}

int SalInstanceComboBoxWithoutEdit::get_max_mru_count() const
{
    assert(false && "not implemented");
    return 0;
}

void SalInstanceComboBoxWithoutEdit::set_max_mru_count(int) { assert(false && "not implemented"); }

OUString SalInstanceComboBoxWithoutEdit::get_mru_entries() const
{
    assert(false && "not implemented");
    return OUString();
}

void SalInstanceComboBoxWithoutEdit::set_mru_entries(const OUString&)
{
    assert(false && "not implemented");
}

void SalInstanceComboBoxWithoutEdit::HandleEventListener(VclWindowEvent& rEvent)
{
    CallHandleEventListener(rEvent);
}

SalInstanceComboBoxWithoutEdit::~SalInstanceComboBoxWithoutEdit()
{
    m_xComboBox->SetSelectHdl(Link<ListBox&, void>());
}

IMPL_LINK_NOARG(SalInstanceComboBoxWithoutEdit, SelectHdl, ListBox&, void)
{
    return signal_changed();
}

SalInstanceComboBoxWithEdit::SalInstanceComboBoxWithEdit(::ComboBox* pComboBox,
                                                         SalInstanceBuilder* pBuilder,
                                                         bool bTakeOwnership)
    : SalInstanceComboBox<::ComboBox>(pComboBox, pBuilder, bTakeOwnership)
    , m_aTextFilter(m_aEntryInsertTextHdl)
    , m_bInSelect(false)
{
    m_xComboBox->SetModifyHdl(LINK(this, SalInstanceComboBoxWithEdit, ChangeHdl));
    m_xComboBox->SetSelectHdl(LINK(this, SalInstanceComboBoxWithEdit, SelectHdl));
    m_xComboBox->SetEntryActivateHdl(LINK(this, SalInstanceComboBoxWithEdit, EntryActivateHdl));
    m_xComboBox->SetTextFilter(&m_aTextFilter);
}

bool SalInstanceComboBoxWithEdit::has_entry() const { return true; }

bool SalInstanceComboBoxWithEdit::changed_by_direct_pick() const
{
    return m_bInSelect && !m_xComboBox->IsModifyByKeyboard() && !m_xComboBox->IsTravelSelect();
}

void SalInstanceComboBoxWithEdit::set_entry_message_type(weld::EntryMessageType eType)
{
    switch (eType)
    {
        case weld::EntryMessageType::Normal:
            m_xComboBox->SetControlForeground();
            break;
        case weld::EntryMessageType::Warning:
            m_xComboBox->SetControlForeground(COL_YELLOW);
            break;
        case weld::EntryMessageType::Error:
            m_xComboBox->SetControlForeground(Color(0xf0, 0, 0));
            break;
    }
}

OUString SalInstanceComboBoxWithEdit::get_active_text() const { return m_xComboBox->GetText(); }

void SalInstanceComboBoxWithEdit::remove(int pos) { m_xComboBox->RemoveEntryAt(pos); }

void SalInstanceComboBoxWithEdit::insert(int pos, const OUString& rStr, const OUString* pId,
                                         const OUString* pIconName, VirtualDevice* pImageSurface)
{
    auto nInsertPos = pos == -1 ? COMBOBOX_APPEND : pos;
    sal_Int32 nInsertedAt;
    if (!pIconName && !pImageSurface)
        nInsertedAt = m_xComboBox->InsertEntry(rStr, nInsertPos);
    else if (pIconName)
        nInsertedAt = m_xComboBox->InsertEntryWithImage(rStr, createImage(*pIconName), nInsertPos);
    else
        nInsertedAt
            = m_xComboBox->InsertEntryWithImage(rStr, createImage(*pImageSurface), nInsertPos);
    if (pId)
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(*pId));
        m_xComboBox->SetEntryData(nInsertedAt, m_aUserData.back().get());
    }
}

void SalInstanceComboBoxWithEdit::insert_separator(int pos, const OUString& /*rId*/)
{
    auto nInsertPos = pos == -1 ? m_xComboBox->GetEntryCount() : pos;
    m_xComboBox->AddSeparator(nInsertPos - 1);
}

void SalInstanceComboBoxWithEdit::set_entry_text(const OUString& rText)
{
    m_xComboBox->SetText(rText);
}

void SalInstanceComboBoxWithEdit::set_entry_width_chars(int nChars)
{
    m_xComboBox->SetWidthInChars(nChars);
}

void SalInstanceComboBoxWithEdit::set_entry_max_length(int nChars)
{
    m_xComboBox->SetMaxTextLen(nChars);
}

void SalInstanceComboBoxWithEdit::set_entry_completion(bool bEnable, bool bCaseSensitive)
{
    m_xComboBox->EnableAutocomplete(bEnable, bCaseSensitive);
}

void SalInstanceComboBoxWithEdit::set_entry_placeholder_text(const OUString& rText)
{
    m_xComboBox->SetPlaceholderText(rText);
}

void SalInstanceComboBoxWithEdit::set_entry_editable(bool bEditable)
{
    m_xComboBox->SetReadOnly(!bEditable);
}

void SalInstanceComboBoxWithEdit::cut_entry_clipboard() { m_xComboBox->Cut(); }

void SalInstanceComboBoxWithEdit::copy_entry_clipboard() { m_xComboBox->Copy(); }

void SalInstanceComboBoxWithEdit::paste_entry_clipboard() { m_xComboBox->Paste(); }

void SalInstanceComboBoxWithEdit::select_entry_region(int nStartPos, int nEndPos)
{
    m_xComboBox->SetSelection(Selection(nStartPos, nEndPos < 0 ? SELECTION_MAX : nEndPos));
}

bool SalInstanceComboBoxWithEdit::get_entry_selection_bounds(int& rStartPos, int& rEndPos)
{
    const Selection& rSelection = m_xComboBox->GetSelection();
    rStartPos = rSelection.Min();
    rEndPos = rSelection.Max();
    return rSelection.Len();
}

void SalInstanceComboBoxWithEdit::set_font(const vcl::Font& rFont)
{
    m_xComboBox->SetControlFont(rFont);
    m_xComboBox->Invalidate();
}

void SalInstanceComboBoxWithEdit::set_entry_font(const vcl::Font& rFont)
{
    Edit* pEdit = m_xComboBox->GetSubEdit();
    assert(pEdit);
    pEdit->SetControlFont(rFont); // tdf#134601 set it as control font to take effect properly
    pEdit->Invalidate();
}

vcl::Font SalInstanceComboBoxWithEdit::get_entry_font()
{
    Edit* pEdit = m_xComboBox->GetSubEdit();
    assert(pEdit);
    return pEdit->GetPointFont(*pEdit->GetOutDev());
}

void SalInstanceComboBoxWithEdit::set_custom_renderer(bool bOn)
{
    if (m_xComboBox->IsUserDrawEnabled() == bOn)
        return;

    auto nOldEntryHeight = m_xComboBox->GetDropDownEntryHeight();
    auto nDropDownLineCount = m_xComboBox->GetDropDownLineCount();

    m_xComboBox->EnableUserDraw(bOn);
    if (bOn)
        m_xComboBox->SetUserDrawHdl(LINK(this, SalInstanceComboBoxWithEdit, UserDrawHdl));
    else
        m_xComboBox->SetUserDrawHdl(Link<UserDrawEvent*, void>());

    // adjust the line count to fit approx the height it would have been before
    // changing the renderer
    auto nNewEntryHeight = m_xComboBox->GetDropDownEntryHeight();
    double fRatio = nOldEntryHeight / static_cast<double>(nNewEntryHeight);
    m_xComboBox->SetDropDownLineCount(nDropDownLineCount * fRatio);
}

int SalInstanceComboBoxWithEdit::get_max_mru_count() const { return m_xComboBox->GetMaxMRUCount(); }

void SalInstanceComboBoxWithEdit::set_max_mru_count(int nCount)
{
    return m_xComboBox->SetMaxMRUCount(nCount);
}

OUString SalInstanceComboBoxWithEdit::get_mru_entries() const
{
    return m_xComboBox->GetMRUEntries();
}

void SalInstanceComboBoxWithEdit::set_mru_entries(const OUString& rEntries)
{
    m_xComboBox->SetMRUEntries(rEntries);
}

void SalInstanceComboBoxWithEdit::HandleEventListener(VclWindowEvent& rEvent)
{
    if (rEvent.GetId() == VclEventId::DropdownPreOpen)
    {
        Size aRowSize(signal_custom_get_size(*m_xComboBox->GetOutDev()));
        m_xComboBox->SetUserItemSize(aRowSize);
    }
    CallHandleEventListener(rEvent);
}

SalInstanceComboBoxWithEdit::~SalInstanceComboBoxWithEdit()
{
    m_xComboBox->SetTextFilter(nullptr);
    m_xComboBox->SetEntryActivateHdl(Link<Edit&, bool>());
    m_xComboBox->SetModifyHdl(Link<Edit&, void>());
    m_xComboBox->SetSelectHdl(Link<::ComboBox&, void>());
}

IMPL_LINK_NOARG(SalInstanceComboBoxWithEdit, ChangeHdl, Edit&, void)
{
    if (!m_xComboBox->IsSyntheticModify()) // SelectHdl will be called
        signal_changed();
}

IMPL_LINK_NOARG(SalInstanceComboBoxWithEdit, SelectHdl, ::ComboBox&, void)
{
    m_bInSelect = true;
    signal_changed();
    m_bInSelect = false;
}

IMPL_LINK_NOARG(SalInstanceComboBoxWithEdit, EntryActivateHdl, Edit&, bool)
{
    return m_aEntryActivateHdl.Call(*this);
}

IMPL_LINK(SalInstanceComboBoxWithEdit, UserDrawHdl, UserDrawEvent*, pEvent, void)
{
    call_signal_custom_render(pEvent);
}

class SalInstanceEntryTreeView : public SalInstanceContainer, public virtual weld::EntryTreeView
{
private:
    DECL_LINK(AutocompleteHdl, Edit&, void);
    DECL_LINK(KeyPressListener, VclWindowEvent&, void);
    SalInstanceEntry* m_pEntry;
    SalInstanceTreeView* m_pTreeView;
    bool m_bTreeChange;

public:
    SalInstanceEntryTreeView(vcl::Window* pContainer, SalInstanceBuilder* pBuilder,
                             bool bTakeOwnership, std::unique_ptr<weld::Entry> xEntry,
                             std::unique_ptr<weld::TreeView> xTreeView)
        : EntryTreeView(std::move(xEntry), std::move(xTreeView))
        , SalInstanceContainer(pContainer, pBuilder, bTakeOwnership)
        , m_pEntry(dynamic_cast<SalInstanceEntry*>(m_xEntry.get()))
        , m_pTreeView(dynamic_cast<SalInstanceTreeView*>(m_xTreeView.get()))
        , m_bTreeChange(false)
    {
        assert(m_pEntry && m_pTreeView);

        Edit& rEntry = m_pEntry->getEntry();
        rEntry.SetAutocompleteHdl(LINK(this, SalInstanceEntryTreeView, AutocompleteHdl));
        rEntry.AddEventListener(LINK(this, SalInstanceEntryTreeView, KeyPressListener));
    }

    virtual void insert_separator(int /*pos*/, const OUString& /*rId*/) override { assert(false); }

    virtual void make_sorted() override
    {
        vcl::Window* pTreeView = m_pTreeView->getWidget();
        pTreeView->SetStyle(pTreeView->GetStyle() | WB_SORT);
    }

    virtual void set_entry_completion(bool bEnable, bool /*bCaseSensitive*/) override
    {
        assert(!bEnable && "not implemented yet");
        (void)bEnable;
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.SetAutocompleteHdl(Link<Edit&, void>());
    }

    virtual void set_font(const vcl::Font&) override { assert(false && "not implemented"); }

    virtual void set_entry_font(const vcl::Font& rFont) override { m_pEntry->set_font(rFont); }

    virtual vcl::Font get_entry_font() override
    {
        Edit& rEntry = m_pEntry->getEntry();
        return rEntry.GetPointFont(*rEntry.GetOutDev());
    }

    virtual void set_entry_placeholder_text(const OUString& rText) override
    {
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.SetPlaceholderText(rText);
    }

    virtual void set_entry_editable(bool bEditable) override
    {
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.SetReadOnly(!bEditable);
    }

    virtual void cut_entry_clipboard() override
    {
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.Cut();
    }

    virtual void copy_entry_clipboard() override
    {
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.Copy();
    }

    virtual void paste_entry_clipboard() override
    {
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.Paste();
    }

    virtual void grab_focus() override { m_xEntry->grab_focus(); }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        m_xEntry->connect_focus_in(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
        m_xEntry->connect_focus_out(rLink);
    }

    virtual bool changed_by_direct_pick() const override { return m_bTreeChange; }

    virtual void set_custom_renderer(bool /*bOn*/) override { assert(false && "not implemented"); }

    virtual int get_max_mru_count() const override
    {
        assert(false && "not implemented");
        return 0;
    }

    virtual void set_max_mru_count(int) override { assert(false && "not implemented"); }

    virtual OUString get_mru_entries() const override
    {
        assert(false && "not implemented");
        return OUString();
    }

    virtual void set_mru_entries(const OUString&) override { assert(false && "not implemented"); }

    virtual void set_item_menu(const OString&, weld::Menu*) override
    {
        assert(false && "not implemented");
    }

    int get_menu_button_width() const override
    {
        assert(false && "not implemented");
        return 0;
    }

    VclPtr<VirtualDevice> create_render_virtual_device() const override
    {
        return VclPtr<VirtualDevice>::Create();
    }

    virtual ~SalInstanceEntryTreeView() override
    {
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.RemoveEventListener(LINK(this, SalInstanceEntryTreeView, KeyPressListener));
        rEntry.SetAutocompleteHdl(Link<Edit&, void>());
    }
};

IMPL_LINK(SalInstanceEntryTreeView, KeyPressListener, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() != VclEventId::WindowKeyInput)
        return;
    const KeyEvent& rKeyEvent = *static_cast<KeyEvent*>(rEvent.GetData());
    sal_uInt16 nKeyCode = rKeyEvent.GetKeyCode().GetCode();
    if (!(nKeyCode == KEY_UP || nKeyCode == KEY_DOWN || nKeyCode == KEY_PAGEUP
          || nKeyCode == KEY_PAGEDOWN))
        return;

    m_pTreeView->disable_notify_events();
    auto& rListBox = m_pTreeView->getTreeView();
    if (!rListBox.FirstSelected())
    {
        if (SvTreeListEntry* pEntry = rListBox.First())
            rListBox.Select(pEntry, true);
    }
    else
        rListBox.KeyInput(rKeyEvent);
    m_xEntry->set_text(m_xTreeView->get_selected_text());
    m_xEntry->select_region(0, -1);
    m_pTreeView->enable_notify_events();
    m_bTreeChange = true;
    m_pEntry->fire_signal_changed();
    m_bTreeChange = false;
}

IMPL_LINK(SalInstanceEntryTreeView, AutocompleteHdl, Edit&, rEdit, void)
{
    Selection aSel = rEdit.GetSelection();

    OUString aFullText = rEdit.GetText();
    OUString aStartText = aFullText.copy(0, static_cast<sal_Int32>(aSel.Max()));

    int nPos = -1;
    int nCount = m_xTreeView->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        if (m_xTreeView->get_text(i).startsWithIgnoreAsciiCase(aStartText))
        {
            nPos = i;
            break;
        }
    }

    m_xTreeView->select(nPos);

    if (nPos != -1)
    {
        OUString aText = m_xTreeView->get_text(nPos);
        Selection aSelection(aText.getLength(), aStartText.getLength());
        rEdit.SetText(aText, aSelection);
    }
}

SalInstancePopover::SalInstancePopover(DockingWindow* pPopover, SalInstanceBuilder* pBuilder,
                                       bool bTakeOwnership)
    : SalInstanceContainer(pPopover, pBuilder, bTakeOwnership)
    , m_xPopover(pPopover)
{
}

SalInstancePopover::~SalInstancePopover()
{
    DockingManager* pDockingManager = vcl::Window::GetDockingManager();
    if (pDockingManager->IsInPopupMode(m_xPopover))
        ImplPopDown();
}

void SalInstancePopover::popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                                       weld::Placement ePlace)
{
    SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pParent);
    assert(pVclWidget);
    vcl::Window* pWidget = pVclWidget->getWidget();

    tools::Rectangle aRect;
    Point aPt = pWidget->OutputToScreenPixel(rRect.TopLeft());
    aRect.SetLeft(aPt.X());
    aRect.SetTop(aPt.Y());
    aPt = pWidget->OutputToScreenPixel(rRect.BottomRight());
    aRect.SetRight(aPt.X());
    aRect.SetBottom(aPt.Y());

    FloatWinPopupFlags nFlags = FloatWinPopupFlags::GrabFocus | FloatWinPopupFlags::NoMouseUpClose;
    if (ePlace == weld::Placement::Under)
        nFlags = nFlags | FloatWinPopupFlags::Down;
    else
        nFlags = nFlags | FloatWinPopupFlags::Right;

    m_xPopover->EnableDocking();
    DockingManager* pDockingManager = vcl::Window::GetDockingManager();
    pDockingManager->SetPopupModeEndHdl(m_xPopover,
                                        LINK(this, SalInstancePopover, PopupModeEndHdl));
    pDockingManager->StartPopupMode(m_xPopover, aRect, nFlags);
}

void SalInstancePopover::ImplPopDown()
{
    vcl::Window::GetDockingManager()->EndPopupMode(m_xPopover);
    m_xPopover->EnableDocking(false);
    signal_closed();
}

void SalInstancePopover::popdown() { ImplPopDown(); }

void SalInstancePopover::resize_to_request()
{
    ::resize_to_request(m_xPopover.get());

    DockingManager* pDockingManager = vcl::Window::GetDockingManager();
    if (pDockingManager->IsInPopupMode(m_xPopover.get()))
    {
        Size aSize = m_xPopover->get_preferred_size();
        tools::Rectangle aRect = pDockingManager->GetPosSizePixel(m_xPopover.get());
        pDockingManager->SetPosSizePixel(m_xPopover.get(), aRect.Left(), aRect.Top(), aSize.Width(),
                                         aSize.Height(), PosSizeFlags::Size);
    }
}

IMPL_LINK_NOARG(SalInstancePopover, PopupModeEndHdl, FloatingWindow*, void) { signal_closed(); }

SalInstanceBuilder::SalInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot,
                                       const OUString& rUIFile,
                                       const css::uno::Reference<css::frame::XFrame>& rFrame)
    : weld::Builder()
    , m_xBuilder(new VclBuilder(pParent, rUIRoot, rUIFile, OString(), rFrame, false))
{
}

std::unique_ptr<weld::MessageDialog> SalInstanceBuilder::weld_message_dialog(const OString& id)
{
    MessageDialog* pMessageDialog = m_xBuilder->get<MessageDialog>(id);
    std::unique_ptr<weld::MessageDialog> pRet(
        pMessageDialog ? new SalInstanceMessageDialog(pMessageDialog, this, false) : nullptr);
    if (pMessageDialog)
    {
        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pMessageDialog);
        m_xBuilder->drop_ownership(pMessageDialog);
    }
    return pRet;
}

std::unique_ptr<weld::Dialog> SalInstanceBuilder::weld_dialog(const OString& id)
{
    Dialog* pDialog = m_xBuilder->get<Dialog>(id);
    std::unique_ptr<weld::Dialog> pRet(pDialog ? new SalInstanceDialog(pDialog, this, false)
                                               : nullptr);
    if (pDialog)
    {
        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pDialog);
        m_xBuilder->drop_ownership(pDialog);
    }
    return pRet;
}

std::unique_ptr<weld::Assistant> SalInstanceBuilder::weld_assistant(const OString& id)
{
    vcl::RoadmapWizard* pDialog = m_xBuilder->get<vcl::RoadmapWizard>(id);
    std::unique_ptr<weld::Assistant> pRet(pDialog ? new SalInstanceAssistant(pDialog, this, false)
                                                  : nullptr);
    if (pDialog)
    {
        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pDialog);
        m_xBuilder->drop_ownership(pDialog);
    }
    return pRet;
}

std::unique_ptr<weld::Window> SalInstanceBuilder::create_screenshot_window()
{
    assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");

    vcl::Window* pRoot = m_xBuilder->get_widget_root();
    if (SystemWindow* pWindow = dynamic_cast<SystemWindow*>(pRoot))
    {
        std::unique_ptr<weld::Window> xRet(new SalInstanceWindow(pWindow, this, false));
        m_aOwnedToplevel.set(pWindow);
        m_xBuilder->drop_ownership(pWindow);
        return xRet;
    }

    VclPtrInstance<Dialog> xDialog(nullptr, WB_HIDE | WB_STDDIALOG | WB_SIZEABLE | WB_CLOSEABLE,
                                   Dialog::InitFlag::NoParent);
    xDialog->SetText(utl::ConfigManager::getProductName());

    auto xContentArea = VclPtr<VclVBox>::Create(xDialog, false, 12);
    pRoot->SetParent(xContentArea);
    assert(pRoot == xContentArea->GetWindow(GetWindowType::FirstChild));
    xContentArea->Show();
    pRoot->Show();
    xDialog->SetHelpId(pRoot->GetHelpId());

    m_aOwnedToplevel.set(xDialog);

    return std::unique_ptr<weld::Dialog>(new SalInstanceDialog(xDialog, this, false));
}

std::unique_ptr<weld::Widget> SalInstanceBuilder::weld_widget(const OString& id)
{
    vcl::Window* pWidget = m_xBuilder->get(id);
    return pWidget ? std::make_unique<SalInstanceWidget>(pWidget, this, false) : nullptr;
}

std::unique_ptr<weld::Container> SalInstanceBuilder::weld_container(const OString& id)
{
    vcl::Window* pContainer = m_xBuilder->get(id);
    return pContainer ? std::make_unique<SalInstanceContainer>(pContainer, this, false) : nullptr;
}

std::unique_ptr<weld::Box> SalInstanceBuilder::weld_box(const OString& id)
{
    VclBox* pContainer = m_xBuilder->get<VclBox>(id);
    return pContainer ? std::make_unique<SalInstanceBox>(pContainer, this, false) : nullptr;
}

std::unique_ptr<weld::Paned> SalInstanceBuilder::weld_paned(const OString& id)
{
    VclPaned* pPaned = m_xBuilder->get<VclPaned>(id);
    return pPaned ? std::make_unique<SalInstancePaned>(pPaned, this, false) : nullptr;
}

std::unique_ptr<weld::Frame> SalInstanceBuilder::weld_frame(const OString& id)
{
    VclFrame* pFrame = m_xBuilder->get<VclFrame>(id);
    std::unique_ptr<weld::Frame> pRet(pFrame ? new SalInstanceFrame(pFrame, this, false) : nullptr);
    return pRet;
}

std::unique_ptr<weld::ScrolledWindow>
SalInstanceBuilder::weld_scrolled_window(const OString& id, bool bUserManagedScrolling)
{
    VclScrolledWindow* pScrolledWindow = m_xBuilder->get<VclScrolledWindow>(id);
    return pScrolledWindow ? std::make_unique<SalInstanceScrolledWindow>(
                                 pScrolledWindow, this, false, bUserManagedScrolling)
                           : nullptr;
}

std::unique_ptr<weld::Notebook> SalInstanceBuilder::weld_notebook(const OString& id)
{
    vcl::Window* pNotebook = m_xBuilder->get(id);
    if (!pNotebook)
        return nullptr;
    if (pNotebook->GetType() == WindowType::TABCONTROL)
        return std::make_unique<SalInstanceNotebook>(static_cast<TabControl*>(pNotebook), this,
                                                     false);
    if (pNotebook->GetType() == WindowType::VERTICALTABCONTROL)
        return std::make_unique<SalInstanceVerticalNotebook>(
            static_cast<VerticalTabControl*>(pNotebook), this, false);
    return nullptr;
}

std::unique_ptr<weld::Button> SalInstanceBuilder::weld_button(const OString& id)
{
    Button* pButton = m_xBuilder->get<Button>(id);
    return pButton ? std::make_unique<SalInstanceButton>(pButton, this, false) : nullptr;
}

std::unique_ptr<weld::MenuButton> SalInstanceBuilder::weld_menu_button(const OString& id)
{
    MenuButton* pButton = m_xBuilder->get<MenuButton>(id);
    return pButton ? std::make_unique<SalInstanceMenuButton>(pButton, this, false) : nullptr;
}

std::unique_ptr<weld::MenuToggleButton>
SalInstanceBuilder::weld_menu_toggle_button(const OString& id)
{
    MenuToggleButton* pButton = m_xBuilder->get<MenuToggleButton>(id);
    return pButton ? std::make_unique<SalInstanceMenuToggleButton>(pButton, this, false) : nullptr;
}

std::unique_ptr<weld::LinkButton> SalInstanceBuilder::weld_link_button(const OString& id)
{
    FixedHyperlink* pButton = m_xBuilder->get<FixedHyperlink>(id);
    return pButton ? std::make_unique<SalInstanceLinkButton>(pButton, this, false) : nullptr;
}

std::unique_ptr<weld::ToggleButton> SalInstanceBuilder::weld_toggle_button(const OString& id)
{
    PushButton* pToggleButton = m_xBuilder->get<PushButton>(id);
    return pToggleButton ? std::make_unique<SalInstanceToggleButton>(pToggleButton, this, false)
                         : nullptr;
}

std::unique_ptr<weld::RadioButton> SalInstanceBuilder::weld_radio_button(const OString& id)
{
    RadioButton* pRadioButton = m_xBuilder->get<RadioButton>(id);
    return pRadioButton ? std::make_unique<SalInstanceRadioButton>(pRadioButton, this, false)
                        : nullptr;
}

std::unique_ptr<weld::CheckButton> SalInstanceBuilder::weld_check_button(const OString& id)
{
    CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
    return pCheckButton ? std::make_unique<SalInstanceCheckButton>(pCheckButton, this, false)
                        : nullptr;
}

std::unique_ptr<weld::Scale> SalInstanceBuilder::weld_scale(const OString& id)
{
    Slider* pSlider = m_xBuilder->get<Slider>(id);
    return pSlider ? std::make_unique<SalInstanceScale>(pSlider, this, false) : nullptr;
}

std::unique_ptr<weld::ProgressBar> SalInstanceBuilder::weld_progress_bar(const OString& id)
{
    ::ProgressBar* pProgress = m_xBuilder->get<::ProgressBar>(id);
    return pProgress ? std::make_unique<SalInstanceProgressBar>(pProgress, this, false) : nullptr;
}

std::unique_ptr<weld::Spinner> SalInstanceBuilder::weld_spinner(const OString& id)
{
    Throbber* pThrobber = m_xBuilder->get<Throbber>(id);
    return pThrobber ? std::make_unique<SalInstanceSpinner>(pThrobber, this, false) : nullptr;
}

std::unique_ptr<weld::Image> SalInstanceBuilder::weld_image(const OString& id)
{
    FixedImage* pImage = m_xBuilder->get<FixedImage>(id);
    return pImage ? std::make_unique<SalInstanceImage>(pImage, this, false) : nullptr;
}

std::unique_ptr<weld::Calendar> SalInstanceBuilder::weld_calendar(const OString& id)
{
    Calendar* pCalendar = m_xBuilder->get<Calendar>(id);
    return pCalendar ? std::make_unique<SalInstanceCalendar>(pCalendar, this, false) : nullptr;
}

std::unique_ptr<weld::Entry> SalInstanceBuilder::weld_entry(const OString& id)
{
    Edit* pEntry = m_xBuilder->get<Edit>(id);
    return pEntry ? std::make_unique<SalInstanceEntry>(pEntry, this, false) : nullptr;
}

std::unique_ptr<weld::SpinButton> SalInstanceBuilder::weld_spin_button(const OString& id)
{
    FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
    return pSpinButton ? std::make_unique<SalInstanceSpinButton>(pSpinButton, this, false)
                       : nullptr;
}

std::unique_ptr<weld::MetricSpinButton>
SalInstanceBuilder::weld_metric_spin_button(const OString& id, FieldUnit eUnit)
{
    std::unique_ptr<weld::SpinButton> xButton(weld_spin_button(id));
    if (xButton)
    {
        SalInstanceSpinButton& rButton = dynamic_cast<SalInstanceSpinButton&>(*xButton);
        rButton.SetUseThousandSep();
    }
    return std::make_unique<weld::MetricSpinButton>(std::move(xButton), eUnit);
}

std::unique_ptr<weld::FormattedSpinButton>
SalInstanceBuilder::weld_formatted_spin_button(const OString& id)
{
    FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
    return pSpinButton ? std::make_unique<SalInstanceFormattedSpinButton>(pSpinButton, this, false)
                       : nullptr;
}

std::unique_ptr<weld::ComboBox> SalInstanceBuilder::weld_combo_box(const OString& id)
{
    vcl::Window* pWidget = m_xBuilder->get(id);
    ::ComboBox* pComboBox = dynamic_cast<::ComboBox*>(pWidget);
    if (pComboBox)
        return std::make_unique<SalInstanceComboBoxWithEdit>(pComboBox, this, false);
    ListBox* pListBox = dynamic_cast<ListBox*>(pWidget);
    return pListBox ? std::make_unique<SalInstanceComboBoxWithoutEdit>(pListBox, this, false)
                    : nullptr;
}

std::unique_ptr<weld::EntryTreeView>
SalInstanceBuilder::weld_entry_tree_view(const OString& containerid, const OString& entryid,
                                         const OString& treeviewid)
{
    vcl::Window* pContainer = m_xBuilder->get(containerid);
    return pContainer ? std::make_unique<SalInstanceEntryTreeView>(pContainer, this, false,
                                                                   weld_entry(entryid),
                                                                   weld_tree_view(treeviewid))
                      : nullptr;
}

std::unique_ptr<weld::TreeView> SalInstanceBuilder::weld_tree_view(const OString& id)
{
    SvTabListBox* pTreeView = m_xBuilder->get<SvTabListBox>(id);
    return pTreeView ? std::make_unique<SalInstanceTreeView>(pTreeView, this, false) : nullptr;
}

std::unique_ptr<weld::IconView> SalInstanceBuilder::weld_icon_view(const OString& id)
{
    IconView* pIconView = m_xBuilder->get<IconView>(id);
    return pIconView ? std::make_unique<SalInstanceIconView>(pIconView, this, false) : nullptr;
}

std::unique_ptr<weld::Label> SalInstanceBuilder::weld_label(const OString& id)
{
    Control* pLabel = m_xBuilder->get<Control>(id);
    return pLabel ? std::make_unique<SalInstanceLabel>(pLabel, this, false) : nullptr;
}

std::unique_ptr<weld::TextView> SalInstanceBuilder::weld_text_view(const OString& id)
{
    VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
    return pTextView ? std::make_unique<SalInstanceTextView>(pTextView, this, false) : nullptr;
}

std::unique_ptr<weld::Expander> SalInstanceBuilder::weld_expander(const OString& id)
{
    VclExpander* pExpander = m_xBuilder->get<VclExpander>(id);
    return pExpander ? std::make_unique<SalInstanceExpander>(pExpander, this, false) : nullptr;
}

std::unique_ptr<weld::DrawingArea>
SalInstanceBuilder::weld_drawing_area(const OString& id, const a11yref& rA11yImpl,
                                      FactoryFunction pUITestFactoryFunction, void* pUserData)
{
    VclDrawingArea* pDrawingArea = m_xBuilder->get<VclDrawingArea>(id);
    return pDrawingArea
               ? std::make_unique<SalInstanceDrawingArea>(pDrawingArea, this, rA11yImpl,
                                                          pUITestFactoryFunction, pUserData, false)
               : nullptr;
}

std::unique_ptr<weld::Menu> SalInstanceBuilder::weld_menu(const OString& id)
{
    PopupMenu* pMenu = m_xBuilder->get_menu(id);
    return pMenu ? std::make_unique<SalInstanceMenu>(pMenu, true) : nullptr;
}

std::unique_ptr<weld::Popover> SalInstanceBuilder::weld_popover(const OString& id)
{
    DockingWindow* pDockingWindow = m_xBuilder->get<DockingWindow>(id);
    std::unique_ptr<weld::Popover> pRet(
        pDockingWindow ? new SalInstancePopover(pDockingWindow, this, false) : nullptr);
    if (pDockingWindow)
    {
        assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
        m_aOwnedToplevel.set(pDockingWindow);
        m_xBuilder->drop_ownership(pDockingWindow);
    }
    return pRet;
}

std::unique_ptr<weld::Toolbar> SalInstanceBuilder::weld_toolbar(const OString& id)
{
    ToolBox* pToolBox = m_xBuilder->get<ToolBox>(id);
    return pToolBox ? std::make_unique<SalInstanceToolbar>(pToolBox, this, false) : nullptr;
}

std::unique_ptr<weld::SizeGroup> SalInstanceBuilder::create_size_group()
{
    return std::make_unique<SalInstanceSizeGroup>();
}

OString SalInstanceBuilder::get_current_page_help_id() const
{
    vcl::Window* pCtrl = m_xBuilder->get("tabcontrol");
    if (!pCtrl)
        return OString();
    VclPtr<vcl::Window> xTabPage;
    if (pCtrl->GetType() == WindowType::TABCONTROL)
    {
        TabControl* pTabCtrl = static_cast<TabControl*>(pCtrl);
        xTabPage = pTabCtrl->GetTabPage(pTabCtrl->GetCurPageId());
    }
    else if (pCtrl->GetType() == WindowType::VERTICALTABCONTROL)
    {
        VerticalTabControl* pTabCtrl = static_cast<VerticalTabControl*>(pCtrl);
        xTabPage = pTabCtrl->GetPage(pTabCtrl->GetCurPageId());
    }
    vcl::Window* pTabChild = xTabPage ? xTabPage->GetWindow(GetWindowType::FirstChild) : nullptr;
    pTabChild = pTabChild ? pTabChild->GetWindow(GetWindowType::FirstChild) : nullptr;
    if (pTabChild)
        return pTabChild->GetHelpId();
    return OString();
}

SalInstanceBuilder::~SalInstanceBuilder()
{
    if (VclBuilderContainer* pOwnedToplevel
        = dynamic_cast<VclBuilderContainer*>(m_aOwnedToplevel.get()))
        pOwnedToplevel->m_pUIBuilder = std::move(m_xBuilder);
    else
        m_xBuilder.reset();
    m_aOwnedToplevel.disposeAndClear();
}

std::unique_ptr<weld::Builder>
SalInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    vcl::Window* pParentWidget = pParentInstance ? pParentInstance->getWidget() : nullptr;
    return std::make_unique<SalInstanceBuilder>(pParentWidget, rUIRoot, rUIFile);
}

std::unique_ptr<weld::Builder> SalInstance::CreateInterimBuilder(vcl::Window* pParent,
                                                                 const OUString& rUIRoot,
                                                                 const OUString& rUIFile, bool,
                                                                 sal_uInt64)
{
    return std::make_unique<SalInstanceBuilder>(pParent, rUIRoot, rUIFile);
}

void SalInstanceWindow::help()
{
    //show help for widget with keyboard focus
    vcl::Window* pWidget = ImplGetSVData()->mpWinData->mpFocusWin;
    if (!pWidget || comphelper::LibreOfficeKit::isActive())
        pWidget = m_xWindow;
    OString sHelpId = pWidget->GetHelpId();
    while (sHelpId.isEmpty())
    {
        pWidget = pWidget->GetParent();
        if (!pWidget)
            break;
        sHelpId = pWidget->GetHelpId();
    }
    std::unique_ptr<weld::Widget> xTemp(
        pWidget != m_xWindow ? new SalInstanceWidget(pWidget, m_pBuilder, false) : nullptr);
    weld::Widget* pSource = xTemp ? xTemp.get() : this;
    bool bRunNormalHelpRequest = !m_aHelpRequestHdl.IsSet() || m_aHelpRequestHdl.Call(*pSource);
    Help* pHelp = bRunNormalHelpRequest ? Application::GetHelp() : nullptr;
    if (!pHelp)
        return;

    // tdf#126007, there's a nice fallback route for offline help where
    // the current page of a notebook will get checked when the help
    // button is pressed and there was no help for the dialog found.
    //
    // But for online help that route doesn't get taken, so bodge this here
    // by using the page help id if available and if the help button itself
    // was the original id
    if (m_pBuilder && sHelpId.endsWith("/help"))
    {
        OString sPageId = m_pBuilder->get_current_page_help_id();
        if (!sPageId.isEmpty())
            sHelpId = sPageId;
        else
        {
            // tdf#129068 likewise the help for the wrapping dialog is less
            // helpful than the help for the content area could be
            vcl::Window* pContentArea = nullptr;
            if (::Dialog* pDialog = dynamic_cast<::Dialog*>(m_xWindow.get()))
                pContentArea = pDialog->get_content_area();
            if (pContentArea)
            {
                vcl::Window* pContentWidget = pContentArea->GetWindow(GetWindowType::LastChild);
                if (pContentWidget)
                    sHelpId = pContentWidget->GetHelpId();
            }
        }
    }
    pHelp->Start(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), pSource);
}

//iterate upwards through the hierarchy from this widgets through its parents
//calling func with their helpid until func returns true or we run out of parents
void SalInstanceWidget::help_hierarchy_foreach(const std::function<bool(const OString&)>& func)
{
    vcl::Window* pParent = m_xWidget;
    while ((pParent = pParent->GetParent()))
    {
        if (func(pParent->GetHelpId()))
            return;
    }
}

weld::MessageDialog* SalInstance::CreateMessageDialog(weld::Widget* pParent,
                                                      VclMessageType eMessageType,
                                                      VclButtonsType eButtonsType,
                                                      const OUString& rPrimaryMessage)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    SystemWindow* pParentWidget = pParentInstance ? pParentInstance->getSystemWindow() : nullptr;
    VclPtrInstance<MessageDialog> xMessageDialog(pParentWidget, rPrimaryMessage, eMessageType,
                                                 eButtonsType);
    return new SalInstanceMessageDialog(xMessageDialog, nullptr, true);
}

weld::Window* SalInstance::GetFrameWeld(const css::uno::Reference<css::awt::XWindow>& rWindow)
{
    UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper();
    if (!pWrapper)
        return nullptr;
    VclPtr<vcl::Window> xWindow = pWrapper->GetWindow(rWindow);
    if (!xWindow)
        return nullptr;
    return xWindow->GetFrameWeld();
}

weld::Window* SalFrame::GetFrameWeld() const
{
    if (!m_xFrameWeld)
    {
        vcl::Window* pWindow = GetWindow();
        if (pWindow)
        {
            assert(pWindow == pWindow->GetFrameWindow());
            m_xFrameWeld.reset(new SalInstanceWindow(pWindow, nullptr, false));
        }
    }
    return m_xFrameWeld.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
