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

#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>

using namespace ::com::sun::star;

// ---------------------- AtkHyperlink ----------------------

struct HyperLink {
    AtkHyperlink const atk_hyper_link;

    uno::Reference< accessibility::XAccessibleHyperlink > xLink;
};

static uno::Reference< accessibility::XAccessibleHyperlink > const &
    getHyperlink( AtkHyperlink *pHyperlink )
{
    HyperLink *pLink = reinterpret_cast<HyperLink *>(pHyperlink);
    return pLink->xLink;
}

static GObjectClass *hyper_parent_class = nullptr;

extern "C" {

static void
hyper_link_finalize (GObject *obj)
{
    HyperLink *hl = reinterpret_cast<HyperLink *>(obj);
    hl->xLink.clear();
    hyper_parent_class->finalize (obj);
}

static gchar *
hyper_link_get_uri( AtkHyperlink *pLink,
                    gint          i )
{
    try {
        uno::Any aAny = getHyperlink( pLink )->getAccessibleActionObject( i );
        OUString aUri = aAny.get< OUString > ();
        return OUStringToGChar(aUri);
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in hyper_link_get_uri" );
    }
    return nullptr;
}

static AtkObject *
hyper_link_get_object( AtkHyperlink *,
                       gint          )
{
    g_warning( "FIXME: hyper_link_get_object unimplemented" );
    return nullptr;
}
static gint
hyper_link_get_end_index( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->getEndIndex();
    }
    catch(const uno::Exception&) {
    }
    return -1;
}
static gint
hyper_link_get_start_index( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->getStartIndex();
    }
    catch(const uno::Exception&) {
    }
    return -1;
}
static gboolean
hyper_link_is_valid( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->isValid();
    }
    catch(const uno::Exception&) {
    }
    return FALSE;
}
static gint
hyper_link_get_n_anchors( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->getAccessibleActionCount();
    }
    catch(const uno::Exception&) {
    }
    return 0;
}

static guint
hyper_link_link_state( AtkHyperlink * )
{
    g_warning( "FIXME: hyper_link_link_state unimplemented" );
    return 0;
}
static gboolean
hyper_link_is_selected_link( AtkHyperlink * )
{
    g_warning( "FIXME: hyper_link_is_selected_link unimplemented" );
    return FALSE;
}

static void
hyper_link_class_init (AtkHyperlinkClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = hyper_link_finalize;

    hyper_parent_class = static_cast<GObjectClass *>(g_type_class_peek_parent (klass));

    klass->get_uri = hyper_link_get_uri;
    klass->get_object = hyper_link_get_object;
    klass->get_end_index = hyper_link_get_end_index;
    klass->get_start_index = hyper_link_get_start_index;
    klass->is_valid = hyper_link_is_valid;
    klass->get_n_anchors = hyper_link_get_n_anchors;
    klass->link_state = hyper_link_link_state;
    klass->is_selected_link = hyper_link_is_selected_link;
}

static GType
hyper_link_get_type()
{
    static GType type = 0;

    if (!type) {
        static const GTypeInfo tinfo = {
            sizeof (AtkHyperlinkClass),
            nullptr,               /* base init */
            nullptr,               /* base finalize */
            reinterpret_cast<GClassInitFunc>(hyper_link_class_init),
            nullptr,               /* class finalize */
            nullptr,               /* class data */
            sizeof (HyperLink), /* instance size */
            0,                  /* nb preallocs */
            nullptr,               /* instance init */
            nullptr                /* value table */
        };

        static const GInterfaceInfo atk_action_info = {
            reinterpret_cast<GInterfaceInitFunc>(actionIfaceInit),
            nullptr,
            nullptr
        };

        type = g_type_register_static (ATK_TYPE_HYPERLINK,
                                       "OOoAtkObjHyperLink", &tinfo,
                                       GTypeFlags(0));
        g_type_add_interface_static (type, ATK_TYPE_ACTION,
                                     &atk_action_info);
    }

    return type;
}

// ---------------------- AtkHyperText ----------------------

/// @throws uno::RuntimeException
static css::uno::Reference<css::accessibility::XAccessibleHypertext>
    getHypertext( AtkHypertext *pHypertext )
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pHypertext );
    if( pWrap )
    {
        if( !pWrap->mpHypertext.is() )
        {
            pWrap->mpHypertext.set(pWrap->mpContext, css::uno::UNO_QUERY);
        }

        return pWrap->mpHypertext;
    }

    return css::uno::Reference<css::accessibility::XAccessibleHypertext>();
}

static AtkHyperlink *
hypertext_get_link( AtkHypertext *hypertext,
                    gint          link_index)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleHypertext> pHypertext
            = getHypertext( hypertext );
        if( pHypertext.is() )
        {
            HyperLink *pLink = static_cast<HyperLink *>(g_object_new( hyper_link_get_type(), nullptr ));
            pLink->xLink = pHypertext->getHyperLink( link_index );
            if( !pLink->xLink.is() ) {
                g_object_unref( G_OBJECT( pLink ) );
                pLink = nullptr;
            }
            return ATK_HYPERLINK( pLink );
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getHyperLink()" );
    }

    return nullptr;
}

static gint
hypertext_get_n_links( AtkHypertext *hypertext )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleHypertext> pHypertext
            = getHypertext( hypertext );
        if( pHypertext.is() )
            return pHypertext->getHyperLinkCount();
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getHyperLinkCount()" );
    }

    return 0;
}

static gint
hypertext_get_link_index( AtkHypertext *hypertext,
                          gint          index)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleHypertext> pHypertext
            = getHypertext( hypertext );
        if( pHypertext.is() )
            return pHypertext->getHyperLinkIndex( index );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getHyperLinkIndex()" );
    }

    return 0;
}

} // extern "C"

void
hypertextIfaceInit (AtkHypertextIface *iface)
{
  g_return_if_fail (iface != nullptr);

  iface->get_link = hypertext_get_link;
  iface->get_n_links = hypertext_get_n_links;
  iface->get_link_index = hypertext_get_link_index;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
