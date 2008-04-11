/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkselection.cxx,v $
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

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

#include <stdio.h>

using namespace ::com::sun::star;

static accessibility::XAccessibleSelection*
    getSelection( AtkSelection *pSelection ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pSelection );
    if( pWrap )
    {
        if( !pWrap->mpSelection && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleSelection::static_type(NULL) );
            pWrap->mpSelection = reinterpret_cast< accessibility::XAccessibleSelection * > (any.pReserved);
            pWrap->mpSelection->acquire();
        }

        return pWrap->mpSelection;
    }

    return NULL;
}

extern "C" {

static gboolean
selection_add_selection( AtkSelection *selection,
                         gint          i )
{
    try {
        accessibility::XAccessibleSelection* pSelection = getSelection( selection );
        if( pSelection )
        {
            pSelection->selectAccessibleChild( i );
            return TRUE;
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in selectAccessibleChild()" );
    }

    return FALSE;
}

static gboolean
selection_clear_selection( AtkSelection *selection )
{
    try {
        accessibility::XAccessibleSelection* pSelection = getSelection( selection );
        if( pSelection )
        {
            pSelection->clearAccessibleSelection();
            return TRUE;
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in selectAccessibleChild()" );
    }

    return FALSE;
}

static AtkObject*
selection_ref_selection( AtkSelection *selection,
                         gint          i )
{
    try {
        accessibility::XAccessibleSelection* pSelection = getSelection( selection );
        if( pSelection )
            return atk_object_wrapper_ref( pSelection->getSelectedAccessibleChild( i ) );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getSelectedAccessibleChild()" );
    }

    return NULL;
}

static gint
selection_get_selection_count( AtkSelection   *selection)
{
    try {
        accessibility::XAccessibleSelection* pSelection = getSelection( selection );
        if( pSelection )
            return pSelection->getSelectedAccessibleChildCount();
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getSelectedAccessibleChildCount()" );
    }

    return -1;
}

static gboolean
selection_is_child_selected( AtkSelection   *selection,
                              gint           i)
{
    try {
        accessibility::XAccessibleSelection* pSelection = getSelection( selection );
        if( pSelection )
            return pSelection->isAccessibleChildSelected( i );
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getSelectedAccessibleChildCount()" );
    }

    return FALSE;
}

static gboolean
selection_remove_selection( AtkSelection *selection,
                            gint           i )
{
    try {
        accessibility::XAccessibleSelection* pSelection = getSelection( selection );
        if( pSelection )
        {
            pSelection->deselectAccessibleChild( i );
            return TRUE;
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getSelectedAccessibleChildCount()" );
    }

    return FALSE;
}

static gboolean
selection_select_all_selection( AtkSelection   *selection)
{
    try {
        accessibility::XAccessibleSelection* pSelection = getSelection( selection );
        if( pSelection )
        {
            pSelection->selectAllAccessibleChildren();
            return TRUE;
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getSelectedAccessibleChildCount()" );
    }

    return FALSE;
}

} // extern "C"

void
selectionIfaceInit( AtkSelectionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->add_selection = selection_add_selection;
  iface->clear_selection = selection_clear_selection;
  iface->ref_selection = selection_ref_selection;
  iface->get_selection_count = selection_get_selection_count;
  iface->is_child_selected = selection_is_child_selected;
  iface->remove_selection = selection_remove_selection;
  iface->select_all_selection = selection_select_all_selection;
}
