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

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <controls/table/AccessibleGridControlHeaderCell.hxx>
#include <vcl/svapp.hxx>

namespace accessibility
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::uno;
    using namespace ::vcl;

AccessibleGridControlHeaderCell::AccessibleGridControlHeaderCell(sal_Int32 _nColumnRowId,
                                  const Reference< XAccessible >& rxParent,
                                  svt::table::TableControl& rTable,
                                  AccessibleTableControlObjType  eObjType)
    : ImplInheritanceHelper(rxParent, rTable,
                            (eObjType == AccessibleTableControlObjType::ROWHEADERCELL) ? _nColumnRowId : 0,
                            (eObjType == AccessibleTableControlObjType::ROWHEADERCELL) ? 0 : _nColumnRowId,
                            eObjType)
    , m_nColumnRowId(_nColumnRowId)
{
    assert(eObjType == AccessibleTableControlObjType::ROWHEADERCELL || eObjType == AccessibleTableControlObjType::COLUMNHEADERCELL);
}

/** Return a bitset of states of the current object.
*/
sal_Int64 AccessibleGridControlHeaderCell::implCreateStateSet()
{
    sal_Int64 nStateSet = 0;

    if( isAlive() )
    {
        // SHOWING done with mxParent
        if( implIsShowing() )
            nStateSet |= AccessibleStateType::SHOWING;

        nStateSet |= AccessibleStateType::VISIBLE;
        nStateSet |= AccessibleStateType::FOCUSABLE;
        nStateSet |= AccessibleStateType::TRANSIENT;
        nStateSet |= AccessibleStateType::SELECTABLE;

        if ( m_aTable.IsRowSelected(m_nColumnRowId) )
            nStateSet |= AccessibleStateType::SELECTED;
    }
    else
        nStateSet |= AccessibleStateType::DEFUNC;

    return nStateSet;
}

/** @return
        The count of visible children.
*/
sal_Int64 SAL_CALL AccessibleGridControlHeaderCell::getAccessibleChildCount()
{
    return 0;
}


/** @return
        The XAccessible interface of the specified child.
*/
Reference<XAccessible > SAL_CALL AccessibleGridControlHeaderCell::getAccessibleChild( sal_Int64 )
{
    throw IndexOutOfBoundsException();
}

        /** @return  The XAccessibleContext interface of this object. */
    Reference< css::accessibility::XAccessibleContext > SAL_CALL AccessibleGridControlHeaderCell::getAccessibleContext()
    {
        ensureAlive();
        return this;
    }


/** Grabs the focus to the column header. */
void SAL_CALL AccessibleGridControlHeaderCell::grabFocus()
{
}

/** @return
        The name of this class.
*/
OUString SAL_CALL AccessibleGridControlHeaderCell::getImplementationName()
{
    return u"com.sun.star.accessibility.AccessibleGridControlHeaderCell"_ustr;
}

AbsoluteScreenPixelRectangle AccessibleGridControlHeaderCell::implGetBoundingBoxOnScreen()
{
    AbsoluteScreenPixelRectangle aGridRect( m_aTable.GetWindowExtentsAbsolute() );
    sal_Int32 nIndex = getAccessibleIndexInParent();
    tools::Rectangle aCellRect;
    if (m_eObjType == AccessibleTableControlObjType::COLUMNHEADERCELL)
        aCellRect = m_aTable.calcHeaderCellRect(true, nIndex);
    else
        aCellRect = m_aTable.calcHeaderCellRect(false, nIndex);
    return AbsoluteScreenPixelRectangle(AbsoluteScreenPixelPoint(aGridRect.Left()+aCellRect.Left(),aGridRect.Top()+aCellRect.Top()), aCellRect.GetSize());
}

sal_Int64 SAL_CALL AccessibleGridControlHeaderCell::getAccessibleIndexInParent()
{
    SolarMutexGuard g;

    ensureAlive();
    return m_nColumnRowId;
}

} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
