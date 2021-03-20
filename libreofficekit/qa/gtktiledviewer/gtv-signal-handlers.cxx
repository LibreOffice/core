/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include "gtv-application-window.hxx"
#include "gtv-helpers.hxx"
#include "gtv-lokdocview-signal-handlers.hxx"
#include "gtv-signal-handlers.hxx"

#include <sal/macros.h>

#include <map>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <optional>

void btn_clicked(GtkWidget* pButton, gpointer)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    GtkToolButton* pItem = GTK_TOOL_BUTTON(pButton);
    const gchar* label = gtk_tool_button_get_label(pItem);
    if (!(gtv_application_window_get_toolbar_broadcast(window) && g_str_has_prefix(label, ".uno:")))
        return;

    std::string aArguments;
    if (g_strcmp0(label, ".uno:InsertAnnotation") == 0)
    {
        std::map<std::string, std::string> aEntries;
        aEntries["Text"] = "";
        GtvHelpers::userPromptDialog(GTK_WINDOW(window), "Insert Comment", aEntries);

        boost::property_tree::ptree aTree;
        aTree.put(boost::property_tree::ptree::path_type("Text/type", '/'), "string");
        aTree.put(boost::property_tree::ptree::path_type("Text/value", '/'), aEntries["Text"]);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        aArguments = aStream.str();
    }

    bool bNotify = g_strcmp0(label, ".uno:Save") == 0;
    if (window->lokdocview)
        lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), label, aArguments.c_str(), bNotify);
}

void doCopy(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    char* pUsedFormat = nullptr;
    // TODO: Should check `text-selection` signal before trying to copy
    char* pSelection = lok_doc_view_copy_selection(LOK_DOC_VIEW(window->lokdocview), "text/html", &pUsedFormat);
    if (!pSelection)
        return;

    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(pButton), GDK_SELECTION_CLIPBOARD);
    std::string aUsedFormat(pUsedFormat);
    if (aUsedFormat == "text/plain;charset=utf-8")
        gtk_clipboard_set_text(pClipboard, pSelection, -1);
    else
        GtvHelpers::clipboardSetHtml(pClipboard, pSelection);

    free(pSelection);
    free(pUsedFormat);
}

void doPaste(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(pButton), GDK_SELECTION_CLIPBOARD);
    GdkAtom* pTargets;
    gint nTargets;
    std::map<std::string, GdkAtom> aTargets;
    if (gtk_clipboard_wait_for_targets(pClipboard, &pTargets, &nTargets))
    {
        for (gint i = 0; i < nTargets; ++i)
        {
            gchar* pName = gdk_atom_name(pTargets[i]);
            aTargets[pName] = pTargets[i];
            g_free(pName);
        }
        g_free(pTargets);
    }

    std::optional<GdkAtom> oTarget;
    std::string aTargetName;

    std::vector<std::string> aPreferredNames =
    {
        std::string("image/png"),
        std::string("text/html")
    };
    for (const std::string& rName : aPreferredNames)
    {
        std::map<std::string, GdkAtom>::iterator it = aTargets.find(rName);
        if (it != aTargets.end())
        {
            aTargetName = it->first;
            oTarget = it->second;
            break;
        }
    }

    if (oTarget)
    {
        GtkSelectionData* pSelectionData = gtk_clipboard_wait_for_contents(pClipboard, *oTarget);
        if (!pSelectionData)
        {
            return;
        }
        gint nLength;
        const guchar* pData = gtk_selection_data_get_data_with_length(pSelectionData, &nLength);
        bool bSuccess = lok_doc_view_paste(LOK_DOC_VIEW(window->lokdocview), aTargetName.c_str(), reinterpret_cast<const char*>(pData), nLength);
        gtk_selection_data_free(pSelectionData);
        if (bSuccess)
            return;
    }

    gchar* pText = gtk_clipboard_wait_for_text(pClipboard);
    if (pText)
        lok_doc_view_paste(LOK_DOC_VIEW(window->lokdocview), "text/plain;charset=utf-8", pText, strlen(pText));
}

void createView(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    gtv_application_window_create_view_from_window(GTV_APPLICATION_WINDOW(window));
}

