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
#include <vcl/builder.hxx>
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

class SalInstanceWidget : public virtual Weld::Widget
{
private:
    vcl::Window* m_pWidget;
    bool m_bTakeOwnership;

public:
    SalInstanceWidget(vcl::Window* pWidget, bool bTakeOwnership)
        : m_pWidget(pWidget)
        , m_bTakeOwnership(bTakeOwnership)
    {
    }

    virtual void set_sensitive(bool sensitive) override
    {
        m_pWidget->Enable(sensitive);
    }

    virtual bool get_sensitive() const override
    {
        return m_pWidget->IsEnabled();
    }

    virtual void set_visible(bool visible) override
    {
        m_pWidget->Show(visible);
    }

    virtual bool get_visible() const override
    {
        return m_pWidget->IsVisible();
    }

    virtual void grab_focus() override
    {
        m_pWidget->GrabFocus();
    }

    virtual bool has_focus() const override
    {
        return m_pWidget->HasFocus();
    }

    virtual void show() override
    {
        m_pWidget->Show();
    }

    virtual void hide() override
    {
        m_pWidget->Hide();
    }

    virtual void set_size_request(int nWidth, int nHeight) override
    {
        m_pWidget->set_width_request(nWidth);
        m_pWidget->set_height_request(nHeight);
    }

    virtual Size get_preferred_size() const override
    {
        return m_pWidget->get_preferred_size();
    }

    virtual float approximate_char_width() const override
    {
        return m_pWidget->approximate_char_width();
    }

    virtual Size get_pixel_size(const OUString& rText) const override
    {
        //TODO, or do I want GetTextBoundRect ?, just using width at the moment anyway
        return Size(m_pWidget->GetTextWidth(rText), m_pWidget->GetTextHeight());
    }

    OString get_buildable_name() const override
    {
        return m_pWidget->get_id().toUtf8();
    }

    virtual ~SalInstanceWidget() override
    {
        if (m_bTakeOwnership)
            delete m_pWidget;
    }

    vcl::Window* get_widget()
    {
        return m_pWidget;
    }
};

class SalInstanceContainer : public SalInstanceWidget, public virtual Weld::Container
{
private:
    vcl::Window* m_pContainer;
public:
    SalInstanceContainer(vcl::Window* pContainer, bool bTakeOwnership)
        : SalInstanceWidget(pContainer, bTakeOwnership)
        , m_pContainer(pContainer)
    {
    }

    virtual void add(Weld::Widget* pWidget) override
    {
        SalInstanceWidget* pInstanceWidget = dynamic_cast<SalInstanceWidget*>(pWidget);
        pInstanceWidget->get_widget()->SetParent(m_pContainer);
    }
};

class SalInstanceWindow : public SalInstanceContainer, public virtual Weld::Window
{
private:
    SystemWindow* m_pWindow;

public:
    SalInstanceWindow(SystemWindow* pWindow, bool bTakeOwnership)
        : SalInstanceContainer(pWindow, bTakeOwnership)
        , m_pWindow(pWindow)
    {
    }

    virtual void set_transient_for(Weld::Window* pParent) override
    {
        SalInstanceWindow* pParentFrame = dynamic_cast<SalInstanceWindow*>(pParent);
        SystemWindow* pParentWidget = pParentFrame->getWindow();
        m_pWindow->SetParent(pParentWidget);
    }

    virtual void set_title(const OUString& rTitle) override
    {
        m_pWindow->SetText(rTitle);
    }

    virtual OUString get_title() const override
    {
        return m_pWindow->GetText();
    }

    SystemWindow* getWindow()
    {
        return m_pWindow;
    }
};

class SalInstanceDialog : public SalInstanceWindow, public virtual Weld::Dialog
{
private:
    ::Dialog* m_pDialog;

public:
    SalInstanceDialog(::Dialog* pDialog, bool bTakeOwnership)
        : SalInstanceWindow(pDialog, bTakeOwnership)
        , m_pDialog(pDialog)
    {
    }

    virtual int run() override
    {
        return m_pDialog->Execute();
    }

    virtual void response(int nResponse) override
    {
        m_pDialog->EndDialog(nResponse);
    }
};


class SalInstanceFrame : public SalInstanceContainer, public virtual Weld::Frame
{
private:
    VclFrame* m_pFrame;
public:
    SalInstanceFrame(VclFrame* pFrame, bool bTakeOwnership)
        : SalInstanceContainer(pFrame, bTakeOwnership)
        , m_pFrame(pFrame)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        m_pFrame->set_label(rText);
    }

    virtual OUString get_label() const override
    {
        return m_pFrame->get_label();
    }
};

