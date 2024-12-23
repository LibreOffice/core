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

#include <sal/config.h>

#include <cstdlib>
#include <string_view>

#include <config_features.h>

#include <comphelper/errcode.hxx>
#include <unotools/resmgr.hxx>
#include "sbxconv.hxx"

#include <unotools/syslocale.hxx>
#include <unotools/charclass.hxx>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <math.h>

#include <sbxbase.hxx>
#include <sbintern.hxx>
#include <sbxform.hxx>

#include <date.hxx>
#include <runtime.hxx>
#include <strings.hrc>

#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <o3tl/temporary.hxx>
#include <o3tl/string_view.hxx>
#include <officecfg/Office/Scripting.hxx>


void ImpGetIntntlSep( sal_Unicode& rcDecimalSep, sal_Unicode& rcThousandSep, sal_Unicode& rcDecimalSepAlt )
{
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
    rcDecimalSep = rData.getNumDecimalSep()[0];
    rcThousandSep = rData.getNumThousandSep()[0];
    rcDecimalSepAlt = rData.getNumDecimalSepAlt().toChar();
}


static bool ImpStrChr( std::u16string_view str, sal_Unicode c ) { return str.find(c) != std::u16string_view::npos; }


// scanning a string according to BASIC-conventions
// but exponent may also be a D, so data type is SbxDOUBLE
// conversion error if data type is fixed and it doesn't fit

