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


#include <numberformatcode.hxx>
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <com/sun/star/i18n/LocaleData.hpp>
#include <comphelper/componentcontext.hxx>



NumberFormatCodeMapper::NumberFormatCodeMapper(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::uno::XComponentContext >& rxContext )
        :
        mxContext( rxContext ),
        bFormatsValid( false )
{
}


NumberFormatCodeMapper::~NumberFormatCodeMapper()
{
}


::com::sun::star::i18n::NumberFormatCode SAL_CALL
NumberFormatCodeMapper::getDefault( sal_Int16 formatType, sal_Int16 formatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{

    ::rtl::OUString elementType = mapElementTypeShortToString(formatType);
    ::rtl::OUString elementUsage = mapElementUsageShortToString(formatUsage);

    getFormats( rLocale );

    for(sal_Int32 i = 0; i < aFormatSeq.getLength(); i++) {
        if(aFormatSeq[i].isDefault && aFormatSeq[i].formatType == elementType &&
            aFormatSeq[i].formatUsage == elementUsage) {
            com::sun::star::i18n::NumberFormatCode anumberFormatCode(formatType,
                                                                    formatUsage,
                                                                    aFormatSeq[i].formatCode,
                                                                    aFormatSeq[i].formatName,
                                                                    aFormatSeq[i].formatKey,
                                                                    aFormatSeq[i].formatIndex,
                                                                    sal_True);
            return anumberFormatCode;
        }
    }
    com::sun::star::i18n::NumberFormatCode defaultNumberFormatCode;
    return defaultNumberFormatCode;
}



::com::sun::star::i18n::NumberFormatCode SAL_CALL
NumberFormatCodeMapper::getFormatCode( sal_Int16 formatIndex, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    getFormats( rLocale );

    for(sal_Int32 i = 0; i < aFormatSeq.getLength(); i++) {
        if(aFormatSeq[i].formatIndex == formatIndex) {
            com::sun::star::i18n::NumberFormatCode anumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                                    mapElementUsageStringToShort(aFormatSeq[i].formatUsage),
                                                                    aFormatSeq[i].formatCode,
                                                                    aFormatSeq[i].formatName,
                                                                    aFormatSeq[i].formatKey,
                                                                    aFormatSeq[i].formatIndex,
                                                                    aFormatSeq[i].isDefault);
            return anumberFormatCode;
        }
    }
    com::sun::star::i18n::NumberFormatCode defaultNumberFormatCode;
    return defaultNumberFormatCode;

}



::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL
NumberFormatCodeMapper::getAllFormatCode( sal_Int16 formatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    getFormats( rLocale );

    sal_Int32 i, count;
    count = 0;
    for(i = 0; i < aFormatSeq.getLength(); i++) {
        sal_Int16 elementUsage = mapElementUsageStringToShort(aFormatSeq[i].formatUsage);
        if( elementUsage == formatUsage)
            count++;
    }

    ::com::sun::star::uno::Sequence<com::sun::star::i18n::NumberFormatCode> seq(count);
    sal_Int32 j = 0;
    for(i = 0; i < aFormatSeq.getLength(); i++) {
        sal_Int16 elementUsage = mapElementUsageStringToShort(aFormatSeq[i].formatUsage);
        if( elementUsage == formatUsage) {
            seq[j] = com::sun::star::i18n::NumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                            formatUsage,
                                                            aFormatSeq[i].formatCode,
                                                            aFormatSeq[i].formatName,
                                                            aFormatSeq[i].formatKey,
                                                            aFormatSeq[i].formatIndex,
                                                            aFormatSeq[i].isDefault);
            j++;
        }
    }
    return seq;

}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL
NumberFormatCodeMapper::getAllFormatCodes( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException)
{
    getFormats( rLocale );

    ::com::sun::star::uno::Sequence<com::sun::star::i18n::NumberFormatCode> seq(aFormatSeq.getLength());
    for(sal_Int32 i = 0; i < aFormatSeq.getLength(); i++)
    {
        seq[i] = com::sun::star::i18n::NumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                        mapElementUsageStringToShort(aFormatSeq[i].formatUsage),
                                                        aFormatSeq[i].formatCode,
                                                        aFormatSeq[i].formatName,
                                                        aFormatSeq[i].formatKey,
                                                        aFormatSeq[i].formatIndex,
                                                        aFormatSeq[i].isDefault);
    }
    return seq;
}


// --- private implementation -----------------------------------------

void NumberFormatCodeMapper::setupLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    if ( aLocale.Country    != rLocale.Country
      || aLocale.Language   != rLocale.Language
      || aLocale.Variant    != rLocale.Variant )
    {
        bFormatsValid = false;
        aLocale = rLocale;
    }
}


