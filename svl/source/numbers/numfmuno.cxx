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


#include <tools/color.hxx>
#include <i18npool/mslangid.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "numfmuno.hxx"
#include <svl/numuno.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/itemprop.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------------

#define SERVICENAME_NUMBERFORMATTER "com.sun.star.util.NumberFormatter"
#define SERVICENAME_NUMBERSETTINGS  "com.sun.star.util.NumberFormatSettings"
#define SERVICENAME_NUMBERFORMATS   "com.sun.star.util.NumberFormats"
#define SERVICENAME_NUMBERFORMAT    "com.sun.star.util.NumberFormatProperties"

//------------------------------------------------------------------------

#define PROPERTYNAME_FMTSTR     "FormatString"
#define PROPERTYNAME_LOCALE     "Locale"
#define PROPERTYNAME_TYPE       "Type"
#define PROPERTYNAME_COMMENT    "Comment"
#define PROPERTYNAME_CURREXT    "CurrencyExtension"
#define PROPERTYNAME_CURRSYM    "CurrencySymbol"
#define PROPERTYNAME_CURRABB    "CurrencyAbbreviation"
#define PROPERTYNAME_DECIMALS   "Decimals"
#define PROPERTYNAME_LEADING    "LeadingZeros"
#define PROPERTYNAME_NEGRED     "NegativeRed"
#define PROPERTYNAME_STDFORM    "StandardFormat"
#define PROPERTYNAME_THOUS      "ThousandsSeparator"
#define PROPERTYNAME_USERDEF    "UserDefined"

#define PROPERTYNAME_NOZERO     "NoZero"
#define PROPERTYNAME_NULLDATE   "NullDate"
#define PROPERTYNAME_STDDEC     "StandardDecimals"
#define PROPERTYNAME_TWODIGIT   "TwoDigitDateStart"

//------------------------------------------------------------------------

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetNumberFormatPropertyMap()
{
    static SfxItemPropertyMapEntry aNumberFormatPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(PROPERTYNAME_FMTSTR),   0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_LOCALE),   0, &getCppuType((lang::Locale*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_TYPE),     0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_COMMENT),  0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_CURREXT),  0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_CURRSYM),  0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_DECIMALS), 0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_LEADING),  0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_NEGRED),   0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_STDFORM),  0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_THOUS),    0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_USERDEF),  0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_CURRABB),  0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0,0}
    };
    return aNumberFormatPropertyMap_Impl;
}

static const SfxItemPropertyMapEntry* lcl_GetNumberSettingsPropertyMap()
{
    static SfxItemPropertyMapEntry aNumberSettingsPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(PROPERTYNAME_NOZERO),   0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_NULLDATE), 0, &getCppuType((util::Date*)0),  beans::PropertyAttribute::BOUND, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_STDDEC),   0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND, 0},
        {MAP_CHAR_LEN(PROPERTYNAME_TWODIGIT), 0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND, 0},
        {0,0,0,0,0,0}
    };
    return aNumberSettingsPropertyMap_Impl;
}

//----------------------------------------------------------------------------------------

static LanguageType lcl_GetLanguage( const lang::Locale& rLocale )
{
    //  empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.isEmpty() )
        return LANGUAGE_SYSTEM;

    LanguageType eRet = MsLangId::convertLocaleToLanguage( rLocale );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM;         //! or throw an exception?

    return eRet;
}

//----------------------------------------------------------------------------------------

SvNumberFormatterServiceObj::SvNumberFormatterServiceObj()
    :m_aMutex()
{
}

SvNumberFormatterServiceObj::~SvNumberFormatterServiceObj()
{
}

com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvNumberFormatterServiceObj_CreateInstance( SAL_UNUSED_PARAMETER const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& )
{
    return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new SvNumberFormatterServiceObj );
}

// XNumberFormatter

void SAL_CALL SvNumberFormatterServiceObj::attachNumberFormatsSupplier(
                            const uno::Reference<util::XNumberFormatsSupplier>& _xSupplier )
                                    throw(uno::RuntimeException)
{
    ::rtl::Reference< SvNumberFormatsSupplierObj > xAutoReleaseOld;

    // SYNCHRONIZED ->
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

        SvNumberFormatsSupplierObj* pNew = SvNumberFormatsSupplierObj::getImplementation( _xSupplier );
        if (!pNew)
            throw uno::RuntimeException();      // wrong object

        xAutoReleaseOld = xSupplier;

        xSupplier = pNew;
        m_aMutex = xSupplier->getSharedMutex();
    }
    // <- SYNCHRONIZED
}

