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

#include "accessibility/extended/AccessibleGridControl.hxx"
#include "accessibility/extended/AccessibleGridControlTable.hxx"
#include "accessibility/extended/AccessibleGridControlHeader.hxx"
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <svtools/accessibletable.hxx>
#include <comphelper/types.hxx>
#include <toolkit/helper/vclunohelper.hxx>



namespace accessibility
{



using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;
using namespace ::svt::table;


class AccessibleGridControl_Impl
{
public:
    /// the XAccessible which created the AccessibleGridControl
    WeakReference< XAccessible >                    m_aCreator;

    /** The data table child. */
    Reference< css::accessibility::XAccessible >    m_xTable;
    AccessibleGridControlTable*                     m_pTable;

    /** The header bar for rows. */
    Reference< css::accessibility::XAccessible >    m_xRowHeaderBar;
    AccessibleGridControlHeader*                    m_pRowHeaderBar;

    /** The header bar for columns (first row of the table). */
    Reference< css::accessibility::XAccessible >    m_xColumnHeaderBar;
    AccessibleGridControlHeader*                    m_pColumnHeaderBar;

    /** The table cell child. */
    Reference< css::accessibility::XAccessible >    m_xCell;
    AccessibleGridControlTableCell*                 m_pCell;

};

AccessibleGridControl::AccessibleGridControl(
            const Reference< XAccessible >& _rxParent, const Reference< XAccessible >& _rxCreator,
            IAccessibleTable& _rTable )
    : AccessibleGridControlBase( _rxParent, _rTable, TCTYPE_GRIDCONTROL )
{
    m_xImpl.reset( new AccessibleGridControl_Impl() );
    m_xImpl->m_aCreator = _rxCreator;
}


AccessibleGridControl::~AccessibleGridControl()
{
}


void SAL_CALL AccessibleGridControl::disposing()
{
    SolarMutexGuard g;

    m_xImpl->m_pTable       = nullptr;
    m_xImpl->m_pColumnHeaderBar = nullptr;
    m_xImpl->m_pRowHeaderBar    = nullptr;
    m_xImpl->m_pCell            = nullptr;
    m_xImpl->m_aCreator.clear();

    Reference< XAccessible >  xTable = m_xImpl->m_xTable;

    Reference< XComponent > xComp( m_xImpl->m_xTable, UNO_QUERY );
    if ( xComp.is() )
    {
        xComp->dispose();
    }
    Reference< XAccessible >  xCell = m_xImpl->m_xCell;

    Reference< XComponent > xCellComp( m_xImpl->m_xCell, UNO_QUERY );
    if ( xCellComp.is() )
    {
        xCellComp->dispose();
    }

    ::comphelper::disposeComponent(m_xImpl->m_xRowHeaderBar);
    ::comphelper::disposeComponent(m_xImpl->m_xColumnHeaderBar);
    AccessibleGridControlBase::disposing();
}


// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL AccessibleGridControl::getAccessibleChildCount()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
    return m_aTable.GetAccessibleControlCount();
}


Reference< XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;

    if (nChildIndex<0 || nChildIndex>=getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    if (isAlive())
    {
        if(nChildIndex == 0 && m_aTable.HasColHeader())
        {
            if(!m_xImpl->m_xColumnHeaderBar.is())
            {
                AccessibleGridControlHeader* pColHeaderBar = new AccessibleGridControlHeader(m_xImpl->m_aCreator, m_aTable, svt::table::TCTYPE_COLUMNHEADERBAR);
                m_xImpl->m_xColumnHeaderBar = pColHeaderBar;
            }
            xChild = m_xImpl->m_xColumnHeaderBar;
        }
        else if(m_aTable.HasRowHeader() && (nChildIndex == 1 || nChildIndex == 0))
        {
            if(!m_xImpl->m_xRowHeaderBar.is())
            {
                AccessibleGridControlHeader* pRowHeaderBar = new AccessibleGridControlHeader(m_xImpl->m_aCreator, m_aTable, svt::table::TCTYPE_ROWHEADERBAR);
                m_xImpl->m_xRowHeaderBar = pRowHeaderBar;
            }
            xChild = m_xImpl->m_xRowHeaderBar;
        }
        else
        {
            if(!m_xImpl->m_xTable.is())
            {
            AccessibleGridControlTable* pTable = new AccessibleGridControlTable(m_xImpl->m_aCreator, m_aTable, svt::table::TCTYPE_TABLE);
            m_xImpl->m_xTable = pTable;
                m_xImpl->m_pTable = pTable;
            }
            xChild = m_xImpl->m_xTable;
        }
    }
    return xChild;
}


