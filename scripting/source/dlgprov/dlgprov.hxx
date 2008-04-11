/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgprov.hxx,v $
 * $Revision: 1.7 $
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

#ifndef SCRIPTING_DLGPROV_HXX
#define SCRIPTING_DLGPROV_HXX

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XDialogProvider2.hpp>
#include <com/sun/star/awt/XContainerWindowProvider.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/XScriptEventsAttacher.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTION_HPP_
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#include <osl/mutex.hxx>


//.........................................................................
namespace dlgprov
{
//.........................................................................

    // =============================================================================
    // mutex
    // =============================================================================

    ::osl::Mutex& getMutex();


    // =============================================================================
    // class DialogProviderImpl
    // =============================================================================

    typedef ::cppu::WeakImplHelper4<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::awt::XDialogProvider2,
        ::com::sun::star::awt::XContainerWindowProvider > DialogProviderImpl_BASE;


    class DialogProviderImpl : public DialogProviderImpl_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptEventsAttacher > m_xScriptEventsAttacher;


        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > createDialogModel( const ::rtl::OUString& sURL );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > createDialogControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxDialogModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent );

        void attachControlEvents( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxControlContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxHandler,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess >& rxIntrospectionAccess,
            bool bDialogProviderMode );
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > inspectHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxHandler );

        // XDialogProvider / XDialogProvider2 impl method
        virtual ::com::sun::star::uno::Reference < ::com::sun::star::awt::XControl > SAL_CALL createDialogImpl(
            const ::rtl::OUString& URL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xHandler,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
            bool bDialogProviderMode )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    public:
        DialogProviderImpl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~DialogProviderImpl();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XDialogProvider
        virtual ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDialog > SAL_CALL createDialog(
            const ::rtl::OUString& URL )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XDialogProvider2
        virtual ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDialog > SAL_CALL createDialogWithHandler(
            const ::rtl::OUString& URL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xHandler )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createContainerWindow(
            const ::rtl::OUString& URL, const ::rtl::OUString& WindowType,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xHandler )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
     };

//.........................................................................
}   // namespace dlgprov
//.........................................................................

#endif // SCRIPTING_DLGPROV_HXX
