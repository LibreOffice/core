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
 *  For LWP filter architecture prototype - table object
 */

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPNUMERICFMT_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPNUMERICFMT_HXX

#include <map>

#include <lwpatomholder.hxx>
#include <lwpcolor.hxx>
#include "lwppiece.hxx"

//For converting to xml
#include <utility>
#include <xfilter/xfnumberstyle.hxx>

class LwpObjectStream;

class LwpNumericFormatSubset final
{
public:
    LwpNumericFormatSubset();
    void QuickRead(LwpObjectStream* pStrm);
    OUString const & GetPrefix() const { return cPrefix.str();}
    OUString const & GetSuffix() const { return cSuffix.str();}
    bool IsDefaultPrefix() const { return !(cSubFlags&SF_OVER_PREFIX); }
    bool IsDefaultSuffix() const { return !(cSubFlags&SF_OVER_SUFFIX); }
    LwpColor GetColor() const;

private:
    LwpColor cColor;
    LwpAtomHolder cPrefix;
    LwpAtomHolder cSuffix;
    sal_uInt16 cSubFlags;

    enum // for cSubFlags
    {
        SF_OVER_PREFIX      = 0x0001,
        SF_OVER_SUFFIX      = 0x0002,
        SF_OVER_COLOR       = 0x0004
    };
};

struct LwpCurrencyInfo
{
    OUString sSymbol;
    bool bPost;
    bool bShowSpace;
    explicit LwpCurrencyInfo(OUString sSym)
      : sSymbol(std::move(sSym)), bPost(false), bShowSpace(false)
    {
    }
    LwpCurrencyInfo() : bPost(false), bShowSpace(false)
    {
    }
    LwpCurrencyInfo(OUString sSym, bool bPost_, bool bShowSpace_)
        : sSymbol(std::move(sSym)), bPost(bPost_), bShowSpace(bShowSpace_)
    {
    }
};

enum
{
    /* These are types of formats.  They are mutually exclusive.
    */
    FMT_NONE                = 0,
    FMT_ARGENTINEANPESO     = 1,
    FMT_AUSTRALIANDOLLAR    = 2,
    FMT_AUSTRIANSCHILLING   = 3,
    FMT_BELGIANFRANC        = 4,
    FMT_BRAZILIANCRUZEIRO   = 5,
    FMT_BRITISHPOUND        = 6,
    FMT_CANADIANDOLLAR      = 7,
    FMT_CHINESEYUAN         = 8,
    FMT_CZECHKORUNA         = 9,
    FMT_DANISHKRONE         = 10,
    FMT_ECU                 = 11,
    FMT_FINNISHMARKKA       = 12,
    FMT_FRENCHFRANC         = 13,
    FMT_GERMANMARK          = 14,
    FMT_GREEKDRACHMA        = 15,
    FMT_HONGKONGDOLLAR      = 16,
    FMT_HUNGARIANFORINT     = 17,
    FMT_INDIANRUPEE         = 18,
    FMT_INDONESIANRUPIAH    = 19,
    FMT_IRISHPUNT           = 20,
    FMT_ITALIANLIRA         = 21,
    FMT_JAPANESEYEN         = 22,
    FMT_LUXEMBOURGFRANC     = 23,
    FMT_MALAYSIANRINGGIT    = 24,
    FMT_MEXICANPESO         = 25,
    FMT_NETHERLANDSGUILDER  = 26,
    FMT_NEWZEALANDDOLLAR    = 27,
    FMT_NORWEGIANKRONE      = 28,
    FMT_POLISHZLOTY         = 29,
    FMT_PORTUGUESEESCUDO    = 30,
    FMT_ROMANIANLEI         = 31,
    FMT_RUSSIANRUBLE        = 32,
    FMT_SINGAPOREDOLLAR     = 33,
    FMT_SLOVAKIANKORUNA     = 34,
    FMT_SLOVENIANTHOLAR     = 35,
    FMT_SOUTHAFRICANRAND    = 36,
    FMT_SOUTHKOREANWON      = 37,
    FMT_SPANISHPESETA       = 38,
    FMT_SWEDISHKRONA        = 39,
    FMT_SWISSFRANC          = 40,
    FMT_TAIWANDOLLAR        = 41,
    FMT_THAIBAHT            = 42,
    FMT_USDOLLAR            = 43,
    FMT_OTHERCURRENCY       = 44,
    FMT_DEFAULT             = 45,
    FMT_GENERAL             = 46,
    FMT_FIXED               = 47,
    FMT_COMMA               = 48,
    FMT_PERCENT             = 49,
    FMT_SCIENTIFIC          = 50,
    FMT_LABEL               = 51,
    FMT_EURO                = 52
};

