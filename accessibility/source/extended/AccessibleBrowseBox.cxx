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

#include "accessibility/extended/AccessibleBrowseBox.hxx"
#include "accessibility/extended/AccessibleBrowseBoxTable.hxx"
#include "accessibility/extended/AccessibleBrowseBoxHeaderBar.hxx"
#include <svtools/accessibletableprovider.hxx>
#include <comphelper/types.hxx>
#include <toolkit/helper/vclunohelper.hxx>



namespace accessibility
{



using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;


class AccessibleBrowseBoxImpl
{
public:
    /// the XAccessible which created the AccessibleBrowseBox
    WeakReference< XAccessible >                                m_aCreator;

    /** The data table child. */
    Reference< css::accessibility::XAccessible >                mxTable;
    AccessibleBrowseBoxTable*                                   m_pTable;

    /** The header bar for rows ("handle column"). */
    Reference< css::accessibility::XAccessible >                mxRowHeaderBar;
    AccessibleBrowseBoxHeaderBar*                               m_pRowHeaderBar;

    /** The header bar for columns (first row of the table). */
    Reference< css::accessibility::XAccessible >                mxColumnHeaderBar;
    AccessibleBrowseBoxHeaderBar*                               m_pColumnHeaderBar;
};

// Ctor/Dtor/disposing --------------------------------------------------------

AccessibleBrowseBox::AccessibleBrowseBox(
            const Reference< XAccessible >& _rxParent, const Reference< XAccessible >& _rxCreator,
            IAccessibleTableProvider& _rBrowseBox )
    : AccessibleBrowseBoxBase( _rxParent, _rBrowseBox,nullptr, BBTYPE_BROWSEBOX )
{
    m_xImpl.reset( new AccessibleBrowseBoxImpl() );
    m_xImpl->m_aCreator = _rxCreator;

    m_xFocusWindow = VCLUnoHelper::GetInterface(mpBrowseBox->GetWindowInstance());
}

void AccessibleBrowseBox::setCreator( const Reference< XAccessible >& _rxCreator )
{
#if OSL_DEBUG_LEVEL > 0
    Reference< XAccessible > xCreator(m_xImpl->m_aCreator);
    OSL_ENSURE( !xCreator.is(), "accessibility/extended/AccessibleBrowseBox::setCreator: creator already set!" );
#endif
    m_xImpl->m_aCreator = _rxCreator;
}


AccessibleBrowseBox::~AccessibleBrowseBox()
{
}


void SAL_CALL AccessibleBrowseBox::disposing()
{
    ::osl::MutexGuard aGuard( getOslMutex() );

    m_xImpl->m_pTable           = nullptr;
    m_xImpl->m_pColumnHeaderBar = nullptr;
    m_xImpl->m_pRowHeaderBar    = nullptr;
    m_xImpl->m_aCreator.clear();

    Reference< XAccessible >  xTable = m_xImpl->mxTable;

    Reference< XComponent > xComp( m_xImpl->mxTable, UNO_QUERY );
    if ( xComp.is() )
    {
        xComp->dispose();

    }
//!    ::comphelper::disposeComponent(m_xImpl->mxTable);
    ::comphelper::disposeComponent(m_xImpl->mxRowHeaderBar);
    ::comphelper::disposeComponent(m_xImpl->mxColumnHeaderBar);

    AccessibleBrowseBoxBase::disposing();
}


// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL AccessibleBrowseBox::getAccessibleChildCount()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return BBINDEX_FIRSTCONTROL + mpBrowseBox->GetAccessibleControlCount();
}


