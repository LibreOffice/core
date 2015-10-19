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
#ifndef INCLUDED_I18NPOOL_INC_TEXTCONVERSIONIMPL_HXX
#define INCLUDED_I18NPOOL_INC_TEXTCONVERSIONIMPL_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedTextConversion.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

namespace com { namespace sun { namespace star { namespace i18n {


//  class TextConversion

class TextConversionImpl : public cppu::WeakImplHelper
<
    css::i18n::XExtendedTextConversion,
    css::lang::XServiceInfo
>
{
public:
    TextConversionImpl( const css::uno::Reference < css::uno::XComponentContext >& rxContext ) : m_xContext(rxContext) {};

        // Methods
        css::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  css::uno::RuntimeException,
                    css::lang::IllegalArgumentException,
                    css::lang::NoSupportException, std::exception ) override;
        OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  css::uno::RuntimeException,
                    css::lang::IllegalArgumentException,
                    css::lang::NoSupportException, std::exception ) override;
        OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, css::uno::Sequence< sal_Int32 >& offset )
            throw(  css::uno::RuntimeException,
                    css::lang::IllegalArgumentException,
                    css::lang::NoSupportException, std::exception ) override;
        sal_Bool SAL_CALL
        interactiveConversion( const css::lang::Locale& aLocale,
            sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
            throw(  css::uno::RuntimeException,
                    css::lang::IllegalArgumentException,
                    css::lang::NoSupportException, std::exception ) override;

    //XServiceInfo
    OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    sal_Bool SAL_CALL
        supportsService(const OUString& ServiceName)
            throw( css::uno::RuntimeException, std::exception ) override;
    css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;
private:
    css::lang::Locale aLocale;
    css::uno::Reference < css::i18n::XExtendedTextConversion > xTC;
    css::uno::Reference < css::uno::XComponentContext > m_xContext;

    void SAL_CALL getLocaleSpecificTextConversion( const css::lang::Locale& rLocale )
            throw( css::lang::NoSupportException );
};

} // i18n
} // star
} // sun
} // com


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
