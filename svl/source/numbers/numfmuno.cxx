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
#include <i18nlangtag/mslangid.hxx>
#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/util/NotNumericException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "numfmuno.hxx"
#include <svl/numuno.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/itemprop.hxx>

using namespace com::sun::star;

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

// All without a Which-ID, Map only for PropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetNumberFormatPropertyMap()
{
    static const SfxItemPropertyMapEntry aNumberFormatPropertyMap_Impl[] =
    {
        {OUString(PROPERTYNAME_FMTSTR),   0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_LOCALE),   0, cppu::UnoType<lang::Locale>::get(),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_TYPE),     0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_COMMENT),  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_CURREXT),  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_CURRSYM),  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_DECIMALS), 0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_LEADING),  0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_NEGRED),   0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_STDFORM),  0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_THOUS),    0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_USERDEF),  0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {OUString(PROPERTYNAME_CURRABB),  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aNumberFormatPropertyMap_Impl;
}

static const SfxItemPropertyMapEntry* lcl_GetNumberSettingsPropertyMap()
{
    static const SfxItemPropertyMapEntry aNumberSettingsPropertyMap_Impl[] =
    {
        {OUString(PROPERTYNAME_NOZERO),   0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND, 0},
        {OUString(PROPERTYNAME_NULLDATE), 0, cppu::UnoType<util::Date>::get(),  beans::PropertyAttribute::BOUND, 0},
        {OUString(PROPERTYNAME_STDDEC),   0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND, 0},
        {OUString(PROPERTYNAME_TWODIGIT), 0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aNumberSettingsPropertyMap_Impl;
}

static LanguageType lcl_GetLanguage( const lang::Locale& rLocale )
{
    LanguageType eRet = LanguageTag::convertToLanguageType( rLocale, false);
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM; //! or throw an exception?

    return eRet;
}

SvNumberFormatterServiceObj::SvNumberFormatterServiceObj()
    :m_aMutex()
{
}

SvNumberFormatterServiceObj::~SvNumberFormatterServiceObj()
{
}

// XNumberFormatter

void SAL_CALL SvNumberFormatterServiceObj::attachNumberFormatsSupplier( const uno::Reference<util::XNumberFormatsSupplier>& _xSupplier )
{
    ::rtl::Reference< SvNumberFormatsSupplierObj > xAutoReleaseOld;

    // SYNCHRONIZED ->
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

        SvNumberFormatsSupplierObj* pNew = SvNumberFormatsSupplierObj::getImplementation( _xSupplier );
        if (!pNew)
            throw uno::RuntimeException(); // wrong object

        xAutoReleaseOld = xSupplier;

        xSupplier = pNew;
        m_aMutex = xSupplier->getSharedMutex();
    }
    // <- SYNCHRONIZED
}

uno::Reference<util::XNumberFormatsSupplier> SAL_CALL SvNumberFormatterServiceObj::getNumberFormatsSupplier()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return xSupplier.get();
}

sal_Int32 SAL_CALL SvNumberFormatterServiceObj::detectNumberFormat( sal_Int32 nKey, const OUString& aString )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
        throw uno::RuntimeException();

    sal_uInt32 nUKey = nKey;
    double fValue = 0.0;
    if ( !pFormatter->IsNumberFormat(aString, nUKey, fValue) )
        throw util::NotNumericException();

    return nUKey;
}

double SAL_CALL SvNumberFormatterServiceObj::convertStringToNumber( sal_Int32 nKey, const OUString& aString )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
        throw uno::RuntimeException();

    sal_uInt32 nUKey = nKey;
    double fValue = 0.0;
    if ( !pFormatter->IsNumberFormat(aString, nUKey, fValue) )
        throw util::NotNumericException();

    return fValue;
}

OUString SAL_CALL SvNumberFormatterServiceObj::convertNumberToString( sal_Int32 nKey, double fValue )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
        throw uno::RuntimeException();

    Color* pColor = nullptr;
    pFormatter->GetOutputString(fValue, nKey, aRet, &pColor);

    return aRet;
}