class LwpCurrencyPool
{
public:
    LwpCurrencyPool(){};
    OUString GetCurrencySymbol(sal_uInt16 nFormat);
    bool IsShowSpace(sal_uInt16 nFormat);
    bool IsSymbolPost(sal_uInt16 nFormat);

private:
    std::map<sal_uInt16,LwpCurrencyInfo> m_aCurrencyInfo
    {
        { FMT_ARGENTINEANPESO, LwpCurrencyInfo(u"A"_ustr) },
        { FMT_AUSTRALIANDOLLAR, LwpCurrencyInfo(u"A$"_ustr) },
        { FMT_AUSTRIANSCHILLING, LwpCurrencyInfo(u"oS"_ustr,true, true) },
        { FMT_BELGIANFRANC, LwpCurrencyInfo(u"BF"_ustr,true, true) },
        { FMT_BRAZILIANCRUZEIRO, LwpCurrencyInfo(u"R$"_ustr,false, true) },
        { FMT_BRITISHPOUND, LwpCurrencyInfo(u"\uFFE1"_ustr) },
        { FMT_CANADIANDOLLAR, LwpCurrencyInfo(u"C$"_ustr) },
        { FMT_CHINESEYUAN, LwpCurrencyInfo(u"PRC\uFFE5"_ustr,false,true) },
        { FMT_CZECHKORUNA, LwpCurrencyInfo(u"Kc"_ustr,true, true) },
        { FMT_DANISHKRONE, LwpCurrencyInfo(u"Dkr"_ustr,false, true) },
        { FMT_ECU, LwpCurrencyInfo(u"ECU"_ustr,true, true) },
        { FMT_FINNISHMARKKA, LwpCurrencyInfo(u"mk"_ustr,true, true) },
        { FMT_FRENCHFRANC, LwpCurrencyInfo(u"F"_ustr,true, true) },
        { FMT_GERMANMARK, LwpCurrencyInfo(u"DM"_ustr,true, true) },
        { FMT_GREEKDRACHMA, LwpCurrencyInfo(u"Dr"_ustr,true, true) },
        { FMT_HONGKONGDOLLAR, LwpCurrencyInfo(u"HK$"_ustr) },
        { FMT_HUNGARIANFORINT, LwpCurrencyInfo(u"Ft"_ustr,true, true) },
        { FMT_INDIANRUPEE, LwpCurrencyInfo(u"Rs"_ustr,false, true) },
        { FMT_INDONESIANRUPIAH, LwpCurrencyInfo(u"Rp"_ustr,false, true) },
        { FMT_IRISHPUNT, LwpCurrencyInfo(u"IR\uFFE1"_ustr) },
        { FMT_ITALIANLIRA, LwpCurrencyInfo(u"L."_ustr,false, true) },
        { FMT_JAPANESEYEN, LwpCurrencyInfo(u"\uFFE5"_ustr) },
        { FMT_LUXEMBOURGFRANC, LwpCurrencyInfo(u"LF"_ustr,true, true) },
        { FMT_MALAYSIANRINGGIT, LwpCurrencyInfo(u"Rm"_ustr,false, true) },
        { FMT_MEXICANPESO, LwpCurrencyInfo(u"Mex$"_ustr) },
        { FMT_NETHERLANDSGUILDER, LwpCurrencyInfo(u"F"_ustr,false, true) },
        { FMT_NEWZEALANDDOLLAR, LwpCurrencyInfo(u"NZ$"_ustr) },
        { FMT_NORWEGIANKRONE, LwpCurrencyInfo(u"Nkr"_ustr,false, true) },
        { FMT_POLISHZLOTY, LwpCurrencyInfo(u"Zl"_ustr,true, true) },
        { FMT_PORTUGUESEESCUDO, LwpCurrencyInfo(u"Esc."_ustr,true, true) },
        { FMT_ROMANIANLEI, LwpCurrencyInfo(u"Leu"_ustr,true, true) },
        { FMT_RUSSIANRUBLE, LwpCurrencyInfo(u"R"_ustr,true, true) },
        { FMT_SINGAPOREDOLLAR, LwpCurrencyInfo(u"S$"_ustr) },
        { FMT_SLOVAKIANKORUNA, LwpCurrencyInfo(u"Sk"_ustr,true, true) },
        { FMT_SLOVENIANTHOLAR, LwpCurrencyInfo(u"SIT"_ustr,false, true) },
        { FMT_SOUTHAFRICANRAND, LwpCurrencyInfo(u"R"_ustr) },
        { FMT_SOUTHKOREANWON, LwpCurrencyInfo(u"W"_ustr) },
        { FMT_SPANISHPESETA, LwpCurrencyInfo(u"Pts"_ustr,true, true) },
        { FMT_SWEDISHKRONA, LwpCurrencyInfo(u"Skr"_ustr,true, true) },
        { FMT_SWISSFRANC, LwpCurrencyInfo(u"SFr"_ustr,false, true) },
        { FMT_TAIWANDOLLAR, LwpCurrencyInfo(u"NT$"_ustr) },
        { FMT_THAIBAHT, LwpCurrencyInfo(u"Bt"_ustr,true, true) },
        { FMT_USDOLLAR, LwpCurrencyInfo(u"$"_ustr) },
        { FMT_OTHERCURRENCY, LwpCurrencyInfo(u"OTH"_ustr,false, true) },
        { FMT_EURO, LwpCurrencyInfo(u"\u20AC"_ustr) }
    };
};