void getRulerState(GtkWidget* pButton, gpointer /*pItem*/)
{
    const std::string type = ".uno:RulerState";
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    pDocument->pClass->getCommandValues(pDocument, type.c_str());
}

static void removeUnoParam(GtkWidget* pWidget, gpointer userdata)
{
    GtkWidget* pParamAreaBox = GTK_WIDGET(userdata);
    GtkWidget* pParamContainer = gtk_widget_get_parent(pWidget);

    gtk_container_remove(GTK_CONTAINER(pParamAreaBox), pParamContainer);
}

static void addMoreUnoParam(GtkWidget* /*pWidget*/, gpointer userdata)
{
    GtkWidget* pUnoParamAreaBox = GTK_WIDGET(userdata);

    GtkWidget* pParamContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pUnoParamAreaBox), pParamContainer, true, true, 2);

    GtkWidget* pTypeEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pTypeEntry, true, true, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pTypeEntry), "Param type (Eg. boolean, string etc.)");

    GtkWidget* pNameEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pNameEntry, true, true, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pNameEntry), "Param name");

    GtkWidget* pValueEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pValueEntry, true, true, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pValueEntry), "Param value");

    GtkWidget* pRemoveButton = gtk_button_new_from_icon_name("list-remove-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(pRemoveButton, "clicked", G_CALLBACK(removeUnoParam), pUnoParamAreaBox);
    gtk_box_pack_start(GTK_BOX(pParamContainer), pRemoveButton, true, true, 2);

    gtk_widget_show_all(pUnoParamAreaBox);
}

static void iterateUnoParams(GtkWidget* pWidget, gpointer userdata)
{
    boost::property_tree::ptree *pTree = static_cast<boost::property_tree::ptree*>(userdata);
    GtvGtkWrapper<GList> pChildren(gtk_container_get_children(GTK_CONTAINER(pWidget)),
                                   [](GList* pList) {
                                       g_list_free(pList);
                                   });
    GList* pIt = nullptr;
    guint i = 0;
    const gchar* unoParam[3];
    for (pIt = pChildren.get(), i = 0; pIt != nullptr && i < 3; pIt = pIt->next, i++)
    {
        unoParam[i] = gtk_entry_get_text(GTK_ENTRY(pIt->data));
    }

    gchar* pPath = g_strconcat(unoParam[1], "/", "type", nullptr);
    pTree->put(boost::property_tree::ptree::path_type(pPath, '/'), unoParam[0]);
    g_free(pPath);
    pPath = g_strconcat(unoParam[1], "/", "value", nullptr);
    pTree->put(boost::property_tree::ptree::path_type(pPath, '/'), unoParam[2]);
    g_free(pPath);
}

void recentUnoChanged( GtkWidget* pSelector, gpointer /* pItem */ )
{
    GtvApplicationWindow* pWindow = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pSelector));
    gchar* pUnoCmd = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(pSelector));

    GtvMainToolbar* pToolbar = gtv_application_window_get_main_toolbar(pWindow);
    const std::string aUnoArgs = gtv_main_toolbar_get_recent_uno_args(pToolbar, pUnoCmd);
    // this will also discard our default placeholder string, "Recent UNO"
    if (aUnoArgs.empty())
        return;

    lok_doc_view_post_command(LOK_DOC_VIEW(pWindow->lokdocview), pUnoCmd, (aUnoArgs.empty() ? nullptr : aUnoArgs.c_str()), false);
    g_free(pUnoCmd);
}

static void addToRecentUnoCommands(GtvApplicationWindow* pWindow, const std::string& rUnoCmd, std::string rArgs)
{
    GtvMainToolbar* pToolbar = gtv_application_window_get_main_toolbar(pWindow);
    rArgs.erase(std::find(rArgs.begin(), rArgs.end(), '\n'));
    const std::string rUnoCmdStr = rUnoCmd + " | " + rArgs;


    // add to file
    std::ofstream outfile("/tmp/gtv-recentunos.txt", std::ios_base::app | std::ios_base::out);
    if (outfile.good())
        outfile << rUnoCmdStr << '\n';

    // add to combo box
    gtv_main_toolbar_add_recent_uno(pToolbar, rUnoCmdStr);
}

