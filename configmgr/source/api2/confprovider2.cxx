/*************************************************************************
 *
 *  $RCSfile: confprovider2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-10 17:29:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "confprovider2.hxx"
#include "apiaccessobj.hxx"

#ifndef CONFIGMGR_API_PROVIDERIMPL2_HXX_
#include "confproviderimpl2.hxx"
#endif
#ifndef CONFIGMGR_MODULE_HXX_
#include "configmodule.hxx"
#endif
/*
#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_API_READ_ACCESS_IMPL_HXX_
#include "readaccessimpl.hxx"
#endif
#ifndef _CONFIGMGR_API_UPDATEACCESSIMPL_HXX_
#include "confupdateimpl.hxx"
#endif
#ifndef CONFIGMGR_API_USERADMINIMPL_HXX_
#include "useradminimpl.hxx"
#endif
*/
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef __SGI_STL_ALGORITHM
#include <stl/algorithm>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#define THISREF() static_cast< ::cppu::OWeakObject* >(this)

namespace configmgr
{
    namespace css  = ::com::sun::star;
    namespace uno  = css::uno;
    namespace lang = css::lang;
    namespace beans = css::beans;
    using ::rtl::OUString;
    using ::vos::ORef;
    using namespace osl;

#define SETTING_USER        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user"))
#define SETTING_PASSWORD    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("password"))

    namespace
    {
        typedef uno::Reference< uno::XInterface > (ConfigurationProviderImpl2::*CreatorFunc2)(const uno::Sequence< uno::Any >& aArguments);

        struct ServiceCreationInfo2
        {
            ServiceInfo const* info;
            CreatorFunc2 create;
        };

        AsciiServiceName const aProvider2Services[] =
        {
            "com.sun.star.configuration.ConfigurationProvider",
            0
        };
        ServiceInfo const aProvider2Info =
        {
            "com.sun.star.configuration.configmgr.ConfigurationProvider2",
            aProvider2Services

        };

        static sal_Int32 getCreateServiceDataCount()
        {
            return 3;
        };

        static const ServiceCreationInfo2* getCreateServiceData()
        {
            static ServiceCreationInfo2 const createServiceData[] =
            {
                { &configapi::aRootElementReadAccessSI, &ConfigurationProviderImpl2::createReadAccess },
                { &configapi::aRootElementUpdateAccessSI, &ConfigurationProviderImpl2::createUpdateAccess },
                { &configapi::aRootElementAdminAccessSI, &ConfigurationProviderImpl2::createUserAdminAccess }
            };
            OSL_ENSHURE(sizeof(createServiceData)/sizeof(createServiceData[0]) == getCreateServiceDataCount(),
                "getCreateServiceData : inconsistent data !");
            return createServiceData;
        }
    }
    ServiceInfo const& ConfigurationProvider2::staticServiceInfo = aProvider2Info;

    const ServiceInfo* getConfigurationProviderServiceInfo()
    {
        return &aProvider2Info;
    }


    static uno::Reference< lang::XMultiServiceFactory > getConfigurationFactory(uno::Reference< lang::XMultiServiceFactory > const& rServiceManager)
    {
        uno::Reference< uno::XInterface > aProvider( rServiceManager->createInstance( OUString::createFromAscii(aProvider2Info.implementationName) ) );
        uno::Reference< lang::XMultiServiceFactory > aConfigFactory( aProvider, uno::UNO_QUERY );
        return aConfigFactory;
    }

    uno::Reference< uno::XInterface > SAL_CALL instantiateReadAccess2(uno::Reference< lang::XMultiServiceFactory > const& rServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > aFactory( getConfigurationFactory(rServiceManager) );

        if (aFactory.is() )
            return aFactory->createInstance( OUString::createFromAscii(configapi::aRootElementReadAccessSI.implementationName) );
        return uno::Reference< uno::XInterface >();
    }

