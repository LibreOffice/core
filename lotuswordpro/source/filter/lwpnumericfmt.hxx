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

#ifndef _LWPNUMBERICFMT_HXX
#define _LWPNUMBERICFMT_HXX

#include "lwpatomholder.hxx"
#include "lwptblcell.hxx"
#include "lwpcolor.hxx"

//For converting to xml
#include "xfilter/xfnumberstyle.hxx"

class LwpObjectStream;

class LwpNumericFormatSubset
{
public:
    LwpNumericFormatSubset();
    ~LwpNumericFormatSubset();
    void QuickRead(LwpObjectStream* pStrm);
    String GetPrefix(){ return cPrefix.str();}
    String GetSuffix(){ return cSuffix.str();}
    sal_Bool IsBlack(){ return (cColor.GetBlue()==0 && cColor.GetGreen()==0 && cColor.GetRed()==0);}
    sal_Bool IsDefaultPrefix(){ return !(cSubFlags&SF_OVER_PREFIX); }
    sal_Bool IsDefaultSuffix(){ return !(cSubFlags&SF_OVER_SUFFIX); }
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
    String sSymbol;
    sal_Bool bPost;
    sal_Bool bShowSpace;
    LwpCurrencyInfo(String sSym)
    {
        sSymbol = sSym;
        bPost = sal_False;
        bShowSpace = sal_False;
    }
    LwpCurrencyInfo()
    {
        bPost = sal_False;
        bShowSpace = sal_False;
    }
    LwpCurrencyInfo(String sSym,sal_Bool bPost_, sal_Bool bShowSpace_)
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

class LwpCurrencyPool
{
public:
    LwpCurrencyPool(){InitCurrencySymbol();}
    String GetCurrencySymbol(sal_uInt16 nFormat);
    sal_Bool IsShowSpace(sal_uInt16 nFormat);
    sal_Bool IsSymbolPost(sal_uInt16 nFormat);

private:
    std::map<sal_uInt16,LwpCurrencyInfo> m_aCurrencyInfo;
    void InitCurrencySymbol()
    {
        sal_uInt16 nC=FMT_ARGENTINEANPESO;
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("A"));                    //FMT_ARGENTINEANPESO       = 1,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("A$"));                   //FMT_AUSTRALIANDOLLAR  = 2,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("oS"),sal_True, sal_True);//FMT_AUSTRIANSCHILLING = 3,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("BF"),sal_True, sal_True);//FMT_BELGIANFRANC      = 4,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("R$"),sal_False, sal_True);//FMT_BRAZILIANCRUZEIRO    = 5,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(String("\357\277\241",RTL_TEXTENCODING_UTF8));                 //FMT_BRITISHPOUND      = 6,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("C$"));                   //FMT_CANADIANDOLLAR        = 7,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(String("PRC\357\277\245",RTL_TEXTENCODING_UTF8),sal_False,sal_True);   //FMT_CHINESEYUAN           = 8,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Kc"),sal_True, sal_True);//FMT_CZECHKORUNA           = 9,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Dkr"),sal_False, sal_True);//FMT_DANISHKRONE         = 10,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("ECU"),sal_True, sal_True);//FMT_ECU                  = 11,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("mk"),sal_True, sal_True);//FMT_FINNISHMARKKA     = 12,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("F"),sal_True, sal_True);//FMT_FRENCHFRANC            = 13,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("DM"),sal_True, sal_True);//FMT_GERMANMARK            = 14,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Dr"),sal_True, sal_True);//FMT_GREEKDRACHMA      = 15,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("HK$"));                  //FMT_HONGKONGDOLLAR        = 16,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Ft"),sal_True, sal_True);//FMT_HUNGARIANFORINT       = 17,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Rs"),sal_False, sal_True);//FMT_INDIANRUPEE          = 18,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Rp"),sal_False, sal_True);//FMT_INDONESIANRUPIAH = 19,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(String("IR\357\277\241",RTL_TEXTENCODING_UTF8));                   //FMT_IRISHPUNT         = 20,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("L."),sal_False, sal_True);//FMT_ITALIANLIRA          = 21,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(String("\357\277\245",RTL_TEXTENCODING_UTF8));             //FMT_JAPANESEYEN           = 22,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("LF"),sal_True, sal_True);//FMT_LUXEMBOURGFRANC       = 23,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Rm"),sal_False, sal_True);//FMT_MALAYSIANRINGGIT = 24,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Mex$"));                 //FMT_MEXICANPESO           = 25,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("F"),sal_False, sal_True);//FMT_NETHERLANDSGUILDER    = 26,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("NZ$"));                  //FMT_NEWZEALANDDOLLAR  = 27,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Nkr"),sal_False, sal_True);//FMT_NORWEGIANKRONE      = 28,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Zl"),sal_True, sal_True);//FMT_POLISHZLOTY           = 29,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Esc."),sal_True, sal_True);//FMT_PORTUGUESEESCUDO    = 30,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Leu"),sal_True, sal_True);//FMT_ROMANIANLEI          = 31,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("R"),sal_True, sal_True);//FMT_RUSSIANRUBLE       = 32,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("S$"));                   //FMT_SINGAPOREDOLLAR       = 33,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Sk"),sal_True, sal_True);//FMT_SLOVAKIANKORUNA       = 34,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("SIT"),sal_False, sal_True);//FMT_SLOVENIANTHOLAR     = 35,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("R"));                    //FMT_SOUTHAFRICANRAND  = 36,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("W"));                    //FMT_SOUTHKOREANWON        = 37,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Pts"),sal_True, sal_True);//FMT_SPANISHPESETA        = 38,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Skr"),sal_True, sal_True);//FMT_SWEDISHKRONA     = 39,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("SFr"),sal_False, sal_True);//FMT_SWISSFRANC          = 40,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("NT$"));                  //FMT_TAIWANDOLLAR      = 41,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("Bt"),sal_True, sal_True);//FMT_THAIBAHT          = 42,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("$"));                    //FMT_USDOLLAR          = 43,
        m_aCurrencyInfo[nC++]=LwpCurrencyInfo(OUString("OTH"),sal_False, sal_True);//FMT_OTHERCURRENCY       = 44,

