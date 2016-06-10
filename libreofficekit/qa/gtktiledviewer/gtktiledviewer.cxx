/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <cmath>
#include <string>
#include <map>
#include <iostream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>
#include <gdk/gdkkeysyms.h>

#include <sal/types.h>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#ifndef g_info
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

static int help()
{
    fprintf(stderr, "Usage: gtktiledviewer <absolute-path-to-libreoffice-install's-program-directory> <path-to-document> [<options> ... ]\n\n");
    fprintf(stderr, "Options:\n\n");
    fprintf(stderr, "--user-profile: Path to a custom user profile.\n");
    fprintf(stderr, "--background-color <color>: Set custom background color, e.g. 'yellow'.\n");
    fprintf(stderr, "--hide-page-shadow: Hide page/slide shadow.\n");
    fprintf(stderr, "--hide-whitespace: Hide whitespace between pages in text documents.\n");
    fprintf(stderr, "--user-profile: Path to a custom user profile.\n");
    return 1;
}

/// Represents the row or column header widget for spreadsheets.
class TiledRowColumnBar
{
public:
    /// Stores size and content of a single row header.
    struct Header
    {
        int m_nSize;
        std::string m_aText;
        Header(int nSize, const std::string& rText)
            : m_nSize(nSize),
            m_aText(rText)
        {
        }
    };

    enum TiledBarType { ROW, COLUMN };

    static const int ROW_HEADER_WIDTH = 50;
    static const int COLUMN_HEADER_HEIGHT = 20;

    GtkWidget* m_pDrawingArea;
    std::vector<Header> m_aHeaders;
    /// Height for row bar, width for column bar.
    int m_nSizePixel;
    /// Left/top position for the column/row bar -- initially 0, then may grow due to scrolling.
    int m_nPositionPixel;
    TiledBarType m_eType;

    explicit TiledRowColumnBar(TiledBarType eType);
    static gboolean draw(GtkWidget* pWidget, cairo_t* pCairo, gpointer pData);
    gboolean drawImpl(GtkWidget* pWidget, cairo_t* pCairo);
    static gboolean docConfigureEvent(GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer pData);
    /// Adjustments of the doc widget changed -- horizontal or vertical scroll.
    static void docAdjustmentChanged(GtkAdjustment* pAdjustment, gpointer pData);
    /// Draws rText at the center of rRectangle on pCairo.
    static void drawText(cairo_t* pCairo, const GdkRectangle& rRectangle, const std::string& rText);
};

/// Represents the button at the top left corner for spreadsheets.
class TiledCornerButton
{
public:
    GtkWidget* m_pDrawingArea;
    TiledCornerButton();
    static gboolean draw(GtkWidget* pWidget, cairo_t* pCairo, gpointer pData);
    static gboolean drawImpl(GtkWidget* pWidget, cairo_t* pCairo);
};

/// Represents all the state that is specific to one GtkWindow of this app.
class TiledWindow
{
public:
    GtkWidget* m_pDocView;
    GtkWidget* m_pStatusBar;
    GtkWidget* m_pProgressBar;
    GtkWidget* m_pStatusbarLabel;
    GtkWidget* m_pZoomLabel;
    GtkToolItem* m_pSaveButton;
    GtkToolItem* m_pCopyButton;
    GtkToolItem* m_pPasteButton;
    GtkToolItem* m_pUndo;
    GtkToolItem* m_pRedo;
    GtkToolItem* m_pEnableEditing;
    GtkToolItem* m_pBold;
    GtkToolItem* m_pItalic;
    GtkToolItem* m_pUnderline;
    GtkToolItem* m_pStrikethrough;
    GtkToolItem* m_pSuperscript;
    GtkToolItem* m_pSubscript;
    GtkToolItem* m_pLeftpara;
    GtkToolItem* m_pCenterpara;
    GtkToolItem* m_pRightpara;
    GtkToolItem* m_pJustifypara;
    GtkToolItem* m_pInsertAnnotation;
    GtkToolItem* m_pDeleteComment;
    GtkWidget* m_pFormulabarEntry;
    GtkWidget* m_pScrolledWindow;
    std::map<GtkToolItem*, std::string> m_aToolItemCommandNames;
    std::map<std::string, GtkToolItem*> m_aCommandNameToolItems;
    bool m_bToolItemBroadcast;
    GtkWidget* m_pVBox;
    GtkComboBoxText* m_pPartSelector;
    GtkWidget* m_pPartModeComboBox;
    /// Should the part selector avoid calling lok::Document::setPart()?
    bool m_bPartSelectorBroadcast;
    GtkWidget* m_pFindbar;
    GtkWidget* m_pFindbarEntry;
    GtkWidget* m_pFindbarLabel;
    bool m_bFindAll;
    std::shared_ptr<TiledRowColumnBar> m_pRowBar;
    std::shared_ptr<TiledRowColumnBar> m_pColumnBar;
    std::shared_ptr<TiledCornerButton> m_pCornerButton;
    std::string m_aAuthor;

    TiledWindow()
        : m_pDocView(nullptr),
        m_pStatusBar(nullptr),
        m_pProgressBar(nullptr),
        m_pStatusbarLabel(nullptr),
        m_pZoomLabel(nullptr),
        m_pSaveButton(nullptr),
        m_pCopyButton(nullptr),
        m_pPasteButton(nullptr),
        m_pUndo(nullptr),
        m_pRedo(nullptr),
        m_pEnableEditing(nullptr),
        m_pBold(nullptr),
        m_pItalic(nullptr),
        m_pUnderline(nullptr),
        m_pStrikethrough(nullptr),
        m_pSuperscript(nullptr),
        m_pSubscript(nullptr),
        m_pLeftpara(nullptr),
        m_pCenterpara(nullptr),
        m_pRightpara(nullptr),
        m_pJustifypara(nullptr),
        m_pInsertAnnotation(nullptr),
        m_pDeleteComment(nullptr),
        m_pFormulabarEntry(nullptr),
        m_pScrolledWindow(nullptr),
        m_bToolItemBroadcast(true),
        m_pVBox(nullptr),
        m_pPartSelector(nullptr),
        m_pPartModeComboBox(nullptr),
        m_bPartSelectorBroadcast(true),
        m_pFindbar(nullptr),
        m_pFindbarEntry(nullptr),
        m_pFindbarLabel(nullptr),
        m_bFindAll(false)
    {
        struct passwd* pPasswd = getpwuid(getuid());
        m_aAuthor = std::string(pPasswd->pw_gecos);
    }
};

static std::map<GtkWidget*, TiledWindow> g_aWindows;

static void setupDocView(GtkWidget* pDocView);
static GtkWidget* createWindow(TiledWindow& rWindow);
static void openDocumentCallback (GObject* source_object, GAsyncResult* res, gpointer userdata);

static TiledWindow& lcl_getTiledWindow(GtkWidget* pWidget)
{
    GtkWidget* pToplevel = gtk_widget_get_toplevel(pWidget);
    assert(g_aWindows.find(pToplevel) != g_aWindows.end());
    return g_aWindows[pToplevel];
}

TiledRowColumnBar::TiledRowColumnBar(TiledBarType eType)
    : m_pDrawingArea(gtk_drawing_area_new()),
    m_nSizePixel(0),
    m_eType(eType)
{
    if (m_eType == ROW)
        gtk_widget_set_size_request(m_pDrawingArea, ROW_HEADER_WIDTH, -1);
    else
        gtk_widget_set_size_request(m_pDrawingArea, -1, COLUMN_HEADER_HEIGHT);
    g_signal_connect(m_pDrawingArea, "draw", G_CALLBACK(TiledRowColumnBar::draw), this);
}

gboolean TiledRowColumnBar::draw(GtkWidget* pWidget, cairo_t* pCairo, gpointer pData)
{
    return static_cast<TiledRowColumnBar*>(pData)->drawImpl(pWidget, pCairo);
}

