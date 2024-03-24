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

#include "atkwrapper.hxx"
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <sal/log.hxx>
#include <gtk/gtk.h>

using namespace ::com::sun::star;

static AtkObjectWrapper* getObjectWrapper(AtkComponent *pComponent)
{
    AtkObjectWrapper *pWrap = nullptr;
    if (ATK_IS_OBJECT_WRAPPER(pComponent))
        pWrap = ATK_OBJECT_WRAPPER(pComponent);
    else if (GTK_IS_DRAWING_AREA(pComponent)) //when using a GtkDrawingArea as a custom widget in welded gtk3
    {
        GtkWidget* pDrawingArea = GTK_WIDGET(pComponent);
        AtkObject* pAtkObject = gtk_widget_get_accessible(pDrawingArea);
        pWrap = ATK_IS_OBJECT_WRAPPER(pAtkObject) ? ATK_OBJECT_WRAPPER(pAtkObject) : nullptr;
    }
    return pWrap;
}

/// @throws uno::RuntimeException
static css::uno::Reference<css::accessibility::XAccessibleComponent>
    getComponent(AtkObjectWrapper *pWrap)
{
    if (pWrap)
    {
        if (!pWrap->mpComponent.is())
            pWrap->mpComponent.set(pWrap->mpContext, css::uno::UNO_QUERY);
        return pWrap->mpComponent;
    }

    return css::uno::Reference<css::accessibility::XAccessibleComponent>();
}

static awt::Point
lcl_getLocationInWindow(AtkComponent* pAtkComponent,
                        css::uno::Reference<accessibility::XAccessibleComponent> const& xComponent)
{
    // calculate position in window by adding the component's position in the parent
    // to the parent's position in the window (unless parent is a window itself)
    awt::Point aPos = xComponent->getLocation();
    AtkObject* pParent = atk_object_get_parent(ATK_OBJECT(pAtkComponent));
    if (ATK_IS_COMPONENT(pParent) && pParent->role != AtkRole::ATK_ROLE_DIALOG
            && pParent->role != AtkRole::ATK_ROLE_FILE_CHOOSER
            && pParent->role != AtkRole::ATK_ROLE_FRAME
            && pParent->role != AtkRole::ATK_ROLE_WINDOW)
    {
        int nX;
        int nY;
        atk_component_get_extents(ATK_COMPONENT(pParent), &nX, &nY, nullptr, nullptr, ATK_XY_WINDOW);
        aPos.X += nX;
        aPos.Y += nY;
    }

    return aPos;
}

/*****************************************************************************/

static awt::Point
translatePoint( AtkComponent* pAtkComponent,
                css::uno::Reference<accessibility::XAccessibleComponent> const & pComponent,
                gint x, gint y, AtkCoordType t)
{
    awt::Point aOrigin( 0, 0 );
    if( t == ATK_XY_SCREEN )
        aOrigin = pComponent->getLocationOnScreen();
    else if (t == ATK_XY_WINDOW)
        aOrigin = lcl_getLocationInWindow(pAtkComponent, pComponent);
    return awt::Point( x - aOrigin.X, y - aOrigin.Y );
}

/*****************************************************************************/

extern "C" {

static gboolean
component_wrapper_grab_focus (AtkComponent *component)
{
    AtkObjectWrapper* obj = getObjectWrapper(component);
    //if we're a native GtkDrawingArea with custom a11y, use the default toolkit a11y
    if (obj && obj->mpOrig)
        return atk_component_grab_focus(ATK_COMPONENT(obj->mpOrig));

    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent(obj);
        if( pComponent.is() )
        {
            pComponent->grabFocus();
            return true;
        }
    }
    catch( const uno::Exception & )
    {
        g_warning( "Exception in grabFocus()" );
    }

    return FALSE;
}

/*****************************************************************************/

