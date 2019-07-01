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

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <salframe.hxx>
#include <salinst.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <saltimer.hxx>
#include <salimestatus.hxx>
#include <salsession.hxx>
#include <salsys.hxx>
#include <salbmp.hxx>
#include <salobj.hxx>
#include <salmenu.hxx>
#include <svdata.hxx>
#include <messagedialog.hxx>
#include <treeglue.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <utility>
#include <tools/helpers.hxx>
#include <vcl/aboutdialog.hxx>
#include <vcl/builder.hxx>
#include <vcl/calendar.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/fmtfield.hxx>
#include <vcl/headbar.hxx>
#include <vcl/ivctrl.hxx>
#include <vcl/layout.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/slider.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svimpbox.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/throbber.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/weld.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/viewdataentry.hxx>
#include <vcl/virdev.hxx>
#include <bitmaps.hlst>

SalFrame::SalFrame()
    : m_pWindow(nullptr)
    , m_pProc(nullptr)
{
}

// this file contains the virtual destructors of the sal interface
// compilers usually put their vtables where the destructor is

SalFrame::~SalFrame()
{
}

void SalFrame::SetCallback( vcl::Window* pWindow, SALFRAMEPROC pProc )
{
    m_pWindow = pWindow;
    m_pProc = pProc;
}

// default to full-frame flushes
// on ports where partial-flushes are much cheaper this method should be overridden
void SalFrame::Flush( const tools::Rectangle& )
{
    Flush();
}

void SalFrame::SetRepresentedURL( const OUString& )
{
    // currently this is Mac only functionality
}

SalInstance::SalInstance(std::unique_ptr<comphelper::SolarMutex> pMutex)
    : m_pYieldMutex(std::move(pMutex))
{
}

SalInstance::~SalInstance()
{
}

comphelper::SolarMutex* SalInstance::GetYieldMutex()
{
    return m_pYieldMutex.get();
}

sal_uInt32 SalInstance::ReleaseYieldMutexAll()
{
    return m_pYieldMutex->release(true);
}

void SalInstance::AcquireYieldMutex(sal_uInt32 nCount)
{
    m_pYieldMutex->acquire(nCount);
}

std::unique_ptr<SalSession> SalInstance::CreateSalSession()
{
    return nullptr;
}

std::unique_ptr<SalMenu> SalInstance::CreateMenu( bool, Menu* )
{
    // default: no native menus
    return nullptr;
}

std::unique_ptr<SalMenuItem> SalInstance::CreateMenuItem( const SalItemParams & )
{
    return nullptr;
}

bool SalInstance::CallEventCallback( void const * pEvent, int nBytes )
{
    return m_pEventInst.is() && m_pEventInst->dispatchEvent( pEvent, nBytes );
}

std::unique_ptr<SalI18NImeStatus> SalInstance::CreateI18NImeStatus()
{
    return std::unique_ptr<SalI18NImeStatus>(new SalI18NImeStatus);
}

SalTimer::~SalTimer() COVERITY_NOEXCEPT_FALSE
{
}

void SalBitmap::DropScaledCache()
{
    if (ImplSVData* pSVData = ImplGetSVData())
    {
        auto& rCache = pSVData->maGDIData.maScaleCache;
        rCache.remove_if([this] (const o3tl::lru_map<SalBitmap*, BitmapEx>::key_value_pair_t& rKeyValuePair)
                         { return rKeyValuePair.first == this; });
    }
}

SalBitmap::~SalBitmap()
{
    DropScaledCache();
}

SalI18NImeStatus::~SalI18NImeStatus()
{
}

SalSystem::~SalSystem()
{
}

SalPrinter::~SalPrinter()
{
}

bool SalPrinter::StartJob( const OUString*, const OUString&, const OUString&,
                           ImplJobSetup*, vcl::PrinterController& )
{
    return false;
}

SalInfoPrinter::~SalInfoPrinter()
{
}

SalVirtualDevice::~SalVirtualDevice()
{
}

SalObject::~SalObject()
{
}

SalMenu::~SalMenu()
{
}

bool SalMenu::ShowNativePopupMenu(FloatingWindow *, const tools::Rectangle&, FloatWinPopupFlags )
{
    return false;
}

void SalMenu::ShowCloseButton(bool)
{
}

bool SalMenu::AddMenuBarButton( const SalMenuButtonItem& )
{
    return false;
}

void SalMenu::RemoveMenuBarButton( sal_uInt16 )
{
}

tools::Rectangle SalMenu::GetMenuBarButtonRectPixel( sal_uInt16, SalFrame* )
{
    return tools::Rectangle();
}

int SalMenu::GetMenuBarHeight() const
{
    return 0;
}

void SalMenu::ApplyPersona()
{
}

SalMenuItem::~SalMenuItem()
{
}

class SalInstanceBuilder;

class SalInstanceWidget : public virtual weld::Widget
{
protected:
    VclPtr<vcl::Window> m_xWidget;
    SalInstanceBuilder* m_pBuilder;

private:
    DECL_LINK(EventListener, VclWindowEvent&, void);
    DECL_LINK(KeyEventListener, VclWindowEvent&, bool);
    DECL_LINK(MouseEventListener, VclSimpleEvent&, void);
    DECL_LINK(MnemonicActivateHdl, vcl::Window&, bool);

    const bool m_bTakeOwnership;
    bool m_bEventListener;
    bool m_bKeyEventListener;
    bool m_bMouseEventListener;
    int m_nBlockNotify;

protected:
    void ensure_event_listener()
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
    void ensure_key_listener()
    {
        if (!m_bKeyEventListener)
        {
            Application::AddKeyListener(LINK(this, SalInstanceWidget, KeyEventListener));
            m_bKeyEventListener = true;
        }
    }

    // we want the ability to know about mouse events that happen in our children
    // so use this variant, we will need to filter them later
    void ensure_mouse_listener()
    {
        if (!m_bMouseEventListener)
        {
            Application::AddEventListener(LINK(this, SalInstanceWidget, MouseEventListener));
            m_bMouseEventListener = true;
        }
    }

    virtual void HandleEventListener(VclWindowEvent& rEvent);
    virtual bool HandleKeyEventListener(VclWindowEvent& rEvent);
    virtual void HandleMouseEventListener(VclSimpleEvent& rEvent);

public:
    SalInstanceWidget(vcl::Window* pWidget, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : m_xWidget(pWidget)
        , m_pBuilder(pBuilder)
        , m_bTakeOwnership(bTakeOwnership)
        , m_bEventListener(false)
        , m_bKeyEventListener(false)
        , m_bMouseEventListener(false)
        , m_nBlockNotify(0)
    {
    }

    virtual void set_sensitive(bool sensitive) override
    {
        m_xWidget->Enable(sensitive);
    }

    virtual bool get_sensitive() const override
    {
        return m_xWidget->IsEnabled();
    }

    virtual bool get_visible() const override
    {
        return m_xWidget->IsVisible();
    }

    virtual bool is_visible() const override
    {
        return m_xWidget->IsReallyVisible();
    }

    virtual void set_can_focus(bool bCanFocus) override
    {
        auto nStyle = m_xWidget->GetStyle() & ~(WB_TABSTOP | WB_NOTABSTOP);
        if (bCanFocus)
            nStyle |= WB_TABSTOP;
        else
            nStyle |= WB_NOTABSTOP;
        m_xWidget->SetStyle(nStyle);
    }

    virtual void grab_focus() override
    {
        m_xWidget->GrabFocus();
    }

    virtual bool has_focus() const override
    {
        return m_xWidget->HasFocus();
    }

    virtual void set_has_default(bool has_default) override
    {
        m_xWidget->set_property("has-default", OUString::boolean(has_default));
    }

    virtual bool get_has_default() const override
    {
        return m_xWidget->GetStyle() & WB_DEFBUTTON;
    }

    virtual void show() override
    {
        m_xWidget->Show();
    }

    virtual void hide() override
    {
        m_xWidget->Hide();
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        m_xWidget->set_width_request(nWidth);
        m_xWidget->set_height_request(nHeight);
    }

    virtual Size get_size_request() const override
    {
        return Size(m_xWidget->get_width_request(),
                    m_xWidget->get_height_request());
    }

    virtual Size get_preferred_size() const override
    {
        return m_xWidget->get_preferred_size();
    }

    virtual float get_approximate_digit_width() const override
    {
        return m_xWidget->approximate_digit_width();
    }

    virtual int get_text_height() const override
    {
        return m_xWidget->GetTextHeight();
    }

    virtual Size get_pixel_size(const OUString& rText) const override
    {
        //TODO, or do I want GetTextBoundRect ?, just using width at the moment anyway
        return Size(m_xWidget->GetTextWidth(rText), m_xWidget->GetTextHeight());
    }

    virtual vcl::Font get_font() override
    {
        return m_xWidget->GetPointFont(*m_xWidget);
    }

    virtual OString get_buildable_name() const override
    {
        return m_xWidget->get_id().toUtf8();
    }

    virtual void set_help_id(const OString& rId) override
    {
        return m_xWidget->SetHelpId(rId);
    }

    virtual OString get_help_id() const override
    {
        return m_xWidget->GetHelpId();
    }

    virtual void set_grid_left_attach(int nAttach) override
    {
        m_xWidget->set_grid_left_attach(nAttach);
    }

    virtual int get_grid_left_attach() const override
    {
        return m_xWidget->get_grid_left_attach();
    }

    virtual void set_grid_width(int nCols) override
    {
        m_xWidget->set_grid_width(nCols);
    }

    virtual void set_grid_top_attach(int nAttach) override
    {
        m_xWidget->set_grid_top_attach(nAttach);
    }

    virtual int get_grid_top_attach() const override
    {
        return m_xWidget->get_grid_top_attach();
    }

    virtual void set_hexpand(bool bExpand) override
    {
        m_xWidget->set_hexpand(bExpand);
    }

    virtual bool get_hexpand() const override
    {
        return m_xWidget->get_hexpand();
    }

    virtual void set_vexpand(bool bExpand) override
    {
        m_xWidget->set_vexpand(bExpand);
    }

    virtual bool get_vexpand() const override
    {
        return m_xWidget->get_vexpand();
    }

    virtual void set_secondary(bool bSecondary) override
    {
        m_xWidget->set_secondary(bSecondary);
    }

    virtual void set_margin_top(int nMargin) override
    {
        m_xWidget->set_margin_top(nMargin);
    }

    virtual void set_margin_bottom(int nMargin) override
    {
        m_xWidget->set_margin_bottom(nMargin);
    }

    virtual void set_accessible_name(const OUString& rName) override
    {
        m_xWidget->SetAccessibleName(rName);
    }

    virtual OUString get_accessible_name() const override
    {
        return m_xWidget->GetAccessibleName();
    }

    virtual OUString get_accessible_description() const override
    {
        return m_xWidget->GetAccessibleDescription();
    }

    virtual void set_accessible_relation_labeled_by(weld::Widget* pLabel) override
    {
        vcl::Window* pAtkLabel = pLabel ? dynamic_cast<SalInstanceWidget&>(*pLabel).getWidget() : nullptr;
        m_xWidget->SetAccessibleRelationLabeledBy(pAtkLabel);
    }

    virtual void set_accessible_relation_label_for(weld::Widget* pLabeled) override
    {
        vcl::Window* pAtkLabeled = pLabeled ? dynamic_cast<SalInstanceWidget&>(*pLabeled).getWidget() : nullptr;
        m_xWidget->SetAccessibleRelationLabelFor(pAtkLabeled);
    }

    virtual void add_extra_accessible_relation(const css::accessibility::AccessibleRelation &rRelation) override
    {
        m_xWidget->AddExtraAccessibleRelation(rRelation);
    }

    virtual void clear_extra_accessible_relations() override
    {
        m_xWidget->ClearExtraAccessibleRelations();
    }

    virtual void set_tooltip_text(const OUString& rTip) override
    {
        m_xWidget->SetQuickHelpText(rTip);
    }

    virtual OUString get_tooltip_text() const override
    {
        return m_xWidget->GetQuickHelpText();
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        ensure_event_listener();
        weld::Widget::connect_focus_in(rLink);
    }

    virtual void connect_mnemonic_activate(const Link<Widget&, bool>& rLink) override
    {
        m_xWidget->SetMnemonicActivateHdl(LINK(this, SalInstanceWidget, MnemonicActivateHdl));
        weld::Widget::connect_mnemonic_activate(rLink);
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override
    {
        ensure_event_listener();
        weld::Widget::connect_focus_out(rLink);
    }

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override
    {
        ensure_event_listener();
        weld::Widget::connect_size_allocate(rLink);
    }

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensure_mouse_listener();
        weld::Widget::connect_mouse_press(rLink);
    }

    virtual void connect_mouse_move(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensure_mouse_listener();
        weld::Widget::connect_mouse_move(rLink);
    }

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink) override
    {
        ensure_mouse_listener();
        weld::Widget::connect_mouse_release(rLink);
    }

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override
    {
        ensure_key_listener();
        weld::Widget::connect_key_press(rLink);
    }

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override
    {
        ensure_key_listener();
        weld::Widget::connect_key_release(rLink);
    }

    virtual bool get_extents_relative_to(Widget& rRelative, int& x, int &y, int& width, int &height) override
    {
        tools::Rectangle aRect(m_xWidget->GetWindowExtentsRelative(dynamic_cast<SalInstanceWidget&>(rRelative).getWidget()));
        x = aRect.Left();
        y = aRect.Top();
        width = aRect.GetWidth();
        height = aRect.GetHeight();
        return true;
    }

    virtual void grab_add() override
    {
        m_xWidget->CaptureMouse();
    }

    virtual bool has_grab() const override
    {
        return m_xWidget->IsMouseCaptured();
    }

    virtual void grab_remove() override
    {
        m_xWidget->ReleaseMouse();
    }

    virtual bool get_direction() const override
    {
        return m_xWidget->IsRTLEnabled();
    }

    virtual void set_direction(bool bRTL) override
    {
        m_xWidget->EnableRTL(bRTL);
    }

    virtual void freeze() override
    {
        m_xWidget->SetUpdateMode(false);
    }

    virtual void thaw() override
    {
        m_xWidget->SetUpdateMode(true);
    }

    virtual std::unique_ptr<weld::Container> weld_parent() const override;

    virtual ~SalInstanceWidget() override
    {
        if (m_aMnemonicActivateHdl.IsSet())
            m_xWidget->SetMnemonicActivateHdl(Link<vcl::Window&,bool>());
        if (m_bMouseEventListener)
            Application::RemoveEventListener(LINK(this, SalInstanceWidget, MouseEventListener));
        if (m_bKeyEventListener)
            Application::RemoveKeyListener(LINK(this, SalInstanceWidget, KeyEventListener));
        if (m_bEventListener)
            m_xWidget->RemoveEventListener(LINK(this, SalInstanceWidget, EventListener));
        if (m_bTakeOwnership)
            m_xWidget.disposeAndClear();
    }

    vcl::Window* getWidget()
    {
        return m_xWidget;
    }

    void disable_notify_events()
    {
        ++m_nBlockNotify;
    }

    bool notify_events_disabled()
    {
        return m_nBlockNotify != 0;
    }

    void enable_notify_events()
    {
        --m_nBlockNotify;
    }

    virtual void help_hierarchy_foreach(const std::function<bool(const OString&)>& func) override;

    virtual OUString strip_mnemonic(const OUString &rLabel) const override
    {
        return rLabel.replaceFirst("~", "");
    }

    virtual VclPtr<VirtualDevice> create_virtual_device() const override
    {
        // create with (annoying) seperate alpha layer that LibreOffice itself uses
        return VclPtr<VirtualDevice>::Create(*Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    }

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override
    {
        return m_xWidget->GetDropTarget();
    }

    virtual void set_stack_background() override
    {
        m_xWidget->SetControlBackground(m_xWidget->GetSettings().GetStyleSettings().GetWindowColor());
        m_xWidget->SetBackground(m_xWidget->GetControlBackground());
        // turn off WB_CLIPCHILDREN otherwise the bg won't extend "under"
        // transparent children of the widget
        m_xWidget->SetStyle(m_xWidget->GetStyle() & ~WB_CLIPCHILDREN);
    }

    SystemWindow* getSystemWindow()
    {
        return m_xWidget->GetSystemWindow();
    }
};

void SalInstanceWidget::HandleEventListener(VclWindowEvent& rEvent)
{
    if (rEvent.GetId() == VclEventId::WindowGetFocus)
        m_aFocusInHdl.Call(*this);
    else if (rEvent.GetId() == VclEventId::WindowLoseFocus)
        m_aFocusOutHdl.Call(*this);
    else if (rEvent.GetId() == VclEventId::WindowResize)
        m_aSizeAllocateHdl.Call(m_xWidget->GetSizePixel());
}

void SalInstanceWidget::HandleMouseEventListener(VclSimpleEvent& rEvent)
{
    if (rEvent.GetId() == VclEventId::WindowMouseButtonDown)
    {
        auto& rWinEvent = static_cast<VclWindowEvent&>(rEvent);
        if (m_xWidget->IsWindowOrChild(rWinEvent.GetWindow()))
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            m_aMousePressHdl.Call(*pMouseEvent);
        }
    }
    else if (rEvent.GetId() == VclEventId::WindowMouseButtonUp)
    {
        auto& rWinEvent = static_cast<VclWindowEvent&>(rEvent);
        if (m_xWidget->IsWindowOrChild(rWinEvent.GetWindow()))
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            m_aMouseReleaseHdl.Call(*pMouseEvent);
        }
    }
    else if (rEvent.GetId() == VclEventId::WindowMouseMove)
    {
        auto& rWinEvent = static_cast<VclWindowEvent&>(rEvent);
        if (m_xWidget->IsWindowOrChild(rWinEvent.GetWindow()))
        {
            const MouseEvent* pMouseEvent = static_cast<const MouseEvent*>(rWinEvent.GetData());
            m_aMouseMotionHdl.Call(*pMouseEvent);
        }
    }
}

bool SalInstanceWidget::HandleKeyEventListener(VclWindowEvent& rEvent)
{
    // we get all key events here, ignore them unless we have focus
    if (!has_focus())
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

IMPL_LINK(SalInstanceWidget, MouseEventListener, VclSimpleEvent&, rEvent, void)
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
            assert((rImage == "dialog-warning" || rImage == "dialog-error" || rImage == "dialog-information") && "unknown stock image");
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

    sal_uInt16 insert_to_menu(sal_uInt16 nLastId, PopupMenu* pMenu, int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, const VirtualDevice* pImageSurface, bool bCheck)
    {
        const sal_uInt16 nNewid = nLastId + 1;
        pMenu->InsertItem(nNewid, rStr, bCheck ? MenuItemBits::CHECKABLE : MenuItemBits::NONE,
                            OUStringToOString(rId, RTL_TEXTENCODING_UTF8), pos == -1 ? MENU_APPEND : pos);
        if (pIconName)
        {
            pMenu->SetItemImage(nNewid, createImage(*pIconName));
        }
        else if (pImageSurface)
        {
            pMenu->SetItemImage(nNewid, createImage(*pImageSurface));
        }
        return nNewid;
    }
}