ErrCode ImpScan( const OUString& rWSrc, double& nVal, SbxDataType& rType,
                  sal_uInt16* pLen, bool bOnlyIntntl )
{
    sal_Unicode cIntntlDecSep, cIntntlGrpSep, cIntntlDecSepAlt;
    sal_Unicode cNonIntntlDecSep = '.';
    if( bOnlyIntntl )
    {
        ImpGetIntntlSep( cIntntlDecSep, cIntntlGrpSep, cIntntlDecSepAlt );
        cNonIntntlDecSep = cIntntlDecSep;
        // Ensure that the decimal separator alternative is really one.
        if (cIntntlDecSepAlt && cIntntlDecSepAlt == cNonIntntlDecSep)
            cIntntlDecSepAlt = 0;
    }
    else
    {
        cIntntlDecSep = cNonIntntlDecSep;
        cIntntlGrpSep = 0;  // no group separator accepted in non-i18n
        cIntntlDecSepAlt = 0;
    }

    const sal_Unicode* const pStart = rWSrc.getStr();
    const sal_Unicode* p = pStart;
    OUStringBuffer aBuf( rWSrc.getLength());
    bool bRes = true;
    bool bMinus = false;
    nVal = 0;
    SbxDataType eScanType = SbxSINGLE;
    while( *p == ' ' || *p == '\t' )
        p++;
    if (*p == '+')
        p++;
    else if( *p == '-' )
    {
        p++;
        bMinus = true;
    }
    if( rtl::isAsciiDigit( *p ) || ((*p == cNonIntntlDecSep || *p == cIntntlDecSep ||
                    (cIntntlGrpSep && *p == cIntntlGrpSep) || (cIntntlDecSepAlt && *p == cIntntlDecSepAlt)) &&
                rtl::isAsciiDigit( *(p+1) )))
    {
        // tdf#118442: Whitespace and minus are skipped; store the position to calculate index
        const sal_Unicode* const pDigitsStart = p;
        short exp = 0;
        short decsep = 0;
        short ndig = 0;
        short ncdig = 0;    // number of digits after decimal point
        OUStringBuffer aSearchStr(OUString::Concat("0123456789DEde") + OUStringChar(cNonIntntlDecSep));
        if( cIntntlDecSep != cNonIntntlDecSep )
            aSearchStr.append(cIntntlDecSep);
        if( cIntntlDecSepAlt && cIntntlDecSepAlt != cNonIntntlDecSep )
            aSearchStr.append(cIntntlDecSepAlt);
        if( bOnlyIntntl )
            aSearchStr.append(cIntntlGrpSep);
        const OUString pSearchStr = aSearchStr.makeStringAndClear();
        static constexpr OUStringLiteral pDdEe = u"DdEe";
        while( ImpStrChr( pSearchStr, *p ) )
        {
            aBuf.append( *p );
            if( bOnlyIntntl && *p == cIntntlGrpSep )
            {
                p++;
                continue;
            }
            if( *p == cNonIntntlDecSep || *p == cIntntlDecSep || (cIntntlDecSepAlt && *p == cIntntlDecSepAlt) )
            {
                // Use the separator that is passed to stringToDouble()
                aBuf[p - pDigitsStart] = cIntntlDecSep;
                p++;
                if( ++decsep > 1 )
                    continue;
            }
            else if( ImpStrChr( pDdEe, *p ) )
            {
                if( ++exp > 1 )
                {
                    p++;
                    continue;
                }
                if( *p == 'D' || *p == 'd' )
                    eScanType = SbxDOUBLE;
                aBuf[p - pDigitsStart] = 'E';
                p++;
                if (*p == '+')
                    ++p;
                else if (*p == '-')
                {
                    aBuf.append('-');
                    ++p;
                }
            }
            else
            {
                p++;
                if( decsep && !exp )
                    ncdig++;
            }
            if( !exp )
                ndig++;
        }

        if( decsep > 1 || exp > 1 )
            bRes = false;

        rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
        sal_Int32 nParseEnd = 0;
        nVal = rtl::math::stringToDouble( aBuf, cIntntlDecSep, cIntntlGrpSep, &eStatus, &nParseEnd );
        if( eStatus != rtl_math_ConversionStatus_Ok || nParseEnd != aBuf.getLength() )
            bRes = false;

        if( !decsep && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                eScanType = SbxLONG;
        }

        ndig = ndig - decsep;
        // too many numbers for SINGLE?
        if( ndig > 15 || ncdig > 6 )
            eScanType = SbxDOUBLE;

        // type detection?
        static constexpr OUStringLiteral pTypes = u"%!&#";
        if( ImpStrChr( pTypes, *p ) )
            p++;
    }
    // hex/octal number? read in and convert:
    else if( *p == '&' )
    {
        p++;
        eScanType = SbxLONG;
        OUString aCmp( u"0123456789ABCDEF"_ustr );
        char base = 16;
        char ndig = 8;
        switch( *p++ )
        {
            case 'O':
            case 'o':
                aCmp = "01234567";
                base = 8;
                ndig = 11;
                break;
            case 'H':
            case 'h':
                break;
            default :
                bRes = false;
        }
        while( rtl::isAsciiAlphanumeric( *p ) )    /* XXX: really munge all alnum also when error? */
        {
            sal_Unicode ch = rtl::toAsciiUpperCase(*p);
            if( ImpStrChr( aCmp, ch ) )
                aBuf.append( ch );
            else
                bRes = false;
            p++;
        }
        OUString aBufStr( aBuf.makeStringAndClear());
        sal_Int32 l = 0;
        for( const sal_Unicode* q = aBufStr.getStr(); bRes && *q; q++ )
        {
            int i = *q - '0';
            if( i > 9 )
                i -= 7;     // 'A'-'0' = 17 => 10, ...
            l = ( l * base ) + i;
            if( !ndig-- )
                bRes = false;
        }
        if( *p == '&' )
            p++;
        nVal = static_cast<double>(l);
        if( l >= SbxMININT && l <= SbxMAXINT )
            eScanType = SbxINTEGER;
    }
#if HAVE_FEATURE_SCRIPTING
    else if ( SbiRuntime::isVBAEnabled() )
    {
        return ERRCODE_BASIC_CONVERSION;
    }
#endif
    // tdf#146672 - skip whitespaces and tabs at the end of the scanned string
    while (*p == ' ' || *p == '\t')
        p++;
    if( pLen )
        *pLen = static_cast<sal_uInt16>( p - pStart );
    if( !bRes )
        return ERRCODE_BASIC_CONVERSION;
    if( bMinus )
        nVal = -nVal;
    rType = eScanType;
    return ERRCODE_NONE;
}

