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
#include <unotools/accessiblerelationsethelper.hxx>
#include <utility>
#include <tools/helpers.hxx>
#include <vcl/builder.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fmtfield.hxx>
#include <vcl/headbar.hxx>
#include <vcl/layout.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/slider.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/weld.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/virdev.hxx>
#include <bitmaps.hlst>
#include <o3tl/make_unique.hxx>

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

SalBitmap::~SalBitmap()
{
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

SalMenuItem::~SalMenuItem()
{
}

class SalInstanceWidget : public virtual weld::Widget
{
private:
    VclPtr<vcl::Window> m_xWidget;

    DECL_LINK(EventListener, VclWindowEvent&, void);

    const bool m_bTakeOwnership;
    bool m_bEventListener;
    int m_nBlockNotify;

    void ensure_event_listener()
    {
        if (!m_bEventListener)
        {
            m_xWidget->AddEventListener(LINK(this, SalInstanceWidget, EventListener));
            m_bEventListener = true;
        }
    }

public:
    SalInstanceWidget(vcl::Window* pWidget, bool bTakeOwnership)
        : m_xWidget(pWidget)
        , m_bTakeOwnership(bTakeOwnership)
        , m_bEventListener(false)
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

    virtual void set_visible(bool visible) override
    {
        m_xWidget->Show(visible);
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

    virtual void set_tooltip_text(const OUString& rTip) override
    {
        m_xWidget->SetQuickHelpText(rTip);
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override
    {
        ensure_event_listener();
        weld::Widget::connect_focus_in(rLink);
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

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override
    {
        ensure_event_listener();
        weld::Widget::connect_key_press(rLink);
    }

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override
    {
        ensure_event_listener();
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

    virtual weld::Container* weld_parent() const override;

    virtual ~SalInstanceWidget() override
    {
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

    SystemWindow* getSystemWindow()
    {
        return m_xWidget->GetSystemWindow();
    }
};

IMPL_LINK(SalInstanceWidget, EventListener, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() == VclEventId::WindowGetFocus || rEvent.GetId() == VclEventId::WindowActivate)
        m_aFocusInHdl.Call(*this);
    else if (rEvent.GetId() == VclEventId::WindowLoseFocus || rEvent.GetId() == VclEventId::WindowDeactivate)
        m_aFocusOutHdl.Call(*this);
    else if (rEvent.GetId() == VclEventId::WindowResize)
        m_aSizeAllocateHdl.Call(m_xWidget->GetSizePixel());
    else if (rEvent.GetId() == VclEventId::WindowKeyInput)
    {
        const KeyEvent* pKeyEvent = static_cast<const KeyEvent*>(rEvent.GetData());
        m_aKeyPressHdl.Call(*pKeyEvent);
    }
    else if (rEvent.GetId() == VclEventId::WindowKeyUp)
    {
        const KeyEvent* pKeyEvent = static_cast<const KeyEvent*>(rEvent.GetData());
        m_aKeyReleaseHdl.Call(*pKeyEvent);
    }
}

namespace
{
    Image createImage(const OUString& rImage)
    {
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

    Image createImage(VirtualDevice& rDevice)
    {
        return Image(rDevice.GetBitmapEx(Point(), rDevice.GetOutputSizePixel()));
    }

    void insert_to_menu(PopupMenu* pMenu, int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface, bool bCheck)
    {
        const auto nCount = pMenu->GetItemCount();
        const sal_uInt16 nLastId = nCount ? pMenu->GetItemId(nCount-1) : 0;
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
    }
}

class SalInstanceMenu : public weld::Menu
{
private:
    VclPtr<PopupMenu> m_xMenu;

    bool const m_bTakeOwnership;

public:
    SalInstanceMenu(PopupMenu* pMenu, bool bTakeOwnership)
        : m_xMenu(pMenu)
        , m_bTakeOwnership(bTakeOwnership)
    {
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
    virtual void show(const OString& rIdent, bool bShow) override
    {
        m_xMenu->ShowItem(m_xMenu->GetItemId(rIdent), bShow);
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface, bool bCheck) override
    {
        insert_to_menu(m_xMenu, pos, rId, rStr, pIconName, pImageSurface, bCheck);
    }

    virtual ~SalInstanceMenu() override
    {
        if (m_bTakeOwnership)
            m_xMenu.disposeAndClear();
    }
};

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
    SalInstanceContainer(vcl::Window* pContainer, bool bTakeOwnership)
        : SalInstanceWidget(pContainer, bTakeOwnership)
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

weld::Container* SalInstanceWidget::weld_parent() const
{
    vcl::Window* pParent = m_xWidget->GetParent();
    return pParent ? new SalInstanceContainer(pParent, false) : nullptr;
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
    SalInstanceWindow(vcl::Window* pWindow, bool bTakeOwnership)
        : SalInstanceContainer(pWindow, bTakeOwnership)
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
        std::unique_ptr<weld::Widget> xTemp(pWidget != m_xWindow ? new SalInstanceWidget(pWidget, false) : nullptr);
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
        if (::Dialog* pDialog = dynamic_cast<::Dialog*>(m_xWindow.get()))
            return pDialog->SetModalInputMode(bModal);
        return m_xWindow->ImplGetFrame()->SetModal(bModal);
    }

    virtual bool get_modal() const override
    {
        if (const ::Dialog* pDialog = dynamic_cast<const ::Dialog*>(m_xWindow.get()))
            return pDialog->IsModalInputMode();
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

    virtual bool get_resizable() const override
    {
        return m_xWindow->GetStyle() & WB_SIZEABLE;
    }

    virtual bool has_toplevel_focus() const override
    {
        return m_xWindow->HasChildPathFocus();
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

class SalInstanceDialog : public SalInstanceWindow, public virtual weld::Dialog
{
private:
    VclPtr<::Dialog> m_xDialog;

public:
    SalInstanceDialog(::Dialog* pDialog, bool bTakeOwnership)
        : SalInstanceWindow(pDialog, bTakeOwnership)
        , m_xDialog(pDialog)
    {
    }

    virtual bool runAsync(std::shared_ptr<weld::DialogController> aOwner, const std::function<void(sal_Int32)> &rEndDialogFn) override
    {
        VclAbstractDialog::AsyncContext aCtx;
        aCtx.mxOwnerDialog = aOwner;
        aCtx.maEndDialogFn = rEndDialogFn;
        return m_xDialog->StartExecuteAsync(aCtx);
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
        xButton->Show();
        m_xDialog->add_button(xButton, nResponse, true);
    }

    virtual weld::Button* get_widget_for_response(int nResponse) override;

    virtual void set_default_response(int nResponse) override
    {
        m_xDialog->set_default_response(nResponse);
    }

    virtual Container* weld_content_area() override
    {
        return new SalInstanceContainer(m_xDialog->get_content_area(), false);
    }
};

class SalInstanceMessageDialog : public SalInstanceDialog, public virtual weld::MessageDialog
{
private:
    VclPtr<::MessageDialog> m_xMessageDialog;
public:
    SalInstanceMessageDialog(::MessageDialog* pDialog, bool bTakeOwnership)
        : SalInstanceDialog(pDialog, bTakeOwnership)
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
        return new SalInstanceContainer(m_xMessageDialog->get_message_area(), false);
    }
};

class SalInstanceFrame : public SalInstanceContainer, public virtual weld::Frame
{
private:
    VclPtr<VclFrame> m_xFrame;
public:
    SalInstanceFrame(VclFrame* pFrame, bool bTakeOwnership)
        : SalInstanceContainer(pFrame, bTakeOwnership)
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
    SalInstanceScrolledWindow(VclScrolledWindow* pScrolledWindow, bool bTakeOwnership)
        : SalInstanceContainer(pScrolledWindow, bTakeOwnership)
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

    virtual int vadjustment_get_page_size() const override
    {
        ScrollBar& rVertScrollBar = m_xScrolledWindow->getVertScrollBar();
        return rVertScrollBar.GetVisibleSize();
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

IMPL_LINK_NOARG(SalInstanceScrolledWindow, VscrollHdl, ScrollBar*, void)
{
    signal_vadjustment_changed();
    if (!m_bUserManagedScrolling)
        m_aOrigVScrollHdl.Call(&m_xScrolledWindow->getVertScrollBar());
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
    SalInstanceNotebook(TabControl* pNotebook, bool bTakeOwnership)
        : SalInstanceContainer(pNotebook, bTakeOwnership)
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
            m_aPages[nPageIndex].reset(new SalInstanceContainer(pChild, false));
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

class SalInstanceButton : public SalInstanceContainer, public virtual weld::Button
{
private:
    VclPtr<::Button> m_xButton;
    Link<::Button*,void> const m_aOldClickHdl;

    DECL_LINK(ClickHdl, ::Button*, void);
public:
    SalInstanceButton(::Button* pButton, bool bTakeOwnership)
        : SalInstanceContainer(pButton, bTakeOwnership)
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
        {
            BitmapEx aBitmap(pDevice->GetBitmap(Point(0, 0), pDevice->GetOutputSize()));
            m_xButton->SetModeImage(Image(aBitmap));
        }
        else
            m_xButton->SetModeImage(Image());
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        m_xButton->SetModeImage(::Image(StockImage::Yes, rIconName));
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
    return pButton ? new SalInstanceButton(pButton, false) : nullptr;
}

class SalInstanceMenuButton : public SalInstanceButton, public virtual weld::MenuButton
{
private:
    VclPtr<::MenuButton> m_xMenuButton;

    DECL_LINK(MenuSelectHdl, ::MenuButton*, void);
    DECL_LINK(ActivateHdl, ::MenuButton*, void);

public:
    SalInstanceMenuButton(::MenuButton* pButton, bool bTakeOwnership)
        : SalInstanceButton(pButton, bTakeOwnership)
        , m_xMenuButton(pButton)
    {
        m_xMenuButton->SetActivateHdl(LINK(this, SalInstanceMenuButton, ActivateHdl));
        m_xMenuButton->SetSelectHdl(LINK(this, SalInstanceMenuButton, MenuSelectHdl));
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
        insert_to_menu(m_xMenuButton->GetPopupMenu(), pos, rId, rStr, pIconName, pImageSurface, bCheck);
    }

    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override
    {
        PopupMenu* pMenu = m_xMenuButton->GetPopupMenu();
        pMenu->EnableItem(rIdent, bSensitive);
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

class SalInstanceRadioButton : public SalInstanceButton, public virtual weld::RadioButton
{
private:
    VclPtr<::RadioButton> m_xRadioButton;

    DECL_LINK(ToggleHdl, ::RadioButton&, void);

public:
    SalInstanceRadioButton(::RadioButton* pButton, bool bTakeOwnership)
        : SalInstanceButton(pButton, bTakeOwnership)
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

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        m_xRadioButton->SetModeRadioImage(::Image(StockImage::Yes, rIconName));
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
    SalInstanceToggleButton(PushButton* pButton, bool bTakeOwnership)
        : SalInstanceButton(pButton, bTakeOwnership)
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
    SalInstanceCheckButton(CheckBox* pButton, bool bTakeOwnership)
        : SalInstanceButton(pButton, bTakeOwnership)
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
    SalInstanceScale(Slider* pScale, bool bTakeOwnership)
        : SalInstanceWidget(pScale, bTakeOwnership)
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

class SalInstanceProgressBar : public SalInstanceWidget, public virtual weld::ProgressBar
{
private:
    VclPtr<::ProgressBar> m_xProgressBar;

public:
    SalInstanceProgressBar(::ProgressBar* pProgressBar, bool bTakeOwnership)
        : SalInstanceWidget(pProgressBar, bTakeOwnership)
        , m_xProgressBar(pProgressBar)
    {
    }

    virtual void set_percentage(int value) override
    {
        m_xProgressBar->SetValue(value);
    }
};

class SalInstanceImage : public SalInstanceWidget, public virtual weld::Image
{
private:
    VclPtr<FixedImage> m_xImage;

public:
    SalInstanceImage(FixedImage* pImage, bool bTakeOwnership)
        : SalInstanceWidget(pImage, bTakeOwnership)
        , m_xImage(pImage)
    {
    }

    virtual void set_from_icon_name(const OUString& rIconName) override
    {
        m_xImage->SetImage(::Image(StockImage::Yes, rIconName));
    }
};

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
    SalInstanceEntry(Edit* pEntry, bool bTakeOwnership)
        : SalInstanceWidget(pEntry, bTakeOwnership)
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

    virtual void set_error(bool bError) override
    {
        if (bError)
        {
            // #i75179# enable setting the background to a different color
            m_xEntry->SetForceControlBackground(true);
            m_xEntry->SetControlForeground(COL_WHITE);
            m_xEntry->SetControlBackground(0xff6563);
        }
        else
        {
            m_xEntry->SetForceControlBackground(false);
            m_xEntry->SetControlForeground();
            m_xEntry->SetControlBackground();
        }
    }

    virtual vcl::Font get_font() override
    {
        return m_xEntry->GetFont();
    }

    virtual void set_font(const vcl::Font& rFont) override
    {
        m_xEntry->SetFont(rFont);
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
    SvTreeListEntry* iter;
};

class SalInstanceTreeView : public SalInstanceContainer, public virtual weld::TreeView
{
private:
    // owner for UserData
    std::vector<std::unique_ptr<OUString>> m_aUserData;
    VclPtr<SvTabListBox> m_xTreeView;
    SvLBoxButtonData m_aCheckButtonData;
    SvLBoxButtonData m_aRadioButtonData;

    DECL_LINK(SelectHdl, SvTreeListBox*, void);
    DECL_LINK(DoubleClickHdl, SvTreeListBox*, bool);
    DECL_LINK(ExpandingHdl, SvTreeListBox*, bool);
    DECL_LINK(EndDragHdl, HeaderBar*, void);
    DECL_LINK(ToggleHdl, SvLBoxButtonData*, void);
public:
    SalInstanceTreeView(SvTabListBox* pTreeView, bool bTakeOwnership)
        : SalInstanceContainer(pTreeView, bTakeOwnership)
        , m_xTreeView(pTreeView)
        , m_aCheckButtonData(pTreeView, false)
        , m_aRadioButtonData(pTreeView, true)
    {
        m_xTreeView->SetNodeDefaultImages();
        m_xTreeView->SetSelectHdl(LINK(this, SalInstanceTreeView, SelectHdl));
        m_xTreeView->SetDoubleClickHdl(LINK(this, SalInstanceTreeView, DoubleClickHdl));
        m_xTreeView->SetExpandingHdl(LINK(this, SalInstanceTreeView, ExpandingHdl));
        const long aTabPositions[] = { 0 };
        m_xTreeView->SetTabs(SAL_N_ELEMENTS(aTabPositions), aTabPositions);
        SvHeaderTabListBox* pHeaderBox = dynamic_cast<SvHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            //make the last entry fill available space
            pHeaderBar->SetItemSize(pHeaderBar->GetItemId(pHeaderBar->GetItemCount() - 1 ), HEADERBAR_FULLSIZE);
            pHeaderBar->SetEndDragHdl(LINK(this, SalInstanceTreeView, EndDragHdl));
        }
        m_aCheckButtonData.SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
        m_aRadioButtonData.SetLink(LINK(this, SalInstanceTreeView, ToggleHdl));
    }

    virtual void set_column_fixed_widths(const std::vector<int>& rWidths) override
    {
        std::vector<long> aTabPositions;
        aTabPositions.push_back(0);
        for (size_t i = 0; i < rWidths.size(); ++i)
            aTabPositions.push_back(aTabPositions[i] + rWidths[i]);
        m_xTreeView->SetTabs(aTabPositions.size(), aTabPositions.data(), MapUnit::MapPixel);
        SvHeaderTabListBox* pHeaderBox = dynamic_cast<SvHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            for (size_t i = 0; i < rWidths.size(); ++i)
                pHeaderBar->SetItemSize(pHeaderBar->GetItemId(i), rWidths[i]);
        }
        // call Resize to recalculate based on the new tabs
        m_xTreeView->Resize();
    }

    virtual OUString get_column_title(int nColumn) const override
    {
        SvHeaderTabListBox* pHeaderBox = dynamic_cast<SvHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            return pHeaderBar->GetItemText(pHeaderBar->GetItemId(nColumn));
        }
        return OUString();
    }

    virtual void set_column_title(int nColumn, const OUString& rTitle) override
    {
        SvHeaderTabListBox* pHeaderBox = dynamic_cast<SvHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            return pHeaderBar->SetItemText(pHeaderBar->GetItemId(nColumn), rTitle);
        }
    }

    virtual void show() override
    {
        SvHeaderTabListBox* pHeaderBox = dynamic_cast<SvHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            pHeaderBar->Show();
        }
        SalInstanceContainer::show();
    }

    virtual void hide() override
    {
        SvHeaderTabListBox* pHeaderBox = dynamic_cast<SvHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            pHeaderBar->Hide();
        }
        SalInstanceContainer::hide();
    }

    virtual void insert(weld::TreeIter* pParent, int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface, const OUString* pExpanderName,
                        bool bChildrenOnDemand) override
    {
        SalInstanceTreeIter* pVclIter = static_cast<SalInstanceTreeIter*>(pParent);
        SvTreeListEntry* iter = pVclIter ? pVclIter->iter : nullptr;
        auto nInsertPos = pos == -1 ? TREELIST_APPEND : pos;
        void* pUserData;
        if (pId)
        {
            m_aUserData.emplace_back(o3tl::make_unique<OUString>(*pId));
            pUserData = m_aUserData.back().get();
        }
        else
            pUserData = nullptr;

        SvTreeListEntry* pEntry = new SvTreeListEntry;
        if (pIconName || pImageSurface)
        {
            Image aImage(pIconName ? createImage(*pIconName) : createImage(*pImageSurface));
            pEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(aImage, aImage, false));
        }
        else
        {
            Image aDummy;
            pEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(aDummy, aDummy, false));
        }
        if (pStr)
            pEntry->AddItem(o3tl::make_unique<SvLBoxString>(*pStr));
        pEntry->SetUserData(pUserData);
        m_xTreeView->Insert(pEntry, iter, nInsertPos);

        if (pExpanderName)
        {
            Image aImage(createImage(*pExpanderName));
            m_xTreeView->SetExpandedEntryBmp(pEntry, aImage);
            m_xTreeView->SetCollapsedEntryBmp(pEntry, aImage);
        }

        if (bChildrenOnDemand)
        {
            m_xTreeView->InsertEntry("<dummy>", pEntry, false, 0, nullptr);
        }
    }

    virtual void set_font_color(int pos, const Color& rColor) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        pEntry->SetTextColor(&rColor);
    }

    virtual void remove(int pos) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        m_xTreeView->RemoveEntry(pEntry);
    }

    virtual int find_text(const OUString& rText) const override
    {
        for (SvTreeListEntry* pEntry = m_xTreeView->First(); pEntry; pEntry = m_xTreeView->Next(pEntry))
        {
            if (SvTabListBox::GetEntryText(pEntry, 0) == rText)
                return m_xTreeView->GetAbsPos(pEntry);
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
                return m_xTreeView->GetAbsPos(pEntry);
        }
        return -1;
    }

    virtual void set_top_entry(int pos) override
    {
        SvTreeList* pModel = m_xTreeView->GetModel();
        SvTreeListEntry* pEntry = pModel->GetEntry(nullptr, pos);
        pModel->Move(pEntry, nullptr, 0);
    }

    virtual void clear() override
    {
        m_xTreeView->Clear();
        m_aUserData.clear();
    }

    virtual int n_children() const override
    {
        return m_xTreeView->GetEntryCount();
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
        }
        enable_notify_events();
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
            aRows.push_back(m_xTreeView->GetAbsPos(pEntry));

        return aRows;
    }

    virtual OUString get_text(int pos, int col) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
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

    virtual void set_text(int pos, const OUString& rText, int col) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        if (col == -1)
        {
            m_xTreeView->SetEntryText(pEntry, rText);
            return;
        }

        ++col; //skip dummy/expander column

        // blank out missing entries
        for (int i = pEntry->ItemCount(); i < col ; ++i)
            pEntry->AddItem(o3tl::make_unique<SvLBoxString>(""));

        if (static_cast<size_t>(col) == pEntry->ItemCount())
        {
            pEntry->AddItem(o3tl::make_unique<SvLBoxString>(rText));
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

    virtual bool get_toggle(int pos, int col) const override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        if (col == -1)
            return m_xTreeView->GetCheckButtonState(pEntry) == SvButtonState::Checked;

        ++col; //skip dummy/expander column

        if (static_cast<size_t>(col) == pEntry->ItemCount())
            return false;

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxButton*>(&rItem));
        return static_cast<SvLBoxButton&>(rItem).IsStateChecked();
    }

    virtual void set_toggle(int pos, bool bOn, int col) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        if (col == -1)
        {
            m_xTreeView->SetCheckButtonState(pEntry, bOn ? SvButtonState::Checked : SvButtonState::Unchecked);
            return;
        }

        bool bRadio = std::find(m_aRadioIndexes.begin(), m_aRadioIndexes.end(), col) != m_aRadioIndexes.end();
        ++col; //skip dummy/expander column

        // blank out missing entries
        for (int i = pEntry->ItemCount(); i < col ; ++i)
            pEntry->AddItem(o3tl::make_unique<SvLBoxString>(""));

        if (static_cast<size_t>(col) == pEntry->ItemCount())
        {
            pEntry->AddItem(o3tl::make_unique<SvLBoxButton>(SvLBoxButtonKind::EnabledCheckbox,
                                                            bRadio ? &m_aRadioButtonData : &m_aCheckButtonData));
            SvViewDataEntry* pViewData = m_xTreeView->GetViewDataEntry(pEntry);
            m_xTreeView->InitViewData(pViewData, pEntry);
        }

        assert(col >= 0 && static_cast<size_t>(col) < pEntry->ItemCount());
        SvLBoxItem& rItem = pEntry->GetItem(col);
        assert(dynamic_cast<SvLBoxButton*>(&rItem));
        if (bOn)
            static_cast<SvLBoxButton&>(rItem).SetStateChecked();
        else
            static_cast<SvLBoxButton&>(rItem).SetStateUnchecked();

        m_xTreeView->ModelHasEntryInvalidated(pEntry);
    }

    const OUString* getEntryData(int index) const
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, index);
        return static_cast<const OUString*>(pEntry->GetUserData());
    }

    virtual OUString get_id(int pos) const override
    {
        const OUString* pRet = getEntryData(pos);
        if (!pRet)
            return OUString();
        return *pRet;
    }

    virtual void set_id(int pos, const OUString& rId) override
    {
        SvTreeListEntry* pEntry = m_xTreeView->GetEntry(nullptr, pos);
        m_aUserData.emplace_back(o3tl::make_unique<OUString>(rId));
        pEntry->SetUserData(m_aUserData.back().get());
    }

    virtual int get_selected_index() const override
    {
        assert(m_xTreeView->IsUpdateMode() && "don't request selection when frozen");
        SvTreeListEntry* pEntry = m_xTreeView->FirstSelected();
        if (!pEntry)
            return -1;
        return m_xTreeView->GetAbsPos(pEntry);
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
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        m_xTreeView->RemoveEntry(rVclIter.iter);
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

    virtual void expand_row(weld::TreeIter& rIter) override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
        if (!m_xTreeView->IsExpanded(rVclIter.iter) && signal_expanding(rIter))
            m_xTreeView->Expand(rVclIter.iter);
    }

    virtual void collapse_row(weld::TreeIter& rIter) override
    {
        SalInstanceTreeIter& rVclIter = static_cast<SalInstanceTreeIter&>(rIter);
        if (m_xTreeView->IsExpanded(rVclIter.iter))
            m_xTreeView->Collapse(rVclIter.iter);
    }

    virtual OUString get_text(const weld::TreeIter& rIter) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        return SvTabListBox::GetEntryText(rVclIter.iter, 0);
    }

    virtual OUString get_id(const weld::TreeIter& rIter) const override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        const OUString* pStr = static_cast<const OUString*>(rVclIter.iter->GetUserData());
        if (pStr)
            return *pStr;
        return OUString();
    }

    virtual void set_expander_image(const weld::TreeIter& rIter, const OUString& rImage) override
    {
        const SalInstanceTreeIter& rVclIter = static_cast<const SalInstanceTreeIter&>(rIter);
        Image aImage(createImage(rImage));
        m_xTreeView->SetExpandedEntryBmp(rVclIter.iter, aImage);
        m_xTreeView->SetCollapsedEntryBmp(rVclIter.iter, aImage);
    }

    virtual void set_selection_mode(SelectionMode eMode) override
    {
        m_xTreeView->SetSelectionMode(eMode);
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
        m_xTreeView->GetModel()->Resort();
    }

    SvTabListBox& getTreeView()
    {
        return *m_xTreeView;
    }

    virtual ~SalInstanceTreeView() override
    {
        SvHeaderTabListBox* pHeaderBox = dynamic_cast<SvHeaderTabListBox*>(m_xTreeView.get());
        if (HeaderBar* pHeaderBar = pHeaderBox ? pHeaderBox->GetHeaderBar() : nullptr)
        {
            pHeaderBar->SetEndDragHdl(Link<HeaderBar*, void>());
        }
        m_xTreeView->SetExpandingHdl(Link<SvTreeListBox*, bool>());
        m_xTreeView->SetDoubleClickHdl(Link<SvTreeListBox*, bool>());
        m_xTreeView->SetSelectHdl(Link<SvTreeListBox*, void>());
    }
};

