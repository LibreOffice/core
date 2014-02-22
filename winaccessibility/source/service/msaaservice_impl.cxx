/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <rtl/ref.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XMSAAService.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <prewin.h>
#include <windows.h>
#include <postwin.h>

using namespace ::com::sun::star; 
using namespace ::com::sun::star::uno; 
using namespace ::com::sun::star::accessibility;

using namespace ::com::sun::star::awt;

#include "AccTopWindowListener.hxx"

namespace my_sc_impl
{

static Sequence< OUString > getSupportedServiceNames_MSAAServiceImpl()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = "com.sun.star.accessibility.MSAAService";
    return seqNames;
}

static OUString getImplementationName_MSAAServiceImpl()
{
    return OUString( "com.sun.star.accessibility.my_sc_implementation.MSAAService" );
}

class MSAAServiceImpl : public ::cppu::WeakImplHelper2<
            XMSAAService, lang::XServiceInfo >
{
private:
    rtl::Reference<AccTopWindowListener> m_pTopWindowListener;

public:
    MSAAServiceImpl ();
    virtual ~MSAAServiceImpl();

    
    virtual void SAL_CALL dispose();
    virtual void SAL_CALL addEventListener( const ::css::uno::Reference< ::css::lang::XEventListener >& )    { /* dummy */ }
    virtual void SAL_CALL removeEventListener( const ::css::uno::Reference< ::css::lang::XEventListener >& ) { /* dummy */ }

    
    virtual sal_Int64 SAL_CALL getAccObjectPtr(
            sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam);
    virtual void SAL_CALL handleWindowOpened(sal_Int64);

    
    virtual OUString SAL_CALL getImplementationName();
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames();
};

/**
   * Implemention of getAccObjectPtr.
   * @param
   * @return Com interface.
   */
sal_Int64 MSAAServiceImpl::getAccObjectPtr(
        sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam)
throw (RuntimeException)
{
    SolarMutexGuard g;

    if (!m_pTopWindowListener.is())
    {
        return 0;
    }
    return m_pTopWindowListener->GetMSComPtr(hWnd, lParam, wParam);
}

/**
   * Implemention of handleWindowOpened, the method will be invoked when a
   * top window is opened and AT starts up.
   * @param
   * @return
   */
void MSAAServiceImpl::handleWindowOpened(sal_Int64 nAcc)
{
    SolarMutexGuard g;

    SAL_INFO( "iacc2", "Window opened " << nAcc );

    if (m_pTopWindowListener.is() && nAcc)
    {
        m_pTopWindowListener->HandleWindowOpened(
            static_cast<com::sun::star::accessibility::XAccessible*>(
                reinterpret_cast<void*>(nAcc)));
    }
}

OUString MSAAServiceImpl::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.accessibility.my_sc_impl.MSAAService" );
}

/**
   * Implemention of XServiceInfo,return support service name.
   * @param Service name.
   * @return If the service name is supported.
   */
sal_Bool MSAAServiceImpl::supportsService( OUString const & serviceName ) throw (RuntimeException)
{
    return cppu::supportsService(this, serviceName);
}

/**
   * Implemention of XServiceInfo,return all service names.
   * @param.
   * @return service name sequence.
   */
Sequence< OUString > MSAAServiceImpl::getSupportedServiceNames() throw (RuntimeException)
{
    return getSupportedServiceNames_MSAAServiceImpl();
}