sal_Int32 SAL_CALL SvNumberFormatterServiceObj::queryColorForNumber( sal_Int32 nKey,
                                                                       double fValue,
                                                                       sal_Int32 aDefaultColor )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor; // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
        throw uno::RuntimeException();

    OUString aStr;
    Color* pColor = nullptr;
    pFormatter->GetOutputString(fValue, nKey, aStr, &pColor);
    if (pColor)
        nRet = sal_uInt32(*pColor);
    // Else keep Default

    return nRet;
}

OUString SAL_CALL SvNumberFormatterServiceObj::formatString( sal_Int32 nKey,
                                                             const OUString& aString )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
    {
        throw uno::RuntimeException();
    }

    Color* pColor = nullptr;
    pFormatter->GetOutputString(aString, nKey, aRet, &pColor);

    return aRet;
}

sal_Int32 SAL_CALL SvNumberFormatterServiceObj::queryColorForString( sal_Int32 nKey,
                                                                       const OUString& aString,
                                                                       sal_Int32 aDefaultColor )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor; // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
    {
        throw uno::RuntimeException();
    }

    OUString aStr;
    Color* pColor = nullptr;
    pFormatter->GetOutputString(aString, nKey, aStr, &pColor);
    if (pColor)
    {
        nRet = sal_uInt32(*pColor);
    }
    // Else keep Default

    return nRet;
}

OUString SAL_CALL SvNumberFormatterServiceObj::getInputString( sal_Int32 nKey, double fValue )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
        throw uno::RuntimeException();

    pFormatter->GetInputLineString(fValue, nKey, aRet);

    return aRet;
}

// XNumberFormatPreviewer

OUString SAL_CALL SvNumberFormatterServiceObj::convertNumberToPreviewString( const OUString& aFormat,
                                                                             double fValue,
                                                                             const lang::Locale& nLocale,
                                                                             sal_Bool bAllowEnglish )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
        throw uno::RuntimeException();

    LanguageType eLang = lcl_GetLanguage( nLocale );
    Color* pColor = nullptr;

    bool bOk;
    if ( bAllowEnglish )
        bOk = pFormatter->GetPreviewStringGuess( aFormat, fValue, aRet, &pColor, eLang );
    else
        bOk = pFormatter->GetPreviewString( aFormat, fValue, aRet, &pColor, eLang );

    if (!bOk)
        throw util::MalformedNumberFormatException();

    return aRet;
}

sal_Int32 SAL_CALL SvNumberFormatterServiceObj::queryPreviewColorForNumber( const OUString& aFormat,
                                                                              double fValue,
                                                                              const lang::Locale& nLocale,
                                                                              sal_Bool bAllowEnglish,
                                                                              sal_Int32 aDefaultColor )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor; // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (!pFormatter)
        throw uno::RuntimeException();

    OUString aOutString;
    LanguageType eLang = lcl_GetLanguage( nLocale );
    Color* pColor = nullptr;

    bool bOk;
    if ( bAllowEnglish )
        bOk = pFormatter->GetPreviewStringGuess( aFormat, fValue, aOutString, &pColor, eLang );
    else
        bOk = pFormatter->GetPreviewString( aFormat, fValue, aOutString, &pColor, eLang );

    if (!bOk)
        throw util::MalformedNumberFormatException();

    if (pColor)
        nRet = sal_uInt32(*pColor);
    // Else keep Default

    return nRet;
}

// XServiceInfo

OUString SAL_CALL SvNumberFormatterServiceObj::getImplementationName()
{
    return OUString("com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject");
}

sal_Bool SAL_CALL SvNumberFormatterServiceObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatterServiceObj::getSupportedServiceNames()
{
    return { "com.sun.star.util.NumberFormatter" };
}

SvNumberFormatsObj::SvNumberFormatsObj( SvNumberFormatsSupplierObj& _rParent, ::comphelper::SharedMutex const & _rMutex )
    :m_xSupplier( &_rParent )
    ,m_aMutex( _rMutex )
{
}

SvNumberFormatsObj::~SvNumberFormatsObj()
{
}

// XNumberFormats

uno::Reference<beans::XPropertySet> SAL_CALL SvNumberFormatsObj::getByKey( sal_Int32 nKey )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : nullptr;
    if (!pFormat)
        throw uno::RuntimeException();

    return new SvNumberFormatObj( *m_xSupplier, nKey, m_aMutex );
}

