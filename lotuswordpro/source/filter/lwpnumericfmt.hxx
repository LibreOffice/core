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

#include "lwpatomholder.hxx"
#include "lwptblcell.hxx"
#include "lwpcolor.hxx"
#include "lwppiece.hxx"

//For converting to xml
#include "xfilter/xfnumberstyle.hxx"

class LwpObjectStream;

class LwpNumericFormatSubset
{
public:
    LwpNumericFormatSubset();
    ~LwpNumericFormatSubset();
    void QuickRead(LwpObjectStream* pStrm);
    OUString GetPrefix(){ return cPrefix.str();}
    OUString GetSuffix(){ return cSuffix.str();}
    bool IsDefaultPrefix(){ return !(cSubFlags&SF_OVER_PREFIX); }
    bool IsDefaultSuffix(){ return !(cSubFlags&SF_OVER_SUFFIX); }
    LwpColor GetColor();

protected:
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
    explicit LwpCurrencyInfo(const OUString& sSym)
    {
        sSymbol = sSym;
        bPost = false;
        bShowSpace = false;
    }
    LwpCurrencyInfo()
    {
        bPost = false;
        bShowSpace = false;
    }
    LwpCurrencyInfo(const OUString& sSym, bool bPost_, bool bShowSpace_)
    {
        sSymbol = sSym;
        bPost = bPost_;
        bShowSpace = bShowSpace_;
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

#define RTL_CONSTUTF8_USTRINGPARAM( constAsciiStr ) (&(constAsciiStr)[0]), \
    ((sal_Int32)(SAL_N_ELEMENTS(constAsciiStr)-1)), RTL_TEXTENCODING_UTF8

class LwpCurrencyPool
{
public:
    LwpCurrencyPool(){InitCurrencySymbol();}
    OUString GetCurrencySymbol(sal_uInt16 nFormat);
    bool IsShowSpace(sal_uInt16 nFormat);
    bool IsSymbolPost(sal_uInt16 nFormat);

private:
    std::map<sal_uInt16,LwpCurrencyInfo> m_aCurrencyInfo;
    void InitCurrencySymbol()
    {
        sal_uInt16 nC=FMT_ARGENTINEANPESO;
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("A");                    //FMT_ARGENTINEANPESO       = 1,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("A$");                   //FMT_AUSTRALIANDOLLAR  = 2,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("oS",true, true);//FMT_AUSTRIANSCHILLING = 3,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("BF",true, true);//FMT_BELGIANFRANC      = 4,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("R$",false, true);//FMT_BRAZILIANCRUZEIRO    = 5,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString(RTL_CONSTUTF8_USTRINGPARAM("\357\277\241")));                 //FMT_BRITISHPOUND      = 6,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("C$");                   //FMT_CANADIANDOLLAR        = 7,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString(RTL_CONSTUTF8_USTRINGPARAM("PRC\357\277\245")),false,true);   //FMT_CHINESEYUAN           = 8,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Kc",true, true);//FMT_CZECHKORUNA           = 9,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Dkr",false, true);//FMT_DANISHKRONE         = 10,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("ECU",true, true);//FMT_ECU                  = 11,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("mk",true, true);//FMT_FINNISHMARKKA     = 12,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("F",true, true);//FMT_FRENCHFRANC            = 13,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("DM",true, true);//FMT_GERMANMARK            = 14,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Dr",true, true);//FMT_GREEKDRACHMA      = 15,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("HK$");                  //FMT_HONGKONGDOLLAR        = 16,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Ft",true, true);//FMT_HUNGARIANFORINT       = 17,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Rs",false, true);//FMT_INDIANRUPEE          = 18,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Rp",false, true);//FMT_INDONESIANRUPIAH = 19,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString(RTL_CONSTUTF8_USTRINGPARAM("IR\357\277\241")));                   //FMT_IRISHPUNT         = 20,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("L.",false, true);//FMT_ITALIANLIRA          = 21,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString(RTL_CONSTUTF8_USTRINGPARAM("\357\277\245")));             //FMT_JAPANESEYEN           = 22,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("LF",true, true);//FMT_LUXEMBOURGFRANC       = 23,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Rm",false, true);//FMT_MALAYSIANRINGGIT = 24,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Mex$");                 //FMT_MEXICANPESO           = 25,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("F",false, true);//FMT_NETHERLANDSGUILDER    = 26,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("NZ$");                  //FMT_NEWZEALANDDOLLAR  = 27,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Nkr",false, true);//FMT_NORWEGIANKRONE      = 28,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Zl",true, true);//FMT_POLISHZLOTY           = 29,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Esc.",true, true);//FMT_PORTUGUESEESCUDO    = 30,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Leu",true, true);//FMT_ROMANIANLEI          = 31,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("R",true, true);//FMT_RUSSIANRUBLE       = 32,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("S$");                   //FMT_SINGAPOREDOLLAR       = 33,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Sk",true, true);//FMT_SLOVAKIANKORUNA       = 34,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("SIT",false, true);//FMT_SLOVENIANTHOLAR     = 35,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("R");                    //FMT_SOUTHAFRICANRAND  = 36,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("W");                    //FMT_SOUTHKOREANWON        = 37,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Pts",true, true);//FMT_SPANISHPESETA        = 38,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Skr",true, true);//FMT_SWEDISHKRONA     = 39,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("SFr",false, true);//FMT_SWISSFRANC          = 40,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("NT$");                  //FMT_TAIWANDOLLAR      = 41,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("Bt",true, true);//FMT_THAIBAHT          = 42,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("$");                    //FMT_USDOLLAR          = 43,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo("OTH",false, true);//FMT_OTHERCURRENCY       = 44,

        m_aCurrencyInfo[FMT_EURO]=LwpCurrencyInfo(OUString(RTL_CONSTUTF8_USTRINGPARAM("\342\202\254")));             //FMT_EURO              = 52
    }
};

class LwpNumericFormat
{
public:
    explicit LwpNumericFormat(LwpObjectStream * pStrm);
    void Read();
    static bool IsCurrencyFormat(sal_uInt16 Format);
    sal_uInt16 GetDecimalPlaces();
    bool IsDecimalPlacesOverridden();
    bool IsNegativeOverridden();
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

    void GetCurrencyStr(LwpNumericFormatSubset aNumber, OUString& aPrefix, OUString& aSuffix, bool bNegtive=false);
    void SetNumberType(XFNumberStyle* pStyle);
    static OUString reencode(const OUString& sCode);
};

inline bool
LwpNumericFormat::IsDecimalPlacesOverridden()
{
    return (cFlags & NF_OVER_DECIMAL_PLACES) != 0;
}

inline bool
LwpNumericFormat::IsNegativeOverridden()
{
    return (cFlags & NF_OVER_NEGATIVE) != 0;
}

class LwpLayoutNumerics : public LwpVirtualPiece
{
public:
    LwpLayoutNumerics(LwpObjectHeader const & objHdr, LwpSvStream* pStrm)
    :LwpVirtualPiece(objHdr, pStrm),cNumerics(m_pObjStrm.get()){}
    XFStyle* Convert();
    virtual void Read() override;

protected:
    LwpNumericFormat cNumerics;

private:
    virtual ~LwpLayoutNumerics() override {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
