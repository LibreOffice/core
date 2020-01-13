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
#ifndef INCLUDED_I18NPOOL_INC_TEXTCONVERSION_HXX
#define INCLUDED_I18NPOOL_INC_TEXTCONVERSION_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedTextConversion.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/module.h>

namespace com::sun::star::linguistic2 { class XConversionDictionary; }
namespace com::sun::star::linguistic2 { class XConversionDictionaryList; }
namespace com::sun::star::uno { class XComponentContext; }

namespace i18npool {



class TextConversionService: public cppu::WeakImplHelper
<
    css::i18n::XExtendedTextConversion,
    css::lang::XServiceInfo
>
{
public:
        TextConversionService(const char* pImplName);
        virtual ~TextConversionService() override;
        // Methods
        virtual css::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override = 0;
        virtual OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override = 0;
        virtual OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, css::uno::Sequence< sal_Int32 >& offset ) override = 0;
        virtual sal_Bool SAL_CALL
        interactiveConversion(const css::lang::Locale& aLocale,
            sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions ) override = 0;

    //XServiceInfo
    OUString SAL_CALL
        getImplementationName() override;
    sal_Bool SAL_CALL
        supportsService(const OUString& ServiceName) override;
    css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;
private:
    const char* implementationName;
protected:
#ifndef DISABLE_DYNLOADING
    oslModule hModule;
    oslGenericFunction getFunctionBySymbol(const char* func);
#endif
};

// for Hangul2Hanja conversion
typedef struct {
    sal_Unicode code;
    sal_Int16 address;
    sal_Int16 count;
} Hangul_Index;



class TextConversion_ko final : public TextConversionService
{
public:
    TextConversion_ko( const css::uno::Reference < css::uno::XComponentContext >& rxContext );

        // Methods
        css::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override;
        OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override;
        OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, css::uno::Sequence< sal_Int32 >& offset ) override;
        sal_Bool SAL_CALL
        interactiveConversion(const css::lang::Locale& aLocale,
            sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override;

private:
        // Hangul/Hanja system dictionary
        css::uno::Reference < css::linguistic2::XConversionDictionary > xCD;
        // Hangul/Hanja user defined dictionary list
        css::uno::Reference < css::linguistic2::XConversionDictionaryList > xCDL;
        sal_Int32 maxLeftLength;
        sal_Int32 maxRightLength;
        css::uno::Sequence< OUString >
            getCharConversions(const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, bool toHanja);
};




// for SChines/TChinese word conversion
typedef struct {
    sal_Int16 count;
} STC_WordIndex;

class TextConversion_zh final : public TextConversionService
{
public:
    TextConversion_zh( const css::uno::Reference < css::uno::XComponentContext >& rxContext );

        // Methods
        css::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override;
        OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override;
        OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const css::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, css::uno::Sequence< sal_Int32 >& offset ) override;
        sal_Bool SAL_CALL
        interactiveConversion(const css::lang::Locale& aLocale,
            sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions ) override;
private:
        // user defined dictionary list
        css::uno::Reference < css::linguistic2::XConversionDictionaryList > xCDL;
        OUString getWordConversion(const OUString& aText,
            sal_Int32 nStartPos, sal_Int32 nLength, bool toSChinese, sal_Int32 nConversionOptions, css::uno::Sequence <sal_Int32>& offset);
        rtl:: OUString getCharConversion(const rtl:: OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, bool toSChinese, sal_Int32 nConversionOptions);
        css::lang::Locale aLocale;
};

} // i18npool

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
