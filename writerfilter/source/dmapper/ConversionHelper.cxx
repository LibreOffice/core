/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <ConversionHelper.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <editeng/borderline.hxx>
#include <ooxml/resourceids.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/color.hxx>
#include <algorithm>
#include <functional>

using namespace com::sun::star;
using namespace com::sun::star::table::BorderLineStyle;

namespace writerfilter {
namespace dmapper{
namespace ConversionHelper{

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

sal_Int32 MakeBorderLine( sal_Int32 nSprmValue, table::BorderLine2& rToFill )
{
    
    
    
    
    
    
    
    
    
    
    
    
    
    sal_Int16 nLineThicknessTwip = (sal_Int16)((nSprmValue & 0xff) * 20)/8L ;
    sal_Int32 nLineType       = ((nSprmValue & 0xff00) >> 8);
    sal_Int32 nLineColor    = (nSprmValue & 0xff0000)>>16;
    sal_Int32 nLineDistance = (((nSprmValue & 0x3f000000)>>24) * 2540 + 36)/72L;
    MakeBorderLine( nLineThicknessTwip, nLineType, nLineColor, rToFill, false);
    return nLineDistance;
}
void MakeBorderLine( sal_Int32 nLineThickness,   sal_Int32 nLineType,
                                            sal_Int32 nLineColor,
                                            table::BorderLine2& rToFill, bool bIsOOXML )
{
    static const sal_Int32 aBorderDefColor[] =
    {
        
        
        0, COL_BLACK, COL_LIGHTBLUE, COL_LIGHTCYAN, COL_LIGHTGREEN,
        COL_LIGHTMAGENTA, COL_LIGHTRED, COL_YELLOW, COL_WHITE, COL_BLUE,
        COL_CYAN, COL_GREEN, COL_MAGENTA, COL_RED, COL_BROWN, COL_GRAY,
        COL_LIGHTGRAY
    };
    
    if(!nLineColor)
        ++nLineColor;
    if(!bIsOOXML && sal::static_int_cast<sal_uInt32>(nLineColor) < SAL_N_ELEMENTS(aBorderDefColor))
        nLineColor = aBorderDefColor[nLineColor];

    
    
    
    
    ::editeng::SvxBorderStyle const nLineStyle(
            ::editeng::ConvertBorderStyleFromWord(nLineType));
    rToFill.LineStyle = nLineStyle;
    double const fConverted( (NONE == nLineStyle) ? 0.0 :
        ::editeng::ConvertBorderWidthFromWord(nLineStyle, nLineThickness,
            nLineType));
    rToFill.LineWidth = convertTwipToMM100(fConverted);
    rToFill.Color = nLineColor;
}

namespace {
void lcl_SwapQuotesInField(OUString &rFmt)
{
    
    sal_Int32 nLen = rFmt.getLength();
    OUStringBuffer aBuffer( rFmt.getStr() );
    const sal_Unicode* pFmt = rFmt.getStr();
    for (sal_Int32 nI = 0; nI < nLen; ++nI)
    {
        if ((pFmt[nI] == '\"') && (!nI || pFmt[nI-1] != '\\'))
            aBuffer[nI] = '\'';
        else if ((pFmt[nI] == '\'') && (!nI || pFmt[nI-1] != '\\'))
            aBuffer[nI] = '\"';
    }
    rFmt = aBuffer.makeStringAndClear();
}
bool lcl_IsNotAM(OUString& rFmt, sal_Int32 nPos)
{
    return (
            (nPos == rFmt.getLength() - 1) ||
            (
            (rFmt[nPos+1] != 'M') &&
            (rFmt[nPos+1] != 'm')
            )
        );
}
}

OUString ConvertMSFormatStringToSO(
        const OUString& rFormat, lang::Locale& rLocale, bool bHijri)
{
    OUString sFormat(rFormat);
    lcl_SwapQuotesInField(sFormat);

    
    bool bForceJapanese(false);
    bool bForceNatNum(false);
    sal_Int32 nLen = sFormat.getLength();
    sal_Int32 nI = 0;

    OUStringBuffer aNewFormat( sFormat );
    while (nI < nLen)
    {
        if (aNewFormat[nI] == '\\')
            nI++;
        else if (aNewFormat[nI] == '\"')
        {
            ++nI;
            
            while ((nI < nLen) && (!(aNewFormat[nI] == '\"') && (aNewFormat[nI-1] != '\\')))
                ++nI;
        }
        else 
        {
            sal_Unicode nChar = aNewFormat[nI];
            if (nChar == 'O')
            {
                aNewFormat[nI] = 'M';
                bForceNatNum = true;
            }
            else if (nChar == 'o')
            {
                aNewFormat[nI] = 'm';
                bForceNatNum = true;
            }
            else if ((nChar == 'A') && lcl_IsNotAM(sFormat, nI))
            {
                aNewFormat[nI] = 'D';
                bForceNatNum = true;
            }
            else if ((nChar == 'g') || (nChar == 'G'))
                bForceJapanese = true;
            else if ((nChar == 'a') && lcl_IsNotAM(sFormat, nI))
                bForceJapanese = true;
            else if (nChar == 'E')
            {
                if ((nI != nLen-1) && (aNewFormat[nI+1] == 'E'))
                {
                    
                    aNewFormat[nI] = 'Y';
                    aNewFormat[nI + 1] = 'Y';
                    aNewFormat.insert(nI + 2, "YY");
                    nLen+=2;
                    nI+=3;
                }
                bForceJapanese = true;
            }
            else if (nChar == 'e')
            {
                if ((nI != nLen-1) && (aNewFormat[nI+1] == 'e'))
                {
                    
                    aNewFormat[nI] = 'y';
                    aNewFormat[nI + 1] = 'y';
                    aNewFormat.insert(nI + 2, "yy");
                    nLen+=2;
                    nI+=3;
                }
                bForceJapanese = true;
            }
            else if (nChar == '/')
            {
                
                
                aNewFormat[nI] = '\\';
                aNewFormat.insert(nI + 1, "/");
                nI++;
                nLen++;
            }
        }
        ++nI;
    }

    if (bForceNatNum)
        bForceJapanese = true;

    if (bForceJapanese)
    {
        rLocale.Language = "ja";
        rLocale.Country = "JP";
    }

    if (bForceNatNum)
    {
        aNewFormat.insert( 0, "[NatNum1][$-411]");
    }

    if (bHijri)
    {
        aNewFormat.insert( 0, "[~hijri]");
    }
    return aNewFormat.makeStringAndClear();

}


sal_Int32 convertTwipToMM100(sal_Int32 _t)
{
    return TWIP_TO_MM100( _t );
}


sal_Int32 convertEMUToMM100(sal_Int32 _t)
{
    return _t / 360;
}

/*-------------------------------------------------------------------------
    contains a color from 0xTTRRGGBB to 0xTTRRGGBB
  -----------------------------------------------------------------------*/
sal_Int32 ConvertColor(sal_Int32 nWordColor)
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nWordColor&0xFF)),
        g(static_cast<sal_uInt8>(((nWordColor)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nWordColor>>16)&0xFF)),
        t(static_cast<sal_uInt8>((nWordColor>>24)&0xFF));
    sal_Int32 nRet = (t<<24) + (r<<16) + (g<<8) + b;
    return nRet;
}

