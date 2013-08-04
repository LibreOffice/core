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


#include <htmlcfg.hxx>
#include <parhtml.hxx>
#include <unotools/syslocale.hxx>
#include <tools/debug.hxx>
#include <tools/link.hxx>
#include <sal/macros.h>
#include <rtl/instance.hxx>
#include <list>

// -----------------------------------------------------------------------
#define HTMLCFG_UNKNOWN_TAGS            0x01
//#define HTMLCFG_STYLE_SHEETS          0x02
//#define HTMLCFG_NETSCAPE3             0x04
#define HTMLCFG_STAR_BASIC              0x08
#define HTMLCFG_LOCAL_GRF               0x10
#define HTMLCFG_PRINT_LAYOUT_EXTENSION  0x20
#define HTMLCFG_IGNORE_FONT_FAMILY      0x40
#define HTMLCFG_IS_BASIC_WARNING        0x80
#define HTMLCFG_NUMBERS_ENGLISH_US      0x100

using namespace utl;
using namespace com::sun::star::uno;


struct HtmlOptions_Impl
{
    ::std::list<Link> aList;
    sal_Int32   nFlags;
    sal_Int32   nExportMode;
    sal_Int32   aFontSizeArr[HTML_FONT_COUNT];
    sal_Int32   eEncoding;
    sal_Bool    bIsEncodingDefault;

    HtmlOptions_Impl() :
        nFlags(HTMLCFG_LOCAL_GRF|HTMLCFG_IS_BASIC_WARNING),
        nExportMode(HTML_CFG_NS40),
        eEncoding( osl_getThreadTextEncoding() ),
        bIsEncodingDefault(sal_True)
    {
        aFontSizeArr[0] = HTMLFONTSZ1_DFLT;
        aFontSizeArr[1] = HTMLFONTSZ2_DFLT;
        aFontSizeArr[2] = HTMLFONTSZ3_DFLT;
        aFontSizeArr[3] = HTMLFONTSZ4_DFLT;
        aFontSizeArr[4] = HTMLFONTSZ5_DFLT;
        aFontSizeArr[5] = HTMLFONTSZ6_DFLT;
        aFontSizeArr[6] = HTMLFONTSZ7_DFLT;
    }
};

const Sequence<OUString>& SvxHtmlOptions::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "Import/UnknownTag",                    //  0
            "Import/FontSetting",                   //  1
            "Import/FontSize/Size_1",               //  2
            "Import/FontSize/Size_2",               //  3
            "Import/FontSize/Size_3",               //  4
            "Import/FontSize/Size_4",               //  5
            "Import/FontSize/Size_5",               //  6
            "Import/FontSize/Size_6",               //  7
            "Import/FontSize/Size_7",               //  8
            "Export/Browser",                       //  9
            "Export/Basic",                         //  0
            "Export/PrintLayout",                   // 11
            "Export/LocalGraphic",                  // 12
            "Export/Warning",                       // 13
            "Export/Encoding",                      // 14
            "Import/NumbersEnglishUS"               // 15
        };
        const int nCount = SAL_N_ELEMENTS(aPropNames);
        aNames.realloc(nCount);
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
// -----------------------------------------------------------------------
SvxHtmlOptions::SvxHtmlOptions() :
    ConfigItem("Office.Common/Filter/HTML")
{
    pImp = new HtmlOptions_Impl;
    Load( GetPropertyNames() );
}

// -----------------------------------------------------------------------
SvxHtmlOptions::~SvxHtmlOptions()
{
    delete pImp;
}

void SvxHtmlOptions::Load( const Sequence< OUString >& aNames )
{
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        pImp->nFlags = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_UNKNOWN_TAGS;
                    break;//"Import/UnknownTag",
                    case  1:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_IGNORE_FONT_FAMILY;
                    break;//"Import/FontSetting",
                    case  2: pValues[nProp] >>= pImp->aFontSizeArr[0]; break;//"Import/FontSize/Size_1",
                    case  3: pValues[nProp] >>= pImp->aFontSizeArr[1]; break;//"Import/FontSize/Size_2",
                    case  4: pValues[nProp] >>= pImp->aFontSizeArr[2]; break;//"Import/FontSize/Size_3",
                    case  5: pValues[nProp] >>= pImp->aFontSizeArr[3]; break;//"Import/FontSize/Size_4",
                    case  6: pValues[nProp] >>= pImp->aFontSizeArr[4]; break;//"Import/FontSize/Size_5",
                    case  7: pValues[nProp] >>= pImp->aFontSizeArr[5]; break;//"Import/FontSize/Size_6",
                    case  8: pValues[nProp] >>= pImp->aFontSizeArr[6]; break;//"Import/FontSize/Size_7",
                    case  9://"Export/Browser",
                        {
                            sal_Int32 nExpMode = 0;
                            pValues[nProp] >>= nExpMode;
                            switch( nExpMode )
                            {
                                case 1:     nExpMode = HTML_CFG_MSIE;    break;
                                case 3:     nExpMode = HTML_CFG_WRITER;     break;
                                case 4:     nExpMode = HTML_CFG_NS40;       break;
                                default:    nExpMode = HTML_CFG_NS40;       break;
                            }

                            pImp->nExportMode = nExpMode;
                        }
                        break;
                    case 10:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_STAR_BASIC;
                    break;//"Export/Basic",
                    case 11:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_PRINT_LAYOUT_EXTENSION;
                    break;//"Export/PrintLayout",
                    case 12:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_LOCAL_GRF;
                    break;//"Export/LocalGraphic",
                    case 13:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_IS_BASIC_WARNING;
                    break;//"Export/Warning"

                    case 14: pValues[nProp] >>= pImp->eEncoding;
                             pImp->bIsEncodingDefault = sal_False;
                    break;//"Export/Encoding"

                    case 15:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_NUMBERS_ENGLISH_US;
                    break;//"Import/NumbersEnglishUS"
                }
            }
        }
    }
}

