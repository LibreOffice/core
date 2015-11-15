/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>
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
#include <postwin.h>

using namespace ::com::sun::star; // for odk interfaces
using namespace ::com::sun::star::uno; // for basic types
using namespace ::com::sun::star::accessibility;

using namespace ::com::sun::star::awt;

#include "AccTopWindowListener.hxx"

namespace my_sc_impl
{

static Sequence< OUString > getSupportedServiceNames_MSAAServiceImpl()
{
    Sequence< OUString > seqNames { "com.sun.star.accessibility.MSAAService" };
    return seqNames;
}

static OUString getImplementationName_MSAAServiceImpl()
{
    return OUString( "com.sun.star.accessibility.my_sc_implementation.MSAAService" );
}

class MSAAServiceImpl : public ::cppu::WeakImplHelper<
            XMSAAService, lang::XServiceInfo >
{
private:
    rtl::Reference<AccTopWindowListener> m_pTopWindowListener;

public:
    MSAAServiceImpl ();
    virtual ~MSAAServiceImpl();

    // XComponent - as used by VCL to lifecycle manage this bridge.
    virtual void SAL_CALL dispose();
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& )    { /* dummy */ }
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& ) { /* dummy */ }

    // XMSAAService
    virtual sal_Int64 SAL_CALL getAccObjectPtr(
            sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam);
    virtual void SAL_CALL handleWindowOpened(sal_Int64);

    // XServiceInfo
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
    return getImplementationName_MSAAServiceImpl();
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
                                             vcl::Window *pWindow, bool bShow)
{
    if ( pWindow )
    {
        css::uno::Reference< css::accessibility::XAccessible > xAccessible;

        SAL_INFO( "iacc2", "Decide whether to register existing window with IAccessible2" );

        // Test for combo box - drop down floating windows first
        vcl::Window * pParentWindow = pWindow->GetParent();

        if ( pParentWindow )
        {
            try
            {
                // The parent window of a combo box floating window should have the role COMBO_BOX
                css::uno::Reference< css::accessibility::XAccessible > xParentAccessible(pParentWindow->GetAccessible());
                if ( xParentAccessible.is() )
                {
                    css::uno::Reference< css::accessibility::XAccessibleContext > xParentAC( xParentAccessible->getAccessibleContext() );
                    if ( xParentAC.is() && (css::accessibility::AccessibleRole::COMBO_BOX == xParentAC->getAccessibleRole()) )
                    {
                        // O.k. - this is a combo box floating window corresponding to the child of role LIST of the parent.
                        // Let's not rely on a specific child order, just search for the child with the role LIST
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
            catch (::com::sun::star::uno::RuntimeException const&)
            {
                // Ignore show events that throw DisposedExceptions in getAccessibleContext(),
                // but keep revoking these windows in hide(s).
                if (bShow)
                    return;
            }
        }

        // We have to rely on the fact that Window::GetAccessible()->getAccessibleContext() returns a valid XAccessibleContext
        // also for other menus than menubar or toplevel popup window. Otherwise we had to traverse the hierarchy to find the
        // context object to this menu floater. This makes the call to Window->IsMenuFloatingWindow() obsolete.
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
        vcl::Window* pTopWindow = Application::GetTopWindow( i );
        css::uno::Reference< css::accessibility::XAccessible > xAccessible = pTopWindow->GetAccessible();
        if ( xAccessible.is() )
        {
            css::uno::Reference< css::accessibility::XAccessibleContext > xAC( xAccessible->getAccessibleContext() );
            if ( xAC.is())
            {
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
Reference< XInterface > SAL_CALL create_MSAAServiceImpl( Reference< XComponentContext > const & /*xContext*/ )
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

    // As all folders and streams contain references to their parents,
    // we must remove these references so that they will be deleted when
    // the hash_map of the root folder is cleared, releasing all subfolders
    // and substreams which in turn release theirs, etc. When xRootFolder is
    // released when this destructor completes, the folder tree should be
    // deleted fully (and automagically).
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
