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

#include <boost/noncopyable.hpp>

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
/** @descr
*/
class AccessibleTableShape : boost::noncopyable, public AccessibleTableShape_Base, public css::accessibility::XAccessibleTableSelection
{
public:
    AccessibleTableShape( const AccessibleShapeInfo& rShapeInfo, const AccessibleShapeTreeInfo& rShapeTreeInfo );
    virtual ~AccessibleTableShape( );

    virtual void Init() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw () override;
    virtual void SAL_CALL release(  ) throw () override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( ) throw(css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount( ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw(css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleTable
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nChildIndex )  throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    // XAccessibleTableSelection
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) throw(css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL disposing( ) override;

    // XSelectionChangeListener
    virtual void SAL_CALL
        disposing (const css::lang::EventObject& Source)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void  SAL_CALL
        selectionChanged (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;
    sal_Int32 mnPreviousSelectionCount;
    using AccessibleShape::disposing;
    friend class AccessibleTableHeaderShape;

    void getColumnAndRow( sal_Int32 nChildIndex, sal_Int32& rnColumn, sal_Int32& rnRow ) throw (css::lang::IndexOutOfBoundsException );
    // overwrite the SetState & ResetState to do special operation for table cell's internal text
    virtual bool SetState (sal_Int16 aState) override;
    virtual bool ResetState (sal_Int16 aState) override;
    // The following two methods are used to set state directly on table object, instead of the internal cell or paragraph.
    bool SetStateDirectly (sal_Int16 aState);
    bool ResetStateDirectly (sal_Int16 aState);
    // Get the currently active cell which is text editing
    AccessibleCell* GetActiveAccessibleCell();

protected:
    virtual OUString CreateAccessibleBaseName() throw (css::uno::RuntimeException) override;

    sdr::table::SvxTableController* getTableController();

    void checkCellPosition( sal_Int32 nCol, sal_Int32 nRow ) throw ( css::lang::IndexOutOfBoundsException );

private:
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

class AccessibleTableHeaderShape : boost::noncopyable,
    public MutexOwner,
    public AccessibleTableHeaderShape_BASE
{
public:
    // bRow, true means rowheader, false means columnheader
    AccessibleTableHeaderShape( AccessibleTableShape* pTable, bool bRow );
    virtual ~AccessibleTableHeaderShape();

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( ) throw(css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

    //XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleTable
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleTableSelection
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
private:
    SVX_DLLPRIVATE explicit AccessibleTableHeaderShape( const css::uno::Reference<css::accessibility::XAccessible>& rxParent );
    bool mbRow;
    rtl::Reference< AccessibleTableShape > mpTable;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
