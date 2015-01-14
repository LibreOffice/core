/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>

#ifndef G_SOURCE_REMOVE
#define G_SOURCE_REMOVE FALSE
#endif

static void lok_docview_class_init( LOKDocViewClass* pClass );
static void lok_docview_init( LOKDocView* pDocView );

// We specifically need to destroy the document when closing in order to ensure
// that lock files etc. are cleaned up.
void lcl_onDestroy( LOKDocView* pDocView, gpointer pData )
{
    (void) pData;
    if ( pDocView->pDocument )
        pDocView->pDocument->pClass->destroy( pDocView->pDocument );
    pDocView->pDocument = NULL;
}

SAL_DLLPUBLIC_EXPORT guint lok_docview_get_type()
{
    static guint lok_docview_type = 0;

    if (!lok_docview_type)
    {
        GtkTypeInfo lok_docview_info =
        {
            "LokDocView",
            sizeof( LOKDocView ),
            sizeof( LOKDocViewClass ),
            (GtkClassInitFunc) lok_docview_class_init,
            (GtkObjectInitFunc) lok_docview_init,
            NULL,
            NULL,
            (GtkClassInitFunc) NULL
        };

        lok_docview_type = gtk_type_unique( gtk_scrolled_window_get_type(), &lok_docview_info );
    }
    return lok_docview_type;
}

static void lok_docview_class_init( LOKDocViewClass* pClass )
{
    (void)pClass;
}

static void lok_docview_init( LOKDocView* pDocView )
{
    // Gtk ScrolledWindow is apparently not fully initialised yet, we specifically
    // have to set the [hv]adjustment to prevent GTK assertions from firing, see
    // https://bugzilla.gnome.org/show_bug.cgi?id=438114 for more info.
    gtk_scrolled_window_set_hadjustment( GTK_SCROLLED_WINDOW( pDocView ), NULL );
    gtk_scrolled_window_set_vadjustment( GTK_SCROLLED_WINDOW( pDocView ), NULL );

    pDocView->pEventBox = gtk_event_box_new();
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pDocView),
                                           pDocView->pEventBox );

    // Allow reacting to button press events.
    gtk_widget_set_events(pDocView->pEventBox, GDK_BUTTON_PRESS_MASK);

    pDocView->pCanvas = gtk_image_new();
    gtk_container_add( GTK_CONTAINER( pDocView->pEventBox ), pDocView->pCanvas );

    gtk_widget_show( pDocView->pCanvas );
    gtk_widget_show( pDocView->pEventBox );

    pDocView->pPixBuf = 0;

    // TODO: figure out a clever view of getting paths set up.
    pDocView->pOffice = 0;
    pDocView->pDocument = 0;

    pDocView->fZoom = 1;
    pDocView->m_bEdit = FALSE;

    gtk_signal_connect( GTK_OBJECT(pDocView), "destroy",
                        GTK_SIGNAL_FUNC(lcl_onDestroy), NULL );
}

SAL_DLLPUBLIC_EXPORT GtkWidget* lok_docview_new( LibreOfficeKit* pOffice )
{
    LOKDocView* pDocView = gtk_type_new( lok_docview_get_type() );
    pDocView->pOffice = pOffice;
    return GTK_WIDGET( pDocView );
}

// We know that VirtualDevises use a DPI of 96.
static const int g_nDPI = 96;

/// Converts from document coordinates to screen pixels.
static float twipToPixel(float nInput)
{
    return nInput / 1440.0f * g_nDPI;
}

void renderDocument( LOKDocView* pDocView )
{
    long nDocumentWidthTwips, nDocumentHeightTwips, nBufferWidthPixels, nBufferHeightPixels;
    unsigned char* pBuffer;
    int nRowStride;

    g_assert( pDocView->pDocument );

    if ( pDocView->pPixBuf )
    {
        g_object_unref( G_OBJECT( pDocView->pPixBuf ) );
    }

    pDocView->pDocument->pClass->getDocumentSize(pDocView->pDocument, &nDocumentWidthTwips, &nDocumentHeightTwips);

    // Draw the whole document at once (for now)

    nBufferWidthPixels = twipToPixel(nDocumentWidthTwips) * pDocView->fZoom;
    nBufferHeightPixels = twipToPixel(nDocumentHeightTwips) * pDocView->fZoom;

    pDocView->pPixBuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
                                        TRUE, 8,
                                        nBufferWidthPixels, nBufferHeightPixels);


    pBuffer = gdk_pixbuf_get_pixels( pDocView->pPixBuf );
    pDocView->pDocument->pClass->paintTile( pDocView->pDocument,
                                            pBuffer,
                                            nBufferWidthPixels, nBufferHeightPixels,
                                            &nRowStride,
                                            0, 0, // origin
                                            nDocumentWidthTwips, nDocumentHeightTwips );
    (void) nRowStride;

    gtk_image_set_from_pixbuf( GTK_IMAGE( pDocView->pCanvas ), pDocView->pPixBuf );
}

