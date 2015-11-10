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

#include "accessibility/extended/AccessibleGridControlHeader.hxx"
#include "accessibility/extended/AccessibleGridControlHeaderCell.hxx"
#include "accessibility/extended/AccessibleGridControlTableCell.hxx"
#include <svtools/accessibletable.hxx>
#include <comphelper/servicehelper.hxx>




using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;
using namespace ::svt::table;



namespace accessibility {



AccessibleGridControlHeader::AccessibleGridControlHeader(
        const Reference< XAccessible >& rxParent,
        ::svt::table::IAccessibleTable&                      rTable,
        ::svt::table::AccessibleTableControlObjType      eObjType):
        AccessibleGridControlTableBase( rxParent, rTable, eObjType )
{
    OSL_ENSURE( isRowBar() || isColumnBar(),
        "accessibility/extended/AccessibleGridControlHeaderBar - invalid object type" );
}

AccessibleGridControlHeader::~AccessibleGridControlHeader()
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlHeader::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;

    if (nChildIndex<0 || nChildIndex>=getAccessibleChildCount())
        throw IndexOutOfBoundsException();
    ensureIsAlive();
    Reference< XAccessible > xChild;
    if(m_eObjType == svt::table::TCTYPE_COLUMNHEADERBAR)
    {
        AccessibleGridControlHeaderCell* pColHeaderCell = new AccessibleGridControlHeaderCell(nChildIndex, this, m_aTable, svt::table::TCTYPE_COLUMNHEADERCELL);
        xChild = pColHeaderCell;
    }
    else if(m_eObjType == svt::table::TCTYPE_ROWHEADERBAR)
    {
        AccessibleGridControlHeaderCell* pRowHeaderCell = new AccessibleGridControlHeaderCell(nChildIndex, this, m_aTable, svt::table::TCTYPE_ROWHEADERCELL);
        xChild = pRowHeaderCell;
    }
    return xChild;
}

sal_Int32 SAL_CALL AccessibleGridControlHeader::getAccessibleIndexInParent()
    throw ( uno::RuntimeException, std::exception )
{
     ensureIsAlive();
     if(m_eObjType == svt::table::TCTYPE_ROWHEADERBAR && m_aTable.HasColHeader())
         return 1;
     else
         return 0;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlHeader::getAccessibleAtPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();

    sal_Int32 nRow = 0;
    sal_Int32 nColumnPos = 0;
    bool bConverted = m_aTable.ConvertPointToCellAddress(nRow, nColumnPos, VCLPoint(rPoint));
    return bConverted ? implGetChild( nRow, nColumnPos ) : Reference< XAccessible >();
}

void SAL_CALL AccessibleGridControlHeader::grabFocus()
    throw ( uno::RuntimeException, std::exception )
{
    ensureIsAlive();
    // focus on header not supported
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleGridControlHeader::getAccessibleRowDescription( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidRow( nRow );
    return OUString();  // no headers in headers
}

OUString SAL_CALL AccessibleGridControlHeader::getAccessibleColumnDescription( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return OUString();  // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlHeader::getAccessibleRowHeaders()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    return nullptr;        // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlHeader::getAccessibleColumnHeaders()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    return nullptr;        // no headers in headers
}
//not selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlHeader::getSelectedAccessibleRows()
    throw ( uno::RuntimeException, std::exception )
{
    Sequence< sal_Int32 > aSelSeq(0);
    return aSelSeq;
}
//columns aren't selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlHeader::getSelectedAccessibleColumns()
    throw ( uno::RuntimeException, std::exception )
{
    Sequence< sal_Int32 > aSelSeq(0);
    return aSelSeq;
}
//row headers not selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleRowSelected( sal_Int32 /*nRow*/ )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    return false;
}
//columns aren't selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleColumnSelected( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    (void)nColumn;
    return false;
}
//not implemented
Reference< XAccessible > SAL_CALL AccessibleGridControlHeader::getAccessibleCellAt(
        sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    return nullptr;
}
// not selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleSelected(
        sal_Int32 /*nRow*/, sal_Int32 /*nColumn */)
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    return false;
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControlHeader::getImplementationName()
    throw ( uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.accessibility.AccessibleGridControlHeader" );
}

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlHeader::getImplementationId()
    throw ( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

// internal virtual methods ---------------------------------------------------

Rectangle AccessibleGridControlHeader::implGetBoundingBox()
{
    vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
    Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( pParent ) );
    Rectangle aHeaderRect (m_aTable.calcHeaderRect(isColumnBar()));
    if(isColumnBar())
        return Rectangle(aGridRect.TopLeft(), Size(aGridRect.getWidth(),aHeaderRect.getHeight()));
    else
        return Rectangle(aGridRect.TopLeft(), Size(aHeaderRect.getWidth(),aGridRect.getHeight()));

}

Rectangle AccessibleGridControlHeader::implGetBoundingBoxOnScreen()
{
    Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( nullptr ) );
    Rectangle aHeaderRect (m_aTable.calcHeaderRect(isColumnBar()));
    if(isColumnBar())
        return Rectangle(aGridRect.TopLeft(), Size(aGridRect.getWidth(),aHeaderRect.getHeight()));
    else
        return Rectangle(aGridRect.TopLeft(), Size(aHeaderRect.getWidth(),aGridRect.getHeight()));
}

// internal helper methods ----------------------------------------------------
Reference< XAccessible > AccessibleGridControlHeader::implGetChild(
        sal_Int32 nRow, sal_uInt32 nColumnPos )
{
    Reference< XAccessible > xChild;
    if(m_eObjType == svt::table::TCTYPE_COLUMNHEADERBAR)
    {
        AccessibleGridControlHeaderCell* pColHeaderCell = new AccessibleGridControlHeaderCell(nColumnPos, this, m_aTable, svt::table::TCTYPE_COLUMNHEADERCELL);
        xChild = pColHeaderCell;
    }
    else if(m_eObjType == svt::table::TCTYPE_ROWHEADERBAR)
    {
        AccessibleGridControlHeaderCell* pRowHeaderCell = new AccessibleGridControlHeaderCell(nRow, this, m_aTable, svt::table::TCTYPE_ROWHEADERCELL);
        xChild = pRowHeaderCell;
    }
    return xChild;
}

} // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
