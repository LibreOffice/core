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

#include "lokdocview_quad.h"

static void lok_docview_quad_class_init( LOKDocViewQuadClass* pClass );
static void lok_docview_quad_init( LOKDocViewQuad* pDocView );

// We specifically need to destroy the document when closing in order to ensure
// that lock files etc. are cleaned up.
void lcl_onDestroy( LOKDocViewQuad* pDocView, gpointer pData )
{
    (void) pData;
    if ( pDocView->pDocument )
        pDocView->pDocument->pClass->destroy( pDocView->pDocument );
    pDocView->pDocument = NULL;
}

SAL_DLLPUBLIC_EXPORT guint lok_docview_quad_get_type()
{
    static guint lok_docview_quad_type = 0;

    if (!lok_docview_quad_type)
    {
        GtkTypeInfo lok_docview_quad_info =
        {
            "LokDocViewQuad",
            sizeof( LOKDocViewQuad ),
            sizeof( LOKDocViewQuadClass ),
            (GtkClassInitFunc) lok_docview_quad_class_init,
            (GtkObjectInitFunc) lok_docview_quad_init,
            NULL,
            NULL,
            (GtkClassInitFunc) NULL
        };

        lok_docview_quad_type = gtk_type_unique( gtk_scrolled_window_get_type(), &lok_docview_quad_info );
    }
    return lok_docview_quad_type;
}

static void lok_docview_quad_class_init( LOKDocViewQuadClass* pClass )
{
    pClass->lok_docview_quad = NULL;
}

static void lok_docview_quad_init( LOKDocViewQuad* pDocView )
{
    int x, y;

    // Gtk ScrolledWindow is apparently not fully initialised yet, we specifically
    // have to set the [hv]adjustment to prevent GTK assertions from firing, see
    // https://bugzilla.gnome.org/show_bug.cgi?id=438114 for more info.
    gtk_scrolled_window_set_hadjustment( GTK_SCROLLED_WINDOW( pDocView ), NULL );
    gtk_scrolled_window_set_vadjustment( GTK_SCROLLED_WINDOW( pDocView ), NULL );

    pDocView->pEventBox = gtk_event_box_new();
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pDocView),
                                           pDocView->pEventBox );

    pDocView->pGrid = gtk_table_new( 2, 2, TRUE );
    gtk_container_add( GTK_CONTAINER( pDocView->pEventBox ), pDocView->pGrid );

    for ( x = 0; x < 2; x++ )
    {
        for ( y = 0; y < 2; y++ )
        {
            pDocView->pCanvas[x][y] = gtk_image_new();
            gtk_table_attach_defaults( GTK_TABLE( pDocView->pGrid ), pDocView->pCanvas[x][y], x, x+1, y, y+1 );
            //gtk_container_add( GTK_CONTAINER( pDocView->pEventBox ), pDocView->pCanvas );
            gtk_widget_show( pDocView->pCanvas[x][y] );

            pDocView->pPixBuf[x][y] = 0;
        }
    }

    gtk_widget_show( pDocView->pGrid );
    gtk_widget_show( pDocView->pEventBox );

    // TODO: figure out a clever view of getting paths set up.
    pDocView->pOffice = 0;
    pDocView->pDocument = 0;

    pDocView->fZoom = 1;

    gtk_signal_connect( GTK_OBJECT(pDocView), "destroy",
                        GTK_SIGNAL_FUNC(lcl_onDestroy), NULL );
}

SAL_DLLPUBLIC_EXPORT GtkWidget* lok_docview_quad_new( LibreOfficeKit* pOffice )
{
    LOKDocViewQuad* pDocView = gtk_type_new( lok_docview_quad_get_type() );
    pDocView->pOffice = pOffice;
    return GTK_WIDGET( pDocView );
}

