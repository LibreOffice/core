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

#include <string_view>

#include <config_features.h>

#include <vcl/errcode.hxx>
#include <unotools/resmgr.hxx>
#include "sbxconv.hxx"
#include <rtlproto.hxx>

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


void ImpGetIntntlSep( sal_Unicode& rcDecimalSep, sal_Unicode& rcThousandSep, sal_Unicode& rcDecimalSepAlt )
{
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
    rcDecimalSep = rData.getNumDecimalSep()[0];
    rcThousandSep = rData.getNumThousandSep()[0];
    rcDecimalSepAlt = rData.getNumDecimalSepAlt().toChar();
}


static bool ImpStrChr( const OUString& str, sal_Unicode c ) { return str.indexOf(c) >= 0; }


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
                    (cIntntlDecSep && *p == cIntntlGrpSep) || (cIntntlDecSepAlt && *p == cIntntlDecSepAlt)) &&
                rtl::isAsciiDigit( *(p+1) )))
    {
        // tdf#118442: Whitespace and minus are skipped; store the position to calculate index
        const sal_Unicode* const pDigitsStart = p;
        short exp = 0;
        short decsep = 0;
        short ndig = 0;
        short ncdig = 0;    // number of digits after decimal point
        OUStringBuffer aSearchStr("0123456789DEde");
        aSearchStr.append(cNonIntntlDecSep);
        if( cIntntlDecSep != cNonIntntlDecSep )
            aSearchStr.append(cIntntlDecSep);
        if( cIntntlDecSepAlt && cIntntlDecSepAlt != cNonIntntlDecSep )
            aSearchStr.append(cIntntlDecSepAlt);
        if( bOnlyIntntl )
            aSearchStr.append(cIntntlGrpSep);
        const OUString pSearchStr = aSearchStr.makeStringAndClear();
        static const OUStringLiteral pDdEe = u"DdEe";
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

        OUString aBufStr( aBuf.makeStringAndClear());
        rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
        sal_Int32 nParseEnd = 0;
        nVal = rtl::math::stringToDouble( aBufStr, cIntntlDecSep, cIntntlGrpSep, &eStatus, &nParseEnd );
        if( eStatus != rtl_math_ConversionStatus_Ok || nParseEnd != aBufStr.getLength() )
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
        static const OUStringLiteral pTypes = u"%!&#";
        if( ImpStrChr( pTypes, *p ) )
            p++;
    }
    // hex/octal number? read in and convert:
    else if( *p == '&' )
    {
        p++;
        eScanType = SbxLONG;
        OUString aCmp( "0123456789ABCDEF" );
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
    if( pLen )
        *pLen = static_cast<sal_uInt16>( p - pStart );
    if( !bRes )
        return ERRCODE_BASIC_CONVERSION;
    if( bMinus )
        nVal = -nVal;
    rType = eScanType;
    return ERRCODE_NONE;
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
    sal_Unicode cDecimalSep, cThousandSep, cDecimalSepAlt;
    ImpGetIntntlSep( cDecimalSep, cThousandSep, cDecimalSepAlt );
    if( bCoreString )
        cDecimalSep = '.';

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

static sal_uInt16 printfmtstr( const OUString& rStr, OUString& rRes, const OUString& rFmt )
{
    OUStringBuffer aTemp;
    const sal_Unicode* pStr = rStr.getStr();
    const sal_Unicode* pFmtStart = rFmt.getStr();
    const sal_Unicode* pFmt = pFmtStart;

    switch( *pFmt )
    {
    case '!':
        aTemp.append(*pStr++);
        pFmt++;
        break;
    case '\\':
        do
        {
            aTemp.append( *pStr ? *pStr++ : u' ');
            pFmt++;
        }
        while( *pFmt && *pFmt != '\\' );
        aTemp.append(*pStr ? *pStr++ : u' ');
        pFmt++; break;
    case '&':
        aTemp = rStr;
        pFmt++; break;
    default:
        aTemp = rStr;
        break;
    }
    rRes = aTemp.makeStringAndClear();
    return static_cast<sal_uInt16>( pFmt - pFmtStart );
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
        eRes = ImpScan( rSrc, n, t, pLen, !LibreOffice6FloatingPointMode() );
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
    Null
};

#if HAVE_FEATURE_SCRIPTING

struct VbaFormatInfo
{
    VbaFormatType meType;
    std::u16string_view mpVbaFormat; // Format string in vba
    NfIndexTableOffset meOffset; // SvNumberFormatter format index, if meType = VbaFormatType::Offset
    const char* mpOOoFormat;     // if meType = VbaFormatType::UserDefined
};

const VbaFormatInfo pFormatInfoTable[] =
{
    { VbaFormatType::Offset,      std::u16string_view(u"Long Date"),   NF_DATE_SYSTEM_LONG,    nullptr },
    { VbaFormatType::UserDefined, std::u16string_view(u"Medium Date"), NF_NUMBER_STANDARD,     "DD-MMM-YY" },
    { VbaFormatType::Offset,      std::u16string_view(u"Short Date"),  NF_DATE_SYSTEM_SHORT,   nullptr },
    { VbaFormatType::UserDefined, std::u16string_view(u"Long Time"),   NF_NUMBER_STANDARD,     "H:MM:SS AM/PM" },
    { VbaFormatType::Offset,      std::u16string_view(u"Medium Time"), NF_TIME_HHMMAMPM,       nullptr },
    { VbaFormatType::Offset,      std::u16string_view(u"Short Time"),  NF_TIME_HHMM,           nullptr },
    { VbaFormatType::Offset,      std::u16string_view(u"ddddd"),       NF_DATE_SYSTEM_SHORT,   nullptr },
    { VbaFormatType::Offset,      std::u16string_view(u"dddddd"),      NF_DATE_SYSTEM_LONG,    nullptr },
    { VbaFormatType::UserDefined, std::u16string_view(u"ttttt"),       NF_NUMBER_STANDARD,     "H:MM:SS AM/PM" },
    { VbaFormatType::Offset,      std::u16string_view(u"ww"),          NF_DATE_WW,             nullptr },
    { VbaFormatType::Null,        std::u16string_view(u""),            NF_INDEX_TABLE_ENTRIES, nullptr }
};

const VbaFormatInfo* getFormatInfo( const OUString& rFmt )
{
    const VbaFormatInfo* pInfo = pFormatInfoTable;
    while( pInfo->meType != VbaFormatType::Null )
    {
        if( rFmt.equalsIgnoreAsciiCase( pInfo->mpVbaFormat ) )
            break;
        ++pInfo;
    }
    return pInfo;
}
#endif

} // namespace