uno::Sequence<sal_Int32> SAL_CALL SvNumberFormatsObj::queryKeys( sal_Int16 nType,
                                                                 const lang::Locale& nLocale,
                                                                 sal_Bool bCreate )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if ( !pFormatter )
        throw uno::RuntimeException();

    sal_uInt32 nIndex = 0;
    LanguageType eLang = lcl_GetLanguage( nLocale );
    SvNumberFormatTable& rTable = bCreate ?
                                    pFormatter->ChangeCL( static_cast<SvNumFormatType>(nType), nIndex, eLang ) :
                                    pFormatter->GetEntryTable( static_cast<SvNumFormatType>(nType), nIndex, eLang );
    sal_uInt32 nCount = rTable.size();
    uno::Sequence<sal_Int32> aSeq(nCount);
    sal_Int32* pAry = aSeq.getArray();
    sal_uInt32 i=0;
    for (const auto& rEntry : rTable)
    {
        pAry[i] = rEntry.first;
        ++i;
    }
    return aSeq;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::queryKey( const OUString& aFormat,
                                                 const lang::Locale& nLocale,
                                                 sal_Bool bScan )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    LanguageType eLang = lcl_GetLanguage( nLocale );
    if (bScan)
    {
        //! FIXME: Something still needs to happen here ...
    }
    sal_Int32 nRet = pFormatter->GetEntryKey( aFormat, eLang );
    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::addNew( const OUString& aFormat,
                                               const lang::Locale& nLocale )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    OUString aFormStr = aFormat;
    LanguageType eLang = lcl_GetLanguage( nLocale );
    sal_uInt32 nKey = 0;
    sal_Int32 nCheckPos = 0;
    SvNumFormatType nType = SvNumFormatType::ALL;
    bool bOk = pFormatter->PutEntry( aFormStr, nCheckPos, nType, nKey, eLang );
    if (bOk)
        nRet = nKey;
    else if (nCheckPos)
    {
        throw util::MalformedNumberFormatException(); // Invalid Format
    }
    else
        throw uno::RuntimeException(); // Other error (e.g. already added)

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::addNewConverted( const OUString& aFormat,
                                                        const lang::Locale& nLocale,
                                                        const lang::Locale& nNewLocale )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    OUString aFormStr = aFormat;
    LanguageType eLang = lcl_GetLanguage( nLocale );
    LanguageType eNewLang = lcl_GetLanguage( nNewLocale );
    sal_uInt32 nKey = 0;
    sal_Int32 nCheckPos = 0;
    SvNumFormatType nType = SvNumFormatType::ALL;
    // This is used also when reading OOXML documents, there's no indicator
    // whether to convert date particle order as well, so don't. See tdf#119013
    bool bOk = pFormatter->PutandConvertEntry( aFormStr, nCheckPos, nType, nKey, eLang, eNewLang, false);
    if (bOk || nKey > 0)
        nRet = nKey;
    else if (nCheckPos)
    {
        throw util::MalformedNumberFormatException();       // Invalid format
    }
    else
        throw uno::RuntimeException(); // Other error (e.g. already added)

    return nRet;
}

void SAL_CALL SvNumberFormatsObj::removeByKey( sal_Int32 nKey )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();

    if (pFormatter)
    {
        pFormatter->DeleteEntry(nKey);
    }
}

OUString SAL_CALL SvNumberFormatsObj::generateFormat( sal_Int32 nBaseKey,
                                                      const lang::Locale& nLocale,
                                                      sal_Bool bThousands,
                                                      sal_Bool bRed, sal_Int16 nDecimals,
                                                      sal_Int16 nLeading )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    LanguageType eLang = lcl_GetLanguage( nLocale );
    OUString aRet = pFormatter->GenerateFormat(nBaseKey, eLang, bThousands, bRed, nDecimals, nLeading);
    return aRet;
}

// XNumberFormatTypes

