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

#include <controls/table/AccessibleGridControlHeader.hxx>
#include <controls/table/AccessibleGridControlHeaderCell.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/accessibletable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;
using namespace ::vcl::table;


namespace accessibility {


AccessibleGridControlHeader::AccessibleGridControlHeader(
        const Reference< XAccessible >& rxParent,
        ::vcl::table::IAccessibleTable&                      rTable,
        ::vcl::table::AccessibleTableControlObjType      eObjType):
        AccessibleGridControlTableBase( rxParent, rTable, eObjType )
{
    OSL_ENSURE( isRowBar() || isColumnBar(),
        "AccessibleGridControlHeaderBar - invalid object type" );
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlHeader::getAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    if (nChildIndex<0 || nChildIndex>=getAccessibleChildCount())
        throw IndexOutOfBoundsException();
    ensureIsAlive();
    Reference< XAccessible > xChild;
    if (m_eObjType == vcl::table::AccessibleTableControlObjType::COLUMNHEADERBAR)
    {
        rtl::Reference<AccessibleGridControlHeaderCell> pColHeaderCell = new AccessibleGridControlHeaderCell(nChildIndex, this, m_aTable,
                                                                                                             vcl::table::AccessibleTableControlObjType::COLUMNHEADERCELL);
        xChild = pColHeaderCell;
    }
    else if (m_eObjType == vcl::table::AccessibleTableControlObjType::ROWHEADERBAR)
    {
        rtl::Reference<AccessibleGridControlHeaderCell> pRowHeaderCell = new AccessibleGridControlHeaderCell(nChildIndex, this, m_aTable,
                                                                                                             vcl::table::AccessibleTableControlObjType::ROWHEADERCELL);
        xChild = pRowHeaderCell;
    }
    return xChild;
}

sal_Int64 SAL_CALL AccessibleGridControlHeader::getAccessibleIndexInParent()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    if (m_eObjType == vcl::table::AccessibleTableControlObjType::ROWHEADERBAR && m_aTable.HasColHeader())
        return 1;
    else
        return 0;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlHeader::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();

    sal_Int32 nRow = 0;
    sal_Int32 nColumnPos = 0;
    bool bConverted = m_aTable.ConvertPointToCellAddress(nRow, nColumnPos,
                                                         vcl::unohelper::ConvertToVCLPoint(rPoint));
    return bConverted ? implGetChild( nRow, nColumnPos ) : Reference< XAccessible >();
}

void SAL_CALL AccessibleGridControlHeader::grabFocus()
{
    ensureIsAlive();
    // focus on header not supported
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleGridControlHeader::getAccessibleRowDescription( sal_Int32 nRow )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidRow( nRow );
    return OUString();  // no headers in headers
}

OUString SAL_CALL AccessibleGridControlHeader::getAccessibleColumnDescription( sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return OUString();  // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlHeader::getAccessibleRowHeaders()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return nullptr;        // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlHeader::getAccessibleColumnHeaders()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return nullptr;        // no headers in headers
}
//not selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlHeader::getSelectedAccessibleRows()
{
    return {};
}
//columns aren't selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlHeader::getSelectedAccessibleColumns()
{
    return {};
}
//row headers not selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleRowSelected( sal_Int32 /*nRow*/ )
{
    return false;
}
//columns aren't selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleColumnSelected( sal_Int32 )
{
    return false;
}

Reference< XAccessible > SAL_CALL AccessibleGridControlHeader::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn)
{
    SolarMutexGuard g;

    ensureIsAlive();
    ensureIsValidAddress(nRow, nColumn);
    return implGetChild(nRow, nColumn);
}
// not selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleSelected(
        sal_Int32 /*nRow*/, sal_Int32 /*nColumn */)
{
    return false;
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControlHeader::getImplementationName()
{
    return u"com.sun.star.accessibility.AccessibleGridControlHeader"_ustr;
}

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlHeader::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// internal virtual methods ---------------------------------------------------

tools::Rectangle AccessibleGridControlHeader::implGetBoundingBox()
{
    vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
    tools::Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( *pParent ) );
    tools::Rectangle aHeaderRect (m_aTable.calcHeaderRect(isColumnBar()));
    if(isColumnBar())
        return tools::Rectangle(aGridRect.TopLeft(), Size(aGridRect.getOpenWidth(),aHeaderRect.getOpenHeight()));
    else
        return tools::Rectangle(aGridRect.TopLeft(), Size(aHeaderRect.getOpenWidth(),aGridRect.getOpenHeight()));

}

AbsoluteScreenPixelRectangle AccessibleGridControlHeader::implGetBoundingBoxOnScreen()
{
    AbsoluteScreenPixelRectangle aGridRect( m_aTable.GetWindowExtentsAbsolute() );
    tools::Rectangle aHeaderRect (m_aTable.calcHeaderRect(isColumnBar()));
    if(isColumnBar())
        return AbsoluteScreenPixelRectangle(aGridRect.TopLeft(), Size(aGridRect.getOpenWidth(),aHeaderRect.getOpenHeight()));
    else
        return AbsoluteScreenPixelRectangle(aGridRect.TopLeft(), Size(aHeaderRect.getOpenWidth(),aGridRect.getOpenHeight()));
}

// internal helper methods ----------------------------------------------------
Reference< XAccessible > AccessibleGridControlHeader::implGetChild(
        sal_Int32 nRow, sal_uInt32 nColumnPos )
{
    Reference< XAccessible > xChild;
    if (m_eObjType == vcl::table::AccessibleTableControlObjType::COLUMNHEADERBAR)
    {
        rtl::Reference<AccessibleGridControlHeaderCell> pColHeaderCell = new AccessibleGridControlHeaderCell(nColumnPos, this, m_aTable,
                                                                                                             vcl::table::AccessibleTableControlObjType::COLUMNHEADERCELL);
        xChild = pColHeaderCell;
    }
    else if (m_eObjType == vcl::table::AccessibleTableControlObjType::ROWHEADERBAR)
    {
        rtl::Reference<AccessibleGridControlHeaderCell> pRowHeaderCell = new AccessibleGridControlHeaderCell(nRow, this, m_aTable,
                                                                                                             vcl::table::AccessibleTableControlObjType::ROWHEADERCELL);
        xChild = pRowHeaderCell;
    }
    return xChild;
}

} // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
