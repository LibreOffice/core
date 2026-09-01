/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/accessibility/XAccessibleTable.hpp>

#include <rtl/ref.hxx>

#include <cppuhelper/implbase.hxx>

#include <svx/AccessibleShape.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>

namespace sdr::table { class SvxTableController; }

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
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override;
    virtual void acquire(  ) noexcept override;
    virtual void release(  ) noexcept override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount( ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual sal_Int16 getAccessibleRole() override;

    // XAccessibleTable
    virtual sal_Int32 getAccessibleRowCount() override;
    virtual sal_Int32 getAccessibleColumnCount(  ) override;
    virtual OUString getAccessibleRowDescription( sal_Int32 nRow ) override;
    virtual OUString getAccessibleColumnDescription( sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > getAccessibleRowHeaders(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > getAccessibleColumnHeaders(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 > getSelectedAccessibleRows(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 > getSelectedAccessibleColumns(  ) override;
    virtual bool isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual bool isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleCaption(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleSummary(  ) override;
    virtual bool isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int64 getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRow( sal_Int64 nChildIndex ) override;
    virtual sal_Int32 getAccessibleColumn( sal_Int64 nChildIndex ) override;

    // XAccessibleSelection
    virtual void selectAccessibleChild( sal_Int64 nChildIndex ) override;
    virtual bool isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    virtual void clearAccessibleSelection(  ) override;
    virtual void selectAllAccessibleChildren(  ) override;
    virtual sal_Int64 getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    virtual void deselectAccessibleChild( sal_Int64 nChildIndex ) override;
    // XAccessibleTableSelection
    virtual bool selectRow( sal_Int32 row ) override ;
    virtual bool selectColumn( sal_Int32 column ) override ;
    virtual bool unselectRow( sal_Int32 row ) override ;
    virtual bool unselectColumn( sal_Int32 column ) override ;

    // XServiceInfo
    virtual OUString getImplementationName( ) override;

    // XComponent
    virtual void disposing( ) override;

    // XSelectionChangeListener
    virtual void
        disposing (const css::lang::EventObject& Source) override;
    virtual void
        selectionChanged (const css::lang::EventObject& rEvent) override;
    using AccessibleShape::disposing;
    friend class AccessibleTableHeaderShape;

    /// @throws css::lang::IndexOutOfBoundsException
    void getColumnAndRow( sal_Int64 nChildIndex, sal_Int32& rnColumn, sal_Int32& rnRow );
    // overwrite the SetState & ResetState to do special operation for table cell's internal text
    virtual bool SetState (sal_Int64 aState) override;
    virtual bool ResetState (sal_Int64 aState) override;
    // The following two methods are used to set state directly on table object, instead of the internal cell or paragraph.
    bool SetStateDirectly (sal_Int64 aState);
    bool ResetStateDirectly (sal_Int64 aState);
    // Get the currently active cell which is text editing
    AccessibleCell* GetActiveAccessibleCell();

private:
    virtual OUString CreateAccessibleBaseName() override;

    sdr::table::SvxTableController* getTableController();

    /// @throws css::lang::IndexOutOfBoundsException
    void checkCellPosition( sal_Int32 nCol, sal_Int32 nRow );

    rtl::Reference< AccessibleTableShapeImpl > mxImpl;
    sal_Int64 GetIndexOfSelectedChild( sal_Int64 nSelectedChildIndex ) const;
};

typedef ::cppu::WeakImplHelper<
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleTable,
            css::accessibility::XAccessibleTableSelection >
            AccessibleTableHeaderShape_BASE;

class AccessibleTableHeaderShape final : public AccessibleTableHeaderShape_BASE
{
public:
    // bRow, true means rowheader, false means columnheader
    AccessibleTableHeaderShape( AccessibleTableShape* pTable, bool bRow );
    virtual ~AccessibleTableHeaderShape() override;
    AccessibleTableHeaderShape(const AccessibleTableHeaderShape&) = delete;
    AccessibleTableHeaderShape& operator=(const AccessibleTableHeaderShape&) = delete;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> getAccessibleContext( ) override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleParent(  ) override;
    virtual sal_Int64 getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 getAccessibleRole(  ) override;
    virtual OUString getAccessibleDescription(  ) override;
    virtual OUString getAccessibleName(  ) override;
    virtual sal_Int64 getAccessibleStateSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;
    virtual css::lang::Locale getLocale(  ) override;

    //XAccessibleComponent
    virtual bool containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle getBounds(  ) override;
    virtual css::awt::Point getLocation(  ) override;
    virtual css::awt::Point getLocationOnScreen(  ) override;
    virtual css::awt::Size getSize(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;
    virtual void grabFocus(  ) override;

    // XAccessibleTable
    virtual sal_Int32 getAccessibleRowCount() override;
    virtual sal_Int32 getAccessibleColumnCount(  ) override;
    virtual OUString getAccessibleRowDescription( sal_Int32 nRow ) override;
    virtual OUString getAccessibleColumnDescription( sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > getAccessibleRowHeaders(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > getAccessibleColumnHeaders(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 > getSelectedAccessibleRows(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 > getSelectedAccessibleColumns(  ) override;
    virtual bool isAccessibleRowSelected( sal_Int32 nRow ) override;
    virtual bool isAccessibleColumnSelected( sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleCaption(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleSummary(  ) override;
    virtual bool isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int64 getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;
    virtual sal_Int32 getAccessibleRow( sal_Int64 nChildIndex ) override;
    virtual sal_Int32 getAccessibleColumn( sal_Int64 nChildIndex ) override;

    // XAccessibleTableSelection
    virtual bool selectRow( sal_Int32 row ) override ;
    virtual bool selectColumn( sal_Int32 column ) override ;
    virtual bool unselectRow( sal_Int32 row ) override ;
    virtual bool unselectColumn( sal_Int32 column ) override ;
private:
    bool mbRow;
    rtl::Reference< AccessibleTableShape > mpTable;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