static gboolean
component_wrapper_contains (AtkComponent *component,
                            gint          x,
                            gint          y,
                            AtkCoordType  coord_type)
{
    AtkObjectWrapper* obj = getObjectWrapper(component);
    //if we're a native GtkDrawingArea with custom a11y, use the default toolkit a11y
    if (obj && obj->mpOrig)
        return atk_component_contains(ATK_COMPONENT(obj->mpOrig), x, y, coord_type);

    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent(obj);
        if( pComponent.is() )
            return pComponent->containsPoint(
                translatePoint(component, pComponent, x, y, coord_type));
    }
    catch( const uno::Exception & )
    {
        g_warning( "Exception in containsPoint()" );
    }

    return FALSE;
}

/*****************************************************************************/

static AtkObject *
component_wrapper_ref_accessible_at_point (AtkComponent *component,
                                           gint          x,
                                           gint          y,
                                           AtkCoordType  coord_type)
{
    AtkObjectWrapper* obj = getObjectWrapper(component);
    //if we're a native GtkDrawingArea with custom a11y, use the default toolkit a11y
    if (obj && obj->mpOrig)
        return atk_component_ref_accessible_at_point(ATK_COMPONENT(obj->mpOrig), x, y, coord_type);

    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent(obj);

        if( pComponent.is() )
        {
            uno::Reference< accessibility::XAccessible > xAccessible = pComponent->getAccessibleAtPoint(
                translatePoint(component, pComponent, x, y, coord_type));
            return atk_object_wrapper_ref( xAccessible );
        }
    }
    catch( const uno::Exception & )
    {
        g_warning( "Exception in getAccessibleAtPoint()" );
    }

    return nullptr;
}

/*****************************************************************************/

static void
component_wrapper_get_position (AtkComponent   *component,
                                gint           *x,
                                gint           *y,
                                AtkCoordType   coord_type)
{
    AtkObjectWrapper* obj = getObjectWrapper(component);
    //if we're a native GtkDrawingArea with custom a11y, use the default toolkit a11y
    if (obj && obj->mpOrig)
    {
        atk_component_get_extents(ATK_COMPONENT(obj->mpOrig), x, y, nullptr, nullptr, coord_type);
        return;
    }

    *x = *y = -1;

    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent(obj);
        if( pComponent.is() )
        {
            awt::Point aPos;

            if( coord_type == ATK_XY_SCREEN )
                aPos = pComponent->getLocationOnScreen();
            else if (coord_type == ATK_XY_WINDOW)
                aPos = lcl_getLocationInWindow(component, pComponent);
#if ATK_CHECK_VERSION(2, 30, 0)
            else if (coord_type == ATK_XY_PARENT)
#else
            // ATK_XY_PARENT added in ATK 2.30, so can't use the constant here
            else
#endif
                aPos = pComponent->getLocation();
#if ATK_CHECK_VERSION(2, 30, 0)
            else
            {
                SAL_WARN("vcl.gtk",
                         "component_wrapper_get_position called with unknown AtkCoordType "
                             << coord_type);
                return;
            }
#endif

            *x = aPos.X;
            *y = aPos.Y;
        }
    }
    catch( const uno::Exception & )
    {
        g_warning( "Exception in getLocation[OnScreen]()" );
    }
}

/*****************************************************************************/

static void
component_wrapper_get_size (AtkComponent   *component,
                            gint           *width,
                            gint           *height)
{
    AtkObjectWrapper* obj = getObjectWrapper(component);
    //if we're a native GtkDrawingArea with custom a11y, use the default toolkit a11y
    if (obj && obj->mpOrig)
    {
        atk_component_get_extents(ATK_COMPONENT(obj->mpOrig), nullptr, nullptr, width, height, ATK_XY_WINDOW);
        return;
    }

    *width = *height = -1;

    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent(obj);
        if( pComponent.is() )
        {
            awt::Size aSize = pComponent->getSize();
            *width = aSize.Width;
            *height = aSize.Height;
        }
    }
    catch( const uno::Exception & )
    {
        g_warning( "Exception in getSize()" );
    }
}

/*****************************************************************************/

static void
component_wrapper_get_extents (AtkComponent *component,
                               gint         *x,
                               gint         *y,
                               gint         *width,
                               gint         *height,
                               AtkCoordType  coord_type)
{
    component_wrapper_get_position( component, x, y, coord_type );
    component_wrapper_get_size( component, width, height );
}

/*****************************************************************************/

