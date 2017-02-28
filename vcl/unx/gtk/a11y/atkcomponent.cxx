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

using namespace ::com::sun::star;

static css::uno::Reference<css::accessibility::XAccessibleComponent>
    getComponent( AtkComponent *pComponent ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pComponent );
    if( pWrap )
    {
        if( !pWrap->mpComponent.is() )
        {
            pWrap->mpComponent.set(pWrap->mpContext, css::uno::UNO_QUERY);
        }

        return pWrap->mpComponent;
    }

    return css::uno::Reference<css::accessibility::XAccessibleComponent>();
}

/*****************************************************************************/

static awt::Point
translatePoint( css::uno::Reference<accessibility::XAccessibleComponent> const & pComponent,
                gint x, gint y, AtkCoordType t)
{
    awt::Point aOrigin( 0, 0 );
    if( t == ATK_XY_SCREEN )
        aOrigin = pComponent->getLocationOnScreen();
    return awt::Point( x - aOrigin.X, y - aOrigin.Y );
}

/*****************************************************************************/

extern "C" {

static gboolean
component_wrapper_grab_focus (AtkComponent *component)
{
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent( component );
        if( pComponent.is() )
        {
            pComponent->grabFocus();
            return TRUE;
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
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent( component );
        if( pComponent.is() )
            return pComponent->containsPoint( translatePoint( pComponent, x, y, coord_type ) );
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
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent( component );

        if( pComponent.is() )
        {
            uno::Reference< accessibility::XAccessible > xAccessible;
            xAccessible = pComponent->getAccessibleAtPoint(
                translatePoint( pComponent, x, y, coord_type ) );
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
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent( component );
        if( pComponent.is() )
        {
            awt::Point aPos;

            if( coord_type == ATK_XY_SCREEN )
                aPos = pComponent->getLocationOnScreen();
            else
                aPos = pComponent->getLocation();

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
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleComponent> pComponent
            = getComponent( component );
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

    match_type = (GSignalMatchType) (G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC);
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
componentIfaceInit (AtkComponentIface *iface)
{
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
