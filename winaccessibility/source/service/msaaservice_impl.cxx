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

#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/accessibility/XMSAAService.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <prewin.h>
#include <windows.h>
#include <postwin.h>

using namespace ::rtl; // for OUString
using namespace ::com::sun::star; // for odk interfaces
using namespace ::com::sun::star::uno; // for basic types
using namespace ::com::sun::star::accessibility;

using namespace ::com::sun::star::awt;

#include "AccTopWindowListener.hxx"
#include "g_msacc.hxx"

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

class MSAAServiceImpl : public ::cppu::WeakImplHelper4<
            XMSAAService, lang::XServiceInfo,
            lang::XInitialization, lang::XComponent >
{
    OUString m_arg;
public:
    // focus on four interfaces,
    // no need to implement XInterface, XTypeProvider, XWeak etc.
    MSAAServiceImpl ();
    virtual ~MSAAServiceImpl( void );

    // XInitialization will be called upon createInstanceWithArguments[AndContext]()
    virtual void SAL_CALL initialize( Sequence< Any > const & args ) throw (Exception);

    // XComponent - as used by VCL to lifecycle manage this bridge.
    virtual void SAL_CALL dispose();
    virtual void SAL_CALL addEventListener( const ::css::uno::Reference< ::css::lang::XEventListener >& )    { /* dummy */ }
    virtual void SAL_CALL removeEventListener( const ::css::uno::Reference< ::css::lang::XEventListener >& ) { /* dummy */ }

    // XMSAAService
    virtual sal_Int64 SAL_CALL getAccObjectPtr(
            sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam);
    virtual void SAL_CALL handleWindowOpened(sal_Int64);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName();
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames();
};

void MSAAServiceImpl::initialize( Sequence< Any > const & args ) throw (Exception)
{
    if (1 != args.getLength())
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("give a string instanciating this component!") ),
            (::cppu::OWeakObject *)this, // resolve to XInterface reference
            0 ); // argument pos
    }
    if (! (args[ 0 ] >>= m_arg))
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no string given as argument!") ),
            (::cppu::OWeakObject *)this, // resolve to XInterface reference
            0 ); // argument pos
    }
}

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

    return GetMSComPtr( hWnd, lParam, wParam );
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
    handleWindowOpened_impl( nAcc );
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
    // this object only supports one service, so the test is simple
    return serviceName == "com.sun.star.accessibility.MSAAService";
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

        // Test for combo box - drop down floating windows first
        Window * pParentWindow = pWindow->GetParent();

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
            catch (::com::sun::star::uno::RuntimeException e)
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
            xAccMgr->handleWindowOpened( (long)xAccessible.get() );
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

static bool HasAtHook()
{
    sal_Int32 bIsRuning=0;
    // BOOL WINAPI SystemParametersInfo(
    //    __in     UINT uiAction,
    //    __in     UINT uiParam,
    //    __inout  PVOID pvParam,
    //    __in     UINT fWinIni
    //  );
    // pvParam must be BOOL (defined in MFC as int)
    // End
    return SystemParametersInfo( SPI_GETSCREENREADER, 0,
                                 &bIsRuning, 0) && bIsRuning;
}

/**
 * Static method that can create an entity of our MSAA Service
 * @param xContext No use here.
 * @return The object interface.
 */
Reference< XInterface > SAL_CALL create_MSAAServiceImpl( Reference< XComponentContext > const & /*xContext*/ ) SAL_THROW( () )
{
    bool bRunWithoutAt = getenv("SAL_FORCE_IACCESSIBLE2");

    if ( !HasAtHook() )
    {
        if ( !bRunWithoutAt )
        {
            SAL_INFO("iacc2", "Apparently no running AT -> not enabling IAccessible2 integration");
            return Reference< XMSAAService >();
        }
    }

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
        g_pTop = new AccTopWindowListener();
        Reference< XTopWindowListener> xRef( g_pTop );
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
    FreeTopWindowListener();
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
    SAL_DLLPUBLIC_EXPORT void SAL_CALL iacc2_component_getImplementationEnvironment(
        sal_Char const ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL iacc2_component_getFactory(
        sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
        registry::XRegistryKey * xRegistry )
    {
        return ::cppu::component_getFactoryHelper(
                   implName, xMgr, xRegistry, ::my_sc_impl::s_component_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