class SalInstanceNotebook : public SalInstanceContainer, public virtual Weld::Notebook
{
private:
    TabControl* m_pNotebook;
    mutable std::vector<std::unique_ptr<SalInstanceContainer>> m_aPages;

    DECL_LINK(DeactivatePageHdl, TabControl*, bool)
    {
        return m_aLeavePageHdl.Call(get_current_page_ident());
    }

    DECL_LINK(ActivatePageHdl, TabControl*, void)
    {
        m_aEnterPageHdl.Call(get_current_page_ident());
    }

public:
    SalInstanceNotebook(TabControl* pNotebook, bool bTakeOwnership)
        : SalInstanceContainer(pNotebook, bTakeOwnership)
        , m_pNotebook(pNotebook)
    {
        m_pNotebook->SetActivatePageHdl(LINK(this, SalInstanceNotebook, ActivatePageHdl));
        m_pNotebook->SetDeactivatePageHdl(LINK(this, SalInstanceNotebook, DeactivatePageHdl));
    }

    virtual int get_current_page() const override
    {
        return m_pNotebook->GetPagePos(m_pNotebook->GetCurPageId());
    }

    virtual OString get_current_page_ident() const override
    {
        return m_pNotebook->GetPageName(m_pNotebook->GetCurPageId());
    }

    virtual Weld::Container* get_page(const OString& rIdent) const override
    {
        sal_uInt16 nPageId = m_pNotebook->GetPageId(rIdent);
        sal_uInt16 nPageIndex = m_pNotebook->GetPagePos(nPageId);
        if (nPageIndex == TAB_PAGE_NOTFOUND)
            return nullptr;
        TabPage* pPage = m_pNotebook->GetTabPage(nPageId);
        vcl::Window* pChild = pPage->GetChild(0);
        if (m_aPages.size() < nPageIndex + 1U)
            m_aPages.resize(nPageIndex + 1U);
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new SalInstanceContainer(pChild, false));
        return m_aPages[nPageIndex].get();
    }

    virtual void set_current_page(int nPage) override
    {
        m_pNotebook->SetCurPageId(m_pNotebook->GetPageId(nPage));
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        m_pNotebook->SetCurPageId(m_pNotebook->GetPageId(rIdent));
    }

    virtual int get_n_pages() const override
    {
        return m_pNotebook->GetPageCount();
    }

    virtual ~SalInstanceNotebook() override
    {
        m_pNotebook->SetActivatePageHdl(Link<TabControl*,void>());
        m_pNotebook->SetDeactivatePageHdl(Link<TabControl*,bool>());
    }
};

class SalInstanceButton : public SalInstanceContainer, public virtual Weld::Button
{
private:
    ::Button* m_pButton;

    DECL_LINK(ClickHdl, ::Button*, void)
    {
        signal_clicked();
    }
public:
    SalInstanceButton(::Button* pButton, bool bTakeOwnership)
        : SalInstanceContainer(pButton, bTakeOwnership)
        , m_pButton(pButton)
    {
        m_pButton->SetClickHdl(LINK(this, SalInstanceButton, ClickHdl));
    }

    virtual void set_label(const OUString& rText) override
    {
        m_pButton->SetText(rText);
    }

    virtual OUString get_label() const override
    {
        return m_pButton->GetText();
    }

    virtual ~SalInstanceButton() override
    {
        m_pButton->SetClickHdl(Link<::Button*,void>());
    }
};

class SalInstanceRadioButton : public SalInstanceButton, public virtual Weld::RadioButton
{
private:
    ::RadioButton* m_pRadioButton;

    DECL_LINK(ToggleHdl, ::RadioButton&, void)
    {
        signal_toggled();
    }

public:
    SalInstanceRadioButton(::RadioButton* pButton, bool bTakeOwnership)
        : SalInstanceButton(pButton, bTakeOwnership)
        , m_pRadioButton(pButton)
    {
        m_pRadioButton->SetToggleHdl(LINK(this, SalInstanceRadioButton, ToggleHdl));
    }

    virtual void set_active(bool active) override
    {
        m_pRadioButton->Check(active);
    }

    virtual bool get_active() const override
    {
        return m_pRadioButton->IsChecked();
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
        m_pRadioButton->SetToggleHdl(Link<::RadioButton&, void>());
    }
};

class SalInstanceCheckButton : public SalInstanceButton, public virtual Weld::CheckButton
{
private:
    CheckBox* m_pCheckButton;

    DECL_LINK(ToggleHdl, CheckBox&, void)
    {
        signal_toggled();
    }

public:
    SalInstanceCheckButton(CheckBox* pButton, bool bTakeOwnership)
        : SalInstanceButton(pButton, bTakeOwnership)
        , m_pCheckButton(pButton)
    {
        m_pCheckButton->SetToggleHdl(LINK(this, SalInstanceCheckButton, ToggleHdl));
    }