IMPL_LINK(SalInstanceTreeView, ToggleHdl, SvLBoxButtonData*, pData, void)
{
    SvTreeListEntry* pEntry = pData->GetActEntry();
    SvLBoxButton* pBox = pData->GetActBox();

    for (int i = 1, nCount = pEntry->ItemCount(); i < nCount; ++i)
    {
        SvLBoxItem& rItem = pEntry->GetItem(i);
        if (&rItem == pBox)
        {
            int nRow = m_xTreeView->GetAbsPos(pEntry);
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

    SalInstanceTreeIter aIter(nullptr);
    aIter.iter = pEntry;
    bool bRet = signal_expanding(aIter);

    //expand disallowed, restore placeholder
    if (!bRet && bPlaceHolder)
    {
        m_xTreeView->InsertEntry("<dummy>", pEntry, false, 0, nullptr);
    }

    return bRet;
}

class SalInstanceSpinButton : public SalInstanceEntry, public virtual weld::SpinButton
{
private:
    VclPtr<FormattedField> m_xButton;

    DECL_LINK(UpDownHdl, SpinField&, void);
    DECL_LINK(LoseFocusHdl, Control&, void);
    DECL_LINK(OutputHdl, Edit&, bool);
    DECL_LINK(InputHdl, sal_Int64*, TriState);

    double toField(int nValue) const
    {
        return static_cast<double>(nValue) / Power10(get_digits());
    }

    int fromField(double fValue) const
    {
        return FRound(fValue * Power10(get_digits()));
    }

public:
    SalInstanceSpinButton(FormattedField* pButton, bool bTakeOwnership)
        : SalInstanceEntry(pButton, bTakeOwnership)
        , m_xButton(pButton)
    {
        m_xButton->SetThousandsSep(false);  //off by default, MetricSpinButton enables it
        m_xButton->SetUpHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_xButton->SetDownHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_xButton->SetLoseFocusHdl(LINK(this, SalInstanceSpinButton, LoseFocusHdl));
        m_xButton->SetOutputHdl(LINK(this, SalInstanceSpinButton, OutputHdl));
        m_xButton->SetInputHdl(LINK(this, SalInstanceSpinButton, InputHdl));
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
        m_xButton->SetInputHdl(Link<sal_Int64*, TriState>());
        m_xButton->SetOutputHdl(Link<Edit&, bool>());
        m_xButton->SetLoseFocusHdl(Link<Control&, void>());
        m_xButton->SetDownHdl(Link<SpinField&, void>());
        m_xButton->SetUpHdl(Link<SpinField&, void>());
    }
};

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
    SalInstanceFormattedSpinButton(FormattedField* pButton, bool bTakeOwnership)
        : SalInstanceEntry(pButton, bTakeOwnership)
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
    VclPtr<FixedText> m_xLabel;
public:
    SalInstanceLabel(FixedText* pLabel, bool bTakeOwnership)
        : SalInstanceWidget(pLabel, bTakeOwnership)
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
        SalInstanceWidget* pTargetWidget = dynamic_cast<SalInstanceWidget*>(pTarget);
        m_xLabel->set_mnemonic_widget(pTargetWidget ? pTargetWidget->getWidget() : nullptr);
    }
};