sal_Int16 SAL_CALL AccessibleGridControl::getAccessibleRole()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    return AccessibleRole::PANEL;
}


// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleAtPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
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


void SAL_CALL AccessibleGridControl::grabFocus()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
        m_aTable.GrabFocus();
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControl::getImplementationName()
    throw ( uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.accessibility.AccessibleGridControl" );
}


// internal virtual methods ---------------------------------------------------

Rectangle AccessibleGridControl::implGetBoundingBox()
{
    vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
    OSL_ENSURE( pParent, "implGetBoundingBox - missing parent window" );
    return m_aTable.GetWindowExtentsRelative( pParent );
}


Rectangle AccessibleGridControl::implGetBoundingBoxOnScreen()
{
    return m_aTable.GetWindowExtentsRelative( nullptr );
}
// internal helper methods ----------------------------------------------------

Reference< XAccessible > AccessibleGridControl::implGetTable()
{
    if( !m_xImpl->m_xTable.is() )
    {
        m_xImpl->m_pTable = createAccessibleTable();
        m_xImpl->m_xTable  = m_xImpl->m_pTable;
    }
    return m_xImpl->m_xTable;
}


Reference< XAccessible >
AccessibleGridControl::implGetHeaderBar( AccessibleTableControlObjType eObjType )
{
    Reference< XAccessible > xRet;
    Reference< XAccessible >* pxMember = nullptr;

    if( eObjType == TCTYPE_ROWHEADERBAR )
        pxMember = &m_xImpl->m_xRowHeaderBar;
    else if( eObjType ==  TCTYPE_COLUMNHEADERBAR )
        pxMember = &m_xImpl->m_xColumnHeaderBar;

    if( pxMember )
    {
        if( !pxMember->is() )
        {
            AccessibleGridControlHeader* pHeaderBar = new AccessibleGridControlHeader(
                m_xImpl->m_aCreator, m_aTable, eObjType );

            if ( TCTYPE_COLUMNHEADERBAR == eObjType)
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

AccessibleGridControlTable* AccessibleGridControl::createAccessibleTable()
{
    Reference< XAccessible > xCreator(m_xImpl->m_aCreator);
    OSL_ENSURE( xCreator.is(), "accessibility/extended/AccessibleGirdControl::createAccessibleTable: my creator died - how this?" );
    return new AccessibleGridControlTable( xCreator, m_aTable, TCTYPE_TABLE );
}

void AccessibleGridControl::commitCellEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    sal_Int32 nChildCount = getAccessibleChildCount();
    if(nChildCount != 0)
    {
        for(sal_Int32 i=0;i<nChildCount;i++)
        {
            Reference< XAccessible > xAccessible = getAccessibleChild(i);
            css::uno::Reference< css::accessibility::XAccessibleContext > xAccessibleChild = xAccessible->getAccessibleContext();
            if(m_xImpl->m_xTable == xAccessible)
            {
                std::vector< AccessibleGridControlTableCell* >& rCells =
                    m_xImpl->m_pTable->getCellVector();
                size_t nIndex = m_aTable.GetCurrentRow() * m_aTable.GetColumnCount()
                              + m_aTable.GetCurrentColumn();
                if (nIndex < rCells.size() && rCells[nIndex])
                {
                    m_xImpl->m_pCell = rCells[nIndex];
                    m_xImpl->m_pCell->commitEvent( _nEventId, _rNewValue, _rOldValue );
                }
            }
        }
    }
    else
    {
        if ( m_xImpl->m_xTable.is() )
            m_xImpl->m_pTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
    }
}

void AccessibleGridControl::commitTableEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    if ( m_xImpl->m_xTable.is() )
    {
        if(_nEventId == AccessibleEventId::ACTIVE_DESCENDANT_CHANGED)
        {
            Reference< XAccessible > xChild = m_xImpl->m_pTable->getAccessibleChild(m_aTable.GetCurrentRow()*m_aTable.GetColumnCount()+m_aTable.GetCurrentColumn());
            m_xImpl->m_pTable->commitEvent(_nEventId, makeAny(xChild),_rOldValue);
        }
        else if(_nEventId == AccessibleEventId::TABLE_MODEL_CHANGED)
        {
            AccessibleTableModelChange aChange;
            if(_rNewValue >>= aChange)
            {
                if(aChange.Type == AccessibleTableModelChangeType::DELETE)
                {
                    std::vector< AccessibleGridControlTableCell* >& rCells =
                        m_xImpl->m_pTable->getCellVector();
                    std::vector< Reference< XAccessible > >& rAccCells =
                        m_xImpl->m_pTable->getAccessibleCellVector();
                    int nColCount = m_aTable.GetColumnCount();
                    // check valid index - entries are inserted lazily
                    size_t const nStart = nColCount * aChange.FirstRow;
                    size_t const nEnd   = nColCount * aChange.LastRow;
                    if (nStart < rCells.size())
                    {
                        m_xImpl->m_pTable->getCellVector().erase(
                            rCells.begin() + nStart,
                            rCells.begin() + std::min(rCells.size(), nEnd));
                    }
                    if (nStart < rAccCells.size())
                    {
                        m_xImpl->m_pTable->getAccessibleCellVector().erase(
                            rAccCells.begin() + nStart,
                            rAccCells.begin() + std::min(rAccCells.size(), nEnd));
                    }
                    m_xImpl->m_pTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
                }
                else
                    m_xImpl->m_pTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
            }
        }
        else
            m_xImpl->m_pTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
    }
}

// = AccessibleGridControlAccess



AccessibleGridControlAccess::AccessibleGridControlAccess(
        const Reference< XAccessible >& rxParent, IAccessibleTable& rTable )
    : m_xParent( rxParent )
    , m_pTable( & rTable )
    , m_pContext( nullptr )
{
}


AccessibleGridControlAccess::~AccessibleGridControlAccess()
{
}


void AccessibleGridControlAccess::DisposeAccessImpl()
{
    SolarMutexGuard g;

    m_pTable = nullptr;
    m_pContext = nullptr;
    ::comphelper::disposeComponent( m_xContext );
}


Reference< XAccessibleContext > SAL_CALL AccessibleGridControlAccess::getAccessibleContext() throw ( RuntimeException, std::exception )
{
    SolarMutexGuard g;

    OSL_ENSURE( ( m_pContext && m_xContext.is() ) || ( !m_pContext && !m_xContext.is() ),
        "accessibility/extended/AccessibleGridControlAccess::getAccessibleContext: inconsistency!" );

    // if the context died meanwhile (we're no listener, so it won't tell us explicitily when this happens),
    // then reset an re-create.
    if ( m_pContext && !m_pContext->isAlive() )
        m_xContext = m_pContext = nullptr;

    if (!m_xContext.is() && m_pTable)
        m_xContext = m_pContext =
            new AccessibleGridControl(m_xParent, this, *m_pTable);

    return m_xContext;
}


bool AccessibleGridControlAccess::isContextAlive() const
{
    return  ( nullptr != m_pContext ) && m_pContext->isAlive();
}



}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
