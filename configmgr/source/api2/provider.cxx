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
#include "provider.hxx"
#include "providerimpl.hxx"
#include "tracer.hxx"
#include "bootstrap.hxx"
#include <osl/mutex.hxx>
#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
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

    // -----------------------------------------------------------------------------
    typedef ::cppu::WeakImplHelper1<lang::XEventListener> XEventListener_BASE;
    class OProviderDisposingListener : public XEventListener_BASE
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
    OProvider::OProvider(CreationContext const & xContext, ServiceImplementationInfo const* pInfo)
              :ServiceComponentImpl(pInfo)
              ,OPropertyContainer(ServiceComponentImpl::rBHelper)
              ,m_xContext(xContext)
    {
        OSL_ENSURE(m_xContext.is(), "Creating a provider without a context");
        attachToContext();
    }

    //-----------------------------------------------------------------------------
    OProvider::~OProvider()
    {
        discardContext(releaseContext());
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
        return ::comphelper::concatSequences(ServiceComponentImpl::getTypes(), OProvider_Base::getTypes());
    }

    // XPropertySet
    //-----------------------------------------------------------------------------
    uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OProvider::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
    }

    // XInterface
    //-----------------------------------------------------------------------------
    uno::Any SAL_CALL OProvider::queryInterface(uno::Type const& rType) throw(uno::RuntimeException)
    {
        UnoApiLock aLock;
        uno::Any aRet( ServiceComponentImpl::queryInterface(rType) );
        if ( !aRet.hasValue() )
            aRet = OProvider_Base::queryInterface(rType);
        return aRet;
    }

    //-----------------------------------------------------------------------------
    void OProvider::implConnect(OProviderImpl& _rFreshProviderImpl, const ContextReader& _rSettings) throw(uno::Exception)
    {
        UnoApiLock aLock;
        if (!_rFreshProviderImpl.initSession(_rSettings))
            throw uno::Exception(::rtl::OUString::createFromAscii("Could not connect to the configuration. Please check your settings."), THISREF() );
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
        uno::Reference< lang::XComponent > xComp = releaseContext();

        ServiceComponentImpl::disposing();
        OPropertyContainer::disposing();

        discardContext( xComp );
    }
/*
    // com::sun::star::lang::XUnoTunnel
    //------------------------------------------------------------------
    sal_Int64 OProvider::getSomething( const uno::Sequence< sal_Int8 > & _rId ) throw (uno::RuntimeException)
    {
        if (_rId.getLength() != 16)
            return 0;

        if (0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  _rId.getConstArray(), 16 ) )
            return reinterpret_cast<sal_Int64>(this);

        return 0;
    }

    //--------------------------------------------------------------------------
    uno::Sequence< sal_Int8 > OProvider::getUnoTunnelImplementationId() throw (uno::RuntimeException)
    {
        static ::cppu::OImplementationId * pId = 0;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }
*/
} // namespace configmgr


