/*************************************************************************
 *
 *  $RCSfile: numfmuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include <vcl/svapp.hxx>
#include <vcl/color.hxx>
#include <tools/debug.hxx>
#include <tools/isolang.hxx>
#include <vos/mutex.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "numfmuno.hxx"
#include "numuno.hxx"
#include "zforlist.hxx"
#include "zformat.hxx"
#include "itemprop.hxx"

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

const SfxItemPropertyMap* lcl_GetNumberFormatPropertyMap()
{
    static SfxItemPropertyMap aNumberFormatPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(PROPERTYNAME_FMTSTR),   0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_LOCALE),   0, &getCppuType((lang::Locale*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_TYPE),     0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_COMMENT),  0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_CURREXT),  0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_CURRSYM),  0, &getCppuType((rtl::OUString*)0),beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_DECIMALS), 0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_LEADING),  0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_NEGRED),   0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_STDFORM),  0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_THOUS),    0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(PROPERTYNAME_USERDEF),  0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY},
        {0,0,0,0}
    };
    return aNumberFormatPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetNumberSettingsPropertyMap()
{
    static SfxItemPropertyMap aNumberSettingsPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(PROPERTYNAME_NOZERO),   0, &getBooleanCppuType(),         beans::PropertyAttribute::BOUND},
        {MAP_CHAR_LEN(PROPERTYNAME_NULLDATE), 0, &getCppuType((util::Date*)0),  beans::PropertyAttribute::BOUND},
        {MAP_CHAR_LEN(PROPERTYNAME_STDDEC),   0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND},
        {MAP_CHAR_LEN(PROPERTYNAME_TWODIGIT), 0, &getCppuType((sal_Int16*)0),   beans::PropertyAttribute::BOUND},
        {0,0,0,0}
    };
    return aNumberSettingsPropertyMap_Impl;
}

//----------------------------------------------------------------------------------------

LanguageType lcl_GetLanguage( const lang::Locale& rLocale )
{
    //  empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_SYSTEM;

    String aLangStr = rLocale.Language;
    String aCtryStr = rLocale.Country;
    //  Variant is ignored

    LanguageType eRet = ConvertIsoNamesToLanguage( aLangStr, aCtryStr );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM;         //! or throw an exception?

    return eRet;
}

void lcl_FillLocale( lang::Locale& rLocale, LanguageType eLang )
{
    String aLangStr, aCtryStr;
    ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );
    rLocale.Language = aLangStr;
    rLocale.Country  = aCtryStr;
}

//----------------------------------------------------------------------------------------

SvNumberFormatterServiceObj::SvNumberFormatterServiceObj() :
    pSupplier(NULL)
{
}

SvNumberFormatterServiceObj::~SvNumberFormatterServiceObj()
{
    if (pSupplier)
        pSupplier->release();
}

uno::Reference<uno::XInterface> SAL_CALL SvNumberFormatterServiceObj_NewInstance(
                    const uno::Reference<lang::XMultiServiceFactory>& rSMgr )
{
    return (cppu::OWeakObject*) new SvNumberFormatterServiceObj();
}

// XNumberFormatter

void SAL_CALL SvNumberFormatterServiceObj::attachNumberFormatsSupplier(
                            const uno::Reference<util::XNumberFormatsSupplier>& xSupplier )
                                    throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    SvNumberFormatsSupplierObj* pNew = SvNumberFormatsSupplierObj::getImplementation( xSupplier );
    if (!pNew)
        throw uno::RuntimeException();      // wrong object

    if (pNew)
        pNew->acquire();
    if (pSupplier)
        pSupplier->release();
    pSupplier = pNew;
}

uno::Reference<util::XNumberFormatsSupplier> SAL_CALL
                            SvNumberFormatterServiceObj::getNumberFormatsSupplier()
                                    throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
    return pSupplier;
}

sal_Int32 SAL_CALL SvNumberFormatterServiceObj::detectNumberFormat(
                                    sal_Int32 nKey, const rtl::OUString& aString )
                            throw(util::NotNumericException, uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aTemp = aString;
        ULONG nUKey = nKey;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    double fRet = 0.0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aTemp = aString;
        ULONG nUKey = nKey;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    String aRet;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        Color* pColor = NULL;
        pFormatter->GetOutputString(fValue, nKey, aRet, &pColor);
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

util::color SAL_CALL SvNumberFormatterServiceObj::queryColorForNumber( sal_Int32 nKey,
                                    double fValue, util::color aDefaultColor )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    util::color nRet = aDefaultColor;       // color = INT32
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    String aRet;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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

util::color SAL_CALL SvNumberFormatterServiceObj::queryColorForString( sal_Int32 nKey,
                                    const rtl::OUString& aString,util::color aDefaultColor )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    util::color nRet = aDefaultColor;       // color = INT32
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    String aRet;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    String aRet;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aOutString;
        String aFormString = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        Color* pColor = NULL;

        BOOL bOk;
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

util::color SAL_CALL SvNumberFormatterServiceObj::queryPreviewColorForNumber(
                                    const rtl::OUString& aFormat, double fValue,
                                    const lang::Locale& nLocale, sal_Bool bAllowEnglish,
                                    util::color aDefaultColor )
                            throw(util::MalformedNumberFormatException, uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    util::color nRet = aDefaultColor;       // color = INT32
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aOutString;
        String aFormString = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        Color* pColor = NULL;

        BOOL bOk;
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
    return rtl::OUString::createFromAscii("SvNumberFormatterServiceObj");
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
    pArray[0] = rtl::OUString::createFromAscii(SERVICENAME_NUMBERFORMATTER);
    return aRet;
}

//------------------------------------------------------------------------

SvNumberFormatsObj::SvNumberFormatsObj(SvNumberFormatsSupplierObj* pParent) :
    pSupplier(pParent)
{
    if (pSupplier)
        pSupplier->acquire();
}

SvNumberFormatsObj::~SvNumberFormatsObj()
{
    if (pSupplier)
        pSupplier->release();
}

// XNumberFormats

uno::Reference<beans::XPropertySet> SAL_CALL SvNumberFormatsObj::getByKey( sal_Int32 nKey )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : NULL;
    if (pFormat)
        return new SvNumberFormatObj( pSupplier, nKey );
    else
        throw uno::RuntimeException();

    return NULL;
}

uno::Sequence<sal_Int32> SAL_CALL SvNumberFormatsObj::queryKeys( sal_Int16 nType,
                                    const lang::Locale& nLocale, sal_Bool bCreate )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if ( pFormatter )
    {
        ULONG nIndex = 0;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        SvNumberFormatTable& rTable = bCreate ?
                                        pFormatter->ChangeCL( nType, nIndex, eLang ) :
                                        pFormatter->GetEntryTable( nType, nIndex, eLang );
        ULONG nCount = rTable.Count();
        uno::Sequence<sal_Int32> aSeq(nCount);
        sal_Int32* pAry = aSeq.getArray();
        for (ULONG i=0; i<nCount; i++)
            pAry[i] = rTable.GetObjectKey( i );

        return aSeq;
    }
    else
        throw uno::RuntimeException();

    return uno::Sequence<sal_Int32>(0);
}

sal_Int32 SAL_CALL SvNumberFormatsObj::queryKey( const rtl::OUString& aFormat,
                                    const lang::Locale& nLocale, sal_Bool bScan )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aFormStr = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        ULONG nKey = 0;
        xub_StrLen nCheckPos = 0;
        short nType = 0;
        BOOL bOk = pFormatter->PutEntry( aFormStr, nCheckPos, nType, nKey, eLang );
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aFormStr = aFormat;
        LanguageType eLang = lcl_GetLanguage( nLocale );
        LanguageType eNewLang = lcl_GetLanguage( nNewLocale );
        ULONG nKey = 0;
        xub_StrLen nCheckPos = 0;
        short nType = 0;
        BOOL bOk = pFormatter->PutandConvertEntry( aFormStr, nCheckPos, nType, nKey, eLang, eNewLang );
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

void SAL_CALL SvNumberFormatsObj::removeByKey( sal_Int32 nKey ) throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;

    if (pFormatter)
    {
        pFormatter->DeleteEntry(nKey);
        pSupplier->NumberFormatDeleted(nKey);       // Benachrichtigung fuers Dokument
    }
}

rtl::OUString SAL_CALL SvNumberFormatsObj::generateFormat( sal_Int32 nBaseKey,
                                    const lang::Locale& nLocale, sal_Bool bThousands,
                                    sal_Bool bRed, sal_Int16 nDecimals, sal_Int16 nLeading )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    String aRet;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        LanguageType eLang = lcl_GetLanguage( nLocale );
        pFormatter->GenerateFormat( aRet, nBaseKey, eLang, bThousands, bRed, nDecimals, nLeading );
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

// XNumberFormatTypes

sal_Int32 SAL_CALL SvNumberFormatsObj::getStandardIndex( const lang::Locale& nLocale )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    BOOL bRet = FALSE;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
        bRet = pFormatter->IsCompatible( nOldType, nNewType );
    else
        throw uno::RuntimeException();

    return bRet;
}

sal_Int32 SAL_CALL SvNumberFormatsObj::getFormatForLocale( sal_Int32 nKey, const lang::Locale& nLocale )
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    INT32 nRet = 0;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
    return rtl::OUString::createFromAscii("SvNumberFormatsObj");
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
    pArray[0] = rtl::OUString::createFromAscii(SERVICENAME_NUMBERFORMATS);
    return aRet;
}

//------------------------------------------------------------------------

SvNumberFormatObj::SvNumberFormatObj(SvNumberFormatsSupplierObj* pParent, ULONG nK) :
    pSupplier(pParent),
    nKey(nK)
{
    if (pSupplier)
        pSupplier->acquire();
}

SvNumberFormatObj::~SvNumberFormatObj()
{
    if (pSupplier)
        pSupplier->release();
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL SvNumberFormatObj::getPropertySetInfo()
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( lcl_GetNumberFormatPropertyMap() );
    return aRef;
}

void SAL_CALL SvNumberFormatObj::setPropertyValue( const rtl::OUString& aPropertyName,
                                                    const uno::Any& aValue )
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    uno::Any aRet;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : NULL;
    if (pFormat)
    {
        String aSymbol, aExt;
        pFormat->GetNewCurrencySymbol( aSymbol, aExt );
        String aFmtStr = pFormat->GetFormatstring();
        String aComment = ((SvNumberformat*)pFormat)->GetComment();
        //! warum ist GetComment nicht const ???
        BOOL bStandard = ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
        //! SvNumberformat Member bStandard rausreichen?
        BOOL bUserDef = ( ( pFormat->GetType() & NUMBERFORMAT_DEFINED ) != 0 );
        BOOL bThousand, bRed;
        USHORT nDecimals, nLeading;
        ((SvNumberformat*)pFormat)->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        //! warum ist GetFormatSpecialInfo nicht const ???

        String aString = aPropertyName;
        if (aString.EqualsAscii( PROPERTYNAME_FMTSTR ))         aRet <<= rtl::OUString( aFmtStr );
        else if (aString.EqualsAscii( PROPERTYNAME_LOCALE ))
        {
            lang::Locale aLocale;
            lcl_FillLocale( aLocale, pFormat->GetLanguage() );
            aRet <<= aLocale;
        }
        else if (aString.EqualsAscii( PROPERTYNAME_TYPE ))      aRet <<= (sal_Int16)( pFormat->GetType() );
        else if (aString.EqualsAscii( PROPERTYNAME_COMMENT ))   aRet <<= rtl::OUString( aComment );
        else if (aString.EqualsAscii( PROPERTYNAME_STDFORM ))   aRet.setValue( &bStandard, getBooleanCppuType() );
        else if (aString.EqualsAscii( PROPERTYNAME_USERDEF ))   aRet.setValue( &bUserDef, getBooleanCppuType() );
        else if (aString.EqualsAscii( PROPERTYNAME_DECIMALS ))  aRet <<= (sal_Int16)( nDecimals );
        else if (aString.EqualsAscii( PROPERTYNAME_LEADING ))   aRet <<= (sal_Int16)( nLeading );
        else if (aString.EqualsAscii( PROPERTYNAME_NEGRED ))    aRet.setValue( &bRed, getBooleanCppuType() );
        else if (aString.EqualsAscii( PROPERTYNAME_THOUS ))     aRet.setValue( &bThousand, getBooleanCppuType() );
        else if (aString.EqualsAscii( PROPERTYNAME_CURRSYM ))   aRet <<= rtl::OUString( aSymbol );
        else if (aString.EqualsAscii( PROPERTYNAME_CURREXT ))   aRet <<= rtl::OUString( aExt );
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
    DBG_ERROR("not implemented");
}

void SAL_CALL SvNumberFormatObj::removePropertyChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL SvNumberFormatObj::addVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL SvNumberFormatObj::removeVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL SvNumberFormatObj::getPropertyValues()
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    const SvNumberformat* pFormat = pFormatter ? pFormatter->GetEntry(nKey) : NULL;
    if (pFormat)
    {
        String aSymbol, aExt;
        pFormat->GetNewCurrencySymbol( aSymbol, aExt );
        String aFmtStr = pFormat->GetFormatstring();
        String aComment = ((SvNumberformat*)pFormat)->GetComment();
        //! warum ist GetComment nicht const ???
        BOOL bStandard = ( ( nKey % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 );
        //! SvNumberformat Member bStandard rausreichen?
        BOOL bUserDef = ( ( pFormat->GetType() & NUMBERFORMAT_DEFINED ) != 0 );
        BOOL bThousand, bRed;
        USHORT nDecimals, nLeading;
        ((SvNumberformat*)pFormat)->GetFormatSpecialInfo( bThousand, bRed, nDecimals, nLeading );
        //! warum ist GetFormatSpecialInfo nicht const ???
        lang::Locale aLocale;
        lcl_FillLocale( aLocale, pFormat->GetLanguage() );

        uno::Sequence<beans::PropertyValue> aSeq(12);
        beans::PropertyValue* pArray = aSeq.getArray();

        pArray[0].Name = rtl::OUString::createFromAscii( PROPERTYNAME_FMTSTR );
        pArray[0].Value <<= rtl::OUString( aFmtStr );
        pArray[1].Name = rtl::OUString::createFromAscii( PROPERTYNAME_LOCALE );
        pArray[1].Value <<= aLocale;
        pArray[2].Name = rtl::OUString::createFromAscii( PROPERTYNAME_TYPE );
        pArray[2].Value <<= (sal_Int16)( pFormat->GetType() );
        pArray[3].Name = rtl::OUString::createFromAscii( PROPERTYNAME_COMMENT );
        pArray[3].Value <<= rtl::OUString( aComment );
        pArray[4].Name = rtl::OUString::createFromAscii( PROPERTYNAME_STDFORM );
        pArray[4].Value.setValue( &bStandard, getBooleanCppuType() );
        pArray[5].Name = rtl::OUString::createFromAscii( PROPERTYNAME_USERDEF );
        pArray[5].Value.setValue( &bUserDef, getBooleanCppuType() );
        pArray[6].Name = rtl::OUString::createFromAscii( PROPERTYNAME_DECIMALS );
        pArray[6].Value <<= (sal_Int16)( nDecimals );
        pArray[7].Name = rtl::OUString::createFromAscii( PROPERTYNAME_LEADING );
        pArray[7].Value <<= (sal_Int16)( nLeading );
        pArray[8].Name = rtl::OUString::createFromAscii( PROPERTYNAME_NEGRED );
        pArray[8].Value.setValue( &bRed, getBooleanCppuType() );
        pArray[9].Name = rtl::OUString::createFromAscii( PROPERTYNAME_THOUS );
        pArray[9].Value.setValue( &bThousand, getBooleanCppuType() );
        pArray[10].Name = rtl::OUString::createFromAscii( PROPERTYNAME_CURRSYM );
        pArray[10].Value <<= rtl::OUString( aSymbol );
        pArray[11].Name = rtl::OUString::createFromAscii( PROPERTYNAME_CURREXT );
        pArray[11].Value <<= rtl::OUString( aExt );

        return aSeq;
    }
    else
        throw uno::RuntimeException();

    return uno::Sequence<beans::PropertyValue>(0);
}

void SAL_CALL SvNumberFormatObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& aProps )
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
    return rtl::OUString::createFromAscii("SvNumberFormatObj");
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
    pArray[0] = rtl::OUString::createFromAscii(SERVICENAME_NUMBERFORMAT);
    return aRet;
}

//------------------------------------------------------------------------

SvNumberFormatSettingsObj::SvNumberFormatSettingsObj(SvNumberFormatsSupplierObj* pParent) :
    pSupplier(pParent)
{
    if (pSupplier)
        pSupplier->acquire();
}

SvNumberFormatSettingsObj::~SvNumberFormatSettingsObj()
{
    if (pSupplier)
        pSupplier->release();
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL SvNumberFormatSettingsObj::getPropertySetInfo()
                            throw(uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());
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
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
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
            sal_Int16 nInt16;
            if ( aValue >>= nInt16 )
                pFormatter->ChangeStandardPrec( nInt16 );
        }
        else if (aString.EqualsAscii( PROPERTYNAME_TWODIGIT ))
        {
            sal_Int16 nInt16;
            if ( aValue >>= nInt16 )
                pFormatter->SetYear2000( nInt16 );
        }
        else
            throw beans::UnknownPropertyException();

        pSupplier->SettingsChanged();
    }
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL SvNumberFormatSettingsObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    NAMESPACE_VOS(OGuard) aGuard(Application::GetSolarMutex());

    uno::Any aRet;
    SvNumberFormatter* pFormatter = pSupplier ? pSupplier->GetNumberFormatter() : NULL;
    if (pFormatter)
    {
        String aString = aPropertyName;
        if (aString.EqualsAscii( PROPERTYNAME_NOZERO ))
        {
            BOOL bNoZero = pFormatter->GetNoZero();
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
    DBG_ERROR("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removePropertyChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::addVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL SvNumberFormatSettingsObj::removeVetoableChangeListener( const rtl::OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
                        throw(beans::UnknownPropertyException,
                        lang::WrappedTargetException, uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

// XServiceInfo

rtl::OUString SAL_CALL SvNumberFormatSettingsObj::getImplementationName()
                            throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii("SvNumberFormatSettingsObj");
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
    pArray[0] = rtl::OUString::createFromAscii(SERVICENAME_NUMBERSETTINGS);
    return aRet;
}


