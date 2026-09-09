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

#include <vcl/accessibility/AccessibleBrowseBoxHeaderBar.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <vcl/unohelp.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using ::com::sun::star::uno::Reference;
using ::cpo::uno::Sequence;
using ::cpo::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;


// Ctor/Dtor/disposing --------------------------------------------------------

AccessibleBrowseBoxHeaderBar::AccessibleBrowseBoxHeaderBar(
        const Reference< XAccessible >& rxParent,
        vcl::IAccessibleTableProvider& rBrowseBox,
        AccessibleBrowseBoxObjType eObjType ) :
    AccessibleBrowseBoxTableBase( rxParent, rBrowseBox,eObjType )
{
    OSL_ENSURE( isRowBar() || isColumnBar(),
        "AccessibleBrowseBoxHeaderBar - invalid object type" );
}

AccessibleBrowseBoxHeaderBar::~AccessibleBrowseBoxHeaderBar()
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible >
AccessibleBrowseBoxHeaderBar::getAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidHeaderIndex( nChildIndex );
    return implGetChild( nChildIndex, implToVCLColumnPos( nChildIndex ) );
}

sal_Int64 AccessibleBrowseBoxHeaderBar::getAccessibleIndexInParent()
{
    return isRowBar() ? vcl::BBINDEX_ROWHEADERBAR : vcl::BBINDEX_COLUMNHEADERBAR;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible >
AccessibleBrowseBoxHeaderBar::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    sal_Int32 nRow = 0;
    sal_uInt16 nColumnPos = 0;
    bool bConverted
        = isRowBar()
              ? mpBrowseBox->ConvertPointToRowHeader(nRow, vcl::unohelper::ConvertToVCLPoint(rPoint))
              : mpBrowseBox->ConvertPointToColumnHeader(nColumnPos,
                                                        vcl::unohelper::ConvertToVCLPoint(rPoint));

    return bConverted ? implGetChild( nRow, nColumnPos ) : Reference< XAccessible >();
}

void AccessibleBrowseBoxHeaderBar::grabFocus()
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    // focus on header not supported
}

// XAccessibleTable -----------------------------------------------------------

OUString AccessibleBrowseBoxHeaderBar::getAccessibleRowDescription( sal_Int32 nRow )
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    ensureIsValidRow( nRow );
    return OUString();  // no headers in headers
}

OUString AccessibleBrowseBoxHeaderBar::getAccessibleColumnDescription( sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return OUString();  // no headers in headers
}

Reference< XAccessibleTable > AccessibleBrowseBoxHeaderBar::getAccessibleRowHeaders()
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    return nullptr;        // no headers in headers
}

Reference< XAccessibleTable > AccessibleBrowseBoxHeaderBar::getAccessibleColumnHeaders()
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    return nullptr;        // no headers in headers
}

Sequence< sal_Int32 > AccessibleBrowseBoxHeaderBar::getSelectedAccessibleRows()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    // row of column header bar not selectable
    if( isRowBar() )
        implGetSelectedRows( aSelSeq );
    return aSelSeq;
}

Sequence< sal_Int32 > AccessibleBrowseBoxHeaderBar::getSelectedAccessibleColumns()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    // column of row header bar ("handle column") not selectable
    if( isColumnBar() )
        implGetSelectedColumns( aSelSeq );
    return aSelSeq;
}

bool AccessibleBrowseBoxHeaderBar::isAccessibleRowSelected( sal_Int32 nRow )
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    ensureIsValidRow( nRow );
    return isRowBar() && implIsRowSelected( nRow );
}

bool AccessibleBrowseBoxHeaderBar::isAccessibleColumnSelected( sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return isColumnBar() && implIsColumnSelected( nColumn );
}

Reference< XAccessible > AccessibleBrowseBoxHeaderBar::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );

    return implGetChild( nRow, implToVCLColumnPos( nColumn ) );
}

bool AccessibleBrowseBoxHeaderBar::isAccessibleSelected(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidAddress( nRow, nColumn );
    return isRowBar() ? implIsRowSelected( nRow ) : implIsColumnSelected( nColumn );
}

// XAccessibleSelection -------------------------------------------------------

void AccessibleBrowseBoxHeaderBar::selectAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidHeaderIndex( nChildIndex );
    if( isRowBar() )
        implSelectRow( nChildIndex, true );
    else
        implSelectColumn( implToVCLColumnPos( nChildIndex ), true );
}