void TiledRowColumnBar::drawText(cairo_t* pCairo, const GdkRectangle& rRectangle, const std::string& rText)
{
    cairo_text_extents_t extents;
    cairo_text_extents(pCairo, rText.c_str(), &extents);
    // Cairo reference point for text is the bottom left corner.
    cairo_move_to(pCairo, rRectangle.x + rRectangle.width / 2 - extents.width / 2, rRectangle.y + rRectangle.height / 2 + extents.height / 2);
    cairo_show_text(pCairo, rText.c_str());
}

gboolean TiledRowColumnBar::drawImpl(GtkWidget* /*pWidget*/, cairo_t* pCairo)
{
    cairo_set_source_rgb(pCairo, 0, 0, 0);

    int nPrevious = 0;
    for (const Header& rHeader : m_aHeaders)
    {
        GdkRectangle aRectangle;
        if (m_eType == ROW)
        {
            aRectangle.x = 0;
            aRectangle.y = nPrevious - 1;
            aRectangle.width = ROW_HEADER_WIDTH - 1;
            aRectangle.height = rHeader.m_nSize - nPrevious;
            // Left line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.y, 1, aRectangle.height);
            cairo_fill(pCairo);
            // Bottom line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.y + aRectangle.height, aRectangle.width, 1);
            cairo_fill(pCairo);
            // Right line.
            cairo_rectangle(pCairo, aRectangle.width, aRectangle.y, 1, aRectangle.height);
            cairo_fill(pCairo);
        }
        else
        {
            aRectangle.x = nPrevious - 1;
            aRectangle.y = 0;
            aRectangle.width = rHeader.m_nSize - nPrevious;
            aRectangle.height = COLUMN_HEADER_HEIGHT - 1;
            // Top line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.y, aRectangle.width, 1);
            cairo_fill(pCairo);
            // Right line.
            cairo_rectangle(pCairo, aRectangle.x + aRectangle.width , aRectangle.y, 1, aRectangle.height);
            cairo_fill(pCairo);
            // Bottom line.
            cairo_rectangle(pCairo, aRectangle.x, aRectangle.height, aRectangle.width, 1);
            cairo_fill(pCairo);
        }
        drawText(pCairo, aRectangle, rHeader.m_aText);
        nPrevious = rHeader.m_nSize;
        if (rHeader.m_nSize > m_nSizePixel)
            break;
    }

    return FALSE;
}

void TiledRowColumnBar::docAdjustmentChanged(GtkAdjustment* /*pAdjustment*/, gpointer pData)
{
    GtkWidget* pDocView = static_cast<GtkWidget*>(pData);
    docConfigureEvent(pDocView, nullptr, nullptr);
}

gboolean TiledRowColumnBar::docConfigureEvent(GtkWidget* pDocView, GdkEventConfigure* /*pEvent*/, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pDocView);
    GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    rWindow.m_pRowBar->m_nSizePixel = gtk_adjustment_get_page_size(pVAdjustment);
    rWindow.m_pRowBar->m_nPositionPixel = gtk_adjustment_get_value(pVAdjustment);
    GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    rWindow.m_pColumnBar->m_nSizePixel = gtk_adjustment_get_page_size(pHAdjustment);
    rWindow.m_pColumnBar->m_nPositionPixel = gtk_adjustment_get_value(pHAdjustment);

    LibreOfficeKitDocument* pDocument = lok_doc_view_get_document(LOK_DOC_VIEW(pDocView));
    if (pDocument && pDocument->pClass->getDocumentType(pDocument) == LOK_DOCTYPE_SPREADSHEET)
    {
        std::stringstream aCommand;
        aCommand << ".uno:ViewRowColumnHeaders";
        aCommand << "?x=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), rWindow.m_pColumnBar->m_nPositionPixel));
        aCommand << "&width=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), rWindow.m_pColumnBar->m_nSizePixel));
        aCommand << "&y=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), rWindow.m_pRowBar->m_nPositionPixel));
        aCommand << "&height=" << int(lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), rWindow.m_pRowBar->m_nSizePixel));
        std::stringstream ss;
        ss << "lok::Document::getCommandValues(" << aCommand.str() << ")";
        g_info("%s", ss.str().c_str());
        char* pValues = pDocument->pClass->getCommandValues(pDocument, aCommand.str().c_str());
        g_info("lok::Document::getCommandValues() returned '%s'", pValues);
        std::stringstream aStream(pValues);
        free(pValues);
        assert(!aStream.str().empty());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        gtk_widget_show(rWindow.m_pCornerButton->m_pDrawingArea);

        rWindow.m_pRowBar->m_aHeaders.clear();
        try
        {
            for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
            {
                int nSize = std::round(lok_doc_view_twip_to_pixel(LOK_DOC_VIEW(pDocView), std::atof(rValue.second.get<std::string>("size").c_str())));
                if (nSize >= rWindow.m_pRowBar->m_nPositionPixel)
                {
                    int nScrolledSize = nSize - rWindow.m_pRowBar->m_nPositionPixel;
                    Header aHeader(nScrolledSize, rValue.second.get<std::string>("text"));
                    rWindow.m_pRowBar->m_aHeaders.push_back(aHeader);
                }
            }
        }
        catch (boost::property_tree::ptree_bad_path& rException)
        {
            std::cerr << "TiledRowColumnBar::docConfigureEvent: failed to get rows: " << rException.what() << std::endl;
        }
        gtk_widget_show(rWindow.m_pRowBar->m_pDrawingArea);
        gtk_widget_queue_draw(rWindow.m_pRowBar->m_pDrawingArea);

        rWindow.m_pColumnBar->m_aHeaders.clear();
        try
        {
            for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("columns"))
            {
                int nSize = std::round(lok_doc_view_twip_to_pixel(LOK_DOC_VIEW(pDocView), std::atof(rValue.second.get<std::string>("size").c_str())));
                if (nSize >= rWindow.m_pColumnBar->m_nPositionPixel)
                {
                    int nScrolledSize = nSize - rWindow.m_pColumnBar->m_nPositionPixel;
                    Header aHeader(nScrolledSize, rValue.second.get<std::string>("text"));
                    rWindow.m_pColumnBar->m_aHeaders.push_back(aHeader);
                }
            }
        }
        catch (boost::property_tree::ptree_bad_path& rException)
        {
            std::cerr << "TiledRowColumnBar::docConfigureEvent: failed to get columns: " << rException.what() << std::endl;
        }
        gtk_widget_show(rWindow.m_pColumnBar->m_pDrawingArea);
        gtk_widget_queue_draw(rWindow.m_pColumnBar->m_pDrawingArea);
        gtk_widget_show(rWindow.m_pFormulabarEntry);
    }

    return TRUE;
}

TiledCornerButton::TiledCornerButton()
    : m_pDrawingArea(gtk_drawing_area_new())
{
    gtk_widget_set_size_request(m_pDrawingArea, TiledRowColumnBar::ROW_HEADER_WIDTH, TiledRowColumnBar::COLUMN_HEADER_HEIGHT);
    g_signal_connect(m_pDrawingArea, "draw", G_CALLBACK(TiledCornerButton::draw), this);
}

gboolean TiledCornerButton::draw(GtkWidget* pWidget, cairo_t* pCairo, gpointer)
{
    return drawImpl(pWidget, pCairo);
}

gboolean TiledCornerButton::drawImpl(GtkWidget* /*pWidget*/, cairo_t* pCairo)
{
    cairo_set_source_rgb(pCairo, 0, 0, 0);

    GdkRectangle aRectangle;
    aRectangle.x = 0;
    aRectangle.y = 0;
    aRectangle.width = TiledRowColumnBar::ROW_HEADER_WIDTH;
    aRectangle.height = TiledRowColumnBar::COLUMN_HEADER_HEIGHT;
    cairo_rectangle(pCairo, aRectangle.x, aRectangle.y, aRectangle.width, aRectangle.height);
    cairo_stroke(pCairo);

    return FALSE;
}

