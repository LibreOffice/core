/*************************************************************************
 *
 *  $RCSfile: fontcfg.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:17:42 $
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

#ifndef _FONTCFG_HXX
#include <fontcfg.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

#define C2S(cChar) String::CreateFromAscii(cChar)
#define C2U(cChar) OUString::createFromAscii(cChar)
/* -----------------07.10.2002 12:15-----------------
 *
 * --------------------------------------------------*/
inline LanguageType lcl_LanguageOfType(sal_Int16 nType, sal_Int16 eWestern, sal_Int16 eCJK, sal_Int16 eCTL)
{
    return LanguageType(
                nType < FONT_STANDARD_CJK ? eWestern :
                    nType >= FONT_STANDARD_CTL ? eCTL : eWestern);
}
/* -----------------------------08.09.00 15:52--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwStdFontConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "DefaultFont/Standard",    // 0
        "DefaultFont/Heading",     // 1
        "DefaultFont/List",        // 2
        "DefaultFont/Caption",     // 3
        "DefaultFont/Index",       // 4
        "DefaultFontCJK/Standard", // 5
        "DefaultFontCJK/Heading",  // 6
        "DefaultFontCJK/List",     // 7
        "DefaultFontCJK/Caption",  // 8
        "DefaultFontCJK/Index",    // 9
        "DefaultFontCTL/Standard", // 10
        "DefaultFontCTL/Heading",  // 11
        "DefaultFontCTL/List",     // 12
        "DefaultFontCTL/Caption",  // 13
        "DefaultFontCTL/Index",    // 14
    };
    const int nCount = 15;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-----------------03.09.96 15.00-------------------

--------------------------------------------------*/

SwStdFontConfig::SwStdFontConfig() :
    utl::ConfigItem(C2U("Office.Writer"))
{
    SvtLinguOptions aLinguOpt;

    // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
    SwLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   eWestern = aLinguOpt.nDefaultLanguage,
                eCJK = aLinguOpt.nDefaultLanguage_CJK,
                eCTL = aLinguOpt.nDefaultLanguage_CTL;
    for(sal_Int16 i = 0; i < DEF_FONT_COUNT; i++)
        sDefaultFonts[i] = GetDefaultFor(i,
            lcl_LanguageOfType(i, eWestern, eCJK, eCTL));

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                OUString sVal;
                pValues[nProp] >>= sVal;
                sDefaultFonts[nProp] = sVal;
            }
        }
    }
}
/* -----------------------------08.09.00 15:58--------------------------------

 ---------------------------------------------------------------------------*/
void    SwStdFontConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    SvtLinguOptions aLinguOpt;

    // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
    SwLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   eWestern = aLinguOpt.nDefaultLanguage,
                eCJK = aLinguOpt.nDefaultLanguage_CJK,
                eCTL = aLinguOpt.nDefaultLanguage_CTL;
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(GetDefaultFor(nProp, lcl_LanguageOfType(nProp, eWestern, eCJK, eCTL)) != sDefaultFonts[nProp])
                pValues[nProp] <<= OUString(sDefaultFonts[nProp]);
    }
    PutProperties(aNames, aValues);
}
/* -----------------------------08.09.00 15:56--------------------------------

 ---------------------------------------------------------------------------*/
SwStdFontConfig::~SwStdFontConfig()
{}
/*-----------------18.01.97 10.05-------------------

--------------------------------------------------*/
BOOL SwStdFontConfig::IsFontDefault(USHORT nFontType) const
{
    BOOL bSame;
    SvtLinguOptions aLinguOpt;

    // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
    SwLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   eWestern = aLinguOpt.nDefaultLanguage,
                eCJK = aLinguOpt.nDefaultLanguage_CJK,
                eCTL = aLinguOpt.nDefaultLanguage_CTL;
    String sDefFont(GetDefaultFor(FONT_STANDARD, eWestern));
    String sDefFontCJK(GetDefaultFor(FONT_STANDARD_CJK, eCJK));
    String sDefFontCTL(GetDefaultFor(FONT_STANDARD_CTL, eCTL));
    LanguageType eLang = lcl_LanguageOfType(nFontType, eWestern, eCJK, eCTL);
    switch( nFontType )
    {
        case FONT_STANDARD:
            bSame = sDefaultFonts[nFontType] == sDefFont;
        break;
        case FONT_STANDARD_CJK:
            bSame = sDefaultFonts[nFontType] == sDefFontCJK;
        break;
        case FONT_STANDARD_CTL:
            bSame = sDefaultFonts[nFontType] == sDefFontCTL;
        break;
        case FONT_OUTLINE :
        case FONT_OUTLINE_CJK :
        case FONT_OUTLINE_CTL :
            bSame = sDefaultFonts[nFontType] ==
                GetDefaultFor(nFontType, eLang);
        break;
        case FONT_LIST    :
        case FONT_CAPTION :
        case FONT_INDEX   :
            bSame = sDefaultFonts[nFontType] == sDefFont &&
                    sDefaultFonts[FONT_STANDARD] == sDefFont;
        break;
        case FONT_LIST_CJK    :
        case FONT_CAPTION_CJK :
        case FONT_INDEX_CJK   :
        {
            BOOL b1 = sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCJK;
            bSame = b1 && sDefaultFonts[nFontType] == sDefFontCJK;
        }
        break;
        case FONT_LIST_CTL    :
        case FONT_CAPTION_CTL :
        case FONT_INDEX_CTL   :
        {
            BOOL b1 = sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCTL;
            bSame = b1 && sDefaultFonts[nFontType] == sDefFontCTL;
        }
        break;
    }
    return bSame;
}

/* -----------------11.01.99 13:16-------------------
 * Standards auslesen
 * --------------------------------------------------*/
String  SwStdFontConfig::GetDefaultFor(USHORT nFontType, LanguageType eLang)
{
    String sRet;
    USHORT nFontId;
    switch( nFontType )
    {
        case FONT_OUTLINE :
            nFontId = DEFAULTFONT_LATIN_HEADING;
        break;
        case FONT_OUTLINE_CJK :
            nFontId = DEFAULTFONT_CJK_HEADING;
        break;
        case FONT_OUTLINE_CTL :
            nFontId = DEFAULTFONT_CTL_HEADING;
        break;
        case FONT_STANDARD_CJK:
        case FONT_LIST_CJK    :
        case FONT_CAPTION_CJK :
        case FONT_INDEX_CJK   :
            nFontId = DEFAULTFONT_CJK_TEXT;
        break;
        case FONT_STANDARD_CTL:
        case FONT_LIST_CTL    :
        case FONT_CAPTION_CTL :
        case FONT_INDEX_CTL   :
            nFontId = DEFAULTFONT_CTL_TEXT;
        break;
//        case FONT_STANDARD:
//        case FONT_LIST    :
//        case FONT_CAPTION :
//        case FONT_INDEX   :
        default:
            nFontId = DEFAULTFONT_LATIN_TEXT;
    }
    Font aFont = OutputDevice::GetDefaultFont(nFontId, eLang, DEFAULTFONT_FLAGS_ONLYONE);
    return  aFont.GetName();
}
