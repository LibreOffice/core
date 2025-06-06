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
#include <optional>
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

ErrCode ImpScan( std::u16string_view rWSrc, double& nVal, SbxDataType& rType,
                 sal_Int32* pLen, bool* pHasNumber, bool bOnlyIntntl )
{
    sal_Unicode cDecSep, cGrpSep, cDecSepAlt;
    if( bOnlyIntntl )
    {
        ImpGetIntntlSep(cDecSep, cGrpSep, cDecSepAlt);
        // Ensure that the decimal separator alternative is really one.
        if (cDecSepAlt == cDecSep)
            cDecSepAlt = 0;
    }
    else
    {
        cDecSep = '.';
        cGrpSep = 0;  // no group separator accepted in non-i18n
        cDecSepAlt = 0;
    }

    auto const pStart = rWSrc.begin();
    auto p = pStart;
    bool bMinus = false;
    nVal = 0;
    SbxDataType eScanType = SbxSINGLE;
    while (p != rWSrc.end() && (*p == ' ' || *p == '\t'))
        p++;
    if (p != rWSrc.end() && *p == '+')
        p++;
    else if (p != rWSrc.end() && *p == '-')
    {
        p++;
        bMinus = true;
    }
#if HAVE_FEATURE_SCRIPTING
    if (SbiRuntime::isVBAEnabled())
    {
        while (p != rWSrc.end() && (*p == ' ' || *p == '\t'))
            p++;
    }
#endif
    const auto pNumberStart = p;
    if (p != rWSrc.end()
        && (rtl::isAsciiDigit(*p)
            || ((*p == cDecSep || (cGrpSep && *p == cGrpSep) || (cDecSepAlt && *p == cDecSepAlt))
                && p + 1 != rWSrc.end() && rtl::isAsciiDigit(*(p + 1)))))
    {
        bool exp = false;
        bool decsep = false;
        short ndig = 0;
        short ncdig = 0;    // number of digits after decimal point
        OUStringBuffer aSearchStr("0123456789DEde" + OUStringChar(cDecSep));
        if (cDecSepAlt)
            aSearchStr.append(cDecSepAlt);
        if (cGrpSep)
            aSearchStr.append(cGrpSep);
        OUStringBuffer aBuf(rWSrc.end() - p);
        for (; p != rWSrc.end() && ImpStrChr(aSearchStr, *p); ++p)
        {
            if (rtl::isAsciiDigit(*p))
            {
                aBuf.append(*p);
                if (!exp)
                {
                    ndig++;
                    if (decsep)
                        ncdig++;
                }
            }
            else if (cGrpSep && *p == cGrpSep)
            {
                aBuf.append(*p);
            }
            else if (*p == cDecSep || (cDecSepAlt && *p == cDecSepAlt))
            {
                if (decsep)
                    return ERRCODE_BASIC_CONVERSION;
                decsep = true;

                // Use the separator that is passed to stringToDouble()
                aBuf.append(cDecSep);
            }
            else // DdEe
            {
                if (exp)
                    return ERRCODE_BASIC_CONVERSION;
                exp = true;

                if( *p == 'D' || *p == 'd' )
                    eScanType = SbxDOUBLE;
                aBuf.append('E');
                if (auto pNext = p + 1; pNext != rWSrc.end())
                {
                    if (*pNext == '+')
                        ++p;
                    else if (*pNext == '-')
                    {
                        aBuf.append('-');
                        ++p;
                    }
                }
            }
        }

        rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
        sal_Int32 nParseEnd = 0;
        nVal = rtl::math::stringToDouble(aBuf, cDecSep, cGrpSep, &eStatus, &nParseEnd);
        if( eStatus != rtl_math_ConversionStatus_Ok || nParseEnd != aBuf.getLength() )
            return ERRCODE_BASIC_CONVERSION;

        if( !decsep && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                eScanType = SbxLONG;
        }

        // too many numbers for SINGLE?
        if( ndig > 15 || ncdig > 6 )
            eScanType = SbxDOUBLE;

        // type detection?
        static constexpr std::u16string_view pTypes = u"%!&#";
        if (p != rWSrc.end() && ImpStrChr(pTypes, *p))
            p++;
    }
    // hex/octal number? read in and convert:
    else if (p != rWSrc.end() && *p == '&')
    {
        if (++p == rWSrc.end())
            return ERRCODE_BASIC_CONVERSION;
        eScanType = SbxLONG;
        auto isValidDigit = rtl::isAsciiHexDigit<sal_Unicode>;
        char base = 16;
        char ndig = 8;
        switch( *p++ )
        {
            case 'O':
            case 'o':
                isValidDigit = rtl::isAsciiOctalDigit<sal_Unicode>;
                base = 8;
                ndig = 11;
                break;
            case 'H':
            case 'h':
                break;
            default :
                return ERRCODE_BASIC_CONVERSION;
        }
        const auto pDigitsStart = p;
        for (; p != rWSrc.end() && rtl::isAsciiAlphanumeric(*p); ++p)
        {
            if (!isValidDigit(*p))
                return ERRCODE_BASIC_CONVERSION;
        }
        if (p - pDigitsStart > ndig)
                return ERRCODE_BASIC_CONVERSION;
        sal_Int32 l = o3tl::toInt32(rWSrc.substr(pDigitsStart - pStart, p - pDigitsStart), base);
        if (p != rWSrc.end() && *p == '&')
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
    const auto pNumberEnd = p;
    // tdf#146672 - skip whitespaces and tabs at the end of the scanned string
    while (p != rWSrc.end() && (*p == ' ' || *p == '\t'))
        p++;
    if( pLen )
        *pLen = p - pStart;
    if (pHasNumber)
        *pHasNumber = pNumberEnd > pNumberStart;
    if( bMinus )
        nVal = -nVal;
    rType = eScanType;
    return ERRCODE_NONE;
}

ErrCode ImpScan(std::u16string_view rSrc, double& nVal, SbxDataType& rType, sal_Int32* pLen)
{
    using namespace officecfg::Office::Scripting;
    static const bool bEnv = std::getenv("LIBREOFFICE6FLOATINGPOINTMODE") != nullptr;
    bool bMode = bEnv || Basic::Compatibility::UseLibreOffice6FloatingPointConversion::get();

    return ImpScan(rSrc, nVal, rType, pLen, nullptr, !bMode);
}

// port for CDbl in the Basic
ErrCode SbxValue::ScanNumIntnl( const OUString& rSrc, double& nVal, bool bSingle )
{
    sal_Int32 nLen = 0;
    ErrCode nRetError = ImpScan( rSrc, nVal, o3tl::temporary(SbxDataType()), &nLen, nullptr,
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

// formatted number output

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


bool SbxValue::Scan(std::u16string_view rSrc, sal_Int32* pLen)
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

std::shared_ptr<SvNumberFormatter> GetFormatter()
{
    if (auto pInst = GetSbData()->pInst)
    {
        return pInst->GetNumberFormatter();
    }
    else
    {
        return SbiInstance::PrepareNumberFormatter(o3tl::temporary(sal_uInt32()),
                                                   o3tl::temporary(sal_uInt32()),
                                                   o3tl::temporary(sal_uInt32()));
    }
}

std::optional<double> StrToNumberIntl(const OUString& s,
                                      std::shared_ptr<SvNumberFormatter>& rpFormatter)
{
    double ret;
    if (SbxValue::ScanNumIntnl(s, ret) == ERRCODE_NONE)
        return ret;

    // We couldn't detect a Basic-formatted number (including type characters & specific exponents).
    // Try generic number detection (works also for dates/times).

    rpFormatter = GetFormatter();
    assert(rpFormatter);
    LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();

    // Passing an index of a locale switches IsNumberFormat() to use that
    // locale in case the formatter wasn't default created with it.
    sal_uInt32 nIndex = rpFormatter->GetStandardIndex(eLangType);

    if (rpFormatter->IsNumberFormat(s, nIndex, ret))
        return ret;

    return {};
}

// For numeric types, takes the number directly; otherwise, tries to take string and convert it
std::optional<double> GetNumberIntl(const SbxValue& val, OUString& rStrVal,
                                    std::shared_ptr<SvNumberFormatter>& rpFormatter,
                                    bool extendedNumberDetection)
{
    switch (SbxDataType type = val.GetType())
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
            return val.GetDouble();
        case SbxBOOL:
            if (extendedNumberDetection)
                return val.GetDouble();
            [[fallthrough]];
        case SbxSTRING:
        default:
            rStrVal = val.GetOUString();
            return extendedNumberDetection || type == SbxSTRING
                       ? StrToNumberIntl(rStrVal, rpFormatter)
                       : std::nullopt;
    }
}
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
    }

    const SbxDataType eType = GetType();
    if (eType == SbxNULL)
    {
        rRes = SbxBasicFormater::BasicFormatNull(pFmt ? *pFmt : std::u16string_view{});
        return;
    }

    std::shared_ptr<SvNumberFormatter> pFormatter;
    std::optional<double> number = GetNumberIntl(*this, rRes, pFormatter, pFmt != nullptr);

    if (!number)
    {
        if (eType == SbxSTRING && pFmt)
            printfmtstr(rRes, rRes, *pFmt);
        return;
    }

    if (!pFmt)
    {
        ImpCvtNum(*number, eType == SbxSINGLE ? 6 : eType == SbxDOUBLE ? 14 : 0, rRes);
        return;
    }

    if (SbxBasicFormater::isBasicFormat(*pFmt))
    {
        BasicFormatNum(*number, *pFmt, rRes);
        return;
    }

    // pflin, It is better to use SvNumberFormatter to handle the date/time/number format.
    // the SvNumberFormatter output is mostly compatible with
    // VBA output besides the OOo-basic output
#if HAVE_FEATURE_SCRIPTING
    // number format, use SvNumberFormatter to handle it.
    if (!pFormatter)
        pFormatter = GetFormatter();

    LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();

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
        pFormatter->GetOutputString(*number, nIndex, rRes, &pCol);
    }
    else if (aFmtStr.equalsIgnoreAsciiCase("General Date") // VBA general date variants
             || aFmtStr.equalsIgnoreAsciiCase("c"))
    {
        OUString dateStr;
        if (*number <= -1.0 || *number >= 1.0)
        {
            // short date
            nIndex = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLangType );
            pFormatter->GetOutputString(*number, nIndex, dateStr, &pCol);

            if (floor(*number) == *number)
            {
                rRes = dateStr;
                return;
            }
        }
        // long time
        aFmtStr = u"H:MM:SS AM/PM"_ustr;
        pFormatter->PutandConvertEntry(aFmtStr, nCheckPos, nType, nIndex,
                                       LANGUAGE_ENGLISH_US, eLangType, true);
        pFormatter->GetOutputString(*number, nIndex, rRes, &pCol);
        if (!dateStr.isEmpty())
            rRes = dateStr + " " + rRes;
    }
    else if (aFmtStr.equalsIgnoreAsciiCase("n") // VBA minute variants
             || aFmtStr.equalsIgnoreAsciiCase("nn"))
    {
        sal_Int32 nMin = implGetMinute(*number);
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
        rRes = OUString::number(implGetWeekDay(*number));
    }
    else if (aFmtStr.equalsIgnoreAsciiCase("y")) // VBA year day number
    {
        sal_Int16 nYear = implGetDateYear(*number);
        double dBaseDate;
        implDateSerial( nYear, 1, 1, true, SbDateCorrection::None, dBaseDate );
        sal_Int32 nYear32 = 1 + sal_Int32(*number - dBaseDate);
        rRes = OUString::number(nYear32);
    }
    else
    {
        pFormatter->PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH_US, eLangType, true);
        pFormatter->GetOutputString(*number, nIndex, rRes, &pCol);
    }
#endif
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