static void lcl_registerToolItem(TiledWindow& rWindow, GtkToolItem* pItem, const std::string& rName)
{
    rWindow.m_aToolItemCommandNames[pItem] = rName;
    rWindow.m_aCommandNameToolItems[rName] = pItem;
}

const float fZooms[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 5.0 };

static void iterateUnoParams(GtkWidget* pWidget, gpointer userdata)
{
    boost::property_tree::ptree *pTree = static_cast<boost::property_tree::ptree*>(userdata);

    GList* pChildren = gtk_container_get_children(GTK_CONTAINER(pWidget));
    GList* pIt;
    guint i = 0;
    const gchar* unoParam[3];
    for (pIt = pChildren, i = 0; pIt != nullptr && i < 3; pIt = pIt->next, i++)
    {
        unoParam[i] = gtk_entry_get_text(GTK_ENTRY(pIt->data));
    }

    pTree->put(boost::property_tree::ptree::path_type(g_strconcat(unoParam[1], "/", "type", nullptr), '/'), unoParam[0]);
    pTree->put(boost::property_tree::ptree::path_type(g_strconcat(unoParam[1], "/", "value", nullptr), '/'), unoParam[2]);
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
    gtk_box_pack_start(GTK_BOX(pUnoParamAreaBox), pParamContainer, TRUE, TRUE, 2);

    GtkWidget* pTypeEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pTypeEntry, TRUE, TRUE, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pTypeEntry), "Param type (Eg. boolean, string etc.)");

    GtkWidget* pNameEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pNameEntry, TRUE, TRUE, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pNameEntry), "Param name");

    GtkWidget* pValueEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pParamContainer), pValueEntry, TRUE, TRUE, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pValueEntry), "Param value");

    GtkWidget* pRemoveButton = gtk_button_new_from_icon_name("list-remove-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(pRemoveButton, "clicked", G_CALLBACK(removeUnoParam), pUnoParamAreaBox);
    gtk_box_pack_start(GTK_BOX(pParamContainer), pRemoveButton, TRUE, TRUE, 2);

    gtk_widget_show_all(pUnoParamAreaBox);
}

static void unoCommandDebugger(GtkWidget* pButton, gpointer /* pItem */)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    LOKDocView* pDocView = LOK_DOC_VIEW(rWindow.m_pDocView);

    GtkWidget* pUnoCmdDialog = gtk_dialog_new_with_buttons ("Execute UNO command",
                                                            GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(pDocView))),
                                                            GTK_DIALOG_MODAL,
                                                            "Execute",
                                                            GTK_RESPONSE_OK,
                                                            nullptr);
    g_object_set(G_OBJECT(pUnoCmdDialog), "resizable", FALSE, nullptr);
    GtkWidget* pDialogMessageArea = gtk_dialog_get_content_area (GTK_DIALOG (pUnoCmdDialog));
    GtkWidget* pUnoCmdAreaBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pUnoCmdAreaBox, TRUE, TRUE, 2);

    GtkWidget* pUnoCmdLabel = gtk_label_new("Enter UNO command");
    gtk_box_pack_start(GTK_BOX(pUnoCmdAreaBox), pUnoCmdLabel, TRUE, TRUE, 2);

    GtkWidget* pUnoCmdEntry = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(pUnoCmdAreaBox), pUnoCmdEntry, TRUE, TRUE, 2);
    gtk_entry_set_placeholder_text(GTK_ENTRY(pUnoCmdEntry), "UNO command (Eg. Bold, Italic etc.)");

    GtkWidget* pUnoParamAreaBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pUnoParamAreaBox, TRUE, TRUE, 2);

    GtkWidget* pAddMoreButton = gtk_button_new_with_label("Add UNO parameter");
    gtk_box_pack_start(GTK_BOX(pDialogMessageArea), pAddMoreButton, TRUE, TRUE, 2);
    g_signal_connect(G_OBJECT(pAddMoreButton), "clicked", G_CALLBACK(addMoreUnoParam), pUnoParamAreaBox);

    gtk_widget_show_all(pUnoCmdDialog);

    gint res = gtk_dialog_run (GTK_DIALOG(pUnoCmdDialog));
    switch (res)
    {
    case GTK_RESPONSE_OK:
    {
        const gchar* sUnoCmd = g_strconcat(".uno:", gtk_entry_get_text(GTK_ENTRY(pUnoCmdEntry)), nullptr);

        boost::property_tree::ptree aTree;
        gtk_container_foreach(GTK_CONTAINER(pUnoParamAreaBox), iterateUnoParams, &aTree);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        std::string aArguments = aStream.str();

        g_info("Generated UNO command: %s %s", sUnoCmd, aArguments.c_str());

        lok_doc_view_post_command(pDocView, sUnoCmd, (aArguments.empty() ? nullptr : aArguments.c_str()), false);
    }
        break;
    }

    gtk_widget_destroy(pUnoCmdDialog);
}

/// Get the visible area of the scrolled window
static void getVisibleAreaTwips(GtkWidget* pDocView, GdkRectangle* pArea)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pDocView);

    GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));

    pArea->x      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_value(pHAdjustment));
    pArea->y      = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_value(pVAdjustment));
    pArea->width  = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_page_size(pHAdjustment));
    pArea->height = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView),
                                               gtk_adjustment_get_page_size(pVAdjustment));
}

static void changeZoom( GtkWidget* pButton, gpointer /* pItem */ )
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkWidget* pDocView = rWindow.m_pDocView;

    const char *sName = gtk_tool_button_get_icon_name( GTK_TOOL_BUTTON(pButton) );

    float fZoom = 0;
    float fCurrentZoom = 0;

    if ( pDocView )
    {
        fCurrentZoom = lok_doc_view_get_zoom( LOK_DOC_VIEW(pDocView) );
    }

    if ( strcmp(sName, "zoom-in-symbolic") == 0)
    {
        for ( unsigned int i = 0; i < sizeof( fZooms ) / sizeof( fZooms[0] ); i++ )
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
        for ( unsigned int i = 0; i < sizeof( fZooms ) / sizeof( fZooms[0] ); i++ )
        {
            if ( fCurrentZoom > fZooms[i] )
            {
                fZoom = fZooms[i];
            }
        }
    }

    if ( fZoom != 0 )
    {
        if ( pDocView )
        {
            lok_doc_view_set_zoom( LOK_DOC_VIEW(pDocView), fZoom );
            GdkRectangle aVisibleArea;
            getVisibleAreaTwips(pDocView, &aVisibleArea);
            lok_doc_view_set_visible_area(LOK_DOC_VIEW(pDocView), &aVisibleArea);
        }
    }
    std::string aZoom = std::to_string(int(fZoom * 100)) + std::string("%");
    gtk_label_set_text(GTK_LABEL(rWindow.m_pZoomLabel), aZoom.c_str());
}

/// User clicked on the button -> inform LOKDocView.
static void toggleEditing(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);

    LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);
    bool bActive = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rWindow.m_pEnableEditing));
    if (bool(lok_doc_view_get_edit(pLOKDocView)) != bActive)
        lok_doc_view_set_edit(pLOKDocView, bActive);
}

/// Toggles if search should find all results or only the first one.
static void toggleFindAll(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkEntry* pEntry = GTK_ENTRY(rWindow.m_pFindbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);

    rWindow.m_bFindAll = !rWindow.m_bFindAll;
    lok_doc_view_highlight_all(LOK_DOC_VIEW(rWindow.m_pDocView), pText);
}

