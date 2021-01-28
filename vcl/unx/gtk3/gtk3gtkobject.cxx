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

#include <unx/gtk/gtkbackend.hxx>
#include <unx/gtk/gtkobject.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>

GtkSalObjectBase::GtkSalObjectBase(GtkSalFrame* pParent)
    : m_pSocket(nullptr)
    , m_pParent(pParent)
    , m_pRegion(nullptr)
{
    if (!m_pParent)
        return;
}

GtkSalObject::GtkSalObject(GtkSalFrame* pParent, bool bShow)
    : GtkSalObjectBase(pParent)
{
    if (!m_pParent)
        return;

    // our plug window
    m_pSocket = gtk_grid_new();
    Show( bShow );
    // insert into container
    gtk_fixed_put( pParent->getFixedContainer(),
                   m_pSocket,
                   0, 0 );

    Init();

    g_signal_connect( G_OBJECT(m_pSocket), "destroy", G_CALLBACK(signalDestroy), this );

    // #i59255# necessary due to sync effects with java child windows
    pParent->Flush();
}

void GtkSalObjectBase::Init()
{
    // realize so we can get a window id
    gtk_widget_realize( m_pSocket );

    // system data
    // tdf#139609 deliberately defer using m_pParent->GetNativeWindowHandle(m_pSocket)) to set m_aSystemData.aWindow
    // unless its explicitly needed
    m_aSystemData.aShellWindow  = reinterpret_cast<sal_IntPtr>(this);
    m_aSystemData.pSalFrame     = nullptr;
    m_aSystemData.pWidget       = m_pSocket;
    m_aSystemData.nScreen       = m_pParent->getXScreenNumber().getXScreen();
    m_aSystemData.toolkit       = SystemEnvData::Toolkit::Gtk;
    GdkScreen* pScreen = gtk_widget_get_screen(m_pParent->getWindow());
    GdkVisual* pVisual = gdk_screen_get_system_visual(pScreen);

#if defined(GDK_WINDOWING_X11)
    GdkDisplay *pDisplay = GtkSalFrame::getGdkDisplay();
    if (DLSYM_GDK_IS_X11_DISPLAY(pDisplay))
    {
        m_aSystemData.pDisplay = gdk_x11_display_get_xdisplay(pDisplay);
        m_aSystemData.pVisual = gdk_x11_visual_get_xvisual(pVisual);
        m_aSystemData.platform = SystemEnvData::Platform::Xcb;
    }
#endif
#if defined(GDK_WINDOWING_WAYLAND)
    if (DLSYM_GDK_IS_WAYLAND_DISPLAY(pDisplay))
    {
        m_aSystemData.pDisplay = gdk_wayland_display_get_wl_display(pDisplay);
        m_aSystemData.platform = SystemEnvData::Platform::Wayland;
    }
#endif

    g_signal_connect( G_OBJECT(m_pSocket), "button-press-event", G_CALLBACK(signalButton), this );
    g_signal_connect( G_OBJECT(m_pSocket), "button-release-event", G_CALLBACK(signalButton), this );
    g_signal_connect( G_OBJECT(m_pSocket), "focus-in-event", G_CALLBACK(signalFocus), this );
    g_signal_connect( G_OBJECT(m_pSocket), "focus-out-event", G_CALLBACK(signalFocus), this );
}

GtkSalObjectBase::~GtkSalObjectBase()
{
    if( m_pRegion )
    {
        cairo_region_destroy( m_pRegion );
    }
}

GtkSalObject::~GtkSalObject()
{
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
        gdk_window_shape_combine_region( gtk_widget_get_window(m_pSocket), nullptr, 0, 0 );
}

void GtkSalObjectBase::BeginSetClipRegion( sal_uInt32 )
{
    if (m_pRegion)
        cairo_region_destroy(m_pRegion);
    m_pRegion = cairo_region_create();
}

void GtkSalObjectBase::UnionClipRegion( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
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
        gdk_window_shape_combine_region( gtk_widget_get_window(m_pSocket), m_pRegion, 0, 0 );
}