    virtual void set_active(bool active) override
    {
        m_pCheckButton->Check(active);
    }

    virtual bool get_active() const override
    {
        return m_pCheckButton->IsChecked();
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
        m_pCheckButton->SetState(inconsistent ? TRISTATE_INDET : TRISTATE_FALSE);
    }

    virtual bool get_inconsistent() const override
    {
        return m_pCheckButton->GetState() == TRISTATE_INDET;
    }

    virtual ~SalInstanceCheckButton() override
    {
        m_pCheckButton->SetToggleHdl(Link<CheckBox&, void>());
    }
};

class SalInstanceEntry : public SalInstanceWidget, public virtual Weld::Entry
{
private:
    Edit* m_pEntry;

    DECL_LINK(ChangeHdl, Edit&, void)
    {
        signal_changed();
    }

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

        virtual OUString filter(const OUString &rText)
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
        , m_pEntry(pEntry)
        , m_aTextFilter(m_aInsertTextHdl)
    {
        m_pEntry->SetModifyHdl(LINK(this, SalInstanceEntry, ChangeHdl));
        m_pEntry->SetTextFilter(&m_aTextFilter);
    }

    virtual void set_text(const OUString& rText) override
    {
        m_pEntry->SetText(rText);
    }

    virtual OUString get_text() const override
    {
        return m_pEntry->GetText();
    }

    virtual void set_width_chars(int nChars) override
    {
        m_pEntry->SetWidthInChars(nChars);
    }

    virtual ~SalInstanceEntry() override
    {
        m_pEntry->SetTextFilter(nullptr);
        m_pEntry->SetModifyHdl(Link<Edit&, void>());
    }
};
#if 0
namespace
{
    struct Search
    {
        OString str;
        int index = -1;
        Search(const OUString& rText)
            : str(OUStringToOString(rText, RTL_TEXTENCODING_UTF8))
            , index(-1)
        {
        }
    };

    bool foreach_find(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, Search* search)
    {
        gchar *pStr = nullptr;
        gtk_tree_model_get(model, iter, 0, &pStr, -1);
        bool found = strcmp(pStr, search->str.getStr()) == 0;
        if (found)
            search->index = gtk_tree_path_get_indices(path)[0];
        g_free(pStr);
        return found;
    }
}

class GtkInstanceTreeView : public GtkInstanceContainer, public virtual Weld::TreeView
{
private:
    GtkTreeView* m_pTreeView;
    GtkListStore* m_pListStore;
    gulong m_nChangedSignalId;
    gulong m_nRowActivatedSignalId;

    static void signalChanged(GtkTreeView*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_changed();
    }