void unoCommandDebugger(GtkWidget* pButton, gpointer /* pItem */)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    GtkWidget* pUnoCmdDialog = gtk_dialog_new_with_buttons ("Execute UNO command",
                                                            GTK_WINDOW (window),
                                                            GTK_DIALOG_MODAL,
                                                            "Execute",
                                                            GTK_RESPONSE_OK,
                                                            nullptr);
    g_object_set(G_OBJECT(pUnoCmdDialog), "resizable", FALSE, nullptr);
    GtkWidget* pDialogMessageArea = gtk_dialog_get_content_area (GTK_DIALOG (pUnoCmdDialog));
    GtkWidget* pUnoCmdAreaBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pUnoCmdAreaBox, true, true, 2);

    GtkWidget* pUnoCmdLabel = gtk_label_new("Enter UNO command");
    gtk_box_pack_start(GTK_BOX(pUnoCmdAreaBox), pUnoCmdLabel, true, true, 2);

    GtkWidget* pUnoCmdEntry = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(pUnoCmdAreaBox), pUnoCmdEntry, true, true, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pUnoCmdEntry), "UNO command (Eg. Bold, Italic etc.)");
    GtkWidget* pUnoParamAreaBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pUnoParamAreaBox, true, true, 2);

    GtkWidget* pAddMoreButton = gtk_button_new_with_label("Add UNO parameter");
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pAddMoreButton, true, true, 2);
    g_signal_connect(G_OBJECT(pAddMoreButton), "clicked", G_CALLBACK(addMoreUnoParam), pUnoParamAreaBox);

    gtk_widget_show_all(pUnoCmdDialog);

    gint res = gtk_dialog_run (GTK_DIALOG(pUnoCmdDialog));
    if (res == GTK_RESPONSE_OK)
    {
        gchar* sUnoCmd = g_strconcat(".uno:", gtk_entry_get_text(GTK_ENTRY(pUnoCmdEntry)), nullptr);

        boost::property_tree::ptree aTree;
        gtk_container_foreach(GTK_CONTAINER(pUnoParamAreaBox), iterateUnoParams, &aTree);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree, false);
        std::string aArguments = aStream.str();

        g_info("Generated UNO command: %s %s", sUnoCmd, aArguments.c_str());

        lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), sUnoCmd, (aArguments.empty() ? nullptr : aArguments.c_str()), false);
        addToRecentUnoCommands(window, sUnoCmd, aArguments);

        g_free(sUnoCmd);
    }

    gtk_widget_destroy(pUnoCmdDialog);
}

void toggleEditing(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    bool bActive = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pButton));
    if (bool(lok_doc_view_get_edit(LOK_DOC_VIEW(window->lokdocview))) != bActive)
        lok_doc_view_set_edit(LOK_DOC_VIEW(window->lokdocview), bActive);
}

void changePart( GtkWidget* pSelector, gpointer /* pItem */ )
{
    int nPart = gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) );
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pSelector));

    if (gtv_application_window_get_part_broadcast(window) && window->lokdocview)
    {
        lok_doc_view_set_part( LOK_DOC_VIEW(window->lokdocview), nPart );
        lok_doc_view_reset_view(LOK_DOC_VIEW(window->lokdocview));
    }
}

void changePartMode( GtkWidget* pSelector, gpointer /* pItem */ )
{
    // Just convert directly back to the LibreOfficeKitPartMode enum.
    // I.e. the ordering above should match the enum member ordering.
    LibreOfficeKitPartMode ePartMode =
        LibreOfficeKitPartMode( gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) ) );
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pSelector));

    if ( window->lokdocview )
    {
        lok_doc_view_set_partmode( LOK_DOC_VIEW(window->lokdocview), ePartMode );
    }
}