/// Toggle the visibility of the findbar.
static void toggleFindbar(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    if (gtk_widget_get_visible(rWindow.m_pFindbar))
    {
        gtk_widget_hide(rWindow.m_pFindbar);
    }
    else
    {
        gtk_widget_show_all(rWindow.m_pFindbar);
        gtk_widget_grab_focus(rWindow.m_pFindbarEntry);
    }
}

static void
setLOKFeatures (GtkWidget* pDocView)
{
    g_object_set(G_OBJECT(pDocView),
                 "doc-password", TRUE,
                 "doc-password-to-modify", TRUE,
                 nullptr);
}

/// Common initialization, regardless if it's just a new view or a full init.
static TiledWindow& setupWidgetAndCreateWindow(GtkWidget* pDocView)
{
    setupDocView(pDocView);
    setLOKFeatures(pDocView);
    TiledWindow aWindow;
    aWindow.m_pDocView = pDocView;
    GtkWidget* pWindow = createWindow(aWindow);
    return lcl_getTiledWindow(pWindow);
}

/// Creates a new view, i.e. no LOK init or document load.
static void createView(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkWidget* pDocView = lok_doc_view_new_from_widget(LOK_DOC_VIEW(rWindow.m_pDocView));

    TiledWindow& rNewWindow = setupWidgetAndCreateWindow(pDocView);
    // Hide the unused progress bar.
    gtk_widget_show_all(rNewWindow.m_pStatusBar);
    gtk_widget_hide(rNewWindow.m_pProgressBar);
}

/// Creates a new model, i.e. LOK init and document load, one view implicitly.
static void createModelAndView(const char* pLOPath, const char* pDocPath, const std::vector<std::string>& rArguments)
{
    std::string aUserProfile;
    for (size_t i = 0; i < rArguments.size(); ++i)
    {
        const std::string& rArgument = rArguments[i];
        if (rArgument == "--user-profile" && i + 1 < rArguments.size())
            aUserProfile = std::string("vnd.sun.star.pathname:")
                + rArguments[i + 1].c_str();
    }
    const gchar* pUserProfile = aUserProfile.empty() ? nullptr : aUserProfile.c_str();
    GtkWidget* pDocView = lok_doc_view_new_from_user_profile(pLOPath, pUserProfile, nullptr, nullptr);

    setupWidgetAndCreateWindow(pDocView);

    boost::property_tree::ptree aTree;
    for (size_t i = 0; i < rArguments.size(); ++i)
    {
        const std::string& rArgument = rArguments[i];
        if (rArgument == "--background-color" && i + 1 < rArguments.size())
        {
            GdkRGBA color;
            gdk_rgba_parse(&color, rArguments[i + 1].c_str());
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            gtk_widget_override_background_color(gtk_widget_get_toplevel(pDocView), GTK_STATE_FLAG_NORMAL, &color);
            SAL_WNODEPRECATED_DECLARATIONS_POP
        }
        else if (rArgument == "--hide-page-shadow")
        {
            aTree.put(boost::property_tree::ptree::path_type(".uno:ShowBorderShadow/type", '/'), "boolean");
            aTree.put(boost::property_tree::ptree::path_type(".uno:ShowBorderShadow/value", '/'), false);
        }
        else if (rArgument == "--hide-whitespace")
        {
            aTree.put(boost::property_tree::ptree::path_type(".uno:HideWhitespace/type", '/'), "boolean");
            aTree.put(boost::property_tree::ptree::path_type(".uno:HideWhitespace/value", '/'), true);
        }
    }

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    std::string aArguments = aStream.str();
    lok_doc_view_open_document(LOK_DOC_VIEW(pDocView), pDocPath, aArguments.c_str(), nullptr, openDocumentCallback, pDocView);
}

/// Our GtkClipboardGetFunc implementation for HTML.
static void htmlGetFunc(GtkClipboard* /*pClipboard*/, GtkSelectionData* pSelectionData, guint /*info*/, gpointer pUserData)
{
    GdkAtom aAtom(gdk_atom_intern("text/html", false));
    const gchar* pSelection = static_cast<const gchar*>(pUserData);
    gtk_selection_data_set(pSelectionData, aAtom, 8, reinterpret_cast<const guchar *>(pSelection), strlen(pSelection));
}

/// Our GtkClipboardClearFunc implementation for HTML.
static void htmlClearFunc(GtkClipboard* /*pClipboard*/, gpointer pData)
{
    g_free(pData);
}

/// Same as gtk_clipboard_set_text(), but sets HTML.
static void clipboardSetHtml(GtkClipboard* pClipboard, const char* pSelection)
{
    GtkTargetList* pList = gtk_target_list_new(nullptr, 0);
    GdkAtom aAtom(gdk_atom_intern("text/html", false));
    gtk_target_list_add(pList, aAtom, 0, 0);
    gint nTargets = 0;
    GtkTargetEntry* pTargets = gtk_target_table_new_from_list(pList, &nTargets);

    gtk_clipboard_set_with_data(pClipboard, pTargets, nTargets, htmlGetFunc, htmlClearFunc, g_strdup(pSelection));

    gtk_target_table_free(pTargets, nTargets);
    gtk_target_list_unref(pList);
}

/// Handler for the copy button: write clipboard.
static void doCopy(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);
    char* pUsedFormat = nullptr;
    // TODO: Should check `text-selection` signal before trying to copy
    char* pSelection = lok_doc_view_copy_selection(pLOKDocView, "text/html", &pUsedFormat);
    if (!pSelection)
        return;

    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(rWindow.m_pDocView), GDK_SELECTION_CLIPBOARD);
    std::string aUsedFormat(pUsedFormat);
    if (aUsedFormat == "text/plain;charset=utf-8")
        gtk_clipboard_set_text(pClipboard, pSelection, -1);
    else
        clipboardSetHtml(pClipboard, pSelection);

    free(pSelection);
    free(pUsedFormat);
}

static void doPaste(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);

    GtkClipboard* pClipboard = gtk_clipboard_get_for_display(gtk_widget_get_display(rWindow.m_pDocView), GDK_SELECTION_CLIPBOARD);

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

    boost::optional<GdkAtom> oTarget;
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
        bool bSuccess = lok_doc_view_paste(pLOKDocView, aTargetName.c_str(), reinterpret_cast<const char*>(pData), nLength);
        gtk_selection_data_free(pSelectionData);
        if (bSuccess)
            return;
    }

    gchar* pText = gtk_clipboard_wait_for_text(pClipboard);
    if (pText)
        lok_doc_view_paste(pLOKDocView, "text/plain;charset=utf-8", pText, strlen(pText));
}

/// Click handler for the search next button.
static void signalSearchNext(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkEntry* pEntry = GTK_ENTRY(rWindow.m_pFindbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);

    lok_doc_view_find_next(LOK_DOC_VIEW(rWindow.m_pDocView), pText, rWindow.m_bFindAll);
}

/// Click handler for the search previous button.
static void signalSearchPrev(GtkWidget* pButton, gpointer /*pItem*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pButton);
    GtkEntry* pEntry = GTK_ENTRY(rWindow.m_pFindbarEntry);
    const char* pText = gtk_entry_get_text(pEntry);

    lok_doc_view_find_prev(LOK_DOC_VIEW(rWindow.m_pDocView), pText, rWindow.m_bFindAll);
}

/// Handles the key-press-event of the search entry widget.
static gboolean signalFindbar(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pWidget);
    gtk_label_set_text(GTK_LABEL(rWindow.m_pFindbarLabel), "");
    switch(pEvent->keyval)
    {
        case GDK_KEY_Return:
        {
            // Search forward.
            signalSearchNext(pWidget, nullptr);
            return TRUE;
        }
        case GDK_KEY_Escape:
        {
            // Hide the findbar.
            gtk_widget_hide(rWindow.m_pFindbar);
            return TRUE;
        }
    }
    return FALSE;
}

/// Handles the key-press-event of the formula entry widget.
static gboolean signalFormulabar(GtkWidget* /*pWidget*/, GdkEventKey* /*pEvent*/, gpointer /*pData*/)
{
    // for now it just displays the callback
    // TODO - submit the edited formula
    return TRUE;
}

