/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkhypertext.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>

#include <stdio.h>

using namespace ::com::sun::star;


// ---------------------- AtkHyperlink ----------------------

typedef struct {
    AtkHyperlink atk_hyper_link;

    uno::Reference< accessibility::XAccessibleHyperlink > xLink;
} HyperLink;

static uno::Reference< accessibility::XAccessibleHyperlink >
    getHyperlink( AtkHyperlink *pHyperlink )
{
    HyperLink *pLink = (HyperLink *) pHyperlink;
    return pLink->xLink;
}

static GObjectClass *hyper_parent_class = NULL;

extern "C" {

static void
hyper_link_finalize (GObject *obj)
{
    HyperLink *hl = (HyperLink *) obj;
    hl->xLink.clear();
    hyper_parent_class->finalize (obj);
}

static gchar *
hyper_link_get_uri( AtkHyperlink *pLink,
                    gint          i )
{
    try {
        uno::Any aAny = getHyperlink( pLink )->getAccessibleActionObject( i );
        rtl::OUString aUri = aAny.get< rtl::OUString > ();
        return OUStringToGChar(aUri);
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in hyper_link_get_uri" );
    }
    return NULL;
}

static AtkObject *
hyper_link_get_object( AtkHyperlink *pLink,
                       gint          i)
{
    try {
        uno::Any aAny = getHyperlink( pLink )->getAccessibleActionObject( i );
        uno::Reference< accessibility::XAccessible > xObj( aAny, uno::UNO_QUERY_THROW );
        return atk_object_wrapper_ref( xObj );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in hyper_link_get_object" );
    }
    return NULL;
}
static gint
hyper_link_get_end_index( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->getEndIndex();
    }
    catch(const uno::Exception& e) {
    }
    return -1;
}
static gint
hyper_link_get_start_index( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->getStartIndex();
    }
    catch(const uno::Exception& e) {
    }
    return -1;
}
static gboolean
hyper_link_is_valid( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->isValid();
    }
    catch(const uno::Exception& e) {
    }
    return FALSE;
}
static gint
hyper_link_get_n_anchors( AtkHyperlink *pLink )
{
    try {
        return getHyperlink( pLink )->getAccessibleActionCount();
    }
    catch(const uno::Exception& e) {
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

    hyper_parent_class = (GObjectClass *)g_type_class_peek_parent (klass);

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
hyper_link_get_type (void)
{
    static GType type = 0;

    if (!type) {
        static const GTypeInfo tinfo = {
            sizeof (AtkHyperlinkClass),
            NULL,               /* base init */
            NULL,               /* base finalize */
            (GClassInitFunc) hyper_link_class_init,
            NULL,               /* class finalize */
            NULL,               /* class data */
            sizeof (HyperLink), /* instance size */
            0,                  /* nb preallocs */
            NULL,               /* instance init */
            NULL                /* value table */
        };

        static const GInterfaceInfo atk_action_info = {
            (GInterfaceInitFunc) actionIfaceInit,
            (GInterfaceFinalizeFunc) NULL,
            NULL
        };

        type = g_type_register_static (ATK_TYPE_HYPERLINK,
                                       "OOoAtkObjHyperLink", &tinfo,
                                       (GTypeFlags)0);
        g_type_add_interface_static (type, ATK_TYPE_ACTION,
                                     &atk_action_info);
    }

    return type;
}

// ---------------------- AtkHyperText ----------------------

static accessibility::XAccessibleHypertext*
    getHypertext( AtkHypertext *pHypertext ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pHypertext );
    if( pWrap )
    {
        if( !pWrap->mpHypertext && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleHypertext::static_type(NULL) );
            pWrap->mpHypertext = reinterpret_cast< accessibility::XAccessibleHypertext * > (any.pReserved);
            pWrap->mpHypertext->acquire();
        }

        return pWrap->mpHypertext;
    }

    return NULL;
}


static AtkHyperlink *
hypertext_get_link( AtkHypertext *hypertext,
                    gint          link_index)
{
    try {
        accessibility::XAccessibleHypertext* pHypertext = getHypertext( hypertext );
        if( pHypertext )
        {
            HyperLink *pLink = (HyperLink *)g_object_new( hyper_link_get_type(), NULL );
            pLink->xLink = pHypertext->getHyperLink( link_index );
            if( !pLink->xLink.is() ) {
                g_object_unref( G_OBJECT( pLink ) );
                pLink = NULL;
            }
            return ATK_HYPERLINK( pLink );
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getHyperLink()" );
    }

    return NULL;
}

static gint
hypertext_get_n_links( AtkHypertext *hypertext )
{
    try {
        accessibility::XAccessibleHypertext* pHypertext = getHypertext( hypertext );
        if( pHypertext )
            return pHypertext->getHyperLinkCount();
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getHyperLinkCount()" );
    }

    return 0;
}

static gint
hypertext_get_link_index( AtkHypertext *hypertext,
                          gint          index)
{
    try {
        accessibility::XAccessibleHypertext* pHypertext = getHypertext( hypertext );
        if( pHypertext )
            return pHypertext->getHyperLinkIndex( index );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getHyperLinkIndex()" );
    }

    return 0;
}

} // extern "C"

void
hypertextIfaceInit (AtkHypertextIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_link = hypertext_get_link;
  iface->get_n_links = hypertext_get_n_links;
  iface->get_link_index = hypertext_get_link_index;
}
