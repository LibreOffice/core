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

#include <salframe.hxx>
#include <salinst.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <saltimer.hxx>
#include <salimestatus.hxx>
#include <salsys.hxx>
#include <salbmp.hxx>
#include <salobj.hxx>
#include <salmenu.hxx>
#include <svdata.hxx>
#include <vcl/messagedialog.hxx>
#include <vcl/builder.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/weld.hxx>

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

SalInstance::~SalInstance()
{
}

SalMenu* SalInstance::CreateMenu( bool, Menu* )
{
    // default: no native menus
    return nullptr;
}

void SalInstance::DestroyMenu( SalMenu* pMenu )
{
    OSL_ENSURE( pMenu == nullptr, "DestroyMenu called with non-native menus" );
}

SalMenuItem* SalInstance::CreateMenuItem( const SalItemParams* )
{
    return nullptr;
}

void SalInstance::DestroyMenuItem( SalMenuItem* pItem )
{
    OSL_ENSURE( pItem == nullptr, "DestroyMenu called with non-native menus" );
}

bool SalInstance::CallEventCallback( void const * pEvent, int nBytes )
{
    return m_pEventInst.is() && m_pEventInst->dispatchEvent( pEvent, nBytes );
}

SalI18NImeStatus* SalInstance::CreateI18NImeStatus()
{
    return new SalI18NImeStatus;
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
    bool m_bTakeOwnership;

public:
    SalInstanceWidget(vcl::Window* pWidget, bool bTakeOwnership)
        : m_xWidget(pWidget)
        , m_bTakeOwnership(bTakeOwnership)
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

    virtual void grab_focus() override
    {
        m_xWidget->GrabFocus();
    }

    virtual bool has_focus() const override
    {
        return m_xWidget->HasFocus();
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

    virtual Size get_preferred_size() const override
    {
        return m_xWidget->get_preferred_size();
    }

    virtual float get_approximate_char_width() const override
    {
        return m_xWidget->approximate_char_width();
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

    virtual OString get_help_id() const override
    {
        return m_xWidget->GetHelpId();
    }

    virtual weld::Container* weld_parent() const override;

    virtual ~SalInstanceWidget() override
    {
        if (m_bTakeOwnership)
            m_xWidget.disposeAndClear();
    }

    vcl::Window* getWidget()
    {
        return m_xWidget;
    }

    SystemWindow* getSystemWindow()
    {
        return m_xWidget->GetSystemWindow();
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
    virtual void remove(weld::Widget* pWidget) override
    {
        SalInstanceWidget* pGtkWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
        assert(pGtkWidget);
        pGtkWidget->getWidget()->SetParent(nullptr);
    }
    virtual void add(weld::Widget* pWidget) override
    {
        SalInstanceWidget* pGtkWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
        assert(pGtkWidget);
        pGtkWidget->getWidget()->SetParent(m_xContainer);
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
    VclPtr<SystemWindow> m_xWindow;

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
    SalInstanceWindow(SystemWindow* pWindow, bool bTakeOwnership)
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

    bool help()
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
        return false;
    }

    virtual ~SalInstanceWindow() override
    {
        clear_child_help(m_xWindow);
    }
};

IMPL_LINK_NOARG(SalInstanceWindow, HelpHdl, vcl::Window&, bool)
{
    return help();
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

    virtual int run() override
    {
        return m_xDialog->Execute();
    }

    virtual void response(int nResponse) override
    {
        m_xDialog->EndDialog(nResponse);
    }

    virtual void add_button(const OUString& rText, int nResponse) override
    {
        VclButtonBox* pBox = m_xDialog->get_action_area();
        VclPtr<PushButton> xButton(VclPtr<PushButton>::Create(pBox, WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER));
        xButton->SetText(rText);
        m_xDialog->add_button(xButton, nResponse, true);
    }

    virtual void set_default_response(int nResponse) override
    {
        m_xDialog->set_default_response(nResponse);
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

class SalInstanceNotebook : public SalInstanceContainer, public virtual weld::Notebook
{
private:
    VclPtr<TabControl> m_xNotebook;
    mutable std::vector<std::unique_ptr<SalInstanceContainer>> m_aPages;

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

    virtual int get_n_pages() const override
    {
        return m_xNotebook->GetPageCount();
    }

    virtual ~SalInstanceNotebook() override
    {
        m_xNotebook->SetActivatePageHdl(Link<TabControl*,void>());
        m_xNotebook->SetDeactivatePageHdl(Link<TabControl*,bool>());
    }
};

IMPL_LINK_NOARG(SalInstanceNotebook, DeactivatePageHdl, TabControl*, bool)
{
    return m_aLeavePageHdl.Call(get_current_page_ident());
}

IMPL_LINK_NOARG(SalInstanceNotebook, ActivatePageHdl, TabControl*, void)
{
    m_aEnterPageHdl.Call(get_current_page_ident());
}

class SalInstanceButton : public SalInstanceContainer, public virtual weld::Button
{
private:
    VclPtr<::Button> m_xButton;
    Link<::Button*,void> m_aOldClickHdl;

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
        m_xRadioButton->Check(active);
    }

    virtual bool get_active() const override
    {
        return m_xRadioButton->IsChecked();
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
        m_xCheckButton->Check(active);
    }

    virtual bool get_active() const override
    {
        return m_xCheckButton->IsChecked();
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
        m_xCheckButton->SetState(inconsistent ? TRISTATE_INDET : TRISTATE_FALSE);
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
    signal_toggled();
}

class SalInstanceEntry : public SalInstanceWidget, public virtual weld::Entry
{
private:
    VclPtr<Edit> m_xEntry;

    DECL_LINK(ChangeHdl, Edit&, void);

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

    WeldTextFilter m_aTextFilter;
public:
    SalInstanceEntry(Edit* pEntry, bool bTakeOwnership)
        : SalInstanceWidget(pEntry, bTakeOwnership)
        , m_xEntry(pEntry)
        , m_aTextFilter(m_aInsertTextHdl)
    {
        m_xEntry->SetModifyHdl(LINK(this, SalInstanceEntry, ChangeHdl));
        m_xEntry->SetTextFilter(&m_aTextFilter);
    }

    virtual void set_text(const OUString& rText) override
    {
        m_xEntry->SetText(rText);
    }

    virtual OUString get_text() const override
    {
        return m_xEntry->GetText();
    }

    virtual void set_width_chars(int nChars) override
    {
        m_xEntry->SetWidthInChars(nChars);
    }

    virtual ~SalInstanceEntry() override
    {
        m_xEntry->SetTextFilter(nullptr);
        m_xEntry->SetModifyHdl(Link<Edit&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceEntry, ChangeHdl, Edit&, void)
{
    signal_changed();
}

class SalInstanceTreeView : public SalInstanceContainer, public virtual weld::TreeView
{
private:
    VclPtr<ListBox> m_xTreeView;

    DECL_LINK(SelectHdl, ListBox&, void);
    DECL_LINK(DoubleClickHdl, ListBox&, void);

public:
    SalInstanceTreeView(ListBox* pTreeView, bool bTakeOwnership)
        : SalInstanceContainer(pTreeView, bTakeOwnership)
        , m_xTreeView(pTreeView)
    {
        m_xTreeView->SetSelectHdl(LINK(this, SalInstanceTreeView, SelectHdl));
        m_xTreeView->SetDoubleClickHdl(LINK(this, SalInstanceTreeView, DoubleClickHdl));
    }

    virtual void append(const OUString& rText) override
    {
        m_xTreeView->InsertEntry(rText);
    }

    virtual void insert(const OUString& rText, int pos) override
    {
        m_xTreeView->InsertEntry(rText, pos);
    }

    using SalInstanceContainer::remove;

    virtual void remove(int pos) override
    {
        m_xTreeView->RemoveEntry(pos);
    }

    virtual int find(const OUString& rText) const override
    {
        sal_Int32 nRet = m_xTreeView->GetEntryPos(rText);
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    virtual void set_top_entry(int pos) override
    {
        m_xTreeView->SetTopEntry(pos);
    }

    virtual void clear() override
    {
        m_xTreeView->Clear();
    }

    virtual int n_children() const override
    {
        return m_xTreeView->GetEntryCount();
    }

    virtual void select(int pos) override
    {
        if (pos == -1)
        {
            m_xTreeView->SetNoSelection();
            return;
        }
        m_xTreeView->SelectEntryPos(pos);
    }

    virtual OUString get_selected() override
    {
        return m_xTreeView->GetSelectedEntry();
    }

    virtual OUString get(int pos) override
    {
        return m_xTreeView->GetEntry(pos);
    }

    virtual int get_selected_index() override
    {
        const sal_Int32 nRet = m_xTreeView->GetSelectedEntryPos();
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    virtual void freeze() override
    {
        m_xTreeView->SetUpdateMode(false);
    }

    virtual void thaw() override
    {
        m_xTreeView->SetUpdateMode(true);
    }

    virtual int get_height_rows(int nRows) const override
    {
        return m_xTreeView->CalcWindowSizePixel(nRows);
    }

    virtual ~SalInstanceTreeView() override
    {
        m_xTreeView->SetDoubleClickHdl(Link<ListBox&, void>());
        m_xTreeView->SetSelectHdl(Link<ListBox&, void>());
    }
};

IMPL_LINK_NOARG(SalInstanceTreeView, SelectHdl, ListBox&, void)
{
    signal_changed();
}

IMPL_LINK_NOARG(SalInstanceTreeView, DoubleClickHdl, ListBox&, void)
{
    signal_row_activated();
}

class SalInstanceSpinButton : public SalInstanceEntry, public virtual weld::SpinButton
{
private:
    VclPtr<NumericField> m_xButton;

    DECL_LINK(UpDownHdl, SpinField&, void);
    DECL_LINK(LoseFocusHdl, Control&, void);
    DECL_LINK(OutputHdl, Edit&, bool);

public:
    SalInstanceSpinButton(NumericField* pButton, bool bTakeOwnership)
        : SalInstanceEntry(pButton, bTakeOwnership)
        , m_xButton(pButton)
    {
        m_xButton->SetUpHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_xButton->SetDownHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_xButton->SetLoseFocusHdl(LINK(this, SalInstanceSpinButton, LoseFocusHdl));
        m_xButton->SetOutputHdl(LINK(this, SalInstanceSpinButton, OutputHdl));
    }

    virtual int get_value() const override
    {
        return m_xButton->GetValue();
    }

    virtual void set_value(int value) override
    {
        m_xButton->SetValue(value);
    }

    virtual void set_range(int min, int max) override
    {
        m_xButton->SetMin(min);
        m_xButton->SetFirst(min);
        m_xButton->SetMax(max);
        m_xButton->SetLast(max);
    }

    virtual void get_range(int& min, int& max) const override
    {
        min = m_xButton->GetMin();
        max = m_xButton->GetMax();
    }

    virtual void set_increments(int step, int /*page*/) override
    {
        m_xButton->SetSpinSize(step);
    }

    virtual void get_increments(int& step, int& page) const override
    {
        step = m_xButton->GetSpinSize();
        page = m_xButton->GetSpinSize();
    }

    virtual void set_digits(unsigned int digits) override
    {
        m_xButton->SetDecimalDigits(digits);
    }

    virtual unsigned int get_digits() const override
    {
        return m_xButton->GetDecimalDigits();
    }

    virtual ~SalInstanceSpinButton() override
    {
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

    virtual OUString get_text() const override
    {
        return m_xTextView->GetText();
    }

    virtual Selection get_selection() const override
    {
        return m_xTextView->GetSelection();
    }

    virtual void set_selection(const Selection& rSelection) override
    {
        m_xTextView->SetSelection(rSelection);
    }
};

class SalInstanceDrawingArea : public SalInstanceWidget, public virtual weld::DrawingArea
{
private:
    VclPtr<VclDrawingArea> m_xDrawingArea;

    DECL_LINK(PaintHdl, vcl::RenderContext&, void);
    DECL_LINK(ResizeHdl, const Size&, void);

public:
    SalInstanceDrawingArea(VclDrawingArea* pDrawingArea, bool bTakeOwnership)
        : SalInstanceWidget(pDrawingArea, bTakeOwnership)
        , m_xDrawingArea(pDrawingArea)
    {
        m_xDrawingArea->SetPaintHdl(LINK(this, SalInstanceDrawingArea, PaintHdl));
        m_xDrawingArea->SetResizeHdl(LINK(this, SalInstanceDrawingArea, ResizeHdl));
    }

    virtual void queue_draw() override
    {
        m_xDrawingArea->Invalidate();
    }

    virtual ~SalInstanceDrawingArea() override
    {
        m_xDrawingArea->SetResizeHdl(Link<const Size&, void>());
        m_xDrawingArea->SetPaintHdl(Link<vcl::RenderContext&, void>());
    }
};

IMPL_LINK(SalInstanceDrawingArea, PaintHdl, vcl::RenderContext&, rDevice, void)
{
    m_aDrawHdl.Call(rDevice);
}

IMPL_LINK(SalInstanceDrawingArea, ResizeHdl, const Size&, rSize, void)
{
    m_aSizeAllocateHdl.Call(rSize);
}

//ComboBox and ListBox have the same apis, ComboBoxes in LibreOffice have an edit box and ListBoxes
//don't. This distinction isn't there in Gtk. Use a template to sort this problem out.
template <class vcl_type>
class SalInstanceComboBoxText : public SalInstanceContainer, public virtual weld::ComboBoxText
{
private:
    VclPtr<vcl_type> m_xComboBoxText;

    static void LinkStubSetSelectHdl(void* instance, vcl_type&)
    {
        return static_cast<SalInstanceComboBoxText*>(instance)->signal_changed();
    }

public:
    SalInstanceComboBoxText(vcl_type* pComboBoxText, bool bTakeOwnership)
        : SalInstanceContainer(pComboBoxText, bTakeOwnership)
        , m_xComboBoxText(pComboBoxText)
    {
        m_xComboBoxText->SetSelectHdl(LINK(this, SalInstanceComboBoxText, SetSelectHdl));
    }

    virtual int get_active() const override
    {
        const sal_Int32 nRet = m_xComboBoxText->GetSelectedEntryPos();
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    const OUString* getEntryData(int index) const
    {
        return static_cast<const OUString*>(m_xComboBoxText->GetEntryData(index));
    }

    virtual OUString get_active_id() const override
    {
        const OUString* pRet = getEntryData(m_xComboBoxText->GetSelectedEntryPos());
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
                m_xComboBoxText->SelectEntryPos(i);
        }
    }

    virtual void set_active(int pos) override
    {
        if (pos == -1)
        {
            m_xComboBoxText->SetNoSelection();
            return;
        }
        m_xComboBoxText->SelectEntryPos(pos);
    }

    virtual OUString get_active_text() const override
    {
        return m_xComboBoxText->GetSelectedEntry();
    }

    virtual OUString get_text(int pos) const override
    {
        return m_xComboBoxText->GetEntry(pos);
    }

    virtual OUString get_id(int pos) const override
    {
        const OUString* pRet = getEntryData(pos);
        if (!pRet)
            return OUString();
        return *pRet;
    }

    virtual void append_text(const OUString& rStr) override
    {
        m_xComboBoxText->InsertEntry(rStr);
    }

    virtual void insert_text(int pos, const OUString& rStr) override
    {
        m_xComboBoxText->InsertEntry(rStr, pos);
    }

    virtual void append(const OUString& rId, const OUString& rStr) override
    {
        m_xComboBoxText->SetEntryData(m_xComboBoxText->InsertEntry(rStr), new OUString(rId));
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr) override
    {
        m_xComboBoxText->SetEntryData(m_xComboBoxText->InsertEntry(rStr, pos), new OUString(rId));
    }

    virtual int get_count() const override
    {
        return m_xComboBoxText->GetEntryCount();
    }

    virtual int find_text(const OUString& rStr) const override
    {
        const sal_Int32 nRet = m_xComboBoxText->GetEntryPos(rStr);
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    virtual void clear() override
    {
        for (int i = 0; i < get_count(); ++i)
        {
            const OUString* pId = getEntryData(i);
            delete pId;
        }
        return m_xComboBoxText->Clear();
    }

    virtual void make_sorted() override
    {
        m_xComboBoxText->SetStyle(m_xComboBoxText->GetStyle() | WB_SORT);
    }

    virtual ~SalInstanceComboBoxText() override
    {
        m_xComboBoxText->SetSelectHdl(Link<vcl_type&, void>());
        clear();
    }
};

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

    virtual weld::MessageDialog* weld_message_dialog(const OString &id, bool bTakeOwnership) override
    {
        MessageDialog* pMessageDialog = m_xBuilder->get<MessageDialog>(id);
        weld::MessageDialog* pRet = pMessageDialog ? new SalInstanceMessageDialog(pMessageDialog, false) : nullptr;
        if (bTakeOwnership && pMessageDialog)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pMessageDialog);
            m_xBuilder->drop_ownership(pMessageDialog);
        }
        return pRet;
    }

    virtual weld::Dialog* weld_dialog(const OString &id, bool bTakeOwnership) override
    {
        Dialog* pDialog = m_xBuilder->get<Dialog>(id);
        weld::Dialog* pRet = pDialog ? new SalInstanceDialog(pDialog, false) : nullptr;
        if (bTakeOwnership && pDialog)
        {
            assert(!m_aOwnedToplevel && "only one toplevel per .ui allowed");
            m_aOwnedToplevel.set(pDialog);
            m_xBuilder->drop_ownership(pDialog);
        }
        return pRet;
    }

    virtual weld::Window* weld_window(const OString &id, bool bTakeOwnership) override
    {
        SystemWindow* pWindow = m_xBuilder->get<SystemWindow>(id);
        return pWindow ? new SalInstanceWindow(pWindow, bTakeOwnership) : nullptr;
    }

    virtual weld::Widget* weld_widget(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pWidget = m_xBuilder->get<vcl::Window>(id);
        return pWidget ? new SalInstanceWidget(pWidget, bTakeOwnership) : nullptr;
    }

    virtual weld::Container* weld_container(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pContainer = m_xBuilder->get<vcl::Window>(id);
        return pContainer ? new SalInstanceContainer(pContainer, bTakeOwnership) : nullptr;
    }

    virtual weld::Frame* weld_frame(const OString &id, bool bTakeOwnership) override
    {
        VclFrame* pFrame = m_xBuilder->get<VclFrame>(id);
        return pFrame ? new SalInstanceFrame(pFrame, bTakeOwnership) : nullptr;
    }

    virtual weld::Notebook* weld_notebook(const OString &id, bool bTakeOwnership) override
    {
        TabControl* pNotebook = m_xBuilder->get<TabControl>(id);
        return pNotebook ? new SalInstanceNotebook(pNotebook, bTakeOwnership) : nullptr;
    }

    virtual weld::Button* weld_button(const OString &id, bool bTakeOwnership) override
    {
        Button* pButton = m_xBuilder->get<Button>(id);
        return pButton ? new SalInstanceButton(pButton, bTakeOwnership) : nullptr;
    }

    virtual weld::RadioButton* weld_radio_button(const OString &id, bool bTakeOwnership) override
    {
        RadioButton* pRadioButton = m_xBuilder->get<RadioButton>(id);
        return pRadioButton ? new SalInstanceRadioButton(pRadioButton, bTakeOwnership) : nullptr;
    }

    virtual weld::CheckButton* weld_check_button(const OString &id, bool bTakeOwnership) override
    {
        CheckBox* pCheckButton = m_xBuilder->get<CheckBox>(id);
        return pCheckButton ? new SalInstanceCheckButton(pCheckButton, bTakeOwnership) : nullptr;
    }

    virtual weld::Entry* weld_entry(const OString &id, bool bTakeOwnership) override
    {
        Edit* pEntry = m_xBuilder->get<Edit>(id);
        return pEntry ? new SalInstanceEntry(pEntry, bTakeOwnership) : nullptr;
    }

    virtual weld::SpinButton* weld_spin_button(const OString &id, bool bTakeOwnership) override
    {
        NumericField* pSpinButton = m_xBuilder->get<NumericField>(id);
        return pSpinButton ? new SalInstanceSpinButton(pSpinButton, bTakeOwnership) : nullptr;
    }

    virtual weld::ComboBoxText* weld_combo_box_text(const OString &id, bool bTakeOwnership) override
    {
        vcl::Window* pComboBoxText = m_xBuilder->get<vcl::Window>(id);
        ComboBox* pComboBox = dynamic_cast<ComboBox*>(pComboBoxText);
        if (pComboBox)
            return new SalInstanceComboBoxText<ComboBox>(pComboBox, bTakeOwnership);
        ListBox* pListBox = dynamic_cast<ListBox*>(pComboBoxText);
        return pListBox ? new SalInstanceComboBoxText<ListBox>(pListBox, bTakeOwnership) : nullptr;
    }

    virtual weld::TreeView* weld_tree_view(const OString &id, bool bTakeOwnership) override
    {
        ListBox* pTreeView = m_xBuilder->get<ListBox>(id);
        return pTreeView ? new SalInstanceTreeView(pTreeView, bTakeOwnership) : nullptr;
    }

    virtual weld::Label* weld_label(const OString &id, bool bTakeOwnership) override
    {
        FixedText* pLabel = m_xBuilder->get<FixedText>(id);
        return pLabel ? new SalInstanceLabel(pLabel, bTakeOwnership) : nullptr;
    }

    virtual weld::TextView* weld_text_view(const OString &id, bool bTakeOwnership) override
    {
        VclMultiLineEdit* pTextView = m_xBuilder->get<VclMultiLineEdit>(id);
        return pTextView ? new SalInstanceTextView(pTextView, bTakeOwnership) : nullptr;
    }

    virtual weld::DrawingArea* weld_drawing_area(const OString &id, bool bTakeOwnership) override
    {
        VclDrawingArea* pDrawingArea = m_xBuilder->get<VclDrawingArea>(id);
        return pDrawingArea ? new SalInstanceDrawingArea(pDrawingArea, bTakeOwnership) : nullptr;
    }

    virtual ~SalInstanceBuilder() override
    {
        if (VclBuilderContainer* pOwnedToplevel = dynamic_cast<VclBuilderContainer*>(m_aOwnedToplevel.get()))
            pOwnedToplevel->m_pUIBuilder = std::move(m_xBuilder);
        m_aOwnedToplevel.disposeAndClear();
    }
};

weld::Builder* SalInstance::CreateBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    vcl::Window* pParentWidget = pParentInstance ? pParentInstance->getWidget() : nullptr;
    return new SalInstanceBuilder(pParentWidget, rUIRoot, rUIFile);
}

weld::MessageDialog* SalInstance::CreateMessageDialog(weld::Widget* pParent, VclMessageType eMessageType, VclButtonsType eButtonsType, const OUString& rPrimaryMessage)
{
    SalInstanceWidget* pParentInstance = dynamic_cast<SalInstanceWidget*>(pParent);
    SystemWindow* pParentWidget = pParentInstance ? pParentInstance->getSystemWindow() : nullptr;
    VclPtrInstance<MessageDialog> xMessageDialog(pParentWidget, rPrimaryMessage, eMessageType, eButtonsType);
    return new SalInstanceMessageDialog(xMessageDialog, true);
}

weld::Window* SalFrame::GetFrameWeld() const
{
    if (!m_xFrameWeld)
    {
        vcl::Window* pWindow = GetWindow();
        pWindow = pWindow ? pWindow->ImplGetWindow() : nullptr;
        SystemWindow* pSystemWindow = pWindow ? pWindow->GetSystemWindow() : nullptr;
        if (pSystemWindow)
            m_xFrameWeld.reset(new SalInstanceWindow(pSystemWindow, false));
    }
    return m_xFrameWeld.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
