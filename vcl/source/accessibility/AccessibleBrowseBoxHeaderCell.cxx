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


#include <accessibility/AccessibleBrowseBoxHeaderCell.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

AccessibleBrowseBoxHeaderCell::AccessibleBrowseBoxHeaderCell(sal_Int32 _nColumnRowId,
                                  const Reference< XAccessible >& rxParent,
                                  vcl::IAccessibleTableProvider& rBrowseBox,
                                  AccessibleBrowseBoxObjType eObjType)
: BrowseBoxAccessibleElement(rxParent,
                          rBrowseBox,
                          eObjType,
                          rBrowseBox.GetAccessibleObjectName( eObjType ,_nColumnRowId),
                          rBrowseBox.GetAccessibleObjectDescription( eObjType ,_nColumnRowId))
, m_nColumnRowId(_nColumnRowId)
{
}
/** Return a bitset of states of the current object.
*/
sal_Int64 AccessibleBrowseBoxHeaderCell::implCreateStateSet()
{
    SolarMethodGuard aGuard( getMutex() );

    sal_Int64 nStateSet = 0;

    if( isAlive() )
    {
        // SHOWING done with mxParent
        if( implIsShowing() )
            nStateSet |= AccessibleStateType::SHOWING;

        mpBrowseBox->FillAccessibleStateSet( nStateSet, getType() );
        nStateSet |= AccessibleStateType::VISIBLE;
        nStateSet |= AccessibleStateType::FOCUSABLE;
        nStateSet |= AccessibleStateType::TRANSIENT;
        nStateSet |= AccessibleStateType::SELECTABLE;

        bool bSelected = isRowBarCell() ? mpBrowseBox->IsRowSelected(m_nColumnRowId) : mpBrowseBox->IsColumnSelected(m_nColumnRowId);
        if ( bSelected )
            nStateSet |= AccessibleStateType::SELECTED;
    }
    else
        nStateSet |= AccessibleStateType::DEFUNC;

    return nStateSet;
}

/** @return
        The count of visible children.
*/
sal_Int64 SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleChildCount()
{
    return 0;
}


/** @return
        The XAccessible interface of the specified child.
*/
Reference<XAccessible > SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleChild( sal_Int64 )
{
    throw IndexOutOfBoundsException();
}


/** Grabs the focus to the column header. */
void SAL_CALL AccessibleBrowseBoxHeaderCell::grabFocus()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    if ( isRowBarCell() )
        mpBrowseBox->SelectRow(m_nColumnRowId);
    else
        mpBrowseBox->SelectColumn(static_cast<sal_uInt16>(m_nColumnRowId)); //!!!
}

/** @return
        The name of this class.
*/
OUString SAL_CALL AccessibleBrowseBoxHeaderCell::getImplementationName()
{
    return u"com.sun.star.comp.svtools.AccessibleBrowseBoxHeaderCell"_ustr;
}

tools::Rectangle AccessibleBrowseBoxHeaderCell::implGetBoundingBox()
{
    sal_Int32 nRow  = 0;
    sal_uInt16 nCol =  static_cast<sal_uInt16>(m_nColumnRowId);
    if (isRowBarCell())
    {
        nRow = m_nColumnRowId + 1;
        nCol = 0;
    }

    tools::Rectangle aRet(mpBrowseBox->GetFieldRectPixel(nRow , nCol, true, false));
    return tools::Rectangle(aRet.TopLeft() - Point(0, aRet.GetHeight()), aRet.GetSize());
}

sal_Int64 SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleIndexInParent()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    sal_Int64 nIndex = m_nColumnRowId;
    if ( mpBrowseBox->HasRowHeader() )
        --nIndex;
    return nIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
