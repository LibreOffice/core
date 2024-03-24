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

#include <sal/config.h>

#include <string_view>

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;

static AtkObject *
atk_object_wrapper_conditional_ref( const uno::Reference< accessibility::XAccessible >& rxAccessible )
{
    if( rxAccessible.is() )
        return atk_object_wrapper_ref( rxAccessible );

    return nullptr;
}

/*****************************************************************************/

// FIXME
static const gchar *
getAsConst( std::u16string_view rString )
{
    static const int nMax = 10;
    static OString aUgly[nMax];
    static int nIdx = 0;
    nIdx = (nIdx + 1) % nMax;
    aUgly[nIdx] = OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return aUgly[ nIdx ].getStr();
}

/*****************************************************************************/

/// @throws uno::RuntimeException
static css::uno::Reference<css::accessibility::XAccessibleTable>
    getTable( AtkTable *pTable )
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pTable );
    if( pWrap )
    {
        if( !pWrap->mpTable.is() )
        {
            pWrap->mpTable.set(pWrap->mpContext, css::uno::UNO_QUERY);
        }

        return pWrap->mpTable;
    }

    return css::uno::Reference<css::accessibility::XAccessibleTable>();
}

static css::uno::Reference<css::accessibility::XAccessibleTableSelection>
    getTableSelection(AtkTable *pTable)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER(pTable);
    if (pWrap)
    {
        if (!pWrap->mpTableSelection.is())
        {
            pWrap->mpTableSelection.set(pWrap->mpContext, css::uno::UNO_QUERY);
        }

        return pWrap->mpTableSelection;
    }

    return css::uno::Reference<css::accessibility::XAccessibleTableSelection>();
}

/*****************************************************************************/

extern "C" {

static AtkObject*
table_wrapper_ref_at (AtkTable *table,
                      gint      row,
                      gint      column)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable = getTable( table );
        if( pTable.is() )
            return atk_object_wrapper_conditional_ref( pTable->getAccessibleCellAt( row, column ) );
    }

    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleCellAt()" );
    }

    return nullptr;
}

/*****************************************************************************/

static gint
table_wrapper_get_index_at (AtkTable      *table,
                            gint          row,
                            gint          column)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
        {
            sal_Int64 nIndex = pTable->getAccessibleIndex( row, column );
            if (nIndex > std::numeric_limits<gint>::max())
            {
                // use -2 when the child index is too large to fit into 32 bit to neither use the
                // valid index of another cell nor -1, which might easily be interpreted as the cell
                // not/no longer being valid
                SAL_WARN("vcl.gtk", "table_wrapper_get_index_at: Child index exceeds maximum gint value, "
                                    "returning -2.");
                nIndex = -2;
            }
            return nIndex;
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleIndex()" );
    }

    return -1;
}

/*****************************************************************************/

static gint
table_wrapper_get_column_at_index (AtkTable      *table,
                                   gint          nIndex)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->getAccessibleColumn( nIndex );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleColumn()" );
    }

    return -1;
}

/*****************************************************************************/

static gint
table_wrapper_get_row_at_index( AtkTable *table,
                                gint      nIndex )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->getAccessibleRow( nIndex );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleRow()" );
    }

    return -1;
}

/*****************************************************************************/

static gint
table_wrapper_get_n_columns( AtkTable *table )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->getAccessibleColumnCount();
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleColumnCount()" );
    }

    return -1;
}

/*****************************************************************************/

static gint
table_wrapper_get_n_rows( AtkTable *table )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->getAccessibleRowCount();
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleRowCount()" );
    }

    return -1;
}

/*****************************************************************************/

static gint
table_wrapper_get_column_extent_at( AtkTable *table,
                                    gint      row,
                                    gint      column )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->getAccessibleColumnExtentAt( row, column );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleColumnExtentAt()" );
    }

    return -1;
}

/*****************************************************************************/

static gint
table_wrapper_get_row_extent_at( AtkTable *table,
                                 gint      row,
                                 gint      column )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->getAccessibleRowExtentAt( row, column );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleRowExtentAt()" );
    }

    return -1;
}