uno::Reference<util::XNumberFormatsSupplier> SAL_CALL
                            SvNumberFormatterServiceObj::getNumberFormatsSupplier()
                                    throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return xSupplier.get();
}

sal_Int32 SAL_CALL SvNumberFormatterServiceObj::detectNumberFormat(
                                    sal_Int32 nKey, const rtl::OUString& aString )
                            throw(util::NotNumericException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aTemp = aString;
        sal_uInt32 nUKey = nKey;
        double fValue = 0.0;
        if ( pFormatter->IsNumberFormat(aTemp, nUKey, fValue) )
            nRet = nUKey;
        else
            throw util::NotNumericException();
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

double SAL_CALL SvNumberFormatterServiceObj::convertStringToNumber(
                                    sal_Int32 nKey, const rtl::OUString& aString )
                            throw(util::NotNumericException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    double fRet = 0.0;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aTemp = aString;
        sal_uInt32 nUKey = nKey;
        double fValue = 0.0;
        if ( pFormatter->IsNumberFormat(aTemp, nUKey, fValue) )
            fRet = fValue;
        else
            throw util::NotNumericException();
    }
    else
        throw uno::RuntimeException();

    return fRet;
}

rtl::OUString SAL_CALL SvNumberFormatterServiceObj::convertNumberToString(
                                    sal_Int32 nKey, double fValue ) throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    String aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        Color* pColor = NULL;
        pFormatter->GetOutputString(fValue, nKey, aRet, &pColor);
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

util::Color SAL_CALL SvNumberFormatterServiceObj::queryColorForNumber( sal_Int32 nKey,
                                    double fValue, util::Color aDefaultColor )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor;       // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aStr;
        Color* pColor = NULL;
        pFormatter->GetOutputString(fValue, nKey, aStr, &pColor);
        if (pColor)
            nRet = pColor->GetColor();
        // sonst Default behalten
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

rtl::OUString SAL_CALL SvNumberFormatterServiceObj::formatString( sal_Int32 nKey,
                                    const rtl::OUString& aString ) throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    String aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aTemp = aString;
        Color* pColor = NULL;
        pFormatter->GetOutputString(aTemp, nKey, aRet, &pColor);
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

util::Color SAL_CALL SvNumberFormatterServiceObj::queryColorForString( sal_Int32 nKey,
                                    const rtl::OUString& aString,util::Color aDefaultColor )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor;       // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aTemp = aString;
        String aStr;
        Color* pColor = NULL;
        pFormatter->GetOutputString(aTemp, nKey, aStr, &pColor);
        if (pColor)
            nRet = pColor->GetColor();
        // sonst Default behalten
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

rtl::OUString SAL_CALL SvNumberFormatterServiceObj::getInputString( sal_Int32 nKey, double fValue )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    String aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
        pFormatter->GetInputLineString(fValue, nKey, aRet);
    else
        throw uno::RuntimeException();

    return aRet;
}

// XNumberFormatPreviewer