#if HAVE_FEATURE_SCRIPTING
constexpr OUStringLiteral VBAFORMAT_GENERALDATE = u"General Date";
constexpr OUStringLiteral VBAFORMAT_C = u"c";
constexpr OUStringLiteral VBAFORMAT_N = u"n";
constexpr OUStringLiteral VBAFORMAT_NN = u"nn";
constexpr OUStringLiteral VBAFORMAT_W = u"w";
constexpr OUStringLiteral VBAFORMAT_Y = u"y";
constexpr OUStringLiteral VBAFORMAT_LOWERCASE = u"<";
constexpr OUStringLiteral VBAFORMAT_UPPERCASE = u">";
#endif

void SbxValue::Format( OUString& rRes, const OUString* pFmt ) const
{
    short nComma = 0;
    double d = 0;

    // pflin, It is better to use SvNumberFormatter to handle the date/time/number format.
    // the SvNumberFormatter output is mostly compatible with
    // VBA output besides the OOo-basic output
#if HAVE_FEATURE_SCRIPTING
    if( pFmt && !SbxBasicFormater::isBasicFormat( *pFmt ) )
    {
        OUString aStr = GetOUString();

        SvtSysLocale aSysLocale;
        const CharClass& rCharClass = aSysLocale.GetCharClass();

        if( pFmt->equalsIgnoreAsciiCase( VBAFORMAT_LOWERCASE ) )
        {
            rRes = rCharClass.lowercase( aStr );
            return;
        }
        if( pFmt->equalsIgnoreAsciiCase( VBAFORMAT_UPPERCASE ) )
        {
            rRes = rCharClass.uppercase( aStr );
            return;
        }

        LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
        std::shared_ptr<SvNumberFormatter> pFormatter;
        if (GetSbData()->pInst)
        {
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
        }
        else
        {
            sal_uInt32 n;   // Dummy
            pFormatter = SbiInstance::PrepareNumberFormatter( n, n, n );
        }

        // Passing an index of a locale switches IsNumberFormat() to use that
        // locale in case the formatter wasn't default created with it.
        sal_uInt32 nIndex = pFormatter->GetStandardIndex( eLangType);
        double nNumber;
        const Color* pCol;

        bool bSuccess = pFormatter->IsNumberFormat( aStr, nIndex, nNumber );

        // number format, use SvNumberFormatter to handle it.
        if( bSuccess )
        {
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType;
            OUString aFmtStr = *pFmt;
            const VbaFormatInfo* pInfo = getFormatInfo( aFmtStr );
            if( pInfo->meType != VbaFormatType::Null )
            {
                if( pInfo->meType == VbaFormatType::Offset )
                {
                    nIndex = pFormatter->GetFormatIndex( pInfo->meOffset, eLangType );
                }
                else
                {
                    aFmtStr = OUString::createFromAscii(pInfo->mpOOoFormat);
                    pFormatter->PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH_US, eLangType, true);
                }
                pFormatter->GetOutputString( nNumber, nIndex, rRes, &pCol );
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_GENERALDATE )
                    || aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_C ))
            {
                if( nNumber <=-1.0 || nNumber >= 1.0 )
                {
                    // short date
                    nIndex = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLangType );
                    pFormatter->GetOutputString( nNumber, nIndex, rRes, &pCol );

                    // long time
                    if( floor( nNumber ) != nNumber )
                    {
                        aFmtStr = "H:MM:SS AM/PM";
                        pFormatter->PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH_US, eLangType, true);
                        OUString aTime;
                        pFormatter->GetOutputString( nNumber, nIndex, aTime, &pCol );
                        rRes += " " + aTime;
                    }
                }
                else
                {
                    // long time only
                    aFmtStr = "H:MM:SS AM/PM";
                    pFormatter->PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH_US, eLangType, true);
                    pFormatter->GetOutputString( nNumber, nIndex, rRes, &pCol );
                }
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_N ) ||
                     aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_NN ))
            {
                sal_Int32 nMin = implGetMinute( nNumber );
                if( nMin < 10 && aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_NN ))
                {
                    // Minute in two digits
                     sal_Unicode aBuf[2];
                     aBuf[0] = '0';
                     aBuf[1] = '0' + nMin;
                     rRes = OUString(aBuf, SAL_N_ELEMENTS(aBuf));
                }
                else
                {
                    rRes = OUString::number(nMin);
                }
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_W ))
            {
                sal_Int32 nWeekDay = implGetWeekDay( nNumber );
                rRes = OUString::number(nWeekDay);
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_Y ))
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
    }