sal_Int32 SAL_CALL SvNumberFormatsObj::getStandardIndex( const lang::Locale& nLocale )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    LanguageType eLang = lcl_GetLanguage( nLocale );
    sal_Int32 nRet = pFormatter->GetStandardIndex(eLang);
    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getStandardFormat( sal_Int16 nType, const lang::Locale& nLocale )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    LanguageType eLang = lcl_GetLanguage( nLocale );
    // Mask out "defined" bit, so type from an existing number format
    // can directly be used for getStandardFormat
    SvNumFormatType nType2 = static_cast<SvNumFormatType>(nType);
    nType2 &= ~SvNumFormatType::DEFINED;
    sal_Int32 nRet = pFormatter->GetStandardFormat(nType2, eLang);
    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getFormatIndex( sal_Int16 nIndex, const lang::Locale& nLocale )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    LanguageType eLang = lcl_GetLanguage( nLocale );
    sal_Int32 nRet = pFormatter->GetFormatIndex( static_cast<NfIndexTableOffset>(nIndex), eLang );
    return nRet;
}

sal_Bool SAL_CALL SvNumberFormatsObj::isTypeCompatible( sal_Int16 nOldType, sal_Int16 nNewType )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return SvNumberFormatter::IsCompatible( static_cast<SvNumFormatType>(nOldType), static_cast<SvNumFormatType>(nNewType) );
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getFormatForLocale( sal_Int32 nKey, const lang::Locale& nLocale )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    LanguageType eLang = lcl_GetLanguage( nLocale );
    sal_Int32 nRet = pFormatter->GetFormatForLanguageIfBuiltIn(nKey, eLang);
    return nRet;
}

// XServiceInfo

OUString SAL_CALL SvNumberFormatsObj::getImplementationName()
{
    return OUString("SvNumberFormatsObj");
}

sal_Bool SAL_CALL SvNumberFormatsObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatsObj::getSupportedServiceNames()
{
    return { "com.sun.star.util.NumberFormats" };
}

SvNumberFormatObj::SvNumberFormatObj( SvNumberFormatsSupplierObj& rParent, sal_uLong nK, const ::comphelper::SharedMutex& _rMutex )
    :m_xSupplier( &rParent )
    ,nKey( nK )
    ,m_aMutex( _rMutex )
{
}

SvNumberFormatObj::~SvNumberFormatObj()
{
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL SvNumberFormatObj::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetNumberFormatPropertyMap() );
    return aRef;
}

void SAL_CALL SvNumberFormatObj::setPropertyValue( const OUString&,
                                                   const uno::Any& )
{
    throw beans::UnknownPropertyException(); // Everything is read-only
}

uno::Any SAL_CALL SvNumberFormatObj::getPropertyValue( const OUString& aPropertyName )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Any aRet;
    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : nullptr;
    if (!pFormat)
        throw uno::RuntimeException();

    bool bThousand, bRed;
    sal_uInt16 nDecimals, nLeading;

    if (aPropertyName == PROPERTYNAME_FMTSTR)
    {
        aRet <<= pFormat->GetFormatstring();
    }
    else if (aPropertyName == PROPERTYNAME_LOCALE)
    {
        lang::Locale aLocale( LanguageTag::convertToLocale( pFormat->GetLanguage(), false));
        aRet <<= aLocale;
    }
    else if (aPropertyName == PROPERTYNAME_TYPE)
    {
        aRet <<= static_cast<sal_Int16>( pFormat->GetType() );
    }
    else if (aPropertyName == PROPERTYNAME_COMMENT)
    {
        aRet <<= pFormat->GetComment();
    }
    else if (aPropertyName == PROPERTYNAME_STDFORM)
    {
        //! Pass through SvNumberformat Member bStandard?
        aRet <<= ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
    }
    else if (aPropertyName == PROPERTYNAME_USERDEF)
    {
        aRet <<= bool( pFormat->GetType() & SvNumFormatType::DEFINED );
    }
    else if (aPropertyName == PROPERTYNAME_DECIMALS)
    {
        pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        aRet <<= static_cast<sal_Int16>(nDecimals);
    }
    else if (aPropertyName == PROPERTYNAME_LEADING)
    {
        pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        aRet <<= static_cast<sal_Int16>(nLeading);
    }
    else if (aPropertyName == PROPERTYNAME_NEGRED)
    {
        pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        aRet <<= bRed;
    }
    else if (aPropertyName == PROPERTYNAME_THOUS)
    {
        pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        aRet <<= bThousand;
    }
    else if (aPropertyName == PROPERTYNAME_CURRSYM)
    {
        OUString aSymbol, aExt;
        pFormat->GetNewCurrencySymbol( aSymbol, aExt );
        aRet <<= aSymbol;
    }
    else if (aPropertyName == PROPERTYNAME_CURREXT)
    {
        OUString aSymbol, aExt;
        pFormat->GetNewCurrencySymbol( aSymbol, aExt );
        aRet <<= aExt;
    }
    else if (aPropertyName == PROPERTYNAME_CURRABB)
    {
        OUString aSymbol, aExt;
        bool bBank = false;
        pFormat->GetNewCurrencySymbol( aSymbol, aExt );
        const NfCurrencyEntry* pCurr = SvNumberFormatter::GetCurrencyEntry( bBank,
            aSymbol, aExt, pFormat->GetLanguage() );
        if ( pCurr )
            aRet <<= pCurr->GetBankSymbol();
        else
            aRet <<= OUString();
    }
    else
        throw beans::UnknownPropertyException();

    return aRet;
}