/*****************************************************************************/

static AtkObject *
table_wrapper_get_caption( AtkTable *table )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return atk_object_wrapper_conditional_ref( pTable->getAccessibleCaption() );
    }

    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleCaption()" );
    }

    return nullptr;
}

/*****************************************************************************/

static const gchar *
table_wrapper_get_row_description( AtkTable *table,
                                   gint      row )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return getAsConst( pTable->getAccessibleRowDescription( row ) );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleRowDescription()" );
    }

    return nullptr;
}

/*****************************************************************************/

static const gchar *
table_wrapper_get_column_description( AtkTable *table,
                                      gint      column )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return getAsConst( pTable->getAccessibleColumnDescription( column ) );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleColumnDescription()" );
    }

    return nullptr;
}

/*****************************************************************************/

static AtkObject *
table_wrapper_get_row_header( AtkTable *table,
                              gint      row )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
        {
            uno::Reference< accessibility::XAccessibleTable > xRowHeaders( pTable->getAccessibleRowHeaders() );
            if( xRowHeaders.is() )
                return atk_object_wrapper_conditional_ref( xRowHeaders->getAccessibleCellAt( row, 0 ) );
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleRowHeaders()" );
    }

    return nullptr;
}

/*****************************************************************************/

static AtkObject *
table_wrapper_get_column_header( AtkTable *table,
                                 gint      column )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
        {
            uno::Reference< accessibility::XAccessibleTable > xColumnHeaders( pTable->getAccessibleColumnHeaders() );
            if( xColumnHeaders.is() )
                return atk_object_wrapper_conditional_ref( xColumnHeaders->getAccessibleCellAt( 0, column ) );
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleColumnHeaders()" );
    }

    return nullptr;
}

/*****************************************************************************/

static AtkObject *
table_wrapper_get_summary( AtkTable *table )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
        {
            return atk_object_wrapper_conditional_ref( pTable->getAccessibleSummary() );
        }
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getAccessibleSummary()" );
    }

    return nullptr;
}

/*****************************************************************************/

static gint
convertToGIntArray( const uno::Sequence< ::sal_Int32 >& aSequence, gint **pSelected )
{
    if( aSequence.hasElements() )
    {
        *pSelected = g_new( gint, aSequence.getLength() );

        *pSelected = comphelper::sequenceToArray(*pSelected, aSequence);
    }

    return aSequence.getLength();
}

/*****************************************************************************/

static gint
table_wrapper_get_selected_columns( AtkTable      *table,
                                    gint          **pSelected )
{
    *pSelected = nullptr;
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return convertToGIntArray( pTable->getSelectedAccessibleColumns(), pSelected );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getSelectedAccessibleColumns()" );
    }

    return 0;
}

/*****************************************************************************/

static gint
table_wrapper_get_selected_rows( AtkTable      *table,
                                 gint          **pSelected )
{
    *pSelected = nullptr;
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return convertToGIntArray( pTable->getSelectedAccessibleRows(), pSelected );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in getSelectedAccessibleRows()" );
    }

    return 0;
}

/*****************************************************************************/

static gboolean
table_wrapper_is_column_selected( AtkTable      *table,
                                  gint          column )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->isAccessibleColumnSelected( column );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in isAccessibleColumnSelected()" );
    }

    return 0;
}

/*****************************************************************************/

static gboolean
table_wrapper_is_row_selected( AtkTable      *table,
                               gint          row )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->isAccessibleRowSelected( row );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in isAccessibleRowSelected()" );
    }

    return FALSE;
}

/*****************************************************************************/

static gboolean
table_wrapper_is_selected( AtkTable      *table,
                           gint          row,
                           gint          column )
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTable> pTable
            = getTable( table );
        if( pTable.is() )
            return pTable->isAccessibleSelected( row, column );
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in isAccessibleSelected()" );
    }

    return FALSE;
}

/*****************************************************************************/

static gboolean
table_wrapper_add_row_selection(AtkTable *pTable, gint row)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTableSelection> xTableSelection = getTableSelection(pTable);
        if (xTableSelection.is())
            return xTableSelection->selectRow(row);
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in selectRow()" );
    }

    return false;
}