/// LOKDocView changed edit state -> inform the tool button.
static void signalEdit(LOKDocView* pLOKDocView, gboolean bWasEdit, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    gboolean bEdit = lok_doc_view_get_edit(pLOKDocView);
    g_info("signalEdit: %d -> %d", bWasEdit, bEdit);

    // Set toggle button sensitivity
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pBold), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pItalic), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pUnderline), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pStrikethrough), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pSuperscript), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pSubscript), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pLeftpara), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pCenterpara), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pRightpara), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pJustifypara), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pInsertAnnotation), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pDeleteComment), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pUndo), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pRedo), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pPasteButton), bEdit);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pSaveButton), bEdit);
}

/// LOKDocView changed command state -> inform the tool button.
static void signalCommand(LOKDocView* pLOKDocView, char* pPayload, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));

    std::string aPayload(pPayload);
    size_t nPosition = aPayload.find("=");
    if (nPosition != std::string::npos)
    {
        std::string aKey = aPayload.substr(0, nPosition);
        std::string aValue = aPayload.substr(nPosition + 1);

        if (rWindow.m_aCommandNameToolItems.find(aKey) != rWindow.m_aCommandNameToolItems.end())
        {
            GtkToolItem* pItem = rWindow.m_aCommandNameToolItems[aKey];
            gboolean bEdit = aValue == "true";
            if (GTK_IS_TOGGLE_TOOL_BUTTON(pItem))
            {
                if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(pItem)) != bEdit)
                {
                    // Avoid invoking lok_doc_view_post_command().
                    rWindow.m_bToolItemBroadcast = false;
                    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(pItem), bEdit);
                    rWindow.m_bToolItemBroadcast = true;
                }
            }
        }
    }
}

/// LOKDocView command finished -> just write it to the console, not that useful for the viewer.
static void signalCommandResult(LOKDocView* /*pLOKDocView*/, char* pPayload, gpointer /*pData*/)
{
    fprintf(stderr, "Command finished: %s\n", pPayload);
}

static void loadChanged(LOKDocView* /*pLOKDocView*/, gdouble fValue, gpointer pData)
{
    GtkWidget* pProgressBar = GTK_WIDGET (pData);
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pProgressBar), fValue);
}

/// LOKDocView found no search matches -> set the search label accordingly.
static void signalSearch(LOKDocView* pLOKDocView, char* /*pPayload*/, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    gtk_label_set_text(GTK_LABEL(rWindow.m_pFindbarLabel), "Search key not found");
}

/// LOKDocView found some search matches -> set the search label accordingly.
static void signalSearchResultCount(LOKDocView* pLOKDocView, char* pPayload, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    std::stringstream ss;
    ss << pPayload << " match(es)";
    gtk_label_set_text(GTK_LABEL(rWindow.m_pFindbarLabel), ss.str().c_str());
}

static void signalPart(LOKDocView* pLOKDocView, int nPart, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    rWindow.m_bPartSelectorBroadcast = false;
    gtk_combo_box_set_active(GTK_COMBO_BOX(rWindow.m_pPartSelector), nPart);
    rWindow.m_bPartSelectorBroadcast = true;
}

/// User clicked on a command button -> inform LOKDocView.
static void signalHyperlink(LOKDocView* /*pLOKDocView*/, char* pPayload, gpointer /*pData*/)
{
    GError* pError = nullptr;
    gtk_show_uri(nullptr, pPayload, GDK_CURRENT_TIME, &pError);
    if (pError != nullptr)
    {
        g_warning("Unable to show URI %s : %s", pPayload, pError->message);
        g_error_free(pError);
    }
}

/// Cursor position changed
static void cursorChanged(LOKDocView* pDocView, gint nX, gint nY,
                          gint /*nWidth*/, gint /*nHeight*/, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pDocView));

    GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    GdkRectangle visArea;
    gdouble upper;
    gint x = -1, y = -1;

    getVisibleAreaTwips(GTK_WIDGET(pDocView), &visArea);

    // check vertically
    if (nY < visArea.y)
    {
        y = nY - visArea.height/2;
        if (y < 0)
            y = gtk_adjustment_get_lower(vadj);
    }
    else if (nY > visArea.y + visArea.height)
    {
        y = nY - visArea.height/2;
        upper = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), gtk_adjustment_get_upper(vadj));
        if (y > upper)
            y = upper;

    }

    if (nX < visArea.x)
    {
        x = nX - visArea.width/2;
        if (x < 0)
            x = gtk_adjustment_get_lower(hadj);
    }
    else if (nX > visArea.x + visArea.width)
    {
        x = nX - visArea.width/2;
        upper = lok_doc_view_pixel_to_twip(LOK_DOC_VIEW(pDocView), gtk_adjustment_get_upper(hadj));
        if (x > upper)
            x = upper;
    }

    if (y!=-1)
        gtk_adjustment_set_value(vadj, lok_doc_view_twip_to_pixel(LOK_DOC_VIEW(pDocView), y));
    if (x!=-1)
        gtk_adjustment_set_value(hadj, lok_doc_view_twip_to_pixel(LOK_DOC_VIEW(pDocView), x));
}

/// LOKDocView the formula has changed
static void formulaChanged(LOKDocView* pLOKDocView, char* pPayload, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    gtk_entry_set_text(GTK_ENTRY(rWindow.m_pFormulabarEntry), pPayload);
}

/// LOKDocView password is requried to open the document
static void passwordRequired(LOKDocView* pLOKDocView, gchar* pUrl, gboolean bModify, gpointer /*pData*/)
{
    GtkWidget* pPasswordDialog = gtk_dialog_new_with_buttons ("Password required",
                                                              GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(pLOKDocView))),
                                                              GTK_DIALOG_MODAL,
                                                              "OK",
                                                              GTK_RESPONSE_OK,
                                                              nullptr);
    g_object_set(G_OBJECT(pPasswordDialog), "resizable", FALSE, nullptr);
    GtkWidget* pDialogMessageArea = gtk_dialog_get_content_area (GTK_DIALOG (pPasswordDialog));
    GtkWidget* pPasswordEntry = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY(pPasswordEntry), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY(pPasswordEntry), '*');
    gtk_box_pack_end(GTK_BOX(pDialogMessageArea), pPasswordEntry, TRUE, TRUE, 2);
    if (bModify)
    {
        GtkWidget* pSecondaryLabel = gtk_label_new ("Document requires password to edit");
        gtk_box_pack_end(GTK_BOX(pDialogMessageArea), pSecondaryLabel, TRUE, TRUE, 2);
        gtk_dialog_add_button (GTK_DIALOG (pPasswordDialog), "Open as read-only", GTK_RESPONSE_ACCEPT);
    }
    gtk_widget_show_all(pPasswordDialog);

    gint res = gtk_dialog_run (GTK_DIALOG(pPasswordDialog));
    switch (res)
    {
    case GTK_RESPONSE_OK:
        lok_doc_view_set_document_password (pLOKDocView, pUrl, gtk_entry_get_text(GTK_ENTRY(pPasswordEntry)));
        break;
    case GTK_RESPONSE_ACCEPT:
        // User accepts to open this document as read-only
    case GTK_RESPONSE_DELETE_EVENT:
        lok_doc_view_set_document_password (pLOKDocView, pUrl, nullptr);
        break;
    }

    gtk_widget_destroy(pPasswordDialog);
}