rtl::OUString SAL_CALL SvNumberFormatterServiceObj::convertNumberToPreviewString(
                                    const rtl::OUString& aFormat, double fValue,
                                    const lang::Locale& nLocale, sal_Bool bAllowEnglish )
                            throw(util::MalformedNumberFormatException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    String aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aOutString;
        String aFormString = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        Color* pColor = NULL;

        bool bOk;
        if ( bAllowEnglish )
            bOk = pFormatter->GetPreviewStringGuess(
                                aFormString, fValue, aOutString, &pColor, eLang );
        else
            bOk = pFormatter->GetPreviewString(
                                aFormString, fValue, aOutString, &pColor, eLang );

        if (bOk)
            aRet = aOutString;
        else
            throw util::MalformedNumberFormatException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

util::Color SAL_CALL SvNumberFormatterServiceObj::queryPreviewColorForNumber(
                                    const rtl::OUString& aFormat, double fValue,
                                    const lang::Locale& nLocale, sal_Bool bAllowEnglish,
                                    util::Color aDefaultColor )
                            throw(util::MalformedNumberFormatException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor;       // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aOutString;
        String aFormString = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        Color* pColor = NULL;

        bool bOk;
        if ( bAllowEnglish )
            bOk = pFormatter->GetPreviewStringGuess(
                                aFormString, fValue, aOutString, &pColor, eLang );
        else
            bOk = pFormatter->GetPreviewString(
                                aFormString, fValue, aOutString, &pColor, eLang );

        if (bOk)
        {
            if (pColor)
                nRet = pColor->GetColor();
            // sonst Default behalten
        }
        else
            throw util::MalformedNumberFormatException();
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

// XServiceInfo

rtl::OUString SAL_CALL SvNumberFormatterServiceObj::getImplementationName()
                            throw(uno::RuntimeException)
{
    return rtl::OUString("com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject");
}

sal_Bool SAL_CALL SvNumberFormatterServiceObj::supportsService( const rtl::OUString& ServiceName )
                            throw(uno::RuntimeException)
{
    return ( ServiceName.compareToAscii(SERVICENAME_NUMBERFORMATTER) == 0 );
}

uno::Sequence<rtl::OUString> SAL_CALL SvNumberFormatterServiceObj::getSupportedServiceNames()
                            throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(SERVICENAME_NUMBERFORMATTER);
    return aRet;
}

//------------------------------------------------------------------------

SvNumberFormatsObj::SvNumberFormatsObj( SvNumberFormatsSupplierObj& _rParent, ::comphelper::SharedMutex& _rMutex )
    :rSupplier( _rParent )
    ,m_aMutex( _rMutex )
{
    rSupplier.acquire();
}

SvNumberFormatsObj::~SvNumberFormatsObj()
{
    rSupplier.release();
}

// XNumberFormats

uno::Reference<beans::XPropertySet> SAL_CALL SvNumberFormatsObj::getByKey( sal_Int32 nKey )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : NULL;
    if (pFormat)
        return new SvNumberFormatObj( rSupplier, nKey, m_aMutex );
    else
        throw uno::RuntimeException();
}

uno::Sequence<sal_Int32> SAL_CALL SvNumberFormatsObj::queryKeys( sal_Int16 nType,
                                    const lang::Locale& nLocale, sal_Bool bCreate )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if ( pFormatter )
    {
        sal_uInt32 nIndex = 0;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        SvNumberFormatTable& rTable = bCreate ?
                                        pFormatter->ChangeCL( nType, nIndex, eLang ) :
                                        pFormatter->GetEntryTable( nType, nIndex, eLang );
        sal_uInt32 nCount = rTable.size();
        uno::Sequence<sal_Int32> aSeq(nCount);
        sal_Int32* pAry = aSeq.getArray();
        sal_uInt32 i=0;
        for (SvNumberFormatTable::iterator it = rTable.begin(); it != rTable.end(); ++it, ++i)
            pAry[i] = it->first;

        return aSeq;
    }
    else
        throw uno::RuntimeException();
}

