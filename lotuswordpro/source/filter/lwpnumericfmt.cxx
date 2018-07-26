/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 *  For LWP filter architecture prototype - table cell numerics format
 */

#include <memory>
#include "lwpnumericfmt.hxx"

LwpCurrencyPool  LwpNumericFormat::m_aCurrencyInfo;

OUString LwpCurrencyPool::GetCurrencySymbol(sal_uInt16 nFormat)
{
    return m_aCurrencyInfo[nFormat].sSymbol;
}

bool LwpCurrencyPool::IsSymbolPost(sal_uInt16 nFormat)
{
    return m_aCurrencyInfo[nFormat].bPost;
}

bool LwpCurrencyPool::IsShowSpace(sal_uInt16 nFormat)
{
    return m_aCurrencyInfo[nFormat].bShowSpace;
}

XFStyle* LwpLayoutNumerics::Convert()
{
    return cNumerics.Convert();
}

void LwpLayoutNumerics::Read()
{
    LwpVirtualPiece::Read();

    if(LwpFileHeader::m_nFileRevision >= 0x000b)
    {
        cNumerics.Read();
        m_pObjStrm->SkipExtra();
    }
}

void LwpNumericFormatSubset::QuickRead(LwpObjectStream* pStrm)
{
    cColor.Read(pStrm);
    cPrefix.Read(pStrm);
    cSuffix.Read(pStrm);
    cSubFlags = pStrm->QuickReaduInt16();

    pStrm->SkipExtra();
}
LwpColor LwpNumericFormatSubset::GetColor()
{
    if (cSubFlags&0x04)
    {
        return cColor;
    }
    else
    {
        return LwpColor();
    }
}
LwpNumericFormatSubset::LwpNumericFormatSubset():cSubFlags(0)
{
}