static void toggleToolItem(GtkWidget* pWidget, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(pWidget);

    if (rWindow.m_bToolItemBroadcast)
    {
        LOKDocView* pLOKDocView = LOK_DOC_VIEW(rWindow.m_pDocView);
        GtkToolItem* pItem = GTK_TOOL_ITEM(pWidget);
        const std::string& rString = rWindow.m_aToolItemCommandNames[pItem];
        g_info("toggleToolItem: lok_doc_view_post_command('%s')", rString.c_str());

        // notify about the finished Save
        gboolean bNotify = (rString == ".uno:Save");

        std::string aArguments;
        if (rString == ".uno:InsertAnnotation" && !rWindow.m_aAuthor.empty())
        {
            boost::property_tree::ptree aTree;
            aTree.put(boost::property_tree::ptree::path_type("Author/type", '/'), "string");
            aTree.put(boost::property_tree::ptree::path_type("Author/value", '/'), rWindow.m_aAuthor);
            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            aArguments = aStream.str();
        }

        lok_doc_view_post_command(pLOKDocView, rString.c_str(), (aArguments.empty() ? nullptr : aArguments.c_str()), bNotify);
    }
}

static void populatePartSelector(LOKDocView* pLOKDocView)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    gtk_list_store_clear( GTK_LIST_STORE(
                              gtk_combo_box_get_model(
                                  GTK_COMBO_BOX(rWindow.m_pPartSelector) )) );

    if (!pLOKDocView)
    {
        return;
    }

    const int nMaxLength = 50;
    char sText[nMaxLength];

    int nParts = lok_doc_view_get_parts(pLOKDocView);
    for ( int i = 0; i < nParts; i++ )
    {
        char* pName = lok_doc_view_get_part_name(pLOKDocView, i);
        assert( pName );
        snprintf( sText, nMaxLength, "%i (%s)", i+1, pName );
        free( pName );

        gtk_combo_box_text_append_text( rWindow.m_pPartSelector, sText );
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(rWindow.m_pPartSelector), lok_doc_view_get_part(pLOKDocView));
}

static void signalSize(LOKDocView* pLOKDocView, gpointer /*pData*/)
{
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pLOKDocView));
    rWindow.m_bPartSelectorBroadcast = false;
    populatePartSelector(pLOKDocView);
    rWindow.m_bPartSelectorBroadcast = true;
}

static void changePart( GtkWidget* pSelector, gpointer /* pItem */ )
{
    int nPart = gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) );
    TiledWindow& rWindow = lcl_getTiledWindow(pSelector);

    if (rWindow.m_bPartSelectorBroadcast && rWindow.m_pDocView)
    {
        lok_doc_view_set_part( LOK_DOC_VIEW(rWindow.m_pDocView), nPart );
        lok_doc_view_reset_view(LOK_DOC_VIEW(rWindow.m_pDocView));
    }
}

static void removeChildrenFromStatusbar(GtkWidget* children, gpointer pData)
{
    GtkWidget* pStatusBar = static_cast<GtkWidget*>(pData);

    gtk_container_remove(GTK_CONTAINER(pStatusBar), children);
}

static void populatePartModeSelector( GtkComboBoxText* pSelector )
{
    gtk_combo_box_text_append_text( pSelector, "Standard" );
    gtk_combo_box_text_append_text( pSelector, "Notes" );
    gtk_combo_box_set_active( GTK_COMBO_BOX(pSelector), 0 );
}

static void changePartMode( GtkWidget* pSelector, gpointer /* pItem */ )
{
    // Just convert directly back to the LibreOfficeKitPartMode enum.
    // I.e. the ordering above should match the enum member ordering.
    LibreOfficeKitPartMode ePartMode =
        LibreOfficeKitPartMode( gtk_combo_box_get_active( GTK_COMBO_BOX(pSelector) ) );
    TiledWindow& rWindow = lcl_getTiledWindow(pSelector);

    if ( rWindow.m_pDocView )
    {
        lok_doc_view_set_partmode( LOK_DOC_VIEW(rWindow.m_pDocView), ePartMode );
    }
}

static void openDocumentCallback (GObject* source_object, GAsyncResult* res, gpointer /*userdata*/)
{
    LOKDocView* pDocView = LOK_DOC_VIEW (source_object);
    TiledWindow& rWindow = lcl_getTiledWindow(GTK_WIDGET(pDocView));
    GError* error = nullptr;
    GList *focusChain = nullptr;

    if (!lok_doc_view_open_document_finish(pDocView, res, &error))
    {
        GtkDialogFlags eFlags = GTK_DIALOG_DESTROY_WITH_PARENT;
        GtkWidget* pDialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pDocView))),
                                                    eFlags,
                                                    GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE,
                                                    "Error occurred while opening the document: '%s'",
                                                    error->message);
        gtk_dialog_run(GTK_DIALOG(pDialog));
        gtk_widget_destroy(pDialog);

        g_error_free(error);
        gtk_widget_destroy(GTK_WIDGET(pDocView));
        gtk_main_quit();
        return;
    }

    populatePartSelector(pDocView);
    populatePartModeSelector( GTK_COMBO_BOX_TEXT(rWindow.m_pPartModeComboBox) );
    // Connect these signals after populating the selectors, to avoid re-rendering on setting the default part/partmode.
    g_signal_connect(G_OBJECT(rWindow.m_pPartModeComboBox), "changed", G_CALLBACK(changePartMode), 0);
    g_signal_connect(G_OBJECT(rWindow.m_pPartSelector), "changed", G_CALLBACK(changePart), 0);

    focusChain = g_list_append( focusChain, pDocView );
    gtk_container_set_focus_chain ( GTK_CONTAINER (rWindow.m_pVBox), focusChain );

    gtk_widget_show_all(rWindow.m_pStatusBar);
    gtk_widget_hide(rWindow.m_pProgressBar);
}

