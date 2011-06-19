/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef UUI_INTERACTIONHANDLER_HXX
#define UUI_INTERACTIONHANDLER_HXX

#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XInteractionHandler2.hpp"
#include "cppuhelper/implbase3.hxx"

class UUIInteractionHelper;

class UUIInteractionHandler:
    public cppu::WeakImplHelper3< com::sun::star::lang::XServiceInfo,
                                  com::sun::star::lang::XInitialization,
                                  com::sun::star::task::XInteractionHandler2 >
{
public:
    static char const m_aImplementationName[];

    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL
    createInstance(
        com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory > const &
            rServiceFactory)
        SAL_THROW((com::sun::star::uno::Exception));

private:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xServiceFactory;
    UUIInteractionHelper * m_pImpl;

    UUIInteractionHandler(UUIInteractionHandler &); // not implemented
    void operator =(UUIInteractionHandler); // not implemented

    UUIInteractionHandler(com::sun::star::uno::Reference<
                  com::sun::star::lang::XMultiServiceFactory >
                  const & rServiceFactory)
        SAL_THROW(());

    virtual ~UUIInteractionHandler() SAL_THROW(());

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &
                          rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    initialize(
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            rArguments)
        throw (com::sun::star::uno::Exception);

    virtual void SAL_CALL
    handle(com::sun::star::uno::Reference<
           com::sun::star::task::XInteractionRequest > const &
       rRequest)
        throw (com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL
        handleInteractionRequest(
            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& _Request
        )   throw ( ::com::sun::star::uno::RuntimeException );
};

#endif // UUI_INTERACTIONHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