void SAL_CALL SvNumberFormatObj::addPropertyChangeListener( const OUString&,
                                                            const uno::Reference<beans::XPropertyChangeListener>&)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::removePropertyChangeListener( const OUString&,
                                                               const uno::Reference<beans::XPropertyChangeListener>&)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::addVetoableChangeListener( const OUString&,
                                                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::removeVetoableChangeListener( const OUString&,
                                                               const uno::Reference<beans::XVetoableChangeListener>&)
{
    OSL_FAIL("not implemented");
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL SvNumberFormatObj::getPropertyValues()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : nullptr;
    if (!pFormat)
        throw uno::RuntimeException();

    OUString aSymbol, aExt;
    OUString aAbb;
    bool bBank = false;
    pFormat->GetNewCurrencySymbol( aSymbol, aExt );
    const NfCurrencyEntry* pCurr = SvNumberFormatter::GetCurrencyEntry( bBank,
        aSymbol, aExt, pFormat->GetLanguage() );
    if ( pCurr )
        aAbb = pCurr->GetBankSymbol();

    OUString aFmtStr = pFormat->GetFormatstring();
    OUString aComment = pFormat->GetComment();
    bool bStandard = ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
    //! Pass through SvNumberformat Member bStandard?
    bool bUserDef( pFormat->GetType() & SvNumFormatType::DEFINED );
    bool bThousand, bRed;
    sal_uInt16 nDecimals, nLeading;
    pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
    lang::Locale aLocale( LanguageTag( pFormat->GetLanguage()).getLocale());

    uno::Sequence<beans::PropertyValue> aSeq( comphelper::InitPropertySequence({
            { PROPERTYNAME_FMTSTR, uno::Any(aFmtStr) },
            { PROPERTYNAME_LOCALE, uno::Any(aLocale) },
            { PROPERTYNAME_TYPE, uno::Any(sal_Int16( pFormat->GetType() )) },
            { PROPERTYNAME_COMMENT, uno::Any(aComment) },
            { PROPERTYNAME_STDFORM, uno::Any(bStandard) },
            { PROPERTYNAME_USERDEF, uno::Any(bUserDef) },
            { PROPERTYNAME_DECIMALS, uno::Any(sal_Int16( nDecimals )) },
            { PROPERTYNAME_LEADING, uno::Any(sal_Int16( nLeading )) },
            { PROPERTYNAME_NEGRED, uno::Any(bRed) },
            { PROPERTYNAME_THOUS, uno::Any(bThousand) },
            { PROPERTYNAME_CURRSYM, uno::Any(aSymbol) },
            { PROPERTYNAME_CURREXT, uno::Any(aExt) },
            { PROPERTYNAME_CURRABB, uno::Any(aAbb) }
        }));

    return aSeq;
}

void SAL_CALL SvNumberFormatObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& )
{
    throw beans::UnknownPropertyException(); // Everything is read-only
}