bool AccessibleBrowseBoxHeaderBar::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    // using interface methods - no mutex
    return isRowBar() ?
        isAccessibleRowSelected( nChildIndex ) :
        isAccessibleColumnSelected( nChildIndex );
}

void AccessibleBrowseBoxHeaderBar::clearAccessibleSelection()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    mpBrowseBox->SetNoSelection();
}

void AccessibleBrowseBoxHeaderBar::selectAllAccessibleChildren()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    // no multiselection of columns possible
    if( isRowBar() )
        mpBrowseBox->SelectAll();
    else
        implSelectColumn( implToVCLColumnPos( 0 ), true );
}

sal_Int64 AccessibleBrowseBoxHeaderBar::getSelectedAccessibleChildCount()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    return isRowBar() ? implGetSelectedRowCount() : implGetSelectedColumnCount();
}

Reference< XAccessible >
AccessibleBrowseBoxHeaderBar::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    // method may throw lang::IndexOutOfBoundsException
    sal_Int64 nIndex = implGetChildIndexFromSelectedIndex( nSelectedChildIndex );
    assert(nIndex < std::numeric_limits<sal_Int32>::max());
    return implGetChild( nIndex, implToVCLColumnPos( nIndex ) );
}

void AccessibleBrowseBoxHeaderBar::deselectAccessibleChild(
        sal_Int64 nSelectedChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    // method may throw lang::IndexOutOfBoundsException
    if ( isAccessibleChildSelected(nSelectedChildIndex) )
    {
        if( isRowBar() )
            implSelectRow( nSelectedChildIndex, false );
        else
            implSelectColumn( implToVCLColumnPos( nSelectedChildIndex ), false );
    }
}

// XInterface -----------------------------------------------------------------

Any AccessibleBrowseBoxHeaderBar::queryInterface( const cpo::uno::Type& rType )
{
    Any aAny( AccessibleBrowseBoxTableBase::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleBrowseBoxHeaderBarImplHelper::queryInterface( rType );
}

void AccessibleBrowseBoxHeaderBar::acquire() noexcept
{
    AccessibleBrowseBoxTableBase::acquire();
}

void AccessibleBrowseBoxHeaderBar::release() noexcept
{
    AccessibleBrowseBoxTableBase::release();
}

// XServiceInfo ---------------------------------------------------------------

OUString AccessibleBrowseBoxHeaderBar::getImplementationName()
{
    return u"com.sun.star.comp.svtools.AccessibleBrowseBoxHeaderBar"_ustr;
}

// internal virtual methods ---------------------------------------------------

tools::Rectangle AccessibleBrowseBoxHeaderBar::implGetBoundingBox()
{
    return mpBrowseBox->calcHeaderRect(isColumnBar());
}

sal_Int32 AccessibleBrowseBoxHeaderBar::implGetRowCount() const
{
    // column header bar: only 1 row
    return isRowBar() ? AccessibleBrowseBoxTableBase::implGetRowCount() : 1;
}

sal_Int32 AccessibleBrowseBoxHeaderBar::implGetColumnCount() const
{
    // row header bar ("handle column"): only 1 column
    return isColumnBar() ? AccessibleBrowseBoxTableBase::implGetColumnCount() : 1;
}

// internal helper methods ----------------------------------------------------

Reference< XAccessible > AccessibleBrowseBoxHeaderBar::implGetChild(
        sal_Int32 nRow, sal_uInt16 nColumnPos )
{
    return isRowBar() ?
        mpBrowseBox->CreateAccessibleRowHeader( nRow ) :
        mpBrowseBox->CreateAccessibleColumnHeader( nColumnPos );
}

sal_Int64 AccessibleBrowseBoxHeaderBar::implGetChildIndexFromSelectedIndex(
        sal_Int64 nSelectedChildIndex )
{
    Sequence< sal_Int32 > aSelSeq;
    if( isRowBar() )
        implGetSelectedRows( aSelSeq );
    else
        implGetSelectedColumns( aSelSeq );

    if( (nSelectedChildIndex < 0) || (nSelectedChildIndex >= aSelSeq.getLength()) )
        throw lang::IndexOutOfBoundsException();

    return aSelSeq[ nSelectedChildIndex ];
}

void AccessibleBrowseBoxHeaderBar::ensureIsValidHeaderIndex( sal_Int32 nIndex )
{
    if( isRowBar() )
        ensureIsValidRow( nIndex );
    else
        ensureIsValidColumn( nIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