/// Creates the GtkWindow that has main widget as children and registers it in the window map.
static GtkWidget* createWindow(TiledWindow& rWindow)
{
    GtkWidget *pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pWindow), "LibreOfficeKit GTK Tiled Viewer");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 1280, 720);
    g_signal_connect(pWindow, "destroy", G_CALLBACK(gtk_main_quit), 0);

    rWindow.m_pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(pWindow), rWindow.m_pVBox);

    // Upper toolbar.
    GtkWidget* pUpperToolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(pUpperToolbar), GTK_TOOLBAR_ICONS);

    // Save.
    rWindow.m_pSaveButton = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pSaveButton), "document-save-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pSaveButton, "Save");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), rWindow.m_pSaveButton, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pSaveButton), "clicked", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pSaveButton, ".uno:Save");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pSaveButton), false);

    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), gtk_separator_tool_item_new(), -1);

    // Copy and paste.
    rWindow.m_pCopyButton = gtk_tool_button_new( nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pCopyButton), "edit-copy-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pCopyButton, "Copy");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), rWindow.m_pCopyButton, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pCopyButton), "clicked", G_CALLBACK(doCopy), nullptr);

    rWindow.m_pPasteButton = gtk_tool_button_new( nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pPasteButton), "edit-paste-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pPasteButton, "Paste");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), rWindow.m_pPasteButton, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pPasteButton), "clicked", G_CALLBACK(doPaste), nullptr);
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pPasteButton), false);

    gtk_toolbar_insert( GTK_TOOLBAR(pUpperToolbar), gtk_separator_tool_item_new(), -1);

    // Undo and redo.
    rWindow.m_pUndo = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pUndo), "edit-undo-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pUndo, "Undo");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), rWindow.m_pUndo, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pUndo), "clicked", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pUndo, ".uno:Undo");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pUndo), false);

    rWindow.m_pRedo = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pRedo), "edit-redo-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pRedo, "Redo");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), rWindow.m_pRedo, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pRedo), "clicked", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pRedo, ".uno:Redo");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pRedo), false);

    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), gtk_separator_tool_item_new(), -1);

    // Find.
    GtkToolItem* pFindButton = gtk_tool_button_new( nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindButton), "edit-find-symbolic");
    gtk_tool_item_set_tooltip_text(pFindButton, "Find");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pFindButton, -1);
    g_signal_connect(G_OBJECT(pFindButton), "clicked", G_CALLBACK(toggleFindbar), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), gtk_separator_tool_item_new(), -1);

    // Misc upper toolbar.
    GtkToolItem* pZoomIn = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pZoomIn), "zoom-in-symbolic");
    gtk_tool_item_set_tooltip_text(pZoomIn, "Zoom In");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pZoomIn, -1);
    g_signal_connect(G_OBJECT(pZoomIn), "clicked", G_CALLBACK(changeZoom), nullptr);

    GtkToolItem* pZoom1 = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(pZoom1), "zoom-original-symbolic");
    gtk_tool_item_set_tooltip_text(pZoom1, "Normal Size");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pZoom1, -1);
    g_signal_connect(G_OBJECT(pZoom1), "clicked", G_CALLBACK(changeZoom), nullptr);

    GtkToolItem* pZoomOut = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pZoomOut), "zoom-out-symbolic");
    gtk_tool_item_set_tooltip_text(pZoomOut, "Zoom Out");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pZoomOut, -1);
    g_signal_connect(G_OBJECT(pZoomOut), "clicked", G_CALLBACK(changeZoom), nullptr);

    GtkToolItem* pPartSelectorToolItem = gtk_tool_item_new();
    GtkWidget* pComboBox = gtk_combo_box_text_new();
    gtk_container_add(GTK_CONTAINER(pPartSelectorToolItem), pComboBox);
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pPartSelectorToolItem, -1);

    rWindow.m_pPartSelector = GTK_COMBO_BOX_TEXT(pComboBox);

    GtkToolItem* pPartModeSelectorToolItem = gtk_tool_item_new();
    rWindow.m_pPartModeComboBox = gtk_combo_box_text_new();
    gtk_container_add(GTK_CONTAINER(pPartModeSelectorToolItem), rWindow.m_pPartModeComboBox);
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pPartModeSelectorToolItem, -1);

    GtkToolItem* pEnableEditing = gtk_toggle_tool_button_new();
    rWindow.m_pEnableEditing = pEnableEditing;
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pEnableEditing), "insert-text-symbolic");
    gtk_tool_item_set_tooltip_text(pEnableEditing, "Edit");
    gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pEnableEditing, -1);
    g_signal_connect(G_OBJECT(pEnableEditing), "toggled", G_CALLBACK(toggleEditing), nullptr);

    static bool bViewCallback = getenv("LOK_VIEW_CALLBACK");
    if (bViewCallback)
    {
        GtkToolItem* pNewViewButton = gtk_tool_button_new( nullptr, nullptr);
        gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pNewViewButton), "view-continuous-symbolic");
        gtk_tool_item_set_tooltip_text(pNewViewButton, "New View");
        gtk_toolbar_insert(GTK_TOOLBAR(pUpperToolbar), pNewViewButton, -1);
        g_signal_connect(G_OBJECT(pNewViewButton), "clicked", G_CALLBACK(createView), nullptr);
    }
    gtk_box_pack_start(GTK_BOX(rWindow.m_pVBox), pUpperToolbar, FALSE, FALSE, 0 ); // Adds to top.

    // Lower toolbar.
    GtkWidget* pLowerToolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(pLowerToolbar), GTK_TOOLBAR_ICONS);

    // Bold, italic, underline and strikethrough.
    rWindow.m_pBold = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pBold), "format-text-bold-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pBold, "Bold");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pBold, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pBold), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pBold, ".uno:Bold");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pBold), false);

    rWindow.m_pItalic = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (rWindow.m_pItalic), "format-text-italic-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pItalic, "Italic");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pItalic, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pItalic), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pItalic, ".uno:Italic");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pItalic), false);

    rWindow.m_pUnderline = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (rWindow.m_pUnderline), "format-text-underline-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pUnderline, "Underline");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pUnderline, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pUnderline), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pUnderline, ".uno:Underline");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pUnderline), false);

    rWindow.m_pStrikethrough = gtk_toggle_tool_button_new ();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pStrikethrough), "format-text-strikethrough-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pStrikethrough, "Strikethrough");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pStrikethrough, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pStrikethrough), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pStrikethrough, ".uno:Strikeout");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pStrikethrough), false);

    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), gtk_separator_tool_item_new(), -1);

    // Superscript and subscript.
    rWindow.m_pSuperscript = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pSuperscript), "go-up-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pSuperscript, "Superscript");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pSuperscript, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pSuperscript), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pSuperscript, ".uno:SuperScript");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pSuperscript), false);

    rWindow.m_pSubscript = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pSubscript), "go-down-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pSubscript, "Subscript");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pSubscript, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pSubscript), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pSubscript, ".uno:SubScript");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pSubscript), false);

    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), gtk_separator_tool_item_new(), -1);

    // Align left, center horizontally, align right and justified.
    rWindow.m_pLeftpara = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pLeftpara), "format-justify-left-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pLeftpara, "Align Left");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pLeftpara, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pLeftpara), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pLeftpara, ".uno:LeftPara");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pLeftpara), false);

    rWindow.m_pCenterpara = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pCenterpara), "format-justify-center-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pCenterpara, "Center Horizontally");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pCenterpara, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pCenterpara), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pCenterpara, ".uno:CenterPara");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pCenterpara), false);

    rWindow.m_pRightpara = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pRightpara), "format-justify-right-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pRightpara, "Align Right");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pRightpara, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pRightpara), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pRightpara, ".uno:RightPara");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pRightpara), false);

    rWindow.m_pJustifypara = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pJustifypara), "format-justify-fill-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pJustifypara, "Justified");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pJustifypara, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pJustifypara), "toggled", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pJustifypara, ".uno:JustifyPara");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pJustifypara), false);

    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), gtk_separator_tool_item_new(), -1);

    // Insert/delete comments.
    rWindow.m_pInsertAnnotation = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pInsertAnnotation), "changes-allow-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pInsertAnnotation, "Insert Comment");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pInsertAnnotation, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pInsertAnnotation), "clicked", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pInsertAnnotation, ".uno:InsertAnnotation");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pInsertAnnotation), false);

    rWindow.m_pDeleteComment = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(rWindow.m_pDeleteComment), "changes-prevent-symbolic");
    gtk_tool_item_set_tooltip_text(rWindow.m_pDeleteComment, "Delete Comment");
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), rWindow.m_pDeleteComment, -1);
    g_signal_connect(G_OBJECT(rWindow.m_pDeleteComment), "clicked", G_CALLBACK(toggleToolItem), nullptr);
    lcl_registerToolItem(rWindow, rWindow.m_pDeleteComment, ".uno:DeleteComment");
    gtk_widget_set_sensitive(GTK_WIDGET(rWindow.m_pDeleteComment), false);

    // Formula bar
    GtkToolItem* pFormulaEntryContainer = gtk_tool_item_new();
    rWindow.m_pFormulabarEntry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(pFormulaEntryContainer), rWindow.m_pFormulabarEntry);
    g_signal_connect(rWindow.m_pFormulabarEntry, "key-press-event", G_CALLBACK(signalFormulabar), 0);
    gtk_toolbar_insert(GTK_TOOLBAR(pLowerToolbar), pFormulaEntryContainer, -1);
    gtk_box_pack_start(GTK_BOX(rWindow.m_pVBox), pLowerToolbar, FALSE, FALSE, 0 ); // Adds to top.

    // Findbar
    rWindow.m_pFindbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(rWindow.m_pFindbar), GTK_TOOLBAR_ICONS);

    GtkToolItem* pFindbarClose = gtk_tool_button_new( nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarClose), "window-close-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarClose, -1);
    g_signal_connect(G_OBJECT(pFindbarClose), "clicked", G_CALLBACK(toggleFindbar), nullptr);

    GtkToolItem* pEntryContainer = gtk_tool_item_new();
    rWindow.m_pFindbarEntry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(pEntryContainer), rWindow.m_pFindbarEntry);
    g_signal_connect(rWindow.m_pFindbarEntry, "key-press-event", G_CALLBACK(signalFindbar), 0);
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pEntryContainer, -1);

    GtkToolItem* pFindbarNext = gtk_tool_button_new( nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarNext), "go-down-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarNext, -1);
    g_signal_connect(G_OBJECT(pFindbarNext), "clicked", G_CALLBACK(signalSearchNext), nullptr);

    GtkToolItem* pFindbarPrev = gtk_tool_button_new( nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON (pFindbarPrev), "go-up-symbolic");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarPrev, -1);
    g_signal_connect(G_OBJECT(pFindbarPrev), "clicked", G_CALLBACK(signalSearchPrev), nullptr);

    GtkToolItem* pFindAll = gtk_toggle_tool_button_new();
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(pFindAll), "Highlight All");
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindAll, -1);
    g_signal_connect(G_OBJECT(pFindAll), "toggled", G_CALLBACK(toggleFindAll), nullptr);

    GtkToolItem* pFindbarLabelContainer = gtk_tool_item_new();
    rWindow.m_pFindbarLabel = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(pFindbarLabelContainer), rWindow.m_pFindbarLabel);
    gtk_toolbar_insert(GTK_TOOLBAR(rWindow.m_pFindbar), pFindbarLabelContainer, -1);

    gtk_box_pack_end(GTK_BOX(rWindow.m_pVBox), rWindow.m_pFindbar, FALSE, FALSE, 0);

    // Grid for the row/column bar + doc view.
    GtkWidget* pGrid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(rWindow.m_pVBox), pGrid);
    rWindow.m_pCornerButton.reset(new TiledCornerButton());
    // "A1" cell of the grid.
    gtk_grid_attach(GTK_GRID(pGrid), rWindow.m_pCornerButton->m_pDrawingArea, 0, 0, 1, 1);
    rWindow.m_pRowBar.reset(new TiledRowColumnBar(TiledRowColumnBar::ROW));
    // "A2" cell of the grid.
    gtk_grid_attach(GTK_GRID(pGrid), rWindow.m_pRowBar->m_pDrawingArea, 0, 1, 1, 1);
    rWindow.m_pColumnBar.reset(new TiledRowColumnBar(TiledRowColumnBar::COLUMN));
    // "B1" cell of the grid.
    gtk_grid_attach(GTK_GRID(pGrid), rWindow.m_pColumnBar->m_pDrawingArea, 1, 0, 1, 1);

    // Scrolled window for DocView
    rWindow.m_pScrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_widget_set_hexpand(rWindow.m_pScrolledWindow, TRUE);
    gtk_widget_set_vexpand(rWindow.m_pScrolledWindow, TRUE);
    // "B2" cell of the grid
    gtk_grid_attach(GTK_GRID(pGrid), rWindow.m_pScrolledWindow, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(rWindow.m_pScrolledWindow), rWindow.m_pDocView);
    GtkAdjustment* pHAdjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    g_signal_connect(pHAdjustment, "value-changed", G_CALLBACK(TiledRowColumnBar::docAdjustmentChanged), rWindow.m_pDocView);
    GtkAdjustment* pVAdjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(rWindow.m_pScrolledWindow));
    g_signal_connect(pVAdjustment, "value-changed", G_CALLBACK(TiledRowColumnBar::docAdjustmentChanged), rWindow.m_pDocView);

    rWindow.m_pProgressBar = gtk_progress_bar_new ();
    g_signal_connect(rWindow.m_pDocView, "load-changed", G_CALLBACK(loadChanged), rWindow.m_pProgressBar);

    GtkWidget* pStatusBar = gtk_statusbar_new();
    rWindow.m_pStatusBar = pStatusBar;
    gtk_container_forall(GTK_CONTAINER(pStatusBar), removeChildrenFromStatusbar, pStatusBar);
    gtk_container_add (GTK_CONTAINER(rWindow.m_pVBox), pStatusBar);
    gtk_container_add (GTK_CONTAINER(pStatusBar), rWindow.m_pProgressBar);
    gtk_widget_set_hexpand(rWindow.m_pProgressBar, true);

    rWindow.m_pStatusbarLabel = gtk_label_new("");
    gtk_widget_set_hexpand(rWindow.m_pStatusbarLabel, TRUE);
    gtk_container_add(GTK_CONTAINER(pStatusBar), rWindow.m_pStatusbarLabel);

    rWindow.m_pZoomLabel = gtk_label_new("100%");
    gtk_container_add(GTK_CONTAINER(pStatusBar), rWindow.m_pZoomLabel);

    gtk_widget_show_all(pWindow);
    // Hide the findbar by default.
    gtk_widget_hide(rWindow.m_pFindbar);
    // Same for the row/column bar.
    gtk_widget_hide(rWindow.m_pCornerButton->m_pDrawingArea);
    gtk_widget_hide(rWindow.m_pRowBar->m_pDrawingArea);
    gtk_widget_hide(rWindow.m_pColumnBar->m_pDrawingArea);
    gtk_widget_hide(rWindow.m_pFormulabarEntry);
    // Hide the non-progressbar children of the status bar by default.
    gtk_widget_hide(rWindow.m_pStatusbarLabel);
    gtk_widget_hide(rWindow.m_pZoomLabel);

    g_aWindows[pWindow] = rWindow;
    g_signal_connect(rWindow.m_pDocView, "configure-event", G_CALLBACK(TiledRowColumnBar::docConfigureEvent), 0);
    return pWindow;
}

