/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <gtv-application-window.hxx>
#include <gtv-main-toolbar.hxx>
#include <gtv-signal-handlers.hxx>
#include <gtv-helpers.hxx>
#include <gtv-calc-header-bar.hxx>
#include <gtv-lok-dialog.hxx>

#include <map>
#include <memory>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

struct GtvMainToolbarPrivateImpl
{
    GtkWidget* toolbar1;
    GtkWidget* toolbar2;

    GtkWidget* m_pEnableEditing;
    GtkWidget* m_pLeftpara;
    GtkWidget* m_pCenterpara;
    GtkWidget* m_pRightpara;
    GtkWidget* m_pJustifypara;
    GtkWidget* m_pDeleteComment;
    GtkWidget* m_pPartSelector;
    GtkWidget* m_pPartModeSelector;
    GtkWidget* m_pDialogSelector;

    /// Sensitivity (enabled or disabled) for each tool item, ignoring edit state
    std::map<GtkToolItem*, bool> m_aToolItemSensitivities;

    GtvMainToolbarPrivateImpl() :
        toolbar1(nullptr),
        toolbar2(nullptr),
        m_pEnableEditing(nullptr),
        m_pLeftpara(nullptr),
        m_pCenterpara(nullptr),
        m_pRightpara(nullptr),
        m_pJustifypara(nullptr),
        m_pDeleteComment(nullptr),
        m_pPartSelector(nullptr),
        m_pPartModeSelector(nullptr),
        m_pDialogSelector(nullptr)
        { }
};

struct GtvMainToolbarPrivate
{
    GtvMainToolbarPrivateImpl* m_pImpl;

    GtvMainToolbarPrivateImpl* operator->()
    {
        return m_pImpl;
    }
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvMainToolbar, gtv_main_toolbar, GTK_TYPE_BOX);

static GtvMainToolbarPrivate&
getPrivate(GtvMainToolbar* toolbar)
{
    return *static_cast<GtvMainToolbarPrivate*>(gtv_main_toolbar_get_instance_private(toolbar));
}

static void
gtv_main_toolbar_init(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    priv.m_pImpl = new GtvMainToolbarPrivateImpl();

    const std::string uiFilePath = GtvHelpers::getDirPath(__FILE__) + std::string(UI_FILE_NAME);
    GtvGtkWrapper<GtkBuilder> builder(gtk_builder_new_from_file(uiFilePath.c_str()),
                                      [](GtkBuilder* pBuilder) {
                                          g_object_unref(pBuilder);
                                      });

    priv->toolbar1 = GTK_WIDGET(gtk_builder_get_object(builder.get(), "toolbar1"));
    gtk_box_pack_start(GTK_BOX(toolbar), priv->toolbar1, false, false, false);
    priv->toolbar2 = GTK_WIDGET(gtk_builder_get_object(builder.get(), "toolbar2"));
    gtk_box_pack_start(GTK_BOX(toolbar), priv->toolbar2, false, false, false);

    priv->m_pEnableEditing = GTK_WIDGET(gtk_builder_get_object(builder.get(), "btn_editmode"));
    priv->m_pLeftpara = GTK_WIDGET(gtk_builder_get_object(builder.get(), "btn_justifyleft"));
    priv->m_pCenterpara = GTK_WIDGET(gtk_builder_get_object(builder.get(), "btn_justifycenter"));
    priv->m_pRightpara = GTK_WIDGET(gtk_builder_get_object(builder.get(), "btn_justifyright"));
    priv->m_pJustifypara = GTK_WIDGET(gtk_builder_get_object(builder.get(), "btn_justifyfill"));
    priv->m_pDeleteComment = GTK_WIDGET(gtk_builder_get_object(builder.get(), "btn_removeannotation"));
    priv->m_pPartSelector = GTK_WIDGET(gtk_builder_get_object(builder.get(), "combo_partselector"));
    priv->m_pPartModeSelector = GTK_WIDGET(gtk_builder_get_object(builder.get(), "combo_partsmodeselector"));
    priv->m_pDialogSelector = GTK_WIDGET(gtk_builder_get_object(builder.get(), "combo_dialogselector"));

    toolbar->m_pAddressbar = GTK_WIDGET(gtk_builder_get_object(builder.get(), "addressbar_entry"));
    toolbar->m_pFormulabar = GTK_WIDGET(gtk_builder_get_object(builder.get(), "formulabar_entry"));

    // TODO: compile with -rdynamic and get rid of it
    gtk_builder_add_callback_symbol(builder.get(), "btn_clicked", G_CALLBACK(btn_clicked));
    gtk_builder_add_callback_symbol(builder.get(), "doCopy", G_CALLBACK(doCopy));
    gtk_builder_add_callback_symbol(builder.get(), "doPaste", G_CALLBACK(doPaste));
    gtk_builder_add_callback_symbol(builder.get(), "createView", G_CALLBACK(createView));
    gtk_builder_add_callback_symbol(builder.get(), "getRulerState", G_CALLBACK(getRulerState));
    gtk_builder_add_callback_symbol(builder.get(), "unoCommandDebugger", G_CALLBACK(unoCommandDebugger));
    gtk_builder_add_callback_symbol(builder.get(), "toggleEditing", G_CALLBACK(toggleEditing));
    gtk_builder_add_callback_symbol(builder.get(), "changePartMode", G_CALLBACK(changePartMode));
    gtk_builder_add_callback_symbol(builder.get(), "changePart", G_CALLBACK(changePart));
    gtk_builder_add_callback_symbol(builder.get(), "changeZoom", G_CALLBACK(changeZoom));
    gtk_builder_add_callback_symbol(builder.get(), "toggleFindbar", G_CALLBACK(toggleFindbar));
    gtk_builder_add_callback_symbol(builder.get(), "documentRedline", G_CALLBACK(documentRedline));
    gtk_builder_add_callback_symbol(builder.get(), "documentRepair", G_CALLBACK(documentRepair));
    gtk_builder_add_callback_symbol(builder.get(), "signalAddressbar", G_CALLBACK(signalAddressbar));
    gtk_builder_add_callback_symbol(builder.get(), "signalFormulabar", G_CALLBACK(signalFormulabar));
    gtk_builder_add_callback_symbol(builder.get(), "openLokDialog", G_CALLBACK(openLokDialog));

    // find toolbar
    // Note: These buttons are not the part of GtvMainToolbar
    gtk_builder_add_callback_symbol(builder.get(), "signalSearchNext", G_CALLBACK(signalSearchNext));
    gtk_builder_add_callback_symbol(builder.get(), "signalSearchPrev", G_CALLBACK(signalSearchPrev));
    gtk_builder_add_callback_symbol(builder.get(), "signalFindbar", G_CALLBACK(signalFindbar));
    gtk_builder_add_callback_symbol(builder.get(), "toggleFindAll", G_CALLBACK(toggleFindAll));

    gtk_builder_connect_signals(builder.get(), nullptr);

    gtk_widget_show_all(GTK_WIDGET(toolbar));
}