void NumberFormatCodeMapper::getFormats( const ::com::sun::star::lang::Locale& rLocale )
{
    setupLocale( rLocale );
    if ( !bFormatsValid )
    {
        createLocaleDataObject();
        if( !mxLocaleData.is() )
            aFormatSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > (0);
        else
            aFormatSeq = mxLocaleData->getAllFormats( aLocale );
        bFormatsValid = true;
    }
}


::rtl::OUString
NumberFormatCodeMapper::mapElementTypeShortToString(sal_Int16 formatType)
{

    switch ( formatType )
    {
        case com::sun::star::i18n::KNumberFormatType::SHORT :
            return ::rtl::OUString(  "short"  );
        case com::sun::star::i18n::KNumberFormatType::MEDIUM :
            return ::rtl::OUString(  "medium"  );
        case com::sun::star::i18n::KNumberFormatType::LONG :
            return ::rtl::OUString(  "long"  );
    }
    return ::rtl::OUString();
}

sal_Int16
NumberFormatCodeMapper::mapElementTypeStringToShort(const ::rtl::OUString& formatType)
{
    if ( formatType == "short" )
        return com::sun::star::i18n::KNumberFormatType::SHORT;
    if ( formatType == "medium" )
        return com::sun::star::i18n::KNumberFormatType::MEDIUM;
    if ( formatType == "long" )
        return com::sun::star::i18n::KNumberFormatType::LONG;

    return com::sun::star::i18n::KNumberFormatType::SHORT;
}

::rtl::OUString
NumberFormatCodeMapper::mapElementUsageShortToString(sal_Int16 formatUsage)
{
    switch ( formatUsage )
    {
        case com::sun::star::i18n::KNumberFormatUsage::DATE :
            return ::rtl::OUString(  "DATE"  );
        case com::sun::star::i18n::KNumberFormatUsage::TIME :
            return ::rtl::OUString(  "TIME"  );
        case com::sun::star::i18n::KNumberFormatUsage::DATE_TIME :
            return ::rtl::OUString(  "DATE_TIME"  );
        case com::sun::star::i18n::KNumberFormatUsage::FIXED_NUMBER :
            return ::rtl::OUString(  "FIXED_NUMBER"  );
        case com::sun::star::i18n::KNumberFormatUsage::FRACTION_NUMBER :
            return ::rtl::OUString(  "FRACTION_NUMBER"  );
        case com::sun::star::i18n::KNumberFormatUsage::PERCENT_NUMBER :
            return ::rtl::OUString(  "PERCENT_NUMBER"  );
        case com::sun::star::i18n::KNumberFormatUsage::CURRENCY :
            return ::rtl::OUString(  "CURRENCY"  );
        case com::sun::star::i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER :
            return ::rtl::OUString(  "SCIENTIFIC_NUMBER"  );
    }
    return ::rtl::OUString();
}


sal_Int16
NumberFormatCodeMapper::mapElementUsageStringToShort(const ::rtl::OUString& formatUsage)
{
    if ( formatUsage == "DATE" )
        return com::sun::star::i18n::KNumberFormatUsage::DATE;
    if ( formatUsage == "TIME" )
        return com::sun::star::i18n::KNumberFormatUsage::TIME;
    if ( formatUsage == "DATE_TIME" )
        return com::sun::star::i18n::KNumberFormatUsage::DATE_TIME;
    if ( formatUsage == "FIXED_NUMBER" )
        return com::sun::star::i18n::KNumberFormatUsage::FIXED_NUMBER;
    if ( formatUsage == "FRACTION_NUMBER" )
        return com::sun::star::i18n::KNumberFormatUsage::FRACTION_NUMBER;
    if ( formatUsage == "PERCENT_NUMBER" )
        return  com::sun::star::i18n::KNumberFormatUsage::PERCENT_NUMBER;
    if ( formatUsage == "CURRENCY" )
        return com::sun::star::i18n::KNumberFormatUsage::CURRENCY;
    if ( formatUsage == "SCIENTIFIC_NUMBER" )
        return com::sun::star::i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER;

    return 0;
}


void
NumberFormatCodeMapper::createLocaleDataObject() {

    if(mxLocaleData.is())
        return;

    mxLocaleData.set( com::sun::star::i18n::LocaleData::create(mxContext) );
}

::rtl::OUString SAL_CALL
NumberFormatCodeMapper::getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString("com.sun.star.i18n.NumberFormatCodeMapper");
}

const sal_Char cNumFormat[] = "com.sun.star.i18n.NumberFormatMapper";

sal_Bool SAL_CALL
NumberFormatCodeMapper::supportsService(const rtl::OUString& rServiceName)
                throw( ::com::sun::star::uno::RuntimeException )
{
    return !rServiceName.compareToAscii(cNumFormat);
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
NumberFormatCodeMapper::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString::createFromAscii(cNumFormat);
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