    static void signalRowActivated(GtkTreeView*, gpointer widget)
    {
        GtkInstanceTreeView* pThis = static_cast<GtkInstanceTreeView*>(widget);
        pThis->signal_row_activated();
    }
public:
    GtkInstanceTreeView(GtkTreeView* pTreeView, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pTreeView), bTakeOwnership)
        , m_pTreeView(pTreeView)
        , m_pListStore(GTK_LIST_STORE(gtk_tree_view_get_model(m_pTreeView)))
        , m_nChangedSignalId(g_signal_connect(gtk_tree_view_get_selection(pTreeView), "changed",
                             G_CALLBACK(signalChanged), this))
        , m_nRowActivatedSignalId(g_signal_connect(pTreeView, "row-activated", G_CALLBACK(signalChanged), this))
    {
    }

    virtual void append(const OUString& rText) override
    {
        insert(rText, -1);
    }

    virtual void insert(const OUString& rText, int pos) override
    {
        GtkTreeIter iter;
        gtk_list_store_insert(m_pListStore, &iter, pos);
        gtk_list_store_set(m_pListStore, &iter, 0, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr(), -1);
    }

    virtual void remove(int pos) override
    {
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);
        gtk_list_store_remove(m_pListStore, &iter);
    }

    virtual int find(const OUString& rText) const override
    {
        Search aSearch(rText);
        gtk_tree_model_foreach(GTK_TREE_MODEL(m_pListStore), (GtkTreeModelForeachFunc)foreach_find, &aSearch);
        return aSearch.index;
    }

    virtual void move_before(int pos, int before) override
    {
        if (pos == before)
            return;

        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);

        GtkTreeIter position;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &position, nullptr, before);

        gtk_list_store_move_before(m_pListStore, &iter, &position);
    }

    virtual void clear() override
    {
        gtk_list_store_clear(m_pListStore);
    }

    virtual int n_children() const override
    {
        return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_pListStore), nullptr);
    }

    virtual void select(int pos) override
    {
        if (pos != -1)
        {
            GtkTreePath* path = gtk_tree_path_new_from_indices(pos, -1);
            gtk_tree_selection_select_path(gtk_tree_view_get_selection(m_pTreeView), path);
            gtk_tree_path_free(path);
        }
        else
        {
            gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(m_pTreeView));
        }
    }

    virtual OUString get_selected() override
    {
        OUString sRet;
        GtkTreeIter iter;
        GtkTreeModel* pModel;
        if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(m_pTreeView), &pModel, &iter))
        {
            gchar *pStr = nullptr;
            gtk_tree_model_get(pModel, &iter, 0, &pStr, -1);
            sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

    virtual OUString get(int pos) override
    {
        GtkTreeIter iter;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(m_pListStore), &iter, nullptr, pos);
        gchar *pStr = nullptr;
        gtk_tree_model_get(GTK_TREE_MODEL(m_pListStore), &iter, 0, &pStr, -1);
        OUString sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);
        return sRet;
    }

    virtual int get_selected_index() override
    {
        int nRet = -1;
        GtkTreeIter iter;
        GtkTreeModel* pModel;
        if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(m_pTreeView), &pModel, &iter))
        {
            GtkTreePath* path = gtk_tree_model_get_path(pModel, &iter);
            nRet = gtk_tree_path_get_indices(path)[0];
            gtk_tree_path_free(path);
        }
        return nRet;
    }

    virtual void freeze() override
    {
        g_object_ref(m_pListStore);
        gtk_tree_view_set_model(m_pTreeView, nullptr);
    }

    virtual void thaw() override
    {
        gtk_tree_view_set_model(m_pTreeView, GTK_TREE_MODEL(m_pListStore));
        g_object_unref(m_pListStore);
    }

    int get_height_rows(int nRows) const override
    {
        GtkTreeViewColumn* pColumn = gtk_tree_view_get_column(m_pTreeView, 0);
        GList *pRenderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(pColumn));
        GtkCellRenderer* pRenderer = GTK_CELL_RENDERER(g_list_nth_data(pRenderers, 0));
        gint nRowHeight;
        gtk_cell_renderer_get_preferred_height(pRenderer, GTK_WIDGET(m_pTreeView), nullptr, &nRowHeight);
        g_list_free(pRenderers);

        gint nVerticalSeparator;
        gtk_widget_style_get(GTK_WIDGET(m_pTreeView), "vertical-separator", &nVerticalSeparator, nullptr);

        return (nRowHeight * nRows) + (nVerticalSeparator * (nRows + 1));
    }

    virtual ~GtkInstanceTreeView() override
    {
        g_signal_handler_disconnect(gtk_tree_view_get_selection(m_pTreeView), m_nChangedSignalId);
        g_signal_handler_disconnect(m_pTreeView, m_nRowActivatedSignalId);
    }
};

#endif

class SalInstanceSpinButton : public SalInstanceEntry, public virtual Weld::SpinButton
{
private:
    NumericField* m_pButton;

    DECL_LINK(UpDownHdl, SpinField&, void)
    {
        signal_value_changed();
    }

    DECL_LINK(LoseFocusHdl, Control&, void)
    {
        signal_value_changed();
    }

    DECL_LINK(OutputHdl, Edit&, bool)
    {
        return signal_output();
    }

public:
    SalInstanceSpinButton(NumericField* pButton, bool bTakeOwnership)
        : SalInstanceEntry(pButton, bTakeOwnership)
        , m_pButton(pButton)
    {
        m_pButton->SetUpHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_pButton->SetDownHdl(LINK(this, SalInstanceSpinButton, UpDownHdl));
        m_pButton->SetLoseFocusHdl(LINK(this, SalInstanceSpinButton, LoseFocusHdl));
        m_pButton->SetOutputHdl(LINK(this, SalInstanceSpinButton, OutputHdl));
    }

    virtual int get_value() const override
    {
        return m_pButton->GetValue();
    }

    virtual void set_value(int value) override
    {
        m_pButton->SetValue(value);
    }

    virtual void set_range(int min, int max) override
    {
        m_pButton->SetMin(min);
        m_pButton->SetMax(max);
    }

    virtual void get_range(int& min, int& max) const override
    {
        min = m_pButton->GetMin();
        max = m_pButton->GetMax();
    }

    virtual void set_increments(int step, int /*page*/) override
    {
        m_pButton->SetSpinSize(step);
    }

    virtual void get_increments(int& step, int& page) const override
    {
        step = m_pButton->GetSpinSize();
        page = m_pButton->GetSpinSize();
    }

    virtual void set_digits(unsigned int digits) override
    {
        m_pButton->SetDecimalDigits(digits);
    }

