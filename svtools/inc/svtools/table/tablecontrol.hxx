/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef SVTOOLS_INC_TABLE_TABLECONTROL_HXX
#define SVTOOLS_INC_TABLE_TABLECONTROL_HXX

#include <svtools/table/tablemodel.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/seleng.hxx>
#include <svtools/table/tabledatawindow.hxx>
//........................................................................

namespace svt { namespace table
{
//........................................................................

    class TableControl_Impl;
    class TableDataWindow;
    //====================================================================
    //= TableControl
    //====================================================================
    /** a basic control which manages table-like data, i.e. a number of cells
        organized in <code>m</code> rows and <code>n</code> columns.

        The control itself does not do any assumptions about the concrete data
        it displays, this is encapsulated in an instance supporting the
        ->ITableModel interface.

        Also, the control does not do any assumptions about how the model's
        content is rendered. This is the responsibility of a component
        supporting the ->ITableRenderer interface (the renderer is obtained from
        the model).

        The control supports the concept of a <em>current</em> (or <em>active</em>
        cell).

        // TODO: scrolling?
    */
    class TableControl : public Control
    {
    private:
        DECL_LINK( ImplMouseButtonDownHdl, MouseEvent* );
        DECL_LINK( ImplMouseButtonUpHdl, MouseEvent* );

        TableControl_Impl*  m_pImpl;
    public:
        TableControl( Window* _pParent, WinBits _nStyle );
        ~TableControl();

        /// sets a new table model
        void        SetModel( PTableModel _pModel );
        /// retrieves the current table model
        PTableModel GetModel() const;

        /// returns the top row, i.e. the first visible row
        RowPos  GetTopRow() const;
        /// sets a new top row. The top row is the first visible row in the control
        void    SetTopRow( RowPos _nRow );

        /** retrieves the current row

            The current row is the one which contains the active cell.

            @return
                the row index of the active cell, or ->ROW_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        RowPos  GetCurrentRow() const;

        /** returns the row, which contains the input point*/

        RowPos  GetCurrentRow (const Point& rPoint);

        /** retrieves the current column

            The current col is the one which contains the active cell.

            @return
                the column index of the active cell, or ->COL_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        ColPos  GetCurrentColumn() const;

        /** activates the cell at the given position

            @return
                <TRUE/> if the move was successful, <FALSE/> otherwise. Usual
                failure conditions include some other instance vetoing the move,
                or impossibility to execute the move at all (for instance because
                of invalid coordinates).
        */
        bool    GoTo( ColPos _nColumn, RowPos _nRow );

        /** moves the active cell to the given column, by keeping the active row

            @return
                <TRUE/> if the move was successful, <FALSE/> otherwise. Usual
                failure conditions include some other instance vetoing the move,
                or impossibility to execute the move at all (for instance because
                of invalid coordinates).
        */
        inline  bool    GoToColumn( ColPos _nColumn )
        {
            return GoTo( _nColumn, GetCurrentRow() );
        }

        /** moves the active cell to the given row, by keeping the active column

            @return
                <TRUE/> if the move was successful, <FALSE/> otherwise. Usual
                failure conditions include some other instance vetoing the move,
                or impossibility to execute the move at all (for instance because
                of invalid coordinates).
        */
        bool    GoToRow( RowPos _nRow )
        {
            return GoTo( GetCurrentColumn(), _nRow );
        }
        virtual void        Resize();

        /**invalidates the table if table has been changed e.g. new row added
        */
        void InvalidateDataWindow(RowPos _nRowStart, bool _bRemoved);
        /**gets the vector, which contains the selected rows
        */
        std::vector<RowPos> getSelectedRows();
        /**after removing a row, updates the vector which contains the selected rows
            if the row, which should be removed, is selected, it will be erased from the vector
        */
        void removeSelectedRow(RowPos _nRowPos);
        SelectionEngine* getSelEngine();
        TableDataWindow* getDataWindow();
   // protected:
        // Window overridables
        virtual void        GetFocus();
        virtual void        LoseFocus();
        virtual void        KeyInput( const KeyEvent& rKEvt );
        //virtual long      Notify(NotifyEvent& rNEvt);

    private:
        TableControl();                                 // never implemented
        TableControl( const TableControl& );            // never implemented
        TableControl& operator=( const TableControl& ); // never implemented
    };

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLECONTROL_HXX