void GtkSalObject::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight)
{
    if (m_pSocket)
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pSocket));
        gtk_fixed_move( pContainer, m_pSocket, nX, nY );
        gtk_widget_set_size_request( m_pSocket, nWidth, nHeight );
        m_pParent->nopaint_container_resize_children(GTK_CONTAINER(pContainer));
    }
}

void GtkSalObject::Reparent(SalFrame* pFrame)
{
    GtkSalFrame* pNewParent = static_cast<GtkSalFrame*>(pFrame);
    if (m_pSocket)
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pSocket));

        gint nX(0), nY(0);
        gtk_container_child_get(GTK_CONTAINER(pContainer), m_pSocket,
                "x", &nX,
                "y", &nY,
                nullptr);

        g_object_ref(m_pSocket);
        gtk_container_remove(GTK_CONTAINER(pContainer), m_pSocket);

        gtk_fixed_put(pNewParent->getFixedContainer(),
                      m_pSocket,
                      nX, nY);

        g_object_unref(m_pSocket);
    }
    m_pParent = pNewParent;
}

void GtkSalObject::Show( bool bVisible )
{
    if( m_pSocket )
    {
        if( bVisible )
            gtk_widget_show(m_pSocket);
        else
            gtk_widget_hide(m_pSocket);
    }
}

Size GtkSalObjectBase::GetOptimalSize() const
{
    if (m_pSocket)
    {
        bool bVisible = gtk_widget_get_visible(m_pSocket);
        if (!bVisible)
            gtk_widget_set_visible(m_pSocket, true);
        GtkRequisition size;
        gtk_widget_get_preferred_size(m_pSocket, nullptr, &size);
        if (!bVisible)
            gtk_widget_set_visible(m_pSocket, false);
        return Size(size.width, size.height);
    }
    return Size();
}

const SystemEnvData* GtkSalObjectBase::GetSystemData() const
{
    return &m_aSystemData;
}

gboolean GtkSalObjectBase::signalButton( GtkWidget*, GdkEventButton* pEvent, gpointer object )
{
    GtkSalObjectBase* pThis = static_cast<GtkSalObject*>(object);

    if( pEvent->type == GDK_BUTTON_PRESS )
    {
        pThis->CallCallback( SalObjEvent::ToTop );
    }

    return FALSE;
}

gboolean GtkSalObjectBase::signalFocus( GtkWidget*, GdkEventFocus* pEvent, gpointer object )
{
    GtkSalObjectBase* pThis = static_cast<GtkSalObject*>(object);

    pThis->CallCallback( pEvent->in ? SalObjEvent::GetFocus : SalObjEvent::LoseFocus );

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

void GtkSalObjectBase::SetForwardKey( bool bEnable )
{
    if( bEnable )
        gtk_widget_add_events( GTK_WIDGET( m_pSocket ), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );
    else
        gtk_widget_set_events( GTK_WIDGET( m_pSocket ), ~(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK) & gtk_widget_get_events( GTK_WIDGET( m_pSocket ) ) );
}

GtkSalObjectWidgetClip::GtkSalObjectWidgetClip(GtkSalFrame* pParent, bool bShow)
    : GtkSalObjectBase(pParent)
    , m_pScrolledWindow(nullptr)
{
    if( !pParent )
        return;

    m_pScrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_pScrolledWindow),
                                   GTK_POLICY_EXTERNAL, GTK_POLICY_EXTERNAL);
    g_signal_connect(m_pScrolledWindow, "scroll-event", G_CALLBACK(signalScroll), this);

    // insert into container
    gtk_fixed_put( pParent->getFixedContainer(),
                   m_pScrolledWindow,
                   0, 0 );

    // deliberately without adjustments to avoid gtk's auto adjustment on changing focus
    GtkWidget* pViewPort = gtk_viewport_new(nullptr, nullptr);

    // force in a fake background of a suitable color
    GtkStyleContext *pWidgetContext = gtk_widget_get_style_context(pViewPort);
    GtkCssProvider* pBgCssProvider = gtk_css_provider_new();
    OUString sColor = Application::GetSettings().GetStyleSettings().GetDialogColor().AsRGBHexString();
    OUString aBuffer = "* { background-color: #" + sColor + "; }";
    OString aResult = OUStringToOString(aBuffer, RTL_TEXTENCODING_UTF8);
    gtk_css_provider_load_from_data(pBgCssProvider, aResult.getStr(), aResult.getLength(), nullptr);
    gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(pBgCssProvider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_container_add(GTK_CONTAINER(m_pScrolledWindow), pViewPort);
    gtk_widget_show(pViewPort);

    // our plug window
    m_pSocket = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(pViewPort), m_pSocket);
    gtk_widget_show(m_pSocket);

    Show(bShow);

    Init();

    g_signal_connect( G_OBJECT(m_pSocket), "destroy", G_CALLBACK(signalDestroy), this );
}

