/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

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

        TCSolarGuard aSolarGuard;
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
::rtl::OUString SAL_CALL AccessibleGridControlHeaderCell::getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.accessibility.AccessibleGridControlHeaderCell" ) );
}
// -----------------------------------------------------------------------------
Rectangle AccessibleGridControlHeaderCell::implGetBoundingBox()
{
    Window* pParent = m_aTable.GetAccessibleParentWindow();
    Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( pParent ) );
    sal_Int32 nIndex = getAccessibleIndexInParent();
    Rectangle aCellRect;
    if(m_eObjType == TCTYPE_COLUMNHEADERCELL)
        aCellRect = m_aTable.calcHeaderCellRect(true, nIndex);
    else
        aCellRect = m_aTable.calcHeaderCellRect(false, nIndex);
    return Rectangle(Point(aGridRect.Left()+aCellRect.Left(),aGridRect.Top()+aCellRect.Top()), aCellRect.GetSize());
}
// -----------------------------------------------------------------------------

Rectangle AccessibleGridControlHeaderCell::implGetBoundingBoxOnScreen()
{
    Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( NULL ) );
    sal_Int32 nIndex = getAccessibleIndexInParent();
    Rectangle aCellRect;
    if(m_eObjType == TCTYPE_COLUMNHEADERCELL)
        aCellRect = m_aTable.calcHeaderCellRect(true, nIndex);
    else
        aCellRect = m_aTable.calcHeaderCellRect(false, nIndex);
    return Rectangle(Point(aGridRect.Left()+aCellRect.Left(),aGridRect.Top()+aCellRect.Top()), aCellRect.GetSize());
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


