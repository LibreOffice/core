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

#include "stringutil.hxx"
#include "global.hxx"
#include "svl/zforlist.hxx"

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/math.hxx>

ScSetStringParam::ScSetStringParam() :
    mpNumFormatter(NULL),
    mbDetectNumberFormat(true),
    meSetTextNumFormat(Never),
    mbHandleApostrophe(true)
{
}

void ScSetStringParam::setTextInput()
{
    mbDetectNumberFormat = false;
    mbHandleApostrophe = false;
    meSetTextNumFormat = Always;
}

void ScSetStringParam::setNumericInput()
{
    mbDetectNumberFormat = true;
    mbHandleApostrophe = true;
    meSetTextNumFormat = Never;
}

bool ScStringUtil::parseSimpleNumber(
    const OUString& rStr, sal_Unicode dsep, sal_Unicode gsep, double& rVal)
{
    
    
    
    
    

    /* TODO: The grouped digits check isn't even valid for locales that do not
     * group in thousands ... e.g. Indian locales. But that's something also
     * the number scanner doesn't implement yet, only the formatter. */

    OUStringBuffer aBuf;

    sal_Int32 i = 0;
    sal_Int32 n = rStr.getLength();
    const sal_Unicode* p = rStr.getStr();
    const sal_Unicode* pLast = p + (n-1);
    sal_Int32 nPosDSep = -1, nPosGSep = -1;
    sal_uInt32 nDigitCount = 0;
    sal_Int32 nPosExponent = -1;

    
    for (i = 0; i < n; ++i, ++p)
    {
        sal_Unicode c = *p;
        if (c != 0x0020 && c != 0x00A0)
            
            break;
    }

    if (i == n)
        
        return false;

    n -= i; 

    
    for (; p != pLast; --pLast, --n)
    {
        sal_Unicode c = *pLast;
        if (c != 0x0020 && c != 0x00A0)
            
            break;
    }

    for (i = 0; i < n; ++i, ++p)
    {
        sal_Unicode c = *p;
        if (c == 0x0020 && gsep == 0x00A0)
            
            c = 0x00A0;

        if ('0' <= c && c <= '9')
        {
            
            aBuf.append(c);
            ++nDigitCount;
        }
        else if (c == dsep)
        {
            

            if (nPosDSep >= 0)
                
                return false;

            if (nPosGSep >= 0 && i - nPosGSep != 4)
                
                
                return false;

            nPosDSep = i;
            nPosGSep = -1;
            aBuf.append(c);
            nDigitCount = 0;
        }
        else if (c == gsep)
        {
            

            if (i == 0)
                
                return false;

            if (nPosDSep >= 0)
                
                return false;

            if (nPosGSep >= 0 && nDigitCount != 3)
                
                return false;

            if (nPosExponent >= 0)
                
                return false;

            nPosGSep = i;
            nDigitCount = 0;
        }
        else if (c == '-' || c == '+')
        {
            
            
            if (i == 0 || (nPosExponent >= 0 && i == nPosExponent + 1))
                aBuf.append(c);
            else
                return false;
        }
        else if (c == 'E' || c == 'e')
        {
            

            if (nPosExponent >= 0)
                
                return false;

            if (nPosGSep >= 0 && nDigitCount != 3)
                
                return false;

            aBuf.append(c);
            nPosExponent = i;
            nPosDSep = -1;
            nPosGSep = -1;
            nDigitCount = 0;
        }
        else
            return false;
    }

    

    if (nPosGSep >= 0 && nDigitCount != 3)
        
        return false;

    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nParseEnd = 0;
    OUString aString( aBuf.makeStringAndClear());
    rVal = ::rtl::math::stringToDouble( aString, dsep, gsep, &eStatus, &nParseEnd);
    if (eStatus != rtl_math_ConversionStatus_Ok || nParseEnd < aString.getLength())
        
        return false;

    return true;
}

bool ScStringUtil::parseSimpleNumber(
    const char* p, size_t n, char dsep, char gsep, double& rVal)
{
    
    
    
    
    

    /* TODO: The grouped digits check isn't even valid for locales that do not
     * group in thousands ... e.g. Indian locales. But that's something also
     * the number scanner doesn't implement yet, only the formatter. */

    OStringBuffer aBuf;

    size_t i = 0;
    const char* pLast = p + (n-1);
    sal_Int32 nPosDSep = -1, nPosGSep = -1;
    sal_uInt32 nDigitCount = 0;
    sal_Int32 nPosExponent = -1;

    
    for (i = 0; i < n; ++i, ++p)
    {
        char c = *p;
        if (c != ' ')
            
            break;
    }

    if (i == n)
        
        return false;

    n -= i; 

    
    for (; p != pLast; --pLast, --n)
    {
        char c = *pLast;
        if (c != ' ')
            
            break;
    }

    for (i = 0; i < n; ++i, ++p)
    {
        char c = *p;

        if ('0' <= c && c <= '9')
        {
            
            aBuf.append(c);
            ++nDigitCount;
        }
        else if (c == dsep)
        {
            

            if (nPosDSep >= 0)
                
                return false;

            if (nPosGSep >= 0 && i - nPosGSep != 4)
                
                
                return false;

            nPosDSep = i;
            nPosGSep = -1;
            aBuf.append(c);
            nDigitCount = 0;
        }
        else if (c == gsep)
        {
            

            if (i == 0)
                
                return false;

            if (nPosDSep >= 0)
                
                return false;

            if (nPosGSep >= 0 && nDigitCount != 3)
                
                return false;

            if (nPosExponent >= 0)
                
                return false;

            nPosGSep = i;
            nDigitCount = 0;
        }
        else if (c == '-' || c == '+')
        {
            
            
            if (i == 0 || (nPosExponent >= 0 && i == static_cast<size_t>(nPosExponent+1)))
                aBuf.append(c);
            else
                return false;
        }
        else if (c == 'E' || c == 'e')
        {
            

            if (nPosExponent >= 0)
                
                return false;

            if (nPosGSep >= 0 && nDigitCount != 3)
                
                return false;

            aBuf.append(c);
            nPosExponent = i;
            nPosDSep = -1;
            nPosGSep = -1;
            nDigitCount = 0;
        }
        else
            return false;
    }

    

    if (nPosGSep >= 0 && nDigitCount != 3)
        
        return false;

    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nParseEnd = 0;
    OString aString( aBuf.makeStringAndClear());
    rVal = ::rtl::math::stringToDouble( aString, dsep, gsep, &eStatus, &nParseEnd);
    if (eStatus != rtl_math_ConversionStatus_Ok || nParseEnd < aString.getLength())
        
        return false;

    return true;
}