/*****************************************************************************/

static gboolean
table_wrapper_remove_row_selection(AtkTable *pTable, gint row)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTableSelection> xTableSelection = getTableSelection(pTable);
        if (xTableSelection.is())
            return xTableSelection->unselectRow(row);
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in unselectRow()" );
    }

    return false;
}

/*****************************************************************************/

static gboolean
table_wrapper_add_column_selection(AtkTable *pTable, gint column)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTableSelection> xTableSelection = getTableSelection(pTable);
        if (xTableSelection.is())
            return xTableSelection->selectColumn(column);
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in selectColumn()" );
    }

    return false;
}

/*****************************************************************************/

static gboolean
table_wrapper_remove_column_selection(AtkTable *pTable, gint column)
{
    try {
        css::uno::Reference<css::accessibility::XAccessibleTableSelection> xTableSelection = getTableSelection(pTable);
        if (xTableSelection.is())
            return xTableSelection->unselectColumn(column);
    }
    catch(const uno::Exception&) {
        g_warning( "Exception in unselectColumn()" );
    }

    return false;
}

/*****************************************************************************/

static void
table_wrapper_set_caption( AtkTable *, AtkObject * )
{ // meaningless helper
}

/*****************************************************************************/

static void
table_wrapper_set_column_description( AtkTable *, gint, const gchar * )
{ // meaningless helper
}

/*****************************************************************************/

static void
table_wrapper_set_column_header( AtkTable *, gint, AtkObject * )
{ // meaningless helper
}

/*****************************************************************************/

static void
table_wrapper_set_row_description( AtkTable *, gint, const gchar * )
{ // meaningless helper
}

/*****************************************************************************/

static void
table_wrapper_set_row_header( AtkTable *, gint, AtkObject * )
{ // meaningless helper
}

/*****************************************************************************/

static void
table_wrapper_set_summary( AtkTable *, AtkObject * )
{ // meaningless helper
}

/*****************************************************************************/

} // extern "C"

void
tableIfaceInit (gpointer iface_, gpointer)
{
  auto const iface = static_cast<AtkTableIface *>(iface_);
  g_return_if_fail (iface != nullptr);

  iface->ref_at = table_wrapper_ref_at;
  iface->get_n_rows = table_wrapper_get_n_rows;
  iface->get_n_columns = table_wrapper_get_n_columns;
  iface->get_index_at = table_wrapper_get_index_at;
  iface->get_column_at_index = table_wrapper_get_column_at_index;
  iface->get_row_at_index = table_wrapper_get_row_at_index;
  iface->is_row_selected = table_wrapper_is_row_selected;
  iface->is_selected = table_wrapper_is_selected;
  iface->get_selected_rows = table_wrapper_get_selected_rows;
  iface->add_row_selection = table_wrapper_add_row_selection;
  iface->remove_row_selection = table_wrapper_remove_row_selection;
  iface->add_column_selection = table_wrapper_add_column_selection;
  iface->remove_column_selection = table_wrapper_remove_column_selection;
  iface->get_selected_columns = table_wrapper_get_selected_columns;
  iface->is_column_selected = table_wrapper_is_column_selected;
  iface->get_column_extent_at = table_wrapper_get_column_extent_at;
  iface->get_row_extent_at = table_wrapper_get_row_extent_at;
  iface->get_row_header = table_wrapper_get_row_header;
  iface->set_row_header = table_wrapper_set_row_header;
  iface->get_column_header = table_wrapper_get_column_header;
  iface->set_column_header = table_wrapper_set_column_header;
  iface->get_caption = table_wrapper_get_caption;
  iface->set_caption = table_wrapper_set_caption;
  iface->get_summary = table_wrapper_get_summary;
  iface->set_summary = table_wrapper_set_summary;
  iface->get_row_description = table_wrapper_get_row_description;
  iface->set_row_description = table_wrapper_set_row_description;
  iface->get_column_description = table_wrapper_get_column_description;
  iface->set_column_description = table_wrapper_set_column_description;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