        m_aCurrencyInfo[FMT_EURO]=LwpCurrencyInfo(String("\342\202\254",RTL_TEXTENCODING_UTF8));             //FMT_EURO              = 52
    }
};

class LwpNumericFormat
{
public:
    LwpNumericFormat(LwpObjectStream * pStrm);
    ~LwpNumericFormat(){}
    void Read();
    static sal_Bool IsCurrencyFormat(sal_uInt16 Format);
    sal_uInt16 GetDecimalPlaces(void);
    sal_Bool IsDecimalPlacesOverridden(void);
    sal_Bool IsNegativeOverridden(void);
    sal_Bool IsZeroOverridden(void);
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

    void GetCurrencyStr(LwpNumericFormatSubset aNumber, String& aPrefix, String& aSuffix, sal_Bool bNegtive=sal_False);
    void SetNumberType(XFNumberStyle* pStyle);
    OUString    reencode(OUString sCode);
};

inline sal_Bool
LwpNumericFormat::IsDecimalPlacesOverridden(void)
{
    return (cFlags & NF_OVER_DECIMAL_PLACES) != 0;
}

inline sal_Bool
LwpNumericFormat::IsNegativeOverridden(void)
{
    return (cFlags & NF_OVER_NEGATIVE) != 0;
}

inline sal_Bool
LwpNumericFormat::IsZeroOverridden(void)
{
    return (cFlags & NF_OVER_ZERO) != 0;
}

#include "lwppiece.hxx"
class LwpLayoutNumerics : public LwpVirtualPiece
{
public:
    LwpLayoutNumerics(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    :LwpVirtualPiece(objHdr, pStrm),cNumerics(m_pObjStrm){}
    ~LwpLayoutNumerics(){}
    XFStyle* Convert();
    virtual void Read();

protected:
    LwpNumericFormat cNumerics;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
