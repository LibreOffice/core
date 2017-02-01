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

GtkSalObject::GtkSalObject( GtkSalFrame* pParent, bool bShow )
        : m_pSocket(nullptr)
        , m_pParent(pParent)
        , m_pRegion(nullptr)
{
    if( pParent )
    {
        // our plug window
        m_pSocket = gtk_grid_new();
        Show( bShow );
        // insert into container
        gtk_fixed_put( pParent->getFixedContainer(),
                       m_pSocket,
                       0, 0 );
        // realize so we can get a window id
        gtk_widget_realize( m_pSocket );

        // system data
        m_aSystemData.nSize         = sizeof( SystemEnvData );
        m_aSystemData.aWindow       = pParent->GetNativeWindowHandle(m_pSocket);
        m_aSystemData.aShellWindow  = reinterpret_cast<sal_IntPtr>(this);
        m_aSystemData.pSalFrame     = nullptr;
        m_aSystemData.pWidget       = m_pSocket;
        m_aSystemData.nScreen       = pParent->getXScreenNumber().getXScreen();
        m_aSystemData.pAppContext   = nullptr;
        m_aSystemData.pShellWidget  = GTK_WIDGET(pParent->getWindow());
        m_aSystemData.pToolkit      = "gtk3";
        GdkScreen* pScreen = gtk_window_get_screen(GTK_WINDOW(pParent->getWindow()));
        GdkVisual* pVisual = gdk_screen_get_system_visual(pScreen);
        m_aSystemData.nDepth = gdk_visual_get_depth(pVisual);

#if defined(GDK_WINDOWING_X11)
        GdkDisplay *pDisplay = GtkSalFrame::getGdkDisplay();
        if (GDK_IS_X11_DISPLAY(pDisplay))
        {
            m_aSystemData.pDisplay = gdk_x11_display_get_xdisplay(pDisplay);
            m_aSystemData.pVisual = gdk_x11_visual_get_xvisual(pVisual);
        }
#endif

        g_signal_connect( G_OBJECT(m_pSocket), "button-press-event", G_CALLBACK(signalButton), this );
        g_signal_connect( G_OBJECT(m_pSocket), "button-release-event", G_CALLBACK(signalButton), this );
        g_signal_connect( G_OBJECT(m_pSocket), "focus-in-event", G_CALLBACK(signalFocus), this );
        g_signal_connect( G_OBJECT(m_pSocket), "focus-out-event", G_CALLBACK(signalFocus), this );
        g_signal_connect( G_OBJECT(m_pSocket), "destroy", G_CALLBACK(signalDestroy), this );

        // #i59255# necessary due to sync effects with java child windows
        pParent->Flush();
    }
}

GtkSalObject::~GtkSalObject()
{
    if( m_pRegion )
    {
        cairo_region_destroy( m_pRegion );
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
        gdk_window_shape_combine_region( widget_get_window(m_pSocket), nullptr, 0, 0 );
}

void GtkSalObject::BeginSetClipRegion( sal_uLong )
{
    if( m_pRegion )
        cairo_region_destroy( m_pRegion );
    m_pRegion = cairo_region_create();
}

void GtkSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    GdkRectangle aRect;
    aRect.x         = nX;
    aRect.y         = nY;
    aRect.width     = nWidth;
    aRect.height    = nHeight;

    cairo_region_union_rectangle( m_pRegion, &aRect );
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
        m_pParent->nopaint_container_resize_children(GTK_CONTAINER(pContainer));
    }
}

void GtkSalObject::Show( bool bVisible )
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
    GtkSalObject* pThis = static_cast<GtkSalObject*>(object);

    if( pEvent->type == GDK_BUTTON_PRESS )
    {
        pThis->CallCallback( SalObjEvent::ToTop, nullptr );
    }

    return FALSE;
}

gboolean GtkSalObject::signalFocus( GtkWidget*, GdkEventFocus* pEvent, gpointer object )
{
    GtkSalObject* pThis = static_cast<GtkSalObject*>(object);

    pThis->CallCallback( pEvent->in ? SalObjEvent::GetFocus : SalObjEvent::LoseFocus, nullptr );

    return FALSE;
}

void GtkSalObject::signalDestroy( GtkWidget* pObj, gpointer object )
{
    GtkSalObject* pThis = static_cast<GtkSalObject*>(object);
    if( pObj == pThis->m_pSocket )
    {
        pThis->m_pSocket = nullptr;
    }
}

void GtkSalObject::SetForwardKey( bool bEnable )
{
    if( bEnable )
        gtk_widget_add_events( GTK_WIDGET( m_pSocket ), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE );
    else
        gtk_widget_set_events( GTK_WIDGET( m_pSocket ), ~(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE) & gtk_widget_get_events( GTK_WIDGET( m_pSocket ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