class SalInstanceMenu : public weld::Menu
{
private:
    VclPtr<PopupMenu> m_xMenu;

    bool const m_bTakeOwnership;
    sal_uInt16 m_nLastId;

public:
    SalInstanceMenu(PopupMenu* pMenu, bool bTakeOwnership)
        : m_xMenu(pMenu)
        , m_bTakeOwnership(bTakeOwnership)
    {
        const auto nCount = m_xMenu->GetItemCount();
        m_nLastId = nCount ? pMenu->GetItemId(nCount-1) : 0;
    }
    virtual OString popup_at_rect(weld::Widget* pParent, const tools::Rectangle &rRect) override
    {
        SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pParent);
        assert(pVclWidget);
        m_xMenu->Execute(pVclWidget->getWidget(), rRect, PopupMenuFlags::ExecuteDown | PopupMenuFlags::NoMouseUpClose);
        return m_xMenu->GetCurItemIdent();
    }
    virtual void set_sensitive(const OString& rIdent, bool bSensitive) override
    {
        m_xMenu->EnableItem(rIdent, bSensitive);
    }
    virtual void set_active(const OString& rIdent, bool bActive) override
    {
        m_xMenu->CheckItem(rIdent, bActive);
    }
    virtual bool get_active(const OString& rIdent) const override
    {
        return m_xMenu->IsItemChecked(m_xMenu->GetItemId(rIdent));
    }
    virtual void set_label(const OString& rIdent, const OUString& rLabel) override
    {
        m_xMenu->SetItemText(m_xMenu->GetItemId(rIdent), rLabel);
    }
    virtual void set_visible(const OString& rIdent, bool bShow) override
    {
        m_xMenu->ShowItem(m_xMenu->GetItemId(rIdent), bShow);
    }
    virtual void clear() override
    {
        m_xMenu->Clear();
    }
    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface, bool bCheck) override
    {
        m_nLastId = insert_to_menu(m_nLastId, m_xMenu, pos, rId, rStr, pIconName, pImageSurface, bCheck);
    }
    virtual void insert_separator(int pos, const OUString& rId) override
    {
        auto nInsertPos = pos == -1 ? MENU_APPEND : pos;
        m_xMenu->InsertSeparator(rId.toUtf8(), nInsertPos);
    }
    virtual ~SalInstanceMenu() override
    {
        if (m_bTakeOwnership)
            m_xMenu.disposeAndClear();
    }
};

class SalInstanceToolbar : public SalInstanceWidget, public virtual weld::Toolbar
{
private:
    VclPtr<ToolBox> m_xToolBox;

    DECL_LINK(ClickHdl, ToolBox*, void);
public:
    SalInstanceToolbar(ToolBox* pToolBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pToolBox, pBuilder, bTakeOwnership)
        , m_xToolBox(pToolBox)
    {
        m_xToolBox->SetSelectHdl(LINK(this, SalInstanceToolbar, ClickHdl));
    }

    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override
    {
        m_xToolBox->EnableItem(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), bSensitive);
    }

    virtual bool get_item_sensitive(const OString& rIdent) const override
    {
        return m_xToolBox->IsItemEnabled(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)));
    }

    virtual void set_item_active(const OString& rIdent, bool bActive) override
    {
        m_xToolBox->CheckItem(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)), bActive);
    }

    virtual bool get_item_active(const OString& rIdent) const override
    {
        return m_xToolBox->IsItemChecked(m_xToolBox->GetItemId(OUString::fromUtf8(rIdent)));
    }

    virtual void insert_separator(int pos, const OUString& /*rId*/) override
    {
        auto nInsertPos = pos == -1 ? ToolBox::APPEND : pos;
        m_xToolBox->InsertSeparator(nInsertPos, 5);
    }

    virtual ~SalInstanceToolbar() override
    {
        m_xToolBox->SetSelectHdl(Link<ToolBox*, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceToolbar, ClickHdl, ToolBox*, void)
{
    sal_uInt16 nItemId = m_xToolBox->GetCurItemId();
    signal_clicked(m_xToolBox->GetItemCommand(nItemId).toUtf8());
}

class SalInstanceSizeGroup : public weld::SizeGroup
{
private:
    std::shared_ptr<VclSizeGroup> m_xGroup;
public:
    SalInstanceSizeGroup()
        : m_xGroup(new VclSizeGroup)
    {
    }
    virtual void add_widget(weld::Widget* pWidget) override
    {
        SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
        assert(pVclWidget);
        m_xGroup->insert(pVclWidget->getWidget());
    }
    virtual void set_mode(VclSizeGroupMode eMode) override
    {
        m_xGroup->set_mode(eMode);
    }
};

class SalInstanceContainer : public SalInstanceWidget, public virtual weld::Container
{
private:
    VclPtr<vcl::Window> m_xContainer;
public:
    SalInstanceContainer(vcl::Window* pContainer, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pContainer, pBuilder, bTakeOwnership)
        , m_xContainer(pContainer)
    {
    }
    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override
    {
        SalInstanceWidget* pVclWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
        assert(pVclWidget);
        SalInstanceContainer* pNewVclParent = dynamic_cast<SalInstanceContainer*>(pNewParent);
        assert(!pNewParent || pNewVclParent);
        pVclWidget->getWidget()->SetParent(pNewVclParent ? pNewVclParent->getWidget() : nullptr);
    }
};

std::unique_ptr<weld::Container> SalInstanceWidget::weld_parent() const
{
    vcl::Window* pParent = m_xWidget->GetParent();
    if (!pParent)
        return nullptr;
    return std::make_unique<SalInstanceContainer>(pParent, m_pBuilder, false);
}

class SalInstanceWindow : public SalInstanceContainer, public virtual weld::Window
{
private:
    VclPtr<vcl::Window> m_xWindow;

    DECL_LINK(HelpHdl, vcl::Window&, bool);

    void override_child_help(vcl::Window* pParent)
    {
        for (vcl::Window *pChild = pParent->GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
            override_child_help(pChild);
        pParent->SetHelpHdl(LINK(this, SalInstanceWindow, HelpHdl));
    }

    void clear_child_help(vcl::Window* pParent)
    {
        for (vcl::Window *pChild = pParent->GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
            clear_child_help(pChild);
        pParent->SetHelpHdl(Link<vcl::Window&,bool>());
    }

public:
    SalInstanceWindow(vcl::Window* pWindow, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pWindow, pBuilder, bTakeOwnership)
        , m_xWindow(pWindow)
    {
        override_child_help(m_xWindow);
    }

    virtual void set_title(const OUString& rTitle) override
    {
        m_xWindow->SetText(rTitle);
    }

    virtual OUString get_title() const override
    {
        return m_xWindow->GetText();
    }

    void help()
    {
        //show help for widget with keyboard focus
        vcl::Window* pWidget = ImplGetSVData()->maWinData.mpFocusWin;
        if (!pWidget)
            pWidget = m_xWindow;
        OString sHelpId = pWidget->GetHelpId();
        while (sHelpId.isEmpty())
        {
            pWidget = pWidget->GetParent();
            if (!pWidget)
                break;
            sHelpId = pWidget->GetHelpId();
        }
        std::unique_ptr<weld::Widget> xTemp(pWidget != m_xWindow ? new SalInstanceWidget(pWidget, m_pBuilder, false) : nullptr);
        weld::Widget* pSource = xTemp ? xTemp.get() : this;
        bool bRunNormalHelpRequest = !m_aHelpRequestHdl.IsSet() || m_aHelpRequestHdl.Call(*pSource);
        Help* pHelp = bRunNormalHelpRequest ? Application::GetHelp() : nullptr;
        if (pHelp)
            pHelp->Start(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), pSource);
    }

    virtual void set_busy_cursor(bool bBusy) override
    {
        if (bBusy)
            m_xWindow->EnterWait();
        else
            m_xWindow->LeaveWait();
    }

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() override
    {
        css::uno::Reference<css::awt::XWindow> xWindow(m_xWindow->GetComponentInterface(), css::uno::UNO_QUERY);
        return xWindow;
    }

    virtual void resize_to_request() override
    {
        if (SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(m_xWindow.get()))
        {
            pSysWin->setOptimalLayoutSize();
            return;
        }
        if (DockingWindow* pDockWin = dynamic_cast<DockingWindow*>(m_xWindow.get()))
        {
            pDockWin->setOptimalLayoutSize();
            return;
        }
        assert(false && "must be system or docking window");
    }

    virtual void set_modal(bool bModal) override
    {
        m_xWindow->ImplGetFrame()->SetModal(bModal);
    }

    virtual bool get_modal() const override
    {
        return m_xWindow->ImplGetFrame()->GetModal();
    }

    virtual void window_move(int x, int y) override
    {
        m_xWindow->SetPosPixel(Point(x, y));
    }

    virtual Size get_size() const override
    {
        return m_xWindow->GetSizePixel();
    }

    virtual Point get_position() const override
    {
        return m_xWindow->GetPosPixel();
    }

    virtual tools::Rectangle get_monitor_workarea() const override
    {
        return m_xWindow->GetDesktopRectPixel();
    }

    virtual void set_centered_on_parent(bool /*bTrackGeometryRequests*/) override
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

    virtual bool get_resizable() const override
    {
        return m_xWindow->GetStyle() & WB_SIZEABLE;
    }

    virtual bool has_toplevel_focus() const override
    {
        return m_xWindow->HasChildPathFocus();
    }

    virtual void present() override
    {
        m_xWindow->ToTop(ToTopFlags::RestoreWhenMin | ToTopFlags::ForegroundTask);
    }

    virtual void set_window_state(const OString& rStr) override
    {
        SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(m_xWindow.get());
        assert(pSysWin);
        pSysWin->SetWindowState(rStr);
    }

    virtual OString get_window_state(WindowStateMask nMask) const override
    {
        SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(m_xWindow.get());
        assert(pSysWin);
        return pSysWin->GetWindowState(nMask);
    }

    virtual SystemEnvData get_system_data() const override
    {
        return *m_xWindow->GetSystemData();
    }

    virtual void connect_toplevel_focus_changed(const Link<weld::Widget&, void>& rLink) override
    {
        ensure_event_listener();
        weld::Window::connect_toplevel_focus_changed(rLink);
    }

    virtual void HandleEventListener(VclWindowEvent& rEvent) override
    {
        if (rEvent.GetId() == VclEventId::WindowActivate || rEvent.GetId() == VclEventId::WindowDeactivate)
        {
            signal_toplevel_focus_changed();
            return;
        }
        SalInstanceContainer::HandleEventListener(rEvent);
    }

    virtual ~SalInstanceWindow() override
    {
        clear_child_help(m_xWindow);
    }
};

IMPL_LINK_NOARG(SalInstanceWindow, HelpHdl, vcl::Window&, bool)
{
    help();
    return false;
}

typedef std::set<VclPtr<vcl::Window> > winset;

namespace
{
    void hideUnless(const vcl::Window *pTop, const winset& rVisibleWidgets,
        std::vector<VclPtr<vcl::Window> > &rWasVisibleWidgets)
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

class SalInstanceDialog : public SalInstanceWindow, public virtual weld::Dialog
{
private:
    VclPtr<::Dialog> m_xDialog;

    // for calc ref dialog that shrink to range selection widgets and resize back
    VclPtr<vcl::Window> m_xRefEdit;
    std::vector<VclPtr<vcl::Window> > m_aHiddenWidgets;    // vector of hidden Controls
    long m_nOldEditWidthReq; // Original width request of the input field
    sal_Int32 m_nOldBorderWidth; // border width for expanded dialog

public:
    SalInstanceDialog(::Dialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWindow(pDialog, pBuilder, bTakeOwnership)
        , m_xDialog(pDialog)
        , m_nOldEditWidthReq(0)
        , m_nOldBorderWidth(0)
    {
    }

    virtual bool runAsync(std::shared_ptr<weld::DialogController> aOwner, const std::function<void(sal_Int32)> &rEndDialogFn) override
    {
        VclAbstractDialog::AsyncContext aCtx;
        aCtx.mxOwnerDialogController = aOwner;
        aCtx.maEndDialogFn = rEndDialogFn;
        VclButtonBox* pActionArea = m_xDialog->get_action_area();
        if (pActionArea)
           pActionArea->sort_native_button_order();
        return m_xDialog->StartExecuteAsync(aCtx);
    }

    virtual bool runAsync(std::shared_ptr<Dialog> const & rxSelf, const std::function<void(sal_Int32)> &rEndDialogFn) override
    {
        assert( rxSelf.get() == this );
        VclAbstractDialog::AsyncContext aCtx;
        // In order to store a shared_ptr to ourself, we have to have been constructed by make_shared,
        // which is that rxSelf enforces.
        aCtx.mxOwnerSelf = rxSelf;
        aCtx.maEndDialogFn = rEndDialogFn;
        VclButtonBox* pActionArea = m_xDialog->get_action_area();
        if (pActionArea)
           pActionArea->sort_native_button_order();
        return m_xDialog->StartExecuteAsync(aCtx);
    }

    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override
    {
        SalInstanceWidget* pVclEdit = dynamic_cast<SalInstanceWidget*>(pEdit);
        SalInstanceWidget* pVclButton = dynamic_cast<SalInstanceWidget*>(pButton);

        vcl::Window* pRefEdit = pVclEdit->getWidget();
        vcl::Window* pRefBtn = pVclButton ? pVclButton->getWidget() : nullptr;

        auto nOldEditWidth = pRefEdit->GetSizePixel().Width();
        m_nOldEditWidthReq = pRefEdit->get_width_request();

        //We want just pRefBtn and pRefEdit to be shown
        //mark widgets we want to be visible, starting with pRefEdit
        //and all its direct parents.
        winset aVisibleWidgets;
        vcl::Window *pContentArea = m_xDialog->get_content_area();
        for (vcl::Window *pCandidate = pRefEdit;
            pCandidate && (pCandidate != pContentArea && pCandidate->IsVisible());
            pCandidate = pCandidate->GetWindow(GetWindowType::RealParent))
        {
            aVisibleWidgets.insert(pCandidate);
        }
        //same again with pRefBtn, except stop if there's a
        //shared parent in the existing widgets
        for (vcl::Window *pCandidate = pRefBtn;
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
        if (vcl::Window *pActionArea = m_xDialog->get_action_area())
            pActionArea->Hide();
        m_xDialog->setOptimalLayoutSize();
        m_xRefEdit = pRefEdit;
    }

    virtual void undo_collapse() override
    {
        // All others: Show();
        for (VclPtr<vcl::Window> const & pWindow : m_aHiddenWidgets)
        {
            pWindow->Show();
        }
        m_aHiddenWidgets.clear();

        m_xRefEdit->set_width_request(m_nOldEditWidthReq);
        m_xRefEdit.clear();
        m_xDialog->set_border_width(m_nOldBorderWidth);
        if (vcl::Window *pActionArea = m_xDialog->get_action_area())
            pActionArea->Show();
        m_xDialog->setOptimalLayoutSize();
    }

    virtual void SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink) override
    {
        m_xDialog->SetInstallLOKNotifierHdl(rLink);
    }

    virtual int run() override
    {
        VclButtonBox* pActionArea = m_xDialog->get_action_area();
        if (pActionArea)
           pActionArea->sort_native_button_order();
        return m_xDialog->Execute();
    }

    virtual void response(int nResponse) override
    {
        m_xDialog->EndDialog(nResponse);
    }

    virtual void add_button(const OUString& rText, int nResponse, const OString& rHelpId) override
    {
        VclButtonBox* pBox = m_xDialog->get_action_area();
        VclPtr<PushButton> xButton(VclPtr<PushButton>::Create(pBox, WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER));
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

    virtual void set_modal(bool bModal) override
    {
        if (get_modal() == bModal)
            return;
        m_xDialog->SetModalInputMode(bModal);
    }

    virtual bool get_modal() const override
    {
        return m_xDialog->IsModalInputMode();
    }

    virtual weld::Button* get_widget_for_response(int nResponse) override;

    virtual void set_default_response(int nResponse) override
    {
        m_xDialog->set_default_response(nResponse);
    }

    virtual Container* weld_content_area() override
    {
        return new SalInstanceContainer(m_xDialog->get_content_area(), m_pBuilder, false);
    }
};

class SalInstanceMessageDialog : public SalInstanceDialog, public virtual weld::MessageDialog
{
private:
    VclPtr<::MessageDialog> m_xMessageDialog;
public:
    SalInstanceMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceDialog(pDialog, pBuilder, bTakeOwnership)
        , m_xMessageDialog(pDialog)
    {
    }

    virtual void set_primary_text(const OUString& rText) override
    {
        m_xMessageDialog->set_primary_text(rText);
    }

    virtual OUString get_primary_text() const override
    {
        return m_xMessageDialog->get_primary_text();
    }

    virtual void set_secondary_text(const OUString& rText) override
    {
        m_xMessageDialog->set_secondary_text(rText);
    }

    virtual OUString get_secondary_text() const override
    {
        return m_xMessageDialog->get_secondary_text();
    }

    virtual Container* weld_message_area() override
    {
        return new SalInstanceContainer(m_xMessageDialog->get_message_area(), m_pBuilder, false);
    }
};

class SalInstanceAboutDialog : public SalInstanceDialog, public virtual weld::AboutDialog
{
private:
    VclPtr<vcl::AboutDialog> m_xAboutDialog;
public:
    SalInstanceAboutDialog(vcl::AboutDialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceDialog(pDialog, pBuilder, bTakeOwnership)
        , m_xAboutDialog(pDialog)
    {
    }
    virtual void set_version(const OUString& rVersion) override
    {
        m_xAboutDialog->SetVersion(rVersion);
    }
    virtual void set_copyright(const OUString& rCopyright) override
    {
        m_xAboutDialog->SetCopyright(rCopyright);
    }
    virtual void set_website(const OUString& rURL) override
    {
        m_xAboutDialog->SetWebsiteLink(rURL);
    }
    virtual void set_website_label(const OUString& rLabel) override
    {
        m_xAboutDialog->SetWebsiteLabel(rLabel);
    }
    virtual OUString get_website_label() const override
    {
        return m_xAboutDialog->GetWebsiteLabel();
    }
    virtual void set_logo(VirtualDevice* pDevice) override
    {
        m_xAboutDialog->SetLogo(createImage(*pDevice));
    }
    virtual void set_background(VirtualDevice* pDevice) override
    {
        m_xAboutDialog->SetBackground(createImage(*pDevice));
    }
};

class SalInstanceFrame : public SalInstanceContainer, public virtual weld::Frame
{
private:
    VclPtr<VclFrame> m_xFrame;
public:
    SalInstanceFrame(VclFrame* pFrame, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pFrame, pBuilder, bTakeOwnership)
        , m_xFrame(pFrame)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        m_xFrame->set_label(rText);
    }

    virtual OUString get_label() const override
    {
        return m_xFrame->get_label();
    }

    virtual std::unique_ptr<weld::Label> weld_label_widget() const override;
};

class SalInstanceScrolledWindow : public SalInstanceContainer, public virtual weld::ScrolledWindow
{
private:
    VclPtr<VclScrolledWindow> m_xScrolledWindow;
    Link<ScrollBar*,void> m_aOrigVScrollHdl;
    Link<ScrollBar*,void> m_aOrigHScrollHdl;
    bool m_bUserManagedScrolling;

    DECL_LINK(VscrollHdl, ScrollBar*, void);
    DECL_LINK(HscrollHdl, ScrollBar*, void);

public:
    SalInstanceScrolledWindow(VclScrolledWindow* pScrolledWindow, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pScrolledWindow, pBuilder, bTakeOwnership)
        , m_xScrolledWindow(pScrolledWindow)
        , m_bUserManagedScrolling(false)
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        m_aOrigVScrollHdl = rVertScrollBar.GetScrollHdl();
        rVertScrollBar.SetScrollHdl(LINK(this, SalInstanceScrolledWindow, VscrollHdl));
        ScrollBar& rHorzScrollBar = m_xScrolledWindow->getHorzScrollBar();
        m_aOrigHScrollHdl = rHorzScrollBar.GetScrollHdl();
        rHorzScrollBar.SetScrollHdl(LINK(this, SalInstanceScrolledWindow, HscrollHdl));
    }

    virtual void hadjustment_configure(int value, int lower, int upper,
                                       int step_increment, int page_increment,
                                       int page_size) override
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

    virtual void set_hpolicy(VclPolicyType eHPolicy) override
    {
        WinBits nWinBits = m_xScrolledWindow->GetStyle() & ~(WB_AUTOHSCROLL|WB_HSCROLL);
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

    virtual int get_hscroll_height() const override
    {
        return m_xScrolledWindow->getHorzScrollBar().get_preferred_size().Height();
    }

    virtual void vadjustment_configure(int value, int lower, int upper,
                                       int step_increment, int page_increment,
                                       int page_size) override
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

    virtual void set_vpolicy(VclPolicyType eVPolicy) override
    {
        WinBits nWinBits = m_xScrolledWindow->GetStyle() & ~(WB_AUTOVSCROLL|WB_VSCROLL);
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

    virtual int get_vscroll_width() const override
    {
        return m_xScrolledWindow->getVertScrollBar().get_preferred_size().Width();
    }

    virtual void set_user_managed_scrolling() override
    {
        m_bUserManagedScrolling = true;
        m_xScrolledWindow->setUserManagedScrolling(true);
    }

    virtual ~SalInstanceScrolledWindow() override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        rVertScrollBar.SetScrollHdl(m_aOrigVScrollHdl);
    }
};

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

class SalInstanceNotebook : public SalInstanceContainer, public virtual weld::Notebook
{
private:
    VclPtr<TabControl> m_xNotebook;
    mutable std::vector<std::unique_ptr<SalInstanceContainer>> m_aPages;
    std::vector<VclPtr<TabPage>> m_aAddedPages;
    std::vector<VclPtr<VclGrid>> m_aAddedGrids;

    DECL_LINK(DeactivatePageHdl, TabControl*, bool);
    DECL_LINK(ActivatePageHdl, TabControl*, void);

public:
    SalInstanceNotebook(TabControl* pNotebook, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pNotebook, pBuilder, bTakeOwnership)
        , m_xNotebook(pNotebook)
    {
        m_xNotebook->SetActivatePageHdl(LINK(this, SalInstanceNotebook, ActivatePageHdl));
        m_xNotebook->SetDeactivatePageHdl(LINK(this, SalInstanceNotebook, DeactivatePageHdl));
    }

    virtual int get_current_page() const override
    {
        return m_xNotebook->GetPagePos(m_xNotebook->GetCurPageId());
    }

    virtual OString get_current_page_ident() const override
    {
        return m_xNotebook->GetPageName(m_xNotebook->GetCurPageId());
    }

    virtual weld::Container* get_page(const OString& rIdent) const override
    {
        sal_uInt16 nPageId = m_xNotebook->GetPageId(rIdent);
        sal_uInt16 nPageIndex = m_xNotebook->GetPagePos(nPageId);
        if (nPageIndex == TAB_PAGE_NOTFOUND)
            return nullptr;
        TabPage* pPage = m_xNotebook->GetTabPage(nPageId);
        vcl::Window* pChild = pPage->GetChild(0);
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
        m_xNotebook->SetCurPageId(m_xNotebook->GetPageId(rIdent));
    }

    virtual void remove_page(const OString& rIdent) override
    {
        m_xNotebook->RemovePage(m_xNotebook->GetPageId(rIdent));
    }

    virtual void append_page(const OString& rIdent, const OUString& rLabel) override
    {
        sal_uInt16 nNewPageCount = m_xNotebook->GetPageCount() + 1;
        sal_uInt16 nNewPageId = nNewPageCount;
        m_xNotebook->InsertPage(nNewPageId, rLabel);
        VclPtrInstance<TabPage> xPage(m_xNotebook);
        VclPtrInstance<VclGrid> xGrid(xPage);
        xPage->Show();
        xGrid->set_hexpand(true);
        xGrid->set_vexpand(true);
        xGrid->Show();
        m_xNotebook->SetTabPage(nNewPageId, xPage);
        m_xNotebook->SetPageName(nNewPageId, rIdent);
        m_aAddedPages.push_back(xPage);
        m_aAddedGrids.push_back(xGrid);
    }

    virtual int get_n_pages() const override
    {
        return m_xNotebook->GetPageCount();
    }

    virtual OUString get_tab_label_text(const OString& rIdent) const override
    {
        return m_xNotebook->GetPageText(m_xNotebook->GetPageId(rIdent));
    }

    virtual ~SalInstanceNotebook() override
    {
        for (auto &rGrid : m_aAddedGrids)
            rGrid.disposeAndClear();
        for (auto &rPage : m_aAddedPages)
            rPage.disposeAndClear();
        m_xNotebook->SetActivatePageHdl(Link<TabControl*,void>());
        m_xNotebook->SetDeactivatePageHdl(Link<TabControl*,bool>());
    }
};

IMPL_LINK_NOARG(SalInstanceNotebook, DeactivatePageHdl, TabControl*, bool)
{
    return !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
}

IMPL_LINK_NOARG(SalInstanceNotebook, ActivatePageHdl, TabControl*, void)
{
    m_aEnterPageHdl.Call(get_current_page_ident());
}

class SalInstanceVerticalNotebook : public SalInstanceContainer, public virtual weld::Notebook
{
private:
    VclPtr<VerticalTabControl> m_xNotebook;
    mutable std::vector<std::unique_ptr<SalInstanceContainer>> m_aPages;

    DECL_LINK(DeactivatePageHdl, VerticalTabControl*, bool);
    DECL_LINK(ActivatePageHdl, VerticalTabControl*, void);

public:
    SalInstanceVerticalNotebook(VerticalTabControl* pNotebook, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pNotebook, pBuilder, bTakeOwnership)
        , m_xNotebook(pNotebook)
    {
        m_xNotebook->SetActivatePageHdl(LINK(this, SalInstanceVerticalNotebook, ActivatePageHdl));
        m_xNotebook->SetDeactivatePageHdl(LINK(this, SalInstanceVerticalNotebook, DeactivatePageHdl));
    }

    virtual int get_current_page() const override
    {
        return m_xNotebook->GetPagePos(m_xNotebook->GetCurPageId());
    }

    virtual OString get_current_page_ident() const override
    {
        return m_xNotebook->GetCurPageId();
    }

    virtual weld::Container* get_page(const OString& rIdent) const override
    {
        sal_uInt16 nPageIndex = m_xNotebook->GetPagePos(rIdent);
        if (nPageIndex == TAB_PAGE_NOTFOUND)
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
        m_xNotebook->RemovePage(rIdent);
    }

    virtual void append_page(const OString& rIdent, const OUString& rLabel) override
    {
        VclPtrInstance<VclGrid> xGrid(m_xNotebook->GetPageParent());
        xGrid->set_hexpand(true);
        xGrid->set_vexpand(true);
        m_xNotebook->InsertPage(rIdent, rLabel, Image(), "", xGrid);
    }

    virtual int get_n_pages() const override
    {
        return m_xNotebook->GetPageCount();
    }

    virtual OUString get_tab_label_text(const OString& rIdent) const override
    {
        return m_xNotebook->GetPageText(rIdent);
    }

    virtual ~SalInstanceVerticalNotebook() override
    {
        m_xNotebook->SetActivatePageHdl(Link<VerticalTabControl*,void>());
        m_xNotebook->SetDeactivatePageHdl(Link<VerticalTabControl*,bool>());
    }
};

IMPL_LINK_NOARG(SalInstanceVerticalNotebook, DeactivatePageHdl, VerticalTabControl*, bool)
{
    return !m_aLeavePageHdl.IsSet() || m_aLeavePageHdl.Call(get_current_page_ident());
}

IMPL_LINK_NOARG(SalInstanceVerticalNotebook, ActivatePageHdl, VerticalTabControl*, void)
{
    m_aEnterPageHdl.Call(get_current_page_ident());
}

class SalInstanceButton : public SalInstanceContainer, public virtual weld::Button
{
private:
    VclPtr<::Button> m_xButton;
    Link<::Button*,void> const m_aOldClickHdl;

    DECL_LINK(ClickHdl, ::Button*, void);
public:
    SalInstanceButton(::Button* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pButton, pBuilder, bTakeOwnership)
        , m_xButton(pButton)
        , m_aOldClickHdl(pButton->GetClickHdl())
    {
        m_xButton->SetClickHdl(LINK(this, SalInstanceButton, ClickHdl));
    }

    virtual void set_label(const OUString& rText) override
    {
        m_xButton->SetText(rText);
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        m_xButton->SetImageAlign(ImageAlign::Left);
        if (pDevice)
            m_xButton->SetModeImage(createImage(*pDevice));
        else
            m_xButton->SetModeImage(Image());
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        m_xButton->SetImageAlign(ImageAlign::Left);
        m_xButton->SetModeImage(Image(rImage));
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        m_xButton->SetModeImage(Image(StockImage::Yes, rIconName));
    }

    virtual void set_label_line_wrap(bool wrap) override
    {
        WinBits nBits = m_xButton->GetStyle();
        nBits &= ~WB_WORDBREAK;
        if (wrap)
            nBits |= WB_WORDBREAK;
        m_xButton->SetStyle(nBits);
        m_xButton->queue_resize();
    }

    virtual OUString get_label() const override
    {
        return m_xButton->GetText();
    }

    virtual ~SalInstanceButton() override
    {
        m_xButton->SetClickHdl(Link<::Button*,void>());
    }
};

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

weld::Button* SalInstanceDialog::get_widget_for_response(int nResponse)
{
    PushButton* pButton = dynamic_cast<PushButton*>(m_xDialog->get_widget_for_response(nResponse));
    return pButton ? new SalInstanceButton(pButton, nullptr, false) : nullptr;
}

class SalInstanceMenuButton : public SalInstanceButton, public virtual weld::MenuButton
{
private:
    VclPtr<::MenuButton> m_xMenuButton;
    sal_uInt16 m_nLastId;

    DECL_LINK(MenuSelectHdl, ::MenuButton*, void);
    DECL_LINK(ActivateHdl, ::MenuButton*, void);

public:
    SalInstanceMenuButton(::MenuButton* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
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
            m_nLastId = nCount ? pMenu->GetItemId(nCount-1) : 0;
        }
    }

    virtual void set_active(bool active) override
    {
        if (active == get_active())
            return;
        if (active)
            m_xMenuButton->ExecuteMenu();
        else
            m_xMenuButton->CancelMenu();
    }

    virtual bool get_active() const override
    {
        return m_xMenuButton->MenuShown();
    }

    virtual void set_inconsistent(bool /*inconsistent*/) override
    {
        //not available
    }

    virtual bool get_inconsistent() const override
    {
        return false;
    }

    virtual void insert_item(int pos, const OUString& rId, const OUString& rStr,
                             const OUString* pIconName, VirtualDevice* pImageSurface, bool bCheck) override
    {
        m_nLastId = insert_to_menu(m_nLastId, m_xMenuButton->GetPopupMenu(), pos, rId, rStr, pIconName, pImageSurface, bCheck);
    }

    virtual void insert_separator(int pos, const OUString& rId) override
    {
        auto nInsertPos = pos == -1 ? MENU_APPEND : pos;
        m_xMenuButton->GetPopupMenu()->InsertSeparator(rId.toUtf8(), nInsertPos);
    }

    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->EnableItem(rIdent, bSensitive);
    }

    virtual void remove_item(const OString& rId) override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->RemoveItem(pMenu->GetItemPos(pMenu->GetItemId(rId)));
    }

    virtual void clear() override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->Clear();
    }

    virtual void set_item_active(const OString& rIdent, bool bActive) override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->CheckItem(rIdent, bActive);
    }

    virtual void set_item_label(const OString& rIdent, const OUString& rText) override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->SetItemText(pMenu->GetItemId(rIdent), rText);
    }

    virtual OUString get_item_label(const OString& rIdent) const override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        return pMenu->GetItemText(pMenu->GetItemId(rIdent));
    }

    virtual void set_item_visible(const OString& rIdent, bool bShow) override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->ShowItem(pMenu->GetItemId(rIdent), bShow);
    }

    virtual void set_item_help_id(const OString& rIdent, const OString& rHelpId) override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->SetHelpId(pMenu->GetItemId(rIdent), rHelpId);
    }

    virtual OString get_item_help_id(const OString& rIdent) const override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        return pMenu->GetHelpId(pMenu->GetItemId(rIdent));
    }

    virtual void set_popover(weld::Widget* pPopover) override
    {
        SalInstanceWidget* pPopoverWidget = dynamic_cast<SalInstanceWidget*>(pPopover);
        m_xMenuButton->SetPopover(pPopoverWidget ? pPopoverWidget->getWidget() : nullptr);
    }

    virtual ~SalInstanceMenuButton() override
    {
        m_xMenuButton->SetSelectHdl(Link<::MenuButton*, void>());
        m_xMenuButton->SetActivateHdl(Link<::MenuButton*, void>());
    }
};

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