    virtual unsigned int get_digits() const override
    {
        return m_pButton->GetDecimalDigits();
    }

    virtual ~SalInstanceSpinButton() override
    {
        m_pButton->SetOutputHdl(Link<Edit&, bool>());
        m_pButton->SetLoseFocusHdl(Link<Control&, void>());
        m_pButton->SetDownHdl(Link<SpinField&, void>());
        m_pButton->SetUpHdl(Link<SpinField&, void>());
    }
};

class SalInstanceLabel : public SalInstanceWidget, public virtual Weld::Label
{
private:
    FixedText* m_pLabel;
public:
    SalInstanceLabel(FixedText* pLabel, bool bTakeOwnership)
        : SalInstanceWidget(pLabel, bTakeOwnership)
        , m_pLabel(pLabel)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        m_pLabel->SetText(rText);
    }
};

class SalInstanceTextView : public SalInstanceContainer, public virtual Weld::TextView
{
private:
    VclMultiLineEdit* m_pTextView;
public:
    SalInstanceTextView(VclMultiLineEdit* pTextView, bool bTakeOwnership)
        : SalInstanceContainer(pTextView, bTakeOwnership)
        , m_pTextView(pTextView)
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        m_pTextView->SetText(rText);
    }

    virtual OUString get_text() const override
    {
        return m_pTextView->GetText();
    }

    virtual Selection get_selection() const override
    {
        return m_pTextView->GetSelection();
    }

    virtual void set_selection(const Selection& rSelection) override
    {
        m_pTextView->SetSelection(rSelection);
    }
};

#if 0
class GtkInstanceDrawingArea : public GtkInstanceWidget, public virtual Weld::DrawingArea
{
private:
    GtkDrawingArea* m_pDrawingArea;
    ScopedVclPtrInstance<VirtualDevice> m_xDevice;
    std::vector<unsigned char> m_aBuffer;
    cairo_surface_t* m_pSurface;
    gulong m_nDrawSignalId;
    gulong m_nSizeAllocateSignalId;
    static gboolean signalDraw(GtkWidget*, cairo_t* cr, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        pThis->signal_draw(cr);
        return false;
    }
    static void signalSizeAllocate(GtkWidget*, GdkRectangle* allocation, gpointer widget)
    {
        GtkInstanceDrawingArea* pThis = static_cast<GtkInstanceDrawingArea*>(widget);
        pThis->signal_size_allocate(allocation->width, allocation->height);
    }
    void signal_size_allocate(guint nWidth, guint nHeight)
    {
        if (m_pSurface)
            cairo_surface_destroy(m_pSurface);

        const int nScale = gtk_widget_get_scale_factor(GTK_WIDGET(m_pDrawingArea));
        const int nStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, nWidth * nScale);
        m_aBuffer.resize(nHeight * nScale * nStride);
        m_xDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nWidth, nHeight), Fraction(1.0), Point(),
                                                          m_aBuffer.data());
        m_pSurface = cairo_image_surface_create_for_data(m_aBuffer.data(), CAIRO_FORMAT_ARGB32,
                                                         nWidth * nScale, nHeight * nScale, nStride);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 14, 0)
        cairo_surface_set_device_scale(m_pSurface, nScale, nScale);
#endif
        m_aSizeAllocateHdl.Call(Size(nWidth, nHeight));
    }
    void signal_draw(cairo_t* cr)
    {
        m_aDrawHdl.Call(*m_xDevice);
        cairo_surface_mark_dirty(m_pSurface);

        cairo_set_source_surface(cr, m_pSurface, 0, 0);
        cairo_paint(cr);
    }
public:
    GtkInstanceDrawingArea(GtkDrawingArea* pDrawingArea, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pDrawingArea), bTakeOwnership)
        , m_pDrawingArea(pDrawingArea)
        , m_xDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT)
        , m_pSurface(nullptr)
        , m_nDrawSignalId(g_signal_connect(pDrawingArea, "draw", G_CALLBACK(signalDraw), this))
        , m_nSizeAllocateSignalId(g_signal_connect(pDrawingArea, "size_allocate", G_CALLBACK(signalSizeAllocate), this))
    {
    }

    virtual void queue_draw() override
    {
        gtk_widget_queue_draw(GTK_WIDGET(m_pDrawingArea));
    }

    virtual ~GtkInstanceDrawingArea() override
    {
        if (m_pSurface)
            cairo_surface_destroy(m_pSurface);
        g_signal_handler_disconnect(m_pDrawingArea, m_nSizeAllocateSignalId);
        g_signal_handler_disconnect(m_pDrawingArea, m_nDrawSignalId);
    }
};