static void
gtv_main_toolbar_finalize(GObject* object)
{
    GtvMainToolbarPrivate& priv = getPrivate(GTV_MAIN_TOOLBAR(object));

    delete priv.m_pImpl;
    priv.m_pImpl = nullptr;

    G_OBJECT_CLASS (gtv_main_toolbar_parent_class)->finalize (object);
}

static void
gtv_main_toolbar_class_init(GtvMainToolbarClass* klass)
{
    G_OBJECT_CLASS(klass)->finalize = gtv_main_toolbar_finalize;
}

static void populatePartSelector(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(toolbar)));
    gtv_application_window_set_part_broadcast(window, false);
    gtk_list_store_clear( GTK_LIST_STORE(
                              gtk_combo_box_get_model(
                                  GTK_COMBO_BOX(priv->m_pPartSelector) )) );

    if (!window->lokdocview)
    {
        return;
    }

    const int nMaxLength = 50;
    char sText[nMaxLength];

    int nParts = lok_doc_view_get_parts(LOK_DOC_VIEW(window->lokdocview));
    for ( int i = 0; i < nParts; i++ )
    {
        char* pName = lok_doc_view_get_part_name(LOK_DOC_VIEW(window->lokdocview), i);
        assert( pName );
        snprintf( sText, nMaxLength, "%i (%s)", i+1, pName );
        free( pName );

        gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(priv->m_pPartSelector), sText );
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv->m_pPartSelector), lok_doc_view_get_part(LOK_DOC_VIEW(window->lokdocview)));

    gtv_application_window_set_part_broadcast(window, true);
}

static void populateDialogSelector(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);

    GtkComboBoxText* pSelector = GTK_COMBO_BOX_TEXT(priv->m_pDialogSelector);
    gtk_combo_box_text_append_text( pSelector, ".uno:SearchDialog" );
    gtk_combo_box_text_append_text( pSelector, ".uno:AcceptTrackedChanges" );
    gtk_combo_box_text_append_text( pSelector, ".uno:SpellingAndGrammarDialog" );
    gtk_combo_box_text_append_text( pSelector, ".uno:InsertField" );
    gtk_combo_box_text_append_text( pSelector, ".uno:ImageMapDialog" );
    gtk_combo_box_text_append_text( pSelector, ".uno:WordCountDialog" );
    gtk_combo_box_text_append_text( pSelector, ".uno:HyperlinkDialog" );
    gtk_combo_box_text_append_text( pSelector, ".uno:InsertIndexesEntry" );
    gtk_combo_box_text_append_text( pSelector, ".uno:InsertAuthoritiesEntry");
}

