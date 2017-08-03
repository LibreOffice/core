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

#if 0

class GtkInstanceNotebook : public GtkInstanceContainer, public virtual Weld::Notebook
{
private:
    GtkNotebook* m_pNotebook;
    gulong m_nSignalId;
    mutable std::vector<std::unique_ptr<GtkInstanceContainer>> m_aPages;

    static void signalSwitchPage(GtkNotebook*, GtkWidget*, guint nNewPage, gpointer widget)
    {
        GtkInstanceNotebook* pThis = static_cast<GtkInstanceNotebook*>(widget);
        pThis->signal_switch_page(nNewPage);
    }

    void signal_switch_page(guint nNewPage)
    {
        OString sNewIdent(get_page_ident(nNewPage));
        bool bAllow = m_aSwitchPageHdl.Call(sNewIdent);
        if (!bAllow)
            g_signal_stop_emission_by_name(m_pNotebook, "switch-page");
    }

    OString get_page_ident(guint nPage) const
    {
        const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pNotebook, gtk_notebook_get_nth_page(m_pNotebook, nPage));
        const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
        return OString(pStr, pStr ? strlen(pStr) : 0);
    }

    gint get_page_number(const OString& rIdent) const
    {
        gint nPages = gtk_notebook_get_n_pages(m_pNotebook);
        for (gint i = 0; i < nPages; ++i)
        {
            const GtkWidget* pTabWidget = gtk_notebook_get_tab_label(m_pNotebook, gtk_notebook_get_nth_page(m_pNotebook, i));
            const gchar* pStr = gtk_buildable_get_name(GTK_BUILDABLE(pTabWidget));
            if (strcmp(pStr, rIdent.getStr()) == 0)
                return i;
        }
        return -1;
    }

public:
    GtkInstanceNotebook(GtkNotebook* pNotebook, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pNotebook), bTakeOwnership)
        , m_pNotebook(pNotebook)
        , m_nSignalId(g_signal_connect(pNotebook, "switch-page", G_CALLBACK(signalSwitchPage), this))
    {
    }

    virtual int get_current_page() const override
    {
        return gtk_notebook_get_current_page(m_pNotebook);
    }

    virtual OString get_current_page_ident() const override
    {
        return get_page_ident(get_current_page());
    }

    virtual Weld::Container* get_page(const OString& rIdent) const override
    {
        int nPage = get_page_number(rIdent);
        if (nPage < 0)
            return nullptr;
        GtkContainer* pChild = GTK_CONTAINER(gtk_notebook_get_nth_page(m_pNotebook, nPage));
        unsigned int nPageIndex = (unsigned int)nPage;
        if (m_aPages.size() < nPageIndex + 1)
            m_aPages.resize(nPageIndex + 1);
        if (!m_aPages[nPageIndex])
            m_aPages[nPageIndex].reset(new GtkInstanceContainer(pChild, false));
        return m_aPages[nPageIndex].get();
    }

    virtual void set_current_page(int nPage) override
    {
        gtk_notebook_set_current_page(m_pNotebook, nPage);
    }

    virtual void set_current_page(const OString& rIdent) override
    {
        gint nPage = get_page_number(rIdent);
        set_current_page(nPage);
    }

    virtual int get_n_pages() const override
    {
        return gtk_notebook_get_n_pages(m_pNotebook);
    }

    virtual ~GtkInstanceNotebook() override
    {
        g_signal_handler_disconnect(m_pNotebook, m_nSignalId);
    }
};

#endif

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

#if 0

class GtkInstanceToggleButton : public GtkInstanceButton, public virtual Weld::ToggleButton
{
private:
    GtkToggleButton* m_pToggleButton;
    gulong m_nSignalId;

    static void signalToggled(GtkToggleButton*, gpointer widget)
    {
        GtkInstanceToggleButton* pThis = static_cast<GtkInstanceToggleButton*>(widget);
        pThis->signal_toggled();
    }
public:
    GtkInstanceToggleButton(GtkToggleButton* pButton, bool bTakeOwnership)
        : GtkInstanceButton(GTK_BUTTON(pButton), bTakeOwnership)
        , m_pToggleButton(pButton)
        , m_nSignalId(g_signal_connect(m_pToggleButton, "toggled", G_CALLBACK(signalToggled), this))
    {
    }

    virtual void set_active(bool active) override
    {
        gtk_toggle_button_set_active(m_pToggleButton, active);
    }

    virtual bool get_active() const override
    {
        return gtk_toggle_button_get_active(m_pToggleButton);
    }

    virtual void set_inconsistent(bool inconsistent) override
    {
        gtk_toggle_button_set_inconsistent(m_pToggleButton, inconsistent);
    }

    virtual bool get_inconsistent() const override
    {
        return gtk_toggle_button_get_inconsistent(m_pToggleButton);
    }

    virtual ~GtkInstanceToggleButton() override
    {
        g_signal_handler_disconnect(m_pToggleButton, m_nSignalId);
    }
};

class GtkInstanceRadioButton : public GtkInstanceToggleButton, public virtual Weld::RadioButton
{
public:
    GtkInstanceRadioButton(GtkRadioButton* pButton, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), bTakeOwnership)
    {
    }
};

