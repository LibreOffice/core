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

#include <o3tl/any.hxx>
#include <osl/thread.h>
#include <svtools/htmlcfg.hxx>
#include <svtools/parhtml.hxx>
#include <unotools/syslocale.hxx>
#include <tools/debug.hxx>
#include <tools/link.hxx>
#include <sal/macros.h>
#include <rtl/instance.hxx>
#include <list>


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
    sal_Int32   nFlags;
    sal_Int32   nExportMode;
    sal_Int32   aFontSizeArr[HTML_FONT_COUNT];
    sal_Int32   eEncoding;
    bool        bIsEncodingDefault;

    HtmlOptions_Impl() :
        nFlags(HTMLCFG_LOCAL_GRF|HTMLCFG_IS_BASIC_WARNING),
        nExportMode(HTML_CFG_NS40),
        eEncoding( osl_getThreadTextEncoding() ),
        bIsEncodingDefault(true)
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

SvxHtmlOptions::SvxHtmlOptions() :
    ConfigItem("Office.Common/Filter/HTML"),
    pImpl( new HtmlOptions_Impl )
{
    Load( GetPropertyNames() );
}


SvxHtmlOptions::~SvxHtmlOptions()
{
}

void SvxHtmlOptions::Load( const Sequence< OUString >& aNames )
{
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        pImpl->nFlags = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            pImpl->nFlags |= HTMLCFG_UNKNOWN_TAGS;
                    break;//"Import/UnknownTag",
                    case  1:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            pImpl->nFlags |= HTMLCFG_IGNORE_FONT_FAMILY;
                    break;//"Import/FontSetting",
                    case  2: pValues[nProp] >>= pImpl->aFontSizeArr[0]; break;//"Import/FontSize/Size_1",
                    case  3: pValues[nProp] >>= pImpl->aFontSizeArr[1]; break;//"Import/FontSize/Size_2",
                    case  4: pValues[nProp] >>= pImpl->aFontSizeArr[2]; break;//"Import/FontSize/Size_3",
                    case  5: pValues[nProp] >>= pImpl->aFontSizeArr[3]; break;//"Import/FontSize/Size_4",
                    case  6: pValues[nProp] >>= pImpl->aFontSizeArr[4]; break;//"Import/FontSize/Size_5",
                    case  7: pValues[nProp] >>= pImpl->aFontSizeArr[5]; break;//"Import/FontSize/Size_6",
                    case  8: pValues[nProp] >>= pImpl->aFontSizeArr[6]; break;//"Import/FontSize/Size_7",
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

                            pImpl->nExportMode = nExpMode;
                        }
                        break;
                    case 10:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            pImpl->nFlags |= HTMLCFG_STAR_BASIC;
                    break;//"Export/Basic",
                    case 11:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            pImpl->nFlags |= HTMLCFG_PRINT_LAYOUT_EXTENSION;
                    break;//"Export/PrintLayout",
                    case 12:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            pImpl->nFlags |= HTMLCFG_LOCAL_GRF;
                    break;//"Export/LocalGraphic",
                    case 13:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            pImpl->nFlags |= HTMLCFG_IS_BASIC_WARNING;
                    break;//"Export/Warning"

                    case 14: pValues[nProp] >>= pImpl->eEncoding;
                             pImpl->bIsEncodingDefault = false;
                    break;//"Export/Encoding"

                    case 15:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            pImpl->nFlags |= HTMLCFG_NUMBERS_ENGLISH_US;
                    break;//"Import/NumbersEnglishUS"
                }
            }
        }
    }
}


void    SvxHtmlOptions::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        bool bSet = false;
        switch(nProp)
        {
            case  0: bSet = 0 != (pImpl->nFlags & HTMLCFG_UNKNOWN_TAGS);break;//"Import/UnknownTag",
            case  1: bSet = 0 != (pImpl->nFlags & HTMLCFG_IGNORE_FONT_FAMILY);break;//"Import/FontSetting",
            case  2: pValues[nProp] <<= pImpl->aFontSizeArr[0];break;//"Import/FontSize/Size_1",
            case  3: pValues[nProp] <<= pImpl->aFontSizeArr[1];break;//"Import/FontSize/Size_2",
            case  4: pValues[nProp] <<= pImpl->aFontSizeArr[2];break;//"Import/FontSize/Size_3",
            case  5: pValues[nProp] <<= pImpl->aFontSizeArr[3];break;//"Import/FontSize/Size_4",
            case  6: pValues[nProp] <<= pImpl->aFontSizeArr[4];break;//"Import/FontSize/Size_5",
            case  7: pValues[nProp] <<= pImpl->aFontSizeArr[5];break;//"Import/FontSize/Size_6",
            case  8: pValues[nProp] <<= pImpl->aFontSizeArr[6];break;//"Import/FontSize/Size_7",
            case  9:                //"Export/Browser",
                {
                    sal_Int32 nExpMode = pImpl->nExportMode;

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
            case 10: bSet = 0 != (pImpl->nFlags & HTMLCFG_STAR_BASIC);break;//"Export/Basic",
            case 11: bSet = 0 != (pImpl->nFlags & HTMLCFG_PRINT_LAYOUT_EXTENSION);break;//"Export/PrintLayout",
            case 12: bSet = 0 != (pImpl->nFlags & HTMLCFG_LOCAL_GRF);break;//"Export/LocalGraphic",
            case 13: bSet = 0 != (pImpl->nFlags & HTMLCFG_IS_BASIC_WARNING);break;//"Export/Warning"
            case 14:
                if(!pImpl->bIsEncodingDefault)
                    pValues[nProp] <<= pImpl->eEncoding;
                break;//"Export/Encoding",
            case 15: bSet = 0 != (pImpl->nFlags & HTMLCFG_NUMBERS_ENGLISH_US);break;//"Import/NumbersEnglishUS"
        }
        if(nProp < 2 || ( nProp > 9 && nProp < 14 ) || nProp == 15)
            pValues[nProp] <<= bSet;
    }
    PutProperties(aNames, aValues);
}

