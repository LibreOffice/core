/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confprovider2.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:06:13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <stdio.h>

#include "confprovider2.hxx"
#include "apiaccessobj.hxx"

#ifndef CONFIGMGR_API_PROVIDERIMPL2_HXX_
#include "confproviderimpl2.hxx"
#endif
#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#include "bootstrapcontext.hxx"
#endif
#ifndef CONFIGMGR_WRAPEXCEPTION_HXX
#include "wrapexception.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
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
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>

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
    //------------------------------------------------------------------------
        AsciiServiceName const aConfigProviderServices[] =
        {
            "com.sun.star.configuration.ConfigurationProvider",
            0
        };
        AsciiServiceName const aAdminProviderServices[] =
        {
            "com.sun.star.configuration.AdministrationProvider",
            0
        };
    //------------------------------------------------------------------------

        ServiceImplementationInfo const aConfigProviderInfo =
        {
            "com.sun.star.comp.configuration.ConfigurationProvider",
            aConfigProviderServices,
            0
        };
        ServiceImplementationInfo const aAdminProviderInfo =
        {
            "com.sun.star.comp.configuration.AdministrationProvider",
            aAdminProviderServices,
            aConfigProviderServices
        };
    //------------------------------------------------------------------------

        AsciiServiceName const
            aDefaultProviderServiceAndImplName  = A_DefaultProviderServiceAndImplName;

    //------------------------------------------------------------------------

        AsciiServiceName const aDefaultProviderServices[] =
        {
            aDefaultProviderServiceAndImplName,
            0
        };
    //------------------------------------------------------------------------

        ServiceRegistrationInfo const aDefaultProviderInfo =
        {
            aDefaultProviderServiceAndImplName,
            aDefaultProviderServices
        };
        SingletonRegistrationInfo const aDefaultProviderSingletonInfo =
        {
            A_DefaultProviderSingletonName,
            aDefaultProviderServiceAndImplName,
            aDefaultProviderServiceAndImplName,
            0
        };
    //------------------------------------------------------------------------
        typedef uno::Reference< uno::XInterface > (OConfigurationProviderImpl::*CreatorFunc)(const uno::Sequence< uno::Any >& aArguments);
        struct ServiceCreationInfo
        {
            ServiceRegistrationInfo const* info;
            CreatorFunc create;
        };

        static sal_Int32 getCreateServiceDataCount()
        {
            return 2;
        }

        static const ServiceCreationInfo* getCreateServiceData()
        {
            static ServiceCreationInfo const createServiceData[] =
            {
                { &configapi::aCreateReadAccessSI, &OConfigurationProviderImpl::createReadAccess },
                { &configapi::aCreateUpdateAccessSI, &OConfigurationProviderImpl::createUpdateAccess },
            };
            OSL_ENSURE(sizeof(createServiceData)/sizeof(createServiceData[0]) == getCreateServiceDataCount(),
                "getCreateServiceData : inconsistent data !");
            return createServiceData;
        }
    //------------------------------------------------------------------------
    }

    static ServiceCreationInfo const* findCreationInfo( const OUString& aServiceSpecifier )
    {
        for (int i= 0; i<getCreateServiceDataCount(); ++i)
        {
            ServiceCreationInfo const& rCreationInfo = getCreateServiceData()[i];

            ServiceRegistrationInfo const* pInfo = rCreationInfo.info;
            if (!pInfo)
                continue;

            if (AsciiServiceName pImplName = pInfo->implementationName)
            {
                if (0 == aServiceSpecifier.compareToAscii(pImplName))
                    return &rCreationInfo;
            }

            if (AsciiServiceName const* pNames = pInfo->registeredServiceNames)
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

    //#define ID_PREFETCH       1
    static const int ID_PREFETCH=1;
    static const int ID_ENABLEASYNC=2;

    static inline
    OUString getPrefetchPropName() { return OUString( RTL_CONSTASCII_USTRINGPARAM("PrefetchNodes") ); }
    static inline
    OUString getEnableAsyncPropName() { return OUString( RTL_CONSTASCII_USTRINGPARAM("EnableAsync") ); }
    //=============================================================================
    //= OConfigurationProvider
    //=============================================================================
    // service info export
    const ServiceRegistrationInfo* getConfigurationProviderServiceInfo()
    {
        return getRegistrationInfo(&aConfigProviderInfo);
    }

    const ServiceRegistrationInfo* getAdminProviderServiceInfo()
    {
        return getRegistrationInfo(&aAdminProviderInfo);
    }

    const ServiceRegistrationInfo* getDefaultProviderServiceInfo()
    {
        return &aDefaultProviderInfo;
    }

    const SingletonRegistrationInfo* getDefaultProviderSingletonInfo()
    {
        return &aDefaultProviderSingletonInfo;
    }

    //-----------------------------------------------------------------------------
    uno::Reference<uno::XInterface> SAL_CALL
        getDefaultConfigProviderSingleton( CreationContext const& xContext )
    {
        OSL_ENSURE( xContext.is(), "ERROR: NULL context has no singletons" );

        UnoContextTunnel aTunnel;
        aTunnel.passthru( xContext );

        uno::Reference<uno::XInterface> xResult;

        if (xContext.is())
        try
        {
            OUString aSingletonName = SINGLETON(A_DefaultProviderSingletonName);
            uno::Any aResult = xContext->getValueByName(aSingletonName);
            aResult >>= xResult;
        }
        catch (uno::Exception & )
        {
            // to do: really use the tunneled failure when that is set properly
            if ( aTunnel.recoverFailure(true).hasValue() )
            {
                // have a failure, but can't recover it
                // -> try to regenerate
                instantiateDefaultProvider(xContext);

                OSL_ENSURE(false, "Cannot recreate configuration backend instantiation failure - using generic error");
            }
            // cannot recover any failure
            throw;
        }

        return xResult;
    }
    // ------------------------------------------------------------------------
    // ----------------------------------------------------------------------------
    #define TUNNEL_ALL_EXCEPTIONS()     \
        WRAP_CONFIGBACKEND_CREATION_EXCEPTIONS1( UnoContextTunnel::tunnelFailure, true)

    // ----------------------------------------------------------------------------
    // ------------------------------------------------------------------------

    uno::Reference< uno::XInterface > SAL_CALL instantiateDefaultProvider( OProvider::CreationContext const & xTargetContext )
    {
        CreationContext xContext = UnoContextTunnel::recoverContext(xTargetContext);

        ServiceImplementationInfo const * pProviderInfo =
            ContextReader::testAdminService(xContext,true) ? &aAdminProviderInfo : &aConfigProviderInfo;

        OConfigurationProvider* pNewProvider = new OConfigurationProvider(xContext,pProviderInfo);
        uno::Reference< lang::XMultiServiceFactory > aRet( pNewProvider );

        try
        {
            pNewProvider->connect();
        }
        TUNNEL_ALL_EXCEPTIONS()

            return  uno::Reference< uno::XInterface >( aRet, uno::UNO_QUERY );
    }

    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    OConfigurationProvider::OConfigurationProvider(
        CreationContext const & xContext,
        const ServiceImplementationInfo* pServices
        )
       :OProvider(xContext,pServices)
       ,m_pImpl(NULL)
       ,m_bEnableAsync(false)
    {
        registerProperty(getPrefetchPropName(), ID_PREFETCH, 0,&m_aPrefetchNodes, ::getCppuType(&m_aPrefetchNodes));
        registerProperty(getEnableAsyncPropName(),  ID_ENABLEASYNC, 0, &m_bEnableAsync, ::getBooleanCppuType());
    }

    //-----------------------------------------------------------------------------
    OConfigurationProvider::~OConfigurationProvider()
    {
        delete m_pImpl;
    }

    //-----------------------------------------------------------------------------
    void OConfigurationProvider::connect() throw (uno::Exception)
    {
        OSL_ENSURE( m_pImpl == NULL, "Error: Configuration Provider already is connected");

        std::auto_ptr<OConfigurationProviderImpl> pNewImpl( new OConfigurationProviderImpl(this, m_xContext) );

        implConnect(*pNewImpl,ContextReader(m_xContext));

        m_pImpl = pNewImpl.release();
        if (m_pImpl)
        {
            sal_Bool isEnabled = m_pImpl->getDefaultOptions().isAsyncEnabled();
            this->setPropertyValue( getEnableAsyncPropName(), uno::makeAny(isEnabled) );
        }
    }

    //-----------------------------------------------------------------------------
    void SAL_CALL OConfigurationProvider::disposing()
    {
        UnoApiLock aLock;

        if (m_pImpl)
            m_pImpl->dispose();

        OProvider::disposing();
    }

    //-----------------------------------------------------------------------------
    uno::Reference< uno::XInterface > SAL_CALL OConfigurationProvider::createInstance( const OUString& aServiceSpecifier )
        throw(uno::Exception, uno::RuntimeException)
    {
        UnoApiLock aLock;
        // same as creating with empty sequence of arguments
        return this->createInstanceWithArguments( aServiceSpecifier, uno::Sequence< uno::Any >() );
    }

    //-----------------------------------------------------------------------------
    uno::Reference< uno::XInterface > SAL_CALL
        OConfigurationProvider::createInstanceWithArguments( const OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& aArguments )
            throw(uno::Exception, uno::RuntimeException)
    {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OConfigurationProvider: no implementation available");

        if (ServiceCreationInfo const* pInfo = findCreationInfo(aServiceSpecifier))
        {
            // it's a known service name - try to create without args
            if (CreatorFunc create = pInfo->create)
            {
                return (m_pImpl->*create)(aArguments);
            }
        }

        rtl::OUStringBuffer sMsg;
        sMsg.appendAscii("ConfigurationProvider: Cannot create view - ");
        sMsg.append( aServiceSpecifier ) .appendAscii(" is not a valid configuration access service. ");

        throw lang::ServiceNotRegisteredException(sMsg.makeStringAndClear(),*this);
    }

    //-----------------------------------------------------------------------------
    uno::Sequence< OUString > SAL_CALL OConfigurationProvider::getAvailableServiceNames(  )
        throw(uno::RuntimeException)
    {
        UnoApiLock aLock;

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
                ServiceRegistrationInfo const* pInfo = getCreateServiceData()[i].info;
                AsciiServiceName const* pNames = pInfo ? pInfo->registeredServiceNames : 0;

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

    // XLocalizable
    //-----------------------------------------------------------------------------
    void SAL_CALL OConfigurationProvider::setLocale( const lang::Locale& eLocale )
        throw (uno::RuntimeException)
    {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OConfigurationProvider: no implementation available");

        m_pImpl->setDefaultLocale( eLocale );
    }

    //-----------------------------------------------------------------------------
    lang::Locale SAL_CALL OConfigurationProvider::getLocale()
        throw (uno::RuntimeException)
    {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OConfigurationProvider: no implementation available");

        return m_pImpl->getDefaultOptions().getUnoLocale();
    }

    //XRefreshable
    //-----------------------------------------------------------------------------
    void SAL_CALL OConfigurationProvider::refresh()
        throw (uno::RuntimeException)
     {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OConfigurationProvider: no implementation available");

        try
        {
            m_pImpl->refreshAll();
        }
        catch (uno::RuntimeException& ) { throw; }
        catch (uno::Exception& e)
        {
            // FIXME: use getCaughtException()
            throw lang::WrappedTargetRuntimeException(e.Message, *this, uno::makeAny(e));
        }
        //Broadcast the changes
        uno::Reference< css::util::XRefreshListener > const * const pRefresh = 0;
        cppu::OInterfaceContainerHelper * aInterfaceContainer=
            getBroadcastHelper().getContainer (::getCppuType(pRefresh));

        if(aInterfaceContainer)
        {
            lang::EventObject aEventObject(*this);

            cppu::OInterfaceIteratorHelper aIterator(*aInterfaceContainer);
            while(aIterator.hasMoreElements())
            {
                uno::Reference< uno::XInterface > xIface = aIterator.next();
                uno::Reference< util::XRefreshListener > xRefresh (xIface, uno::UNO_QUERY);
                if (xRefresh.is())
                {
                    xRefresh->refreshed(aEventObject);
                }
            }
        }
     }
     //-----------------------------------------------------------------------------
     void SAL_CALL OConfigurationProvider::addRefreshListener(
         const uno::Reference< util::XRefreshListener >& aListener )
            throw (uno::RuntimeException)
     {
        UnoApiLock aLock;
        getBroadcastHelper().addListener(::getCppuType(&aListener), aListener);
     }
     //-----------------------------------------------------------------------------
     void SAL_CALL OConfigurationProvider::removeRefreshListener(
         const uno::Reference< util::XRefreshListener >& aListener )
            throw (uno::RuntimeException)
     {
        UnoApiLock aLock;
        getBroadcastHelper().removeListener(::getCppuType(&aListener), aListener);
     }
     //XFlushable
     //-----------------------------------------------------------------------------
     void SAL_CALL OConfigurationProvider::flush(  )
                throw (uno::RuntimeException)
     {
         UnoApiLock aLock;
         OSL_ENSURE(m_pImpl, "OConfigurationProvider: no implementation available");
         m_pImpl->flushAll();

         //Broadcast the changes
         uno::Reference< css::util::XFlushListener > const * const pFlush = 0;
         cppu::OInterfaceContainerHelper * aInterfaceContainer=
            getBroadcastHelper().getContainer(::getCppuType(pFlush));

         if(aInterfaceContainer)
         {
             lang::EventObject aEventObject(*this);

             cppu::OInterfaceIteratorHelper aIterator(*aInterfaceContainer);
             while(aIterator.hasMoreElements())
             {
                 uno::Reference< uno::XInterface > xIface = aIterator.next();
                 uno::Reference< util::XFlushListener > xFlush (xIface, uno::UNO_QUERY);
                 if (xFlush.is())
                 {
                     xFlush->flushed(aEventObject);
                 }
             }
         }
     }
     //-----------------------------------------------------------------------------
     void SAL_CALL OConfigurationProvider::addFlushListener(
         const uno::Reference< util::XFlushListener >& aListener )
            throw (uno::RuntimeException)
     {
          UnoApiLock aLock;
          getBroadcastHelper().addListener(::getCppuType(&aListener), aListener);
     }
     //-----------------------------------------------------------------------------
     void SAL_CALL OConfigurationProvider::removeFlushListener(
         const uno::Reference< util::XFlushListener >& aListener )
            throw (uno::RuntimeException)
     {
         UnoApiLock aLock;
         getBroadcastHelper().removeListener(::getCppuType(&aListener), aListener);
     }
    // XInterface
    //-----------------------------------------------------------------------------
    ::com::sun::star::uno::Any SAL_CALL OConfigurationProvider::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
    {
        UnoApiLock aLock;
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
        UnoApiLock aLock;
        uno::Sequence< beans::Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }
    // -------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper & OConfigurationProvider::getInfoHelper()
    {
        UnoApiLock aLock;
        return *const_cast<OConfigurationProvider*>(this)->getArrayHelper();
    }

    //-----------------------------------------------------------------------------


    void SAL_CALL OConfigurationProvider::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue)
                                                 throw (::com::sun::star::uno::Exception)
    {
        UnoApiLock aLock;

        OProvider::setFastPropertyValue_NoBroadcast( nHandle, rValue );

        switch(nHandle)
        {
            case ID_PREFETCH:
            {
                uno::Sequence< OUString > aNodeList;
                rValue >>= aNodeList;

                RequestOptions const aOptions = m_pImpl->getDefaultOptions();

                for (sal_Int32 i = 0; i < aNodeList.getLength(); i++)
                {
                    using namespace configuration;
                    AbsolutePath aModulePath = AbsolutePath::makeModulePath(aNodeList[i], AbsolutePath::NoValidate());
                    m_pImpl->fetchSubtree(aModulePath , aOptions);
                }
            }break;

            case ID_ENABLEASYNC:
            {
                //Forward to TreeManager
                sal_Bool bAsync;
                if (rValue >>= bAsync)
                {
                    m_pImpl->enableAsync(bAsync);
                    if (!bAsync)
                        this->flush();
                }
                else
                    OSL_ENSURE(false, "Unexpected type of new property value");
            }
            break;
            default:
            {
                OSL_ENSURE(false, "OConfigurationProvider::setFastPropertyValue_NoBroadcast -unknown property");
            }

        }
    }


} // namespace configmgr


