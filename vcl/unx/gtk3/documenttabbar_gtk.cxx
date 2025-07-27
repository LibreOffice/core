/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/documenttabbar.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkframe.hxx>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

/**
 * GTK-specific implementation for DocumentTabBar
 *
 * This implementation uses GtkNotebook as the underlying widget
 * to provide native GTK theming and behavior while maintaining
 * LibreOffice's DocumentTabBar interface.
 */
class GtkDocumentTabBar : public DocumentTabBar
{
private:
    GtkWidget* m_pNotebook;         ///< GTK notebook widget
    GtkWidget* m_pContainer;        ///< Container widget
    std::map<sal_uInt32, GtkWidget*> m_aTabPages;  ///< Map tab IDs to GTK pages

    // GTK signal handlers
    static void OnSwitchPage(GtkNotebook* notebook, GtkWidget* page,
                           guint page_num, gpointer user_data);
    static gboolean OnButtonPress(GtkWidget* widget, GdkEventButton* event,
                                 gpointer user_data);
    static void OnPageAdded(GtkNotebook* notebook, GtkWidget* child,
                          guint page_num, gpointer user_data);
    static void OnPageRemoved(GtkNotebook* notebook, GtkWidget* child,
                            guint page_num, gpointer user_data);

    // Helper methods
    void ImplInitGtkWidget();
    void ImplConfigureNotebook();
    GtkWidget* ImplCreateTabLabel(const OUString& rTitle, bool bModified);
    void ImplUpdateTabLabel(GtkWidget* pLabel, const OUString& rTitle, bool bModified);
    sal_uInt32 ImplGetTabIdFromPage(GtkWidget* pPage) const;
    GtkWidget* ImplGetPageFromTabId(sal_uInt32 nTabId) const;

public:
    explicit GtkDocumentTabBar(vcl::Window* pParent, WinBits nWinBits = 0);
    virtual ~GtkDocumentTabBar() override;

    // Override DocumentTabBar methods for GTK-specific behavior
    virtual sal_uInt32 AddTab(SfxViewFrame* pViewFrame) override;
    virtual void RemoveTab(sal_uInt32 nTabId) override;
    virtual void ActivateTab(sal_uInt32 nTabId) override;
    virtual void SetTabTitle(sal_uInt32 nTabId, const OUString& rTitle) override;
    virtual void SetTabModified(sal_uInt32 nTabId, bool bModified) override;

    // GTK-specific methods
    GtkWidget* GetGtkWidget() const { return m_pNotebook; }
    void SetNativeTheming(bool bNative);
};

GtkDocumentTabBar::GtkDocumentTabBar(vcl::Window* pParent, WinBits nWinBits)
    : DocumentTabBar(pParent, nWinBits)
    , m_pNotebook(nullptr)
    , m_pContainer(nullptr)
{
    ImplInitGtkWidget();
}

GtkDocumentTabBar::~GtkDocumentTabBar()
{
    if (m_pNotebook)
    {
        g_object_unref(m_pNotebook);
    }
    if (m_pContainer)
    {
        g_object_unref(m_pContainer);
    }
}

void GtkDocumentTabBar::ImplInitGtkWidget()
{
    // Create GTK notebook widget
    m_pNotebook = gtk_notebook_new();
    g_object_ref_sink(m_pNotebook);

    // Create container to hold the notebook
    m_pContainer = gtk_fixed_new();
    g_object_ref_sink(m_pContainer);

    // Add notebook to container
    gtk_fixed_put(GTK_FIXED(m_pContainer), m_pNotebook, 0, 0);

    ImplConfigureNotebook();

    // Connect signals
    g_signal_connect(m_pNotebook, "switch-page",
                     G_CALLBACK(OnSwitchPage), this);
    g_signal_connect(m_pNotebook, "button-press-event",
                     G_CALLBACK(OnButtonPress), this);
    g_signal_connect(m_pNotebook, "page-added",
                     G_CALLBACK(OnPageAdded), this);
    g_signal_connect(m_pNotebook, "page-removed",
                     G_CALLBACK(OnPageRemoved), this);

    // Show widgets
    gtk_widget_show_all(m_pContainer);

    // TODO: Integrate with VCL window system
    // This would require platform-specific window embedding code
}

void GtkDocumentTabBar::ImplConfigureNotebook()
{
    if (!m_pNotebook)
        return;

    // Configure notebook properties
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(m_pNotebook), GTK_POS_TOP);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(m_pNotebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(m_pNotebook), FALSE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(m_pNotebook), TRUE);

    // Enable tab reordering and detaching
    gtk_notebook_set_group_name(GTK_NOTEBOOK(m_pNotebook), "libreoffice-documents");

    // Apply LibreOffice styling if available
    GtkStyleContext* pContext = gtk_widget_get_style_context(m_pNotebook);
    gtk_style_context_add_class(pContext, "libreoffice-document-tabs");
}