void renderDocument( LOKDocViewQuad* pDocView )
{
    long nWidth, nHeight;
    int nRenderWidth, nRenderHeight;
    int x, y;
    GdkPixbuf* pTempBuf;

    g_assert( pDocView->pDocument );

    for ( x = 0; x < 2; x++ )
    {
        for ( y = 0; y < 2; y++ )
        {
            if ( pDocView->pPixBuf[x][y] )
            {
                g_object_unref( G_OBJECT( pDocView->pPixBuf[x][y] ) );
            }
        }
    }

    pDocView->pDocument->pClass->getDocumentSize( pDocView->pDocument, &nWidth, &nHeight );

    // Draw the whole document at once (for now)

    // TODO: we really should scale by screen DPI here -- 10 seems to be a vaguely
    // correct factor for my screen at least.
    nRenderWidth = nWidth * pDocView->fZoom / 10;
    nRenderHeight = nHeight * pDocView->fZoom / 10;

    // TOP-LEFT: standard
    // TOP-RIGHT: 2x resolution rendered (post-scaled to 50%)
    // BOTTOM-LEFT: 1/2 resolution rendered (post-scaled 200%)
    // BOTTOM-RIGHT: 1/2 resolution rendered (post-scaled 400%)
    pDocView->pPixBuf[0][0] = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
                                              TRUE, 8,
                                              nRenderWidth / 2, nRenderHeight / 2 );
    pDocView->pDocument->pClass->paintTile( pDocView->pDocument,
                                            gdk_pixbuf_get_pixels( pDocView->pPixBuf[0][0] ),
                                            nRenderWidth / 2, nRenderHeight / 2,
                                            0, 0, // origin
                                            nWidth / 2, nHeight / 2 );

    pDocView->pPixBuf[1][0] = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
                                              TRUE, 8,
                                              nRenderWidth, nRenderHeight );
    pDocView->pDocument->pClass->paintTile( pDocView->pDocument,
                                            gdk_pixbuf_get_pixels( pDocView->pPixBuf[1][0] ),
                                            nRenderWidth, nRenderHeight,
                                            nWidth / 2, 0,
                                            nWidth / 2, nHeight / 2 );
    pTempBuf = gdk_pixbuf_scale_simple( GDK_PIXBUF( pDocView->pPixBuf[1][0] ),
                                        nRenderWidth / 2,
                                        nRenderHeight / 2,
                                        GDK_INTERP_BILINEAR );
    g_object_unref( G_OBJECT( pDocView->pPixBuf[1][0] ) );
    pDocView->pPixBuf[1][0] = pTempBuf;


    pDocView->pPixBuf[0][1] = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
                                              TRUE, 8,
                                              nRenderWidth / 4, nRenderHeight / 4 );
    pDocView->pDocument->pClass->paintTile( pDocView->pDocument,
                                            gdk_pixbuf_get_pixels( pDocView->pPixBuf[0][1] ),
                                            nRenderWidth / 4, nRenderHeight / 4,
                                            0, nHeight / 2,
                                            nWidth / 2, nHeight / 2 );
    pTempBuf = gdk_pixbuf_scale_simple( GDK_PIXBUF( pDocView->pPixBuf[0][1] ),
                                        nRenderWidth / 2,
                                        nRenderHeight / 2,
                                        GDK_INTERP_BILINEAR );
    g_object_unref( G_OBJECT( pDocView->pPixBuf[0][1] ) );
    pDocView->pPixBuf[0][1] = pTempBuf;

    pDocView->pPixBuf[1][1] = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
                                              TRUE, 8,
                                              nRenderWidth / 8, nRenderHeight / 8 );
    pDocView->pDocument->pClass->paintTile( pDocView->pDocument,
                                            gdk_pixbuf_get_pixels( pDocView->pPixBuf[1][1] ),
                                            nRenderWidth / 8, nRenderHeight / 8,
                                            nWidth / 2, nHeight / 2,
                                            nWidth / 2, nHeight / 2 );
    pTempBuf = gdk_pixbuf_scale_simple( GDK_PIXBUF( pDocView->pPixBuf[1][1] ),
                                        nRenderWidth / 2,
                                        nRenderHeight / 2,
                                        GDK_INTERP_BILINEAR );
    g_object_unref( G_OBJECT( pDocView->pPixBuf[1][1] ) );
    pDocView->pPixBuf[1][1] = pTempBuf;



    //    gtk_image_set_from_pixbuf( GTK_IMAGE( pDocView->pCanvas ), pDocView->pPixBuf );
    for ( x = 0; x < 2; x++ )
    {
        for ( y = 0; y < 2; y++ )
        {
            gtk_image_set_from_pixbuf( GTK_IMAGE( pDocView->pCanvas[x][y] ), pDocView->pPixBuf[x][y] );
        }
    }
}

SAL_DLLPUBLIC_EXPORT gboolean lok_docview_quad_open_document( LOKDocViewQuad* pDocView, char* pPath )
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
        renderDocument( pDocView );

    return TRUE;
}

SAL_DLLPUBLIC_EXPORT void lok_docview_quad_set_zoom ( LOKDocViewQuad* pDocView, float fZoom )
{
    pDocView->fZoom = fZoom;
    if ( pDocView->pDocument )
    {
        renderDocument( pDocView );
    }
    // TODO: maybe remember and reset positiong?
}

SAL_DLLPUBLIC_EXPORT float lok_docview_quad_get_zoom ( LOKDocViewQuad* pDocView )
{
    return pDocView->fZoom;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