ErrCode ImpScan(const OUString& rSrc, double& nVal, SbxDataType& rType, sal_uInt16* pLen)
{
    using namespace officecfg::Office::Scripting;
    static const bool bEnv = std::getenv("LIBREOFFICE6FLOATINGPOINTMODE") != nullptr;
    bool bMode = bEnv || Basic::Compatibility::UseLibreOffice6FloatingPointConversion::get();

    return ImpScan(rSrc, nVal, rType, pLen, !bMode);
}

// port for CDbl in the Basic
ErrCode SbxValue::ScanNumIntnl( const OUString& rSrc, double& nVal, bool bSingle )
{
    SbxDataType t;
    sal_uInt16 nLen = 0;
    ErrCode nRetError = ImpScan( rSrc, nVal, t, &nLen,
        /*bOnlyIntntl*/true );
    // read completely?
    if( nRetError == ERRCODE_NONE && nLen != rSrc.getLength() )
    {
        nRetError = ERRCODE_BASIC_CONVERSION;
    }
    if( bSingle )
    {
        SbxValues aValues( nVal );
        nVal = static_cast<double>(ImpGetSingle( &aValues ));    // here error at overflow
    }
    return nRetError;
}

// The number is prepared unformattedly with the given number of
// NK-positions. A leading minus is added if applicable.
// This routine is public because it's also used by the Put-functions
// in the class SbxImpSTRING.

void ImpCvtNum( double nNum, short nPrec, OUString& rRes, bool bCoreString )
{
    sal_Unicode cDecimalSep;
    if( bCoreString )
        cDecimalSep = '.';
    else
        ImpGetIntntlSep(cDecimalSep, o3tl::temporary(sal_Unicode()), o3tl::temporary(sal_Unicode()));

    // tdf#143575 - use rtl::math::doubleToUString to convert numbers to strings in basic
    rRes = rtl::math::doubleToUString(nNum, rtl_math_StringFormat_Automatic, nPrec, cDecimalSep, true);
}

bool ImpConvStringExt( OUString& rSrc, SbxDataType eTargetType )
{
    bool bChanged = false;
    OUString aNewString;

    // only special cases are handled, nothing on default
    switch( eTargetType )
    {
        // consider international for floating point
        case SbxSINGLE:
        case SbxDOUBLE:
        case SbxCURRENCY:
        {
            sal_Unicode cDecimalSep, cThousandSep, cDecimalSepAlt;
            ImpGetIntntlSep( cDecimalSep, cThousandSep, cDecimalSepAlt );
            aNewString = rSrc;

            if( cDecimalSep != '.' || (cDecimalSepAlt && cDecimalSepAlt != '.') )
            {
                sal_Int32 nPos = aNewString.indexOf( cDecimalSep );
                if( nPos == -1 && cDecimalSepAlt )
                    nPos = aNewString.indexOf( cDecimalSepAlt );
                if( nPos != -1 )
                {
                    sal_Unicode* pStr = const_cast<sal_Unicode*>(aNewString.getStr());
                    pStr[nPos] = '.';
                    bChanged = true;
                }
            }
            break;
        }

        // check as string in case of sal_Bool sal_True and sal_False
        case SbxBOOL:
        {
            if( rSrc.equalsIgnoreAsciiCase("true") )
            {
                aNewString = OUString::number( SbxTRUE );
                bChanged = true;
            }
            else if( rSrc.equalsIgnoreAsciiCase("false") )
            {
                aNewString = OUString::number( SbxFALSE );
                bChanged = true;
            }
            break;
        }
        default: break;
    }

    if( bChanged )
        rSrc = aNewString;
    return bChanged;
}


// formatted number output
// the return value is the number of characters used
// from the format

