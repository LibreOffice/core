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

#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/Locale.hpp>


namespace com { namespace sun { namespace star { namespace i18n {

class OrdinalSuffix : public cppu::WeakImplHelper2
<
    XOrdinalSuffix,
    com::sun::star::lang::XServiceInfo
>
{
    public:
        OrdinalSuffix(const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF);
        virtual ~OrdinalSuffix();

        // XOrdinalSuffix
        virtual rtl::OUString SAL_CALL getOrdinalSuffix( sal_Int32 nNumber, const com::sun::star::lang::Locale &rLocale) throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw(com::sun::star::uno::RuntimeException);
        virtual com::sun::star::uno::Sequence < rtl::OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

    private:
        com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > _xServiceManager;
};
} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
