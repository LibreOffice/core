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
#if !GTK_CHECK_VERSION(4, 0, 0)
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
#endif
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
    if( !m_pSocket )
        return;

#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_unparent(m_pSocket);
#else
    // remove socket from parent frame's fixed container
    gtk_container_remove( GTK_CONTAINER(gtk_widget_get_parent(m_pSocket)),
                          m_pSocket );
    // get rid of the socket
    // actually the gtk_container_remove should let the ref count
    // of the socket sink to 0 and destroy it (see signalDestroy)
    // this is just a sanity check
    if( m_pSocket )
        gtk_widget_destroy( m_pSocket );
#endif
}

void GtkSalObject::ResetClipRegion()
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    if( m_pSocket )
        gdk_window_shape_combine_region( widget_get_surface(m_pSocket), nullptr, 0, 0 );
#endif
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
#if !GTK_CHECK_VERSION(4, 0, 0)
    if( m_pSocket )
        gdk_window_shape_combine_region( widget_get_surface(m_pSocket), m_pRegion, 0, 0 );
#endif
}

void GtkSalObject::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight)
{
    if (m_pSocket)
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pSocket));
        gtk_fixed_move( pContainer, m_pSocket, nX, nY );
        gtk_widget_set_size_request( m_pSocket, nWidth, nHeight );
#if !GTK_CHECK_VERSION(4, 0, 0)
        m_pParent->nopaint_container_resize_children(GTK_CONTAINER(pContainer));
#endif
    }
}

void GtkSalObject::Reparent(SalFrame* pFrame)
{
    GtkSalFrame* pNewParent = static_cast<GtkSalFrame*>(pFrame);
    if (m_pSocket)
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pSocket));

#if !GTK_CHECK_VERSION(4, 0, 0)
        gint nX(0), nY(0);
        gtk_container_child_get(GTK_CONTAINER(pContainer), m_pSocket,
                "x", &nX,
                "y", &nY,
                nullptr);
#else
        double nX(0), nY(0);
        gtk_fixed_get_child_position(pContainer, m_pSocket, &nX, &nY);
#endif

        g_object_ref(m_pSocket);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_remove(GTK_CONTAINER(pContainer), m_pSocket);
#else
        gtk_fixed_remove(pContainer, m_pSocket);
#endif

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

        // Undo SetPosSize before getting its preferred size
        gint width(-1), height(-1);
        gtk_widget_get_size_request(m_pSocket, &width, &height);
        gtk_widget_set_size_request(m_pSocket, -1, -1);

        GtkRequisition size;
        gtk_widget_get_preferred_size(m_pSocket, nullptr, &size);

        // Restore SetPosSize size
        gtk_widget_set_size_request(m_pSocket, width, height);

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

#if !GTK_CHECK_VERSION(4, 0, 0)
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
#endif

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
#if !GTK_CHECK_VERSION(4, 0, 0)
    if( bEnable )
        gtk_widget_add_events( GTK_WIDGET( m_pSocket ), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK );
    else
        gtk_widget_set_events( GTK_WIDGET( m_pSocket ), ~(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK) & gtk_widget_get_events( GTK_WIDGET( m_pSocket ) ) );
#else
    (void)bEnable;
#endif
}

GtkSalObjectWidgetClip::GtkSalObjectWidgetClip(GtkSalFrame* pParent, bool bShow)
    : GtkSalObjectBase(pParent)
    , m_pScrolledWindow(nullptr)
{
    if( !pParent )
        return;

#if !GTK_CHECK_VERSION(4, 0, 0)
    m_pScrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);
    g_signal_connect(m_pScrolledWindow, "scroll-event", G_CALLBACK(signalScroll), this);
#else
    m_pScrolledWindow = gtk_scrolled_window_new();
    GtkEventController* pScrollController = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
    g_signal_connect(pScrollController, "scroll", G_CALLBACK(signalScroll), this);
    gtk_widget_add_controller(m_pScrolledWindow, pScrollController);
#endif
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_pScrolledWindow),
                                   GTK_POLICY_EXTERNAL, GTK_POLICY_EXTERNAL);

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
    css_provider_load_from_data(pBgCssProvider, aResult.getStr(), aResult.getLength());
    gtk_style_context_add_provider(pWidgetContext, GTK_STYLE_PROVIDER(pBgCssProvider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_container_add(GTK_CONTAINER(m_pScrolledWindow), pViewPort);
#else
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(m_pScrolledWindow), pViewPort);
#endif
    gtk_widget_show(pViewPort);

    // our plug window
    m_pSocket = gtk_grid_new();
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_container_add(GTK_CONTAINER(pViewPort), m_pSocket);
#else
    gtk_viewport_set_child(GTK_VIEWPORT(pViewPort), m_pSocket);
#endif
    gtk_widget_show(m_pSocket);

    Show(bShow);

    Init();

    g_signal_connect( G_OBJECT(m_pSocket), "destroy", G_CALLBACK(signalDestroy), this );
}