namespace
{
    gint sort_func(GtkTreeModel* pModel, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
    {
        comphelper::string::NaturalStringSorter* pSorter = (comphelper::string::NaturalStringSorter*)user_data;
        gchar* pName1;
        gchar* pName2;
        gtk_tree_model_get(pModel, a, 0, &pName1, -1);
        gtk_tree_model_get(pModel, b, 0, &pName2, -1);
        gint ret = pSorter->compare(OUString(pName1, strlen(pName1), RTL_TEXTENCODING_UTF8),
                                    OUString(pName2, strlen(pName2), RTL_TEXTENCODING_UTF8));
        g_free(pName1);
        g_free(pName2);
        return ret;
    }
}

class GtkInstanceComboBoxText : public GtkInstanceContainer, public virtual Weld::ComboBoxText
{
private:
    GtkComboBoxText* m_pComboBoxText;
    std::unique_ptr<comphelper::string::NaturalStringSorter> m_xSorter;
    gulong m_nSignalId;

    static void signalChanged(GtkComboBox*, gpointer widget)
    {
        GtkInstanceComboBoxText* pThis = static_cast<GtkInstanceComboBoxText*>(widget);
        pThis->signal_changed();
    }

    OUString get(int pos, int col) const
    {
        OUString sRet;
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        GtkTreeIter iter;
        if (gtk_tree_model_iter_nth_child(pModel, &iter, nullptr, pos))
        {
            gchar* pStr;
            gtk_tree_model_get(pModel, &iter, col, &pStr, -1);
            sRet = OUString(pStr, strlen(pStr), RTL_TEXTENCODING_UTF8);
            g_free(pStr);
        }
        return sRet;
    }

public:
    GtkInstanceComboBoxText(GtkComboBoxText* pComboBoxText, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pComboBoxText), bTakeOwnership)
        , m_pComboBoxText(pComboBoxText)
        , m_nSignalId(g_signal_connect(pComboBoxText, "changed", G_CALLBACK(signalChanged), this))
    {
    }

    virtual int get_active() const override
    {
        return gtk_combo_box_get_active(GTK_COMBO_BOX(m_pComboBoxText));
    }

