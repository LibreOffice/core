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

#include "accessibility/extended/AccessibleGridControl.hxx"
#include "accessibility/extended/AccessibleGridControlTable.hxx"
#include "accessibility/extended/AccessibleGridControlHeader.hxx"
#include <svtools/accessibletable.hxx>
#include <comphelper/types.hxx>
#include <toolkit/helper/vclunohelper.hxx>

// ============================================================================

namespace accessibility
{

// ============================================================================

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;
using namespace ::svt::table;

// ============================================================================
class AccessibleGridControl_Impl
{
public:
    /// the XAccessible which created the AccessibleGridControl
    WeakReference< XAccessible >                                m_aCreator;

    /** The data table child. */
    Reference<
        ::com::sun::star::accessibility::XAccessible >          m_xTable;
    AccessibleGridControlTable*             m_pTable;

    /** The header bar for rows. */
    Reference<
        ::com::sun::star::accessibility::XAccessible >          m_xRowHeaderBar;
    AccessibleGridControlHeader*                m_pRowHeaderBar;

    /** The header bar for columns (first row of the table). */
    Reference<
        ::com::sun::star::accessibility::XAccessible >          m_xColumnHeaderBar;
    AccessibleGridControlHeader*                m_pColumnHeaderBar;
};

AccessibleGridControl::AccessibleGridControl(
            const Reference< XAccessible >& _rxParent, const Reference< XAccessible >& _rxCreator,
            IAccessibleTable& _rTable )
    : AccessibleGridControlBase( _rxParent, _rTable, TCTYPE_GRIDCONTROL )
{
    m_pImpl.reset( new AccessibleGridControl_Impl() );
    m_pImpl->m_aCreator = _rxCreator;
}

// -----------------------------------------------------------------------------
AccessibleGridControl::~AccessibleGridControl()
{
}
// -----------------------------------------------------------------------------

void SAL_CALL AccessibleGridControl::disposing()
{
    ::osl::MutexGuard aGuard( getOslMutex() );

    m_pImpl->m_pTable       = NULL;
    m_pImpl->m_pColumnHeaderBar = NULL;
    m_pImpl->m_pRowHeaderBar    = NULL;
    m_pImpl->m_aCreator         = Reference< XAccessible >();

    Reference< XAccessible >  xTable = m_pImpl->m_xTable;

    Reference< XComponent > xComp( m_pImpl->m_xTable, UNO_QUERY );
    if ( xComp.is() )
    {
        xComp->dispose();
    }

    AccessibleGridControlBase::disposing();
}
// -----------------------------------------------------------------------------

// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL AccessibleGridControl::getAccessibleChildCount()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return m_aTable.GetAccessibleControlCount();
}
// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );

    if (nChildIndex<0 || nChildIndex>=getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    if (isAlive())
    {
        if(nChildIndex == 0 && m_aTable.HasColHeader())
        {
            if(!m_pImpl->m_xColumnHeaderBar.is())
            {
                AccessibleGridControlHeader* pColHeaderBar = new AccessibleGridControlHeader(m_pImpl->m_aCreator, m_aTable, svt::table::TCTYPE_COLUMNHEADERBAR);
                m_pImpl->m_xColumnHeaderBar = pColHeaderBar;
            }
            xChild = m_pImpl->m_xColumnHeaderBar;
        }
        else if(m_aTable.HasRowHeader() && (nChildIndex == 1 || nChildIndex == 0))
        {
            if(!m_pImpl->m_xRowHeaderBar.is())
            {
                AccessibleGridControlHeader* pRowHeaderBar = new AccessibleGridControlHeader(m_pImpl->m_aCreator, m_aTable, svt::table::TCTYPE_ROWHEADERBAR);
                m_pImpl->m_xRowHeaderBar = pRowHeaderBar;
            }
            xChild = m_pImpl->m_xRowHeaderBar;
        }
        else
        {
            AccessibleGridControlTable* pTable = new AccessibleGridControlTable(m_pImpl->m_aCreator, m_aTable, svt::table::TCTYPE_TABLE);
            m_pImpl->m_xTable = pTable;
            xChild = m_pImpl->m_xTable;
        }
    }
    return xChild;
}
// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL AccessibleGridControl::getAccessibleRole()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return AccessibleRole::PANEL;
}
// -----------------------------------------------------------------------------

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleAtPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Reference< XAccessible > xChild;
    sal_Int32 nIndex = 0;
    if( m_aTable.ConvertPointToControlIndex( nIndex, VCLPoint( rPoint ) ) )
        xChild = m_aTable.CreateAccessibleControl( nIndex );
    else
    {
        // try whether point is in one of the fixed children
        // (table, header bars, corner control)
        Point aPoint( VCLPoint( rPoint ) );
        for( nIndex = 0; (nIndex < 3) && !xChild.is(); ++nIndex )
        {
            Reference< XAccessible > xCurrChild( implGetFixedChild( nIndex ) );
            Reference< XAccessibleComponent >
            xCurrChildComp( xCurrChild, uno::UNO_QUERY );

            if( xCurrChildComp.is() &&
                VCLRectangle( xCurrChildComp->getBounds() ).IsInside( aPoint ) )
            xChild = xCurrChild;
        }
    }
    return xChild;
}
// -----------------------------------------------------------------------------

