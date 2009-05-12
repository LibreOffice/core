/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: provider.cxx,v $
 * $Revision: 1.19 $
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

#include "apiserviceinfo.hxx"
#include "confapifactory.hxx"
#include "configpath.hxx"
#include "datalock.hxx"
#include "provider.hxx"
#include "providerimpl.hxx"
#include "tracer.hxx"
#include "bootstrap.hxx"
#include "wrapexception.hxx"
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

namespace configmgr
{
    namespace css  = ::com::sun::star;
    namespace uno  = css::uno;
    namespace lang = css::lang;
    namespace beans = css::beans;

    namespace
    {
    //------------------------------------------------------------------------
        sal_Char const * const aConfigProviderServices[] =
        {
            "com.sun.star.configuration.ConfigurationProvider",
            0
        };
        sal_Char const * const aAdminProviderServices[] =
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

        sal_Char const * const
            aDefaultProviderServiceAndImplName  = A_DefaultProviderServiceAndImplName;

    //------------------------------------------------------------------------

        sal_Char const * const aDefaultProviderServices[] =
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
        typedef uno::Reference< uno::XInterface > (OProviderImpl::*CreatorFunc)(const uno::Sequence< uno::Any >& aArguments);
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
                { &configapi::aCreateReadAccessSI, &OProviderImpl::createReadAccess },
                { &configapi::aCreateUpdateAccessSI, &OProviderImpl::createUpdateAccess },
            };
            OSL_ENSURE(sizeof(createServiceData)/sizeof(createServiceData[0]) == getCreateServiceDataCount(),
                "getCreateServiceData : inconsistent data !");
            return createServiceData;
        }
    //------------------------------------------------------------------------
    }

    static ServiceCreationInfo const* findCreationInfo( const rtl::OUString& aServiceSpecifier )
    {
        for (int i= 0; i<getCreateServiceDataCount(); ++i)
        {
            ServiceCreationInfo const& rCreationInfo = getCreateServiceData()[i];

            ServiceRegistrationInfo const* pInfo = rCreationInfo.info;
            if (!pInfo)
                continue;

            if (sal_Char const * pImplName = pInfo->implementationName)
            {
                if (0 == aServiceSpecifier.compareToAscii(pImplName))
                    return &rCreationInfo;
            }

            if (sal_Char const * const* pNames = pInfo->registeredServiceNames)
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
    rtl::OUString getPrefetchPropName() { return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PrefetchNodes") ); }
    static inline
    rtl::OUString getEnableAsyncPropName() { return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("EnableAsync") ); }
    //=============================================================================
    //= OProvider
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
        getDefaultConfigProviderSingleton( uno::Reference< uno::XComponentContext > const& xContext )
    {
        OSL_ENSURE( xContext.is(), "ERROR: NULL context has no singletons" );

        UnoContextTunnel aTunnel;
        aTunnel.passthru( xContext );

        uno::Reference<uno::XInterface> xResult;

        if (xContext.is())
        try
        {
            rtl::OUString aSingletonName = SINGLETON(A_DefaultProviderSingletonName);
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

    uno::Reference< uno::XInterface > SAL_CALL instantiateDefaultProvider( uno::Reference< uno::XComponentContext > const & xTargetContext )
    {
        uno::Reference< uno::XComponentContext > xContext = UnoContextTunnel::recoverContext(xTargetContext);

        ServiceImplementationInfo const * pProviderInfo =
            ContextReader::testAdminService(xContext,true) ? &aAdminProviderInfo : &aConfigProviderInfo;

        OProvider* pNewProvider = new OProvider(xContext,pProviderInfo);
        uno::Reference< lang::XMultiServiceFactory > aRet( pNewProvider );

        try
        {
            pNewProvider->connect();
        }
        TUNNEL_ALL_EXCEPTIONS()

            return  uno::Reference< uno::XInterface >( aRet, uno::UNO_QUERY );
    }

    // -----------------------------------------------------------------------------
    class OProviderDisposingListener : public cppu::WeakImplHelper1<lang::XEventListener>
    {
        OProvider* m_pProvider;

    public:
        OProviderDisposingListener(OProvider* pProvider)
            :m_pProvider(pProvider){}

        virtual void SAL_CALL disposing(com::sun::star::lang::EventObject const& rEvt) throw()
            {
                CFG_TRACE_INFO("Service Manager or context disposed, disposing the provider");
                if (OProvider* pProvider = m_pProvider)
                {
                    m_pProvider = NULL;
                    pProvider->disposing(rEvt);
                }
            }
    };

    //=============================================================================
    //= OProvider
    //=============================================================================
    //-----------------------------------------------------------------------------
    OProvider::OProvider(uno::Reference< uno::XComponentContext > const & xContext, ServiceImplementationInfo const* pInfo)
              :ServiceComponentImpl(pInfo)
              ,OPropertyContainer(ServiceComponentImpl::rBHelper)
              ,m_xContext(xContext)
              ,m_pImpl(NULL)
              ,m_bEnableAsync(false)
    {
        OSL_ENSURE(m_xContext.is(), "Creating a provider without a context");
        attachToContext();
        registerProperty(getPrefetchPropName(), ID_PREFETCH, 0,&m_aPrefetchNodes, ::getCppuType(&m_aPrefetchNodes));
        registerProperty(getEnableAsyncPropName(),  ID_ENABLEASYNC, 0, &m_bEnableAsync, ::getBooleanCppuType());
    }

    //-----------------------------------------------------------------------------
    OProvider::~OProvider()
    {
        delete m_pImpl;
        discardContext(releaseContext());
    }

    //-----------------------------------------------------------------------------
    void OProvider::connect() throw (uno::Exception)
    {
        OSL_ENSURE( m_pImpl == NULL, "Error: Configuration Provider already is connected");

        std::auto_ptr<OProviderImpl> pNewImpl( new OProviderImpl(this, m_xContext) );

        implConnect(*pNewImpl,ContextReader(m_xContext));

        m_pImpl = pNewImpl.release();
        if (m_pImpl)
        {
            sal_Bool isEnabled = m_pImpl->getDefaultOptions().isAsyncEnabled();
            this->setPropertyValue( getEnableAsyncPropName(), uno::makeAny(isEnabled) );
        }
    }

    //-----------------------------------------------------------------------------
    void OProvider::attachToContext()
    {
        UnoApiLock aLock;
        ::osl::MutexGuard aGuard(ServiceComponentImpl::rBHelper.rMutex);
        OSL_ASSERT(!m_xDisposeListener.is());
        if (m_xContext.is())
        {
            uno::Reference< lang::XComponent > xContextComp(m_xContext, uno::UNO_QUERY);
            uno::Reference< lang::XComponent > xServiceMgrComp(m_xContext->getServiceManager(), uno::UNO_QUERY);

            m_xDisposeListener = new OProviderDisposingListener(this);

            if (xContextComp.is())    xContextComp   ->addEventListener(m_xDisposeListener);
            if (xServiceMgrComp.is()) xServiceMgrComp->addEventListener(m_xDisposeListener);

            OSL_ENSURE(xServiceMgrComp.is() || xContextComp.is(),
                        "Provider cannot detect shutdown -> no XComponent found");
        }
    }

    //-----------------------------------------------------------------------------
    uno::Reference< lang::XComponent > OProvider::releaseContext()
    {
        UnoApiLock aLock;
        ::osl::MutexGuard aGuard(ServiceComponentImpl::rBHelper.rMutex);

        uno::Reference< lang::XComponent > xContextComp(m_xContext, uno::UNO_QUERY);
        if (m_xDisposeListener.is() && m_xContext.is())
        {
            uno::Reference< lang::XComponent > xServiceMgrComp(m_xContext->getServiceManager(), uno::UNO_QUERY);

            if (xContextComp.is())
            {
                try { xContextComp   ->removeEventListener(m_xDisposeListener); }
                catch (uno::Exception & ) {}
            }

            if (xServiceMgrComp.is())
            {
                try { xServiceMgrComp->removeEventListener(m_xDisposeListener); }
                catch (uno::Exception & ) {}
            }
        }
        m_xDisposeListener = NULL;
        m_xContext = NULL;

        return xContextComp;
    }
    //-----------------------------------------------------------------------------

    void OProvider::discardContext(uno::Reference< lang::XComponent > const & xContext)
    {
        UnoApiLock aLock;
        if (xContext.is())
        {
            uno::Reference< uno::XComponentContext > xCC(xContext,uno::UNO_QUERY);
            OSL_ASSERT(xCC.is());

            if (BootstrapContext::isWrapper(xCC))
            {
                try { xContext->dispose(); }
                catch (uno::Exception & ) {}
            }
        }
    }
    //-----------------------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------------------
    uno::Sequence< uno::Type > SAL_CALL OProvider::getTypes(  ) throw(uno::RuntimeException)
    {
        cppu::OTypeCollection aCollection(::getCppuType( (const uno::Reference< beans::XPropertySet > *)0 ),
                                          ::getCppuType( (const uno::Reference< beans::XFastPropertySet > *)0 ),
                                          ::getCppuType( (const uno::Reference< beans::XMultiPropertySet > *)0 ),
                                          ::comphelper::concatSequences(ServiceComponentImpl::getTypes(), cppu::ImplHelper4< lang::XMultiServiceFactory, lang::XLocalizable, util::XRefreshable, util::XFlushable >::getTypes()));
        return aCollection.getTypes();
    }

    // XPropertySet
    //-----------------------------------------------------------------------------
    uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OProvider::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
    }

    //-----------------------------------------------------------------------------
    uno::Reference< uno::XInterface > SAL_CALL OProvider::createInstance( const rtl::OUString& aServiceSpecifier )
        throw(uno::Exception, uno::RuntimeException)
    {
        UnoApiLock aLock;
        // same as creating with empty sequence of arguments
        return this->createInstanceWithArguments( aServiceSpecifier, uno::Sequence< uno::Any >() );
    }

    //-----------------------------------------------------------------------------
    uno::Reference< uno::XInterface > SAL_CALL
        OProvider::createInstanceWithArguments( const rtl::OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& aArguments )
            throw(uno::Exception, uno::RuntimeException)
    {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OProvider: no implementation available");

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
    uno::Sequence< rtl::OUString > SAL_CALL OProvider::getAvailableServiceNames(  )
        throw(uno::RuntimeException)
    {
        UnoApiLock aLock;

        sal_Int32 nCount = 0;
        {
            for (int i= 0; i< getCreateServiceDataCount(); ++i)
                nCount += countServices(getCreateServiceData()[i].info);
        }

        uno::Sequence< rtl::OUString > aNames(nCount);
        if (nCount > 0)
        {
            sal_Int32 n = 0;
            for (int i= 0; i<getCreateServiceDataCount(); ++i)
            {
                ServiceRegistrationInfo const* pInfo = getCreateServiceData()[i].info;
                sal_Char const * const* pNames = pInfo ? pInfo->registeredServiceNames : 0;

                if (pNames)
                {
                    while(*pNames)
                    {
                        aNames[n] = rtl::OUString::createFromAscii(*pNames);
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
    void SAL_CALL OProvider::setLocale( const lang::Locale& eLocale )
        throw (uno::RuntimeException)
    {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OProvider: no implementation available");

        m_pImpl->setDefaultLocale( eLocale );
    }

    //-----------------------------------------------------------------------------
    lang::Locale SAL_CALL OProvider::getLocale()
        throw (uno::RuntimeException)
    {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OProvider: no implementation available");

        return m_pImpl->getDefaultOptions().getUnoLocale();
    }

    //XRefreshable
    //-----------------------------------------------------------------------------
    void SAL_CALL OProvider::refresh()
        throw (uno::RuntimeException)
     {
        UnoApiLock aLock;

        OSL_ENSURE(m_pImpl, "OProvider: no implementation available");

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
     void SAL_CALL OProvider::addRefreshListener(
         const uno::Reference< util::XRefreshListener >& aListener )
            throw (uno::RuntimeException)
     {
        UnoApiLock aLock;
        getBroadcastHelper().addListener(::getCppuType(&aListener), aListener);
     }
     //-----------------------------------------------------------------------------
     void SAL_CALL OProvider::removeRefreshListener(
         const uno::Reference< util::XRefreshListener >& aListener )
            throw (uno::RuntimeException)
     {
        UnoApiLock aLock;
        getBroadcastHelper().removeListener(::getCppuType(&aListener), aListener);
     }
     //XFlushable
     //-----------------------------------------------------------------------------
     void SAL_CALL OProvider::flush(  )
                throw (uno::RuntimeException)
     {
         UnoApiLock aLock;
         OSL_ENSURE(m_pImpl, "OProvider: no implementation available");
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
     void SAL_CALL OProvider::addFlushListener(
         const uno::Reference< util::XFlushListener >& aListener )
            throw (uno::RuntimeException)
     {
          UnoApiLock aLock;
          getBroadcastHelper().addListener(::getCppuType(&aListener), aListener);
     }
     //-----------------------------------------------------------------------------
     void SAL_CALL OProvider::removeFlushListener(
         const uno::Reference< util::XFlushListener >& aListener )
            throw (uno::RuntimeException)
     {
         UnoApiLock aLock;
         getBroadcastHelper().removeListener(::getCppuType(&aListener), aListener);
     }

    // XInterface
    //-----------------------------------------------------------------------------
    uno::Any SAL_CALL OProvider::queryInterface(uno::Type const& rType) throw(uno::RuntimeException)
    {
        UnoApiLock aLock;
        uno::Any aRet( ServiceComponentImpl::queryInterface(rType) );
        if ( !aRet.hasValue() )
            aRet = cppu::ImplHelper4< lang::XMultiServiceFactory, lang::XLocalizable, util::XRefreshable, util::XFlushable >::queryInterface(rType);
        if ( !aRet.hasValue() )
            aRet = queryPropertyInterface(rType);
        return aRet;
    }

    //-----------------------------------------------------------------------------
    void OProvider::implConnect(OProviderImpl& _rFreshProviderImpl, const ContextReader& _rSettings) throw(uno::Exception)
    {
        UnoApiLock aLock;
        if (!_rFreshProviderImpl.initSession(_rSettings))
            throw uno::Exception(::rtl::OUString::createFromAscii("Could not connect to the configuration. Please check your settings."), static_cast< ::cppu::OWeakObject* >(this) );
    }

    //-----------------------------------------------------------------------------
       void SAL_CALL OProvider::disposing(com::sun::star::lang::EventObject const& /*rEvt*/) throw()
    {
        UnoApiLock aLock;
        releaseContext();
        this->dispose();
    }

    //-----------------------------------------------------------------------------
    void SAL_CALL OProvider::disposing()
    {
        UnoApiLock aLock;

        if (m_pImpl)
            m_pImpl->dispose();

        uno::Reference< lang::XComponent > xComp = releaseContext();

        ServiceComponentImpl::disposing();
        OPropertyContainer::disposing();

        discardContext( xComp );
    }

    // OPropertyArrayUsageHelper
    // -------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OProvider::createArrayHelper( ) const
    {
        UnoApiLock aLock;
        uno::Sequence< beans::Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }
    // -------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper & OProvider::getInfoHelper()
    {
        UnoApiLock aLock;
        return *getArrayHelper();
    }

    void SAL_CALL OProvider::setFastPropertyValue_NoBroadcast(  sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue)
                                                 throw (::com::sun::star::uno::Exception)
    {
        UnoApiLock aLock;

        OPropertyContainer::setFastPropertyValue_NoBroadcast( nHandle, rValue );

        switch(nHandle)
        {
            case ID_PREFETCH:
            {
                uno::Sequence< rtl::OUString > aNodeList;
                rValue >>= aNodeList;

                RequestOptions const aOptions = m_pImpl->getDefaultOptions();

                for (sal_Int32 i = 0; i < aNodeList.getLength(); i++)
                {
                    configuration::AbsolutePath aModulePath = configuration::AbsolutePath::makeModulePath(aNodeList[i]);
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
                OSL_ENSURE(false, "OProvider::setFastPropertyValue_NoBroadcast -unknown property");
            }

        }
    }
} // namespace configmgr