sal_Int32 SAL_CALL SvNumberFormatsObj::queryKey( const rtl::OUString& aFormat,
                                    const lang::Locale& nLocale, sal_Bool bScan )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        String aFormStr = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        if (bScan)
        {
            //! irgendwas muss hier noch passieren...
        }
        nRet = pFormatter->GetEntryKey( aFormat, eLang );
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::addNew( const rtl::OUString& aFormat,
                                    const lang::Locale& nLocale )
                            throw(util::MalformedNumberFormatException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        String aFormStr = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        sal_uInt32 nKey = 0;
        xub_StrLen nCheckPos = 0;
        short nType = 0;
        bool bOk = pFormatter->PutEntry( aFormStr, nCheckPos, nType, nKey, eLang );
        if (bOk)
            nRet = nKey;
        else if (nCheckPos)
        {
            throw util::MalformedNumberFormatException();       // ungueltiges Format
        }
        else
            throw uno::RuntimeException();                      // anderer Fehler (z.B. schon vorhanden)
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::addNewConverted( const rtl::OUString& aFormat,
                                    const lang::Locale& nLocale, const lang::Locale& nNewLocale )
                            throw(util::MalformedNumberFormatException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        String aFormStr = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        LanguageType eNewLang = lcl_GetLanguage( nNewLocale );
        sal_uInt32 nKey = 0;
        xub_StrLen nCheckPos = 0;
        short nType = 0;
        bool bOk = pFormatter->PutandConvertEntry( aFormStr, nCheckPos, nType, nKey, eLang, eNewLang );
        if (bOk || nKey > 0)
            nRet = nKey;
        else if (nCheckPos)
        {
            throw util::MalformedNumberFormatException();       // ungueltiges Format
        }
        else
            throw uno::RuntimeException();                      // anderer Fehler (z.B. schon vorhanden)
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

void SAL_CALL SvNumberFormatsObj::removeByKey( sal_Int32 nKey ) throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();

    if (pFormatter)
    {
        pFormatter->DeleteEntry(nKey);
        rSupplier.NumberFormatDeleted(nKey);        // Benachrichtigung fuers Dokument
    }
}

OUString SAL_CALL SvNumberFormatsObj::generateFormat( sal_Int32 nBaseKey,
                                    const lang::Locale& nLocale, sal_Bool bThousands,
                                    sal_Bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        aRet = pFormatter->GenerateFormat(nBaseKey, eLang, bThousands, bRed, nDecimals, nLeading);
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

// XNumberFormatTypes

sal_Int32 SAL_CALL SvNumberFormatsObj::getStandardIndex( const lang::Locale& nLocale )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        nRet = pFormatter->GetStandardIndex(eLang);
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getStandardFormat( sal_Int16 nType, const lang::Locale& nLocale )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        //  mask out "defined" bit, so type from an existing number format
        //  can directly be used for getStandardFormat
        nType &= ~NUMBERFORMAT_DEFINED;
        nRet = pFormatter->GetStandardFormat(nType, eLang);
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getFormatIndex( sal_Int16 nIndex, const lang::Locale& nLocale )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        nRet = pFormatter->GetFormatIndex( (NfIndexTableOffset)nIndex, eLang );
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Bool SAL_CALL SvNumberFormatsObj::isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Bool bRet = sal_False;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
        bRet = pFormatter->IsCompatible( nOldType, nNewType );
    else
        throw uno::RuntimeException();

    return bRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getFormatForLocale( sal_Int32 nKey, const lang::Locale& nLocale )
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        nRet = pFormatter->GetFormatForLanguageIfBuiltIn(nKey, eLang);
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

// XServiceInfo

rtl::OUString SAL_CALL SvNumberFormatsObj::getImplementationName()
                            throw(uno::RuntimeException)
{
    return rtl::OUString("SvNumberFormatsObj");
}

sal_Bool SAL_CALL SvNumberFormatsObj::supportsService( const rtl::OUString& ServiceName )
                            throw(uno::RuntimeException)
{
    return ( ServiceName.compareToAscii(SERVICENAME_NUMBERFORMATS) == 0 );
}

uno::Sequence<rtl::OUString> SAL_CALL SvNumberFormatsObj::getSupportedServiceNames()
                            throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(SERVICENAME_NUMBERFORMATS);
    return aRet;
}

//------------------------------------------------------------------------

SvNumberFormatObj::SvNumberFormatObj( SvNumberFormatsSupplierObj& rParent, sal_uLong nK, const ::comphelper::SharedMutex& _rMutex )
    :rSupplier( rParent )
    ,nKey( nK )
    ,m_aMutex( _rMutex )
{
    rSupplier.acquire();
}

SvNumberFormatObj::~SvNumberFormatObj()
{
    rSupplier.release();
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL SvNumberFormatObj::getPropertySetInfo()
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetNumberFormatPropertyMap() );
    return aRef;
}

void SAL_CALL SvNumberFormatObj::setPropertyValue( const rtl::OUString&,
                                                   const uno::Any& )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    throw beans::UnknownPropertyException();    //  everything is read-only
}

