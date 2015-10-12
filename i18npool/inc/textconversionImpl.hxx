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
    com::sun::star::i18n::XExtendedTextConversion,
    com::sun::star::lang::XServiceInfo
>
{
public:
    TextConversionImpl( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext ) : m_xContext(rxContext) {};

        // Methods
        com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException, std::exception ) override;
        OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException, std::exception ) override;
        OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, com::sun::star::uno::Sequence< sal_Int32 >& offset )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException, std::exception ) override;
        sal_Bool SAL_CALL
        interactiveConversion( const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException, std::exception ) override;

    //XServiceInfo
    OUString SAL_CALL
        getImplementationName()
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    sal_Bool SAL_CALL
        supportsService(const OUString& ServiceName)
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;
private:
    com::sun::star::lang::Locale aLocale;
    com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedTextConversion > xTC;
    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;

    void SAL_CALL getLocaleSpecificTextConversion( const com::sun::star::lang::Locale& rLocale )
            throw( com::sun::star::lang::NoSupportException );
};

} // i18n
} // star
} // sun
} // com


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