    uno::Reference< uno::XInterface > SAL_CALL instantiateUpdateAccess2(uno::Reference< lang::XMultiServiceFactory > const& rServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > aFactory( getConfigurationFactory(rServiceManager) );
        if (aFactory.is() )
            return aFactory->createInstance( OUString::createFromAscii(configapi::aRootElementUpdateAccessSI.implementationName) );

        return uno::Reference< uno::XInterface >();
    }

    // move to module !
/*  ORef<ConfigurationProvider2> ConfigurationProvider2::instance()
    {
        static ORef<ConfigurationProvider2> theProvider;
        if (theProvider.isEmpty())
        {
            static osl::Mutex aMutex;
            osl::MutexGuard aGuard( aMutex );

            if (theProvider.isEmpty())
                theProvider = createInstance();
        }
        return theProvider;
    }
*/


    void ConfigurationProvider2::implConnect() throw(uno::Exception)
    {
        IConfigSession* pNewSession = m_aModule.connect(m_aModule.getConnectionData(), m_aSecurityOverride);
        if (!pNewSession)
            throw uno::Exception(::rtl::OUString::createFromAscii("Could not connect to the configuration registry. Please check your settings."), NULL);
        m_pImpl = new ConfigurationProviderImpl2(this, pNewSession, m_aModule.getConverter());
        m_aModule.newProvider(this);
    }

    ConfigurationProvider2::ConfigurationProvider2(Module& aModule)
    : ServiceComponentImpl(&aProvider2Info)
    , m_pImpl(NULL)
    , m_aModule(aModule)
    {
    }

    ConfigurationProvider2::~ConfigurationProvider2()
    {
        if (m_pImpl)
        {
            m_aModule.disconnect();
            m_aModule.disposing(this);
            delete m_pImpl;
        }
    }

