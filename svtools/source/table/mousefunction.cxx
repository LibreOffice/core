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


#include "mousefunction.hxx"
#include <table/tablecontrolinterface.hxx>
#include <svtools/table/tablesort.hxx>

#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>
#include <vcl/ptrstyle.hxx>


namespace svt { namespace table
{


    //= ColumnResize


    FunctionResult ColumnResize::handleMouseMove( ITableControl& i_tableControl, MouseEvent const & i_event )
    {
        Point const aPoint = i_event.GetPosPixel();

        if ( m_nResizingColumn == COL_INVALID )
        {
            // if we hit a column divider, change the mouse pointer accordingly
            PointerStyle aNewPointer( PointerStyle::Arrow );
            TableCell const tableCell = i_tableControl.hitTest( aPoint );
            if ( ( tableCell.nRow == ROW_COL_HEADERS ) && ( tableCell.eArea == ColumnDivider ) )
            {
                aNewPointer = PointerStyle::HSplit;
            }
            i_tableControl.setPointer( aNewPointer );

            return SkipFunction;    // TODO: is this correct?
        }

        ::Size const tableSize = i_tableControl.getTableSizePixel();

        // set proper pointer
        PointerStyle aNewPointer( PointerStyle::Arrow );
        ColumnMetrics const & columnMetrics( i_tableControl.getColumnMetrics( m_nResizingColumn ) );
        if  (   ( aPoint.X() > tableSize.Width() )
            ||  ( aPoint.X() < columnMetrics.nStartPixel )
            )
        {
            aNewPointer = PointerStyle::NotAllowed;
        }
        else
        {
            aNewPointer = PointerStyle::HSplit;
        }
        i_tableControl.setPointer( aNewPointer );

        // show tracking line
        i_tableControl.hideTracking();
        i_tableControl.showTracking(
            tools::Rectangle(
                Point( aPoint.X(), 0 ),
                Size( 1, tableSize.Height() )
            ),
            ShowTrackFlags::Split | ShowTrackFlags::TrackWindow
        );

        return ContinueFunction;
    }


    FunctionResult ColumnResize::handleMouseDown( ITableControl& i_tableControl, MouseEvent const & i_event )
    {
        if ( m_nResizingColumn != COL_INVALID )
        {
            OSL_ENSURE( false, "ColumnResize::handleMouseDown: suspicious: MouseButtonDown while still tracking?" );
            return ContinueFunction;
        }

        TableCell const tableCell( i_tableControl.hitTest( i_event.GetPosPixel() ) );
        if ( tableCell.nRow == ROW_COL_HEADERS )
        {
            if  (   ( tableCell.nColumn != COL_INVALID )
                &&  ( tableCell.eArea == ColumnDivider )
                )
            {
                m_nResizingColumn = tableCell.nColumn;
                i_tableControl.captureMouse();
                return ActivateFunction;
            }
        }

        return SkipFunction;
    }


    FunctionResult ColumnResize::handleMouseUp( ITableControl& i_tableControl, MouseEvent const & i_event )
    {
        if ( m_nResizingColumn == COL_INVALID )
            return SkipFunction;

        Point const aPoint = i_event.GetPosPixel();

        i_tableControl.hideTracking();
        PColumnModel const pColumn = i_tableControl.getModel()->getColumnModel( m_nResizingColumn );
        long const maxWidthLogical = pColumn->getMaxWidth();
        long const minWidthLogical = pColumn->getMinWidth();

        // new position of mouse
        long const requestedEnd = aPoint.X();

        // old position of right border
        long const oldEnd = i_tableControl.getColumnMetrics( m_nResizingColumn ).nEndPixel;

        // position of left border if cursor in the to-be-resized column
        long const columnStart = i_tableControl.getColumnMetrics( m_nResizingColumn ).nStartPixel;
        long const requestedWidth = requestedEnd - columnStart;
            // TODO: this is not correct, strictly: It assumes that the mouse was pressed exactly on the "end" pos,
            // but for a while now, we have relaxed this, and allow clicking a few pixels aside, too

        if ( requestedEnd >= columnStart )
        {
            long requestedWidthLogical = i_tableControl.pixelWidthToAppFont( requestedWidth );
            // respect column width limits
            if ( oldEnd > requestedEnd )
            {
                // column has become smaller, check against minimum width
                if ( ( minWidthLogical != 0 ) && ( requestedWidthLogical < minWidthLogical ) )
                    requestedWidthLogical = minWidthLogical;
            }
            else if ( oldEnd < requestedEnd )
            {
                // column has become larger, check against max width
                if ( ( maxWidthLogical != 0 ) && ( requestedWidthLogical >= maxWidthLogical ) )
                    requestedWidthLogical = maxWidthLogical;
            }
            pColumn->setWidth( requestedWidthLogical );
            i_tableControl.invalidate( TableArea::All );
        }

        i_tableControl.setPointer( PointerStyle::Arrow );
        i_tableControl.releaseMouse();

        m_nResizingColumn = COL_INVALID;
        return DeactivateFunction;
    }