// XServiceInfo

OUString SAL_CALL SvNumberFormatObj::getImplementationName()
{
    return OUString("SvNumberFormatObj");
}

sal_Bool SAL_CALL SvNumberFormatObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatObj::getSupportedServiceNames()
{
    return { "com.sun.star.util.NumberFormatProperties" };
}

SvNumberFormatSettingsObj::SvNumberFormatSettingsObj( SvNumberFormatsSupplierObj& rParent, const ::comphelper::SharedMutex& _rMutex )
    :m_xSupplier( &rParent )
    ,m_aMutex( _rMutex )
{
}

SvNumberFormatSettingsObj::~SvNumberFormatSettingsObj()
{
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL SvNumberFormatSettingsObj::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetNumberSettingsPropertyMap() );
    return aRef;
}

void SAL_CALL SvNumberFormatSettingsObj::setPropertyValue( const OUString& aPropertyName,
                                                           const uno::Any& aValue )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    if (aPropertyName == PROPERTYNAME_NOZERO)
    {
        //  operator >>= shouldn't be used for bool (?)
        if ( auto b = o3tl::tryAccess<bool>(aValue) )
            pFormatter->SetNoZero( *b );
    }
    else if (aPropertyName == PROPERTYNAME_NULLDATE)
    {
        util::Date aDate;
        if ( aValue >>= aDate )
            pFormatter->ChangeNullDate( aDate.Day, aDate.Month, aDate.Year );
    }
    else if (aPropertyName == PROPERTYNAME_STDDEC)
    {
        sal_Int16 nInt16 = sal_Int16();
        if ( aValue >>= nInt16 )
            pFormatter->ChangeStandardPrec( nInt16 );
    }
    else if (aPropertyName == PROPERTYNAME_TWODIGIT)
    {
        sal_Int16 nInt16 = sal_Int16();
        if ( aValue >>= nInt16 )
            pFormatter->SetYear2000( nInt16 );
    }
    else
        throw beans::UnknownPropertyException();

}

uno::Any SAL_CALL SvNumberFormatSettingsObj::getPropertyValue( const OUString& aPropertyName )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Any aRet;
    SvNumberFormatter* pFormatter = m_xSupplier->GetNumberFormatter();
    if (!pFormatter)
        throw uno::RuntimeException();

    if (aPropertyName == PROPERTYNAME_NOZERO)
    {
        aRet <<= pFormatter->GetNoZero();
    }
    else if (aPropertyName == PROPERTYNAME_NULLDATE)
    {
        const Date& rDate = pFormatter->GetNullDate();
        aRet <<= rDate.GetUNODate();
    }
    else if (aPropertyName == PROPERTYNAME_STDDEC)
        aRet <<= static_cast<sal_Int16>( pFormatter->GetStandardPrec() );
    else if (aPropertyName == PROPERTYNAME_TWODIGIT)
        aRet <<= static_cast<sal_Int16>( pFormatter->GetYear2000() );
    else
        throw beans::UnknownPropertyException();

    return aRet;
}

void SAL_CALL SvNumberFormatSettingsObj::addPropertyChangeListener( const OUString&,
                                                                    const uno::Reference<beans::XPropertyChangeListener>&)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removePropertyChangeListener( const OUString&,
                                                                       const uno::Reference<beans::XPropertyChangeListener>&)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::addVetoableChangeListener( const OUString&,
                                                                    const uno::Reference<beans::XVetoableChangeListener>&)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removeVetoableChangeListener( const OUString&,
                                                                       const uno::Reference<beans::XVetoableChangeListener>&)
{
    OSL_FAIL("not implemented");
}

// XServiceInfo

OUString SAL_CALL SvNumberFormatSettingsObj::getImplementationName()
{
    return OUString("SvNumberFormatSettingsObj");
}

sal_Bool SAL_CALL SvNumberFormatSettingsObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatSettingsObj::getSupportedServiceNames()
{
    return { "com.sun.star.util.NumberFormatSettings" };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_uno_util_numbers_SvNumberFormatterServiceObject_get_implementation(css::uno::XComponentContext*,
                                                                                css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvNumberFormatterServiceObj());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
