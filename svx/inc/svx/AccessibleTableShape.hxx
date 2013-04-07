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

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_TABLE_SHAPE_HXX
#define _SVX_ACCESSIBILITY_ACCESSIBLE_TABLE_SHAPE_HXX

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <rtl/ref.hxx>

#include <cppuhelper/implbase2.hxx>

#include <svx/AccessibleShape.hxx>

#include <boost/noncopyable.hpp>

namespace sdr { namespace table {
    class SvxTableController;
} }

namespace accessibility
{
    class AccessibleTableShapeImpl;

    typedef ::cppu::ImplInheritanceHelper2< AccessibleShape,
                                            ::com::sun::star::accessibility::XAccessibleSelection,
                                            ::com::sun::star::accessibility::XAccessibleTable
                                          > AccessibleTableShape_Base;
/** @descr
*/
class AccessibleTableShape : boost::noncopyable, public AccessibleTableShape_Base
{
public:
    AccessibleTableShape( const AccessibleShapeInfo& rShapeInfo, const AccessibleShapeTreeInfo& rShapeTreeInfo );
    virtual ~AccessibleTableShape( );

    virtual void Init (void);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( ) throw(::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount( ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole (void) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleTable
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL clearAccessibleSelection(  ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL selectAllAccessibleChildren(  ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nChildIndex )  throw ( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL disposing( );

    using AccessibleShape::disposing;

protected:
    virtual OUString CreateAccessibleBaseName(void) throw (::com::sun::star::uno::RuntimeException);

    sdr::table::SvxTableController* getTableController();

    void checkCellPosition( sal_Int32 nCol, sal_Int32 nRow ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
    void getColumnAndRow( sal_Int32 nChildIndex, sal_Int32& rnColumn, sal_Int32& rnRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException );

private:
    rtl::Reference< AccessibleTableShapeImpl > mxImpl;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