void SvxHtmlOptions::Notify( const css::uno::Sequence< OUString >& )
{
    Load( GetPropertyNames() );
}


sal_uInt16  SvxHtmlOptions::GetFontSize(sal_uInt16 nPos) const
{
    if(nPos < HTML_FONT_COUNT)
        return (sal_uInt16)pImpl->aFontSizeArr[nPos];
    return 0;
}

void SvxHtmlOptions::SetFontSize(sal_uInt16 nPos, sal_uInt16 nSize)
{
    if(nPos < HTML_FONT_COUNT)
    {
        pImpl->aFontSizeArr[nPos] = nSize;
        SetModified();
    }
}


bool SvxHtmlOptions::IsImportUnknown() const
{
    return 0 != (pImpl->nFlags & HTMLCFG_UNKNOWN_TAGS) ;
}


void SvxHtmlOptions::SetImportUnknown(bool bSet)
{
    if(bSet)
        pImpl->nFlags |= HTMLCFG_UNKNOWN_TAGS;
    else
        pImpl->nFlags &= ~HTMLCFG_UNKNOWN_TAGS;
    SetModified();
}


sal_uInt16  SvxHtmlOptions::GetExportMode() const
{
    return (sal_uInt16)pImpl->nExportMode;
}


bool SvxHtmlOptions::IsStarBasic() const
{
    return 0 != (pImpl->nFlags & HTMLCFG_STAR_BASIC) ;
}


void SvxHtmlOptions::SetStarBasic(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HTMLCFG_STAR_BASIC;
    else
        pImpl->nFlags &= ~HTMLCFG_STAR_BASIC;
    SetModified();
}

bool SvxHtmlOptions::IsSaveGraphicsLocal() const
{
    return 0 != (pImpl->nFlags & HTMLCFG_LOCAL_GRF) ;
}

void SvxHtmlOptions::SetSaveGraphicsLocal(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HTMLCFG_LOCAL_GRF;
    else
        pImpl->nFlags &= ~HTMLCFG_LOCAL_GRF;
    SetModified();
}

bool SvxHtmlOptions::IsPrintLayoutExtension() const
{
    bool bRet = 0 != (pImpl->nFlags & HTMLCFG_PRINT_LAYOUT_EXTENSION);
    switch( pImpl->nExportMode )
    {
        case HTML_CFG_MSIE:
        case HTML_CFG_NS40  :
        case HTML_CFG_WRITER :
        break;
        default:
            bRet = false;
    }
    return bRet;
}

void    SvxHtmlOptions::SetPrintLayoutExtension(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HTMLCFG_PRINT_LAYOUT_EXTENSION;
    else
        pImpl->nFlags &= ~HTMLCFG_PRINT_LAYOUT_EXTENSION;
    SetModified();
}

bool SvxHtmlOptions::IsIgnoreFontFamily() const
{
    return 0 != (pImpl->nFlags & HTMLCFG_IGNORE_FONT_FAMILY) ;
}

void SvxHtmlOptions::SetIgnoreFontFamily(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HTMLCFG_IGNORE_FONT_FAMILY;
    else
        pImpl->nFlags &= ~HTMLCFG_IGNORE_FONT_FAMILY;
    SetModified();
}

bool SvxHtmlOptions::IsStarBasicWarning() const
{
    return 0 != (pImpl->nFlags & HTMLCFG_IS_BASIC_WARNING) ;
}

void SvxHtmlOptions::SetStarBasicWarning(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HTMLCFG_IS_BASIC_WARNING;
    else
        pImpl->nFlags &= ~HTMLCFG_IS_BASIC_WARNING;
    SetModified();
}

rtl_TextEncoding SvxHtmlOptions::GetTextEncoding() const
{
    rtl_TextEncoding eRet;
    if(pImpl->bIsEncodingDefault)
        eRet = SvtSysLocale::GetBestMimeEncoding();
    else
        eRet = (rtl_TextEncoding)pImpl->eEncoding;
    return eRet;
}

void SvxHtmlOptions::SetTextEncoding( rtl_TextEncoding eEnc )
{
    pImpl->eEncoding = eEnc;
    pImpl->bIsEncodingDefault = false;
    SetModified();
}

bool SvxHtmlOptions::IsDefaultTextEncoding() const
{
    return pImpl->bIsEncodingDefault;
}

namespace
{
    class theSvxHtmlOptions : public rtl::Static<SvxHtmlOptions, theSvxHtmlOptions> {};
}

SvxHtmlOptions& SvxHtmlOptions::Get()
{
    return theSvxHtmlOptions::get();
}

bool SvxHtmlOptions::IsNumbersEnglishUS() const
{
    return 0 != (pImpl->nFlags & HTMLCFG_NUMBERS_ENGLISH_US) ;
}

void SvxHtmlOptions::SetNumbersEnglishUS(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HTMLCFG_NUMBERS_ENGLISH_US;
    else
        pImpl->nFlags &= ~HTMLCFG_NUMBERS_ENGLISH_US;
    SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