class GtkInstanceCheckButton : public GtkInstanceToggleButton, public virtual Weld::CheckButton
{
public:
    GtkInstanceCheckButton(GtkCheckButton* pButton, bool bTakeOwnership)
        : GtkInstanceToggleButton(GTK_TOGGLE_BUTTON(pButton), bTakeOwnership)
    {
    }
};

class GtkInstanceEntry : public GtkInstanceWidget, public virtual Weld::Entry
{
private:
    GtkEntry* m_pEntry;
    gulong m_nChangedSignalId;
    gulong m_nInsertTextSignalId;

    static void signalChanged(GtkEntry*, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
        pThis->signal_changed();
    }

    static void signalInsertText(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength,
                                 gint* position, gpointer widget)
    {
        GtkInstanceEntry* pThis = static_cast<GtkInstanceEntry*>(widget);
        pThis->signal_insert_text(pEntry, pNewText, nNewTextLength, position);
    }

    void signal_insert_text(GtkEntry* pEntry, const gchar* pNewText, gint nNewTextLength, gint* position)
    {
        if (!m_aInsertTextHdl.IsSet())
            return;
        OUString sText(pNewText, nNewTextLength, RTL_TEXTENCODING_UTF8);
        const bool bContinue = m_aInsertTextHdl.Call(sText);
        if (bContinue && !sText.isEmpty())
        {
            OString sFinalText(OUStringToOString(sText, RTL_TEXTENCODING_UTF8));
            g_signal_handlers_block_by_func(pEntry, gpointer(signalInsertText), this);
            gtk_editable_insert_text(GTK_EDITABLE(pEntry), sFinalText.getStr(), sFinalText.getLength(), position);
            g_signal_handlers_unblock_by_func(pEntry, gpointer(signalInsertText), this);
        }
        g_signal_stop_emission_by_name(pEntry, "insert-text");
    }
public:
    GtkInstanceEntry(GtkEntry* pEntry, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pEntry), bTakeOwnership)
        , m_pEntry(pEntry)
        , m_nChangedSignalId(g_signal_connect(pEntry, "changed", G_CALLBACK(signalChanged), this))
        , m_nInsertTextSignalId(g_signal_connect(pEntry, "insert-text", G_CALLBACK(signalInsertText), this))
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        gtk_entry_set_text(m_pEntry, OUStringToOString(rText, RTL_TEXTENCODING_UTF8).getStr());
    }

    virtual OUString get_text() const override
    {
        const gchar* pText = gtk_entry_get_text(m_pEntry);
        OUString sRet(pText, pText ? strlen(pText) : 0, RTL_TEXTENCODING_UTF8);
        return sRet;
    }

    virtual void set_width_chars(int nChars) override
    {
        gtk_entry_set_width_chars(m_pEntry, nChars);
    }

    virtual ~GtkInstanceEntry() override
    {
        g_signal_handler_disconnect(m_pEntry, m_nInsertTextSignalId);
        g_signal_handler_disconnect(m_pEntry, m_nChangedSignalId);
    }
};

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


class GtkInstanceSpinButton : public GtkInstanceEntry, public virtual Weld::SpinButton
{
private:
    GtkSpinButton* m_pButton;
    gulong m_nValueChangedSignalId;
    gulong m_nOutputSignalId;

    static void signalValueChanged(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        pThis->signal_value_changed();
    }

    static gboolean signalOutput(GtkSpinButton*, gpointer widget)
    {
        GtkInstanceSpinButton* pThis = static_cast<GtkInstanceSpinButton*>(widget);
        return pThis->signal_output();
    }

    double toGtk(int nValue) const
    {
        return static_cast<double>(nValue) / Power10(get_digits());
    }

    int fromGtk(double fValue) const
    {
        return FRound(fValue * Power10(get_digits()));
    }

public:
    GtkInstanceSpinButton(GtkSpinButton* pButton, bool bTakeOwnership)
        : GtkInstanceEntry(GTK_ENTRY(pButton), bTakeOwnership)
        , m_pButton(pButton)
        , m_nValueChangedSignalId(g_signal_connect(pButton, "value-changed", G_CALLBACK(signalValueChanged), this))
        , m_nOutputSignalId(g_signal_connect(pButton, "output", G_CALLBACK(signalOutput), this))
    {
    }

    virtual int get_value() const override
    {
        return fromGtk(gtk_spin_button_get_value(m_pButton));
    }

    virtual void set_value(int value) override
    {
        gtk_spin_button_set_value(m_pButton, toGtk(value));
    }

    virtual void set_range(int min, int max) override
    {
        gtk_spin_button_set_range(m_pButton, toGtk(min), toGtk(max));
    }

    virtual void get_range(int& min, int& max) const override
    {
        double gtkmin, gtkmax;
        gtk_spin_button_get_range(m_pButton, &gtkmin, &gtkmax);
        min = fromGtk(gtkmin);
        max = fromGtk(gtkmax);
    }

    virtual void set_increments(int step, int page) override
    {
        gtk_spin_button_set_increments(m_pButton, toGtk(step), toGtk(page));
    }