static gboolean
component_wrapper_set_extents (AtkComponent *, gint, gint, gint, gint, AtkCoordType)
{
    g_warning( "AtkComponent::set_extents unimplementable" );
    return FALSE;
}

/*****************************************************************************/

static gboolean
component_wrapper_set_position (AtkComponent *, gint, gint, AtkCoordType)
{
    g_warning( "AtkComponent::set_position unimplementable" );
    return FALSE;
}

/*****************************************************************************/

static gboolean
component_wrapper_set_size (AtkComponent *, gint, gint)
{
    g_warning( "AtkComponent::set_size unimplementable" );
    return FALSE;
}

/*****************************************************************************/

static AtkLayer
component_wrapper_get_layer (AtkComponent   *component)
{
    AtkRole role = atk_object_get_role( ATK_OBJECT( component ) );
    AtkLayer layer = ATK_LAYER_WIDGET;

    switch (role)
    {
        case ATK_ROLE_POPUP_MENU:
        case ATK_ROLE_MENU_ITEM:
        case ATK_ROLE_CHECK_MENU_ITEM:
        case ATK_ROLE_SEPARATOR:
        case ATK_ROLE_LIST_ITEM:
            layer = ATK_LAYER_POPUP;
            break;
        case ATK_ROLE_MENU:
            {
                AtkObject * parent = atk_object_get_parent( ATK_OBJECT( component ) );
                if( atk_object_get_role( parent ) != ATK_ROLE_MENU_BAR )
                    layer = ATK_LAYER_POPUP;
            }
            break;

        case ATK_ROLE_LIST:
            {
                AtkObject * parent = atk_object_get_parent( ATK_OBJECT( component ) );
                if( atk_object_get_role( parent ) == ATK_ROLE_COMBO_BOX )
                    layer = ATK_LAYER_POPUP;
            }
            break;

        default:
            ;
    }

    return layer;
}

/*****************************************************************************/

static gint
component_wrapper_get_mdi_zorder (AtkComponent   *)
{
    // only needed for ATK_LAYER_MDI (not used) or ATK_LAYER_WINDOW (inherited from GAIL)
    return G_MININT;
}

/*****************************************************************************/

// This code is mostly stolen from libgail ..

static guint
component_wrapper_add_focus_handler (AtkComponent    *component,
                                     AtkFocusHandler  handler)
{
    GSignalMatchType match_type;
    gulong ret;
    guint signal_id;

    match_type = GSignalMatchType(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC);
    signal_id = g_signal_lookup( "focus-event", ATK_TYPE_OBJECT );

    ret = g_signal_handler_find( component, match_type, signal_id, 0, nullptr,
                                 static_cast<gpointer>(&handler), nullptr);
    if (!ret)
    {
        return g_signal_connect_closure_by_id (component,
                                               signal_id, 0,
                                               g_cclosure_new (
                                               G_CALLBACK (handler), nullptr,
                                               nullptr),
                                               FALSE);
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************/

static void
component_wrapper_remove_focus_handler (AtkComponent  *component,
                                        guint         handler_id)
{
    g_signal_handler_disconnect (component, handler_id);
}

/*****************************************************************************/

} // extern "C"

void
componentIfaceInit (gpointer iface_, gpointer)
{
  auto const iface = static_cast<AtkComponentIface *>(iface_);
  g_return_if_fail (iface != nullptr);

  iface->add_focus_handler = component_wrapper_add_focus_handler;
  iface->contains = component_wrapper_contains;
  iface->get_extents = component_wrapper_get_extents;
  iface->get_layer = component_wrapper_get_layer;
  iface->get_mdi_zorder = component_wrapper_get_mdi_zorder;
  iface->get_position = component_wrapper_get_position;
  iface->get_size = component_wrapper_get_size;
  iface->grab_focus = component_wrapper_grab_focus;
  iface->ref_accessible_at_point = component_wrapper_ref_accessible_at_point;
  iface->remove_focus_handler = component_wrapper_remove_focus_handler;
  iface->set_extents = component_wrapper_set_extents;
  iface->set_position = component_wrapper_set_position;
  iface->set_size = component_wrapper_set_size;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
