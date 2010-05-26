/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SVTOOLS_INC_TABLE_ABSTRACTTABLECONTROL_HXX
#define SVTOOLS_INC_TABLE_ABSTRACTTABLECONTROL_HXX

#include <sal/types.h>
#include <vcl/event.hxx>
#include <vcl/seleng.hxx>
#include "tabletypes.hxx"
//........................................................................
namespace svt { namespace table
{
//........................................................................

    enum TableControlAction
    {
        /// moves the cursor in the table control one row up, if possible, by keeping the current column
        cursorUp,
        /// moves the cursor in the table control one row down, if possible, by keeping the current column
        cursorDown,
        /// moves the cursor in the table control one column to the left, if possible, by keeping the current row
        cursorLeft,
        /// moves the cursor in the table control one column to the right, if possible, by keeping the current row
        cursorRight,
        /// moves the cursor to the beginning of the current line
        cursorToLineStart,
        /// moves the cursor to the end of the current line
        cursorToLineEnd,
        /// moves the cursor to the first row, keeping the current column
        cursorToFirstLine,
        /// moves the cursor to the last row, keeping the current column
        cursorToLastLine,
        /// moves the cursor one page up, keeping the current column
        cursorPageUp,
        /// moves the cursor one page down, keeping the current column
        cursorPageDown,
        /// moves the cursor to the top-most, left-most cell
        cursorTopLeft,
        /// moves the cursor to the bottom-most, right-most cell
        cursorBottomRight,
        /// selects the row, where the actual cursor is
        cursorSelectRow,
        /// selects the rows, above the actual cursor is
        cursorSelectRowUp,
        /// selects the row, beneath the actual cursor is
        cursorSelectRowDown,
        /// selects the row, from the actual cursor till top
        cursorSelectRowAreaTop,
        /// selects the row, from the actual cursor till bottom
        cursorSelectRowAreaBottom,


        /// invalid and final enumeration value, not to be actually used
        invalidTableControlAction
    };

    //====================================================================
    //= IAbstractTableControl
    //====================================================================
    /** defines a callback interface to be implemented by a concrete table control
    */
    class SAL_NO_VTABLE IAbstractTableControl
    {
    public:
        /** hides the cell cursor

            The method cares for successive calls, that is, for every call to
            ->hideCursor(), you need one call to ->showCursor. Only if the number
            of both calls matches, the cursor is really shown.

            @see showCursor
        */
        virtual void    hideCursor() = 0;

        /** shows the cell cursor

            @see hideCursor
        */
        virtual void    showCursor() = 0;

        /** dispatches an action to the table control

            @return
                <TRUE/> if the action could be dispatched successfully, <FALSE/> otherwise. Usual
                failure conditions include some other instance vetoing the action, or impossibility
                to execute the action at all (for instance moving up one row when already positioned
                on the very first row).

            @see TableControlAction
        */
        virtual bool    dispatchAction( TableControlAction _eAction ) = 0;
    /** returns selection engine*/
    virtual SelectionEngine* getSelEngine() = 0;
    virtual void setCursorAtCurrentCell(const Point& rPoint) = 0;
    virtual rtl::OUString& setTooltip(const Point& rPoint ) = 0;
    virtual RowPos getCurrentRow(const Point& rPoint ) = 0;
    virtual void resizeColumn(const Point& rPoint ) = 0;
    virtual bool startResizeColumn(const Point& rPoint) = 0;
    virtual bool endResizeColumn(const Point& rPoint) = 0;
    virtual bool isRowSelected(RowPos _nRow) = 0;

        virtual ~IAbstractTableControl() {};
    };

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_ABSTRACTTABLECONTROL_HXX
