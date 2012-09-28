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


#include "accessibility/extended/AccessibleBrowseBoxHeaderCell.hxx"
#include <svtools/accessibletableprovider.hxx>
#include "accessibility/extended/AccessibleBrowseBox.hxx"

namespace accessibility
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::uno;
    using namespace ::svt;

AccessibleBrowseBoxHeaderCell::AccessibleBrowseBoxHeaderCell(sal_Int32 _nColumnRowId,
                                  const Reference< XAccessible >& rxParent,
                                  IAccessibleTableProvider&                  rBrowseBox,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                                  AccessibleBrowseBoxObjType  eObjType)
: BrowseBoxAccessibleElement(rxParent,
                          rBrowseBox,
                          _xFocusWindow,
                          eObjType,
                          rBrowseBox.GetAccessibleObjectName( eObjType ,_nColumnRowId),
                          rBrowseBox.GetAccessibleObjectDescription( eObjType ,_nColumnRowId))
, m_nColumnRowId(_nColumnRowId)
{
}
/** Creates a new AccessibleStateSetHelper and fills it with states of the
    current object.
    @return
        A filled AccessibleStateSetHelper.
*/
::utl::AccessibleStateSetHelper* AccessibleBrowseBoxHeaderCell::implCreateStateSetHelper()
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ::utl::AccessibleStateSetHelper*
        pStateSetHelper = new ::utl::AccessibleStateSetHelper;

    if( isAlive() )
    {
        // SHOWING done with mxParent
        if( implIsShowing() )
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );

        SolarMutexGuard aSolarGuard;
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
        pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
        pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
        pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );

        sal_Bool bSelected = isRowBarCell() ? mpBrowseBox->IsRowSelected(m_nColumnRowId) : mpBrowseBox->IsColumnSelected(m_nColumnRowId);
        if ( bSelected )
            pStateSetHelper->AddState( AccessibleStateType::SELECTED );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}
// -----------------------------------------------------------------------------
/** @return
        The count of visible children.
*/
sal_Int32 SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleChildCount()
    throw ( RuntimeException )
{
    return 0;
}
// -----------------------------------------------------------------------------

/** @return
        The XAccessible interface of the specified child.
*/
Reference<XAccessible > SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleChild( sal_Int32 )
    throw ( IndexOutOfBoundsException,RuntimeException )
{
    throw IndexOutOfBoundsException();
}
// -----------------------------------------------------------------------------

/** Grabs the focus to the column header. */
void SAL_CALL AccessibleBrowseBoxHeaderCell::grabFocus()
    throw ( ::com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    if ( isRowBarCell() )
        mpBrowseBox->SelectRow(m_nColumnRowId);
    else
        mpBrowseBox->SelectColumn(static_cast<sal_uInt16>(m_nColumnRowId)); //!!!
}
// -----------------------------------------------------------------------------
/** @return
        The name of this class.
*/
OUString SAL_CALL AccessibleBrowseBoxHeaderCell::getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.svtools.AccessibleBrowseBoxHeaderCell" );
}
// -----------------------------------------------------------------------------
namespace
{
    Rectangle getRectangle(IAccessibleTableProvider* _pBrowseBox,sal_Int32 _nRowColIndex, sal_Bool _bOnScreen,sal_Bool _bRowBar)
    {
        sal_Int32 nRow  = 0;
        sal_uInt16 nCol =  (sal_uInt16)_nRowColIndex;
        if ( _bRowBar )
        {
            nRow = _nRowColIndex + 1;
            nCol = 0;
        }

        Rectangle aRet(_pBrowseBox->GetFieldRectPixelAbs( nRow , nCol, sal_True, _bOnScreen));
        return Rectangle(aRet.TopLeft() - Point(0,aRet.GetHeight()),aRet.GetSize());
    }
}

Rectangle AccessibleBrowseBoxHeaderCell::implGetBoundingBox()
{
    return getRectangle(mpBrowseBox,m_nColumnRowId,sal_False,isRowBarCell());
}
// -----------------------------------------------------------------------------

Rectangle AccessibleBrowseBoxHeaderCell::implGetBoundingBoxOnScreen()
{
    return getRectangle(mpBrowseBox,m_nColumnRowId,sal_True,isRowBarCell());
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL AccessibleBrowseBoxHeaderCell::getAccessibleIndexInParent()
    throw ( RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    sal_Int32 nIndex = m_nColumnRowId;
    if ( mpBrowseBox->HasRowHeader() )
        --nIndex;
    return nIndex;
}
// -----------------------------------------------------------------------------
} // namespace accessibility
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
