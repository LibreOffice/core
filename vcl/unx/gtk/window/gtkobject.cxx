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


#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <unx/gtk/gtkobject.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>

GtkSalObject::GtkSalObject( GtkSalFrame* pParent, sal_Bool bShow )
        : m_pSocket( NULL ),
          m_pRegion( NULL )
{
    if( pParent )
    {
        // our plug window
        m_pSocket = gtk_drawing_area_new();
        Show( bShow );
        // insert into container
        gtk_fixed_put( pParent->getFixedContainer(),
                       m_pSocket,
                       0, 0 );
        // realize so we can get a window id
        gtk_widget_realize( m_pSocket );

        // make it transparent; some plugins may not insert
        // their own window here but use the socket window itself
        gtk_widget_set_app_paintable( m_pSocket, TRUE );

        // system data
        SalDisplay* pDisp = GetGenericData()->GetSalDisplay();
        m_aSystemData.nSize         = sizeof( SystemEnvData );
        m_aSystemData.pDisplay      = pDisp->GetDisplay();
        m_aSystemData.aWindow       = GDK_WINDOW_XWINDOW(widget_get_window(m_pSocket));
        m_aSystemData.pSalFrame     = NULL;
        m_aSystemData.pWidget       = m_pSocket;
        m_aSystemData.pVisual       = pDisp->GetVisual(pParent->getXScreenNumber()).GetVisual();
        m_aSystemData.nScreen       = pParent->getXScreenNumber().getXScreen();
        m_aSystemData.nDepth        = pDisp->GetVisual(pParent->getXScreenNumber()).GetDepth();
        m_aSystemData.aColormap     = pDisp->GetColormap(pParent->getXScreenNumber()).GetXColormap();
        m_aSystemData.pAppContext   = NULL;
        m_aSystemData.aShellWindow  = GDK_WINDOW_XWINDOW(widget_get_window(GTK_WIDGET(pParent->getWindow())));
        m_aSystemData.pShellWidget  = GTK_WIDGET(pParent->getWindow());

        g_signal_connect( G_OBJECT(m_pSocket), "button-press-event", G_CALLBACK(signalButton), this );
        g_signal_connect( G_OBJECT(m_pSocket), "button-release-event", G_CALLBACK(signalButton), this );
        g_signal_connect( G_OBJECT(m_pSocket), "focus-in-event", G_CALLBACK(signalFocus), this );
        g_signal_connect( G_OBJECT(m_pSocket), "focus-out-event", G_CALLBACK(signalFocus), this );
        g_signal_connect( G_OBJECT(m_pSocket), "destroy", G_CALLBACK(signalDestroy), this );

        // #i59255# necessary due to sync effects with java child windows
        pParent->Sync();
    }
}

GtkSalObject::~GtkSalObject()
{
    if( m_pRegion )
    {
#if GTK_CHECK_VERSION(3,0,0)
        cairo_region_destroy( m_pRegion );
#else
        gdk_region_destroy( m_pRegion );
#endif
    }
    if( m_pSocket )
    {
        // remove socket from parent frame's fixed container
        gtk_container_remove( GTK_CONTAINER(gtk_widget_get_parent(m_pSocket)),
                              m_pSocket );
        // get rid of the socket
        // actually the gtk_container_remove should let the ref count
        // of the socket sink to 0 and destroy it (see signalDestroy)
        // this is just a sanity check
        if( m_pSocket )
            gtk_widget_destroy( m_pSocket );
    }
}

void GtkSalObject::ResetClipRegion()
{
    if( m_pSocket )
        gdk_window_shape_combine_region( widget_get_window(m_pSocket), NULL, 0, 0 );
}

sal_uInt16 GtkSalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

void GtkSalObject::BeginSetClipRegion( sal_uLong )
{
#if GTK_CHECK_VERSION(3,0,0)
    if( m_pRegion )
        cairo_region_destroy( m_pRegion );
    m_pRegion = cairo_region_create();
#else
    if( m_pRegion )
        gdk_region_destroy( m_pRegion );
    m_pRegion = gdk_region_new();
#endif
}

void GtkSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    GdkRectangle aRect;
    aRect.x         = nX;
    aRect.y         = nY;
    aRect.width     = nWidth;
    aRect.height    = nHeight;

#if GTK_CHECK_VERSION(3,0,0)
    cairo_region_union_rectangle( m_pRegion, &aRect );
#else
    gdk_region_union_with_rect( m_pRegion, &aRect );
#endif
}

void GtkSalObject::EndSetClipRegion()
{
    if( m_pSocket )
        gdk_window_shape_combine_region( widget_get_window(m_pSocket), m_pRegion, 0, 0 );
}

void GtkSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    if( m_pSocket )
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pSocket));
        gtk_fixed_move( pContainer, m_pSocket, nX, nY );
        gtk_widget_set_size_request( m_pSocket, nWidth, nHeight );
        gtk_container_resize_children( GTK_CONTAINER(pContainer) );
    }
}

void GtkSalObject::Show( sal_Bool bVisible )
{
    if( m_pSocket )
    {
        if( bVisible )
            gtk_widget_show( m_pSocket );
        else
            gtk_widget_hide( m_pSocket );
    }
}

const SystemEnvData* GtkSalObject::GetSystemData() const
{
    return &m_aSystemData;
}

gboolean GtkSalObject::signalButton( GtkWidget*, GdkEventButton* pEvent, gpointer object )
{
    GtkSalObject* pThis = (GtkSalObject*)object;

    if( pEvent->type == GDK_BUTTON_PRESS )
    {
        pThis->CallCallback( SALOBJ_EVENT_TOTOP, NULL );
    }

    return FALSE;
}

gboolean GtkSalObject::signalFocus( GtkWidget*, GdkEventFocus* pEvent, gpointer object )
{
    GtkSalObject* pThis = (GtkSalObject*)object;

    pThis->CallCallback( pEvent->in ? SALOBJ_EVENT_GETFOCUS : SALOBJ_EVENT_LOSEFOCUS, NULL );

    return FALSE;
}

void GtkSalObject::signalDestroy( GtkWidget* pObj, gpointer object )
{
    GtkSalObject* pThis = (GtkSalObject*)object;
    if( pObj == pThis->m_pSocket )
    {
        pThis->m_pSocket = NULL;
    }
}

void GtkSalObject::SetForwardKey( sal_Bool bEnable )
{
    if( bEnable )
        gtk_widget_add_events( GTK_WIDGET( m_pSocket ), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE );
    else
        gtk_widget_set_events( GTK_WIDGET( m_pSocket ), ~(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE) & gtk_widget_get_events( GTK_WIDGET( m_pSocket ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
