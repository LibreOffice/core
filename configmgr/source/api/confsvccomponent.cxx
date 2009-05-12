/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confsvccomponent.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_configmgr.hxx"

#include "confsvccomponent.hxx"
#include "datalock.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <osl/mutex.hxx>
#include "utility.hxx"

#include <map>

namespace configmgr {


    ServiceComponentImpl::ServiceComponentImpl(ServiceImplementationInfo const* aInfo)
        : ::cppu::WeakComponentImplHelper1< lang::XServiceInfo >(UnoApiLock::getLock())
        , m_info(aInfo)
    {
    }

    void ServiceComponentImpl::disposing()
    {
        ::cppu::WeakComponentImplHelper1< lang::XServiceInfo >::disposing();
    }
    void ServiceComponentImpl::checkAlive() throw (uno::RuntimeException)
    {
        checkAlive("Object was disposed");
    }
    void ServiceComponentImpl::checkAlive(rtl::OUString const& sMessage) throw (uno::RuntimeException)
    {
        if (rBHelper.bDisposed)
            throw lang::DisposedException(sMessage, *this);
    }

    // XTypeProvider
    uno::Sequence<sal_Int8> ServiceComponentImpl::getStaticImplementationId(ServiceImplementationInfo const* pServiceInfo)
        throw(uno::RuntimeException)
    {
        static osl::Mutex aMapMutex;
        static std::map<ServiceImplementationInfo const*, ::cppu::OImplementationId> aIdMap;

        osl::MutexGuard aMapGuard(aMapMutex);
        return aIdMap[pServiceInfo].getImplementationId();
    }

    uno::Sequence<sal_Int8> SAL_CALL ServiceComponentImpl::getImplementationId()
        throw(uno::RuntimeException)
    {
        return getStaticImplementationId(m_info);
    }

    // XServiceInfo
    rtl::OUString SAL_CALL ServiceComponentImpl::getImplementationName(  ) throw(uno::RuntimeException)
    {
        return ServiceInfoHelper(m_info).getImplementationName();
    }

    sal_Bool SAL_CALL ServiceComponentImpl::supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException)
    {
        return ServiceInfoHelper(m_info).supportsService( ServiceName );
    }

    uno::Sequence< rtl::OUString > SAL_CALL ServiceComponentImpl::getSupportedServiceNames(  ) throw(uno::RuntimeException)
    {
        return ServiceInfoHelper(m_info).getSupportedServiceNames( );
    }

    //ServiceComponentImpl::

} // namespace configmgr