void changeZoom( GtkWidget* pButton, gpointer /* pItem */ )
{
    static const float fZooms[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 5.0 };
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    const char *sName = gtk_tool_button_get_icon_name( GTK_TOOL_BUTTON(pButton) );

    float fZoom = 0;
    float fCurrentZoom = 0;

    if ( window->lokdocview )
    {
        fCurrentZoom = lok_doc_view_get_zoom( LOK_DOC_VIEW(window->lokdocview) );
    }

    if ( strcmp(sName, "zoom-in-symbolic") == 0)
    {
        for ( size_t i = 0; i < SAL_N_ELEMENTS( fZooms ); i++ )
        {
            if ( fCurrentZoom < fZooms[i] )
            {
                fZoom = fZooms[i];
                break;
            }
        }
    }
    else if ( strcmp(sName, "zoom-original-symbolic") == 0)
    {
        fZoom = 1;
    }
    else if ( strcmp(sName, "zoom-out-symbolic") == 0)
    {
        for ( size_t i = 0; i < SAL_N_ELEMENTS( fZooms ); i++ )
        {
            if ( fCurrentZoom > fZooms[i] )
            {
                fZoom = fZooms[i];
            }
        }
    }

    if ( fZoom != 0 && window->lokdocview )
    {
        lok_doc_view_set_zoom( LOK_DOC_VIEW(window->lokdocview), fZoom );
        GdkRectangle aVisibleArea;
        gtv_application_window_get_visible_area(window, &aVisibleArea);
        lok_doc_view_set_visible_area(LOK_DOC_VIEW(window->lokdocview), &aVisibleArea);
    }
    const std::string aZoom = std::string("Zoom: ") + std::to_string(int(fZoom * 100)) + std::string("%");
    gtk_label_set_text(GTK_LABEL(window->zoomlabel), aZoom.c_str());
}

void documentRedline(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    // Get the data.
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    char* pValues = pDocument->pClass->getCommandValues(pDocument, ".uno:AcceptTrackedChanges");
    if (!pValues)
        return;

    std::stringstream aInfo;
    aInfo << "lok::Document::getCommandValues('.uno:AcceptTrackedChanges') returned '" << pValues << "'" << std::endl;
    g_info("%s", aInfo.str().c_str());
    std::stringstream aStream(pValues);
    free(pValues);
    assert(!aStream.str().empty());
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);

    // Create the dialog.
    GtkWidget* pDialog = gtk_dialog_new_with_buttons("Manage Changes",
                                                     GTK_WINDOW (window),
                                                     GTK_DIALOG_MODAL,
                                                     "Accept",
                                                     GTK_RESPONSE_YES,
                                                     "Reject",
                                                     GTK_RESPONSE_NO,
                                                     "Jump",
                                                     GTK_RESPONSE_APPLY,
                                                     nullptr);
    gtk_window_set_default_size(GTK_WINDOW(pDialog), 800, 600);
    GtkWidget* pContentArea = gtk_dialog_get_content_area(GTK_DIALOG (pDialog));
    GtkWidget* pScrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);

    // Build the table.
    GtkTreeStore* pTreeStore = gtk_tree_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    for (const auto& rValue : aTree.get_child("redlines"))
    {
        GtkTreeIter aTreeIter;
        gtk_tree_store_append(pTreeStore, &aTreeIter, nullptr);
        gtk_tree_store_set(pTreeStore, &aTreeIter,
                           0, rValue.second.get<int>("index"),
                           1, rValue.second.get<std::string>("author").c_str(),
                           2, rValue.second.get<std::string>("type").c_str(),
                           3, rValue.second.get<std::string>("comment").c_str(),
                           4, rValue.second.get<std::string>("description").c_str(),
                           5, rValue.second.get<std::string>("dateTime").c_str(),
                           -1);
    }
    GtkWidget* pTreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pTreeStore));
    std::vector<std::string> aColumns = {"Index", "Author", "Type", "Comment", "Description", "Timestamp"};
    for (size_t nColumn = 0; nColumn < aColumns.size(); ++nColumn)
    {
        GtkCellRenderer* pRenderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* pColumn = gtk_tree_view_column_new_with_attributes(aColumns[nColumn].c_str(),
                                                                              pRenderer,
                                                                              "text", nColumn,
                                                                              nullptr);
        gtk_tree_view_append_column(GTK_TREE_VIEW(pTreeView), pColumn);
    }
    gtk_container_add(GTK_CONTAINER(pScrolledWindow), pTreeView);
    gtk_box_pack_start(GTK_BOX(pContentArea), pScrolledWindow, true, true, 2);

    // Show the dialog.
    gtk_widget_show_all(pDialog);
    gint res = gtk_dialog_run(GTK_DIALOG(pDialog));

    // Dispatch the matching command, if necessary.
    if (res == GTK_RESPONSE_YES || res == GTK_RESPONSE_NO || res == GTK_RESPONSE_APPLY)
    {
        GtkTreeSelection* pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pTreeView));
        GtkTreeIter aTreeIter;
        GtkTreeModel* pTreeModel;
        if (gtk_tree_selection_get_selected(pSelection, &pTreeModel, &aTreeIter))
        {
            gint nIndex = 0;
            // 0: index
            gtk_tree_model_get(pTreeModel, &aTreeIter, 0, &nIndex, -1);
            std::string aCommand;
            if (res == GTK_RESPONSE_YES)
                aCommand = ".uno:AcceptTrackedChange";
            else if (res == GTK_RESPONSE_NO)
                aCommand = ".uno:RejectTrackedChange";
            else
                // Just select the given redline, don't accept or reject it.
                aCommand = ".uno:NextTrackedChange";
            // Without the '.uno:' prefix.
            std::string aKey = aCommand.substr(strlen(".uno:"));

            // Post the command.
            boost::property_tree::ptree aCommandTree;
            aCommandTree.put(boost::property_tree::ptree::path_type(aKey + "/type", '/'), "unsigned short");
            aCommandTree.put(boost::property_tree::ptree::path_type(aKey + "/value", '/'), nIndex);

            aStream.str(std::string());
            boost::property_tree::write_json(aStream, aCommandTree);
            std::string aArguments = aStream.str();
            lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), aCommand.c_str(), aArguments.c_str(), false);
        }
    }

    gtk_widget_destroy(pDialog);
}