class SalInstanceLinkButton : public SalInstanceContainer, public virtual weld::LinkButton
{
private:
    VclPtr<FixedHyperlink> m_xButton;
    Link<FixedHyperlink&,void> m_aOrigClickHdl;

    DECL_LINK(ClickHdl, FixedHyperlink&, void);
public:
    SalInstanceLinkButton(FixedHyperlink* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pButton, pBuilder, bTakeOwnership)
        , m_xButton(pButton)
    {
        m_aOrigClickHdl = m_xButton->GetClickHdl();
        m_xButton->SetClickHdl(LINK(this, SalInstanceLinkButton, ClickHdl));
    }

    virtual void set_label(const OUString& rText) override
    {
        m_xButton->SetText(rText);
    }

    virtual OUString get_label() const override
    {
        return m_xButton->GetText();
    }

    virtual void set_uri(const OUString& rUri) override
    {
        m_xButton->SetURL(rUri);
    }

    virtual OUString get_uri() const override
    {
        return m_xButton->GetURL();
    }

    virtual ~SalInstanceLinkButton() override
    {
        m_xButton->SetClickHdl(m_aOrigClickHdl);
    }
};

IMPL_LINK(SalInstanceLinkButton, ClickHdl, FixedHyperlink&, rButton, void)
{
    m_aOrigClickHdl.Call(rButton);
    signal_clicked();
}

class SalInstanceRadioButton : public SalInstanceButton, public virtual weld::RadioButton
{
private:
    VclPtr<::RadioButton> m_xRadioButton;

    DECL_LINK(ToggleHdl, ::RadioButton&, void);

public:
    SalInstanceRadioButton(::RadioButton* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceButton(pButton, pBuilder, bTakeOwnership)
        , m_xRadioButton(pButton)
    {
        m_xRadioButton->SetToggleHdl(LINK(this, SalInstanceRadioButton, ToggleHdl));
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
        m_xRadioButton->Check(active);
        enable_notify_events();
    }

    virtual bool get_active() const override
    {
        return m_xRadioButton->IsChecked();
    }

    virtual void set_image(VirtualDevice* pDevice) override
    {
        m_xRadioButton->SetImageAlign(ImageAlign::Center);
        if (pDevice)
            m_xRadioButton->SetModeImage(createImage(*pDevice));
        else
            m_xRadioButton->SetModeImage(Image());
    }

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override
    {
        m_xRadioButton->SetImageAlign(ImageAlign::Center);
        m_xRadioButton->SetModeImage(Image(rImage));
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        m_xRadioButton->SetModeRadioImage(Image(StockImage::Yes, rIconName));
    }

    virtual void set_inconsistent(bool /*inconsistent*/) override
    {
        //not available
    }

    virtual bool get_inconsistent() const override
    {
        return false;
    }

    virtual ~SalInstanceRadioButton() override
    {
        m_xRadioButton->SetToggleHdl(Link<::RadioButton&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceRadioButton, ToggleHdl, ::RadioButton&, void)
{
    if (notify_events_disabled())
        return;
    signal_toggled();
}

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

    virtual void connect_toggled(const Link<ToggleButton&, void>& rLink) override
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

    virtual bool get_active() const override
    {
        return m_xToggleButton->IsChecked();
    }

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
            m_xToggleButton->RemoveEventListener(LINK(this, SalInstanceToggleButton, ToggleListener));
    }
};

IMPL_LINK(SalInstanceToggleButton, ToggleListener, VclWindowEvent&, rEvent, void)
{
    if (notify_events_disabled())
        return;
    if (rEvent.GetId() == VclEventId::PushbuttonToggle)
        signal_toggled();
}

class SalInstanceCheckButton : public SalInstanceButton, public virtual weld::CheckButton
{
private:
    VclPtr<CheckBox> m_xCheckButton;