static void printfmtstr(std::u16string_view rStr, OUString& rRes, std::u16string_view rFmt)
{
    if (rFmt.empty())
        rFmt = u"&";

    OUStringBuffer aTemp;
    auto pStr = rStr.begin();
    auto pFmt = rFmt.begin();

    switch( *pFmt )
    {
    case '!':
        if (pStr != rStr.end())
            aTemp.append(*pStr);
        break;
    case '\\':
        do
        {
            aTemp.append(pStr != rStr.end() ? *pStr++ : u' ');
        } while (++pFmt != rFmt.end() && *pFmt != '\\');
        aTemp.append(pStr != rStr.end() ? *pStr : u' ');
        break;
    case '&':
    default:
        aTemp = rStr;
        break;
    }
    rRes = aTemp.makeStringAndClear();
}


bool SbxValue::Scan( const OUString& rSrc, sal_uInt16* pLen )
{
    ErrCode eRes = ERRCODE_NONE;
    if( !CanWrite() )
    {
        eRes = ERRCODE_BASIC_PROP_READONLY;
    }
    else
    {
        double n;
        SbxDataType t;
        eRes = ImpScan( rSrc, n, t, pLen );
        if( eRes == ERRCODE_NONE )
        {
            if( !IsFixed() )
            {
                SetType( t );
            }
            PutDouble( n );
        }
    }
    if( eRes )
    {
        SetError( eRes );
        return false;
    }
    else
    {
        return true;
    }
}

std::locale BasResLocale()
{
    return Translate::Create("sb");
}

OUString BasResId(TranslateId aId)
{
    return Translate::get(aId, BasResLocale());
}