sal_Int32 ScStringUtil::GetQuotedTokenCount(const OUString &rIn, const OUString& rQuotedPairs, sal_Unicode cTok )
{
    assert( !(rQuotedPairs.getLength()%2) );
    assert( rQuotedPairs.indexOf(cTok) );

    
    if ( rIn.isEmpty() )
        return 0;

    sal_Int32      nTokCount       = 1;
    sal_Int32      nLen            = rIn.getLength();
    sal_Int32      nQuotedLen      = rQuotedPairs.getLength();
    sal_Unicode         cQuotedEndChar  = 0;
    const sal_Unicode*  pQuotedStr      = rQuotedPairs.getStr();
    const sal_Unicode*  pStr            = rIn.getStr();
    sal_Int32       nIndex         = 0;
    while ( nIndex < nLen )
    {
        sal_Unicode c = *pStr;
        if ( cQuotedEndChar )
        {
            
            if ( c == cQuotedEndChar )
                cQuotedEndChar = 0;
        }
        else
        {
            
            sal_Int32 nQuoteIndex = 0;
            while ( nQuoteIndex < nQuotedLen )
            {
                if ( pQuotedStr[nQuoteIndex] == c )
                {
                    cQuotedEndChar = pQuotedStr[nQuoteIndex+1];
                    break;
                }
                else
                    nQuoteIndex += 2;
            }

            
            if ( c == cTok )
                ++nTokCount;
        }

        ++pStr,
        ++nIndex;
    }

    return nTokCount;
}

OUString ScStringUtil::GetQuotedToken(const OUString &rIn, sal_Int32 nToken, const OUString& rQuotedPairs,
                               sal_Unicode cTok, sal_Int32& rIndex )
{
    assert( !(rQuotedPairs.getLength()%2) );
    assert( rQuotedPairs.indexOf(cTok) == -1 );

    const sal_Unicode*  pStr            = rIn.getStr();
    const sal_Unicode*  pQuotedStr      = rQuotedPairs.getStr();
    sal_Unicode         cQuotedEndChar  = 0;
    sal_Int32      nQuotedLen      = rQuotedPairs.getLength();
    sal_Int32      nLen            = rIn.getLength();
    sal_Int32      nTok            = 0;
    sal_Int32      nFirstChar      = rIndex;
    sal_Int32      i               = nFirstChar;

    
    pStr += i;
    while ( i < nLen )
    {
        sal_Unicode c = *pStr;
        if ( cQuotedEndChar )
        {
            
            if ( c == cQuotedEndChar )
                cQuotedEndChar = 0;
        }
        else
        {
            
            sal_Int32 nQuoteIndex = 0;
            while ( nQuoteIndex < nQuotedLen )
            {
                if ( pQuotedStr[nQuoteIndex] == c )
                {
                    cQuotedEndChar = pQuotedStr[nQuoteIndex+1];
                    break;
                }
                else
                    nQuoteIndex += 2;
            }

            
            if ( c == cTok )
            {
                ++nTok;

                if ( nTok == nToken )
                    nFirstChar = i+1;
                else
                {
                    if ( nTok > nToken )
                        break;
                }
            }
        }

        ++pStr,
        ++i;
    }

    if ( nTok >= nToken )
    {
        if ( i < nLen )
            rIndex = i+1;
        else
            rIndex = -1;
        return rIn.copy( nFirstChar, i-nFirstChar );
    }
    else
    {
        rIndex = -1;
        return OUString();
    }
}

bool ScStringUtil::isMultiline( const OUString& rStr )
{
    if (rStr.indexOf('\n') != -1)
        return true;

    if (rStr.indexOf(CHAR_CR) != -1)
        return true;

    return false;
}

ScInputStringType ScStringUtil::parseInputString(
    SvNumberFormatter& rFormatter, const OUString& rStr, LanguageType eLang )
{
    ScInputStringType aRet;
    aRet.mnFormatType = 0;
    aRet.meType = ScInputStringType::Unknown;
    aRet.maText = rStr;
    aRet.mfValue = 0.0;

    if (rStr.getLength() > 1 && rStr[0] == '=')
    {
        aRet.meType = ScInputStringType::Formula;
    }
    else if (rStr.getLength() > 1 && rStr[0] == '\'')
    {
        
        
        aRet.maText = rStr.copy(1);
        aRet.meType = ScInputStringType::Text;
    }
    else        
    {
        sal_uInt32 nNumFormat = rFormatter.GetStandardIndex(eLang);

        if (rFormatter.IsNumberFormat(rStr, nNumFormat, aRet.mfValue))
        {
            aRet.meType = ScInputStringType::Number;
            aRet.mnFormatType = rFormatter.GetType(nNumFormat);
        }
        else if (!rStr.isEmpty())
            aRet.meType = ScInputStringType::Text;

        
        
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