void SAL_CALL AccessibleGridControl::grabFocus()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
        m_aTable.GrabFocus();
}
// -----------------------------------------------------------------------------

Any SAL_CALL AccessibleGridControl::getAccessibleKeyBinding()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return Any();
}
// -----------------------------------------------------------------------------

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControl::getImplementationName()
    throw ( uno::RuntimeException )
{
    return OUString( "com.sun.star.accessibility.AccessibleGridControl" );
}
// -----------------------------------------------------------------------------

// internal virtual methods ---------------------------------------------------

Rectangle AccessibleGridControl::implGetBoundingBox()
{
    Window* pParent = m_aTable.GetAccessibleParentWindow();
    OSL_ENSURE( pParent, "implGetBoundingBox - missing parent window" );
    return m_aTable.GetWindowExtentsRelative( pParent );
}
// -----------------------------------------------------------------------------

Rectangle AccessibleGridControl::implGetBoundingBoxOnScreen()
{
    return m_aTable.GetWindowExtentsRelative( NULL );
}
// internal helper methods ----------------------------------------------------

Reference< XAccessible > AccessibleGridControl::implGetTable()
{
    if( !m_pImpl->m_xTable.is() )
    {
        m_pImpl->m_pTable = createAccessibleTable();
        m_pImpl->m_xTable  = m_pImpl->m_pTable;
    }
    return m_pImpl->m_xTable;
}
// -----------------------------------------------------------------------------

Reference< XAccessible >
AccessibleGridControl::implGetHeaderBar( AccessibleTableControlObjType eObjType )
{
    Reference< XAccessible > xRet;
    Reference< XAccessible >* pxMember = NULL;

    if( eObjType == TCTYPE_ROWHEADERBAR )
        pxMember = &m_pImpl->m_xRowHeaderBar;
    else if( eObjType ==  TCTYPE_COLUMNHEADERBAR )
        pxMember = &m_pImpl->m_xColumnHeaderBar;

    if( pxMember )
    {
        if( !pxMember->is() )
        {
            AccessibleGridControlHeader* pHeaderBar = new AccessibleGridControlHeader(
                (Reference< XAccessible >)m_pImpl->m_aCreator, m_aTable, eObjType );

            if ( TCTYPE_COLUMNHEADERBAR == eObjType)
                m_pImpl->m_pColumnHeaderBar = pHeaderBar;
            else
                m_pImpl->m_pRowHeaderBar    = pHeaderBar;

            *pxMember = pHeaderBar;
        }
        xRet = *pxMember;
    }
    return xRet;
}
// -----------------------------------------------------------------------------
Reference< XAccessible >
AccessibleGridControl::implGetFixedChild( sal_Int32 nChildIndex )
{
    Reference< XAccessible > xRet;
    switch( nChildIndex )
    {
          case TCINDEX_COLUMNHEADERBAR:
            xRet = implGetHeaderBar( TCTYPE_COLUMNHEADERBAR );
        break;
        case TCINDEX_ROWHEADERBAR:
            xRet = implGetHeaderBar( TCTYPE_ROWHEADERBAR );
        break;
        case TCINDEX_TABLE:
            xRet = implGetTable();
        break;
    }
    return xRet;
}
// -----------------------------------------------------------------------------
AccessibleGridControlTable* AccessibleGridControl::createAccessibleTable()
{
    Reference< XAccessible > xCreator = (Reference< XAccessible >)m_pImpl->m_aCreator;
        OSL_ENSURE( xCreator.is(), "accessibility/extended/AccessibleGirdControl::createAccessibleTable: my creator died - how this?" );
    return new AccessibleGridControlTable( xCreator, m_aTable, TCTYPE_TABLE );
}
// ============================================================================
// = AccessibleGridControlAccess
// ============================================================================

// -----------------------------------------------------------------------------
AccessibleGridControlAccess::AccessibleGridControlAccess( const Reference< XAccessible >& _rxParent, IAccessibleTable& _rTable )
        :m_xParent( _rxParent )
        ,m_rTable( _rTable )
        ,m_pContext( NULL )
{
}

// -----------------------------------------------------------------------------
AccessibleGridControlAccess::~AccessibleGridControlAccess()
{
}

// -----------------------------------------------------------------------------
void AccessibleGridControlAccess::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_pContext = NULL;
    ::comphelper::disposeComponent( m_xContext );
}

// -----------------------------------------------------------------------------
Reference< XAccessibleContext > SAL_CALL AccessibleGridControlAccess::getAccessibleContext() throw ( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OSL_ENSURE( ( m_pContext && m_xContext.is() ) || ( !m_pContext && !m_xContext.is() ),
        "accessibility/extended/AccessibleGridControlAccess::getAccessibleContext: inconsistency!" );

    // if the context died meanwhile (we're no listener, so it won't tell us explicitily when this happens),
    // then reset an re-create.
    if ( m_pContext && !m_pContext->isAlive() )
        m_xContext = m_pContext = NULL;

    if ( !m_xContext.is() )
        m_xContext = m_pContext = new AccessibleGridControl( m_xParent, this, m_rTable );

    return m_xContext;
}

// -----------------------------------------------------------------------------
bool AccessibleGridControlAccess::isContextAlive() const
{
    return  ( NULL != m_pContext ) && m_pContext->isAlive();
}

// ============================================================================

}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