class SalInstanceTextView : public SalInstanceContainer, public virtual weld::TextView
{
private:
    VclPtr<VclMultiLineEdit> m_xTextView;

public:
    SalInstanceTextView(VclMultiLineEdit* pTextView, bool bTakeOwnership)
        : SalInstanceContainer(pTextView, bTakeOwnership)
        , m_xTextView(pTextView)
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        m_xTextView->SetText(rText);
    }

    virtual void replace_selection(const OUString& rText) override
    {
        m_xTextView->ReplaceSelected(rText);
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
};

class SalInstanceExpander : public SalInstanceContainer, public virtual weld::Expander
{
private:
    VclPtr<VclExpander> m_xExpander;

    DECL_LINK(ExpandedHdl, VclExpander&, void);

public:
    SalInstanceExpander(VclExpander* pExpander, bool bTakeOwnership)
        : SalInstanceContainer(pExpander, bTakeOwnership)
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
    DECL_LINK(MousePressHdl, const MouseEvent&, void);
    DECL_LINK(MouseMoveHdl, const MouseEvent&, void);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, void);
    DECL_LINK(KeyPressHdl, const KeyEvent&, bool);
    DECL_LINK(KeyReleaseHdl, const KeyEvent&, bool);
    DECL_LINK(StyleUpdatedHdl, VclDrawingArea&, void);
    DECL_LINK(PopupMenuHdl, const Point&, bool);
    DECL_LINK(QueryTooltipHdl, tools::Rectangle&, OUString);

