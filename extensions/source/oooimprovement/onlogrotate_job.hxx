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


#ifndef EXTENSIONS_OOOIMPROVEMENT_ONLOGROTATEJOB_HXX
#define EXTENSIONS_OOOIMPROVEMENT_ONLOGROTATEJOB_HXX

#include <com/sun/star/task/XAsyncJob.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    class OnLogRotateJob : public ::cppu::WeakImplHelper2<
        ::com::sun::star::task::XAsyncJob,
        ::com::sun::star::lang::XServiceInfo>
    {
        public:
            // XServiceInfo - static version
            static ::rtl::OUString SAL_CALL getImplementationName_static();
            static ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames_static();
            static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL Create(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& context);

            static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL Create(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& sf);

        protected:
            OnLogRotateJob(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& context);
            OnLogRotateJob(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& sm);
            virtual ~OnLogRotateJob();

            // XAsyncJob
            virtual void SAL_CALL executeAsync(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue>& args,
                const ::com::sun::star::uno::Reference< ::com::sun::star::task::XJobListener>& listener)
                throw(::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName()
                throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& service_name)
                throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames()
                throw(::com::sun::star::uno::RuntimeException);

        private:
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_ServiceFactory;
    };
    #undef css
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