// -----------------------------------------------------------------------
void    SvxHtmlOptions::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet = sal_False;
        switch(nProp)
        {
            case  0: bSet = 0 != (pImp->nFlags & HTMLCFG_UNKNOWN_TAGS);break;//"Import/UnknownTag",
            case  1: bSet = 0 != (pImp->nFlags & HTMLCFG_IGNORE_FONT_FAMILY);break;//"Import/FontSetting",
            case  2: pValues[nProp] <<= pImp->aFontSizeArr[0];break;//"Import/FontSize/Size_1",
            case  3: pValues[nProp] <<= pImp->aFontSizeArr[1];break;//"Import/FontSize/Size_2",
            case  4: pValues[nProp] <<= pImp->aFontSizeArr[2];break;//"Import/FontSize/Size_3",
            case  5: pValues[nProp] <<= pImp->aFontSizeArr[3];break;//"Import/FontSize/Size_4",
            case  6: pValues[nProp] <<= pImp->aFontSizeArr[4];break;//"Import/FontSize/Size_5",
            case  7: pValues[nProp] <<= pImp->aFontSizeArr[5];break;//"Import/FontSize/Size_6",
            case  8: pValues[nProp] <<= pImp->aFontSizeArr[6];break;//"Import/FontSize/Size_7",
            case  9:                //"Export/Browser",
                {
                    sal_Int32 nExpMode = pImp->nExportMode;

                    switch( nExpMode )
                    {
                        case HTML_CFG_MSIE:      nExpMode = 1;   break;
                        case HTML_CFG_WRITER:       nExpMode = 3;   break;
                        case HTML_CFG_NS40:         nExpMode = 4;   break;
                        default:                    nExpMode = 4;   break;  // NS40
                    }

                    pValues[nProp] <<= nExpMode;
                    break;
                }
            case 10: bSet = 0 != (pImp->nFlags & HTMLCFG_STAR_BASIC);break;//"Export/Basic",
            case 11: bSet = 0 != (pImp->nFlags & HTMLCFG_PRINT_LAYOUT_EXTENSION);break;//"Export/PrintLayout",
            case 12: bSet = 0 != (pImp->nFlags & HTMLCFG_LOCAL_GRF);break;//"Export/LocalGraphic",
            case 13: bSet = 0 != (pImp->nFlags & HTMLCFG_IS_BASIC_WARNING);break;//"Export/Warning"
            case 14:
                if(!pImp->bIsEncodingDefault)
                    pValues[nProp] <<= pImp->eEncoding;
                break;//"Export/Encoding",
            case 15: bSet = 0 != (pImp->nFlags & HTMLCFG_NUMBERS_ENGLISH_US);break;//"Import/NumbersEnglishUS"
        }
        if(nProp < 2 || ( nProp > 9 && nProp < 14 ) || nProp == 15)
            pValues[nProp].setValue(&bSet, ::getCppuBooleanType());
    }
    PutProperties(aNames, aValues);
}

void SvxHtmlOptions::CallListeners()
{
    for ( ::std::list<Link>::const_iterator iter = pImp->aList.begin(); iter != pImp->aList.end(); ++iter )
        iter->Call( this );
}


void SvxHtmlOptions::Notify( const com::sun::star::uno::Sequence< OUString >& )
{
    Load( GetPropertyNames() );
    CallListeners();
}

