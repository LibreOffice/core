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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "JAccess.hxx"
#include "JoinTableView.hxx"
#include "JoinTableView.hxx"
#include "TableWindow.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include "JoinDesignView.hxx"
#include "JoinController.hxx"
#include "TableConnection.hxx"

namespace dbaui
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    OJoinDesignViewAccess::OJoinDesignViewAccess(OJoinTableView* _pTableView)
        :VCLXAccessibleComponent(_pTableView->GetComponentInterface().is() ? _pTableView->GetWindowPeer() : NULL)
        ,m_pTableView(_pTableView)
    {
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OJoinDesignViewAccess::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString OJoinDesignViewAccess::getImplementationName_Static(void) throw( RuntimeException )
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.JoinViewAccessibility"));
    }
    // -----------------------------------------------------------------------------
    void OJoinDesignViewAccess::clearTableView()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_pTableView = NULL;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    sal_Int32 SAL_CALL OJoinDesignViewAccess::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        // TODO may be this will change to only visible windows
        // this is the same assumption mt implements
        ::osl::MutexGuard aGuard( m_aMutex  );
        sal_Int32 nChildCount = 0;
        if ( m_pTableView )
            nChildCount = m_pTableView->GetTabWinCount() + m_pTableView->getTableConnections()->size();
        return nChildCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OJoinDesignViewAccess::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        Reference< XAccessible > aRet;
        ::osl::MutexGuard aGuard( m_aMutex  );
        if(i >= 0 && i < getAccessibleChildCount() && m_pTableView )
        {
            // check if we should return a table window or a connection
            sal_Int32 nTableWindowCount = m_pTableView->GetTabWinCount();
            if( i < nTableWindowCount )
            {
                OJoinTableView::OTableWindowMap::iterator aIter = m_pTableView->GetTabWinMap()->begin();
                for (sal_Int32 j=i; j; ++aIter,--j)
                    ;
                aRet = aIter->second->GetAccessible();
            }
            else if( size_t(i - nTableWindowCount) < m_pTableView->getTableConnections()->size() )
                aRet = (*m_pTableView->getTableConnections())[i - nTableWindowCount]->GetAccessible();
        }
        else
            throw IndexOutOfBoundsException();
        return aRet;
    }
    // -----------------------------------------------------------------------------
    sal_Bool OJoinDesignViewAccess::isEditable() const
    {
        return m_pTableView && !m_pTableView->getDesignView()->getController().isReadOnly();
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL OJoinDesignViewAccess::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::VIEW_PORT;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL OJoinDesignViewAccess::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return this;
    }
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // XInterface
    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OJoinDesignViewAccess, VCLXAccessibleComponent, OJoinDesignViewAccess_BASE )
    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OJoinDesignViewAccess, VCLXAccessibleComponent, OJoinDesignViewAccess_BASE )
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