void documentRepair(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    // Get the data.
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    // Show it in linear time, so first redo in reverse order, then undo.
    std::vector<std::string> aTypes = {".uno:Redo", ".uno:Undo"};
    std::vector<boost::property_tree::ptree> aTrees;
    for (size_t nType = 0; nType < aTypes.size(); ++nType)
    {
        const std::string& rType = aTypes[nType];
        char* pValues = pDocument->pClass->getCommandValues(pDocument, rType.c_str());
        std::stringstream aInfo;
        aInfo << "lok::Document::getCommandValues('" << rType << "') returned '" << pValues << "'" << std::endl;
        g_info("%s", aInfo.str().c_str());
        std::stringstream aStream(pValues);
        free(pValues);
        assert(!aStream.str().empty());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        aTrees.push_back(aTree);
    }

    // Create the dialog.
    GtkWidget* pDialog = gtk_dialog_new_with_buttons("Repair document",
                                                     GTK_WINDOW (window),
                                                     GTK_DIALOG_MODAL,
                                                     "Jump to state",
                                                     GTK_RESPONSE_OK,
                                                     nullptr);
    gtk_window_set_default_size(GTK_WINDOW(pDialog), 800, 600);
    GtkWidget* pContentArea = gtk_dialog_get_content_area(GTK_DIALOG (pDialog));
    GtkWidget* pScrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);

    // Build the table.
    GtkTreeStore* pTreeStore = gtk_tree_store_new(5, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    for (size_t nTree = 0; nTree < aTrees.size(); ++nTree)
    {
        const auto& rTree = aTrees[nTree];
        for (const auto& rValue : rTree.get_child("actions"))
        {
            GtkTreeIter aTreeIter;
            gtk_tree_store_append(pTreeStore, &aTreeIter, nullptr);
            gtk_tree_store_set(pTreeStore, &aTreeIter,
                               0, aTypes[nTree].c_str(),
                               1, rValue.second.get<int>("index"),
                               2, rValue.second.get<std::string>("comment").c_str(),
                               3, rValue.second.get<std::string>("viewId").c_str(),
                               4, rValue.second.get<std::string>("dateTime").c_str(),
                               -1);
        }
    }
    GtkWidget* pTreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pTreeStore));
    std::vector<std::string> aColumns = {"Type", "Index", "Comment", "View ID", "Timestamp"};
    for (size_t nColumn = 0; nColumn < aColumns.size(); ++nColumn)
    {
        GtkCellRenderer* pRenderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* pColumn = gtk_tree_view_column_new_with_attributes(aColumns[nColumn].c_str(),
                                                                              pRenderer,
                                                                              "text", nColumn,
                                                                              nullptr);
        gtk_tree_view_append_column(GTK_TREE_VIEW(pTreeView), pColumn);
    }
    gtk_container_add(GTK_CONTAINER(pScrolledWindow), pTreeView);
    gtk_box_pack_start(GTK_BOX(pContentArea), pScrolledWindow, true, true, 2);

    // Show the dialog.
    gtk_widget_show_all(pDialog);
    gint res = gtk_dialog_run(GTK_DIALOG(pDialog));

    // Dispatch the matching command, if necessary.
    if (res == GTK_RESPONSE_OK)
    {
        GtkTreeSelection* pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pTreeView));
        GtkTreeIter aTreeIter;
        GtkTreeModel* pTreeModel;
        if (gtk_tree_selection_get_selected(pSelection, &pTreeModel, &aTreeIter))
        {
            gchar* pType = nullptr;
            gint nIndex = 0;
            // 0: type, 1: index
            gtk_tree_model_get(pTreeModel, &aTreeIter, 0, &pType, 1, &nIndex, -1);
            // '.uno:Undo' or '.uno:Redo'
            const std::string aType(pType);
            // Without the '.uno:' prefix.
            std::string aKey = aType.substr(strlen(".uno:"));
            g_free(pType);

            // Post the command.
            boost::property_tree::ptree aTree;
            aTree.put(boost::property_tree::ptree::path_type(aKey + "/type", '/'), "unsigned short");
            aTree.put(boost::property_tree::ptree::path_type(aKey + "/value", '/'), nIndex + 1);

            // Without this, we could only undo our own commands.
            aTree.put(boost::property_tree::ptree::path_type("Repair/type", '/'), "boolean");
            aTree.put(boost::property_tree::ptree::path_type("Repair/value", '/'), true);

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            std::string aArguments = aStream.str();
            lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), aType.c_str(), aArguments.c_str(), false);
        }
    }

    gtk_widget_destroy(pDialog);
}