public:
    SalInstanceDrawingArea(VclDrawingArea* pDrawingArea, const a11yref& rAlly,
            FactoryFunction pUITestFactoryFunction, void* pUserData, bool bTakeOwnership)
        : SalInstanceWidget(pDrawingArea, bTakeOwnership)
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
        m_xDrawingArea->SetPopupMenuHdl(Link<const Point&, bool>());
        m_xDrawingArea->SetStyleUpdatedHdl(Link<VclDrawingArea&, void>());
        m_xDrawingArea->SetMousePressHdl(Link<const MouseEvent&, void>());
        m_xDrawingArea->SetMouseMoveHdl(Link<const MouseEvent&, void>());
        m_xDrawingArea->SetMouseReleaseHdl(Link<const MouseEvent&, void>());
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

IMPL_LINK(SalInstanceDrawingArea, MousePressHdl, const MouseEvent&, rEvent, void)
{
    m_aMousePressHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, MouseMoveHdl, const MouseEvent&, rEvent, void)
{
    m_aMouseMotionHdl.Call(rEvent);
}

IMPL_LINK(SalInstanceDrawingArea, MouseReleaseHdl, const MouseEvent&, rEvent, void)
{
    m_aMouseReleaseHdl.Call(rEvent);
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

IMPL_LINK(SalInstanceDrawingArea, PopupMenuHdl, const Point&, rPos, bool)
{
    return m_aPopupMenuHdl.Call(rPos);
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
    SalInstanceComboBox(vcl_type* pComboBox, bool bTakeOwnership)
        : SalInstanceContainer(pComboBox, bTakeOwnership)
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

    virtual OUString get_active_id() const override
    {
        const OUString* pRet = getEntryData(m_xComboBox->GetSelectedEntryPos());
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
        m_aUserData.emplace_back(o3tl::make_unique<OUString>(rId));
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
};

class SalInstanceComboBoxWithoutEdit : public SalInstanceComboBox<ListBox>
{
private:
    DECL_LINK(SelectHdl, ListBox&, void);

public:
    SalInstanceComboBoxWithoutEdit(ListBox* pListBox, bool bTakeOwnership)
        : SalInstanceComboBox<ListBox>(pListBox, bTakeOwnership)
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
            m_aUserData.emplace_back(o3tl::make_unique<OUString>(*pId));
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

    virtual void set_entry_error(bool /*bError*/) override
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

    virtual void set_entry_completion(bool) override
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
    SalInstanceComboBoxWithEdit(::ComboBox* pComboBox, bool bTakeOwnership)
        : SalInstanceComboBox<::ComboBox>(pComboBox, bTakeOwnership)
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

    virtual void set_entry_error(bool bError) override
    {
        if (bError)
            m_xComboBox->SetControlForeground(Color(0xf0, 0, 0));
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
            m_aUserData.emplace_back(o3tl::make_unique<OUString>(*pId));
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

    virtual void set_entry_completion(bool bEnable) override
    {
        m_xComboBox->EnableAutocomplete(bEnable);
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
    SalInstanceEntryTreeView(vcl::Window *pContainer, bool bTakeOwnership, std::unique_ptr<weld::Entry> xEntry, std::unique_ptr<weld::TreeView> xTreeView)
        : EntryTreeView(std::move(xEntry), std::move(xTreeView))
        , SalInstanceContainer(pContainer, bTakeOwnership)
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

    virtual void set_entry_completion(bool bEnable) override
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

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString &id, bool bTakeOwnership) override
    {
        MessageDialog* pMessageDialog = m_xBuilder->get<MessageDialog>(id);
        std::unique_ptr<weld::MessageDialog> pRet(pMessageDialog ? new SalInstanceMessageDialog(pMessageDialog, false) : nullptr);
        if (bTakeOwnership && pMessageDialog)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pMessageDialog);
            m_xBuilder->drop_ownership(pMessageDialog);
        }
        return pRet;
    }

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString &id, bool bTakeOwnership) override
    {
        Dialog* pDialog = m_xBuilder->get<Dialog>(id);
        std::unique_ptr<weld::Dialog> pRet(pDialog ? new SalInstanceDialog(pDialog, false) : nullptr);
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
        return pWindow ? o3tl::make_unique<SalInstanceWindow>(pWindow, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Widget> weld_widget(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pWidget = m_xBuilder->get<vcl::Window>(id);
        return pWidget ? o3tl::make_unique<SalInstanceWidget>(pWidget, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Container> weld_container(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pContainer = m_xBuilder->get<vcl::Window>(id);
        return pContainer ? o3tl::make_unique<SalInstanceContainer>(pContainer, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Frame> weld_frame(const OString &id, bool bTakeOwnership) override
    {
        VclFrame* pFrame = m_xBuilder->get<VclFrame>(id);
        std::unique_ptr<weld::Frame> pRet(pFrame ? new SalInstanceFrame(pFrame, false) : nullptr);
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
        return pScrolledWindow ? o3tl::make_unique<SalInstanceScrolledWindow>(pScrolledWindow, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString &id, bool bTakeOwnership) override
    {
        TabControl* pNotebook = m_xBuilder->get<TabControl>(id);
        return pNotebook ? o3tl::make_unique<SalInstanceNotebook>(pNotebook, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Button> weld_button(const OString &id, bool bTakeOwnership) override
    {
        Button* pButton = m_xBuilder->get<Button>(id);
        return pButton ? o3tl::make_unique<SalInstanceButton>(pButton, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString &id, bool bTakeOwnership) override
    {
        MenuButton* pButton = m_xBuilder->get<MenuButton>(id);
        return pButton ? o3tl::make_unique<SalInstanceMenuButton>(pButton, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OString &id, bool bTakeOwnership) override
    {
        PushButton* pToggleButton = m_xBuilder->get<PushButton>(id);
        return pToggleButton ? o3tl::make_unique<SalInstanceToggleButton>(pToggleButton, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString &id, bool bTakeOwnership) override
    {
        RadioButton* pRadioButton = m_xBuilder->get<RadioButton>(id);
        return pRadioButton ? o3tl::make_unique<SalInstanceRadioButton>(pRadioButton, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString &id, bool bTakeOwnership) override
    {
        CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
        return pCheckButton ? o3tl::make_unique<SalInstanceCheckButton>(pCheckButton, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Scale> weld_scale(const OString &id, bool bTakeOwnership) override
    {
        Slider* pSlider = m_xBuilder->get<Slider>(id);
        return pSlider ? o3tl::make_unique<SalInstanceScale>(pSlider, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OString &id, bool bTakeOwnership) override
    {
        ::ProgressBar* pProgress = m_xBuilder->get<::ProgressBar>(id);
        return pProgress ? o3tl::make_unique<SalInstanceProgressBar>(pProgress, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Image> weld_image(const OString &id, bool bTakeOwnership) override
    {
        FixedImage* pImage = m_xBuilder->get<FixedImage>(id);
        return pImage ? o3tl::make_unique<SalInstanceImage>(pImage, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Entry> weld_entry(const OString &id, bool bTakeOwnership) override
    {
        Edit* pEntry = m_xBuilder->get<Edit>(id);
        return pEntry ? o3tl::make_unique<SalInstanceEntry>(pEntry, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString &id, bool bTakeOwnership) override
    {
        FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
        return pSpinButton ? o3tl::make_unique<SalInstanceSpinButton>(pSpinButton, bTakeOwnership) : nullptr;
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
        return o3tl::make_unique<weld::MetricSpinButton>(std::move(xButton), eUnit);
    }

    virtual std::unique_ptr<weld::FormattedSpinButton> weld_formatted_spin_button(const OString& id,
                                                                                  bool bTakeOwnership) override
    {
        FormattedField* pSpinButton = m_xBuilder->get<FormattedField>(id);
        return pSpinButton ? o3tl::make_unique<SalInstanceFormattedSpinButton>(pSpinButton, bTakeOwnership) : nullptr;
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
            return o3tl::make_unique<SalInstanceComboBoxWithEdit>(pComboBox, bTakeOwnership);
        ListBox* pListBox = dynamic_cast<ListBox*>(pWidget);
        return pListBox ? o3tl::make_unique<SalInstanceComboBoxWithoutEdit>(pListBox, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::EntryTreeView> weld_entry_tree_view(const OString& containerid, const OString& entryid, const OString& treeviewid, bool bTakeOwnership) override
    {
        vcl::Window* pContainer = m_xBuilder->get<vcl::Window>(containerid);
        return pContainer ? o3tl::make_unique<SalInstanceEntryTreeView>(pContainer, bTakeOwnership, weld_entry(entryid, bTakeOwnership),
                                                                        weld_tree_view(treeviewid, bTakeOwnership)) : nullptr;
    }

    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString &id, bool bTakeOwnership) override
    {
        SvTabListBox* pTreeView = m_xBuilder->get<SvTabListBox>(id);
        return pTreeView ? o3tl::make_unique<SalInstanceTreeView>(pTreeView, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Label> weld_label(const OString &id, bool bTakeOwnership) override
    {
        FixedText* pLabel = m_xBuilder->get<FixedText>(id);
        return pLabel ? o3tl::make_unique<SalInstanceLabel>(pLabel, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString &id, bool bTakeOwnership) override
    {
        VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
        return pTextView ? o3tl::make_unique<SalInstanceTextView>(pTextView, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Expander> weld_expander(const OString &id, bool bTakeOwnership) override
    {
        VclExpander* pExpander = m_xBuilder->get<VclExpander>(id);
        return pExpander ? o3tl::make_unique<SalInstanceExpander>(pExpander, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::DrawingArea> weld_drawing_area(const OString &id, const a11yref& rA11yImpl,
            FactoryFunction pUITestFactoryFunction, void* pUserData, bool bTakeOwnership) override
    {
        VclDrawingArea* pDrawingArea = m_xBuilder->get<VclDrawingArea>(id);
        return pDrawingArea ? o3tl::make_unique<SalInstanceDrawingArea>(pDrawingArea, rA11yImpl,
                pUITestFactoryFunction, pUserData, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::Menu> weld_menu(const OString &id, bool bTakeOwnership) override
    {
        PopupMenu* pMenu = m_xBuilder->get_menu(id);
        return pMenu ? o3tl::make_unique<SalInstanceMenu>(pMenu, bTakeOwnership) : nullptr;
    }

    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override
    {
        return o3tl::make_unique<SalInstanceSizeGroup>();
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

weld::MessageDialog* SalInstance::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType, VclButtonsType eButtonsType, const OUString& rPrimaryMessage)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    SystemWindow* pParentWidget = pParentInstance ? pParentInstance->getSystemWindow() : nullptr;
    VclPtrInstance<MessageDialog> xMessageDialog(pParentWidget, rPrimaryMessage, eMessageType, eButtonsType);
    return new SalInstanceMessageDialog(xMessageDialog, true);
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
            m_xFrameWeld.reset(new SalInstanceWindow(pWindow, false));
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
            m_xOrigParent.reset(m_xRelocate->weld_parent());
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