LwpNumericFormat::LwpNumericFormat(LwpObjectStream * pStrm)
    : m_pObjStrm(pStrm)
    , cFlags(0)
    , cFormat(FMT_DEFAULT)
    , cDecimalPlaces(0)
{
    assert(pStrm);
}
/**
*   Read number format from wordpro file
*/
void LwpNumericFormat::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm;

    if(LwpFileHeader::m_nFileRevision >= 0x000b)
    {
        cFlags          = pStrm->QuickReaduInt16();
        cDecimalPlaces  = pStrm->QuickReaduInt16();
        cFormat             = pStrm->QuickReaduInt16();

        cAnyNumber.QuickRead(pStrm);
        cZero.QuickRead(pStrm);
        cNegative.QuickRead(pStrm);

        pStrm->SkipExtra();
    }
}
bool LwpNumericFormat::IsCurrencyFormat(sal_uInt16 Format)
{
    switch (Format)
    {
    case FMT_ARGENTINEANPESO:
    case FMT_AUSTRALIANDOLLAR:
    case FMT_AUSTRIANSCHILLING:
    case FMT_BELGIANFRANC:
    case FMT_BRAZILIANCRUZEIRO:
    case FMT_BRITISHPOUND:
    case FMT_CANADIANDOLLAR:
    case FMT_CHINESEYUAN:
    case FMT_CZECHKORUNA:
    case FMT_DANISHKRONE:
    case FMT_ECU:
    case FMT_FINNISHMARKKA:
    case FMT_FRENCHFRANC:
    case FMT_GREEKDRACHMA:
    case FMT_HONGKONGDOLLAR:
    case FMT_HUNGARIANFORINT:
    case FMT_INDIANRUPEE:
    case FMT_INDONESIANRUPIAH:
    case FMT_IRISHPUNT:
    case FMT_LUXEMBOURGFRANC:
    case FMT_MALAYSIANRINGGIT:
    case FMT_MEXICANPESO:
    case FMT_NETHERLANDSGUILDER:
    case FMT_NEWZEALANDDOLLAR:
    case FMT_NORWEGIANKRONE:
    case FMT_POLISHZLOTY:
    case FMT_PORTUGUESEESCUDO:
    case FMT_ROMANIANLEI:
    case FMT_RUSSIANRUBLE:
    case FMT_SINGAPOREDOLLAR:
    case FMT_SLOVAKIANKORUNA:
    case FMT_SLOVENIANTHOLAR:
    case FMT_SOUTHAFRICANRAND:
    case FMT_SOUTHKOREANWON:
    case FMT_SWEDISHKRONA:
    case FMT_SWISSFRANC:
    case FMT_TAIWANDOLLAR:
    case FMT_THAIBAHT:
    case FMT_USDOLLAR:
    case FMT_OTHERCURRENCY:
    case FMT_GERMANMARK:
    case FMT_ITALIANLIRA:
    case FMT_JAPANESEYEN:
    case FMT_SPANISHPESETA:
    case FMT_EURO:
        return true;

    default:
        return false;
    }
}
sal_uInt16
LwpNumericFormat::GetDecimalPlaces()
{
    if (IsDecimalPlacesOverridden())
        return cDecimalPlaces;
    return GetDefaultDecimalPlaces(cFormat);
}
void LwpNumericFormat::GetCurrencyStr(LwpNumericFormatSubset aNumber, OUString& aPrefix, OUString& aSuffix, bool bNegtive)
{
    aPrefix = aNumber.GetPrefix();
    aSuffix = aNumber.GetSuffix();

    //Make the default prefix and suffix
    OUString aSymbol = m_aCurrencyInfo.GetCurrencySymbol(cFormat);
    bool bPost = m_aCurrencyInfo.IsSymbolPost(cFormat);
    bool bShowSpace = m_aCurrencyInfo.IsShowSpace(cFormat);
    if ( aNumber.IsDefaultPrefix())
    {
        if (bNegtive)
        {
            aPrefix = "(";
        }
        if (!bPost)
        {
            aPrefix += aSymbol;
            if (bShowSpace)
            {
                aPrefix += " ";
            }
        }
    }
    if ( aNumber.IsDefaultSuffix())
    {
        if (bPost)
        {
            aSuffix = aSymbol;
            if (bShowSpace)
            {
                aSuffix = " " + aSuffix;
            }

        }

        if (bNegtive)
        {
            aSuffix += ")";
        }
    }
}
void LwpNumericFormat::SetNumberType(XFNumberStyle* pStyle)
{
    switch(cFormat)
    {
    case FMT_PERCENT:
        {
            pStyle->SetNumberType(enumXFNumberPercent);
        }
        break;

    case FMT_COMMA:
        {
            pStyle->SetNumberType(enumXFNumberNumber);
            pStyle->SetGroup();
        }
        break;
    case FMT_SCIENTIFIC:
        {
            pStyle->SetNumberType(enumXFNumberScientific);
        }
        break;
    case FMT_FIXED:
    case FMT_GENERAL:
        {
            pStyle->SetNumberType(enumXFNumberNumber);
        }
        break;
    default://including text type, which is not a style of number format in SODC
        {
            pStyle->SetNumberType(enumXFText);
        }
        break;
    }
}
/**
*   Make the xml content of number format
*/
XFStyle* LwpNumericFormat::Convert()
{
    XFNumberStyle* pStyle = new XFNumberStyle;
    OUString aPrefix, aSuffix,aNegPrefix,aNegSuffix;
    LwpColor aColor, aNegativeColor;

    if (IsCurrencyFormat(cFormat))
    {
        pStyle->SetNumberType(enuMXFNumberCurrency);
        pStyle->SetGroup();
        GetCurrencyStr(cAnyNumber, aPrefix, aSuffix);
        GetCurrencyStr(cNegative, aNegPrefix, aNegSuffix,true);
    }
    else
    {
        SetNumberType(pStyle);
        {//Anynumber
            aPrefix     = cAnyNumber.GetPrefix();
            //Set suffix
            aSuffix     = cAnyNumber.GetSuffix();
            //Set color
            aColor  = cAnyNumber.GetColor();
        }

        if (!IsNegativeOverridden())
        {
            aNegPrefix      = aPrefix;
            aNegSuffix      = aSuffix;
            aNegativeColor  = aColor;
        }
        else
        {//negative
            aNegPrefix      = cNegative.GetPrefix();
            aNegSuffix      = cNegative.GetSuffix();
            aNegativeColor  = cNegative.GetColor();
        }
        if (FMT_COMMA==cFormat)
        {
            if (cNegative.IsDefaultPrefix() && aNegPrefix.isEmpty())
            {
                aNegPrefix = "(";
            }
            if (cNegative.IsDefaultSuffix() && aNegSuffix.isEmpty())
            {
                aNegSuffix = ")";
            }
        }

    }

    pStyle->SetDecimalDigits(GetDecimalPlaces());

    aPrefix = reencode(aPrefix);
    aSuffix = reencode(aSuffix);
    aNegPrefix = reencode(aNegPrefix);
    aNegSuffix = reencode(aNegSuffix);

    {//Anynumber
        //Set prefix
        pStyle->SetPrefix(aPrefix);
        //Set suffix
        pStyle->SetSurfix(aSuffix);
        pStyle->SetColor( XFColor( static_cast<sal_uInt8>(aColor.GetRed()),
                                   static_cast<sal_uInt8>(aColor.GetGreen()),
                                   static_cast<sal_uInt8>(aColor.GetBlue())) );
    }
    {//Negtive
        pStyle->SetNegativeStyle( aNegPrefix, aNegSuffix, XFColor(static_cast<sal_uInt8>(aNegativeColor.GetRed()),
                                                                    static_cast<sal_uInt8>(aNegativeColor.GetGreen()),
                                                                    static_cast<sal_uInt8>(aNegativeColor.GetBlue())) );
    }

    return pStyle;
}
/**
*
*   @description for SODC_2754
*   @return fix wrong encoding of POUND symbol
*/
OUString    LwpNumericFormat::reencode(const OUString& sCode)
{
    const sal_Unicode * pString = sCode.getStr();
    sal_uInt16 nLen = sCode.getLength();
    bool bFound = false;
    sal_uInt16 i;
    std::unique_ptr<sal_Unicode[]> pBuff( new sal_Unicode[sCode.getLength()] );

    for (i=0; i< sCode.getLength() - 1; i++)
    {
        if ( (pString[i] == 0x00a1) && (pString[i+1] == 0x00ea))
        {
            bFound = true;
            break;
        }
        pBuff[i] = pString[i];
    }
    if (bFound)
    {
        pBuff[i] = 0xffe1;
        for (sal_Int32 j=i+1; j < sCode.getLength() - 1; ++j)
        {
            pBuff[j] = pString[j+1];
        }
        OUString sRet(pBuff.get(), nLen - 1);
        return sRet;
    }

    return sCode;
}