Reference< XAccessible > SAL_CALL
AccessibleBrowseBox::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Reference< XAccessible > xRet;
    if( nChildIndex >= 0 )
    {
        if( nChildIndex < BBINDEX_FIRSTCONTROL )
            xRet = implGetFixedChild( nChildIndex );
        else
        {
            // additional controls
            nChildIndex -= BBINDEX_FIRSTCONTROL;
            if( nChildIndex < mpBrowseBox->GetAccessibleControlCount() )
                xRet = mpBrowseBox->CreateAccessibleControl( nChildIndex );
        }
    }

    if( !xRet.is() )
        throw lang::IndexOutOfBoundsException();
    return xRet;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleBrowseBox::getAccessibleAtPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Reference< XAccessible > xChild;
    sal_Int32 nIndex = 0;
    if( mpBrowseBox->ConvertPointToControlIndex( nIndex, VCLPoint( rPoint ) ) )
        xChild = mpBrowseBox->CreateAccessibleControl( nIndex );
    else
    {
        // try whether point is in one of the fixed children
        // (table, header bars, corner control)
        Point aPoint( VCLPoint( rPoint ) );
        for( nIndex = 0; (nIndex < BBINDEX_FIRSTCONTROL) && !xChild.is(); ++nIndex )
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


void SAL_CALL AccessibleBrowseBox::grabFocus()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    mpBrowseBox->GrabFocus();
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleBrowseBox::getImplementationName()
    throw ( uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.svtools.AccessibleBrowseBox" );
}


// internal virtual methods ---------------------------------------------------

Rectangle AccessibleBrowseBox::implGetBoundingBox()
{
    vcl::Window* pParent = mpBrowseBox->GetAccessibleParentWindow();
    OSL_ENSURE( pParent, "implGetBoundingBox - missing parent window" );
    return mpBrowseBox->GetWindowExtentsRelative( pParent );
}


Rectangle AccessibleBrowseBox::implGetBoundingBoxOnScreen()
{
    return mpBrowseBox->GetWindowExtentsRelative( nullptr );
}


// internal helper methods ----------------------------------------------------

Reference< XAccessible > AccessibleBrowseBox::implGetTable()
{
    if( !m_xImpl->mxTable.is() )
    {
        m_xImpl->m_pTable = createAccessibleTable();
        m_xImpl->mxTable  = m_xImpl->m_pTable;

    }
    return m_xImpl->mxTable;
}


Reference< XAccessible >
AccessibleBrowseBox::implGetHeaderBar( AccessibleBrowseBoxObjType eObjType )
{
    Reference< XAccessible > xRet;
    Reference< XAccessible >* pxMember = nullptr;

    if( eObjType == BBTYPE_ROWHEADERBAR )
        pxMember = &m_xImpl->mxRowHeaderBar;
    else if( eObjType ==  BBTYPE_COLUMNHEADERBAR )
        pxMember = &m_xImpl->mxColumnHeaderBar;

    if( pxMember )
    {
        if( !pxMember->is() )
        {
            AccessibleBrowseBoxHeaderBar* pHeaderBar = new AccessibleBrowseBoxHeaderBar(
                m_xImpl->m_aCreator, *mpBrowseBox, eObjType );

            if ( BBTYPE_COLUMNHEADERBAR == eObjType)
                m_xImpl->m_pColumnHeaderBar = pHeaderBar;
            else
                m_xImpl->m_pRowHeaderBar    = pHeaderBar;

            *pxMember = pHeaderBar;
        }
        xRet = *pxMember;
    }
    return xRet;
}


Reference< XAccessible >
AccessibleBrowseBox::implGetFixedChild( sal_Int32 nChildIndex )
{
    Reference< XAccessible > xRet;
    switch( nChildIndex )
    {
        case BBINDEX_COLUMNHEADERBAR:
            xRet = implGetHeaderBar( BBTYPE_COLUMNHEADERBAR );
        break;
        case BBINDEX_ROWHEADERBAR:
            xRet = implGetHeaderBar( BBTYPE_ROWHEADERBAR );
        break;
        case BBINDEX_TABLE:
            xRet = implGetTable();
        break;
    }
    return xRet;
}

AccessibleBrowseBoxTable* AccessibleBrowseBox::createAccessibleTable()
{
    Reference< XAccessible > xCreator(m_xImpl->m_aCreator);
    OSL_ENSURE( xCreator.is(), "accessibility/extended/AccessibleBrowseBox::createAccessibleTable: my creator died - how this?" );
    return new AccessibleBrowseBoxTable( xCreator, *mpBrowseBox );
}

void AccessibleBrowseBox::commitTableEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    if ( m_xImpl->mxTable.is() )
    {
        m_xImpl->m_pTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
    }
}

void AccessibleBrowseBox::commitHeaderBarEvent( sal_Int16 _nEventId,
                                                const Any& _rNewValue,
                                                const Any& _rOldValue,bool _bColumnHeaderBar)
{
    Reference< XAccessible > xHeaderBar = _bColumnHeaderBar ? m_xImpl->mxColumnHeaderBar : m_xImpl->mxRowHeaderBar;
    AccessibleBrowseBoxHeaderBar* pHeaderBar = _bColumnHeaderBar ? m_xImpl->m_pColumnHeaderBar : m_xImpl->m_pRowHeaderBar;
    if ( xHeaderBar.is() )
        pHeaderBar->commitEvent(_nEventId,_rNewValue,_rOldValue);
}


// = AccessibleBrowseBoxAccess

AccessibleBrowseBoxAccess::AccessibleBrowseBoxAccess( const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox )
        :m_xParent( _rxParent )
        ,m_rBrowseBox( _rBrowseBox )
        ,m_pContext( nullptr )
{
}


AccessibleBrowseBoxAccess::~AccessibleBrowseBoxAccess()
{
}


void AccessibleBrowseBoxAccess::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_pContext = nullptr;
    ::comphelper::disposeComponent( m_xContext );
}


Reference< XAccessibleContext > SAL_CALL AccessibleBrowseBoxAccess::getAccessibleContext() throw ( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OSL_ENSURE( ( m_pContext && m_xContext.is() ) || ( !m_pContext && !m_xContext.is() ),
        "accessibility/extended/AccessibleBrowseBoxAccess::getAccessibleContext: inconsistency!" );

    // if the context died meanwhile (there is no listener, so it won't tell us explicitily when this happens),
    // then reset and re-create.
    if ( m_pContext && !m_pContext->isAlive() )
        m_xContext = m_pContext = nullptr;

    if ( !m_xContext.is() )
        m_xContext = m_pContext = new AccessibleBrowseBox( m_xParent, this, m_rBrowseBox );

    return m_xContext;
}


bool AccessibleBrowseBoxAccess::isContextAlive() const
{
    return  ( nullptr != m_pContext ) && m_pContext->isAlive();
}



}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
