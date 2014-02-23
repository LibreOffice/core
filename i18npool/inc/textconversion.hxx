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
#ifndef _I18N_TEXTCONVERSION_KO_HXX_
#define _I18N_TEXTCONVERSION_KO_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedTextConversion.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <osl/module.h>

namespace com { namespace sun { namespace star { namespace i18n {


//  class TextConversion

class TextConversion: public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedTextConversion,
    com::sun::star::lang::XServiceInfo
>
{
public:
        TextConversion(const char* pImplName);
        ~TextConversion();
        // Methods
        virtual com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;
        virtual OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;
        virtual OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, com::sun::star::uno::Sequence< sal_Int32 >& offset )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;
        virtual sal_Bool SAL_CALL
        interactiveConversion(const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;

    //XServiceInfo
    OUString SAL_CALL
        getImplementationName()
            throw( com::sun::star::uno::RuntimeException );
    sal_Bool SAL_CALL
        supportsService(const OUString& ServiceName)
            throw( com::sun::star::uno::RuntimeException );
    com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );
private:
    const sal_Char* implementationName;
protected :
#ifndef DISABLE_DYNLOADING
    oslModule hModule;
    oslGenericFunction SAL_CALL getFunctionBySymbol(const sal_Char* func);
#endif
};

// for Hangul2Hanja conversion
typedef struct {
    sal_Unicode code;
    sal_Int16 address;
    sal_Int16 count;
} Hangul_Index;


//  class TextConversion_ko

class TextConversion_ko : public TextConversion
{
public:
    TextConversion_ko( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );

        // Methods
        com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, com::sun::star::uno::Sequence< sal_Int32 >& offset )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        sal_Bool SAL_CALL
        interactiveConversion(const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );

private :
        // Hangul/Hanja system dictionary
        com::sun::star::uno::Reference < com::sun::star::linguistic2::XConversionDictionary > xCD;
        // Hangul/Hanja user defined dictionary list
        com::sun::star::uno::Reference < com::sun::star::linguistic2::XConversionDictionaryList > xCDL;
        sal_Int32 maxLeftLength;
        sal_Int32 maxRightLength;
        com::sun::star::uno::Sequence< OUString > SAL_CALL
            getCharConversions(const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toHanja);
};


//  class TextConversion_zh


// for SChines/TChinese word conversion
typedef struct {
    sal_uInt16 start;
    sal_Int16 count;
} STC_WordIndex;

class TextConversion_zh : public TextConversion
{
public:
    TextConversion_zh( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );

        // Methods
        com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        OUString SAL_CALL
        getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        OUString SAL_CALL
        getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions, com::sun::star::uno::Sequence< sal_Int32 >& offset )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        sal_Bool SAL_CALL
        interactiveConversion(const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
private :
        // user defined dictionary list
        com::sun::star::uno::Reference < com::sun::star::linguistic2::XConversionDictionaryList > xCDL;
        OUString SAL_CALL getWordConversion(const OUString& aText,
            sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toSChinese, sal_Int32 nConversionOptions, com::sun::star::uno::Sequence <sal_Int32>& offset);
        rtl:: OUString SAL_CALL getCharConversion(const rtl:: OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toSChinese, sal_Int32 nConversionOptions);
        com::sun::star::lang::Locale aLocale;
};

} // i18n
} // star
} // sun
} // com

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