GtkSalObjectWidgetClip::~GtkSalObjectWidgetClip()
{
    if( m_pSocket )
    {
        // remove socket from parent frame's fixed container
        gtk_container_remove( GTK_CONTAINER(gtk_widget_get_parent(m_pScrolledWindow)),
                              m_pScrolledWindow );
        // get rid of the socket
        // actually the gtk_container_remove should let the ref count
        // of the socket sink to 0 and destroy it (see signalDestroy)
        // this is just a sanity check
        if( m_pScrolledWindow )
            gtk_widget_destroy( m_pScrolledWindow );
    }
}

void GtkSalObjectWidgetClip::ResetClipRegion()
{
    m_aClipRect = tools::Rectangle();
    ApplyClipRegion();
}

void GtkSalObjectWidgetClip::EndSetClipRegion()
{
    int nRects = cairo_region_num_rectangles(m_pRegion);
    assert(nRects == 0 || nRects == 1);
    if (nRects == 0)
        m_aClipRect = tools::Rectangle();
    else
    {
        cairo_rectangle_int_t rectangle;
        cairo_region_get_rectangle(m_pRegion, 0, &rectangle);
        m_aClipRect = tools::Rectangle(Point(rectangle.x, rectangle.y), Size(rectangle.width, rectangle.height));
    }
    ApplyClipRegion();
}

void GtkSalObjectWidgetClip::ApplyClipRegion()
{
    if( !m_pSocket )
        return;

    GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pScrolledWindow));

    GtkAllocation allocation;
    allocation.x = m_aRect.Left() + m_aClipRect.Left();
    allocation.y = m_aRect.Top() + m_aClipRect.Top();
    if (m_aClipRect.IsEmpty())
    {
        allocation.width = m_aRect.GetWidth();
        allocation.height = m_aRect.GetHeight();
    }
    else
    {
        allocation.width = m_aClipRect.GetWidth();
        allocation.height = m_aClipRect.GetHeight();
    }

    if (AllSettings::GetLayoutRTL())
    {
        GtkAllocation aParentAllocation;
        gtk_widget_get_allocation(GTK_WIDGET(pContainer), &aParentAllocation);
        gtk_fixed_move(pContainer, m_pScrolledWindow, aParentAllocation.width - allocation.width - 1 - allocation.x, allocation.y);
    }
    else
        gtk_fixed_move(pContainer, m_pScrolledWindow, allocation.x, allocation.y);
    gtk_widget_set_size_request(m_pScrolledWindow, allocation.width, allocation.height);
    gtk_widget_size_allocate(m_pScrolledWindow, &allocation);

    gtk_adjustment_set_value(gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(m_pScrolledWindow)), m_aClipRect.Left());
    gtk_adjustment_set_value(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_pScrolledWindow)), m_aClipRect.Top());
}

void GtkSalObjectWidgetClip::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight)
{
    m_aRect = tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight));
    if (m_pSocket)
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pScrolledWindow));
        gtk_widget_set_size_request(m_pSocket, nWidth, nHeight);
        ApplyClipRegion();
        m_pParent->nopaint_container_resize_children(GTK_CONTAINER(pContainer));
    }
}