class LwpNumericFormat
{
public:
    explicit LwpNumericFormat(LwpObjectStream * pStrm);
    void Read();
    static bool IsCurrencyFormat(sal_uInt16 Format);
    sal_uInt16 GetDecimalPlaces();
    bool IsDecimalPlacesOverridden() const;
    bool IsNegativeOverridden() const;
    XFStyle* Convert();

private:
    LwpObjectStream * m_pObjStrm;

    sal_uInt16 cFlags;
    enum // for cFlags
    {
        NF_OVER_ZERO            = 0x0001,
        NF_OVER_NEGATIVE        = 0x0002,
        NF_OVER_DECIMAL_PLACES  = 0x0004
    };

    sal_uInt16 cFormat;

    LwpNumericFormatSubset cAnyNumber;
    LwpNumericFormatSubset cZero;
    LwpNumericFormatSubset cNegative;

    sal_uInt16 cDecimalPlaces;

    static sal_uInt16 GetDefaultDecimalPlaces(sal_uInt16 Format);
    static LwpCurrencyPool m_aCurrencyInfo;

    void GetCurrencyStr(LwpNumericFormatSubset aNumber, OUString& aPrefix, OUString& aSuffix, bool bNegative=false);
    void SetNumberType(XFNumberStyle* pStyle);
    static OUString reencode(const OUString& sCode);
};

inline bool
LwpNumericFormat::IsDecimalPlacesOverridden() const
{
    return (cFlags & NF_OVER_DECIMAL_PLACES) != 0;
}

inline bool
LwpNumericFormat::IsNegativeOverridden() const
{
    return (cFlags & NF_OVER_NEGATIVE) != 0;
}

class LwpLayoutNumerics final : public LwpVirtualPiece
{
public:
    LwpLayoutNumerics(LwpObjectHeader const & objHdr, LwpSvStream* pStrm)
    :LwpVirtualPiece(objHdr, pStrm),cNumerics(m_pObjStrm.get()){}
    XFStyle* Convert();
    virtual void Read() override;

private:
    virtual ~LwpLayoutNumerics() override {}

    LwpNumericFormat cNumerics;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
