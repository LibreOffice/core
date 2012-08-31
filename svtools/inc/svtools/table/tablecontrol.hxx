/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SVTOOLS_INC_TABLE_TABLECONTROL_HXX
#define SVTOOLS_INC_TABLE_TABLECONTROL_HXX

#include "svtools/svtdllapi.h"
#include "svtools/table/tablemodel.hxx"
#include "svtools/accessibletable.hxx"
#include "svtools/accessiblefactory.hxx"

#include <vcl/ctrl.hxx>
#include <vcl/seleng.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

//........................................................................

namespace svt { namespace table
{
//........................................................................

    class TableControl_Impl;

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
        The control supports accessibility, this is encapsulated in IAccessibleTable
    */
    class SVT_DLLPUBLIC TableControl : public Control, public IAccessibleTable
    {
    private:
        ::boost::shared_ptr< TableControl_Impl >            m_pImpl;


    public:
        TableControl( Window* _pParent, WinBits _nStyle );
        ~TableControl();

        /// sets a new table model
        void        SetModel( PTableModel _pModel );
        /// retrieves the current table model
        PTableModel GetModel() const;

        /** retrieves the current row

            The current row is the one which contains the active cell.

            @return
                the row index of the active cell, or ->ROW_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        sal_Int32 GetCurrentRow() const;

        ITableControl&
                getTableControlInterface();

        /** retrieves the current column

            The current col is the one which contains the active cell.

            @return
                the column index of the active cell, or ->COL_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        sal_Int32  GetCurrentColumn() const;

        SVT_DLLPRIVATE virtual void Resize();
        virtual void    Select();

        /**after removing a row, updates the vector which contains the selected rows
            if the row, which should be removed, is selected, it will be erased from the vector
        */
        SelectionEngine*    getSelEngine();
        Window&             getDataWindow();

        // Window overridables
        virtual void        GetFocus();
        virtual void        LoseFocus();
        virtual void        KeyInput( const KeyEvent& rKEvt );
        virtual void        StateChanged( StateChangedType i_nStateChange );

        /** Creates and returns the accessible object of the whole GridControl. */
        SVT_DLLPRIVATE virtual XACC CreateAccessible();
        SVT_DLLPRIVATE virtual XACC CreateAccessibleControl( sal_Int32 _nIndex );
        SVT_DLLPRIVATE virtual ::rtl::OUString GetAccessibleObjectName(AccessibleTableControlObjType eObjType, sal_Int32 _nRow, sal_Int32 _nCol) const;
        SVT_DLLPRIVATE virtual sal_Bool GoToCell( sal_Int32 _nColumnPos, sal_Int32 _nRow );
        SVT_DLLPRIVATE virtual ::rtl::OUString GetAccessibleObjectDescription(AccessibleTableControlObjType eObjType, sal_Int32 _nPosition = -1) const;
        SVT_DLLPRIVATE virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet, AccessibleTableControlObjType eObjType ) const;

        // temporary methods
        // Those do not really belong into the public API - they're intended for firing A11Y-related events. However,
        // firing those events should be an implementation internal to the TableControl resp. TableControl_Impl,
        // instead of something triggered externally.
        void commitCellEventIfAccessibleAlive( sal_Int16 const i_eventID, const ::com::sun::star::uno::Any& i_newValue, const ::com::sun::star::uno::Any& i_oldValue );
        void commitTableEventIfAccessibleAlive( sal_Int16 const i_eventID, const ::com::sun::star::uno::Any& i_newValue, const ::com::sun::star::uno::Any& i_oldValue );

        // .............................................................................................................
        // IAccessibleTable
        virtual Rectangle GetWindowExtentsRelative( Window *pRelativeWindow ) const;
        virtual void GrabFocus();
        virtual XACC GetAccessible( sal_Bool bCreate = sal_True );
        virtual Window* GetAccessibleParentWindow() const;
        virtual Window* GetWindowInstance();
        virtual sal_Int32 GetAccessibleControlCount() const;
        virtual sal_Bool ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint );
        virtual long GetRowCount() const;
        virtual long GetColumnCount() const;
        virtual sal_Bool HasRowHeader() const;
        virtual sal_Bool ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint );
        virtual Rectangle calcHeaderRect( sal_Bool _bIsColumnBar, sal_Bool _bOnScreen = sal_True );
        virtual Rectangle calcHeaderCellRect( sal_Bool _bIsColumnBar, sal_Int32 nPos);
        virtual Rectangle calcTableRect( sal_Bool _bOnScreen = sal_True );
        virtual Rectangle calcCellRect( sal_Int32 _nRowPos, sal_Int32 _nColPos );
        virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
        virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
        virtual void FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const;
        virtual ::rtl::OUString GetRowDescription( sal_Int32 _nRow ) const;
        virtual ::rtl::OUString GetRowName(sal_Int32 _nIndex) const;
        virtual ::rtl::OUString GetColumnDescription( sal_uInt16 _nColumnPos ) const;
        virtual ::rtl::OUString GetColumnName( sal_Int32 _nIndex ) const;
        virtual ::com::sun::star::uno::Any GetCellContent( sal_Int32 _nRowPos, sal_Int32 _nColPos) const;
        virtual sal_Bool HasRowHeader();
        virtual sal_Bool HasColHeader();
        virtual ::rtl::OUString GetAccessibleCellText(sal_Int32 _nRowPos, sal_Int32 _nColPos) const;

        virtual sal_Int32 GetSelectedRowCount() const;
        virtual sal_Int32 GetSelectedRowIndex( sal_Int32 const i_selectionIndex ) const;
        virtual bool IsRowSelected( sal_Int32 const i_rowIndex ) const;
        virtual void SelectRow( sal_Int32 const i_rowIndex, bool const i_select );
        virtual void SelectAllRows( bool const i_select );
        // .............................................................................................................

    private:
        DECL_DLLPRIVATE_LINK( ImplMouseButtonDownHdl, MouseEvent* );
        DECL_DLLPRIVATE_LINK( ImplMouseButtonUpHdl, MouseEvent* );

        DECL_DLLPRIVATE_LINK( ImplSelectHdl, void* );

    private:
        TableControl();                                 // never implemented
        TableControl( const TableControl& );            // never implemented
        TableControl& operator=( const TableControl& ); // never implemented
    };

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
