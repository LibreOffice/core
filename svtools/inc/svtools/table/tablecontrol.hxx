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
#include <svtools/accessibletable.hxx>
#include "svtaccessiblefactory.hxx"
//........................................................................

namespace svt { namespace table
{
//........................................................................

    class TableControl_Impl;
    class TableDataWindow;
    class AccessibleTableControl_Impl;

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
        The control supports supports accessibility, this is encapsulated in IAccessibleTable

        // TODO: scrolling?
    */
    class TableControl : public Control, public IAccessibleTable
    {
    private:
        DECL_LINK( ImplMouseButtonDownHdl, MouseEvent* );
        DECL_LINK( ImplMouseButtonUpHdl, MouseEvent* );

        TableControl_Impl*  m_pImpl;
    public:
        ::std::auto_ptr< AccessibleTableControl_Impl > m_pAccessTable;

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
        sal_Int32 GetCurrentRow() const;

        /** returns the row, which contains the input point*/

        ColPos  GetCurrentRow (const Point& rPoint);

        /** retrieves the current column

            The current col is the one which contains the active cell.

            @return
                the column index of the active cell, or ->COL_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        sal_Int32  GetCurrentColumn() const;

        /** activates the cell at the given position

            @return
                <TRUE/> if the move was successful, <FALSE/> otherwise. Usual
                failure conditions include some other instance vetoing the move,
                or impossibility to execute the move at all (for instance because
                of invalid coordinates).
        */
        bool    GoTo( ColPos _nColumnPos, RowPos _nRow);

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
        std::vector<sal_Int32>& GetSelectedRows();
        /**after removing a row, updates the vector which contains the selected rows
            if the row, which should be removed, is selected, it will be erased from the vector
        */
        void removeSelectedRow(RowPos _nRowPos);
        SelectionEngine* getSelEngine();
        TableDataWindow* getDataWindow();

        // Window overridables
        virtual void        GetFocus();
        virtual void        LoseFocus();
        virtual void        KeyInput( const KeyEvent& rKEvt );
        //virtual long      Notify(NotifyEvent& rNEvt);

            /** Creates and returns the accessible object of the whole BrowseBox. */
        virtual XACC CreateAccessible();
        virtual XACC CreateAccessibleControl( sal_Int32 _nIndex );
        virtual ::rtl::OUString GetAccessibleObjectName(AccessibleTableControlObjType eObjType, sal_Int32 _nRow, sal_Int32 _nCol) const;
        virtual sal_Bool GoToCell( sal_Int32 _nColumnPos, sal_Int32 _nRow );
        virtual ::rtl::OUString GetAccessibleObjectDescription(AccessibleTableControlObjType eObjType, sal_Int32 _nPosition = -1) const;
        virtual void FillAccessibleStateSet(
            ::utl::AccessibleStateSetHelper& rStateSet,
            AccessibleTableControlObjType eObjType ) const;

        //// Window
        virtual Rectangle GetWindowExtentsRelative( Window *pRelativeWindow );
        virtual void GrabFocus();
        virtual XACC GetAccessible( BOOL bCreate = TRUE );
        virtual Window* GetAccessibleParentWindow() const;
        virtual Window* GetWindowInstance();
        virtual sal_Int32 GetAccessibleControlCount() const;
        virtual sal_Bool ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint );
        virtual long GetRowCount() const;
        virtual long GetColumnCount() const;
        virtual sal_Bool HasRowHeader() const;
        virtual sal_Int32 GetSelectedRowCount() const;
        virtual bool IsRowSelected( long _nRow ) const;
        virtual sal_Bool ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint );
        virtual Rectangle calcHeaderRect( sal_Bool _bIsColumnBar, BOOL _bOnScreen = TRUE );
        virtual Rectangle calcTableRect( BOOL _bOnScreen = TRUE );
        virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
        virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
        virtual void FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const;
        virtual ::rtl::OUString GetRowDescription( sal_Int32 _nRow ) const;
        virtual ::rtl::OUString GetRowName(sal_Int32 _nIndex) const;
        virtual ::rtl::OUString GetColumnDescription( sal_uInt16 _nColumnPos ) const;
        virtual ::rtl::OUString GetColumnName( sal_Int32 _nIndex ) const;
        virtual ::rtl::OUString GetCellContent( sal_Int32 _nRowPos, sal_Int32 _nColPos) const;
        virtual sal_Bool HasRowHeader();
        virtual sal_Bool HasColHeader();
        virtual sal_Bool isAccessibleAlive( ) const;
        virtual void commitGridControlEvent( sal_Int16 _nEventId, const com::sun::star::uno::Any& _rNewValue, const com::sun::star::uno::Any& _rOldValue );


    protected:
    /// retrieves the XAccessible implementation associated with the GridControl instance
    ::svt::IAccessibleFactory&   getAccessibleFactory();

    private:
        TableControl();                                 // never implemented
        TableControl( const TableControl& );            // never implemented
        TableControl& operator=( const TableControl& ); // never implemented
    };

    class AccessibleTableControl_Impl
    {
        public:
            AccessibleFactoryAccess m_aFactoryAccess;
            IAccessibleTableControl*   m_pAccessible;

        public:
            AccessibleTableControl_Impl() : m_pAccessible(NULL)
            {
            }


            /// @see AccessibleTableControl::getTableRowHeader
            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                getAccessibleTableHeader( AccessibleTableControlObjType _eObjType );
            /// @see AccessibleTableControl::getTable
            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                getAccessibleTable( );

    };

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLECONTROL_HXX
