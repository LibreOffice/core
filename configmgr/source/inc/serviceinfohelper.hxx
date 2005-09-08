/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: serviceinfohelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:55:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#define CONFIGMGR_SERVICEINFOHELPER_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;
    using ::rtl::OUString;
// -----------------------------------------------------------------------------

    typedef sal_Char const * AsciiServiceName;
// -----------------------------------------------------------------------------

    /// POD struct describing the registration information of a service implementation
    struct ServiceRegistrationInfo
    {
        /// The implementation name of this service implementation
        AsciiServiceName implementationName;
        /// The services for which this service implementation is registered
        AsciiServiceName const * registeredServiceNames;
    };
// -----------------------------------------------------------------------------

    /// POD struct describing the implementation information of a service implementation
    struct ServiceImplementationInfo
    {
        /// The implementation name of this service implementation
        AsciiServiceName implementationName;
        /// The services for which this service implementation is registered
        AsciiServiceName const * registeredServiceNames;
        /// Additional services implemented by this service implementation, for which it is not registered
        AsciiServiceName const * additionalServiceNames;
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
        AsciiServiceName singletonName;
        /// The implementation, which owns this singleton
        AsciiServiceName implementationName;
        /// The service, which should be instatiated for this singleton
        AsciiServiceName instantiatedServiceName;
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

        OUString getImplementationName( ) const
            throw(uno::RuntimeException);

        uno::Sequence< OUString > getRegisteredServiceNames( ) const
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

        OUString getImplementationName( ) const
            throw(uno::RuntimeException);

        sal_Bool supportsService( OUString const & ServiceName ) const
            throw(uno::RuntimeException);

        uno::Sequence< OUString > getSupportedServiceNames( ) const
            throw(uno::RuntimeException);
    };
// -----------------------------------------------------------------------------

} // namespace configmgr

#endif


