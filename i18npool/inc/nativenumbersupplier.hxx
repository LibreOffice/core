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
#ifndef _I18N_NATIVENUMBERSUPPLIER_HXX_
#define _I18N_NATIVENUMBERSUPPLIER_HXX_

#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>
#include <com/sun/star/i18n/NativeNumberMode.hpp>
#include <com/sun/star/i18n/NativeNumberXmlAttributes.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

//      ----------------------------------------------------
//      class NativeNumberSupplier
//      ----------------------------------------------------
class NativeNumberSupplier : public cppu::WeakImplHelper2
<
        com::sun::star::i18n::XNativeNumberSupplier,
        com::sun::star::lang::XServiceInfo
>
{
public:
        NativeNumberSupplier(sal_Bool _useOffset = sal_False) : useOffset(_useOffset) {}

        // Methods
        virtual OUString SAL_CALL getNativeNumberString( const OUString& aNumberString,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL isValidNatNum( const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual ::com::sun::star::i18n::NativeNumberXmlAttributes SAL_CALL convertToXmlAttributes(
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Int16 SAL_CALL convertFromXmlAttributes(
                const ::com::sun::star::i18n::NativeNumberXmlAttributes& aAttr )
                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        //XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
                throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
                throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // following methods are not for XNativeNumberSupplier, they are for calling from transliterations
        OUString SAL_CALL getNativeNumberString( const OUString& aNumberString,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode,
                com::sun::star::uno::Sequence< sal_Int32 >& offset  )
                throw (::com::sun::star::uno::RuntimeException);
        sal_Unicode SAL_CALL getNativeNumberChar( const sal_Unicode inChar,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw(com::sun::star::uno::RuntimeException) ;

private:
        ::com::sun::star::lang::Locale aLocale;
        sal_Bool useOffset;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
