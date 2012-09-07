/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "accessibility/extended/AccessibleGridControlHeaderCell.hxx"
#include <svtools/accessibletable.hxx>
#include "accessibility/extended/AccessibleGridControl.hxx"

namespace accessibility
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::uno;
    using namespace ::svt;
    using namespace ::svt::table;

AccessibleGridControlHeaderCell::AccessibleGridControlHeaderCell(sal_Int32 _nColumnRowId,
                                  const Reference< XAccessible >& rxParent,
                                  IAccessibleTable& rTable,
                                  AccessibleTableControlObjType  eObjType)
: AccessibleGridControlCell( rxParent, rTable, _nColumnRowId, 0, eObjType)
, m_nColumnRowId(_nColumnRowId)
{
}
/** Creates a new AccessibleStateSetHelper and fills it with states of the
    current object.
    @return
        A filled AccessibleStateSetHelper.
*/
::utl::AccessibleStateSetHelper* AccessibleGridControlHeaderCell::implCreateStateSetHelper()
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

        if ( m_aTable.IsRowSelected(m_nColumnRowId) )
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
sal_Int32 SAL_CALL AccessibleGridControlHeaderCell::getAccessibleChildCount()
    throw ( RuntimeException )
{
    return 0;
}
// -----------------------------------------------------------------------------

/** @return
        The XAccessible interface of the specified child.
*/
Reference<XAccessible > SAL_CALL AccessibleGridControlHeaderCell::getAccessibleChild( sal_Int32 )
    throw ( IndexOutOfBoundsException,RuntimeException )
{
    throw IndexOutOfBoundsException();
}
// XInterface -------------------------------------------------------------

    /** Queries for a new interface. */
    ::com::sun::star::uno::Any SAL_CALL AccessibleGridControlHeaderCell::queryInterface(
            const ::com::sun::star::uno::Type& rType )
        throw ( ::com::sun::star::uno::RuntimeException )
    {
        Any aRet = AccessibleGridControlCell::queryInterface(rType);
        return aRet;
    }

    /** Aquires the object (calls acquire() on base class). */
    void SAL_CALL AccessibleGridControlHeaderCell::acquire() throw ()
    {
        AccessibleGridControlCell::acquire();
    }

    /** Releases the object (calls release() on base class). */
    void SAL_CALL AccessibleGridControlHeaderCell::release() throw ()
    {
        AccessibleGridControlCell::release();
    }
        /** @return  The XAccessibleContext interface of this object. */
    Reference< com::sun::star::accessibility::XAccessibleContext > SAL_CALL AccessibleGridControlHeaderCell::getAccessibleContext() throw ( RuntimeException )
    {
        ensureIsAlive();
        return this;
    }

// -----------------------------------------------------------------------------

/** Grabs the focus to the column header. */
void SAL_CALL AccessibleGridControlHeaderCell::grabFocus()
    throw ( ::com::sun::star::uno::RuntimeException )
{
}
// -----------------------------------------------------------------------------
/** @return
        The name of this class.
*/
OUString SAL_CALL AccessibleGridControlHeaderCell::getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException )
{
    return OUString( "com.sun.star.accessibility.AccessibleGridControlHeaderCell" );
}
// -----------------------------------------------------------------------------
Rectangle AccessibleGridControlHeaderCell::implGetBoundingBox()
{
    return Rectangle(Point(0,0),Point(0,0));//To Do - return headercell rectangle
}
// -----------------------------------------------------------------------------

Rectangle AccessibleGridControlHeaderCell::implGetBoundingBoxOnScreen()
{
    return Rectangle(Point(0,0),Point(0,0));//To Do - return headercell rectangle
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL AccessibleGridControlHeaderCell::getAccessibleIndexInParent()
    throw ( RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    sal_Int32 nIndex = m_nColumnRowId;
    return nIndex;
}
// -----------------------------------------------------------------------------
} // namespace accessibility
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
