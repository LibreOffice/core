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

#include "JAccess.hxx"
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
    OUString SAL_CALL OJoinDesignViewAccess::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    OUString OJoinDesignViewAccess::getImplementationName_Static(void) throw( RuntimeException )
    {
        return OUString("org.openoffice.comp.dbu.JoinViewAccessibility");
    }
    void OJoinDesignViewAccess::clearTableView()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_pTableView = NULL;
    }
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
    sal_Bool OJoinDesignViewAccess::isEditable() const
    {
        return m_pTableView && !m_pTableView->getDesignView()->getController().isReadOnly();
    }
    sal_Int16 SAL_CALL OJoinDesignViewAccess::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::VIEW_PORT;
    }
    Reference< XAccessibleContext > SAL_CALL OJoinDesignViewAccess::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return this;
    }
    // XInterface
    IMPLEMENT_FORWARD_XINTERFACE2( OJoinDesignViewAccess, VCLXAccessibleComponent, OJoinDesignViewAccess_BASE )
    // XTypeProvider
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OJoinDesignViewAccess, VCLXAccessibleComponent, OJoinDesignViewAccess_BASE )
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