sal_uInt16
LwpNumericFormat::GetDefaultDecimalPlaces(sal_uInt16 Format)
{
    switch (Format)
    {
    case FMT_ARGENTINEANPESO:
    case FMT_AUSTRALIANDOLLAR:
    case FMT_AUSTRIANSCHILLING:
    case FMT_BELGIANFRANC:
    case FMT_BRAZILIANCRUZEIRO:
    case FMT_BRITISHPOUND:
    case FMT_CANADIANDOLLAR:
    case FMT_CHINESEYUAN:
    case FMT_CZECHKORUNA:
    case FMT_DANISHKRONE:
    case FMT_ECU:
    case FMT_FINNISHMARKKA:
    case FMT_FRENCHFRANC:
    case FMT_GERMANMARK:
    case FMT_HONGKONGDOLLAR:
    case FMT_HUNGARIANFORINT:
    case FMT_INDIANRUPEE:
    case FMT_INDONESIANRUPIAH:
    case FMT_IRISHPUNT:
    case FMT_LUXEMBOURGFRANC:
    case FMT_MALAYSIANRINGGIT:
    case FMT_MEXICANPESO:
    case FMT_NETHERLANDSGUILDER:
    case FMT_NEWZEALANDDOLLAR:
    case FMT_NORWEGIANKRONE:
    case FMT_POLISHZLOTY:
    case FMT_PORTUGUESEESCUDO:
    case FMT_ROMANIANLEI:
    case FMT_RUSSIANRUBLE:
    case FMT_SINGAPOREDOLLAR:
    case FMT_SLOVAKIANKORUNA:
    case FMT_SLOVENIANTHOLAR:
    case FMT_SOUTHAFRICANRAND:
    case FMT_SOUTHKOREANWON:
    case FMT_SWEDISHKRONA:
    case FMT_SWISSFRANC:
    case FMT_TAIWANDOLLAR:
    case FMT_THAIBAHT:
    case FMT_USDOLLAR:
    case FMT_OTHERCURRENCY:
    case FMT_EURO:
        return 2;

    case FMT_GREEKDRACHMA:
    case FMT_ITALIANLIRA:
    case FMT_JAPANESEYEN:
    case FMT_SPANISHPESETA:
        return 0;

    case FMT_DEFAULT:
    case FMT_GENERAL:
    case FMT_FIXED:
    case FMT_COMMA:
    case FMT_PERCENT:
    case FMT_SCIENTIFIC:
    default:
        return 2;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