static void AccessBridgeHandleExistingWindow(const Reference< XMSAAService > &xAccMgr,
                                             Window *pWindow, bool bShow)
{
    if ( pWindow )
    {
        css::uno::Reference< css::accessibility::XAccessible > xAccessible;

        SAL_INFO( "iacc2", "Decide whether to register existing window with IAccessible2" );

        
        Window * pParentWindow = pWindow->GetParent();

        if ( pParentWindow )
        {
            try
            {
                
                css::uno::Reference< css::accessibility::XAccessible > xParentAccessible(pParentWindow->GetAccessible());
                if ( xParentAccessible.is() )
                {
                    css::uno::Reference< css::accessibility::XAccessibleContext > xParentAC( xParentAccessible->getAccessibleContext() );
                    if ( xParentAC.is() && (css::accessibility::AccessibleRole::COMBO_BOX == xParentAC->getAccessibleRole()) )
                    {
                        
                        
                        sal_Int32 nCount = xParentAC->getAccessibleChildCount();
                        for ( sal_Int32 n = 0; (n < nCount) && !xAccessible.is(); n++)
                        {
                            css::uno::Reference< css::accessibility::XAccessible > xChild = xParentAC->getAccessibleChild(n);
                            if ( xChild.is() )
                            {
                                css::uno::Reference< css::accessibility::XAccessibleContext > xChildAC = xChild->getAccessibleContext();
                                if ( xChildAC.is() && (css::accessibility::AccessibleRole::LIST == xChildAC->getAccessibleRole()) )
                                {
                                    xAccessible = xChild;
                                }
                            }
                        }
                    }
                }
            }
            catch (::com::sun::star::uno::RuntimeException e)
            {
                
                
                if (bShow)
                    return;
            }
        }

        
        
        
        if ( ! xAccessible.is() )
            xAccessible = pWindow->GetAccessible();

        assert( xAccMgr.is() );
        if ( xAccessible.is() )
        {
            xAccMgr->handleWindowOpened(
                    reinterpret_cast<sal_Int64>(xAccessible.get()));
            SAL_INFO( "iacc2", "Decide whether to register existing window with IAccessible2" );
        }
    }
}

/*
 * Setup and notify the OS of Accessible peers for all existing windows.
 */
static void AccessBridgeUpdateOldTopWindows( const Reference< XMSAAService > &xAccMgr )
{
    sal_uInt16 nTopWindowCount = (sal_uInt16)Application::GetTopWindowCount();

    for ( sal_uInt16 i = 0; i < nTopWindowCount; i++ )
    {
        Window* pTopWindow = Application::GetTopWindow( i );
        css::uno::Reference< css::accessibility::XAccessible > xAccessible = pTopWindow->GetAccessible();
        if ( xAccessible.is() )
        {
            css::uno::Reference< css::accessibility::XAccessibleContext > xAC( xAccessible->getAccessibleContext() );
            if ( xAC.is())
            {
                short role = xAC->getAccessibleRole();
                if ( !xAC->getAccessibleName().isEmpty() )
                    AccessBridgeHandleExistingWindow( xAccMgr, pTopWindow, true );
            }
        }
    }
}

/**
 * Static method that can create an entity of our MSAA Service
 * @param xContext No use here.
 * @return The object interface.
 */
Reference< XInterface > SAL_CALL create_MSAAServiceImpl( Reference< XComponentContext > const & /*xContext*/ ) SAL_THROW( () )
{
    Reference< XMSAAService > xAccMgr( new MSAAServiceImpl() );

    AccessBridgeUpdateOldTopWindows( xAccMgr );

    SAL_INFO("iacc2", "Created new IAccessible2 service impl.");

    return xAccMgr;
}

MSAAServiceImpl::MSAAServiceImpl()
{
    Reference< XExtendedToolkit > xToolkit =
        Reference< XExtendedToolkit >(Application::GetVCLToolkit(), UNO_QUERY);

    if( xToolkit.is() )
    {
        m_pTopWindowListener.set(new AccTopWindowListener());
        Reference<XTopWindowListener> const xRef(m_pTopWindowListener.get());
        xToolkit->addTopWindowListener( xRef );
        SAL_INFO( "iacc2", "successfully connected to the toolkit event hose" );
    }
    else
        SAL_WARN( "iacc2", "No VCL toolkit interface to listen to for events");
}

MSAAServiceImpl::~MSAAServiceImpl()
{
}

void MSAAServiceImpl::dispose()
{
    SolarMutexGuard g;

    
    
    
    
    
    
    m_pTopWindowListener.clear();
}

}

/* shared lib exports implemented without helpers in service_impl1.cxx */
namespace my_sc_impl
{
static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            create_MSAAServiceImpl, getImplementationName_MSAAServiceImpl,
            getSupportedServiceNames_MSAAServiceImpl,
            ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL iacc2_component_getFactory(
        sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
        registry::XRegistryKey * xRegistry )
    {
        return ::cppu::component_getFactoryHelper(
                   implName, xMgr, xRegistry, ::my_sc_impl::s_component_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
