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

#pragma once

#include <svtools/svtdllapi.h>
#include <table/tablemodel.hxx>
#include <vcl/accessibletable.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/seleng.hxx>

#include <memory>

namespace svt::table
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
    class TableControl final : public Control, public vcl::table::IAccessibleTable
    {
    private:
        std::shared_ptr<TableControl_Impl>            m_pImpl;


    public:
        TableControl( vcl::Window* _pParent, WinBits _nStyle );
        virtual ~TableControl() override;
        virtual void dispose() override;

        /// sets a new table model
        void        SetModel( const PTableModel& _pModel );
        /// retrieves the current table model
        PTableModel GetModel() const;

        /** retrieves the current row

            The current row is the one which contains the active cell.

            @return
                the row index of the active cell, or ->ROW_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        sal_Int32 GetCurrentRow() const override;

        ITableControl&
                getTableControlInterface();

        /** retrieves the current column

            The current col is the one which contains the active cell.

            @return
                the column index of the active cell, or ->COL_INVALID
                if there is no active cell, e.g. because the table does
                not contain any rows or columns.
        */
        sal_Int32  GetCurrentColumn() const override;

        /** activates the cell at the given position
        */
        void    GoTo( ColPos _nColumnPos, RowPos _nRow);

        virtual void Resize() override;
        void    Select();

        /**after removing a row, updates the vector which contains the selected rows
            if the row, which should be removed, is selected, it will be erased from the vector
        */
        SelectionEngine*    getSelEngine();
        vcl::Window&             getDataWindow();

        // Window overridables
        virtual void        GetFocus() override;
        virtual void        LoseFocus() override;
        virtual void        KeyInput( const KeyEvent& rKEvt ) override;
        virtual void        StateChanged( StateChangedType i_nStateChange ) override;

        /** Creates and returns the accessible object of the whole GridControl. */
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleControl( sal_Int32 _nIndex ) override;
        virtual OUString GetAccessibleObjectName(vcl::table::AccessibleTableControlObjType eObjType, sal_Int32 _nRow, sal_Int32 _nCol) const override;
        virtual void GoToCell( sal_Int32 _nColumnPos, sal_Int32 _nRow ) override;
        virtual OUString GetAccessibleObjectDescription(vcl::table::AccessibleTableControlObjType eObjType) const override;
        virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet, vcl::table::AccessibleTableControlObjType eObjType ) const override;

        // temporary methods
        // Those do not really belong into the public API - they're intended for firing A11Y-related events. However,
        // firing those events should be an implementation internal to the TableControl resp. TableControl_Impl,
        // instead of something triggered externally.
        void commitCellEventIfAccessibleAlive( sal_Int16 const i_eventID, const css::uno::Any& i_newValue, const css::uno::Any& i_oldValue );
        void commitTableEventIfAccessibleAlive( sal_Int16 const i_eventID, const css::uno::Any& i_newValue, const css::uno::Any& i_oldValue );


        // IAccessibleTable
        virtual tools::Rectangle GetWindowExtentsRelative(const vcl::Window *pRelativeWindow) const override;
        virtual void GrabFocus() override;
        virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible() override;
        virtual vcl::Window* GetAccessibleParentWindow() const override;
        virtual vcl::Window* GetWindowInstance() override;
        virtual sal_Int32 GetAccessibleControlCount() const override;
        virtual bool ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint ) override;
        virtual long GetRowCount() const override;
        virtual long GetColumnCount() const override;
        virtual bool ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint ) override;
        virtual tools::Rectangle calcHeaderRect( bool _bIsColumnBar ) override;
        virtual tools::Rectangle calcHeaderCellRect( bool _bIsColumnBar, sal_Int32 nPos) override;
        virtual tools::Rectangle calcTableRect() override;
        virtual tools::Rectangle calcCellRect( sal_Int32 _nRowPos, sal_Int32 _nColPos ) override;
        virtual tools::Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) override;
        virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) override;
        virtual void FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const override;
        virtual OUString GetRowName(sal_Int32 _nIndex) const override;
        virtual OUString GetColumnName( sal_Int32 _nIndex ) const override;
        virtual bool HasRowHeader() override;
        virtual bool HasColHeader() override;
        virtual OUString GetAccessibleCellText(sal_Int32 _nRowPos, sal_Int32 _nColPos) const override;

        virtual sal_Int32 GetSelectedRowCount() const override;
        virtual sal_Int32 GetSelectedRowIndex( sal_Int32 const i_selectionIndex ) const override;
        virtual bool IsRowSelected( sal_Int32 const i_rowIndex ) const override;
        virtual void SelectRow( sal_Int32 const i_rowIndex, bool const i_select ) override;
        virtual void SelectAllRows( bool const i_select ) override;


    private:
        DECL_LINK( ImplSelectHdl, LinkParamNone*, void );

    private:
        TableControl( const TableControl& ) = delete;
        TableControl& operator=( const TableControl& ) = delete;
    };


} // namespace svt::table



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