GtkSalObjectWidgetClip::~GtkSalObjectWidgetClip()
{
    if( !m_pSocket )
        return;

    // remove socket from parent frame's fixed container
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_container_remove( GTK_CONTAINER(gtk_widget_get_parent(m_pScrolledWindow)),
                          m_pScrolledWindow );

    // get rid of the socket
    // actually the gtk_container_remove should let the ref count
    // of the socket sink to 0 and destroy it (see signalDestroy)
    // this is just a sanity check
    if( m_pScrolledWindow )
        gtk_widget_destroy( m_pScrolledWindow );
#else
    gtk_fixed_remove(GTK_FIXED(gtk_widget_get_parent(m_pScrolledWindow)),
                     m_pScrolledWindow);
#endif
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
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_size_allocate(m_pScrolledWindow, &allocation);
#else
    gtk_widget_size_allocate(m_pScrolledWindow, &allocation, 0);
#endif

    gtk_adjustment_set_value(gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(m_pScrolledWindow)), m_aClipRect.Left());
    gtk_adjustment_set_value(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(m_pScrolledWindow)), m_aClipRect.Top());
}

void GtkSalObjectWidgetClip::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight)
{
    m_aRect = tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight));
    if (m_pSocket)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pScrolledWindow));
#endif
        gtk_widget_set_size_request(m_pSocket, nWidth, nHeight);
        ApplyClipRegion();
#if !GTK_CHECK_VERSION(4, 0, 0)
        m_pParent->nopaint_container_resize_children(GTK_CONTAINER(pContainer));
#endif
    }
}

void GtkSalObjectWidgetClip::Reparent(SalFrame* pFrame)
{
    GtkSalFrame* pNewParent = static_cast<GtkSalFrame*>(pFrame);
    if (m_pSocket)
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pScrolledWindow));

#if !GTK_CHECK_VERSION(4, 0, 0)
        gint nX(0), nY(0);
        gtk_container_child_get(GTK_CONTAINER(pContainer), m_pScrolledWindow,
                "x", &nX,
                "y", &nY,
                nullptr);
#else
        double nX(0), nY(0);
        gtk_fixed_get_child_position(pContainer, m_pScrolledWindow, &nX, &nY);
#endif

        g_object_ref(m_pScrolledWindow);
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_container_remove(GTK_CONTAINER(pContainer), m_pScrolledWindow);
#else
        gtk_fixed_remove(pContainer, m_pScrolledWindow);
#endif

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
        GtkWidget* pTopLevel = widget_get_toplevel(m_pScrolledWindow);
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

#if !GTK_CHECK_VERSION(4, 0, 0)
gboolean GtkSalObjectWidgetClip::signalScroll(GtkWidget* pScrolledWindow, GdkEvent* pEvent, gpointer object)
{
    GtkSalObjectWidgetClip* pThis = static_cast<GtkSalObjectWidgetClip*>(object);
    return pThis->signal_scroll(pScrolledWindow, pEvent);
}
#else
gboolean GtkSalObjectWidgetClip::signalScroll(GtkEventControllerScroll* pController, double delta_x, double delta_y, gpointer object)
{
    GtkSalObjectWidgetClip* pThis = static_cast<GtkSalObjectWidgetClip*>(object);
    return pThis->signal_scroll(pController, delta_x, delta_y);
}
#endif

// forward the wheel scroll events onto the main window instead
#if !GTK_CHECK_VERSION(4, 0, 0)
bool GtkSalObjectWidgetClip::signal_scroll(GtkWidget*, GdkEvent* pEvent)
{
    GtkWidget* pEventWidget = gtk_get_event_widget(pEvent);

    GtkWidget* pMouseEventWidget = m_pParent->getMouseEventWidget();

    gtk_coord dest_x, dest_y;
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
#else
bool GtkSalObjectWidgetClip::signal_scroll(GtkEventControllerScroll* pController, double delta_x, double delta_y)
{
    GtkWidget* pEventWidget = m_pScrolledWindow;

    GtkWidget* pMouseEventWidget = m_pParent->getMouseEventWidget();

    gtk_coord dest_x, dest_y;
    gtk_widget_translate_coordinates(pEventWidget,
                                     pMouseEventWidget,
                                     delta_x,
                                     delta_y,
                                     &dest_x,
                                     &dest_y);
    delta_x = dest_x;
    delta_y = dest_y;

    GtkSalFrame::signalScroll(pController, delta_x, delta_y, m_pParent);
    return true;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
