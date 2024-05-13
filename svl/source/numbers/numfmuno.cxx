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
#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/util/NotNumericException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "numfmuno.hxx"
#include <svl/numformat.hxx>
#include <svl/numuno.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/itemprop.hxx>
#include <utility>

using namespace com::sun::star;

constexpr OUString PROPERTYNAME_FMTSTR = u"FormatString"_ustr;
constexpr OUString PROPERTYNAME_LOCALE = u"Locale"_ustr;
constexpr OUString PROPERTYNAME_TYPE = u"Type"_ustr;
constexpr OUString PROPERTYNAME_COMMENT = u"Comment"_ustr;
constexpr OUString PROPERTYNAME_CURREXT = u"CurrencyExtension"_ustr;
constexpr OUString PROPERTYNAME_CURRSYM = u"CurrencySymbol"_ustr;
constexpr OUString PROPERTYNAME_CURRABB = u"CurrencyAbbreviation"_ustr;
constexpr OUString PROPERTYNAME_DECIMALS = u"Decimals"_ustr;
constexpr OUString PROPERTYNAME_LEADING = u"LeadingZeros"_ustr;
constexpr OUString PROPERTYNAME_NEGRED = u"NegativeRed"_ustr;
constexpr OUString PROPERTYNAME_STDFORM = u"StandardFormat"_ustr;
constexpr OUString PROPERTYNAME_THOUS = u"ThousandsSeparator"_ustr;
constexpr OUString PROPERTYNAME_USERDEF = u"UserDefined"_ustr;

constexpr OUString PROPERTYNAME_NOZERO = u"NoZero"_ustr;
constexpr OUString PROPERTYNAME_NULLDATE = u"NullDate"_ustr;
constexpr OUString PROPERTYNAME_STDDEC = u"StandardDecimals"_ustr;
constexpr OUString PROPERTYNAME_TWODIGIT = u"TwoDigitDateStart"_ustr;

// All without a Which-ID, Map only for PropertySetInfo

static std::span<const SfxItemPropertyMapEntry> lcl_GetNumberFormatPropertyMap()
{
    static const SfxItemPropertyMapEntry aNumberFormatPropertyMap_Impl[] =
    {
        {PROPERTYNAME_FMTSTR,   0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_LOCALE,   0, cppu::UnoType<lang::Locale>::get(),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_TYPE,     0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_COMMENT,  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_CURREXT,  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_CURRSYM,  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_DECIMALS, 0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_LEADING,  0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_NEGRED,   0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_STDFORM,  0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_THOUS,    0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_USERDEF,  0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
        {PROPERTYNAME_CURRABB,  0, cppu::UnoType<OUString>::get(),    beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY, 0},
    };
    return aNumberFormatPropertyMap_Impl;
}

static std::span<const SfxItemPropertyMapEntry> lcl_GetNumberSettingsPropertyMap()
{
    static const SfxItemPropertyMapEntry aNumberSettingsPropertyMap_Impl[] =
    {
        {PROPERTYNAME_NOZERO,   0, cppu::UnoType<bool>::get(),         beans::PropertyAttribute::BOUND, 0},
        {PROPERTYNAME_NULLDATE, 0, cppu::UnoType<util::Date>::get(),  beans::PropertyAttribute::BOUND, 0},
        {PROPERTYNAME_STDDEC,   0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND, 0},
        {PROPERTYNAME_TWODIGIT, 0, cppu::UnoType<sal_Int16>::get(),   beans::PropertyAttribute::BOUND, 0},
    };
    return aNumberSettingsPropertyMap_Impl;
}

static LanguageType lcl_GetLanguage( const lang::Locale& rLocale )
{
    LanguageType eRet = LanguageTag::convertToLanguageTypeWithFallback( rLocale );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM; //! or throw an exception?

    return eRet;
}

SvNumberFormatterServiceObj::SvNumberFormatterServiceObj()
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

        SvNumberFormatsSupplierObj* pNew = comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>( _xSupplier );
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
    return xSupplier;
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

    const Color* pColor = nullptr;
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
    const Color* pColor = nullptr;
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

    const Color* pColor = nullptr;
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
    const Color* pColor = nullptr;
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
    const Color* pColor = nullptr;

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
    const Color* pColor = nullptr;

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
    return u"com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject"_ustr;
}

sal_Bool SAL_CALL SvNumberFormatterServiceObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatterServiceObj::getSupportedServiceNames()
{
    return { u"com.sun.star.util.NumberFormatter"_ustr };
}

SvNumberFormatsObj::SvNumberFormatsObj( SvNumberFormatsSupplierObj& _rParent, ::comphelper::SharedMutex  _aMutex )
    :m_xSupplier( &_rParent )
    ,m_aMutex(std::move( _aMutex ))
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
    sal_uInt32 nRet = pFormatter->GetEntryKey( aFormat, eLang );
    if (nRet == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        // This seems to be a workaround for what maybe the bScan option was
        // intended for? Tokenize the format code?

        // The format string based search is vague and fuzzy, as it is case
        // sensitive, but the format string is only half way cased, the
        // keywords (except the "General" keyword) are uppercased and literals
        // of course are not. Clients using this queryKey() and if not
        // successful addNew() may still fail if the result of PutEntry() is
        // false because the format actually existed, just with a different
        // casing. The only clean way is to just use PutEntry() and ignore the
        // duplicate case, which clients can't because the API doesn't provide
        // the information.
        // Try just another possibilty here, without any guarantee.

        // Use only ASCII upper, because keywords are only those.
        // Do not transliterate any quoted literals.
        const sal_Int32 nLen = aFormat.getLength();
        OUStringBuffer aBuf(0);
        sal_Unicode* p = aBuf.appendUninitialized( nLen + 1);
        memcpy( p, aFormat.getStr(), (nLen + 1) * sizeof(sal_Unicode));   // including 0-char
        aBuf.setLength( nLen);
        assert(p == aBuf.getStr());
        sal_Unicode const * const pStop = p + aBuf.getLength();
        bool bQuoted = false;
        for ( ; p < pStop; ++p)
        {
            if (bQuoted)
            {
                // Format codes don't have embedded doubled quotes, i.e. "a""b"
                // is two literals displayed as `ab`.
                if (*p == '"')
                    bQuoted = false;
            }
            else if (*p == '"')
                bQuoted = true;
            else if (rtl::isAsciiLowerCase(*p))
                *p = rtl::toAsciiUpperCase(*p);
            else if (*p == '\\')
                ++p;            // skip escaped next char
                // Theoretically that should cater for UTF-32 with
                // iterateCodePoints(), but such character would not match any
                // of [a-z\"] in its UTF-16 units.
        }
        nRet = pFormatter->GetEntryKey( aBuf, eLang );
    }
    return static_cast<sal_Int32>(nRet);
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
    else if (aFormStr != aFormat)
    {
        // The format exists but with a different format code string, and if it
        // was only uppercase vs lowercase keywords; but also syntax extensions
        // are possible like resulting embedded LCIDs and what not the client
        // doesn't know about. Silently accept instead of throwing an error.
        nRet = nKey;
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
    return u"SvNumberFormatsObj"_ustr;
}

sal_Bool SAL_CALL SvNumberFormatsObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatsObj::getSupportedServiceNames()
{
    return { u"com.sun.star.util.NumberFormats"_ustr };
}

SvNumberFormatObj::SvNumberFormatObj( SvNumberFormatsSupplierObj& rParent, sal_Int32 nK, ::comphelper::SharedMutex _aMutex )
    :m_xSupplier( &rParent )
    ,nKey( nK )
    ,m_aMutex(std::move( _aMutex ))
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
        throw beans::UnknownPropertyException(aPropertyName);

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
    return u"SvNumberFormatObj"_ustr;
}

sal_Bool SAL_CALL SvNumberFormatObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatObj::getSupportedServiceNames()
{
    return { u"com.sun.star.util.NumberFormatProperties"_ustr };
}

SvNumberFormatSettingsObj::SvNumberFormatSettingsObj( SvNumberFormatsSupplierObj& rParent, ::comphelper::SharedMutex _aMutex )
    :m_xSupplier( &rParent )
    ,m_aMutex(std::move( _aMutex ))
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
        throw beans::UnknownPropertyException(aPropertyName);

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
        throw beans::UnknownPropertyException(aPropertyName);

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
    return u"SvNumberFormatSettingsObj"_ustr;
}

sal_Bool SAL_CALL SvNumberFormatSettingsObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatSettingsObj::getSupportedServiceNames()
{
    return { u"com.sun.star.util.NumberFormatSettings"_ustr };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_uno_util_numbers_SvNumberFormatterServiceObject_get_implementation(css::uno::XComponentContext*,
                                                                                css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvNumberFormatterServiceObj());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