GtkWidget* GtkDocumentTabBar::ImplCreateTabLabel(const OUString& rTitle, bool bModified)
{
    // Create horizontal box for label content
    GtkWidget* pBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    // Create label for title
    OString aTitle = OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8);
    GtkWidget* pLabel = gtk_label_new(aTitle.getStr());
    gtk_label_set_ellipsize(GTK_LABEL(pLabel), PANGO_ELLIPSIZE_MIDDLE);
    gtk_widget_set_size_request(pLabel, -1, -1);

    // Add modified indicator if needed
    if (bModified)
    {
        gtk_label_set_text(GTK_LABEL(pLabel), ("●" + aTitle.getStr()).getStr());
    }

    // Create close button
    GtkWidget* pCloseButton = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(pCloseButton), GTK_RELIEF_NONE);
    gtk_widget_set_focus_on_click(pCloseButton, FALSE);

    // Add close icon (using standard icon or Unicode symbol)
    GtkWidget* pCloseIcon;
    if (gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), "window-close-symbolic"))
    {
        pCloseIcon = gtk_image_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_MENU);
    }
    else
    {
        pCloseIcon = gtk_label_new("×");
    }

    gtk_container_add(GTK_CONTAINER(pCloseButton), pCloseIcon);

    // Pack widgets into box
    gtk_box_pack_start(GTK_BOX(pBox), pLabel, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pBox), pCloseButton, FALSE, FALSE, 0);

    // Show all widgets
    gtk_widget_show_all(pBox);

    // Store references for later access
    g_object_set_data(G_OBJECT(pBox), "title-label", pLabel);
    g_object_set_data(G_OBJECT(pBox), "close-button", pCloseButton);

    return pBox;
}

void GtkDocumentTabBar::ImplUpdateTabLabel(GtkWidget* pLabel, const OUString& rTitle, bool bModified)
{
    if (!pLabel)
        return;

    GtkWidget* pTitleLabel = GTK_WIDGET(g_object_get_data(G_OBJECT(pLabel), "title-label"));
    if (pTitleLabel)
    {
        OString aTitle = OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8);
        if (bModified)
        {
            gtk_label_set_text(GTK_LABEL(pTitleLabel), ("●" + aTitle.getStr()).getStr());
        }
        else
        {
            gtk_label_set_text(GTK_LABEL(pTitleLabel), aTitle.getStr());
        }
    }
}

sal_uInt32 GtkDocumentTabBar::AddTab(SfxViewFrame* pViewFrame)
{
    if (!m_pNotebook || !pViewFrame)
        return 0;

    // Get the base class tab ID first
    sal_uInt32 nTabId = DocumentTabBar::AddTab(pViewFrame);
    if (nTabId == 0)
        return 0;

    // Create a placeholder page widget
    GtkWidget* pPage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    g_object_ref_sink(pPage);

    // Create tab label
    OUString aTitle;
    bool bModified = false;
    if (SfxObjectShell* pObjShell = pViewFrame->GetObjectShell())
    {
        aTitle = pObjShell->GetTitle();
        bModified = pObjShell->IsModified();
    }

    GtkWidget* pLabel = ImplCreateTabLabel(aTitle, bModified);

    // Add page to notebook
    gint nPageIndex = gtk_notebook_append_page(GTK_NOTEBOOK(m_pNotebook), pPage, pLabel);

    // Make tab reorderable and detachable
    gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(m_pNotebook), pPage, TRUE);
    gtk_notebook_set_tab_detachable(GTK_NOTEBOOK(m_pNotebook), pPage, TRUE);

    // Store mapping
    m_aTabPages[nTabId] = pPage;
    g_object_set_data(G_OBJECT(pPage), "tab-id", GUINT_TO_POINTER(nTabId));

    return nTabId;
}

void GtkDocumentTabBar::RemoveTab(sal_uInt32 nTabId)
{
    GtkWidget* pPage = ImplGetPageFromTabId(nTabId);
    if (pPage && m_pNotebook)
    {
        gint nPageIndex = gtk_notebook_page_num(GTK_NOTEBOOK(m_pNotebook), pPage);
        if (nPageIndex >= 0)
        {
            gtk_notebook_remove_page(GTK_NOTEBOOK(m_pNotebook), nPageIndex);
        }

        m_aTabPages.erase(nTabId);
        g_object_unref(pPage);
    }

    // Call base class implementation
    DocumentTabBar::RemoveTab(nTabId);
}

void GtkDocumentTabBar::ActivateTab(sal_uInt32 nTabId)
{
    GtkWidget* pPage = ImplGetPageFromTabId(nTabId);
    if (pPage && m_pNotebook)
    {
        gint nPageIndex = gtk_notebook_page_num(GTK_NOTEBOOK(m_pNotebook), pPage);
        if (nPageIndex >= 0)
        {
            gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pNotebook), nPageIndex);
        }
    }

    // Call base class implementation
    DocumentTabBar::ActivateTab(nTabId);
}

