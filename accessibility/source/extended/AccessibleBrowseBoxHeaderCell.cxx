/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

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

        BBSolarGuard aSolarGuard;
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
    BBSolarGuard aSolarGuard;
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
::rtl::OUString SAL_CALL AccessibleBrowseBoxHeaderCell::getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.AccessibleBrowseBoxHeaderCell" ) );
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


