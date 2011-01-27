/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#ifndef EXTENSIONS_OOOIMPROVEMENT_CORECONTROLLER_HXX
#define EXTENSIONS_OOOIMPROVEMENT_CORECONTROLLER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/oooimprovement/XCoreController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase2.hxx>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    class CoreController : public ::cppu::WeakImplHelper2<
        css::oooimprovement::XCoreController,
        css::lang::XServiceInfo>
    {
        public:
            // css::lang::XServiceInfo - static version
            static ::rtl::OUString SAL_CALL getImplementationName_static();
            static css::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames_static();
            //static css::uno::Reference< css::uno::XInterface> SAL_CALL Create(
            //    const css::uno::Reference<css::uno::XComponentContext>& context);

            static css::uno::Reference< css::uno::XInterface> SAL_CALL Create(
                const css::uno::Reference< css::lang::XMultiServiceFactory>& sm);

        protected:
            CoreController(const css::uno::Reference< css::uno::XComponentContext>& context);
            CoreController(const css::uno::Reference< css::lang::XMultiServiceFactory>& sm);
            virtual ~CoreController();

            // css::oooimprovement::XCoreController
            virtual sal_Bool SAL_CALL enablingUiEventsLoggerAllowed(sal_Int16 version)
                throw(css::uno::RuntimeException);
            virtual sal_Bool SAL_CALL showBuiltinOptionsPage(sal_Int16 version)
                throw(css::uno::RuntimeException);

            // css::lang::XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName()
                throw(css::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString& service_name)
                throw(css::uno::RuntimeException);
            virtual css::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames()
                throw(css::uno::RuntimeException);

        private:
            css::uno::Reference< css::lang::XMultiServiceFactory> m_ServiceFactory;
    };
    #undef css
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