#endif

    SbxDataType eType = GetType();
    switch( eType )
    {
    case SbxCHAR:
    case SbxBYTE:
    case SbxINTEGER:
    case SbxUSHORT:
    case SbxLONG:
    case SbxULONG:
    case SbxINT:
    case SbxUINT:
    case SbxNULL:       // #45929 NULL with a little cheating
        nComma = 0;     goto cvt;
    case SbxSINGLE:
        nComma = 6;     goto cvt;
    case SbxDOUBLE:
        nComma = 14;

    cvt:
        if( eType != SbxNULL )
        {
            d = GetDouble();
        }
        // #45355 another point to jump in for isnumeric-String
    cvt2:
        if( pFmt )
        {
            SbxAppData& rAppData = GetSbxData_Impl();

            LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
            if( rAppData.pBasicFormater )
            {
                if( rAppData.eBasicFormaterLangType != eLangType )
                {
                    rAppData.pBasicFormater.reset();
                }
            }
            rAppData.eBasicFormaterLangType = eLangType;


            if( !rAppData.pBasicFormater )
            {
                SvtSysLocale aSysLocale;
                const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
                sal_Unicode cComma = rData.getNumDecimalSep()[0];
                sal_Unicode c1000  = rData.getNumThousandSep()[0];
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
                                                                cComma,c1000,aOnStrg,aOffStrg,
                                                                aYesStrg,aNoStrg,aTrueStrg,aFalseStrg,
                                                                aCurrencyStrg,aCurrencyFormatStrg );
            }
            // Remark: For performance reasons there's only ONE BasicFormater-
            //    object created and 'stored', so that the expensive resource-
            //    loading is saved (for country-specific predefined outputs,
            //    e. g. "On/Off") and the continuous string-creation
            //    operations, too.
            // BUT: therefore this code is NOT multithreading capable!

            // here are problems with ;;;Null because this method is only
            // called, if SbxValue is a number!!!
            // in addition rAppData.pBasicFormater->BasicFormatNull( *pFmt ); could be called!
            if( eType != SbxNULL )
            {
                rRes = rAppData.pBasicFormater->BasicFormat( d ,*pFmt );
            }
            else
            {
                rRes = SbxBasicFormater::BasicFormatNull( *pFmt );
            }

        }
        else
            ImpCvtNum( GetDouble(), nComma, rRes );
        break;
    case SbxSTRING:
        if( pFmt )
        {
            // #45355 converting if numeric
            if( IsNumericRTL() )
            {
                ScanNumIntnl( GetOUString(), d );
                goto cvt2;
            }
            else
            {
                printfmtstr( GetOUString(), rRes, *pFmt );
            }
        }
        else
        {
            rRes = GetOUString();
        }
        break;
    default:
        rRes = GetOUString();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