    DECL_LINK(ToggleHdl, CheckBox&, void);
public:
    SalInstanceCheckButton(CheckBox* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceButton(pButton, pBuilder, bTakeOwnership)
        , m_xCheckButton(pButton)
    {
        m_xCheckButton->SetToggleHdl(LINK(this, SalInstanceCheckButton, ToggleHdl));
    }

    virtual void set_active(bool active) override
    {
        disable_notify_events();
        m_xCheckButton->EnableTriState(false);
        m_xCheckButton->Check(active);
        enable_notify_events();
    }

    virtual bool get_active() const override
    {
        return m_xCheckButton->IsChecked();
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
        disable_notify_events();
        m_xCheckButton->EnableTriState(true);
        m_xCheckButton->SetState(inconsistent ? TRISTATE_INDET : TRISTATE_FALSE);
        enable_notify_events();
    }

    virtual bool get_inconsistent() const override
    {
        return m_xCheckButton->GetState() == TRISTATE_INDET;
    }

    virtual ~SalInstanceCheckButton() override
    {
        m_xCheckButton->SetToggleHdl(Link<CheckBox&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceCheckButton, ToggleHdl, CheckBox&, void)
{
    if (notify_events_disabled())
        return;
    m_xCheckButton->EnableTriState(false);
    signal_toggled();
}

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

    virtual void set_value(int value) override
    {
        m_xScale->SetThumbPos(value);
    }

    virtual void set_range(int min, int max) override
    {
        m_xScale->SetRangeMin(min);
        m_xScale->SetRangeMax(max);
    }

    virtual int get_value() const override
    {
        return m_xScale->GetThumbPos();
    }

    virtual ~SalInstanceScale() override
    {
        m_xScale->SetSlideHdl(Link<Slider*, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceScale, SlideHdl, Slider*, void)
{
    signal_value_changed();
}

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

    virtual void start() override
    {
        m_xThrobber->start();
    }

    virtual void stop() override
    {
        m_xThrobber->stop();
    }
};

class SalInstanceProgressBar : public SalInstanceWidget, public virtual weld::ProgressBar
{
private:
    VclPtr<::ProgressBar> m_xProgressBar;

public:
    SalInstanceProgressBar(::ProgressBar* pProgressBar, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pProgressBar, pBuilder, bTakeOwnership)
        , m_xProgressBar(pProgressBar)
    {
    }

    virtual void set_percentage(int value) override
    {
        m_xProgressBar->SetValue(value);
    }

    virtual OUString get_text() const override
    {
        return m_xProgressBar->GetText();
    }

    virtual void set_text(const OUString& rText) override
    {
        m_xProgressBar->SetText(rText);
    }
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
        m_xImage->SetImage(createImage(*pDevice));
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

    virtual void set_date(const Date& rDate) override
    {
        m_xCalendar->SetCurDate(rDate);
    }

    virtual Date get_date() const override
    {
        return m_xCalendar->GetFirstSelectedDate();
    }

    virtual ~SalInstanceCalendar() override
    {
        m_xCalendar->SetSelectHdl(Link<::Calendar*, void>());
        m_xCalendar->SetActivateHdl(Link<::Calendar*, void>());
    }
};

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

namespace
{
    class WeldTextFilter : public TextFilter
    {
    private:
        Link<OUString&, bool>& m_rInsertTextHdl;
    public:
        WeldTextFilter(Link<OUString&, bool>& rInsertTextHdl)
            : TextFilter(OUString())
            , m_rInsertTextHdl(rInsertTextHdl)
        {
        }

        virtual OUString filter(const OUString &rText) override
        {
            if (!m_rInsertTextHdl.IsSet())
                return rText;
            OUString sText(rText);
            const bool bContinue = m_rInsertTextHdl.Call(sText);
            if (!bContinue)
                return OUString();
            return sText;
        }
    };
}

class SalInstanceEntry : public SalInstanceWidget, public virtual weld::Entry
{
private:
    VclPtr<Edit> m_xEntry;

    DECL_LINK(ChangeHdl, Edit&, void);
    DECL_LINK(CursorListener, VclWindowEvent&, void);
    DECL_LINK(ActivateHdl, Edit&, bool);

    WeldTextFilter m_aTextFilter;
public:
    SalInstanceEntry(Edit* pEntry, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pEntry, pBuilder, bTakeOwnership)
        , m_xEntry(pEntry)
        , m_aTextFilter(m_aInsertTextHdl)
    {
        m_xEntry->SetModifyHdl(LINK(this, SalInstanceEntry, ChangeHdl));
        m_xEntry->SetActivateHdl(LINK(this, SalInstanceEntry, ActivateHdl));
        m_xEntry->SetTextFilter(&m_aTextFilter);
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
        m_xEntry->SetText(rText);
        enable_notify_events();
    }

    virtual OUString get_text() const override
    {
        return m_xEntry->GetText();
    }

    virtual void set_width_chars(int nChars) override
    {
        m_xEntry->SetWidthInChars(nChars);
    }

    virtual int get_width_chars() const override
    {
        return m_xEntry->GetWidthInChars();
    }

    virtual void set_max_length(int nChars) override
    {
        m_xEntry->SetMaxTextLen(nChars);
    }

    virtual void select_region(int nStartPos, int nEndPos) override
    {
        disable_notify_events();
        m_xEntry->SetSelection(Selection(nStartPos, nEndPos < 0 ? SELECTION_MAX : nEndPos));
        enable_notify_events();
    }

    bool get_selection_bounds(int& rStartPos, int &rEndPos) override
    {
        const Selection& rSelection = m_xEntry->GetSelection();
        rStartPos = rSelection.Min();
        rEndPos = rSelection.Max();
        return rSelection.Len();
    }

    virtual void replace_selection(const OUString& rText) override
    {
        m_xEntry->ReplaceSelected(rText);
    }

    virtual void set_position(int nCursorPos) override
    {
        disable_notify_events();
        if (nCursorPos < 0)
            m_xEntry->SetCursorAtLast();
        else
            m_xEntry->SetSelection(Selection(nCursorPos, nCursorPos));
        enable_notify_events();
    }

    virtual int get_position() const override
    {
        return m_xEntry->GetSelection().Max();
    }

    virtual void set_editable(bool bEditable) override
    {
        m_xEntry->SetReadOnly(!bEditable);
    }

    virtual bool get_editable() const override
    {
        return !m_xEntry->IsReadOnly();
    }

    virtual void set_message_type(weld::EntryMessageType eType) override
    {
        if (eType == weld::EntryMessageType::Error)
        {
            // tdf#114603: enable setting the background to a different color;
            // relevant for GTK; see also #i75179#
            m_xEntry->SetForceControlBackground(true);
            m_xEntry->SetControlForeground(COL_WHITE);
            m_xEntry->SetControlBackground(0xff6563);
        }
        else if (eType == weld::EntryMessageType::Warning)
        {
            // tdf#114603: enable setting the background to a different color;
            // relevant for GTK; see also #i75179#
            m_xEntry->SetForceControlBackground(true);
            m_xEntry->SetControlForeground();
            m_xEntry->SetControlBackground(COL_YELLOW);
        }
        else
        {
            m_xEntry->SetForceControlBackground(false);
            m_xEntry->SetControlForeground();
            m_xEntry->SetControlBackground();
        }
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_xEntry->SetPointFont(*m_xEntry, rFont);
        m_xEntry->Invalidate();
    }

    virtual void connect_cursor_position(const Link<Entry&, void>& rLink) override
    {
        assert(!m_aCursorPositionHdl.IsSet());
        m_xEntry->AddEventListener(LINK(this, SalInstanceEntry, CursorListener));
        weld::Entry::connect_cursor_position(rLink);
    }

    Edit& getEntry()
    {
        return *m_xEntry;
    }

    void fire_signal_changed()
    {
        signal_changed();
    }

    virtual ~SalInstanceEntry() override
    {
        if (m_aCursorPositionHdl.IsSet())
            m_xEntry->RemoveEventListener(LINK(this, SalInstanceEntry, CursorListener));
        m_xEntry->SetTextFilter(nullptr);
        m_xEntry->SetActivateHdl(Link<Edit&, bool>());
        m_xEntry->SetModifyHdl(Link<Edit&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceEntry, ChangeHdl, Edit&, void)
{
    signal_changed();
}

IMPL_LINK(SalInstanceEntry, CursorListener, VclWindowEvent&, rEvent, void)
{
    if (notify_events_disabled())
        return;
    if (rEvent.GetId() == VclEventId::EditSelectionChanged || rEvent.GetId() == VclEventId::EditCaretChanged)
        signal_cursor_position();
}

IMPL_LINK_NOARG(SalInstanceEntry, ActivateHdl, Edit&, bool)
{
    return m_aActivateHdl.Call(*this);
}

struct SalInstanceTreeIter : public weld::TreeIter
{
    SalInstanceTreeIter(const SalInstanceTreeIter* pOrig)
        : iter(pOrig ? pOrig->iter : nullptr)
    {
    }
    SalInstanceTreeIter(SvTreeListEntry* pIter)
        : iter(pIter)
    {
    }
    virtual bool equal(const TreeIter& rOther) const override
    {
        return iter == static_cast<const SalInstanceTreeIter&>(rOther).iter;
    }
    SvTreeListEntry* iter;
};

namespace
{
    TriState get_toggle(SvTreeListEntry* pEntry, int col)
    {
        ++col; //skip dummy/expander column

        if (static_cast<size_t>(col) == pEntry->ItemCount())
            return TRISTATE_FALSE;

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxButton*>(&rItem));
        SvLBoxButton& rToggle = static_cast<SvLBoxButton&>(rItem);
        if (rToggle.IsStateTristate())
            return TRISTATE_INDET;
        else if (rToggle.IsStateChecked())
            return TRISTATE_TRUE;
        return TRISTATE_FALSE;
    }

    bool get_text_emphasis(SvTreeListEntry* pEntry, int col)
    {
        ++col; //skip dummy/expander column

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxString*>(&rItem));
        return static_cast<SvLBoxString&>(rItem).IsEmphasized();
    }
}

class SalInstanceTreeView;

static SalInstanceTreeView* g_DragSource;

class SalInstanceTreeView : public SalInstanceContainer, public virtual weld::TreeView
{
private:
    // owner for UserData
    std::vector<std::unique_ptr<OUString>> m_aUserData;
    VclPtr<SvTabListBox> m_xTreeView;
    SvLBoxButtonData m_aCheckButtonData;
    SvLBoxButtonData m_aRadioButtonData;
    bool m_bDisableCheckBoxAutoWidth;
    int m_nSortColumn;

    DECL_LINK(SelectHdl, SvTreeListBox*, void);
    DECL_LINK(DeSelectHdl, SvTreeListBox*, void);
    DECL_LINK(DoubleClickHdl, SvTreeListBox*, bool);
    DECL_LINK(ExpandingHdl, SvTreeListBox*, bool);
    DECL_LINK(EndDragHdl, HeaderBar*, void);
    DECL_LINK(HeaderBarClickedHdl, HeaderBar*, void);
    DECL_LINK(ToggleHdl, SvLBoxButtonData*, void);
    DECL_LINK(ModelChangedHdl, SvTreeListBox*, void);
    DECL_LINK(StartDragHdl, SvTreeListBox*, void);
    DECL_STATIC_LINK(SalInstanceTreeView, FinishDragHdl, SvTreeListBox*, void);
    DECL_LINK(EditingEntryHdl, SvTreeListEntry*, bool);
    typedef std::pair<SvTreeListEntry*, OUString> IterString;
    DECL_LINK(EditedEntryHdl, IterString, bool);
    DECL_LINK(VisibleRangeChangedHdl, SvTreeListBox*, void);
    DECL_LINK(CompareHdl, const SvSortData&, sal_Int32);
    DECL_LINK(PopupMenuHdl, const CommandEvent&, bool);

public:
    SalInstanceTreeView(SvTabListBox* pTreeView, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pTreeView, pBuilder, bTakeOwnership)
        , m_xTreeView(pTreeView)
        , m_aCheckButtonData(pTreeView, false)
        , m_aRadioButtonData(pTreeView, true)
        , m_bDisableCheckBoxAutoWidth(false)
        , m_nSortColumn(-1)
    {
        m_xTreeView->SetNodeDefaultImages();
        m_xTreeView->SetSelectHdl(LINK(this, SalInstanceTreeView, SelectHdl));
        m_xTreeView->SetDeselectHdl(LINK(this, SalInstanceTreeView, DeSelectHdl));
        m_xTreeView->SetDoubleClickHdl(LINK(this, SalInstanceTreeView, DoubleClickHdl));
        m_xTreeView->SetExpandingHdl(LINK(this, SalInstanceTreeView, ExpandingHdl));
        m_xTreeView->SetPopupMenuHdl(LINK(this, SalInstanceTreeView, PopupMenuHdl));
        const long aTabPositions[] = { 0 };
        m_xTreeView->SetTabs(SAL_N_ELEMENTS(aTabPositions), aTabPositions);
        LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());

        if (pHeaderBox)
        {
            if (HeaderBar* pHeaderBar = pHeaderBox->GetHeaderBar())
            {
                //make the last entry fill available space
                pHeaderBar->SetItemSize(pHeaderBar->GetItemId(pHeaderBar->GetItemCount() - 1 ), HEADERBAR_FULLSIZE);
                pHeaderBar->SetEndDragHdl(LINK(this, SalInstanceTreeView, EndDragHdl));
                pHeaderBar->SetSelectHdl(LINK(this, SalInstanceTreeView, HeaderBarClickedHdl));
            }
        }
        else
        {
            static_cast<LclTabListBox&>(*m_xTreeView).SetModelChangedHdl(LINK(this, SalInstanceTreeView, ModelChangedHdl));
            static_cast<LclTabListBox&>(*m_xTreeView).SetStartDragHdl(LINK(this, SalInstanceTreeView, StartDragHdl));
            static_cast<LclTabListBox&>(*m_xTreeView).SetEndDragHdl(LINK(this, SalInstanceTreeView, FinishDragHdl));
            static_cast<LclTabListBox&>(*m_xTreeView).SetEditingEntryHdl(LINK(this, SalInstanceTreeView, EditingEntryHdl));
            static_cast<LclTabListBox&>(*m_xTreeView).SetEditedEntryHdl(LINK(this, SalInstanceTreeView, EditedEntryHdl));
        }
        m_aCheckButtonData.SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
        m_aRadioButtonData.SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
    }

    virtual void columns_autosize() override
    {
        std::vector<long> aWidths;
        m_xTreeView->getPreferredDimensions(aWidths);
        if (aWidths.size() > 2)
        {
            std::vector<int> aColWidths;
            for (size_t i = 1; i < aWidths.size() - 1; ++i)
                aColWidths.push_back(aWidths[i] - aWidths[i - 1]);
            set_column_fixed_widths(aColWidths);
        }
    }

    virtual void freeze() override
    {
        SalInstanceWidget::freeze();
        m_xTreeView->SetUpdateMode(false);
    }

    virtual void thaw() override
    {
        m_xTreeView->SetUpdateMode(true);
        SalInstanceWidget::thaw();
    }

    virtual void set_column_fixed_widths(const std::vector<int>& rWidths) override
    {
        m_bDisableCheckBoxAutoWidth = true;
        std::vector<long> aTabPositions;
        aTabPositions.push_back(0);
        for (size_t i = 0; i < rWidths.size(); ++i)
            aTabPositions.push_back(aTabPositions[i] + rWidths[i]);
        m_xTreeView->SetTabs(aTabPositions.size(), aTabPositions.data(), MapUnit::MapPixel);
        LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            for (size_t i = 0; i < rWidths.size(); ++i)
                pHeaderBar->SetItemSize(pHeaderBar->GetItemId(i), rWidths[i]);
        }
        // call Resize to recalculate based on the new tabs
        m_xTreeView->Resize();
    }

    virtual int get_column_width(int nColumn) const override
    {
        LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
            return pHeaderBar->GetItemSize(pHeaderBar->GetItemId(nColumn));
        // GetTab(0) gives the position of the bitmap which is automatically inserted by the TabListBox.
        // So the first text column's width is Tab(2)-Tab(1).
        auto nWidthPixel = m_xTreeView->GetLogicTab(nColumn + 2) - m_xTreeView->GetLogicTab(nColumn + 1);
        nWidthPixel -= SV_TAB_BORDER;
        return nWidthPixel;
    }

    virtual OUString get_column_title(int nColumn) const override
    {
        LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            return pHeaderBar->GetItemText(pHeaderBar->GetItemId(nColumn));
        }
        return OUString();
    }

    virtual void set_column_title(int nColumn, const OUString& rTitle) override
    {
        LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            return pHeaderBar->SetItemText(pHeaderBar->GetItemId(nColumn), rTitle);
        }
    }

