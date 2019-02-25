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

#ifndef INCLUDED_SVX_INC_ACCESSIBLETABLESHAPE_HXX
#define INCLUDED_SVX_INC_ACCESSIBLETABLESHAPE_HXX

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <rtl/ref.hxx>

#include <cppuhelper/implbase.hxx>

#include <svx/AccessibleShape.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <cppuhelper/compbase.hxx>

namespace sdr { namespace table {
    class SvxTableController;
} }

namespace accessibility
{
    class AccessibleTableShapeImpl;
    class AccessibleCell;

    typedef ::cppu::ImplInheritanceHelper< AccessibleShape,
                                            css::accessibility::XAccessibleTable,
                                            css::view::XSelectionChangeListener
                                          > AccessibleTableShape_Base;
class AccessibleTableShape final : public AccessibleTableShape_Base, public css::accessibility::XAccessibleTableSelection
{
    sal_Int32 mnPreviousSelectionCount;
public:
    AccessibleTableShape( const AccessibleShapeInfo& rShapeInfo, const AccessibleShapeTreeInfo& rShapeTreeInfo );
    virtual ~AccessibleTableShape( ) override;
    AccessibleTableShape(const AccessibleTableShape&) = delete;
    AccessibleTableShape& operator=(const AccessibleTableShape&) = delete;

    virtual void Init() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw () override;
    virtual void SAL_CALL release(  ) throw () override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount( ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    // XAccessibleTable
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) override;
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) override;
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nChildIndex ) override;
    // XAccessibleTableSelection
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row ) override ;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column ) override ;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row ) override ;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column ) override ;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) override;

    // XComponent
    virtual void SAL_CALL disposing( ) override;

    // XSelectionChangeListener
    virtual void SAL_CALL
        disposing (const css::lang::EventObject& Source) override;
    virtual void  SAL_CALL
        selectionChanged (const css::lang::EventObject& rEvent) override;
    using AccessibleShape::disposing;
    friend class AccessibleTableHeaderShape;

    /// @throws css::lang::IndexOutOfBoundsException
    void getColumnAndRow( sal_Int32 nChildIndex, sal_Int32& rnColumn, sal_Int32& rnRow );
    // overwrite the SetState & ResetState to do special operation for table cell's internal text
    virtual bool SetState (sal_Int16 aState) override;
    virtual bool ResetState (sal_Int16 aState) override;
    // The following two methods are used to set state directly on table object, instead of the internal cell or paragraph.
    bool SetStateDirectly (sal_Int16 aState);
    bool ResetStateDirectly (sal_Int16 aState);
    // Get the currently active cell which is text editing
    AccessibleCell* GetActiveAccessibleCell();

private:
    virtual OUString CreateAccessibleBaseName() override;

    sdr::table::SvxTableController* getTableController();

    /// @throws css::lang::IndexOutOfBoundsException
    void checkCellPosition( sal_Int32 nCol, sal_Int32 nRow );

    rtl::Reference< AccessibleTableShapeImpl > mxImpl;
    sal_Int32 GetIndexOfSelectedChild( sal_Int32 nSelectedChildIndex ) const;
};

typedef ::cppu::WeakImplHelper<
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleTable,
            css::accessibility::XAccessibleTableSelection >
            AccessibleTableHeaderShape_BASE;

class AccessibleTableHeaderShape : public MutexOwner,
    public AccessibleTableHeaderShape_BASE
{
public:
    // bRow, true means rowheader, false means columnheader
    AccessibleTableHeaderShape( AccessibleTableShape* pTable, bool bRow );
    virtual ~AccessibleTableHeaderShape() override;
    AccessibleTableHeaderShape(const AccessibleTableHeaderShape&) = delete;
    AccessibleTableHeaderShape& operator=(const AccessibleTableHeaderShape&) = delete;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;

    // XAccessibleTable
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) override;
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) override;
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) override;

    // XAccessibleTableSelection
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row ) override ;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column ) override ;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row ) override ;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column ) override ;
private:
    bool mbRow;
    rtl::Reference< AccessibleTableShape > mpTable;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
