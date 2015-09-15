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

#ifndef INCLUDED_SVTOOLS_INC_TABLE_TABLECONTROL_HXX
#define INCLUDED_SVTOOLS_INC_TABLE_TABLECONTROL_HXX

#include <svtools/svtdllapi.h>
#include <svtools/table/tablemodel.hxx>
#include <svtools/accessibletable.hxx>
#include <svtools/accessiblefactory.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/seleng.hxx>

#include <memory>

namespace svt { namespace table
{


    class TableControl_Impl;


    //= TableControl

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
        std::shared_ptr<TableControl_Impl>            m_pImpl;


    public:
        TableControl( vcl::Window* _pParent, WinBits _nStyle );
        virtual ~TableControl();
        virtual void dispose() SAL_OVERRIDE;

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
        sal_Int32 GetCurrentRow() const SAL_OVERRIDE;

        ITableControl&
                getTableControlInterface();

        /** retrieves the current column

            The current col is the one which contains the active cell.

            @return
                the column index of the active cell, or ->COL_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        sal_Int32  GetCurrentColumn() const SAL_OVERRIDE;

        /** activates the cell at the given position

            @return
                <sal_True/> if the move was successful, <FALSE/> otherwise. Usual
                failure conditions include some other instance vetoing the move,
                or impossibility to execute the move at all (for instance because
                of invalid coordinates).
        */
        bool    GoTo( ColPos _nColumnPos, RowPos _nRow);

        SVT_DLLPRIVATE virtual void Resize() SAL_OVERRIDE;
        void    Select();

        /**after removing a row, updates the vector which contains the selected rows
            if the row, which should be removed, is selected, it will be erased from the vector
        */
        SelectionEngine*    getSelEngine();
        vcl::Window&             getDataWindow();

        // Window overridables
        virtual void        GetFocus() SAL_OVERRIDE;
        virtual void        LoseFocus() SAL_OVERRIDE;
        virtual void        KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void        StateChanged( StateChangedType i_nStateChange ) SAL_OVERRIDE;

        /** Creates and returns the accessible object of the whole GridControl. */
        SVT_DLLPRIVATE virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;
        SVT_DLLPRIVATE virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleControl( sal_Int32 _nIndex ) SAL_OVERRIDE;
        SVT_DLLPRIVATE virtual OUString GetAccessibleObjectName(AccessibleTableControlObjType eObjType, sal_Int32 _nRow, sal_Int32 _nCol) const SAL_OVERRIDE;
        SVT_DLLPRIVATE virtual bool GoToCell( sal_Int32 _nColumnPos, sal_Int32 _nRow ) SAL_OVERRIDE;
        SVT_DLLPRIVATE virtual OUString GetAccessibleObjectDescription(AccessibleTableControlObjType eObjType, sal_Int32 _nPosition = -1) const SAL_OVERRIDE;
        SVT_DLLPRIVATE virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet, AccessibleTableControlObjType eObjType ) const SAL_OVERRIDE;

        // temporary methods
        // Those do not really belong into the public API - they're intended for firing A11Y-related events. However,
        // firing those events should be an implementation internal to the TableControl resp. TableControl_Impl,
        // instead of something triggered externally.
        void commitCellEventIfAccessibleAlive( sal_Int16 const i_eventID, const css::uno::Any& i_newValue, const css::uno::Any& i_oldValue );
        void commitTableEventIfAccessibleAlive( sal_Int16 const i_eventID, const css::uno::Any& i_newValue, const css::uno::Any& i_oldValue );


        // IAccessibleTable
        virtual Rectangle GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const SAL_OVERRIDE;
        virtual void GrabFocus() SAL_OVERRIDE;
        virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible( bool bCreate = true ) SAL_OVERRIDE;
        virtual vcl::Window* GetAccessibleParentWindow() const SAL_OVERRIDE;
        virtual vcl::Window* GetWindowInstance() SAL_OVERRIDE;
        virtual sal_Int32 GetAccessibleControlCount() const SAL_OVERRIDE;
        virtual bool ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint ) SAL_OVERRIDE;
        virtual long GetRowCount() const SAL_OVERRIDE;
        virtual long GetColumnCount() const SAL_OVERRIDE;
        virtual bool ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint ) SAL_OVERRIDE;
        virtual Rectangle calcHeaderRect( bool _bIsColumnBar, bool _bOnScreen = true ) SAL_OVERRIDE;
        virtual Rectangle calcHeaderCellRect( bool _bIsColumnBar, sal_Int32 nPos) SAL_OVERRIDE;
        virtual Rectangle calcTableRect( bool _bOnScreen = true ) SAL_OVERRIDE;
        virtual Rectangle calcCellRect( sal_Int32 _nRowPos, sal_Int32 _nColPos ) SAL_OVERRIDE;
        virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) SAL_OVERRIDE;
        virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) SAL_OVERRIDE;
        virtual void FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const SAL_OVERRIDE;
        virtual OUString GetRowDescription( sal_Int32 _nRow ) const SAL_OVERRIDE;
        virtual OUString GetRowName(sal_Int32 _nIndex) const SAL_OVERRIDE;
        virtual OUString GetColumnDescription( sal_uInt16 _nColumnPos ) const SAL_OVERRIDE;
        virtual OUString GetColumnName( sal_Int32 _nIndex ) const SAL_OVERRIDE;
        virtual bool HasRowHeader() SAL_OVERRIDE;
        virtual bool HasColHeader() SAL_OVERRIDE;
        virtual OUString GetAccessibleCellText(sal_Int32 _nRowPos, sal_Int32 _nColPos) const SAL_OVERRIDE;

        virtual sal_Int32 GetSelectedRowCount() const SAL_OVERRIDE;
        virtual sal_Int32 GetSelectedRowIndex( sal_Int32 const i_selectionIndex ) const SAL_OVERRIDE;
        virtual bool IsRowSelected( sal_Int32 const i_rowIndex ) const SAL_OVERRIDE;
        virtual void SelectRow( sal_Int32 const i_rowIndex, bool const i_select ) SAL_OVERRIDE;
        virtual void SelectAllRows( bool const i_select ) SAL_OVERRIDE;


    private:
        DECL_DLLPRIVATE_LINK_TYPED( ImplSelectHdl, LinkParamNone*, void );

    private:
        TableControl( const TableControl& ) SAL_DELETED_FUNCTION;
        TableControl& operator=( const TableControl& ) SAL_DELETED_FUNCTION;
    };


} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_INC_TABLE_TABLECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
