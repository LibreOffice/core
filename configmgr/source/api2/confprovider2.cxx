/*************************************************************************
 *
 *  $RCSfile: confprovider2.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-07 17:17:50 $
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
#include <algorithm>
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

    namespace
    {
        typedef uno::Reference< uno::XInterface > (OConfigurationProviderImpl::*CreatorFunc)(const uno::Sequence< uno::Any >& aArguments);
        struct ServiceCreationInfo
        {
            ServiceInfo const* info;
            CreatorFunc create;
        };

        AsciiServiceName const aProviderServices[] =
        {
            "com.sun.star.configuration.ConfigurationProvider",
            0
        };
        ServiceInfo const aProviderInfo =
        {
            "com.sun.star.comp.configuration.ConfigurationProvider",
            aProviderServices
        };

        static sal_Int32 getCreateServiceDataCount()
        {
            return 2;
        };

        static const ServiceCreationInfo* getCreateServiceData()
        {
            static ServiceCreationInfo const createServiceData[] =
            {
                { &configapi::aCreateReadAccessSI, &OConfigurationProviderImpl::createReadAccess },
                { &configapi::aCreateUpdateAccessSI, &OConfigurationProviderImpl::createUpdateAccess },
            };
            OSL_ENSHURE(sizeof(createServiceData)/sizeof(createServiceData[0]) == getCreateServiceDataCount(),
                "getCreateServiceData : inconsistent data !");
            return createServiceData;
        }
    }

    static ServiceCreationInfo const* findCreationInfo( const OUString& aServiceSpecifier )
    {
        for (int i= 0; i<getCreateServiceDataCount(); ++i)
        {
            ServiceCreationInfo const& rCreationInfo = getCreateServiceData()[i];

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

    #define ID_PREFETCH     1

    //=============================================================================
    //= OConfigurationProvider
    //=============================================================================
    ServiceInfo const& OConfigurationProvider::staticServiceInfo = aProviderInfo;
    //-----------------------------------------------------------------------------
    OConfigurationProvider::OConfigurationProvider(const uno::Reference< lang::XMultiServiceFactory >& _xServiceFactory)
                           :OProvider(_xServiceFactory,&aProviderInfo)
                           ,m_pImpl(NULL)
    {
        registerProperty(rtl::OUString::createFromAscii("PrefetchNodes"),   ID_PREFETCH,            0,&m_aPrefetchNodes, ::getCppuType(static_cast< uno::Sequence< rtl::OUString > const * >(0) ));
    }

    //-----------------------------------------------------------------------------
    OConfigurationProvider::~OConfigurationProvider()
    {
        delete m_pImpl;
    }

    //-----------------------------------------------------------------------------
    void OConfigurationProvider::connect(const ConnectionSettings& _rSettings) throw (uno::Exception)
    {
        ::rtl::OUString sService(RTL_CONSTASCII_USTRINGPARAM("configuration"));
        ConnectionSettings aSettings(_rSettings);
        aSettings.setService(sService);

        IConfigSession* pNewSession = aSettings.createConnection(m_xServiceFactory);
        if (!pNewSession)
            throw uno::Exception(::rtl::OUString::createFromAscii("Could not connect to the configuration registry. Please check your settings."), NULL);
        m_pImpl = new OConfigurationProviderImpl(this, pNewSession, m_xServiceFactory, _rSettings);
    }

    //-----------------------------------------------------------------------------
    void SAL_CALL OConfigurationProvider::disposing()
    {
        if (m_pImpl)
            m_pImpl->dispose();

        OProvider::disposing();
    }

    //-----------------------------------------------------------------------------
    uno::Reference< uno::XInterface > SAL_CALL OConfigurationProvider::createInstance( const OUString& aServiceSpecifier )
        throw(uno::Exception, uno::RuntimeException)
    {

        OSL_ENSURE(m_pImpl, "OConfigurationProvider: no implementation available");

        CFG_TRACE_INFO("going to create a read access instance for %s", "missing unicode conversion");
        if (ServiceCreationInfo const* pInfo = findCreationInfo(aServiceSpecifier))
        {
            // it's a known service name - try to create without args
            if (CreatorFunc create = pInfo->create)
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

    //-----------------------------------------------------------------------------
    uno::Reference< uno::XInterface > SAL_CALL
        OConfigurationProvider::createInstanceWithArguments( const ::rtl::OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& aArguments )
            throw(uno::Exception, uno::RuntimeException)
    {
        OSL_ENSURE(m_pImpl, "OConfigurationProvider: no implementation available");

        if (ServiceCreationInfo const* pInfo = findCreationInfo(aServiceSpecifier))
        {
            // it's a known service name - try to create without args
            if (CreatorFunc create = pInfo->create)
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

    //-----------------------------------------------------------------------------
    uno::Sequence< OUString > SAL_CALL OConfigurationProvider::getAvailableServiceNames(  )
        throw(uno::RuntimeException)
    {
        sal_Int32 nCount = 0;
        {
            for (int i= 0; i< getCreateServiceDataCount(); ++i)
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

    // XInterface
    //-----------------------------------------------------------------------------
    ::com::sun::star::uno::Any SAL_CALL OConfigurationProvider::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
    {

        uno::Any aRet( OProvider::queryInterface(rType) );
        if ( !aRet.hasValue() )
            aRet = queryPropertyInterface(rType);
        return aRet;
    }

    //XTypeProvider
    //-----------------------------------------------------------------------------
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OConfigurationProvider::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
    {
        cppu::OTypeCollection aCollection(::getCppuType( (const uno::Reference< beans::XPropertySet > *)0 ),
                                          ::getCppuType( (const uno::Reference< beans::XFastPropertySet > *)0 ),
                                          ::getCppuType( (const uno::Reference< beans::XMultiPropertySet > *)0 ),
                                          OProvider::getTypes());
        return aCollection.getTypes();
    }

    // OPropertyArrayUsageHelper
    // -------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OConfigurationProvider::createArrayHelper( ) const
    {
        uno::Sequence< beans::Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }
    // -------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper & OConfigurationProvider::getInfoHelper()
    {
        return *const_cast<OConfigurationProvider*>(this)->getArrayHelper();
    }

    //-----------------------------------------------------------------------------
    void SAL_CALL OConfigurationProvider::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue)
                                                 throw (::com::sun::star::uno::Exception)
    {
        OProvider::setFastPropertyValue_NoBroadcast( nHandle, rValue );

        uno::Sequence< OUString > aNodeList;
        rValue >>= aNodeList;

        ::vos::ORef<OOptions> xOptions(new OOptions(m_pImpl->getDefaultOptions()));

        for (sal_Int32 i = 0; i < aNodeList.getLength(); i++)
            m_pImpl->fetchSubtree(aNodeList[i], xOptions);
    }

} // namespace configmgr