void GtkSalObjectWidgetClip::Reparent(SalFrame* pFrame)
{
    GtkSalFrame* pNewParent = static_cast<GtkSalFrame*>(pFrame);
    if (m_pSocket)
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pScrolledWindow));

        gint nX(0), nY(0);
        gtk_container_child_get(GTK_CONTAINER(pContainer), m_pScrolledWindow,
                "x", &nX,
                "y", &nY,
                nullptr);

        g_object_ref(m_pScrolledWindow);
        gtk_container_remove(GTK_CONTAINER(pContainer), m_pScrolledWindow);

        gtk_fixed_put(pNewParent->getFixedContainer(),
                      m_pScrolledWindow,
                      nX, nY);

        g_object_unref(m_pScrolledWindow);
    }
    m_pParent = pNewParent;
}

void GtkSalObjectWidgetClip::Show( bool bVisible )
{
    if (!m_pSocket)
        return;
    bool bCurrentVis = gtk_widget_get_visible(m_pScrolledWindow);
    if (bVisible == bCurrentVis)
        return;
    if( bVisible )
        gtk_widget_show(m_pScrolledWindow);
    else
    {
        // on hiding the widget, if a child has focus gtk will want to move the focus out of the widget
        // but we want to keep the focus where it is, e.g. writer's comments in margin feature put
        // cursor in a sidebar comment and scroll the page so the comment is invisible, we want the focus
        // to stay in the invisible widget, so its there when we scroll back or on a keypress the widget
        // gets the keystroke and scrolls back to make it visible again
        GtkWidget* pTopLevel = gtk_widget_get_toplevel(m_pScrolledWindow);
        GtkWidget* pOldFocus = GTK_IS_WINDOW(pTopLevel) ? gtk_window_get_focus(GTK_WINDOW(pTopLevel)) : nullptr;

        g_object_set_data(G_OBJECT(pTopLevel), "g-lo-BlockFocusChange", GINT_TO_POINTER(true) );

        gtk_widget_hide(m_pScrolledWindow);

        GtkWidget* pNewFocus = GTK_IS_WINDOW(pTopLevel) ? gtk_window_get_focus(GTK_WINDOW(pTopLevel)) : nullptr;
        if (pOldFocus && pOldFocus != pNewFocus)
            gtk_widget_grab_focus(pOldFocus);

        g_object_set_data(G_OBJECT(pTopLevel), "g-lo-BlockFocusChange", GINT_TO_POINTER(false) );
    }
}

void GtkSalObjectWidgetClip::signalDestroy( GtkWidget* pObj, gpointer object )
{
    GtkSalObjectWidgetClip* pThis = static_cast<GtkSalObjectWidgetClip*>(object);
    if( pObj == pThis->m_pSocket )
    {
        pThis->m_pSocket = nullptr;
        pThis->m_pScrolledWindow = nullptr;
    }
}

gboolean GtkSalObjectWidgetClip::signalScroll(GtkWidget* pScrolledWindow, GdkEvent* pEvent, gpointer object)
{
    GtkSalObjectWidgetClip* pThis = static_cast<GtkSalObjectWidgetClip*>(object);
    return pThis->signal_scroll(pScrolledWindow, pEvent);
}

// forward the wheel scroll events onto the main window instead
bool GtkSalObjectWidgetClip::signal_scroll(GtkWidget*, GdkEvent* pEvent)
{
    GtkWidget* pEventWidget = gtk_get_event_widget(pEvent);

    GtkWidget* pMouseEventWidget = m_pParent->getMouseEventWidget();

    gint dest_x, dest_y;
    gtk_widget_translate_coordinates(pEventWidget,
                                     pMouseEventWidget,
                                     pEvent->scroll.x,
                                     pEvent->scroll.y,
                                     &dest_x,
                                     &dest_y);
    pEvent->scroll.x = dest_x;
    pEvent->scroll.y = dest_y;

    GtkSalFrame::signalScroll(pMouseEventWidget, pEvent, m_pParent);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
