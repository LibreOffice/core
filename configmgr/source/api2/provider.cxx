/*************************************************************************
 *
 *  $RCSfile: provider.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:36 $
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
#include "provider.hxx"
#include "providerimpl.hxx"

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
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
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
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
        uno::Any aRet( ServiceComponentImpl::queryInterface(rType) );
        if ( !aRet.hasValue() )
            aRet = OProvider_Base::queryInterface(rType);
        return aRet;
    }

    //-----------------------------------------------------------------------------
    void OProvider::implConnect(OProviderImpl& _rFreshProviderImpl, const ContextReader& _rSettings) throw(uno::Exception)
    {
        if (!_rFreshProviderImpl.initSession(_rSettings))
            throw uno::Exception(::rtl::OUString::createFromAscii("Could not connect to the configuration. Please check your settings."), THISREF() );
    }

    //-----------------------------------------------------------------------------
    void SAL_CALL OProvider::disposing(com::sun::star::lang::EventObject const& rEvt) throw()
    {
        releaseContext();
        this->dispose();
    }

    //-----------------------------------------------------------------------------
    void SAL_CALL OProvider::disposing()
    {
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