    //= RowSelection


    FunctionResult RowSelection::handleMouseMove( ITableControl&, MouseEvent const & )
    {
        return SkipFunction;
    }


    FunctionResult RowSelection::handleMouseDown( ITableControl& i_tableControl, MouseEvent const & i_event )
    {
        bool handled = false;

        TableCell const tableCell( i_tableControl.hitTest( i_event.GetPosPixel() ) );
        if ( tableCell.nRow >= 0 )
        {
            if ( i_tableControl.getSelEngine()->GetSelectionMode() == SelectionMode::NONE )
            {
                i_tableControl.activateCell( tableCell.nColumn, tableCell.nRow );
                handled = true;
            }
            else
            {
                handled = i_tableControl.getSelEngine()->SelMouseButtonDown( i_event );
            }
        }

        if ( handled )
            m_bActive = true;
        return handled ? ActivateFunction : SkipFunction;
    }


    FunctionResult RowSelection::handleMouseUp( ITableControl& i_tableControl, MouseEvent const & i_event )
    {
        TableCell const tableCell = i_tableControl.hitTest( i_event.GetPosPixel() );
        if ( tableCell.nRow >= 0 )
        {
            if ( i_tableControl.getSelEngine()->GetSelectionMode() != SelectionMode::NONE )
            {
                i_tableControl.getSelEngine()->SelMouseButtonUp( i_event );
            }
        }
        if ( m_bActive )
        {
            m_bActive = false;
            return DeactivateFunction;
        }
        return SkipFunction;
    }


    //= ColumnSortHandler


    FunctionResult ColumnSortHandler::handleMouseMove( ITableControl&, MouseEvent const & )
    {
        return SkipFunction;
    }


    FunctionResult ColumnSortHandler::handleMouseDown( ITableControl& i_tableControl, MouseEvent const & i_event )
    {
        if ( m_nActiveColumn != COL_INVALID )
        {
            OSL_ENSURE( false, "ColumnSortHandler::handleMouseDown: called while already active - suspicious!" );
            return ContinueFunction;
        }

        if ( i_tableControl.getModel()->getSortAdapter() == nullptr )
            // no sorting support at the model
            return SkipFunction;

        TableCell const tableCell( i_tableControl.hitTest( i_event.GetPosPixel() ) );
        if ( ( tableCell.nRow != ROW_COL_HEADERS ) || ( tableCell.nColumn < 0 ) )
            return SkipFunction;

        // TODO: ensure the column header is rendered in some special way, indicating its current state

        m_nActiveColumn = tableCell.nColumn;
        return ActivateFunction;
    }


    FunctionResult ColumnSortHandler::handleMouseUp( ITableControl& i_tableControl, MouseEvent const & i_event )
    {
        if ( m_nActiveColumn == COL_INVALID )
            return SkipFunction;

        TableCell const tableCell( i_tableControl.hitTest( i_event.GetPosPixel() ) );
        if ( ( tableCell.nRow == ROW_COL_HEADERS ) && ( tableCell.nColumn == m_nActiveColumn ) )
        {
            ITableDataSort* pSort = i_tableControl.getModel()->getSortAdapter();
            ENSURE_OR_RETURN( pSort != nullptr, "ColumnSortHandler::handleMouseUp: somebody is mocking with us!", DeactivateFunction );
                // in handleMousButtonDown, the model claimed to have sort support ...

            ColumnSortDirection eSortDirection = ColumnSortAscending;
            ColumnSort const aCurrentSort = pSort->getCurrentSortOrder();
            if ( aCurrentSort.nColumnPos == m_nActiveColumn )
                // invert existing sort order
                eSortDirection = ( aCurrentSort.eSortDirection == ColumnSortAscending ) ? ColumnSortDescending : ColumnSortAscending;

            pSort->sortByColumn( m_nActiveColumn, eSortDirection );
        }

        m_nActiveColumn = COL_INVALID;
        return DeactivateFunction;
    }


} } // namespace svt::table


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