uno::Any SAL_CALL SvNumberFormatObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Any aRet;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : NULL;
    if (pFormat)
    {
        bool bThousand, bRed;
        sal_uInt16 nDecimals, nLeading;

        String aString = aPropertyName;
        if (aString.EqualsAscii( PROPERTYNAME_FMTSTR ))
        {
            aRet <<= rtl::OUString( pFormat->GetFormatstring() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_LOCALE ))
        {
            lang::Locale aLocale( MsLangId::convertLanguageToLocale(
                            pFormat->GetLanguage()));
            aRet <<= aLocale;
        }
        else if (aString.EqualsAscii( PROPERTYNAME_TYPE ))
        {
            aRet <<= (sal_Int16)( pFormat->GetType() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_COMMENT ))
        {
            aRet <<= rtl::OUString( pFormat->GetComment() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_STDFORM ))
        {
            //! SvNumberformat Member bStandard rausreichen?
            sal_Bool bStandard = ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
            aRet.setValue( &bStandard, getBooleanCppuType() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_USERDEF ))
        {
            sal_Bool bUserDef = ( ( pFormat->GetType() & NUMBERFORMAT_DEFINED ) != 0 );
            aRet.setValue( &bUserDef, getBooleanCppuType() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_DECIMALS ))
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet <<= (sal_Int16)( nDecimals );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_LEADING ))
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet <<= (sal_Int16)( nLeading );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_NEGRED ))
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet.setValue( &bRed, getBooleanCppuType() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_THOUS ))
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet.setValue( &bThousand, getBooleanCppuType() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_CURRSYM ))
        {
            OUString aSymbol, aExt;
            pFormat->GetNewCurrencySymbol( aSymbol, aExt );
            aRet <<= aSymbol;
        }
        else if (aString.EqualsAscii( PROPERTYNAME_CURREXT ))
        {
            OUString aSymbol, aExt;
            pFormat->GetNewCurrencySymbol( aSymbol, aExt );
            aRet <<= aExt;
        }
        else if (aString.EqualsAscii( PROPERTYNAME_CURRABB ))
        {
            OUString aSymbol, aExt;
            bool bBank = false;
            pFormat->GetNewCurrencySymbol( aSymbol, aExt );
            const NfCurrencyEntry* pCurr = pFormatter->GetCurrencyEntry( bBank,
                aSymbol, aExt, pFormat->GetLanguage() );
            if ( pCurr )
                aRet <<= rtl::OUString( pCurr->GetBankSymbol() );
            else
                aRet <<= rtl::OUString();
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

void SAL_CALL SvNumberFormatObj::addPropertyChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::removePropertyChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::addVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::removeVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL SvNumberFormatObj::getPropertyValues()
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : NULL;
    if (pFormat)
    {
        OUString aSymbol, aExt;
        OUString aAbb;
        bool bBank = false;
        pFormat->GetNewCurrencySymbol( aSymbol, aExt );
        const NfCurrencyEntry* pCurr = pFormatter->GetCurrencyEntry( bBank,
            aSymbol, aExt, pFormat->GetLanguage() );
        if ( pCurr )
            aAbb = pCurr->GetBankSymbol();

        OUString aFmtStr = pFormat->GetFormatstring();
        OUString aComment = pFormat->GetComment();
        sal_Bool bStandard = ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
        //! SvNumberformat Member bStandard rausreichen?
        sal_Bool bUserDef = ( ( pFormat->GetType() & NUMBERFORMAT_DEFINED ) != 0 );
        bool bThousand, bRed;
        sal_uInt16 nDecimals, nLeading;
        pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        lang::Locale aLocale( MsLangId::convertLanguageToLocale(
                    pFormat->GetLanguage()));

        uno::Sequence<beans::PropertyValue> aSeq(13);
        beans::PropertyValue* pArray = aSeq.getArray();

        pArray[0].Name = rtl::OUString(PROPERTYNAME_FMTSTR );
        pArray[0].Value <<= aFmtStr;
        pArray[1].Name = rtl::OUString(PROPERTYNAME_LOCALE );
        pArray[1].Value <<= aLocale;
        pArray[2].Name = rtl::OUString(PROPERTYNAME_TYPE );
        pArray[2].Value <<= (sal_Int16)( pFormat->GetType() );
        pArray[3].Name = rtl::OUString(PROPERTYNAME_COMMENT );
        pArray[3].Value <<= aComment;
        pArray[4].Name = rtl::OUString(PROPERTYNAME_STDFORM );
        pArray[4].Value.setValue( &bStandard, getBooleanCppuType() );
        pArray[5].Name = rtl::OUString(PROPERTYNAME_USERDEF );
        pArray[5].Value.setValue( &bUserDef, getBooleanCppuType() );
        pArray[6].Name = rtl::OUString(PROPERTYNAME_DECIMALS );
        pArray[6].Value <<= (sal_Int16)( nDecimals );
        pArray[7].Name = rtl::OUString(PROPERTYNAME_LEADING );
        pArray[7].Value <<= (sal_Int16)( nLeading );
        pArray[8].Name = rtl::OUString(PROPERTYNAME_NEGRED );
        pArray[8].Value.setValue( &bRed, getBooleanCppuType() );
        pArray[9].Name = rtl::OUString(PROPERTYNAME_THOUS );
        pArray[9].Value.setValue( &bThousand, getBooleanCppuType() );
        pArray[10].Name = rtl::OUString(PROPERTYNAME_CURRSYM );
        pArray[10].Value <<= aSymbol;
        pArray[11].Name = rtl::OUString(PROPERTYNAME_CURREXT );
        pArray[11].Value <<= aExt;
        pArray[12].Name = rtl::OUString(PROPERTYNAME_CURRABB );
        pArray[12].Value <<= aAbb;

        return aSeq;
    }
    else
        throw uno::RuntimeException();
}