    virtual OUString get_active_id() const override
    {
        const gchar* pText = gtk_combo_box_get_active_id(GTK_COMBO_BOX(m_pComboBoxText));
        return OUString(pText, strlen(pText), RTL_TEXTENCODING_UTF8);
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        OString aId(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8));
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(m_pComboBoxText), aId.getStr());
    }

    virtual void set_active(int pos) override
    {
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_pComboBoxText), pos);
    }

    virtual OUString get_active_text() const override
    {
        gchar* pText = gtk_combo_box_text_get_active_text(m_pComboBoxText);
        OUString sRet(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pText);
        return sRet;
    }

    virtual OUString get_text(int pos) const override
    {
        return get(pos, 0);
    }

    virtual OUString get_id(int pos) const override
    {
        gint id_column = gtk_combo_box_get_id_column(GTK_COMBO_BOX(m_pComboBoxText));
        return get(pos, id_column);
    }

    virtual void append_text(const OUString& rStr) override
    {
        gtk_combo_box_text_append_text(m_pComboBoxText, OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void insert_text(int pos, const OUString& rStr) override
    {
        gtk_combo_box_text_insert_text(m_pComboBoxText, pos, OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void append(const OUString& rId, const OUString& rStr) override
    {
        gtk_combo_box_text_append(m_pComboBoxText,
                                  OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr(),
                                  OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual void insert(int pos, const OUString& rId, const OUString& rStr) override
    {
        gtk_combo_box_text_insert(m_pComboBoxText, pos,
                                  OUStringToOString(rId, RTL_TEXTENCODING_UTF8).getStr(),
                                  OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual int get_count() const override
    {
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        return gtk_tree_model_iter_n_children(pModel, nullptr);
    }

    virtual int find_text(const OUString& rStr) const override
    {
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        GtkTreeIter iter;
        if (!gtk_tree_model_get_iter_first(pModel, &iter))
            return -1;

        OString aStr(OUStringToOString(rStr, RTL_TEXTENCODING_UTF8).getStr());
        int nRet = 0;
        do
        {
            gchar* pStr;
            gtk_tree_model_get(pModel, &iter, 0, &pStr, -1);
            const bool bEqual = strcmp(pStr, aStr.getStr()) == 0;
            g_free(pStr);
            if (bEqual)
                return nRet;
            ++nRet;
        } while (gtk_tree_model_iter_next(pModel, &iter));

        return -1;
    }

    virtual void clear() override
    {
        GtkTreeModel *pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        gtk_list_store_clear(GTK_LIST_STORE(pModel));
    }

    virtual void make_sorted() override
    {
        m_xSorter.reset(new comphelper::string::NaturalStringSorter(
                            ::comphelper::getProcessComponentContext(),
                            Application::GetSettings().GetLanguageTag().getLocale()));
        GtkTreeModel* pModel = gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBoxText));
        GtkTreeSortable* pSortable = GTK_TREE_SORTABLE(pModel);
        gtk_tree_sortable_set_sort_func(pSortable, 0, sort_func, m_xSorter.get(), nullptr);
        gtk_tree_sortable_set_sort_column_id(pSortable, 0, GTK_SORT_ASCENDING);
    }

    virtual ~GtkInstanceComboBoxText() override
    {
        g_signal_handler_disconnect(m_pComboBoxText, m_nSignalId);
    }
};

namespace
{
    void fixup_icons(GObject *pObject)
    {
        //wanted: better way to do this, e.g. make gtk use gio for
        //loading from a filename and provide gio protocol handler
        //for our image in a zip urls
        //
        //unpack the images and keep them as dirs and just
        //add the paths to the gtk icon theme dir
        if (GTK_IS_IMAGE(pObject))
        {
            GtkImage* pImage = GTK_IMAGE(pObject);
            const gchar* icon_name;
            gtk_image_get_icon_name(pImage, &icon_name, nullptr);
            GtkIconSize size;
            g_object_get(pImage, "icon-size", &size, nullptr);
            if (icon_name)
            {
                OUString aIconName(icon_name, strlen(icon_name), RTL_TEXTENCODING_UTF8);

                SvMemoryStream aMemStm;
                BitmapEx aBitmap(aIconName);
                vcl::PNGWriter aWriter(aBitmap);
                aWriter.Write(aMemStm);

                GdkPixbufLoader *pixbuf_loader = gdk_pixbuf_loader_new();
                gdk_pixbuf_loader_write(pixbuf_loader, (const guchar*)aMemStm.GetData(),
                                        aMemStm.Seek(STREAM_SEEK_TO_END), nullptr);
                gdk_pixbuf_loader_close(pixbuf_loader, nullptr);
                GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);

                gtk_image_set_from_pixbuf(pImage, pixbuf);
                g_object_unref(pixbuf_loader);
            }
        }
    }
}

#endif

class SalInstanceBuilder : public Weld::Builder
{
private:
    VclBuilder m_aBuilder;
public:
    SalInstanceBuilder(const OUString& rUIRoot, const OUString& rUIFile)
        : Weld::Builder(rUIFile)
        , m_aBuilder(nullptr, rUIRoot, rUIFile)
    {
    }

    virtual Weld::Dialog* weld_dialog(const OString &id) override
    {
        Dialog* pDialog = m_aBuilder.get<Dialog>(id);
        return pDialog ? new SalInstanceDialog(pDialog, true) : nullptr;
    }

    virtual Weld::Window* weld_window(const OString &id) override
    {
        SystemWindow* pWindow = m_aBuilder.get<SystemWindow>(id);
        return pWindow ? new SalInstanceWindow(pWindow, false) : nullptr;
    }

    virtual Weld::Widget* weld_widget(const OString &id) override
    {
        vcl::Window* pWidget = m_aBuilder.get<SystemWindow>(id);
        return pWidget ? new SalInstanceWidget(pWidget, false) : nullptr;
    }

    virtual Weld::Container* weld_container(const OString &id) override
    {
        vcl::Window* pContainer = m_aBuilder.get<vcl::Window>(id);
        return pContainer ? new SalInstanceContainer(pContainer, false) : nullptr;
    }

    virtual Weld::Frame* weld_frame(const OString &id) override
    {
        VclFrame* pFrame = m_aBuilder.get<VclFrame>(id);
        return pFrame ? new SalInstanceFrame(pFrame, false) : nullptr;
    }

    virtual Weld::Notebook* weld_notebook(const OString &id) override
    {
        TabControl* pNotebook = m_aBuilder.get<TabControl>(id);
        return pNotebook ? new SalInstanceNotebook(pNotebook, false) : nullptr;
    }

    virtual Weld::Button* weld_button(const OString &id) override
    {
        Button* pButton = m_aBuilder.get<Button>(id);
        return pButton ? new SalInstanceButton(pButton, false) : nullptr;
    }

    virtual Weld::RadioButton* weld_radio_button(const OString &id) override
    {
        RadioButton* pRadioButton = m_aBuilder.get<RadioButton>(id);
        return pRadioButton ? new SalInstanceRadioButton(pRadioButton, false) : nullptr;
    }

    virtual Weld::CheckButton* weld_check_button(const OString &id) override
    {
        CheckBox* pCheckButton = m_aBuilder.get<CheckBox>(id);
        return pCheckButton ? new SalInstanceCheckButton(pCheckButton, false) : nullptr;
    }

    virtual Weld::Entry* weld_entry(const OString &id) override
    {
        Edit* pEntry = m_aBuilder.get<Edit>(id);
        return pEntry ? new SalInstanceEntry(pEntry, false) : nullptr;
    }

    virtual Weld::SpinButton* weld_spin_button(const OString &id) override
    {
        NumericField* pSpinButton = m_aBuilder.get<NumericField>(id);
        return pSpinButton ? new SalInstanceSpinButton(pSpinButton, false) : nullptr;
    }

#if 0
    virtual Weld::ComboBoxText* weld_combo_box_text(const OString &id) override
    {
        GtkComboBoxText* pComboBoxText = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pComboBoxText ? new GtkInstanceComboBoxText(pComboBoxText, false) : nullptr;
    }

    virtual Weld::TreeView* weld_tree_view(const OString &id) override
    {
        GtkTreeView* pTreeView = GTK_TREE_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pTreeView ? new GtkInstanceTreeView(pTreeView, false) : nullptr;
    }
#endif
    virtual Weld::Label* weld_label(const OString &id) override
    {
        FixedText* pLabel = m_aBuilder.get<FixedText>(id);
        return pLabel ? new SalInstanceLabel(pLabel, false) : nullptr;
    }

    virtual Weld::TextView* weld_text_view(const OString &id) override
    {
        VclMultiLineEdit* pTextView = m_aBuilder.get<VclMultiLineEdit>(id);
        return pTextView ? new SalInstanceTextView(pTextView, false) : nullptr;
    }
#if 0
    virtual Weld::DrawingArea* weld_drawing_area(const OString &id) override
    {
        GtkDrawingArea* pDrawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pDrawingArea ? new GtkInstanceDrawingArea(pDrawingArea, false) : nullptr;
    }
#endif
};

Weld::Builder* SalInstance::CreateBuilder(const OUString& rUIRoot, const OUString& rUIFile)
{
    return new SalInstanceBuilder(rUIRoot, rUIFile);
}

#if 0

GtkMessageType VclToGtk(VclMessageType eType)
{
    GtkMessageType eRet(GTK_MESSAGE_INFO);
    switch (eType)
    {
        case VclMessageType::Info:
            eRet = GTK_MESSAGE_INFO;
            break;
        case VclMessageType::Warning:
            eRet = GTK_MESSAGE_WARNING;
            break;
        case VclMessageType::Question:
            eRet = GTK_MESSAGE_QUESTION;
            break;
        case VclMessageType::Error:
            eRet = GTK_MESSAGE_ERROR;
            break;
    }
    return eRet;
}

GtkButtonsType VclToGtk(VclButtonsType eType)
{
    GtkButtonsType eRet(GTK_BUTTONS_NONE);
    switch (eType)
    {
        case VclButtonsType::NONE:
            eRet = GTK_BUTTONS_NONE;
            break;
        case VclButtonsType::Ok:
            eRet = GTK_BUTTONS_OK;
            break;
        case VclButtonsType::Close:
            eRet = GTK_BUTTONS_CLOSE;
            break;
        case VclButtonsType::Cancel:
            eRet = GTK_BUTTONS_CANCEL;
            break;
        case VclButtonsType::YesNo:
            eRet = GTK_BUTTONS_YES_NO;
            break;
        case VclButtonsType::OkCancel:
            eRet = GTK_BUTTONS_OK_CANCEL;
            break;
    }
    return eRet;
}

Weld::Dialog* GtkInstance::CreateMessageDialog(Weld::Window *pParent, VclMessageType eMessageType, VclButtonsType eButtonsType, const OUString &rPrimaryMessage)
{
    GtkInstanceWindow* pParentInstance = dynamic_cast<GtkInstanceWindow*>(pParent);
    GtkWindow* pParentWidget = pParentInstance ? pParentInstance->getWindow() : nullptr;
    GtkDialog* pDialog = GTK_DIALOG(gtk_message_dialog_new(pParentWidget, GTK_DIALOG_MODAL, VclToGtk(eMessageType), VclToGtk(eButtonsType),
                                                           "%s", OUStringToOString(rPrimaryMessage, RTL_TEXTENCODING_UTF8).getStr()));
    return new GtkInstanceDialog(pDialog, true);
}

Weld::Window* GtkSalFrame::GetFrameWeld() const
{
    if (!m_xFrameWeld)
        m_xFrameWeld.reset(new GtkInstanceWindow(GTK_WINDOW(getWindow()), false));
    return m_xFrameWeld.get();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