namespace
{

enum class VbaFormatType
{
    Offset,      // standard number format
    UserDefined, // user defined number format
};

#if HAVE_FEATURE_SCRIPTING

struct VbaFormatInfo
{
    VbaFormatType meType;
    std::u16string_view mpVbaFormat; // Format string in vba
    NfIndexTableOffset meOffset; // SvNumberFormatter format index, if meType = VbaFormatType::Offset
    OUString mpOOoFormat;     // if meType = VbaFormatType::UserDefined
};

const VbaFormatInfo* getFormatInfo( std::u16string_view rFmt )
{
    static constexpr const VbaFormatInfo formatInfoTable[] =
    {
        { VbaFormatType::Offset,      u"Long Date",   NF_DATE_SYSTEM_LONG,    u""_ustr },
        { VbaFormatType::UserDefined, u"Medium Date", NF_NUMBER_STANDARD,     u"DD-MMM-YY"_ustr },
        { VbaFormatType::Offset,      u"Short Date",  NF_DATE_SYSTEM_SHORT,   u""_ustr },
        { VbaFormatType::UserDefined, u"Long Time",   NF_NUMBER_STANDARD,     u"H:MM:SS AM/PM"_ustr },
        { VbaFormatType::Offset,      u"Medium Time", NF_TIME_HHMMAMPM,       u""_ustr },
        { VbaFormatType::Offset,      u"Short Time",  NF_TIME_HHMM,           u""_ustr },
        { VbaFormatType::Offset,      u"ddddd",       NF_DATE_SYSTEM_SHORT,   u""_ustr },
        { VbaFormatType::Offset,      u"dddddd",      NF_DATE_SYSTEM_LONG,    u""_ustr },
        { VbaFormatType::UserDefined, u"ttttt",       NF_NUMBER_STANDARD,     u"H:MM:SS AM/PM"_ustr },
        { VbaFormatType::Offset,      u"ww",          NF_DATE_WW,             u""_ustr },
    };

    for (auto& info : formatInfoTable)
        if (o3tl::equalsIgnoreAsciiCase(rFmt, info.mpVbaFormat))
            return &info;
    return nullptr;
}
#endif

void BasicFormatNum(double d, const OUString& rFmt, OUString& rRes)
{
    SbxAppData& rAppData = GetSbxData_Impl();

    LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
    if (rAppData.pBasicFormater)
        if (rAppData.eBasicFormaterLangType != eLangType)
            rAppData.pBasicFormater.reset();
    rAppData.eBasicFormaterLangType = eLangType;

    if (!rAppData.pBasicFormater)
    {
        SvtSysLocale aSysLocale;
        const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
        sal_Unicode cComma = rData.getNumDecimalSep()[0];
        sal_Unicode c1000 = rData.getNumThousandSep()[0];
        const OUString& aCurrencyStrg = rData.getCurrSymbol();

        // initialize the Basic-formater help object:
        // get resources for predefined output
        // of the Format()-command, e. g. for "On/Off"
        OUString aOnStrg = BasResId(STR_BASICKEY_FORMAT_ON);
        OUString aOffStrg = BasResId(STR_BASICKEY_FORMAT_OFF);
        OUString aYesStrg = BasResId(STR_BASICKEY_FORMAT_YES);
        OUString aNoStrg = BasResId(STR_BASICKEY_FORMAT_NO);
        OUString aTrueStrg = BasResId(STR_BASICKEY_FORMAT_TRUE);
        OUString aFalseStrg = BasResId(STR_BASICKEY_FORMAT_FALSE);
        OUString aCurrencyFormatStrg = BasResId(STR_BASICKEY_FORMAT_CURRENCY);

        rAppData.pBasicFormater = std::make_unique<SbxBasicFormater>(
            cComma, c1000, aOnStrg, aOffStrg, aYesStrg, aNoStrg, aTrueStrg, aFalseStrg,
            aCurrencyStrg, aCurrencyFormatStrg);
    }
    // Remark: For performance reasons there's only ONE BasicFormater-
    //    object created and 'stored', so that the expensive resource-
    //    loading is saved (for country-specific predefined outputs,
    //    e. g. "On/Off") and the continuous string-creation
    //    operations, too.
    // BUT: therefore this code is NOT multithreading capable!
    rRes = rAppData.pBasicFormater->BasicFormat(d, rFmt);
}

void BasicFormatNum(double d, const OUString* pFmt, SbxDataType eType, OUString& rRes)
{
    if (pFmt)
        BasicFormatNum(d, *pFmt, rRes);
    else
        ImpCvtNum(d, eType == SbxSINGLE ? 6 : eType == SbxDOUBLE ? 14 : 0, rRes);
}

#if HAVE_FEATURE_SCRIPTING
// For numeric types, takes the number directly; otherwise, tries to take string and convert it
bool GetNumberIntl(const SbxValue& val, double& ret)
{
    switch (val.GetType())
    {
        case SbxCHAR:
        case SbxBYTE:
        case SbxINTEGER:
        case SbxUSHORT:
        case SbxLONG:
        case SbxULONG:
        case SbxINT:
        case SbxUINT:
        case SbxSINGLE:
        case SbxDOUBLE:
        case SbxDATE:
            ret = val.GetDouble();
            return true;
        case SbxSTRING:
        default:
            return SbxValue::ScanNumIntnl(val.GetOUString(), ret) == ERRCODE_NONE;
    }
}
#endif
} // namespace