void SAL_CALL SvNumberFormatObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& )
                        throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                                lang::IllegalArgumentException, lang::WrappedTargetException,
                                uno::RuntimeException)
{
    throw beans::UnknownPropertyException();    //  everything is read-only
}

// XServiceInfo

rtl::OUString SAL_CALL SvNumberFormatObj::getImplementationName()
                            throw(uno::RuntimeException)
{
    return rtl::OUString("SvNumberFormatObj");
}

sal_Bool SAL_CALL SvNumberFormatObj::supportsService( const rtl::OUString& ServiceName )
                            throw(uno::RuntimeException)
{
    return ( ServiceName.compareToAscii(SERVICENAME_NUMBERFORMAT) == 0 );
}

uno::Sequence<rtl::OUString> SAL_CALL SvNumberFormatObj::getSupportedServiceNames()
                            throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(SERVICENAME_NUMBERFORMAT);
    return aRet;
}

//------------------------------------------------------------------------

SvNumberFormatSettingsObj::SvNumberFormatSettingsObj( SvNumberFormatsSupplierObj& rParent, const ::comphelper::SharedMutex& _rMutex )
    :rSupplier( rParent )
    ,m_aMutex( _rMutex )
{
    rSupplier.acquire();
}

SvNumberFormatSettingsObj::~SvNumberFormatSettingsObj()
{
    rSupplier.release();
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL SvNumberFormatSettingsObj::getPropertySetInfo()
                            throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetNumberSettingsPropertyMap() );
    return aRef;
}

void SAL_CALL SvNumberFormatSettingsObj::setPropertyValue( const rtl::OUString& aPropertyName,
                                                    const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        String aString = aPropertyName;
        if (aString.EqualsAscii( PROPERTYNAME_NOZERO ))
        {
            //  operator >>= shouldn't be used for bool (?)
            if ( aValue.getValueTypeClass() == uno::TypeClass_BOOLEAN )
                pFormatter->SetNoZero( *(sal_Bool*)aValue.getValue() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_NULLDATE ))
        {
            util::Date aDate;
            if ( aValue >>= aDate )
                pFormatter->ChangeNullDate( aDate.Day, aDate.Month, aDate.Year );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_STDDEC ))
        {
            sal_Int16 nInt16 = sal_Int16();
            if ( aValue >>= nInt16 )
                pFormatter->ChangeStandardPrec( nInt16 );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_TWODIGIT ))
        {
            sal_Int16 nInt16 = sal_Int16();
            if ( aValue >>= nInt16 )
                pFormatter->SetYear2000( nInt16 );
        }
        else
            throw beans::UnknownPropertyException();

        rSupplier.SettingsChanged();
    }
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL SvNumberFormatSettingsObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Any aRet;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        String aString = aPropertyName;
        if (aString.EqualsAscii( PROPERTYNAME_NOZERO ))
        {
            sal_Bool bNoZero = pFormatter->GetNoZero();
            aRet.setValue( &bNoZero, getBooleanCppuType() );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_NULLDATE ))
        {
            Date* pDate = pFormatter->GetNullDate();
            if (pDate)
            {
                util::Date aUnoDate( pDate->GetDay(), pDate->GetMonth(), pDate->GetYear() );
                aRet <<= aUnoDate;
            }
        }
        else if (aString.EqualsAscii( PROPERTYNAME_STDDEC ))
            aRet <<= (sal_Int16)( pFormatter->GetStandardPrec() );
        else if (aString.EqualsAscii( PROPERTYNAME_TWODIGIT ))
            aRet <<= (sal_Int16)( pFormatter->GetYear2000() );
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

void SAL_CALL SvNumberFormatSettingsObj::addPropertyChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removePropertyChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::addVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removeVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

// XServiceInfo

rtl::OUString SAL_CALL SvNumberFormatSettingsObj::getImplementationName()
                            throw(uno::RuntimeException)
{
    return rtl::OUString("SvNumberFormatSettingsObj");
}

sal_Bool SAL_CALL SvNumberFormatSettingsObj::supportsService( const rtl::OUString& ServiceName )
                            throw(uno::RuntimeException)
{
    return ( ServiceName.compareToAscii(SERVICENAME_NUMBERSETTINGS) == 0 );
}

uno::Sequence<rtl::OUString> SAL_CALL SvNumberFormatSettingsObj::getSupportedServiceNames()
                            throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(SERVICENAME_NUMBERSETTINGS);
    return aRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