// -----------------------------------------------------------------------
sal_uInt16  SvxHtmlOptions::GetFontSize(sal_uInt16 nPos) const
{
    if(nPos < HTML_FONT_COUNT)
        return (sal_uInt16)pImp->aFontSizeArr[nPos];
    return 0;
}
// -----------------------------------------------------------------------
void SvxHtmlOptions::SetFontSize(sal_uInt16 nPos, sal_uInt16 nSize)
{
    if(nPos < HTML_FONT_COUNT)
    {
        pImp->aFontSizeArr[nPos] = nSize;
        SetModified();
    }
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------


sal_Bool SvxHtmlOptions::IsImportUnknown() const
{
    return 0 != (pImp->nFlags & HTMLCFG_UNKNOWN_TAGS) ;
}

// -----------------------------------------------------------------------


void SvxHtmlOptions::SetImportUnknown(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |= HTMLCFG_UNKNOWN_TAGS;
    else
        pImp->nFlags &= ~HTMLCFG_UNKNOWN_TAGS;
    SetModified();
}

// -----------------------------------------------------------------------


sal_uInt16  SvxHtmlOptions::GetExportMode() const
{
    return (sal_uInt16)pImp->nExportMode;
}

// -----------------------------------------------------------------------


void SvxHtmlOptions::SetExportMode(sal_uInt16 nSet)
{
    if(nSet <= HTML_CFG_MAX )
    {
        pImp->nExportMode = nSet;
        SetModified();
        CallListeners();
    }
}

// -----------------------------------------------------------------------


sal_Bool SvxHtmlOptions::IsStarBasic() const
{
    return 0 != (pImp->nFlags & HTMLCFG_STAR_BASIC) ;
}

// -----------------------------------------------------------------------


void SvxHtmlOptions::SetStarBasic(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_STAR_BASIC;
    else
        pImp->nFlags &= ~HTMLCFG_STAR_BASIC;
    SetModified();
}

sal_Bool SvxHtmlOptions::IsSaveGraphicsLocal() const
{
    return 0 != (pImp->nFlags & HTMLCFG_LOCAL_GRF) ;
}

void SvxHtmlOptions::SetSaveGraphicsLocal(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_LOCAL_GRF;
    else
        pImp->nFlags &= ~HTMLCFG_LOCAL_GRF;
    SetModified();
}

sal_Bool    SvxHtmlOptions::IsPrintLayoutExtension() const
{
    sal_Bool bRet = 0 != (pImp->nFlags & HTMLCFG_PRINT_LAYOUT_EXTENSION);
    switch( pImp->nExportMode )
    {
        case HTML_CFG_MSIE:
        case HTML_CFG_NS40  :
        case HTML_CFG_WRITER :
        break;
        default:
            bRet = sal_False;
    }
    return bRet;
}

void    SvxHtmlOptions::SetPrintLayoutExtension(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_PRINT_LAYOUT_EXTENSION;
    else
        pImp->nFlags &= ~HTMLCFG_PRINT_LAYOUT_EXTENSION;
    SetModified();
}

sal_Bool SvxHtmlOptions::IsIgnoreFontFamily() const
{
    return 0 != (pImp->nFlags & HTMLCFG_IGNORE_FONT_FAMILY) ;
}

void SvxHtmlOptions::SetIgnoreFontFamily(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_IGNORE_FONT_FAMILY;
    else
        pImp->nFlags &= ~HTMLCFG_IGNORE_FONT_FAMILY;
    SetModified();
}

sal_Bool SvxHtmlOptions::IsStarBasicWarning() const
{
    return 0 != (pImp->nFlags & HTMLCFG_IS_BASIC_WARNING) ;
}

void SvxHtmlOptions::SetStarBasicWarning(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_IS_BASIC_WARNING;
    else
        pImp->nFlags &= ~HTMLCFG_IS_BASIC_WARNING;
    SetModified();
}

rtl_TextEncoding SvxHtmlOptions::GetTextEncoding() const
{
    rtl_TextEncoding eRet;
    if(pImp->bIsEncodingDefault)
        eRet = SvtSysLocale::GetBestMimeEncoding();
    else
        eRet = (rtl_TextEncoding)pImp->eEncoding;
    return eRet;
}

void SvxHtmlOptions::SetTextEncoding( rtl_TextEncoding eEnc )
{
    pImp->eEncoding = eEnc;
    pImp->bIsEncodingDefault = sal_False;
    SetModified();
}

sal_Bool SvxHtmlOptions::IsDefaultTextEncoding() const
{
    return pImp->bIsEncodingDefault;
}

namespace
{
    class theSvxHtmlOptions : public rtl::Static<SvxHtmlOptions, theSvxHtmlOptions> {};
}

SvxHtmlOptions& SvxHtmlOptions::Get()
{
    return theSvxHtmlOptions::get();
}

sal_Bool SvxHtmlOptions::IsNumbersEnglishUS() const
{
    return 0 != (pImp->nFlags & HTMLCFG_NUMBERS_ENGLISH_US) ;
}

void SvxHtmlOptions::SetNumbersEnglishUS(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_NUMBERS_ENGLISH_US;
    else
        pImp->nFlags &= ~HTMLCFG_NUMBERS_ENGLISH_US;
    SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
