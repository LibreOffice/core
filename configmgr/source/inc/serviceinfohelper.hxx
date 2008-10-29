/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: serviceinfohelper.hxx,v $
 * $Revision: 1.5 $
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

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#define CONFIGMGR_SERVICEINFOHELPER_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;
// -----------------------------------------------------------------------------

    /// POD struct describing the registration information of a service implementation
    struct ServiceRegistrationInfo
    {
        /// The implementation name of this service implementation
        sal_Char const * implementationName;
        /// The services for which this service implementation is registered
        sal_Char const * const * registeredServiceNames;
    };
// -----------------------------------------------------------------------------

    /// POD struct describing the implementation information of a service implementation
    struct ServiceImplementationInfo
    {
        /// The implementation name of this service implementation
        sal_Char const * implementationName;
        /// The services for which this service implementation is registered
        sal_Char const * const * registeredServiceNames;
        /// Additional services implemented by this service implementation, for which it is not registered
        sal_Char const * const * additionalServiceNames;
    };
// -----------------------------------------------------------------------------

    // ServiceImplementationInfo has a compatible initial sequence with struct ServiceRegistrationInfo

    inline
    ServiceRegistrationInfo const *
        getRegistrationInfo(ServiceImplementationInfo const * _info)
    {
        return reinterpret_cast<ServiceRegistrationInfo const *>(_info);
    }
// -----------------------------------------------------------------------------

    /// POD struct describing the registration information of a singleton
    struct SingletonRegistrationInfo
    {
        /// The name of this singleton
        sal_Char const * singletonName;
        /// The implementation, which owns this singleton
        sal_Char const * implementationName;
        /// The service, which should be instatiated for this singleton
        sal_Char const * instantiatedServiceName;
        /// A name for a pseudo-implementation, which is mapped to this singleton
        ServiceRegistrationInfo const * mappedImplementation;
    };
// -----------------------------------------------------------------------------

    class ServiceRegistrationHelper
    {
        ServiceRegistrationInfo const*const m_info;

    public:
        ServiceRegistrationHelper(ServiceRegistrationInfo const* _info)
        : m_info(_info)
        {}

        ServiceRegistrationHelper(ServiceImplementationInfo const* _info)
        : m_info(getRegistrationInfo(_info))
        {}

        sal_Int32 countServices() const;

        rtl::OUString getImplementationName( ) const
            throw(uno::RuntimeException);

        uno::Sequence< rtl::OUString > getRegisteredServiceNames( ) const
            throw(uno::RuntimeException);
    };
// -----------------------------------------------------------------------------

    class ServiceInfoHelper
    {
        ServiceImplementationInfo const*const m_info;

    public:
        ServiceInfoHelper(ServiceImplementationInfo const* _info)
        : m_info(_info)
        {}

        sal_Int32 countServices() const;

        rtl::OUString getImplementationName( ) const
            throw(uno::RuntimeException);

        sal_Bool supportsService( rtl::OUString const & ServiceName ) const
            throw(uno::RuntimeException);

        uno::Sequence< rtl::OUString > getSupportedServiceNames( ) const
            throw(uno::RuntimeException);
    };
// -----------------------------------------------------------------------------

} // namespace configmgr

#endif