void SbxValue::Format( OUString& rRes, const OUString* pFmt ) const
{
    if (pFmt)
    {
        if (*pFmt == "<") // VBA lowercase
        {
            rRes = SvtSysLocale().GetCharClass().lowercase(GetOUString());
            return;
        }
        if (*pFmt == ">") // VBA uppercase
        {
            rRes = SvtSysLocale().GetCharClass().uppercase(GetOUString());
            return;
        }

    // pflin, It is better to use SvNumberFormatter to handle the date/time/number format.
    // the SvNumberFormatter output is mostly compatible with
    // VBA output besides the OOo-basic output
#if HAVE_FEATURE_SCRIPTING
        // number format, use SvNumberFormatter to handle it.
        if (double nNumber; !SbxBasicFormater::isBasicFormat(*pFmt) && GetNumberIntl(*this, nNumber))
        {
            LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
            std::shared_ptr<SvNumberFormatter> pFormatter;
            if (GetSbData()->pInst)
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
            }
            else
            {
                sal_uInt32 n; // Dummy
                pFormatter = SbiInstance::PrepareNumberFormatter(n, n, n);
            }

            sal_uInt32 nIndex;
            const Color* pCol;
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType;
            OUString aFmtStr = *pFmt;
            if (const VbaFormatInfo* pInfo = getFormatInfo(aFmtStr))
            {
                if( pInfo->meType == VbaFormatType::Offset )
                {
                    nIndex = pFormatter->GetFormatIndex( pInfo->meOffset, eLangType );
                }
                else
                {
                    aFmtStr = pInfo->mpOOoFormat;
                    pFormatter->PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH_US, eLangType, true);
                }
                pFormatter->GetOutputString( nNumber, nIndex, rRes, &pCol );
            }
            else if (aFmtStr.equalsIgnoreAsciiCase("General Date") // VBA general date variants
                     || aFmtStr.equalsIgnoreAsciiCase("c"))
            {
                OUString dateStr;
                if( nNumber <=-1.0 || nNumber >= 1.0 )
                {
                    // short date
                    nIndex = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLangType );
                    pFormatter->GetOutputString(nNumber, nIndex, dateStr, &pCol);

                    if (floor(nNumber) == nNumber)
                    {
                        rRes = dateStr;
                        return;
                    }
                }
                // long time
                aFmtStr = u"H:MM:SS AM/PM"_ustr;
                pFormatter->PutandConvertEntry(aFmtStr, nCheckPos, nType, nIndex,
                                               LANGUAGE_ENGLISH_US, eLangType, true);
                pFormatter->GetOutputString(nNumber, nIndex, rRes, &pCol);
                if (!dateStr.isEmpty())
                    rRes = dateStr + " " + rRes;
            }
            else if (aFmtStr.equalsIgnoreAsciiCase("n") // VBA minute variants
                     || aFmtStr.equalsIgnoreAsciiCase("nn"))
            {
                sal_Int32 nMin = implGetMinute( nNumber );
                if (nMin < 10 && aFmtStr.equalsIgnoreAsciiCase("nn"))
                {
                    // Minute in two digits
                     sal_Unicode aBuf[2];
                     aBuf[0] = '0';
                     aBuf[1] = '0' + nMin;
                     rRes = OUString(aBuf, std::size(aBuf));
                }
                else
                {
                    rRes = OUString::number(nMin);
                }
            }
            else if (aFmtStr.equalsIgnoreAsciiCase("w")) // VBA weekday number
            {
                rRes = OUString::number(implGetWeekDay(nNumber));
            }
            else if (aFmtStr.equalsIgnoreAsciiCase("y")) // VBA year day number
            {
                sal_Int16 nYear = implGetDateYear( nNumber );
                double dBaseDate;
                implDateSerial( nYear, 1, 1, true, SbDateCorrection::None, dBaseDate );
                sal_Int32 nYear32 = 1 + sal_Int32( nNumber - dBaseDate );
                rRes = OUString::number(nYear32);
            }
            else
            {
                pFormatter->PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH_US, eLangType, true);
                pFormatter->GetOutputString( nNumber, nIndex, rRes, &pCol );
            }

            return;
        }
#endif
    }

    SbxDataType eType = GetType();
    switch( eType )
    {
    case SbxNULL:
        rRes = SbxBasicFormater::BasicFormatNull(pFmt ? *pFmt : std::u16string_view{});
        break;
    case SbxCHAR:
    case SbxBYTE:
    case SbxINTEGER:
    case SbxUSHORT:
    case SbxLONG:
    case SbxULONG:
    case SbxINT:
    case SbxUINT:
    case SbxSINGLE:
    case SbxDOUBLE:
        BasicFormatNum(GetDouble(), pFmt, eType, rRes);
        break;
    case SbxSTRING:
        rRes = GetOUString();
        if( pFmt )
        {
            // #45355 converting if numeric
            if (double d; ScanNumIntnl(rRes, d) == ERRCODE_NONE)
                BasicFormatNum(d, *pFmt, rRes);
            else
                printfmtstr(rRes, rRes, *pFmt);
        }
        break;
    default:
        rRes = GetOUString();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