/// Invoked on the main thread if lok_docview_callback_worker() requests so.
static gboolean lok_docview_callback(gpointer pData)
{
    LOKDocView* pDocView = pData;

#if ! GTK_CHECK_VERSION(2,12,0)
    GDK_THREADS_ENTER();
#endif

    renderDocument(pDocView);

#if ! GTK_CHECK_VERSION(2,12,0)
    GDK_THREADS_LEAVE();
#endif
    return G_SOURCE_REMOVE;
}

/// Our LOK callback, runs on the LO thread.
static void lok_docview_callback_worker(int nType, const char* pPayload, void* pData)
{
    LOKDocView* pDocView = pData;

    switch (nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
        // TODO for now just always render the document.
        (void)pPayload;
#if GTK_CHECK_VERSION(2,12,0)
        gdk_threads_add_idle(lok_docview_callback, pDocView);
#else
        g_idle_add(lok_docview_callback, pDocView);
#endif
        break;
    default:
        break;
    }
}

SAL_DLLPUBLIC_EXPORT gboolean lok_docview_open_document( LOKDocView* pDocView, char* pPath )
{
    if ( pDocView->pDocument )
    {
        pDocView->pDocument->pClass->destroy( pDocView->pDocument );
        pDocView->pDocument = NULL;
    }

    pDocView->pDocument = pDocView->pOffice->pClass->documentLoad( pDocView->pOffice,
                                                                   pPath );
    if ( !pDocView->pDocument )
    {
        // FIXME: should have a GError parameter and populate it.
        char *pError = pDocView->pOffice->pClass->getError( pDocView->pOffice );
        fprintf( stderr, "Error opening document '%s'\n", pError );
        return FALSE;
    }
    else
    {
        pDocView->pDocument->pClass->initializeForRendering(pDocView->pDocument);
        renderDocument( pDocView );
    }

    return TRUE;
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_zoom ( LOKDocView* pDocView, float fZoom )
{
    pDocView->fZoom = fZoom;

    if ( pDocView->pDocument )
    {
        renderDocument( pDocView );
    }
    // TODO: maybe remember and reset positiong?
}

SAL_DLLPUBLIC_EXPORT float lok_docview_get_zoom ( LOKDocView* pDocView )
{
    return pDocView->fZoom;
}

SAL_DLLPUBLIC_EXPORT int lok_docview_get_parts( LOKDocView* pDocView )
{
    return pDocView->pDocument->pClass->getParts( pDocView->pDocument );
}

SAL_DLLPUBLIC_EXPORT int lok_docview_get_part( LOKDocView* pDocView )
{
    return pDocView->pDocument->pClass->getPart( pDocView->pDocument );
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_part( LOKDocView* pDocView, int nPart)
{
    pDocView->pDocument->pClass->setPart( pDocView->pDocument, nPart );
    renderDocument( pDocView );
}

SAL_DLLPUBLIC_EXPORT char* lok_docview_get_part_name( LOKDocView* pDocView, int nPart )
{
    return pDocView->pDocument->pClass->getPartName( pDocView->pDocument, nPart );
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_partmode( LOKDocView* pDocView,
                                                    LibreOfficeKitPartMode ePartMode )
{
    pDocView->pDocument->pClass->setPartMode( pDocView->pDocument, ePartMode );
    renderDocument( pDocView );
}

SAL_DLLPUBLIC_EXPORT void lok_docview_set_edit( LOKDocView* pDocView,
                                                gboolean bEdit )
{
    if (!pDocView->m_bEdit && bEdit)
        pDocView->pDocument->pClass->registerCallback(pDocView->pDocument, &lok_docview_callback_worker, pDocView);
    pDocView->m_bEdit = bEdit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