    void SAL_CALL ConfigurationProvider2::disposing()
    {
        if (m_pImpl)
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if (m_pImpl)
            {
                m_aModule.disconnect();
                m_aModule.disposing(this);
                delete m_pImpl;
                m_pImpl = NULL;
            }
        }
        ServiceComponentImpl::disposing();
    }

    static ServiceCreationInfo2 const* findCreationInfo( const OUString& aServiceSpecifier )
    {
        for (int i= 0; i<getCreateServiceDataCount(); ++i)
        {
            ServiceCreationInfo2 const& rCreationInfo = getCreateServiceData()[i];

            ServiceInfo const* pInfo = rCreationInfo.info;
            if (!pInfo)
                continue;

            if (AsciiServiceName pImplName = pInfo->implementationName)
            {
                if (0 == aServiceSpecifier.compareToAscii(pImplName))
                    return &rCreationInfo;
            }

            if (AsciiServiceName const* pNames = pInfo->serviceNames)
            {
                while(*pNames)
                {
                    if (0 == aServiceSpecifier.compareToAscii(*pNames))
                        return &rCreationInfo;

                    ++pNames;
                }
            }
        }

        // not found
        return 0;
    }

    uno::Reference< uno::XInterface > SAL_CALL ConfigurationProvider2::createInstance( const OUString& aServiceSpecifier )
        throw(uno::Exception, uno::RuntimeException)
    {
        MutexGuard aGuard(m_aMutex);
        ensureConnection();

        CFG_TRACE_INFO("going to create a read access instance for %s", "missing unicode conversion");

        if (ServiceCreationInfo2 const* pInfo = findCreationInfo(aServiceSpecifier))
        {
            // it's a known service name - try to create without args
            if (CreatorFunc2 create = pInfo->create)
            {
                uno::Sequence< uno::Any > aArguments;
                return (m_pImpl->*create)(
                    aArguments);
            }
        }

        // Otherwise try to use it as a configuration node name (and create a deep reader)
        uno::Sequence< uno::Any > aArguments(1);
        aArguments[0] <<= aServiceSpecifier;

        return m_pImpl->createReadAccess(aArguments);
    }

    uno::Reference< uno::XInterface > SAL_CALL
        ConfigurationProvider2::createInstanceWithArguments( const ::rtl::OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& aArguments )
            throw(uno::Exception, uno::RuntimeException)
    {
        MutexGuard aGuard(m_aMutex);
        ensureConnection();
        m_pImpl->setInternationalHelper(m_aModule.getInternationalHelper());

        if (ServiceCreationInfo2 const* pInfo = findCreationInfo(aServiceSpecifier))
        {
            // it's a known service name - try to create without args
            if (CreatorFunc2 create = pInfo->create)
            {
                return (m_pImpl->*create)(aArguments);
            }
        }
        // Otherwise try to use it as a configuration node name (and create a deep reader)
        sal_Int32 nLength = aArguments.getLength();
        uno::Sequence< uno::Any > aMoreArguments( 1 + nLength );

        aMoreArguments[0] <<= aServiceSpecifier;
        std::copy(aArguments.getConstArray(),aArguments.getConstArray() + nLength, aMoreArguments.getArray()+1);

        return m_pImpl->createReadAccess(aMoreArguments);
    }

    uno::Sequence< OUString > SAL_CALL ConfigurationProvider2::getAvailableServiceNames(  )
        throw(uno::RuntimeException)
    {
        MutexGuard aGuard(m_aMutex);

        sal_Int32 nCount = 0;

        {
            for (int i= 0; i<getCreateServiceDataCount(); ++i)
                nCount += countServices(getCreateServiceData()[i].info);
        }

        uno::Sequence< OUString > aNames(nCount);

        if (nCount > 0)
        {
            sal_Int32 n = 0;
            for (int i= 0; i<getCreateServiceDataCount(); ++i)
            {
                ServiceInfo const* pInfo = getCreateServiceData()[i].info;
                AsciiServiceName const* pNames = pInfo ? pInfo->serviceNames : 0;

                if (pNames)
                {
                    while(*pNames)
                    {
                        aNames[n] = OUString::createFromAscii(*pNames);
                        ++n;
                        ++pNames;
                    }
                }
            }
        }

        return aNames;
    }

    // XInitialization
    void SAL_CALL ConfigurationProvider2::initialize( const uno::Sequence< uno::Any >& _rArguments ) throw(uno::Exception, uno::RuntimeException)
    {
        MutexGuard aGuard(m_aMutex);

        if (m_pImpl || m_aSecurityOverride.size())
        {
            if (0 == _rArguments.getLength())
                // allow initialize without arguments ....
                return; //? Should this not ensureConnection() ?

            throw uno::Exception(::rtl::OUString::createFromAscii("The configuration provider has already been initialized."), THISREF());
        }

        const uno::Any* pArguments = _rArguments.getConstArray();
        beans::PropertyValue aCurrentArg;
        for (sal_Int32 i=0; i<_rArguments.getLength(); ++i, ++pArguments)
        {
            if (!((*pArguments) >>= aCurrentArg))
                throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii("Arguments have to be com.sun.star.beans.PropertyValue's."), THISREF(), i);

            // no check if the argument is known and valid. This would require to much testing
            m_aSecurityOverride[aCurrentArg.Name] = aCurrentArg.Value;
        }

        // connect here and now, thus the createInstanceWithArguments fails if no connection is made
        ensureConnection();
    }

    // XTypeProvider
    uno::Sequence< uno::Type > SAL_CALL ConfigurationProvider2::getTypes(  ) throw(uno::RuntimeException)
    {
        return ::comphelper::concatSequences(ServiceComponentImpl::getTypes(), ConfigurationProvider2_Base::getTypes());
    }

    // XInterface boilerplate
    uno::Any SAL_CALL ConfigurationProvider2::queryInterface(uno::Type const& rType) throw(uno::RuntimeException)
    {
        uno::Any aRet( ServiceComponentImpl::queryInterface(rType) );
        if ( !aRet.hasValue() )
            aRet = ConfigurationProvider2_Base::queryInterface(rType);
        return aRet;
    }

} // namespace configmgr


