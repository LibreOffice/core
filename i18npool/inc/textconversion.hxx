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
#ifndef _I18N_TEXTCONVERSION_KO_HXX_
#define _I18N_TEXTCONVERSION_KO_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedTextConversion.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <osl/module.h>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class TextConversion
//  ----------------------------------------------------
class TextConversion: public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedTextConversion,
    com::sun::star::lang::XServiceInfo
>
{
public:
        TextConversion();
        ~TextConversion();
        // Methods
        virtual com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;
        virtual rtl::OUString SAL_CALL
        getConversion( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException ) = 0;
        virtual rtl::OUString SAL_CALL
        getConversionWithOffset( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
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
    rtl::OUString SAL_CALL
        getImplementationName()
            throw( com::sun::star::uno::RuntimeException );
    sal_Bool SAL_CALL
        supportsService(const rtl::OUString& ServiceName)
            throw( com::sun::star::uno::RuntimeException );
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );
protected :
    const sal_Char* implementationName;
    oslModule hModule;
    oslGenericFunction SAL_CALL getFunctionBySymbol(const sal_Char* func);
};

// for Hangul2Hanja conversion
typedef struct {
    sal_Unicode code;
    sal_Int16 address;
    sal_Int16 count;
} Hangul_Index;

//  ----------------------------------------------------
//  class TextConversion_ko
//  ----------------------------------------------------
class TextConversion_ko : public TextConversion
{
public:
    TextConversion_ko( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );

        // Methods
        com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL
        getConversion( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL
        getConversionWithOffset( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
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
        com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
            getCharConversions(const rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toHanja);
};

//  ----------------------------------------------------
//  class TextConversion_zh
//  ----------------------------------------------------

// for SChines/TChinese word conversion
typedef struct {
    sal_uInt16 start;
    sal_Int16 count;
} STC_WordIndex;

class TextConversion_zh : public TextConversion
{
public:
    TextConversion_zh( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );

        // Methods
        com::sun::star::i18n::TextConversionResult SAL_CALL
        getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL
        getConversion( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType,
            sal_Int32 nTextConversionOptions )
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL
        getConversionWithOffset( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
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
        rtl::OUString SAL_CALL getWordConversion(const ::rtl::OUString& aText,
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