void GtkDocumentTabBar::SetTabTitle(sal_uInt32 nTabId, const OUString& rTitle)
{
    GtkWidget* pPage = ImplGetPageFromTabId(nTabId);
    if (pPage && m_pNotebook)
    {
        GtkWidget* pLabel = gtk_notebook_get_tab_label(GTK_NOTEBOOK(m_pNotebook), pPage);
        if (pLabel)
        {
            DocumentTabItem* pItem = ImplGetItem(nTabId);
            bool bModified = pItem ? pItem->mbModified : false;
            ImplUpdateTabLabel(pLabel, rTitle, bModified);
        }
    }

    // Call base class implementation
    DocumentTabBar::SetTabTitle(nTabId, rTitle);
}

void GtkDocumentTabBar::SetTabModified(sal_uInt32 nTabId, bool bModified)
{
    GtkWidget* pPage = ImplGetPageFromTabId(nTabId);
    if (pPage && m_pNotebook)
    {
        GtkWidget* pLabel = gtk_notebook_get_tab_label(GTK_NOTEBOOK(m_pNotebook), pPage);
        if (pLabel)
        {
            DocumentTabItem* pItem = ImplGetItem(nTabId);
            OUString aTitle = pItem ? pItem->maTitle : OUString();
            ImplUpdateTabLabel(pLabel, aTitle, bModified);
        }
    }

    // Call base class implementation
    DocumentTabBar::SetTabModified(nTabId, bModified);
}

sal_uInt32 GtkDocumentTabBar::ImplGetTabIdFromPage(GtkWidget* pPage) const
{
    if (!pPage)
        return 0;

    gpointer pData = g_object_get_data(G_OBJECT(pPage), "tab-id");
    return pData ? GPOINTER_TO_UINT(pData) : 0;
}

GtkWidget* GtkDocumentTabBar::ImplGetPageFromTabId(sal_uInt32 nTabId) const
{
    auto it = m_aTabPages.find(nTabId);
    return (it != m_aTabPages.end()) ? it->second : nullptr;
}

void GtkDocumentTabBar::SetNativeTheming(bool bNative)
{
    if (!m_pNotebook)
        return;

    GtkStyleContext* pContext = gtk_widget_get_style_context(m_pNotebook);

    if (bNative)
    {
        // Remove custom styling to use native theme
        gtk_style_context_remove_class(pContext, "libreoffice-custom");
    }
    else
    {
        // Add custom styling class
        gtk_style_context_add_class(pContext, "libreoffice-custom");
    }
}

// GTK signal handlers

void GtkDocumentTabBar::OnSwitchPage(GtkNotebook* /*notebook*/, GtkWidget* page,
                                    guint /*page_num*/, gpointer user_data)
{
    GtkDocumentTabBar* pThis = static_cast<GtkDocumentTabBar*>(user_data);
    if (!pThis)
        return;

    sal_uInt32 nTabId = pThis->ImplGetTabIdFromPage(page);
    if (nTabId != 0)
    {
        pThis->ImplActivateTab(nTabId);
    }
}

gboolean GtkDocumentTabBar::OnButtonPress(GtkWidget* /*widget*/, GdkEventButton* event,
                                         gpointer user_data)
{
    GtkDocumentTabBar* pThis = static_cast<GtkDocumentTabBar*>(user_data);
    if (!pThis || !event)
        return FALSE;

    // Handle middle-click to close tab
    if (event->button == 2) // Middle mouse button
    {
        // TODO: Determine which tab was clicked and close it
        return TRUE;
    }

    // Handle right-click for context menu
    if (event->button == 3) // Right mouse button
    {
        // TODO: Show context menu
        return TRUE;
    }

    return FALSE;
}

void GtkDocumentTabBar::OnPageAdded(GtkNotebook* /*notebook*/, GtkWidget* /*child*/,
                                   guint /*page_num*/, gpointer /*user_data*/)
{
    // TODO: Handle page added event if needed
}

void GtkDocumentTabBar::OnPageRemoved(GtkNotebook* /*notebook*/, GtkWidget* /*child*/,
                                     guint /*page_num*/, gpointer /*user_data*/)
{
    // TODO: Handle page removed event if needed
}

// Factory function for creating GTK-specific DocumentTabBar
namespace vcl::gtk {

/**
 * Create a GTK-specific DocumentTabBar instance
 * This function should be called instead of the regular constructor
 * when running on GTK platforms to get native theming.
 */
VclPtr<DocumentTabBar> CreateGtkDocumentTabBar(vcl::Window* pParent, WinBits nWinBits)
{
    return VclPtr<GtkDocumentTabBar>::Create(pParent, nWinBits);
}

} // namespace vcl::gtk

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */