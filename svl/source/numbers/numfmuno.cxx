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
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
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
    //  empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.isEmpty() )
        return LANGUAGE_SYSTEM;

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
    throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return xSupplier.get();
}

sal_Int32 SAL_CALL SvNumberFormatterServiceObj::detectNumberFormat( sal_Int32 nKey, const OUString& aString )
    throw(util::NotNumericException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        sal_uInt32 nUKey = nKey;
        double fValue = 0.0;
        if ( pFormatter->IsNumberFormat(aString, nUKey, fValue) )
            nRet = nUKey;
        else
            throw util::NotNumericException();
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

double SAL_CALL SvNumberFormatterServiceObj::convertStringToNumber( sal_Int32 nKey, const OUString& aString )
    throw(util::NotNumericException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    double fRet = 0.0;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        sal_uInt32 nUKey = nKey;
        double fValue = 0.0;
        if ( pFormatter->IsNumberFormat(aString, nUKey, fValue) )
            fRet = fValue;
        else
            throw util::NotNumericException();
    }
    else
        throw uno::RuntimeException();

    return fRet;
}

OUString SAL_CALL SvNumberFormatterServiceObj::convertNumberToString( sal_Int32 nKey, double fValue )
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        Color* pColor = nullptr;
        pFormatter->GetOutputString(fValue, nKey, aRet, &pColor);
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

util::Color SAL_CALL SvNumberFormatterServiceObj::queryColorForNumber( sal_Int32 nKey,
                                                                       double fValue,
                                                                       util::Color aDefaultColor )
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor; // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        OUString aStr;
        Color* pColor = nullptr;
        pFormatter->GetOutputString(fValue, nKey, aStr, &pColor);
        if (pColor)
            nRet = pColor->GetColor();
        // Else keep Default
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

OUString SAL_CALL SvNumberFormatterServiceObj::formatString( sal_Int32 nKey,
                                                             const OUString& aString )
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        Color* pColor = nullptr;
        pFormatter->GetOutputString(aString, nKey, aRet, &pColor);
    }
    else
    {
        throw uno::RuntimeException();
    }
    return aRet;
}

util::Color SAL_CALL SvNumberFormatterServiceObj::queryColorForString( sal_Int32 nKey,
                                                                       const OUString& aString,
                                                                       util::Color aDefaultColor )
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor; // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        OUString aStr;
        Color* pColor = nullptr;
        pFormatter->GetOutputString(aString, nKey, aStr, &pColor);
        if (pColor)
        {
            nRet = pColor->GetColor();
        }
        // Else keep Default
    }
    else
    {
        throw uno::RuntimeException();
    }

    return nRet;
}

OUString SAL_CALL SvNumberFormatterServiceObj::getInputString( sal_Int32 nKey, double fValue )
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
        pFormatter->GetInputLineString(fValue, nKey, aRet);
    else
        throw uno::RuntimeException();

    return aRet;
}

// XNumberFormatPreviewer

OUString SAL_CALL SvNumberFormatterServiceObj::convertNumberToPreviewString( const OUString& aFormat,
                                                                             double fValue,
                                                                             const lang::Locale& nLocale,
                                                                             sal_Bool bAllowEnglish )
    throw(util::MalformedNumberFormatException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString aRet;
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        Color* pColor = nullptr;

        bool bOk;
        if ( bAllowEnglish )
            bOk = pFormatter->GetPreviewStringGuess( aFormat, fValue, aRet, &pColor, eLang );
        else
            bOk = pFormatter->GetPreviewString( aFormat, fValue, aRet, &pColor, eLang );

        if (!bOk)
            throw util::MalformedNumberFormatException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

util::Color SAL_CALL SvNumberFormatterServiceObj::queryPreviewColorForNumber( const OUString& aFormat,
                                                                              double fValue,
                                                                              const lang::Locale& nLocale,
                                                                              sal_Bool bAllowEnglish,
                                                                              util::Color aDefaultColor )
    throw(util::MalformedNumberFormatException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    util::Color nRet = aDefaultColor; // color = sal_Int32
    SvNumberFormatter* pFormatter = xSupplier.is() ? xSupplier->GetNumberFormatter() : nullptr;
    if (pFormatter)
    {
        OUString aOutString;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        Color* pColor = nullptr;

        bool bOk;
        if ( bAllowEnglish )
            bOk = pFormatter->GetPreviewStringGuess( aFormat, fValue, aOutString, &pColor, eLang );
        else
            bOk = pFormatter->GetPreviewString( aFormat, fValue, aOutString, &pColor, eLang );

        if (bOk)
        {
            if (pColor)
                nRet = pColor->GetColor();
            // Else keep Default
        }
        else
            throw util::MalformedNumberFormatException();
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

// XServiceInfo

OUString SAL_CALL SvNumberFormatterServiceObj::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject");
}

sal_Bool SAL_CALL SvNumberFormatterServiceObj::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatterServiceObj::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet { "com.sun.star.util.NumberFormatter" };
    return aRet;
}

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
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : nullptr;
    if (pFormat)
        return new SvNumberFormatObj( rSupplier, nKey, m_aMutex );
    else
        throw uno::RuntimeException();
}

uno::Sequence<sal_Int32> SAL_CALL SvNumberFormatsObj::queryKeys( sal_Int16 nType,
                                                                 const lang::Locale& nLocale,
                                                                 sal_Bool bCreate )
    throw(uno::RuntimeException, std::exception)
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

sal_Int32 SAL_CALL SvNumberFormatsObj::queryKey( const OUString& aFormat,
                                                 const lang::Locale& nLocale,
                                                 sal_Bool bScan )
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        if (bScan)
        {
            //! FIXME: Something still needs to happen here ...
        }
        nRet = pFormatter->GetEntryKey( aFormat, eLang );
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::addNew( const OUString& aFormat,
                                               const lang::Locale& nLocale )
    throw(util::MalformedNumberFormatException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        OUString aFormStr = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        sal_uInt32 nKey = 0;
        sal_Int32 nCheckPos = 0;
        short nType = 0;
        bool bOk = pFormatter->PutEntry( aFormStr, nCheckPos, nType, nKey, eLang );
        if (bOk)
            nRet = nKey;
        else if (nCheckPos)
        {
            throw util::MalformedNumberFormatException(); // Invalid Format
        }
        else
            throw uno::RuntimeException(); // Other error (e.g. already added)
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::addNewConverted( const OUString& aFormat,
                                                        const lang::Locale& nLocale,
                                                        const lang::Locale& nNewLocale )
    throw(util::MalformedNumberFormatException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        OUString aFormStr = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        LanguageType eNewLang = lcl_GetLanguage( nNewLocale );
        sal_uInt32 nKey = 0;
        sal_Int32 nCheckPos = 0;
        short nType = 0;
        bool bOk = pFormatter->PutandConvertEntry( aFormStr, nCheckPos, nType, nKey, eLang, eNewLang );
        if (bOk || nKey > 0)
            nRet = nKey;
        else if (nCheckPos)
        {
            throw util::MalformedNumberFormatException();       // Invalid format
        }
        else
            throw uno::RuntimeException(); // Other error (e.g. already added)
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

void SAL_CALL SvNumberFormatsObj::removeByKey( sal_Int32 nKey ) throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();

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
    throw(uno::RuntimeException, std::exception)
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
                            throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nRet = 0;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        // Mask out "defined" bit, so type from an existing number format
        // can directly be used for getStandardFormat
        nType &= ~css::util::NumberFormat::DEFINED;
        nRet = pFormatter->GetStandardFormat(nType, eLang);
    }
    else
        throw uno::RuntimeException();

    return nRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getFormatIndex( sal_Int16 nIndex, const lang::Locale& nLocale )
    throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bRet = false;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
        bRet = pFormatter->IsCompatible( nOldType, nNewType );
    else
        throw uno::RuntimeException();

    return bRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getFormatForLocale( sal_Int32 nKey, const lang::Locale& nLocale )
    throw(uno::RuntimeException, std::exception)
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

OUString SAL_CALL SvNumberFormatsObj::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("SvNumberFormatsObj");
}

sal_Bool SAL_CALL SvNumberFormatsObj::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatsObj::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet { "com.sun.star.util.NumberFormats" };
    return aRet;
}

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
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetNumberFormatPropertyMap() );
    return aRef;
}

void SAL_CALL SvNumberFormatObj::setPropertyValue( const OUString&,
                                                   const uno::Any& )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
          lang::IllegalArgumentException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException(); // Everything is read-only
}

uno::Any SAL_CALL SvNumberFormatObj::getPropertyValue( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Any aRet;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : nullptr;
    if (pFormat)
    {
        bool bThousand, bRed;
        sal_uInt16 nDecimals, nLeading;

        if (aPropertyName == PROPERTYNAME_FMTSTR)
        {
            aRet <<= OUString( pFormat->GetFormatstring() );
        }
        else if (aPropertyName == PROPERTYNAME_LOCALE)
        {
            lang::Locale aLocale( LanguageTag( pFormat->GetLanguage()).getLocale());
            aRet <<= aLocale;
        }
        else if (aPropertyName == PROPERTYNAME_TYPE)
        {
            aRet <<= (sal_Int16)( pFormat->GetType() );
        }
        else if (aPropertyName == PROPERTYNAME_COMMENT)
        {
            aRet <<= OUString( pFormat->GetComment() );
        }
        else if (aPropertyName == PROPERTYNAME_STDFORM)
        {
            //! Pass through SvNumberformat Member bStandard?
            sal_Bool bStandard = ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
            aRet.setValue( &bStandard, cppu::UnoType<bool>::get() );
        }
        else if (aPropertyName == PROPERTYNAME_USERDEF)
        {
            sal_Bool bUserDef = ( ( pFormat->GetType() & css::util::NumberFormat::DEFINED ) != 0 );
            aRet.setValue( &bUserDef, cppu::UnoType<bool>::get() );
        }
        else if (aPropertyName == PROPERTYNAME_DECIMALS)
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet <<= (sal_Int16)( nDecimals );
        }
        else if (aPropertyName == PROPERTYNAME_LEADING)
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet <<= (sal_Int16)( nLeading );
        }
        else if (aPropertyName == PROPERTYNAME_NEGRED)
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet.setValue( &bRed, cppu::UnoType<bool>::get() );
        }
        else if (aPropertyName == PROPERTYNAME_THOUS)
        {
            pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
            aRet.setValue( &bThousand, cppu::UnoType<bool>::get() );
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
                aRet <<= OUString( pCurr->GetBankSymbol() );
            else
                aRet <<= OUString();
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

void SAL_CALL SvNumberFormatObj::addPropertyChangeListener( const OUString&,
                                                            const uno::Reference<beans::XPropertyChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::removePropertyChangeListener( const OUString&,
                                                               const uno::Reference<beans::XPropertyChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::addVetoableChangeListener( const OUString&,
                                                            const uno::Reference<beans::XVetoableChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatObj::removeVetoableChangeListener( const OUString&,
                                                               const uno::Reference<beans::XVetoableChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL SvNumberFormatObj::getPropertyValues()
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : nullptr;
    if (pFormat)
    {
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
        sal_Bool bStandard = ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
        //! Pass through SvNumberformat Member bStandard?
        sal_Bool bUserDef = ( ( pFormat->GetType() & css::util::NumberFormat::DEFINED ) != 0 );
        bool bThousand, bRed;
        sal_uInt16 nDecimals, nLeading;
        pFormat->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        lang::Locale aLocale( LanguageTag( pFormat->GetLanguage()).getLocale());

        uno::Sequence<beans::PropertyValue> aSeq(13);
        beans::PropertyValue* pArray = aSeq.getArray();

        pArray[0].Name = PROPERTYNAME_FMTSTR;
        pArray[0].Value <<= aFmtStr;
        pArray[1].Name = PROPERTYNAME_LOCALE;
        pArray[1].Value <<= aLocale;
        pArray[2].Name = PROPERTYNAME_TYPE;
        pArray[2].Value <<= (sal_Int16)( pFormat->GetType() );
        pArray[3].Name = PROPERTYNAME_COMMENT;
        pArray[3].Value <<= aComment;
        pArray[4].Name = PROPERTYNAME_STDFORM;
        pArray[4].Value.setValue( &bStandard, cppu::UnoType<bool>::get() );
        pArray[5].Name = PROPERTYNAME_USERDEF;
        pArray[5].Value.setValue( &bUserDef, cppu::UnoType<bool>::get() );
        pArray[6].Name = PROPERTYNAME_DECIMALS;
        pArray[6].Value <<= (sal_Int16)( nDecimals );
        pArray[7].Name = PROPERTYNAME_LEADING;
        pArray[7].Value <<= (sal_Int16)( nLeading );
        pArray[8].Name = PROPERTYNAME_NEGRED;
        pArray[8].Value.setValue( &bRed, cppu::UnoType<bool>::get() );
        pArray[9].Name = PROPERTYNAME_THOUS;
        pArray[9].Value.setValue( &bThousand, cppu::UnoType<bool>::get() );
        pArray[10].Name = PROPERTYNAME_CURRSYM;
        pArray[10].Value <<= aSymbol;
        pArray[11].Name = PROPERTYNAME_CURREXT;
        pArray[11].Value <<= aExt;
        pArray[12].Name = PROPERTYNAME_CURRABB;
        pArray[12].Value <<= aAbb;

        return aSeq;
    }
    else
        throw uno::RuntimeException();
}

void SAL_CALL SvNumberFormatObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
          lang::IllegalArgumentException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    throw beans::UnknownPropertyException(); // Everything is read-only
}

// XServiceInfo

OUString SAL_CALL SvNumberFormatObj::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("SvNumberFormatObj");
}

sal_Bool SAL_CALL SvNumberFormatObj::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatObj::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet { "com.sun.star.util.NumberFormatProperties" };
    return aRet;
}

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
    throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetNumberSettingsPropertyMap() );
    return aRef;
}

void SAL_CALL SvNumberFormatSettingsObj::setPropertyValue( const OUString& aPropertyName,
                                                           const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
          lang::IllegalArgumentException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        if (aPropertyName == PROPERTYNAME_NOZERO)
        {
            //  operator >>= shouldn't be used for bool (?)
            if ( aValue.getValueTypeClass() == uno::TypeClass_BOOLEAN )
                pFormatter->SetNoZero( *static_cast<sal_Bool const *>(aValue.getValue()) );
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
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL SvNumberFormatSettingsObj::getPropertyValue( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Any aRet;
    SvNumberFormatter* pFormatter = rSupplier.GetNumberFormatter();
    if (pFormatter)
    {
        if (aPropertyName == PROPERTYNAME_NOZERO)
        {
            sal_Bool bNoZero = pFormatter->GetNoZero();
            aRet.setValue( &bNoZero, cppu::UnoType<bool>::get() );
        }
        else if (aPropertyName == PROPERTYNAME_NULLDATE)
        {
            Date* pDate = pFormatter->GetNullDate();
            if (pDate)
            {
                aRet <<= pDate->GetUNODate();
            }
        }
        else if (aPropertyName == PROPERTYNAME_STDDEC)
            aRet <<= (sal_Int16)( pFormatter->GetStandardPrec() );
        else if (aPropertyName == PROPERTYNAME_TWODIGIT)
            aRet <<= (sal_Int16)( pFormatter->GetYear2000() );
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

void SAL_CALL SvNumberFormatSettingsObj::addPropertyChangeListener( const OUString&,
                                                                    const uno::Reference<beans::XPropertyChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removePropertyChangeListener( const OUString&,
                                                                       const uno::Reference<beans::XPropertyChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::addVetoableChangeListener( const OUString&,
                                                                    const uno::Reference<beans::XVetoableChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removeVetoableChangeListener( const OUString&,
                                                                       const uno::Reference<beans::XVetoableChangeListener>&)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

// XServiceInfo

OUString SAL_CALL SvNumberFormatSettingsObj::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("SvNumberFormatSettingsObj");
}

sal_Bool SAL_CALL SvNumberFormatSettingsObj::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence<OUString> SAL_CALL SvNumberFormatSettingsObj::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet { "com.sun.star.util.NumberFormatSettings" };
    return aRet;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_uno_util_numbers_SvNumberFormatterServiceObject_get_implementation(css::uno::XComponentContext*,
                                                                                css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvNumberFormatterServiceObj());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
