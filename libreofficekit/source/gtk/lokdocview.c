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

    pDocView->pCanvas = gtk_image_new();
    gtk_container_add( GTK_CONTAINER( pDocView->pEventBox ), pDocView->pCanvas );

    gtk_widget_show( pDocView->pCanvas );
    gtk_widget_show( pDocView->pEventBox );

    pDocView->pPixBuf = 0;

    // TODO: figure out a clever view of getting paths set up.
    pDocView->pOffice = 0;
    pDocView->pDocument = 0;

    pDocView->fZoom = 1;

    gtk_signal_connect( GTK_OBJECT(pDocView), "destroy",
                        GTK_SIGNAL_FUNC(lcl_onDestroy), NULL );
}

SAL_DLLPUBLIC_EXPORT GtkWidget* lok_docview_new( LibreOfficeKit* pOffice )
{
    LOKDocView* pDocView = gtk_type_new( lok_docview_get_type() );
    pDocView->pOffice = pOffice;
    return GTK_WIDGET( pDocView );
}

void renderDocument( LOKDocView* pDocView )
{
    long nWidth, nHeight;
    int nRenderWidth, nRenderHeight;
    unsigned char* pBuffer;
    int nRowStride;
    // TODO: we really should scale by screen DPI here -- 10 seems to be a vaguely
    // correct factor for my screen at least.
    const float fScaleFactor = 0.1;

    // Various things blow up if we try to draw too large a tile,
    // this size seems to be safe. (Very rare/unlikely that
    const int nMaxWidth = 100000;

    g_assert( pDocView->pDocument );

    if ( pDocView->pPixBuf )
    {
        g_object_unref( G_OBJECT( pDocView->pPixBuf ) );
    }

    pDocView->pDocument->pClass->getDocumentSize( pDocView->pDocument, &nWidth, &nHeight );

    if ( nWidth * fScaleFactor > nMaxWidth )
    {
        nWidth = nMaxWidth;
    }
    if ( nHeight * fScaleFactor > nMaxWidth )
    {
        nHeight = nMaxWidth;
    }

    // Draw the whole document at once (for now)

    nRenderWidth = nWidth * pDocView->fZoom * fScaleFactor;
    nRenderHeight = nHeight * pDocView->fZoom * fScaleFactor;

    pDocView->pPixBuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
                                        TRUE, 8,
                                        nRenderWidth, nRenderHeight);


    pBuffer = gdk_pixbuf_get_pixels( pDocView->pPixBuf );
    pDocView->pDocument->pClass->paintTile( pDocView->pDocument,
                                            pBuffer,
                                            nRenderWidth, nRenderHeight,
                                            &nRowStride,
                                            0, 0, // origin
                                            nWidth, nHeight );
    // TODO: double check that the rowstride really matches what we expected,
    // although presumably we'd already be crashing by now if things were
    // wrong.
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
        pDocView->pDocument->pClass->registerCallback(pDocView->pDocument, &lok_docview_callback_worker, pDocView);
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