    virtual void show() override
    {
        if (LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get()))
            pHeaderBox->GetParent()->Show();
        SalInstanceContainer::show();
    }

    virtual void hide() override
    {
        if (LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get()))
            pHeaderBox->GetParent()->Hide();
        SalInstanceContainer::hide();
    }

    virtual void insert(const weld::TreeIter* pParent, int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        const OUString* pExpanderName, bool bChildrenOnDemand, weld::TreeIter* pRet) override
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
            pEntry->AddItem(std::make_unique<SvLBoxString>(*pStr));
        pEntry->SetUserData(pUserData);
        m_xTreeView->Insert(pEntry, iter, nInsertPos);

        if (pExpanderName)
        {
            Image aImage(createImage(*pExpanderName));
            m_xTreeView->SetExpandedEntryBmp(pEntry, aImage);
            m_xTreeView->SetCollapsedEntryBmp(pEntry, aImage);
        }

        if (pRet)
        {
            SalInstanceTreeIter* pVclRetIter = static_cast<SalInstanceTreeIter*>(pRet);
            pVclRetIter->iter = pEntry;
        }

        if (bChildrenOnDemand)
        {
            m_xTreeView->InsertEntry("<dummy>", pEntry, false, 0, nullptr);
        }
        enable_notify_events();
    }

    virtual void bulk_insert_for_each(int nSourceCount,
                                      const std::function<void(weld::TreeIter&, int nSourceIndex)>& func,
                                      const std::vector<int>* pFixedWidths) override
    {
        freeze();
        clear();
        SalInstanceTreeIter aVclIter(static_cast<SvTreeListEntry*>(nullptr));

        m_xTreeView->nTreeFlags |= SvTreeFlags::MANINS;

        if (pFixedWidths)
            set_column_fixed_widths(*pFixedWidths);

        for (int i = 0; i < nSourceCount; ++i)
        {
            aVclIter.iter = new SvTreeListEntry;
            m_xTreeView->Insert(aVclIter.iter, nullptr, TREELIST_APPEND);
            func(aVclIter, i);

            if (!pFixedWidths)
                continue;

            size_t nFixedWidths = std::min(pFixedWidths->size(), aVclIter.iter->ItemCount());
            for (size_t j = 0; j < nFixedWidths; ++j)
            {
                SvLBoxItem& rItem = aVclIter.iter->GetItem(j);
                SvViewDataItem* pViewDataItem = m_xTreeView->GetViewDataItem(aVclIter.iter, &rItem);
                pViewDataItem->mnWidth = (*pFixedWidths)[j];
            }
        }

        m_xTreeView->nTreeFlags &= ~SvTreeFlags::MANINS;

        thaw();
    }

    virtual void set_font_color(int pos, const Color& rColor) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        pEntry->SetTextColor(rColor);
    }

    virtual void set_font_color(const weld::TreeIter& rIter, const Color& rColor) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        rVclIter.iter->SetTextColor(rColor);
    }

    virtual void remove(int pos) override
    {
        disable_notify_events();
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        m_xTreeView->RemoveEntry(pEntry);
        enable_notify_events();
    }

    virtual int find_text(const OUString& rText) const override
    {
        for (SvTreeListEntry* pEntry = m_xTreeView->First(); pEntry; pEntry = m_xTreeView->Next(pEntry))
        {
            if (SvTabListBox::GetEntryText(pEntry, 0) == rText)
                return SvTreeList::GetRelPos(pEntry);
        }
        return -1;
    }

    virtual int find_id(const OUString& rId) const override
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

    virtual void set_top_entry(int pos) override
    {
        SvTreeList* pModel = m_xTreeView->GetModel();
        SvTreeListEntry* pEntry = pModel->GetEntry(nullptr, pos);
        pModel->Move(pEntry, nullptr, 0);
    }

    virtual void swap(int pos1, int pos2) override
    {
        SvTreeList* pModel = m_xTreeView->GetModel();
        SvTreeListEntry* pEntry1 = pModel->GetEntry(nullptr, pos1);
        SvTreeListEntry* pEntry2 = pModel->GetEntry(nullptr, pos2);
        pModel->Move(pEntry1, pEntry2);
    }

    virtual void clear() override
    {
        disable_notify_events();
        m_xTreeView->Clear();
        m_aUserData.clear();
        enable_notify_events();
    }

    virtual int n_children() const override
    {
        return m_xTreeView->GetModel()->GetChildList(nullptr).size();
    }

    virtual void select(int pos) override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't select when frozen");
        disable_notify_events();
        if (pos == -1)
            m_xTreeView->SelectAll(false);
        else
        {
            SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
            m_xTreeView->Select(pEntry, true);
            m_xTreeView->MakeVisible(pEntry);
        }
        enable_notify_events();
    }

    virtual int get_cursor_index() const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetCurEntry();
        if (!pEntry)
            return -1;
        return SvTreeList::GetRelPos(pEntry);
    }

    virtual void set_cursor(int pos) override
    {
        if (pos == -1)
            m_xTreeView->SetCurEntry(nullptr);
        else
        {
            SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
            m_xTreeView->SetCurEntry(pEntry);
        }
    }

    virtual void scroll_to_row(int pos) override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't select when frozen");
        disable_notify_events();
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        m_xTreeView->MakeVisible(pEntry);
        enable_notify_events();
    }

    virtual void unselect(int pos) override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't select when frozen");
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

    virtual std::vector<int> get_selected_rows() const override
    {
        std::vector<int> aRows;

        aRows.reserve(m_xTreeView->GetSelectionCount());
        for (SvTreeListEntry* pEntry = m_xTreeView->FirstSelected(); pEntry; pEntry = m_xTreeView->NextSelected(pEntry))
            aRows.push_back(SvTreeList::GetRelPos(pEntry));

        return aRows;
    }

    static OUString get_text(SvTreeListEntry* pEntry, int col)
    {
        if (col == -1)
            return SvTabListBox::GetEntryText(pEntry, 0);

        ++col; //skip dummy/expander column

        if (static_cast<size_t>(col) == pEntry->ItemCount())
            return OUString();

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxString*>(&rItem));
        return static_cast<SvLBoxString&>(rItem).GetText();
    }

    virtual OUString get_text(int pos, int col) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        return get_text(pEntry, col);
    }

    void set_text(SvTreeListEntry* pEntry, const OUString& rText, int col)
    {
        if (col == -1)
        {
            m_xTreeView->SetEntryText(pEntry, rText);
            return;
        }

        ++col; //skip dummy/expander column

        // blank out missing entries
        for (int i = pEntry->ItemCount(); i < col ; ++i)
            pEntry->AddItem(std::make_unique<SvLBoxString>(""));

        if (static_cast<size_t>(col) == pEntry->ItemCount())
        {
            pEntry->AddItem(std::make_unique<SvLBoxString>(rText));
            SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
            m_xTreeView->InitViewData(pViewData, pEntry);
        }
        else
        {
            assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
            SvLBoxItem& rItem = pEntry->GetItem(col);
            assert(dynamic_cast<SvLBoxString*>(&rItem));
            static_cast<SvLBoxString&>(rItem).SetText(rText);
        }
        m_xTreeView->ModelHasEntryInvalidated(pEntry);
    }

    virtual void set_text(int pos, const OUString& rText, int col) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        set_text(pEntry, rText, col);
    }

    void set_sensitive(SvTreeListEntry* pEntry, bool bSensitive, int col)
    {
        if (col == -1)
        {
            const sal_uInt16 nCount = pEntry->ItemCount();
            for (sal_uInt16 nCur = 0; nCur < nCount; ++nCur)
            {
                SvLBoxItem& rItem = pEntry->GetItem(nCur);
                if (rItem.GetType() == SvLBoxItemType::String)
                {
                    rItem.Enable(bSensitive);
                    m_xTreeView->ModelHasEntryInvalidated(pEntry);
                    break;
                }
            }
            return;
        }

        ++col; //skip dummy/expander column

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        rItem.Enable(bSensitive);

        m_xTreeView->ModelHasEntryInvalidated(pEntry);
    }

    using SalInstanceWidget::set_sensitive;

    virtual void set_sensitive(int pos, bool bSensitive, int col) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        set_sensitive(pEntry, bSensitive, col);
    }

    virtual TriState get_toggle(int pos, int col) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        return ::get_toggle(pEntry, col);
    }

    virtual TriState get_toggle(const weld::TreeIter& rIter, int col) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return ::get_toggle(rVclIter.iter, col);
    }

    void set_toggle(SvTreeListEntry* pEntry, TriState eState, int col)
    {
        bool bRadio = std::find(m_aRadioIndexes.begin(), m_aRadioIndexes.end(), col) != m_aRadioIndexes.end();
        ++col; //skip dummy/expander column

        // blank out missing entries
        for (int i = pEntry->ItemCount(); i < col ; ++i)
            pEntry->AddItem(std::make_unique<SvLBoxString>(""));

        if (static_cast<size_t>(col) == pEntry->ItemCount())
        {
            SvLBoxButtonData* pData = bRadio ? &m_aRadioButtonData : &m_aCheckButtonData;

            // if we want to have the implicit auto-sizing of the checkbox
            // column we need to call EnableCheckButton and CheckBoxInserted to
            // let it figure out that width. But we don't want to override any
            // explicitly set column width, so disable this if we've set
            // explicit column widths
            if (!m_bDisableCheckBoxAutoWidth)
            {
                if (!(m_xTreeView->GetTreeFlags() & SvTreeFlags::CHKBTN))
                {
                    m_xTreeView->EnableCheckButton(pData);
                    // EnableCheckButton clobbered this, restore it
                    pData->SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
                }
            }

            pEntry->AddItem(std::make_unique<SvLBoxButton>(SvLBoxButtonKind::EnabledCheckbox, pData));
            SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
            m_xTreeView->InitViewData(pViewData, pEntry);

            if (!m_bDisableCheckBoxAutoWidth)
                m_xTreeView->CheckBoxInserted(pEntry);
        }

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
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

        m_xTreeView->ModelHasEntryInvalidated(pEntry);
    }

    virtual void set_toggle(int pos, TriState eState, int col) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        set_toggle(pEntry, eState, col);
    }

    virtual void set_toggle(const weld::TreeIter& rIter, TriState eState, int col) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        set_toggle(rVclIter.iter, eState, col);
    }

    void set_text_emphasis(SvTreeListEntry* pEntry, bool bOn, int col)
    {
        ++col; //skip dummy/expander column

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxString*>(&rItem));
        static_cast<SvLBoxString&>(rItem).Emphasize(bOn);

        m_xTreeView->ModelHasEntryInvalidated(pEntry);
    }

    virtual void set_text_emphasis(const weld::TreeIter& rIter, bool bOn, int col) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        set_text_emphasis(rVclIter.iter, bOn, col);
    }

    virtual void set_text_emphasis(int pos, bool bOn, int col) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        set_text_emphasis(pEntry, bOn, col);
    }

    virtual bool get_text_emphasis(const weld::TreeIter& rIter, int col) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return ::get_text_emphasis(rVclIter.iter, col);
    }

    virtual bool get_text_emphasis(int pos, int col) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        return ::get_text_emphasis(pEntry, col);
    }

    virtual void connect_editing_started(const Link<const weld::TreeIter&, bool>& rLink) override
    {
        m_xTreeView->EnableInplaceEditing(true);
        weld::TreeView::connect_editing_started(rLink);
    }

    virtual void connect_editing_done(const Link<const std::pair<const weld::TreeIter&, OUString>&, bool>& rLink) override
    {
        m_xTreeView->EnableInplaceEditing(true);
        weld::TreeView::connect_editing_done(rLink);
    }

    void set_image(SvTreeListEntry* pEntry, const Image& rImage, int col)
    {
        if (col == -1)
        {
            m_xTreeView->SetExpandedEntryBmp(pEntry, rImage);
            m_xTreeView->SetCollapsedEntryBmp(pEntry, rImage);
            return;
        }

        ++col; //skip dummy/expander column

        // blank out missing entries
        for (int i = pEntry->ItemCount(); i < col ; ++i)
            pEntry->AddItem(std::make_unique<SvLBoxString>(""));

        if (static_cast<size_t>(col) == pEntry->ItemCount())
        {
            pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(rImage, rImage, false));
            SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
            m_xTreeView->InitViewData(pViewData, pEntry);
        }
        else
        {
            assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
            SvLBoxItem& rItem = pEntry->GetItem(col);
            assert(dynamic_cast<SvLBoxContextBmp*>(&rItem));
            static_cast<SvLBoxContextBmp&>(rItem).SetBitmap1(rImage);
            static_cast<SvLBoxContextBmp&>(rItem).SetBitmap2(rImage);
        }

        m_xTreeView->SetEntryHeight(pEntry);
        m_xTreeView->ModelHasEntryInvalidated(pEntry);
    }

    virtual void set_image(int pos, const OUString& rImage, int col) override
    {
        set_image(m_xTreeView->GetEntry(nullptr, pos), createImage(rImage), col);
    }

    virtual void set_image(int pos, const css::uno::Reference<css::graphic::XGraphic>& rImage, int col) override
    {
        set_image(m_xTreeView->GetEntry(nullptr, pos), Image(rImage), col);
    }

    virtual void set_image(int pos, VirtualDevice& rImage, int col) override
    {
        set_image(m_xTreeView->GetEntry(nullptr, pos), createImage(rImage), col);
    }

    virtual void set_image(const weld::TreeIter& rIter, const css::uno::Reference<css::graphic::XGraphic>& rImage, int col) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        set_image(rVclIter.iter, Image(rImage), col);
    }

    virtual void set_image(const weld::TreeIter& rIter, const OUString& rImage, int col) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        set_image(rVclIter.iter, createImage(rImage), col);
    }

    const OUString* getEntryData(int index) const
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, index);
        return pEntry ? static_cast<const OUString*>(pEntry->GetUserData()) : nullptr;
    }

    virtual OUString get_id(int pos) const override
    {
        const OUString* pRet = getEntryData(pos);
        if (!pRet)
            return OUString();
        return *pRet;
    }

    void set_id(SvTreeListEntry* pEntry, const OUString& rId)
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(rId));
        pEntry->SetUserData(m_aUserData.back().get());
    }

    virtual void set_id(int pos, const OUString& rId) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        set_id(pEntry, rId);
    }

    virtual int get_selected_index() const override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't request selection when frozen");
        SvTreeListEntry* pEntry = m_xTreeView->FirstSelected();
        if (!pEntry)
            return -1;
        return SvTreeList::GetRelPos(pEntry);
    }

    virtual OUString get_selected_text() const override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't request selection when frozen");
        if (SvTreeListEntry* pEntry = m_xTreeView->FirstSelected())
            return m_xTreeView->GetEntryText(pEntry);
        return OUString();
    }

    virtual OUString get_selected_id() const override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't request selection when frozen");
        if (SvTreeListEntry* pEntry = m_xTreeView->FirstSelected())
        {
            if (const OUString* pStr = static_cast<const OUString*>(pEntry->GetUserData()))
                return *pStr;
        }
        return OUString();
    }

    virtual std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig) const override
    {
        return std::unique_ptr<weld::TreeIter>(new SalInstanceTreeIter(static_cast<const SalInstanceTreeIter*>(pOrig)));
    }

    virtual void copy_iterator(const weld::TreeIter& rSource, weld::TreeIter& rDest) const override
    {
        const SalInstanceTreeIter& rVclSource(static_cast<const SalInstanceTreeIter&>(rSource));
        SalInstanceTreeIter& rVclDest(static_cast<SalInstanceTreeIter&>(rDest));
        rVclDest.iter = rVclSource.iter;
    }

    virtual bool get_selected(weld::TreeIter* pIter) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->FirstSelected();
        auto pVclIter = static_cast<SalInstanceTreeIter*>(pIter);
        if (pVclIter)
            pVclIter->iter = pEntry;
        return pEntry != nullptr;
    }

    virtual bool get_cursor(weld::TreeIter* pIter) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetCurEntry();
        auto pVclIter = static_cast<SalInstanceTreeIter*>(pIter);
        if (pVclIter)
            pVclIter->iter = pEntry;
        return pEntry != nullptr;
    }

    virtual void set_cursor(const weld::TreeIter& rIter) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        m_xTreeView->SetCurEntry(rVclIter.iter);
    }

    virtual bool get_iter_first(weld::TreeIter& rIter) const override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
        rVclIter.iter = m_xTreeView->GetEntry(0);
        return rVclIter.iter != nullptr;
    }

    virtual bool iter_next_sibling(weld::TreeIter& rIter) const override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
        rVclIter.iter = rVclIter.iter->NextSibling();
        return rVclIter.iter != nullptr;
    }

    virtual bool iter_next(weld::TreeIter& rIter) const override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
        rVclIter.iter = m_xTreeView->Next(rVclIter.iter);
        if (rVclIter.iter && m_xTreeView->GetEntryText(rVclIter.iter) == "<dummy>")
            return iter_next(rVclIter);
        return rVclIter.iter != nullptr;
    }

    virtual bool iter_children(weld::TreeIter& rIter) const override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
        rVclIter.iter = m_xTreeView->FirstChild(rVclIter.iter);
        bool bRet = rVclIter.iter != nullptr;
        if (bRet)
        {
            //on-demand dummy entry doesn't count
            return m_xTreeView->GetEntryText(rVclIter.iter) != "<dummy>";
        }
        return bRet;
    }

    virtual bool iter_parent(weld::TreeIter& rIter) const override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
        rVclIter.iter = m_xTreeView->GetParent(rVclIter.iter);
        return rVclIter.iter != nullptr;
    }

    virtual void remove(const weld::TreeIter& rIter) override
    {
        disable_notify_events();
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        m_xTreeView->RemoveEntry(rVclIter.iter);
        enable_notify_events();
    }

    virtual void select(const weld::TreeIter& rIter) override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't select when frozen");
        disable_notify_events();
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        m_xTreeView->Select(rVclIter.iter, true);
        enable_notify_events();
    }

    virtual void scroll_to_row(const weld::TreeIter& rIter) override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't select when frozen");
        disable_notify_events();
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        m_xTreeView->MakeVisible(rVclIter.iter);
        enable_notify_events();
    }

    virtual void unselect(const weld::TreeIter& rIter) override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't unselect when frozen");
        disable_notify_events();
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        m_xTreeView->Select(rVclIter.iter, false);
        enable_notify_events();
    }

    virtual int get_iter_depth(const weld::TreeIter& rIter) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return m_xTreeView->GetModel()->GetDepth(rVclIter.iter);
    }

    virtual bool iter_has_child(const weld::TreeIter& rIter) const override
    {
        weld::TreeIter& rNonConstIter = const_cast<weld::TreeIter&>(rIter);
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rNonConstIter);
        SvTreeListEntry* restore(rVclIter.iter);
        bool ret = iter_children(rNonConstIter);
        rVclIter.iter = restore;
        return ret;
    }

    virtual bool get_row_expanded(const weld::TreeIter& rIter) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return m_xTreeView->IsExpanded(rVclIter.iter);
    }

    virtual void expand_row(const weld::TreeIter& rIter) override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't expand when frozen");
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        if (!m_xTreeView->IsExpanded(rVclIter.iter) && signal_expanding(rIter))
            m_xTreeView->Expand(rVclIter.iter);
    }

    virtual void collapse_row(const weld::TreeIter& rIter) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        if (m_xTreeView->IsExpanded(rVclIter.iter))
            m_xTreeView->Collapse(rVclIter.iter);
    }

    virtual OUString get_text(const weld::TreeIter& rIter, int col) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return get_text(rVclIter.iter, col);
    }

    virtual void set_text(const weld::TreeIter& rIter, const OUString& rText, int col) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        set_text(rVclIter.iter, rText, col);
    }

    virtual OUString get_id(const weld::TreeIter& rIter) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        const OUString* pStr = static_cast<const OUString*>(rVclIter.iter->GetUserData());
        if (pStr)
            return *pStr;
        return OUString();
    }

    virtual void set_id(const weld::TreeIter& rIter, const OUString& rId) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        set_id(rVclIter.iter, rId);
    }

    virtual void set_selection_mode(SelectionMode eMode) override
    {
        m_xTreeView->SetSelectionMode(eMode);
    }

    virtual void all_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        SalInstanceTreeIter aVclIter(m_xTreeView->First());
        while (aVclIter.iter)
        {
            if (func(aVclIter))
                return;
            aVclIter.iter = m_xTreeView->Next(aVclIter.iter);
        }
    }

    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        SalInstanceTreeIter aVclIter(m_xTreeView->FirstSelected());
        while (aVclIter.iter)
        {
            if (func(aVclIter))
                return;
            aVclIter.iter = m_xTreeView->NextSelected(aVclIter.iter);
        }
    }

    virtual void visible_foreach(const std::function<bool(weld::TreeIter&)>& func) override
    {
        SalInstanceTreeIter aVclIter(m_xTreeView->GetFirstEntryInView());
        while (aVclIter.iter)
        {
            if (func(aVclIter))
                return;
            aVclIter.iter = m_xTreeView->GetNextEntryInView(aVclIter.iter);
        }
    }

    virtual void connect_visible_range_changed(const Link<weld::TreeView&, void>& rLink) override
    {
        weld::TreeView::connect_visible_range_changed(rLink);
        m_xTreeView->SetScrolledHdl(LINK(this, SalInstanceTreeView, VisibleRangeChangedHdl));
    }

    virtual void remove_selection() override
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

    virtual bool is_selected(const weld::TreeIter& rIter) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return m_xTreeView->IsSelected(rVclIter.iter);
    }

    virtual int get_iter_index_in_parent(const weld::TreeIter& rIter) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return SvTreeList::GetRelPos(rVclIter.iter);
    }

    virtual int iter_compare(const weld::TreeIter& a, const weld::TreeIter& b) const override
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

    virtual void move_subtree(weld::TreeIter& rNode, const weld::TreeIter* pNewParent, int nIndexInNewParent) override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rNode);
        const SalInstanceTreeIter* pVclParentIter = static_cast<const SalInstanceTreeIter*>(pNewParent);
        m_xTreeView->GetModel()->Move(rVclIter.iter, pVclParentIter ? pVclParentIter->iter : nullptr, nIndexInNewParent);
    }

    virtual int count_selected_rows() const override
    {
        return m_xTreeView->GetSelectionCount();
    }

    virtual int get_height_rows(int nRows) const override
    {
        return m_xTreeView->GetEntryHeight() * nRows;
    }

    virtual void make_sorted() override
    {
        m_xTreeView->SetStyle(m_xTreeView->GetStyle() | WB_SORT);
        m_xTreeView->GetModel()->SetCompareHdl(LINK(this, SalInstanceTreeView, CompareHdl));
        set_sort_order(true);
    }

    virtual void set_sort_func(const std::function<int(const weld::TreeIter&, const weld::TreeIter&)>& func) override
    {
        weld::TreeView::set_sort_func(func);
        SvTreeList* pListModel = m_xTreeView->GetModel();
        pListModel->Resort();
    }

    virtual void make_unsorted() override
    {
        m_xTreeView->SetStyle(m_xTreeView->GetStyle() & ~WB_SORT);
    }

    virtual void set_sort_order(bool bAscending) override
    {
        SvTreeList* pListModel = m_xTreeView->GetModel();
        pListModel->SetSortMode(bAscending ? SortAscending : SortDescending);
        pListModel->Resort();
    }

    virtual bool get_sort_order() const override
    {
        return m_xTreeView->GetModel()->GetSortMode() == SortAscending;
    }

    virtual void set_sort_indicator(TriState eState, int col) override
    {
        if (col == -1)
            col = 0;

        LclHeaderTabListBox* pHeaderBox = dynamic_cast<LclHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
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
    }

    virtual TriState get_sort_indicator(int col) const override
    {
        if (col == -1)
            col = 0;

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

    virtual int get_sort_column() const override
    {
        return m_nSortColumn;
    }

    virtual void set_sort_column(int nColumn) override
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

    SvTabListBox& getTreeView()
    {
        return *m_xTreeView;
    }

    virtual bool get_dest_row_at_pos(const Point &rPos, weld::TreeIter* pResult) override
    {
        SvTreeListEntry* pTarget = m_xTreeView->GetDropTarget(rPos);

        if (pTarget && pResult)
        {
            SalInstanceTreeIter& rSalIter = static_cast<SalInstanceTreeIter&>(*pResult);
            rSalIter.iter = pTarget;
        }

        return pTarget != nullptr;
    }

    virtual TreeView* get_drag_source() const override
    {
        return g_DragSource;
    }

    virtual ~SalInstanceTreeView() override
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
            static_cast<LclTabListBox&>(*m_xTreeView).SetStartDragHdl(Link<SvTreeListBox*, void>());
            static_cast<LclTabListBox&>(*m_xTreeView).SetModelChangedHdl(Link<SvTreeListBox*, void>());
        }
        m_xTreeView->SetPopupMenuHdl(Link<const CommandEvent&, bool>());
        m_xTreeView->SetExpandingHdl(Link<SvTreeListBox*, bool>());
        m_xTreeView->SetDoubleClickHdl(Link<SvTreeListBox*, bool>());
        m_xTreeView->SetSelectHdl(Link<SvTreeListBox*, void>());
        m_xTreeView->SetDeselectHdl(Link<SvTreeListBox*, void>());
        m_xTreeView->SetScrolledHdl(Link<SvTreeListBox*, void>());
    }
};

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
        size_t col = m_nSortColumn;

        ++col; //skip dummy/expander column

        if (col < pLHS->ItemCount())
        {
            const SvLBoxString& rLeftTextItem = static_cast<const SvLBoxString&>(pLHS->GetItem(col));
            pLeftTextItem = &rLeftTextItem;
        }
        else
            pLeftTextItem = nullptr;
        if (col < pRHS->ItemCount())
        {
            const SvLBoxString& rRightTextItem = static_cast<const SvLBoxString&>(pRHS->GetItem(col));
            pRightTextItem = &rRightTextItem;
        }
        else
            pRightTextItem = nullptr;
    }
    else
    {
        pLeftTextItem = static_cast<const SvLBoxString*>(pLHS->GetFirstItem(SvLBoxItemType::String));
        pRightTextItem = static_cast<const SvLBoxString*>(pRHS->GetFirstItem(SvLBoxItemType::String));
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

IMPL_LINK_NOARG(SalInstanceTreeView, StartDragHdl, SvTreeListBox*, void)
{
    g_DragSource = this;
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

    // toggled signal handlers can query get_cursor to get which
    // node was clicked
    m_xTreeView->pImpl->m_pCursor = pEntry;

    for (int i = 1, nCount = pEntry->ItemCount(); i < nCount; ++i)
    {
        SvLBoxItem& rItem = pEntry->GetItem(i);
        if (&rItem == pBox)
        {
            int nRow = SvTreeList::GetRelPos(pEntry);
            int nCol = i - 1; // less dummy/expander column
            signal_toggled(std::make_pair(nRow, nCol));
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
    if (m_xTreeView->GetSelectionMode() == SelectionMode::Single)
        return;
    signal_changed();
}

IMPL_LINK_NOARG(SalInstanceTreeView, DoubleClickHdl, SvTreeListBox*, bool)
{
    if (notify_events_disabled())
        return false;
    signal_row_activated();
    return false;
}

IMPL_LINK(SalInstanceTreeView, EndDragHdl, HeaderBar*, pHeaderBar, void)
{
    std::vector<long> aTabPositions;
    aTabPositions.push_back(0);
    for (int i = 0; i < pHeaderBar->GetItemCount() - 1; ++i)
        aTabPositions.push_back(aTabPositions[i] + pHeaderBar->GetItemSize(pHeaderBar->GetItemId(i)));
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
    if (m_xTreeView->IsExpanded(pEntry))
    {
        //collapsing;
        return true;
    }

    // if there's a preexisting placeholder child, required to make this
    // potentially expandable in the first place, now we remove it
    bool bPlaceHolder = false;
    if (pEntry->HasChildren())
    {
        auto pChild = m_xTreeView->FirstChild(pEntry);
        assert(pChild);
        if (m_xTreeView->GetEntryText(pChild) == "<dummy>")
        {
            m_xTreeView->RemoveEntry(pChild);
            bPlaceHolder = true;
        }
    }

    SalInstanceTreeIter aIter(pEntry);
    bool bRet = signal_expanding(aIter);

    //expand disallowed, restore placeholder
    if (!bRet && bPlaceHolder)
    {
        m_xTreeView->InsertEntry("<dummy>", pEntry, false, 0, nullptr);
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
    return signal_editing_done(std::pair<const weld::TreeIter&, OUString>(SalInstanceTreeIter(rIterString.first), rIterString.second));
}

class SalInstanceSpinButton : public SalInstanceEntry, public virtual weld::SpinButton
{
private:
    VclPtr<FormattedField> m_xButton;

    DECL_LINK(UpDownHdl, SpinField&, void);
    DECL_LINK(LoseFocusHdl, Control&, void);
    DECL_LINK(OutputHdl, Edit&, bool);
    DECL_LINK(InputHdl, sal_Int64*, TriState);
    DECL_LINK(ActivateHdl, Edit&, bool);

    double toField(int nValue) const
    {
        return static_cast<double>(nValue) / Power10(get_digits());
    }

    int fromField(double fValue) const
    {
        return FRound(fValue * Power10(get_digits()));
    }

public:
    SalInstanceSpinButton(FormattedField* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceEntry(pButton, pBuilder, bTakeOwnership)
        , m_xButton(pButton)
    {
        m_xButton->SetThousandsSep(false);  //off by default, MetricSpinButton enables it
        m_xButton->SetUpHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_xButton->SetDownHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_xButton->SetLoseFocusHdl(LINK(this, SalInstanceSpinButton, LoseFocusHdl));
        m_xButton->SetOutputHdl(LINK(this, SalInstanceSpinButton, OutputHdl));
        m_xButton->SetInputHdl(LINK(this, SalInstanceSpinButton, InputHdl));
        if (Edit* pEdit = m_xButton->GetSubEdit())
            pEdit->SetActivateHdl(LINK(this, SalInstanceSpinButton, ActivateHdl));
        else
            m_xButton->SetActivateHdl(LINK(this, SalInstanceSpinButton, ActivateHdl));
    }

    virtual int get_value() const override
    {
        return fromField(m_xButton->GetValue());
    }

    virtual void set_value(int value) override
    {
        m_xButton->SetValue(toField(value));
    }

    virtual void set_range(int min, int max) override
    {
        m_xButton->SetMinValue(toField(min));
        m_xButton->SetMaxValue(toField(max));
    }

    virtual void get_range(int& min, int& max) const override
    {
        min = fromField(m_xButton->GetMinValue());
        max = fromField(m_xButton->GetMaxValue());
    }

    virtual void set_increments(int step, int /*page*/) override
    {
        m_xButton->SetSpinSize(toField(step));
    }

    virtual void get_increments(int& step, int& page) const override
    {
        step = fromField(m_xButton->GetSpinSize());
        page = fromField(m_xButton->GetSpinSize());
    }

    virtual void set_digits(unsigned int digits) override
    {
        m_xButton->SetDecimalDigits(digits);
    }

    //so with hh::mm::ss, incrementing mm will not reset ss
    void DisableRemainderFactor()
    {
        m_xButton->DisableRemainderFactor();
    }

    //off by default for direct SpinButtons, MetricSpinButton enables it
    void SetUseThousandSep()
    {
        m_xButton->SetThousandsSep(true);
    }

    virtual unsigned int get_digits() const override
    {
        return m_xButton->GetDecimalDigits();
    }

    virtual ~SalInstanceSpinButton() override
    {
        if (Edit* pEdit = m_xButton->GetSubEdit())
            pEdit->SetActivateHdl(Link<Edit&, bool>());
        else
            m_xButton->SetActivateHdl(Link<Edit&, bool>());
        m_xButton->SetInputHdl(Link<sal_Int64*, TriState>());
        m_xButton->SetOutputHdl(Link<Edit&, bool>());
        m_xButton->SetLoseFocusHdl(Link<Control&, void>());
        m_xButton->SetDownHdl(Link<SpinField&, void>());
        m_xButton->SetUpHdl(Link<SpinField&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceSpinButton, ActivateHdl, Edit&, bool)
{
    // tdf#122348 return pressed to end dialog
    signal_value_changed();
    return false;
}

IMPL_LINK_NOARG(SalInstanceSpinButton, UpDownHdl, SpinField&, void)
{
    signal_value_changed();
}

IMPL_LINK_NOARG(SalInstanceSpinButton, LoseFocusHdl, Control&, void)
{
    signal_value_changed();
}

IMPL_LINK_NOARG(SalInstanceSpinButton, OutputHdl, Edit&, bool)
{
    return signal_output();
}

IMPL_LINK(SalInstanceSpinButton, InputHdl, sal_Int64*, pResult, TriState)
{
    int nResult;
    TriState eRet = signal_input(&nResult);
    if (eRet == TRISTATE_TRUE)
        *pResult = nResult;
    return eRet;
}

class SalInstanceFormattedSpinButton : public SalInstanceEntry, public virtual weld::FormattedSpinButton
{
private:
    VclPtr<FormattedField> m_xButton;

public:
    SalInstanceFormattedSpinButton(FormattedField* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceEntry(pButton, pBuilder, bTakeOwnership)
        , m_xButton(pButton)
    {
        // #i6278# allow more decimal places than the output format.  As
        // the numbers shown in the edit fields are used for input, it makes more
        // sense to display the values in the input format rather than the output
        // format.
        m_xButton->UseInputStringForFormatting();
    }

    virtual double get_value() const override
    {
        return m_xButton->GetValue();
    }

    virtual void set_value(double value) override
    {
        m_xButton->SetValue(value);
    }

    virtual void set_range(double min, double max) override
    {
        m_xButton->SetMinValue(min);
        m_xButton->SetMaxValue(max);
    }

    virtual void get_range(double& min, double& max) const override
    {
        min = m_xButton->GetMinValue();
        max = m_xButton->GetMaxValue();
    }

    virtual void set_formatter(SvNumberFormatter* pFormatter) override
    {
        m_xButton->SetFormatter(pFormatter);
    }

    virtual sal_Int32 get_format_key() const override
    {
        return m_xButton->GetFormatKey();
    }

    virtual void set_format_key(sal_Int32 nFormatKey) override
    {
        m_xButton->SetFormatKey(nFormatKey);
    }
};

class SalInstanceLabel : public SalInstanceWidget, public virtual weld::Label
{
private:
    // Control instead of FixedText so we can also use this for
    // SelectableFixedText which is derived from Edit. We just typically need
    // [G|S]etText which exists in their shared baseclass
    VclPtr<Control> m_xLabel;
public:
    SalInstanceLabel(Control* pLabel, SalInstanceBuilder *pBuilder, bool bTakeOwnership)
        : SalInstanceWidget(pLabel, pBuilder, bTakeOwnership)
        , m_xLabel(pLabel)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        m_xLabel->SetText(rText);
    }

    virtual OUString get_label() const override
    {
        return m_xLabel->GetText();
    }

    virtual void set_mnemonic_widget(Widget* pTarget) override
    {
        FixedText* pLabel = dynamic_cast<FixedText*>(m_xLabel.get());
        assert(pLabel && "can't use set_mnemonic_widget on SelectableFixedText");
        SalInstanceWidget* pTargetWidget = dynamic_cast<SalInstanceWidget*>(pTarget);
        pLabel->set_mnemonic_widget(pTargetWidget ? pTargetWidget->getWidget() : nullptr);
    }

    virtual void set_message_type(weld::EntryMessageType eType) override
    {
        if (eType == weld::EntryMessageType::Error)
            m_xLabel->SetControlBackground(m_xLabel->GetSettings().GetStyleSettings().GetHighlightColor());
        else if (eType == weld::EntryMessageType::Warning)
            m_xLabel->SetControlBackground(COL_YELLOW);
        else
            m_xLabel->SetControlBackground();
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_xLabel->SetPointFont(*m_xLabel, rFont);
        m_xLabel->Invalidate();
    }
};

std::unique_ptr<weld::Label> SalInstanceFrame::weld_label_widget() const
{
    FixedText* pLabel = dynamic_cast<FixedText*>(m_xFrame->get_label_widget());
    if (!pLabel)
        return nullptr;
    return std::make_unique<SalInstanceLabel>(pLabel, m_pBuilder, false);
}

class SalInstanceTextView : public SalInstanceContainer, public virtual weld::TextView
{
private:
    VclPtr<VclMultiLineEdit> m_xTextView;
    Link<ScrollBar*,void> m_aOrigVScrollHdl;

    DECL_LINK(ChangeHdl, Edit&, void);
    DECL_LINK(VscrollHdl, ScrollBar*, void);
    DECL_LINK(CursorListener, VclWindowEvent&, void);
public:
    SalInstanceTextView(VclMultiLineEdit* pTextView, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pTextView, pBuilder, bTakeOwnership)
        , m_xTextView(pTextView)
    {
        m_xTextView->SetModifyHdl(LINK(this, SalInstanceTextView, ChangeHdl));
        ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
        m_aOrigVScrollHdl = rVertScrollBar.GetScrollHdl();
        rVertScrollBar.SetScrollHdl(LINK(this, SalInstanceTextView, VscrollHdl));
    }

    virtual void set_text(const OUString& rText) override
    {
        disable_notify_events();
        m_xTextView->SetText(rText);
        enable_notify_events();
    }

    virtual void replace_selection(const OUString& rText) override
    {
        disable_notify_events();
        m_xTextView->ReplaceSelected(rText);
        enable_notify_events();
    }

    virtual OUString get_text() const override
    {
        return m_xTextView->GetText();
    }

    bool get_selection_bounds(int& rStartPos, int &rEndPos) override
    {
        const Selection& rSelection = m_xTextView->GetSelection();
        rStartPos = rSelection.Min();
        rEndPos = rSelection.Max();
        return rSelection.Len();
    }

    virtual void select_region(int nStartPos, int nEndPos) override
    {
        disable_notify_events();
        m_xTextView->SetSelection(Selection(nStartPos, nEndPos < 0 ? SELECTION_MAX : nEndPos));
        enable_notify_events();
    }

    virtual void set_editable(bool bEditable) override
    {
        m_xTextView->SetReadOnly(!bEditable);
    }

    virtual void set_monospace(bool bMonospace) override
    {
        vcl::Font aOrigFont = m_xTextView->GetControlFont();
        vcl::Font aFont;
        if (bMonospace)
            aFont = OutputDevice::GetDefaultFont(DefaultFontType::UI_FIXED, LANGUAGE_DONTKNOW, GetDefaultFontFlags::OnlyOne, m_xTextView);
        else
            aFont = Application::GetSettings().GetStyleSettings().GetFieldFont();
        aFont.SetFontHeight(aOrigFont.GetFontHeight());
        m_xTextView->SetFont(aFont);
        m_xTextView->SetControlFont(aFont);
    }

    virtual void connect_cursor_position(const Link<TextView&, void>& rLink) override
    {
        assert(!m_aCursorPositionHdl.IsSet());
        m_xTextView->AddEventListener(LINK(this, SalInstanceTextView, CursorListener));
        weld::TextView::connect_cursor_position(rLink);
    }

    virtual int vadjustment_get_value() const override
    {
        ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
        return rVertScrollBar.GetThumbPos();
    }

    virtual void vadjustment_set_value(int value) override
    {
        ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
        rVertScrollBar.SetThumbPos(value);
        m_aOrigVScrollHdl.Call(&rVertScrollBar);
    }

    virtual int vadjustment_get_upper() const override
    {
        ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
        return rVertScrollBar.GetRangeMax();
    }

    virtual int vadjustment_get_lower() const override
    {
        ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
        return rVertScrollBar.GetRangeMin();
    }

    virtual int vadjustment_get_page_size() const override
    {
        ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
        return rVertScrollBar.GetVisibleSize();
    }

    virtual ~SalInstanceTextView() override
    {
        if (!m_xTextView->IsDisposed())
        {
            if (m_aCursorPositionHdl.IsSet())
                m_xTextView->RemoveEventListener(LINK(this, SalInstanceTextView, CursorListener));
            m_xTextView->SetModifyHdl(Link<Edit&, void>());
            ScrollBar& rVertScrollBar = m_xTextView->GetVScrollBar();
            rVertScrollBar.SetScrollHdl(m_aOrigVScrollHdl);
        }
    }
};

IMPL_LINK(SalInstanceTextView, VscrollHdl, ScrollBar*, pScrollBar, void)
{
    signal_vadjustment_changed();
    m_aOrigVScrollHdl.Call(pScrollBar);
}

IMPL_LINK_NOARG(SalInstanceTextView, ChangeHdl, Edit&, void)
{
    signal_changed();
}

IMPL_LINK(SalInstanceTextView, CursorListener, VclWindowEvent&, rEvent, void)
{
    if (notify_events_disabled())
        return;
    if (rEvent.GetId() == VclEventId::EditSelectionChanged || rEvent.GetId() == VclEventId::EditCaretChanged)
        signal_cursor_position();
}

class SalInstanceExpander : public SalInstanceContainer, public virtual weld::Expander
{
private:
    VclPtr<VclExpander> m_xExpander;

    DECL_LINK(ExpandedHdl, VclExpander&, void);

public:
    SalInstanceExpander(VclExpander* pExpander, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pExpander, pBuilder, bTakeOwnership)
        , m_xExpander(pExpander)
    {
        m_xExpander->SetExpandedHdl(LINK(this, SalInstanceExpander, ExpandedHdl));
    }

    virtual bool get_expanded() const override
    {
        return m_xExpander->get_expanded();
    }

    virtual void set_expanded(bool bExpand) override
    {
        m_xExpander->set_expanded(bExpand);
    }

    virtual ~SalInstanceExpander() override
    {
        m_xExpander->SetExpandedHdl(Link<VclExpander&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceExpander, ExpandedHdl, VclExpander&, void)
{
    signal_expanded();
}

class SalInstanceDrawingArea : public SalInstanceWidget, public virtual weld::DrawingArea
{
private:
    VclPtr<VclDrawingArea> m_xDrawingArea;

    typedef std::pair<vcl::RenderContext&, const tools::Rectangle&> target_and_area;
    DECL_LINK(PaintHdl, target_and_area, void);
    DECL_LINK(ResizeHdl, const Size&, void);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(MouseMoveHdl, const MouseEvent&, bool);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, bool);
    DECL_LINK(KeyPressHdl, const KeyEvent&, bool);
    DECL_LINK(KeyReleaseHdl, const KeyEvent&, bool);
    DECL_LINK(StyleUpdatedHdl, VclDrawingArea&, void);
    DECL_LINK(PopupMenuHdl, const CommandEvent&, bool);
    DECL_LINK(QueryTooltipHdl, tools::Rectangle&, OUString);

    // SalInstanceWidget has a generic listener for all these
    // events, ignore the ones we have specializations for
    // in VclDrawingArea
    virtual void HandleEventListener(VclWindowEvent& rEvent) override
    {
        if (rEvent.GetId() == VclEventId::WindowResize)
            return;
        SalInstanceWidget::HandleEventListener(rEvent);
    }

    virtual void HandleMouseEventListener(VclSimpleEvent& rEvent) override
    {
        if (rEvent.GetId() == VclEventId::WindowMouseButtonDown ||
            rEvent.GetId() == VclEventId::WindowMouseButtonUp ||
            rEvent.GetId() == VclEventId::WindowMouseMove)
        {
            return;
        }
        SalInstanceWidget::HandleMouseEventListener(rEvent);
    }


    virtual bool HandleKeyEventListener(VclWindowEvent& /*rEvent*/) override
    {
        return false;
    }

public:
    SalInstanceDrawingArea(VclDrawingArea* pDrawingArea, SalInstanceBuilder* pBuilder, const a11yref& rAlly,
            FactoryFunction pUITestFactoryFunction, void* pUserData, bool bTakeOwnership)
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
        m_xDrawingArea->SetPopupMenuHdl(LINK(this, SalInstanceDrawingArea, PopupMenuHdl));
        m_xDrawingArea->SetQueryTooltipHdl(LINK(this, SalInstanceDrawingArea, QueryTooltipHdl));
    }

    virtual void queue_draw() override
    {
        m_xDrawingArea->Invalidate();
    }

    virtual void queue_draw_area(int x, int y, int width, int height) override
    {
        m_xDrawingArea->Invalidate(tools::Rectangle(Point(x, y), Size(width, height)));
    }

    virtual void queue_resize() override
    {
        m_xDrawingArea->queue_resize();
    }

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override
    {
        weld::Widget::connect_size_allocate(rLink);
    }

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override
    {
        weld::Widget::connect_key_press(rLink);
    }

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override
    {
        weld::Widget::connect_key_release(rLink);
    }

    virtual void set_cursor(PointerStyle ePointerStyle) override
    {
        m_xDrawingArea->SetPointer(ePointerStyle);
    }

    virtual a11yref get_accessible_parent() override
    {
        vcl::Window* pParent = m_xDrawingArea->GetParent();
        if (pParent)
            return pParent->GetAccessible();
        return css::uno::Reference<css::accessibility::XAccessible>();
    }

    virtual a11yrelationset get_accessible_relation_set() override
    {
        utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
        css::uno::Reference< css::accessibility::XAccessibleRelationSet > xSet = pRelationSetHelper;
        vcl::Window* pWindow = m_xDrawingArea.get();
        if (pWindow)
        {
            vcl::Window *pLabeledBy = pWindow->GetAccessibleRelationLabeledBy();
            if (pLabeledBy && pLabeledBy != pWindow)
            {
                css::uno::Sequence<css::uno::Reference<css::uno::XInterface>> aSequence { pLabeledBy->GetAccessible() };
                pRelationSetHelper->AddRelation( css::accessibility::AccessibleRelation( css::accessibility::AccessibleRelationType::LABELED_BY, aSequence ) );
            }
            vcl::Window* pMemberOf = pWindow->GetAccessibleRelationMemberOf();
            if (pMemberOf && pMemberOf != pWindow)
            {
                css::uno::Sequence<css::uno::Reference<css::uno::XInterface>> aSequence { pMemberOf->GetAccessible() };
                pRelationSetHelper->AddRelation( css::accessibility::AccessibleRelation( css::accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );
            }
        }
        return xSet;
    }

    virtual Point get_accessible_location() override
    {
        return m_xDrawingArea->OutputToAbsoluteScreenPixel(Point());
    }

    virtual ~SalInstanceDrawingArea() override
    {
        m_xDrawingArea->SetQueryTooltipHdl(Link<tools::Rectangle&, OUString>());
        m_xDrawingArea->SetPopupMenuHdl(Link<const CommandEvent&, bool>());
        m_xDrawingArea->SetStyleUpdatedHdl(Link<VclDrawingArea&, void>());
        m_xDrawingArea->SetMousePressHdl(Link<const MouseEvent&, bool>());
        m_xDrawingArea->SetMouseMoveHdl(Link<const MouseEvent&, bool>());
        m_xDrawingArea->SetMouseReleaseHdl(Link<const MouseEvent&, bool>());
        m_xDrawingArea->SetKeyPressHdl(Link<const KeyEvent&, bool>());
        m_xDrawingArea->SetKeyReleaseHdl(Link<const KeyEvent&, bool>());
        m_xDrawingArea->SetResizeHdl(Link<const Size&, void>());
        m_xDrawingArea->SetPaintHdl(Link<std::pair<vcl::RenderContext&, const tools::Rectangle&>, void>());
    }

    virtual OutputDevice& get_ref_device() override
    {
        return *m_xDrawingArea;
    }
};

IMPL_LINK(SalInstanceDrawingArea, PaintHdl, target_and_area, aPayload, void)
{
    m_aDrawHdl.Call(aPayload);
    tools::Rectangle aFocusRect(m_aGetFocusRectHdl.Call(*this));
    if (!aFocusRect.IsEmpty())
        DrawFocusRect(aPayload.first, aFocusRect);
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

IMPL_LINK(SalInstanceDrawingArea, PopupMenuHdl, const CommandEvent&, rEvent, bool)
{
    return m_aPopupMenuHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, QueryTooltipHdl, tools::Rectangle&, rHelpArea, OUString)
{
    return m_aQueryTooltipHdl.Call(rHelpArea);
}

//ComboBox and ListBox have similar apis, ComboBoxes in LibreOffice have an edit box and ListBoxes
//don't. This distinction isn't there in Gtk. Use a template to sort this problem out.
template <class vcl_type>
class SalInstanceComboBox : public SalInstanceContainer, public virtual weld::ComboBox
{
protected:
    // owner for ListBox/ComboBox UserData
    std::vector<std::unique_ptr<OUString>> m_aUserData;
    VclPtr<vcl_type> m_xComboBox;

public:
    SalInstanceComboBox(vcl_type* pComboBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pComboBox, pBuilder, bTakeOwnership)
        , m_xComboBox(pComboBox)
    {
    }

    virtual int get_active() const override
    {
        const sal_Int32 nRet = m_xComboBox->GetSelectedEntryPos();
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    const OUString* getEntryData(int index) const
    {
        return static_cast<const OUString*>(m_xComboBox->GetEntryData(index));
    }

    // ComboBoxes are comprised of multiple subwidgets, consider the lot as
    // one thing for focus
    virtual bool has_focus() const override
    {
        return m_xWidget->HasChildPathFocus();
    }

    virtual OUString get_active_id() const override
    {
        sal_Int32 nPos = m_xComboBox->GetSelectedEntryPos();
        const OUString* pRet;
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            pRet = getEntryData(m_xComboBox->GetSelectedEntryPos());
        else
            pRet = nullptr;
        if (!pRet)
            return OUString();
        return *pRet;
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        for (int i = 0; i < get_count(); ++i)
        {
            const OUString* pId = getEntryData(i);
            if (!pId)
                continue;
            if (*pId == rStr)
                m_xComboBox->SelectEntryPos(i);
        }
    }

    virtual void set_active(int pos) override
    {
        if (pos == -1)
        {
            m_xComboBox->SetNoSelection();
            return;
        }
        m_xComboBox->SelectEntryPos(pos);
    }

    virtual OUString get_text(int pos) const override
    {
        return m_xComboBox->GetEntry(pos);
    }

    virtual OUString get_id(int pos) const override
    {
        const OUString* pRet = getEntryData(pos);
        if (!pRet)
            return OUString();
        return *pRet;
    }

    virtual void set_id(int row, const OUString& rId) override
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(rId));
        m_xComboBox->SetEntryData(row, m_aUserData.back().get());
    }

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting) override
    {
        freeze();
        if (!bKeepExisting)
            clear();
        for (const auto& rItem : rItems)
        {
            insert(-1, rItem.sString, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
                   rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
        }
        thaw();
    }

    virtual int get_count() const override
    {
        return m_xComboBox->GetEntryCount();
    }

    virtual int find_text(const OUString& rStr) const override
    {
        const sal_Int32 nRet = m_xComboBox->GetEntryPos(rStr);
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    virtual int find_id(const OUString& rStr) const override
    {
        for (int i = 0; i < get_count(); ++i)
        {
            const OUString* pId = getEntryData(i);
            if (!pId)
                continue;
            if (*pId == rStr)
                return i;
        }
        return -1;
    }

    virtual void clear() override
    {
        m_xComboBox->Clear();
        m_aUserData.clear();
    }

    virtual void make_sorted() override
    {
        m_xComboBox->SetStyle(m_xComboBox->GetStyle() | WB_SORT);
    }

    virtual bool get_popup_shown() const override
    {
        return m_xComboBox->IsInDropDown();
    }

    virtual void connect_popup_toggled(const Link<ComboBox&, void>& rLink) override
    {
        weld::ComboBox::connect_popup_toggled(rLink);
        ensure_event_listener();
    }

    virtual void HandleEventListener(VclWindowEvent& rEvent) override
    {
        if (rEvent.GetId() == VclEventId::DropdownPreOpen ||
            rEvent.GetId() == VclEventId::DropdownClose)
        {
            signal_popup_toggled();
            return;
        }
        SalInstanceContainer::HandleEventListener(rEvent);
    }
};

class SalInstanceComboBoxWithoutEdit : public SalInstanceComboBox<ListBox>
{
private:
    DECL_LINK(SelectHdl, ListBox&, void);

public:
    SalInstanceComboBoxWithoutEdit(ListBox* pListBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceComboBox<ListBox>(pListBox, pBuilder, bTakeOwnership)
    {
        m_xComboBox->SetSelectHdl(LINK(this, SalInstanceComboBoxWithoutEdit, SelectHdl));
    }

    virtual OUString get_active_text() const override
    {
        return m_xComboBox->GetSelectedEntry();
    }

    virtual void remove(int pos) override
    {
        m_xComboBox->RemoveEntry(pos);
    }

    virtual void insert(int pos, const OUString& rStr, const OUString* pId, const OUString* pIconName, VirtualDevice* pImageSurface) override
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

    virtual void insert_separator(int pos) override
    {
        auto nInsertPos = pos == -1 ? m_xComboBox->GetEntryCount() : pos;
        m_xComboBox->AddSeparator(nInsertPos - 1);
    }

    virtual bool has_entry() const override
    {
        return false;
    }

    virtual void set_entry_message_type(weld::EntryMessageType /*eType*/) override
    {
        assert(false);
    }

    virtual void set_entry_text(const OUString& /*rText*/) override
    {
        assert(false);
    }

    virtual void select_entry_region(int /*nStartPos*/, int /*nEndPos*/) override
    {
        assert(false);
    }

    virtual bool get_entry_selection_bounds(int& /*rStartPos*/, int& /*rEndPos*/) override
    {
        assert(false);
        return false;
    }

    virtual void set_entry_width_chars(int /*nChars*/) override
    {
        assert(false);
    }

    virtual void set_entry_max_length(int /*nChars*/) override
    {
        assert(false);
    }

    virtual void set_entry_completion(bool, bool) override
    {
        assert(false);
    }

    virtual ~SalInstanceComboBoxWithoutEdit() override
    {
        m_xComboBox->SetSelectHdl(Link<ListBox&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceComboBoxWithoutEdit, SelectHdl, ListBox&, void)
{
    return signal_changed();
}

class SalInstanceComboBoxWithEdit : public SalInstanceComboBox<ComboBox>
{
private:
    DECL_LINK(ChangeHdl, Edit&, void);
    DECL_LINK(EntryActivateHdl, Edit&, bool);
    WeldTextFilter m_aTextFilter;
public:
    SalInstanceComboBoxWithEdit(::ComboBox* pComboBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceComboBox<::ComboBox>(pComboBox, pBuilder, bTakeOwnership)
        , m_aTextFilter(m_aEntryInsertTextHdl)
    {
        m_xComboBox->SetModifyHdl(LINK(this, SalInstanceComboBoxWithEdit, ChangeHdl));
        m_xComboBox->SetEntryActivateHdl(LINK(this, SalInstanceComboBoxWithEdit, EntryActivateHdl));
        m_xComboBox->SetTextFilter(&m_aTextFilter);
    }

    virtual bool has_entry() const override
    {
        return true;
    }

    virtual void set_entry_message_type(weld::EntryMessageType eType) override
    {
        if (eType == weld::EntryMessageType::Error)
            m_xComboBox->SetControlForeground(Color(0xf0, 0, 0));
        else if (eType == weld::EntryMessageType::Warning)
            m_xComboBox->SetControlForeground(COL_YELLOW);
        else
            m_xComboBox->SetControlForeground();
    }

    virtual OUString get_active_text() const override
    {
        return m_xComboBox->GetText();
    }

    virtual void remove(int pos) override
    {
        m_xComboBox->RemoveEntryAt(pos);
    }

    virtual void insert(int pos, const OUString& rStr, const OUString* pId, const OUString* pIconName, VirtualDevice* pImageSurface) override
    {
        auto nInsertPos = pos == -1 ? COMBOBOX_APPEND : pos;
        sal_Int32 nInsertedAt;
        if (!pIconName && !pImageSurface)
            nInsertedAt = m_xComboBox->InsertEntry(rStr, nInsertPos);
        else if (pIconName)
            nInsertedAt = m_xComboBox->InsertEntryWithImage(rStr, createImage(*pIconName), nInsertPos);
        else
            nInsertedAt = m_xComboBox->InsertEntryWithImage(rStr, createImage(*pImageSurface), nInsertPos);
        if (pId)
        {
            m_aUserData.emplace_back(std::make_unique<OUString>(*pId));
            m_xComboBox->SetEntryData(nInsertedAt, m_aUserData.back().get());
        }
    }

    virtual void insert_separator(int pos) override
    {
        auto nInsertPos = pos == -1 ? m_xComboBox->GetEntryCount() : pos;
        m_xComboBox->AddSeparator(nInsertPos - 1);
    }

    virtual void set_entry_text(const OUString& rText) override
    {
        m_xComboBox->SetText(rText);
    }

    virtual void set_entry_width_chars(int nChars) override
    {
        m_xComboBox->SetWidthInChars(nChars);
    }

    virtual void set_entry_max_length(int nChars) override
    {
        m_xComboBox->SetMaxTextLen(nChars);
    }

    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive) override
    {
        m_xComboBox->EnableAutocomplete(bEnable, bCaseSensitive);
    }

    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        m_xComboBox->SetSelection(Selection(nStartPos, nEndPos < 0 ? SELECTION_MAX : nEndPos));
    }

    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        const Selection& rSelection = m_xComboBox->GetSelection();
        rStartPos = rSelection.Min();
        rEndPos = rSelection.Max();
        return rSelection.Len();
    }

    virtual ~SalInstanceComboBoxWithEdit() override
    {
        m_xComboBox->SetTextFilter(nullptr);
        m_xComboBox->SetEntryActivateHdl(Link<Edit&, bool>());
        m_xComboBox->SetModifyHdl(Link<Edit&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceComboBoxWithEdit, ChangeHdl, Edit&, void)
{
    signal_changed();
}

IMPL_LINK_NOARG(SalInstanceComboBoxWithEdit, EntryActivateHdl, Edit&, bool)
{
    return m_aEntryActivateHdl.Call(*this);
}

class SalInstanceEntryTreeView : public SalInstanceContainer, public virtual weld::EntryTreeView
{
private:
    DECL_LINK(AutocompleteHdl, Edit&, void);
    DECL_LINK(KeyPressListener, VclWindowEvent&, void);
    SalInstanceEntry* m_pEntry;
    SalInstanceTreeView* m_pTreeView;
public:
    SalInstanceEntryTreeView(vcl::Window *pContainer, SalInstanceBuilder* pBuilder, bool bTakeOwnership,
                             std::unique_ptr<weld::Entry> xEntry, std::unique_ptr<weld::TreeView> xTreeView)
        : EntryTreeView(std::move(xEntry), std::move(xTreeView))
        , SalInstanceContainer(pContainer, pBuilder, bTakeOwnership)
        , m_pEntry(dynamic_cast<SalInstanceEntry*>(m_xEntry.get()))
        , m_pTreeView(dynamic_cast<SalInstanceTreeView*>(m_xTreeView.get()))
    {
        assert(m_pEntry && m_pTreeView);

        Edit& rEntry = m_pEntry->getEntry();
        rEntry.SetAutocompleteHdl(LINK(this, SalInstanceEntryTreeView, AutocompleteHdl));
        rEntry.AddEventListener(LINK(this, SalInstanceEntryTreeView, KeyPressListener));
    }

    virtual void insert_separator(int /*pos*/) override
    {
        assert(false);
    }

    virtual void make_sorted() override
    {
        vcl::Window *pTreeView = m_pTreeView->getWidget();
        pTreeView->SetStyle(pTreeView->GetStyle() | WB_SORT);
    }

    virtual void set_entry_completion(bool bEnable, bool /*bCaseSensitive*/) override
    {
        assert(!bEnable && "not implemented yet"); (void) bEnable;
        Edit& rEntry = m_pEntry->getEntry();
        rEntry.SetAutocompleteHdl(Link<Edit&, void>());
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
    if (nKeyCode == KEY_UP || nKeyCode == KEY_DOWN || nKeyCode == KEY_PAGEUP || nKeyCode == KEY_PAGEDOWN)
    {
        m_pTreeView->disable_notify_events();
        auto& rListBox = m_pTreeView->getTreeView();
        if (!rListBox.FirstSelected())
            rListBox.Select(rListBox.First(), true);
        else
            rListBox.KeyInput(rKeyEvent);
        m_xEntry->set_text(m_xTreeView->get_selected_text());
        m_xEntry->select_region(0, -1);
        m_pTreeView->enable_notify_events();
        m_pEntry->fire_signal_changed();
    }
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

class SalInstanceBuilder : public weld::Builder
{
private:
    std::unique_ptr<VclBuilder> m_xBuilder;
    VclPtr<vcl::Window> m_aOwnedToplevel;
public:
    SalInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile)
        : weld::Builder(rUIFile)
        , m_xBuilder(new VclBuilder(pParent, rUIRoot, rUIFile, OString(), css::uno::Reference<css::frame::XFrame>(), false))
    {
    }

    VclBuilder& get_builder() const
    {
        return *m_xBuilder;
    }

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString &id, bool bTakeOwnership) override
    {
        MessageDialog* pMessageDialog = m_xBuilder->get<MessageDialog>(id);
        std::unique_ptr<weld::MessageDialog> pRet(pMessageDialog ? new SalInstanceMessageDialog(pMessageDialog, this, false) : nullptr);
        if (bTakeOwnership && pMessageDialog)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pMessageDialog);
            m_xBuilder->drop_ownership(pMessageDialog);
        }
        return pRet;
    }

    virtual std::unique_ptr<weld::AboutDialog> weld_about_dialog(const OString &id, bool bTakeOwnership) override
    {
        vcl::AboutDialog* pAboutDialog = m_xBuilder->get<vcl::AboutDialog>(id);
        std::unique_ptr<weld::AboutDialog> pRet(pAboutDialog ? new SalInstanceAboutDialog(pAboutDialog, this, false) : nullptr);
        if (bTakeOwnership && pAboutDialog)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pAboutDialog);
            m_xBuilder->drop_ownership(pAboutDialog);
        }
        return pRet;
    }

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString &id, bool bTakeOwnership) override
    {
        Dialog* pDialog = m_xBuilder->get<Dialog>(id);
        std::unique_ptr<weld::Dialog> pRet(pDialog ? new SalInstanceDialog(pDialog, this, false) : nullptr);
        if (bTakeOwnership && pDialog)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pDialog);
            m_xBuilder->drop_ownership(pDialog);
        }
        return pRet;
    }

    virtual std::unique_ptr<weld::Window> weld_window(const OString &id, bool bTakeOwnership) override
    {
        SystemWindow* pWindow = m_xBuilder->get<SystemWindow>(id);
        return pWindow ? std::make_unique<SalInstanceWindow>(pWindow, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Widget> weld_widget(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pWidget = m_xBuilder->get<vcl::Window>(id);
        return pWidget ? std::make_unique<SalInstanceWidget>(pWidget, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Container> weld_container(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pContainer = m_xBuilder->get<vcl::Window>(id);
        return pContainer ? std::make_unique<SalInstanceContainer>(pContainer, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Frame> weld_frame(const OString &id, bool bTakeOwnership) override
    {
        VclFrame* pFrame = m_xBuilder->get<VclFrame>(id);
        std::unique_ptr<weld::Frame> pRet(pFrame ? new SalInstanceFrame(pFrame, this, false) : nullptr);
        if (bTakeOwnership && pFrame)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pFrame);
            m_xBuilder->drop_ownership(pFrame);
        }
        return pRet;
    }

    virtual std::unique_ptr<weld::ScrolledWindow> weld_scrolled_window(const OString &id, bool bTakeOwnership) override
    {
        VclScrolledWindow* pScrolledWindow = m_xBuilder->get<VclScrolledWindow>(id);
        return pScrolledWindow ? std::make_unique<SalInstanceScrolledWindow>(pScrolledWindow, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pNotebook = m_xBuilder->get<vcl::Window>(id);
        if (pNotebook->GetType() == WindowType::TABCONTROL)
            return std::make_unique<SalInstanceNotebook>(static_cast<TabControl*>(pNotebook), this, bTakeOwnership);
        if (pNotebook->GetType() == WindowType::VERTICALTABCONTROL)
            return std::make_unique<SalInstanceVerticalNotebook>(static_cast<VerticalTabControl*>(pNotebook), this, bTakeOwnership);
        return nullptr;
    }

    virtual std::unique_ptr<weld::Button> weld_button(const OString &id, bool bTakeOwnership) override
    {
        Button* pButton = m_xBuilder->get<Button>(id);
        return pButton ? std::make_unique<SalInstanceButton>(pButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString &id, bool bTakeOwnership) override
    {
        MenuButton* pButton = m_xBuilder->get<MenuButton>(id);
        return pButton ? std::make_unique<SalInstanceMenuButton>(pButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::LinkButton> weld_link_button(const OString &id, bool bTakeOwnership) override
    {
        FixedHyperlink* pButton = m_xBuilder->get<FixedHyperlink>(id);
        return pButton ? std::make_unique<SalInstanceLinkButton>(pButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OString &id, bool bTakeOwnership) override
    {
        PushButton* pToggleButton = m_xBuilder->get<PushButton>(id);
        return pToggleButton ? std::make_unique<SalInstanceToggleButton>(pToggleButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString &id, bool bTakeOwnership) override
    {
        RadioButton* pRadioButton = m_xBuilder->get<RadioButton>(id);
        return pRadioButton ? std::make_unique<SalInstanceRadioButton>(pRadioButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString &id, bool bTakeOwnership) override
    {
        CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
        return pCheckButton ? std::make_unique<SalInstanceCheckButton>(pCheckButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Scale> weld_scale(const OString &id, bool bTakeOwnership) override
    {
        Slider* pSlider = m_xBuilder->get<Slider>(id);
        return pSlider ? std::make_unique<SalInstanceScale>(pSlider, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OString &id, bool bTakeOwnership) override
    {
        ::ProgressBar* pProgress = m_xBuilder->get<::ProgressBar>(id);
        return pProgress ? std::make_unique<SalInstanceProgressBar>(pProgress, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Spinner> weld_spinner(const OString &id, bool bTakeOwnership) override
    {
        Throbber* pThrobber = m_xBuilder->get<Throbber>(id);
        return pThrobber ? std::make_unique<SalInstanceSpinner>(pThrobber, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Image> weld_image(const OString &id, bool bTakeOwnership) override
    {
        FixedImage* pImage = m_xBuilder->get<FixedImage>(id);
        return pImage ? std::make_unique<SalInstanceImage>(pImage, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Calendar> weld_calendar(const OString &id, bool bTakeOwnership) override
    {
        Calendar* pCalendar = m_xBuilder->get<Calendar>(id);
        return pCalendar ? std::make_unique<SalInstanceCalendar>(pCalendar, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Entry> weld_entry(const OString &id, bool bTakeOwnership) override
    {
        Edit* pEntry = m_xBuilder->get<Edit>(id);
        return pEntry ? std::make_unique<SalInstanceEntry>(pEntry, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString &id, bool bTakeOwnership) override
    {
        FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
        return pSpinButton ? std::make_unique<SalInstanceSpinButton>(pSpinButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::MetricSpinButton> weld_metric_spin_button(const OString& id, FieldUnit eUnit,
                                                                            bool bTakeOwnership) override
    {
        std::unique_ptr<weld::SpinButton> xButton(weld_spin_button(id, bTakeOwnership));
        if (xButton)
        {
            SalInstanceSpinButton& rButton = dynamic_cast<SalInstanceSpinButton&>(*xButton);
            rButton.SetUseThousandSep();
        }
        return std::make_unique<weld::MetricSpinButton>(std::move(xButton), eUnit);
    }

    virtual std::unique_ptr<weld::FormattedSpinButton> weld_formatted_spin_button(const OString& id,
                                                                                  bool bTakeOwnership) override
    {
        FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
        return pSpinButton ? std::make_unique<SalInstanceFormattedSpinButton>(pSpinButton, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::TimeSpinButton> weld_time_spin_button(const OString& id, TimeFieldFormat eFormat,
                                                        bool bTakeOwnership) override
    {
        std::unique_ptr<weld::TimeSpinButton> pRet(new weld::TimeSpinButton(weld_spin_button(id, bTakeOwnership), eFormat));
        SalInstanceSpinButton& rButton = dynamic_cast<SalInstanceSpinButton&>(pRet->get_widget());
        rButton.DisableRemainderFactor(); //so with hh::mm::ss, incrementing mm will not reset ss
        return pRet;
    }

    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pWidget = m_xBuilder->get<vcl::Window>(id);
        ::ComboBox* pComboBox = dynamic_cast<::ComboBox*>(pWidget);
        if (pComboBox)
            return std::make_unique<SalInstanceComboBoxWithEdit>(pComboBox, this, bTakeOwnership);
        ListBox* pListBox = dynamic_cast<ListBox*>(pWidget);
        return pListBox ? std::make_unique<SalInstanceComboBoxWithoutEdit>(pListBox, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::EntryTreeView> weld_entry_tree_view(const OString& containerid, const OString& entryid, const OString& treeviewid, bool bTakeOwnership) override
    {
        vcl::Window* pContainer = m_xBuilder->get<vcl::Window>(containerid);
        return pContainer ? std::make_unique<SalInstanceEntryTreeView>(pContainer, this, bTakeOwnership,
                                                                       weld_entry(entryid, bTakeOwnership),
                                                                       weld_tree_view(treeviewid, bTakeOwnership)) : nullptr;
    }

    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString &id, bool bTakeOwnership) override
    {
        SvTabListBox* pTreeView = m_xBuilder->get<SvTabListBox>(id);
        return pTreeView ? std::make_unique<SalInstanceTreeView>(pTreeView, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Label> weld_label(const OString &id, bool bTakeOwnership) override
    {
        Control* pLabel = m_xBuilder->get<Control>(id);
        return pLabel ? std::make_unique<SalInstanceLabel>(pLabel, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString &id, bool bTakeOwnership) override
    {
        VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
        return pTextView ? std::make_unique<SalInstanceTextView>(pTextView, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Expander> weld_expander(const OString &id, bool bTakeOwnership) override
    {
        VclExpander* pExpander = m_xBuilder->get<VclExpander>(id);
        return pExpander ? std::make_unique<SalInstanceExpander>(pExpander, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::DrawingArea> weld_drawing_area(const OString &id, const a11yref& rA11yImpl,
            FactoryFunction pUITestFactoryFunction, void* pUserData, bool bTakeOwnership) override
    {
        VclDrawingArea* pDrawingArea = m_xBuilder->get<VclDrawingArea>(id);
        return pDrawingArea ? std::make_unique<SalInstanceDrawingArea>(pDrawingArea, this, rA11yImpl,
                pUITestFactoryFunction, pUserData, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Menu> weld_menu(const OString &id, bool bTakeOwnership) override
    {
        PopupMenu* pMenu = m_xBuilder->get_menu(id);
        return pMenu ? std::make_unique<SalInstanceMenu>(pMenu, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OString &id, bool bTakeOwnership) override
    {
        ToolBox* pToolBox = m_xBuilder->get<ToolBox>(id);
        return pToolBox ? std::make_unique<SalInstanceToolbar>(pToolBox, this, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override
    {
        return std::make_unique<SalInstanceSizeGroup>();
    }

    virtual ~SalInstanceBuilder() override
    {
        if (VclBuilderContainer* pOwnedToplevel = dynamic_cast<VclBuilderContainer*>(m_aOwnedToplevel.get()))
            pOwnedToplevel->m_pUIBuilder = std::move(m_xBuilder);
        else
            m_xBuilder.reset();
        m_aOwnedToplevel.disposeAndClear();
    }
};

weld::Builder* SalInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    vcl::Window* pParentWidget = pParentInstance ? pParentInstance->getWidget() : nullptr;
    return new SalInstanceBuilder(pParentWidget, rUIRoot, rUIFile);
}

weld::Builder* SalInstance::CreateInterimBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    return new SalInstanceBuilder(pParent, rUIRoot, rUIFile);
}

weld::Builder* Application::CreateInterimBuilder(weld::Widget* pParent, const OUString &rUIFile)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    vcl::Window* pParentWidget = pParentInstance ? pParentInstance->getWidget() : nullptr;
    return Application::CreateInterimBuilder(pParentWidget, rUIFile);
}

//iterate upwards through the hierarchy from this widgets through its parents
//calling func with their helpid until func returns true or we run out of parents
void SalInstanceWidget::help_hierarchy_foreach(const std::function<bool(const OString&)>& func)
{
    vcl::Window* pParent = m_xWidget;
    while ((pParent = pParent->GetParent()))
    {
        if (m_pBuilder && pParent->IsDialog())
        {
            // tdf#122355 During help fallback, before we ask a dialog for its help
            // see if it has a TabControl and ask the active tab of that for help
            TabControl *pCtrl = m_pBuilder->get_builder().get<TabControl>("tabcontrol");
            TabPage* pTabPage = pCtrl ? pCtrl->GetTabPage(pCtrl->GetCurPageId()) : nullptr;
            vcl::Window *pTabChild = pTabPage ? pTabPage->GetWindow(GetWindowType::FirstChild) : nullptr;
            pTabChild = pTabChild ? pTabChild->GetWindow(GetWindowType::FirstChild) : nullptr;
            if (pTabChild)
            {
                if (func(pTabChild->GetHelpId()))
                    return;
            }
        }
        if (func(pParent->GetHelpId()))
            return;
    }
}

weld::MessageDialog* SalInstance::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType, VclButtonsType eButtonsType, const OUString& rPrimaryMessage)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    SystemWindow* pParentWidget = pParentInstance ? pParentInstance->getSystemWindow() : nullptr;
    VclPtrInstance<MessageDialog> xMessageDialog(pParentWidget, rPrimaryMessage, eMessageType, eButtonsType);
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
        pWindow = pWindow ? pWindow->ImplGetWindow() : nullptr;
        assert(!pWindow || (pWindow->IsSystemWindow() || pWindow->IsDockingWindow()));
        if (pWindow)
            m_xFrameWeld.reset(new SalInstanceWindow(pWindow, nullptr, false));
    }
    return m_xFrameWeld.get();
}

namespace weld
{
    bool DialogController::runAsync(const std::shared_ptr<DialogController>& rController, const std::function<void(sal_Int32)>& func)
    {
        return rController->getDialog()->runAsync(rController, func);
    }

    DialogController::~DialogController() COVERITY_NOEXCEPT_FALSE
    {
    }

    Dialog* GenericDialogController::getDialog() { return m_xDialog.get(); }

    GenericDialogController::GenericDialogController(weld::Widget* pParent, const OUString &rUIFile, const OString& rDialogId)
        : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
        , m_xDialog(m_xBuilder->weld_dialog(rDialogId))
    {
    }

    GenericDialogController::~GenericDialogController() COVERITY_NOEXCEPT_FALSE
    {
    }

    Dialog* MessageDialogController::getDialog() { return m_xDialog.get(); }

    MessageDialogController::MessageDialogController(weld::Widget* pParent, const OUString &rUIFile, const OString& rDialogId,
            const OString& rRelocateId)
        : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
        , m_xDialog(m_xBuilder->weld_message_dialog(rDialogId))
        , m_xContentArea(m_xDialog->weld_message_area())
    {
        if (!rRelocateId.isEmpty())
        {
            m_xRelocate = m_xBuilder->weld_container(rRelocateId);
            m_xOrigParent = m_xRelocate->weld_parent();
            //fdo#75121, a bit tricky because the widgets we want to align with
            //don't actually exist in the ui description, they're implied
            m_xOrigParent->move(m_xRelocate.get(), m_xContentArea.get());
        }
    }

    MessageDialogController::~MessageDialogController()
    {
        if (m_xRelocate)
        {
            m_xContentArea->move(m_xRelocate.get(), m_xOrigParent.get());
        }
    }

    void TriStateEnabled::ButtonToggled(weld::ToggleButton& rToggle)
    {
        if (bTriStateEnabled)
        {
            switch (eState)
            {
                case TRISTATE_INDET:
                    rToggle.set_state(TRISTATE_FALSE);
                    break;
                case TRISTATE_TRUE:
                    rToggle.set_state(TRISTATE_INDET);
                    break;
                case TRISTATE_FALSE:
                    rToggle.set_state(TRISTATE_TRUE);
                    break;
            }
        }
        eState = rToggle.get_state();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
