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

#include <extended/AccessibleBrowseBoxTable.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>


using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;


namespace accessibility {


// Ctor/Dtor/disposing --------------------------------------------------------

AccessibleBrowseBoxTable::AccessibleBrowseBoxTable(
        const Reference< XAccessible >& rxParent,
        vcl::IAccessibleTableProvider& rBrowseBox ) :
    AccessibleBrowseBoxTableBase( rxParent, rBrowseBox, vcl::BBTYPE_TABLE )
{
}

AccessibleBrowseBoxTable::~AccessibleBrowseBoxTable()
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleBrowseBoxTable::getAccessibleChild( sal_Int32 nChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidIndex( nChildIndex );
    return mpBrowseBox->CreateAccessibleCell(
        implGetRow( nChildIndex ), static_cast<sal_Int16>(implGetColumn( nChildIndex )) );
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTable::getAccessibleIndexInParent()
{
    osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return vcl::BBINDEX_TABLE;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleBrowseBoxTable::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Reference< XAccessible > xChild;
    sal_Int32 nRow = 0;
    sal_uInt16 nColumnPos = 0;
    if( mpBrowseBox->ConvertPointToCellAddress( nRow, nColumnPos, VCLPoint( rPoint ) ) )
        xChild = mpBrowseBox->CreateAccessibleCell( nRow, nColumnPos );

    return xChild;
}

void SAL_CALL AccessibleBrowseBoxTable::grabFocus()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    mpBrowseBox->GrabTableFocus();
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleBrowseBoxTable::getAccessibleRowDescription( sal_Int32 nRow )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    ensureIsValidRow( nRow );
    return mpBrowseBox->GetRowDescription( nRow );
}

OUString SAL_CALL AccessibleBrowseBoxTable::getAccessibleColumnDescription( sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidColumn( nColumn );
    return mpBrowseBox->GetColumnDescription( static_cast<sal_uInt16>(nColumn) );
}

Reference< XAccessibleTable > SAL_CALL AccessibleBrowseBoxTable::getAccessibleRowHeaders()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return implGetHeaderBar( vcl::BBINDEX_ROWHEADERBAR );
}

Reference< XAccessibleTable > SAL_CALL AccessibleBrowseBoxTable::getAccessibleColumnHeaders()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return implGetHeaderBar( vcl::BBINDEX_COLUMNHEADERBAR );
}

Sequence< sal_Int32 > SAL_CALL AccessibleBrowseBoxTable::getSelectedAccessibleRows()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    implGetSelectedRows( aSelSeq );
    return aSelSeq;
}

Sequence< sal_Int32 > SAL_CALL AccessibleBrowseBoxTable::getSelectedAccessibleColumns()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    implGetSelectedColumns( aSelSeq );
    return aSelSeq;
}

sal_Bool SAL_CALL AccessibleBrowseBoxTable::isAccessibleRowSelected( sal_Int32 nRow )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidRow( nRow );
    return implIsRowSelected( nRow );
}

sal_Bool SAL_CALL AccessibleBrowseBoxTable::isAccessibleColumnSelected( sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidColumn( nColumn );
    return implIsColumnSelected( nColumn );
}

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxTable::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidAddress( nRow, nColumn );
    return mpBrowseBox->CreateAccessibleCell( nRow, static_cast<sal_Int16>(nColumn) );
}

sal_Bool SAL_CALL AccessibleBrowseBoxTable::isAccessibleSelected(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    ensureIsValidAddress( nRow, nColumn );
    return implIsRowSelected( nRow ) || implIsColumnSelected( nColumn );
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleBrowseBoxTable::getImplementationName()
{
    return OUString( "com.sun.star.comp.svtools.AccessibleBrowseBoxTable" );
}

// internal virtual methods ---------------------------------------------------

tools::Rectangle AccessibleBrowseBoxTable::implGetBoundingBox()
{
    return mpBrowseBox->calcTableRect(false);
}

tools::Rectangle AccessibleBrowseBoxTable::implGetBoundingBoxOnScreen()
{
    return mpBrowseBox->calcTableRect();
}

// internal helper methods ----------------------------------------------------

Reference< XAccessibleTable > AccessibleBrowseBoxTable::implGetHeaderBar(
        sal_Int32 nChildIndex )
{
    Reference< XAccessible > xRet;
    Reference< XAccessibleContext > xContext( mxParent, uno::UNO_QUERY );
    if( xContext.is() )
    {
        try
        {
            xRet = xContext->getAccessibleChild( nChildIndex );
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            OSL_FAIL( "implGetHeaderBar - wrong child index" );
        }
        // RuntimeException goes to caller
    }
    return Reference< XAccessibleTable >( xRet, uno::UNO_QUERY );
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