void
gtv_main_toolbar_doc_loaded(GtvMainToolbar* toolbar, LibreOfficeKitDocumentType eDocType, bool bEditMode)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    gtk_widget_set_visible(toolbar->m_pAddressbar, false);
    gtk_widget_set_visible(toolbar->m_pFormulabar, false);
    if (eDocType == LOK_DOCTYPE_SPREADSHEET)
    {
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(priv->m_pLeftpara), ".uno:AlignLeft");
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(priv->m_pCenterpara), ".uno:AlignHorizontalCenter");
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(priv->m_pRightpara), ".uno:AlignRight");
        gtk_widget_hide(priv->m_pJustifypara);
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(priv->m_pDeleteComment), ".uno:DeleteNote");

        gtk_widget_set_visible(toolbar->m_pAddressbar, true);
        gtk_widget_set_visible(toolbar->m_pFormulabar, true);
    }
    else if (eDocType == LOK_DOCTYPE_PRESENTATION)
    {
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(priv->m_pDeleteComment), ".uno:DeleteAnnotation");
    }

    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(priv->m_pEnableEditing), bEditMode);

    // populate combo boxes
    populatePartSelector(toolbar);
    // populate dialogs
    populateDialogSelector(toolbar);
}

GtkContainer*
gtv_main_toolbar_get_first_toolbar(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    return GTK_CONTAINER(priv->toolbar1);
}

GtkContainer*
gtv_main_toolbar_get_second_toolbar(GtvMainToolbar* toolbar)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    return GTK_CONTAINER(priv->toolbar2);
}

void
gtv_main_toolbar_set_sensitive_internal(GtvMainToolbar* toolbar, GtkToolItem* pItem, bool isSensitive)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    priv->m_aToolItemSensitivities[pItem] = isSensitive;
}

static void setSensitiveIfEdit(GtvMainToolbar* toolbar, GtkToolItem* pItem, bool bEdit)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    // some buttons remain enabled always
    const gchar* pIconName = gtk_tool_button_get_icon_name(GTK_TOOL_BUTTON(pItem));
    if (g_strcmp0(pIconName, "zoom-in-symbolic") != 0 &&
        g_strcmp0(pIconName, "zoom-original-symbolic") != 0 &&
        g_strcmp0(pIconName, "zoom-out-symbolic") != 0 &&
        g_strcmp0(pIconName, "insert-text-symbolic") != 0 &&
        g_strcmp0(pIconName, "view-continuous-symbolic") != 0 &&
        g_strcmp0(pIconName, "document-properties") != 0 &&
        g_strcmp0(pIconName, "system-run") != 0)
    {
        bool state = true;
        if (priv->m_aToolItemSensitivities.find(pItem) != priv->m_aToolItemSensitivities.end())
            state = priv->m_aToolItemSensitivities[pItem];

        gtk_widget_set_sensitive(GTK_WIDGET(pItem), bEdit && state);
    }
}

void
gtv_main_toolbar_set_edit(GtvMainToolbar* toolbar, gboolean bEdit)
{
    GtvMainToolbarPrivate& priv = getPrivate(toolbar);
    GtvGtkWrapper<GList> pList(gtk_container_get_children(GTK_CONTAINER(priv->toolbar1)),
                            [](GList* l)
                            {
                                g_list_free(l);
                            });
    for (GList* l = pList.get(); l != nullptr; l = l->next)
    {
        if (GTK_IS_TOOL_BUTTON(l->data))
        {
            setSensitiveIfEdit(toolbar, GTK_TOOL_ITEM(l->data), bEdit);
        }
    }

    pList.reset(gtk_container_get_children(GTK_CONTAINER(priv->toolbar2)));
    for (GList* l = pList.get(); l != nullptr; l = l->next)
    {
        if (GTK_IS_TOOL_BUTTON(l->data))
        {
            setSensitiveIfEdit(toolbar, GTK_TOOL_ITEM(l->data), bEdit);
        }
    }
}

GtkWidget*
gtv_main_toolbar_new()
{
    return GTK_WIDGET(g_object_new(GTV_TYPE_MAIN_TOOLBAR,
                                   "orientation", GTK_ORIENTATION_VERTICAL,
                                   nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
