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

#include <helper/accresmgr.hxx>
#include <strings.hrc>

#include <extended/accessibleeditbrowseboxcell.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>

namespace accessibility
{
    using namespace com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::awt;
    using namespace ::comphelper;

    EditBrowseBoxTableCell::EditBrowseBoxTableCell(
                const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
                const css::uno::Reference< css::accessibility::XAccessible >& _rxOwningAccessible,
                const css::uno::Reference< css::accessibility::XAccessibleContext >& _xControlChild,
                ::vcl::IAccessibleTableProvider& _rBrowseBox,
                const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                sal_Int32 _nRowPos,
                sal_uInt16 _nColPos)
        :AccessibleBrowseBoxCell( _rxParent, _rBrowseBox, _xFocusWindow, _nRowPos, _nColPos )
        ,OAccessibleContextWrapperHelper( ::comphelper::getProcessComponentContext(), rBHelper, _xControlChild, _rxOwningAccessible, _rxParent )
    {
        aggregateProxy( m_refCount, *this );
    }

    EditBrowseBoxTableCell::~EditBrowseBoxTableCell()
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();  // to prevent duplicate dtor calls
            dispose();
        }
    }

    OUString SAL_CALL EditBrowseBoxTableCell::getImplementationName()
    {
        return "com.sun.star.comp.svtools.TableCellProxy";
    }

    IMPLEMENT_FORWARD_XINTERFACE2( EditBrowseBoxTableCell, AccessibleBrowseBoxCell, OAccessibleContextWrapperHelper )

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( EditBrowseBoxTableCell, AccessibleBrowseBoxCell, OAccessibleContextWrapperHelper )

    void EditBrowseBoxTableCell::notifyTranslatedEvent( const AccessibleEventObject& _rEvent )
    {
        commitEvent( _rEvent.EventId, _rEvent.NewValue, _rEvent.OldValue );
    }

    // css::accessibility::XAccessibleComponent
    sal_Int32 SAL_CALL EditBrowseBoxTableCell::getForeground(  )
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        css::uno::Reference< css::accessibility::XAccessibleComponent > xAccComp( m_xInnerContext, UNO_QUERY );
        if ( xAccComp.is() )
            return xAccComp->getForeground();
        return 0;
    }

    sal_Int32 SAL_CALL EditBrowseBoxTableCell::getBackground(  )
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        css::uno::Reference< css::accessibility::XAccessibleComponent > xAccComp( m_xInnerContext, UNO_QUERY );
        if ( xAccComp.is() )
            return xAccComp->getBackground();
        return 0;
    }

    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL EditBrowseBoxTableCell::getAccessibleParent(  )
    {
        return m_xParentAccessible;
    }

    OUString SAL_CALL EditBrowseBoxTableCell::getAccessibleDescription()
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        return m_xInnerContext->getAccessibleDescription();
    }

    OUString SAL_CALL EditBrowseBoxTableCell::getAccessibleName()
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        return AccResId(RID_STR_ACC_COLUMN_NUM).replaceAll("%COLUMNNUMBER", OUString::number(getColumnPos()-1)) + ", "
               + AccResId(RID_STR_ACC_ROW_NUM).replaceAll("%ROWNUMBER", OUString::number(getRowPos()));
    }

    css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL EditBrowseBoxTableCell::getAccessibleRelationSet()
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        return baseGetAccessibleRelationSet( );
    }

    css::uno::Reference<css::accessibility::XAccessibleStateSet > SAL_CALL EditBrowseBoxTableCell::getAccessibleStateSet()
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        return m_xInnerContext->getAccessibleStateSet();
            // TODO: shouldn't we add an ACTIVE here? Isn't the EditBrowseBoxTableCell always ACTIVE?
    }

    sal_Int32 SAL_CALL EditBrowseBoxTableCell::getAccessibleChildCount(  )
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        return baseGetAccessibleChildCount();
    }

    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL EditBrowseBoxTableCell::getAccessibleChild( sal_Int32 i )
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        return baseGetAccessibleChild( i );
    }

    sal_Int16 SAL_CALL EditBrowseBoxTableCell::getAccessibleRole()
    {
        SolarMethodGuard aGuard(getMutex());
        ensureIsAlive();

        return m_xInnerContext->getAccessibleRole( );
    }

    void SAL_CALL EditBrowseBoxTableCell::dispose()
    {
        // simply disambiguate. Note that the OComponentHelper base in AccessibleBrowseBoxCell
        // will call our "disposing()", which will call "dispose()" on the OAccessibleContextWrapperHelper
        // so there is no need to do this here.
        AccessibleBrowseBoxCell::dispose();
    }

    void SAL_CALL EditBrowseBoxTableCell::disposing( const css::lang::EventObject& _rSource )
    {
        AccessibleBrowseBoxCell::disposing( _rSource );
        OAccessibleContextWrapperHelper::disposing( _rSource );
    }

    void SAL_CALL EditBrowseBoxTableCell::disposing()
    {
        SolarMethodGuard aGuard(getMutex());

        OAccessibleContextWrapperHelper::dispose();
            // TODO: do we need to dispose our inner object? The base class does this, but is it a good idea?
        AccessibleBrowseBoxCell::disposing();
    }

    // EditBrowseBoxTableCell
    EditBrowseBoxTableCellAccess::EditBrowseBoxTableCellAccess(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent, const css::uno::Reference< css::accessibility::XAccessible >& _rxControlAccessible,
            const css::uno::Reference< css::awt::XWindow >& _rxFocusWindow,
            ::vcl::IAccessibleTableProvider& _rBrowseBox, sal_Int32 _nRowPos, sal_uInt16 _nColPos )
        :WeakComponentImplHelper( m_aMutex )
        ,m_xParent( _rxParent )
        ,m_xControlAccessible( _rxControlAccessible )
        ,m_xFocusWindow( _rxFocusWindow )
        ,m_pBrowseBox( &_rBrowseBox )
        ,m_nRowPos( _nRowPos )
        ,m_nColPos( _nColPos )
    {
    }

    EditBrowseBoxTableCellAccess::~EditBrowseBoxTableCellAccess( )
    {
    }

    css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL EditBrowseBoxTableCellAccess::getAccessibleContext(  )
    {
        if ( !m_pBrowseBox || !m_xControlAccessible.is() )
            throw DisposedException();
        css::uno::Reference< css::accessibility::XAccessibleContext > xMyContext( m_aContext );
        if ( !xMyContext.is() )
        {
            css::uno::Reference< css::accessibility::XAccessibleContext > xInnerContext = m_xControlAccessible->getAccessibleContext();
            css::uno::Reference< css::accessibility::XAccessible > xMe( this );

            xMyContext = new EditBrowseBoxTableCell( xMe, m_xParent, xInnerContext, *m_pBrowseBox, m_xFocusWindow, m_nRowPos, m_nColPos );
            m_aContext = xMyContext;
        }
        return xMyContext;
    }

    void SAL_CALL EditBrowseBoxTableCellAccess::disposing()
    {
        // dispose our context, if it still alive
        css::uno::Reference< XComponent > xMyContext( m_aContext.get(), UNO_QUERY );
        if ( xMyContext.is() )
        {
            try
            {
                xMyContext->dispose();
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION( "accessibility", "EditBrowseBoxTableCellAccess::disposing: caught an exception while disposing the context!" );
            }
        }

        m_pBrowseBox = nullptr;
        m_xControlAccessible.clear();
        m_aContext.clear();
        // NO dispose of the inner object there: it is the css::accessibility::XAccessible of a window, and disposing
        // it would delete the respective VCL window
    }
} // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