    virtual void get_increments(int& step, int& page) const override
    {
        double gtkstep, gtkpage;
        gtk_spin_button_get_increments(m_pButton, &gtkstep, &gtkpage);
        step = fromGtk(gtkstep);
        page = fromGtk(gtkpage);
    }

    virtual void set_digits(unsigned int digits) override
    {
        gtk_spin_button_set_digits(m_pButton, digits);
    }

    virtual unsigned int get_digits() const override
    {
        return gtk_spin_button_get_digits(m_pButton);
    }

    virtual ~GtkInstanceSpinButton() override
    {
        g_signal_handler_disconnect(m_pButton, m_nOutputSignalId);
        g_signal_handler_disconnect(m_pButton, m_nValueChangedSignalId);
    }
};

class GtkInstanceLabel : public GtkInstanceWidget, public virtual Weld::Label
{
private:
    GtkLabel* m_pLabel;
public:
    GtkInstanceLabel(GtkLabel* pLabel, bool bTakeOwnership)
        : GtkInstanceWidget(GTK_WIDGET(pLabel), bTakeOwnership)
        , m_pLabel(pLabel)
    {
    }

    virtual void set_label(const OUString& rText) override
    {
        gtk_label_set_label(m_pLabel, MapToGtkAccelerator(rText).getStr());
    }
};

class GtkInstanceTextView : public GtkInstanceContainer, public virtual Weld::TextView
{
private:
    GtkTextView* m_pTextView;
public:
    GtkInstanceTextView(GtkTextView* pTextView, bool bTakeOwnership)
        : GtkInstanceContainer(GTK_CONTAINER(pTextView), bTakeOwnership)
        , m_pTextView(pTextView)
    {
    }

    virtual void set_text(const OUString& rText) override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        OString sText(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
        gtk_text_buffer_set_text(pBuffer, sText.getStr(), sText.getLength());
    }

    virtual OUString get_text() const override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(pBuffer, &start, &end);
        char* pStr = gtk_text_buffer_get_text(pBuffer, &start, &end, true);
        OUString sRet = OUString(pStr, pStr ? strlen(pStr) : 0, RTL_TEXTENCODING_UTF8);
        g_free(pStr);
        return sRet;
    }

    virtual Selection get_selection() const override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_selection_bounds(pBuffer, &start, &end);
        return Selection(gtk_text_iter_get_offset(&start), gtk_text_iter_get_offset(&end));
    }

    virtual void set_selection(const Selection& rSelection) override
    {
        GtkTextBuffer* pBuffer = gtk_text_view_get_buffer(m_pTextView);
        GtkTextIter start, end;
        gtk_text_buffer_get_iter_at_offset(pBuffer, &start, rSelection.Min());
        gtk_text_buffer_get_iter_at_offset(pBuffer, &end, rSelection.Max());
        gtk_text_buffer_select_range(pBuffer, &start, &end);
        GtkTextMark* mark = gtk_text_buffer_create_mark(pBuffer, "scroll", &end, true);
        gtk_text_view_scroll_mark_onscreen(m_pTextView, mark);
    }

};

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

#if 0
    virtual Weld::Notebook* weld_notebook(const OString &id) override
    {
        GtkNotebook* pNotebook = GTK_NOTEBOOK(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pNotebook ? new GtkInstanceNotebook(pNotebook, false) : nullptr;
    }
#endif
    virtual Weld::Button* weld_button(const OString &id) override
    {
        Button* pButton = m_aBuilder.get<Button>(id);
        return pButton ? new SalInstanceButton(pButton, false) : nullptr;
    }
#if 0
    virtual Weld::RadioButton* weld_radio_button(const OString &id) override
    {
        GtkRadioButton* pRadioButton = GTK_RADIO_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pRadioButton ? new GtkInstanceRadioButton(pRadioButton, false) : nullptr;
    }

    virtual Weld::CheckButton* weld_check_button(const OString &id) override
    {
        GtkCheckButton* pCheckButton = GTK_CHECK_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pCheckButton ? new GtkInstanceCheckButton(pCheckButton, false) : nullptr;
    }

    virtual Weld::Entry* weld_entry(const OString &id) override
    {
        GtkEntry* pEntry = GTK_ENTRY(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pEntry ? new GtkInstanceEntry(pEntry, false) : nullptr;
    }

    virtual Weld::SpinButton* weld_spin_button(const OString &id) override
    {
        GtkSpinButton* pSpinButton = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pSpinButton ? new GtkInstanceSpinButton(pSpinButton, false) : nullptr;
    }

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

    virtual Weld::Label* weld_label(const OString &id) override
    {
        GtkLabel* pLabel = GTK_LABEL(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pLabel ? new GtkInstanceLabel(pLabel, false) : nullptr;
    }

    virtual Weld::TextView* weld_text_view(const OString &id) override
    {
        GtkTextView* pTextView = GTK_TEXT_VIEW(gtk_builder_get_object(m_pBuilder, id.getStr()));
        return pTextView ? new GtkInstanceTextView(pTextView, false) : nullptr;
    }

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