/// Common setup for DocView (regardless if it's just a new view or a document to be loaded).
static void setupDocView(GtkWidget* pDocView)
{
#if GLIB_CHECK_VERSION(2,40,0)
    g_assert_nonnull(pDocView);
#endif
    g_signal_connect(pDocView, "edit-changed", G_CALLBACK(signalEdit), NULL);
    g_signal_connect(pDocView, "command-changed", G_CALLBACK(signalCommand), NULL);
    g_signal_connect(pDocView, "command-result", G_CALLBACK(signalCommandResult), NULL);
    g_signal_connect(pDocView, "search-not-found", G_CALLBACK(signalSearch), NULL);
    g_signal_connect(pDocView, "search-result-count", G_CALLBACK(signalSearchResultCount), NULL);
    g_signal_connect(pDocView, "part-changed", G_CALLBACK(signalPart), NULL);
    g_signal_connect(pDocView, "size-changed", G_CALLBACK(signalSize), NULL);
    g_signal_connect(pDocView, "hyperlink-clicked", G_CALLBACK(signalHyperlink), NULL);
    g_signal_connect(pDocView, "cursor-changed", G_CALLBACK(cursorChanged), NULL);
    g_signal_connect(pDocView, "formula-changed", G_CALLBACK(formulaChanged), NULL);
    g_signal_connect(pDocView, "password-required", G_CALLBACK(passwordRequired), nullptr);
}

int main( int argc, char* argv[] )
{
    if( argc < 3 ||
        ( argc > 1 && ( !strcmp( argv[1], "--help" ) || !strcmp( argv[1], "-h" ) ) ) )
        return help();

    if ( argv[1][0] != '/' )
    {
        fprintf(stderr, "Absolute path required to libreoffice install\n");
        return 1;
    }

    gtk_init( &argc, &argv );

    std::vector<std::string> aArguments;
    for (int i = 3; i < argc; ++i)
        aArguments.push_back(argv[i]);
    createModelAndView(argv[1], argv[2], aArguments);

    gtk_main();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