sal_Int16 convertTableJustification( sal_Int32 nIntValue )
{
    sal_Int16 nOrient = text::HoriOrientation::LEFT_AND_WIDTH;
    switch( nIntValue )
    {
        case 1 : nOrient = text::HoriOrientation::CENTER; break;
        case 2 : nOrient = text::HoriOrientation::RIGHT; break;
        case 0 :
        
        default:;

    }
    return nOrient;
}

sal_Int16 ConvertNumberingType(sal_Int32 nFmt)
{
    sal_Int16 nRet;
    switch(nFmt)
    {
        case NS_ooxml::LN_Value_ST_NumberFormat_decimal:
        case 0:
            nRet = style::NumberingType::ARABIC;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_upperRoman:
        case 1:
            nRet = style::NumberingType::ROMAN_UPPER;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman:
        case 2:
            nRet = style::NumberingType::ROMAN_LOWER;
            break;
        case 3:
            nRet = style::NumberingType::CHARS_UPPER_LETTER_N;
            break;
        case 4:
            nRet = style::NumberingType::CHARS_LOWER_LETTER_N;
            break;
        case 5:
            nRet = style::NumberingType::ARABIC;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_bullet:
        case 23:
        case 25:
            nRet = style::NumberingType::CHAR_SPECIAL;
        break;
        case NS_ooxml::LN_Value_ST_NumberFormat_none:
        case 255:
            nRet = style::NumberingType::NUMBER_NONE;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_upperLetter:
            nRet = style::NumberingType::CHARS_UPPER_LETTER;
            break;
        case  NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter:
            nRet = style::NumberingType::CHARS_LOWER_LETTER;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_iroha:
            nRet = style::NumberingType::IROHA_HALFWIDTH_JA;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_irohaFullWidth:
            nRet = style::NumberingType::IROHA_FULLWIDTH_JA;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_aiueo:
            nRet = style::NumberingType::AIU_HALFWIDTH_JA;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_aiueoFullWidth:
            nRet = style::NumberingType::AIU_FULLWIDTH_JA;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_hebrew2:
            nRet = style::NumberingType::CHARS_HEBREW;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_thaiLetters:
            nRet = style::NumberingType::CHARS_THAI;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_russianLower:
            nRet = style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_RU;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_russianUpper:
            nRet = style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_RU;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedCircleChinese:
        case NS_ooxml::LN_Value_ST_NumberFormat_ideographEnclosedCircle:
            nRet = style::NumberingType::CIRCLE_NUMBER;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_ideographTraditional:
            nRet = style::NumberingType::TIAN_GAN_ZH;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_ideographZodiac:
            nRet = style::NumberingType::DI_ZI_ZH;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_ganada:
            nRet = style::NumberingType::HANGUL_SYLLABLE_KO;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_chosung:
            nRet = style::NumberingType::HANGUL_JAMO_KO;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_koreanDigital:
        case NS_ooxml::LN_Value_ST_NumberFormat_koreanCounting:
        case NS_ooxml::LN_Value_ST_NumberFormat_koreanDigital2:
            nRet = style::NumberingType::NUMBER_HANGUL_KO;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_ideographLegalTraditional:
            nRet = style::NumberingType::NUMBER_UPPER_ZH_TW;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_arabicAlpha:
            nRet = style::NumberingType::CHARS_ARABIC;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_hindiVowels:
            nRet = style::NumberingType::CHARS_NEPALI;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_japaneseLegal:
            nRet = style::NumberingType::NUMBER_TRADITIONAL_JA;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_chineseCounting:
        case NS_ooxml::LN_Value_ST_NumberFormat_japaneseCounting:
        case NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseCounting:
        case NS_ooxml::LN_Value_ST_NumberFormat_ideographDigital:
        case NS_ooxml::LN_Value_ST_NumberFormat_chineseCountingThousand:
            nRet = style::NumberingType::NUMBER_LOWER_ZH;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_chineseLegalSimplified:
            nRet = style::NumberingType::NUMBER_UPPER_ZH;
            break;
        case NS_ooxml::LN_Value_ST_NumberFormat_hebrew1:
            
            nRet = style::NumberingType::CHARS_HEBREW;
            break;
        default: nRet = style::NumberingType::ARABIC;
    }
/*  TODO: Lots of additional values are available - some are supported in the I18 framework
    NS_ooxml::LN_Value_ST_NumberFormat_ordinal = 91682;
    NS_ooxml::LN_Value_ST_NumberFormat_cardinalText = 91683;
    NS_ooxml::LN_Value_ST_NumberFormat_ordinalText = 91684;
    NS_ooxml::LN_Value_ST_NumberFormat_hex = 91685;
    NS_ooxml::LN_Value_ST_NumberFormat_chicago = 91686;
    NS_ooxml::LN_Value_ST_NumberFormat_decimalFullWidth = 91691;
    NS_ooxml::LN_Value_ST_NumberFormat_decimalHalfWidth = 91692;
    NS_ooxml::LN_Value_ST_NumberFormat_japaneseDigitalTenThousand = 91694;
    NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedCircle = 91695;
    NS_ooxml::LN_Value_ST_NumberFormat_decimalFullWidth2 = 91696;
    NS_ooxml::LN_Value_ST_NumberFormat_decimalZero = 91699;
    NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedFullstop = 91703;
    NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedParen = 91704;
    NS_ooxml::LN_Value_ST_NumberFormat_ideographZodiacTraditional = 91709;
    NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseCountingThousand = 91712;
    NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseDigital = 91713;
    NS_ooxml::LN_Value_ST_NumberFormat_chineseLegalSimplified = 91715;
    NS_ooxml::LN_Value_ST_NumberFormat_chineseCountingThousand = 91716;
    NS_ooxml::LN_Value_ST_NumberFormat_koreanLegal = 91719;
    NS_ooxml::LN_Value_ST_NumberFormat_vietnameseCounting = 91721;
    NS_ooxml::LN_Value_ST_NumberFormat_numberInDash = 91725;
    NS_ooxml::LN_Value_ST_NumberFormat_arabicAbjad:
    NS_ooxml::LN_Value_ST_NumberFormat_hindiConsonants = 91731;
    NS_ooxml::LN_Value_ST_NumberFormat_hindiNumbers = 91732;
    NS_ooxml::LN_Value_ST_NumberFormat_hindiCounting = 91733;
    NS_ooxml::LN_Value_ST_NumberFormat_thaiNumbers = 91735;
    NS_ooxml::LN_Value_ST_NumberFormat_thaiCounting = 91736;*/
    return nRet;
}

com::sun::star::util::DateTime ConvertDateStringToDateTime( const OUString& rDateTime )
{
    com::sun::star::util::DateTime aDateTime;
    
    
    sal_Int32 nIndex = 0;
    OUString sDate = rDateTime.getToken( 0, 'T', nIndex );
    
    
    OUString sTime = rDateTime.getToken( 0, 'Z', nIndex );
    nIndex = 0;
    aDateTime.Year = sal_uInt16( sDate.getToken( 0, '-', nIndex ).toInt32() );
    aDateTime.Month = sal_uInt16( sDate.getToken( 0, '-', nIndex ).toInt32() );
    if (nIndex != -1)
        aDateTime.Day = sal_uInt16( sDate.copy( nIndex ).toInt32() );

    nIndex = 0;
    aDateTime.Hours = sal_uInt16( sTime.getToken( 0, ':', nIndex ).toInt32() );
    aDateTime.Minutes = sal_uInt16( sTime.getToken( 0, ':', nIndex ).toInt32() );
    if (nIndex != -1)
        aDateTime.Seconds = sal_uInt16( sTime.copy( nIndex ).toInt32() );

    return aDateTime;
}


} 
} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