void toggleFindbar(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    gtv_application_window_toggle_findbar(window);
}

void docAdjustmentChanged(GtkAdjustment*, gpointer pData)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(pData);
    if (window->lokdocview)
        LOKDocViewSigHandlers::configureEvent(window->lokdocview, nullptr, nullptr);
}

void signalSearchNext(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    GtkEntry* pEntry = GTK_ENTRY(window->findbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);
    bool findAll = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(window->findAll));
    lok_doc_view_find_next(LOK_DOC_VIEW(window->lokdocview), pText, findAll);
}

void signalSearchPrev(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    GtkEntry* pEntry = GTK_ENTRY(window->findbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);
    bool findAll = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(window->findAll));
    lok_doc_view_find_prev(LOK_DOC_VIEW(window->lokdocview), pText, findAll);
}

gboolean signalFindbar(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer /*pData*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pWidget));
    gtk_label_set_text(GTK_LABEL(window->findbarlabel), "");
    switch(pEvent->keyval)
    {
        case GDK_KEY_Return:
        {
            // Search forward.
            signalSearchNext(pWidget, nullptr);
            return true;
        }
        case GDK_KEY_Escape:
        {
            // Hide the findbar.
            gtk_widget_hide(GTK_WIDGET(window->findtoolbar));
            return true;
        }
    }
    return FALSE;
}

void toggleFindAll(GtkWidget* pButton, gpointer /*pItem*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pButton));
    GtkEntry* pEntry = GTK_ENTRY(window->findbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);
    bool findAll = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(window->findAll));
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(window->findAll), !findAll);
    lok_doc_view_highlight_all(LOK_DOC_VIEW(window->lokdocview), pText);
}

void editButtonClicked(GtkWidget* pWidget, gpointer userdata)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pWidget));
    std::map<std::string, std::string> aEntries;
    aEntries["Text"] = "";

    GtvHelpers::userPromptDialog(GTK_WINDOW(window), "Edit comment", aEntries);

    gchar *commentId = static_cast<gchar*>(g_object_get_data(G_OBJECT(userdata), "id"));

    boost::property_tree::ptree aTree;
    aTree.put(boost::property_tree::ptree::path_type("Id/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("Id/value", '/'), std::string(commentId));

    aTree.put(boost::property_tree::ptree::path_type("Text/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("Text/value", '/'), aEntries["Text"]);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    std::string aArguments = aStream.str();

    lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), ".uno:EditAnnotation", aArguments.c_str(), false);
}

void replyButtonClicked(GtkWidget* pWidget, gpointer userdata)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pWidget));
    std::map<std::string, std::string> aEntries;
    aEntries["Text"] = "";

    GtvHelpers::userPromptDialog(GTK_WINDOW(window), "Reply comment", aEntries);

    gchar *commentId = static_cast<gchar*>(g_object_get_data(G_OBJECT(userdata), "id"));

    boost::property_tree::ptree aTree;
    aTree.put(boost::property_tree::ptree::path_type("Id/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("Id/value", '/'), std::string(commentId));

    aTree.put(boost::property_tree::ptree::path_type("Text/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("Text/value", '/'), aEntries["Text"]);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    std::string aArguments = aStream.str();

    // Different reply UNO command for impress
    std::string replyCommand = ".uno:ReplyComment";
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    if (pDocument && pDocument->pClass->getDocumentType(pDocument) == LOK_DOCTYPE_PRESENTATION)
        replyCommand = ".uno:ReplyToAnnotation";
    lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), replyCommand.c_str(), aArguments.c_str(), false);
}

void deleteCommentButtonClicked(GtkWidget* pWidget, gpointer userdata)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pWidget));
    gchar *commentid = static_cast<gchar*>(g_object_get_data(G_OBJECT(userdata), "id"));

    boost::property_tree::ptree aTree;
    aTree.put(boost::property_tree::ptree::path_type("Id/type", '/'), "string");
    aTree.put(boost::property_tree::ptree::path_type("Id/value", '/'), std::string(commentid));

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    std::string aArguments = aStream.str();

    // Different reply UNO command for impress
    std::string deleteCommand = ".uno:DeleteComment";
    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(window->lokdocview));
    if (pDocument)
    {
        if (pDocument->pClass->getDocumentType(pDocument) == LOK_DOCTYPE_PRESENTATION)
            deleteCommand = ".uno:DeleteAnnotation";
        else if (pDocument->pClass->getDocumentType(pDocument) == LOK_DOCTYPE_SPREADSHEET)
            deleteCommand = ".uno:DeleteNote";
    }

    lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), deleteCommand.c_str(), aArguments.c_str(), false);
}

/// Handles the key-press-event of the address entry widget.
gboolean signalAddressbar(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer /*pData*/)
{
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtk_widget_get_toplevel(pWidget));
    switch(pEvent->keyval)
    {
        case GDK_KEY_Return:
        {
            GtkEntry* pEntry = GTK_ENTRY(pWidget);
            const char* pText = gtk_entry_get_text(pEntry);

            boost::property_tree::ptree aTree;
            aTree.put(boost::property_tree::ptree::path_type("ToPoint/type", '/'), "string");
            aTree.put(boost::property_tree::ptree::path_type("ToPoint/value", '/'), pText);
            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            std::string aArguments = aStream.str();

            lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), ".uno:GoToCell", aArguments.c_str(), false);
            gtk_widget_grab_focus(window->lokdocview);
            return true;
        }
        case GDK_KEY_Escape:
        {
            std::string aArguments;
            lok_doc_view_post_command(LOK_DOC_VIEW(window->lokdocview), ".uno:Cancel", aArguments.c_str(), false);
            gtk_widget_grab_focus(window->lokdocview);
            return true;
        }
    }
    return FALSE;
}

/// Handles the key-press-event of the formula entry widget.
gboolean signalFormulabar(GtkWidget* /*pWidget*/, GdkEventKey* /*pEvent*/, gpointer /*pData*/)
{
    // for now it just displays the callback
    // TODO - submit the edited formula
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
