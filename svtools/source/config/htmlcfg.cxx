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
#include <o3tl/typed_flags_set.hxx>

enum class HtmlCfgFlags {
    NONE                  = 0x000,
    UnknownTags           = 0x001,
    StarBasic             = 0x008,
    LocalGrf              = 0x010,
    PrintLayoutExtension  = 0x020,
    IgnoreFontFamily      = 0x040,
    IsBasicWarning        = 0x080,
    NumbersEnglishUS      = 0x100,
};
namespace o3tl {
    template<> struct typed_flags<HtmlCfgFlags> : is_typed_flags<HtmlCfgFlags, 0x1f9> {};
}

using namespace utl;
using namespace com::sun::star::uno;


struct HtmlOptions_Impl
{
    HtmlCfgFlags nFlags;
    sal_Int32    nExportMode;
    sal_Int32    aFontSizeArr[HTML_FONT_COUNT];
    sal_Int32    eEncoding;
    bool         bIsEncodingDefault;

    HtmlOptions_Impl() :
        nFlags(HtmlCfgFlags::LocalGrf|HtmlCfgFlags::IsBasicWarning),
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
    if(aValues.getLength() != aNames.getLength())
        return;

    pImpl->nFlags = HtmlCfgFlags::NONE;
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case  0:
                    if(*o3tl::doAccess<bool>(pValues[nProp]))
                        pImpl->nFlags |= HtmlCfgFlags::UnknownTags;
                break;//"Import/UnknownTag",
                case  1:
                    if(*o3tl::doAccess<bool>(pValues[nProp]))
                        pImpl->nFlags |= HtmlCfgFlags::IgnoreFontFamily;
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
                        pImpl->nFlags |= HtmlCfgFlags::StarBasic;
                break;//"Export/Basic",
                case 11:
                    if(*o3tl::doAccess<bool>(pValues[nProp]))
                        pImpl->nFlags |= HtmlCfgFlags::PrintLayoutExtension;
                break;//"Export/PrintLayout",
                case 12:
                    if(*o3tl::doAccess<bool>(pValues[nProp]))
                        pImpl->nFlags |= HtmlCfgFlags::LocalGrf;
                break;//"Export/LocalGraphic",
                case 13:
                    if(*o3tl::doAccess<bool>(pValues[nProp]))
                        pImpl->nFlags |= HtmlCfgFlags::IsBasicWarning;
                break;//"Export/Warning"

                case 14: pValues[nProp] >>= pImpl->eEncoding;
                         pImpl->bIsEncodingDefault = false;
                break;//"Export/Encoding"

                case 15:
                    if(*o3tl::doAccess<bool>(pValues[nProp]))
                        pImpl->nFlags |= HtmlCfgFlags::NumbersEnglishUS;
                break;//"Import/NumbersEnglishUS"
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
            case  0: bSet = bool(pImpl->nFlags & HtmlCfgFlags::UnknownTags);break;//"Import/UnknownTag",
            case  1: bSet = bool(pImpl->nFlags & HtmlCfgFlags::IgnoreFontFamily);break;//"Import/FontSetting",
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
            case 10: bSet = bool(pImpl->nFlags & HtmlCfgFlags::StarBasic);break;//"Export/Basic",
            case 11: bSet = bool(pImpl->nFlags & HtmlCfgFlags::PrintLayoutExtension);break;//"Export/PrintLayout",
            case 12: bSet = bool(pImpl->nFlags & HtmlCfgFlags::LocalGrf);break;//"Export/LocalGraphic",
            case 13: bSet = bool(pImpl->nFlags & HtmlCfgFlags::IsBasicWarning);break;//"Export/Warning"
            case 14:
                if(!pImpl->bIsEncodingDefault)
                    pValues[nProp] <<= pImpl->eEncoding;
                break;//"Export/Encoding",
            case 15: bSet = bool(pImpl->nFlags & HtmlCfgFlags::NumbersEnglishUS);break;//"Import/NumbersEnglishUS"
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
        return static_cast<sal_uInt16>(pImpl->aFontSizeArr[nPos]);
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
    return bool(pImpl->nFlags & HtmlCfgFlags::UnknownTags) ;
}


void SvxHtmlOptions::SetImportUnknown(bool bSet)
{
    if(bSet)
        pImpl->nFlags |= HtmlCfgFlags::UnknownTags;
    else
        pImpl->nFlags &= ~HtmlCfgFlags::UnknownTags;
    SetModified();
}


sal_uInt16  SvxHtmlOptions::GetExportMode() const
{
    return static_cast<sal_uInt16>(pImpl->nExportMode);
}


bool SvxHtmlOptions::IsStarBasic() const
{
    return bool(pImpl->nFlags & HtmlCfgFlags::StarBasic) ;
}


void SvxHtmlOptions::SetStarBasic(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HtmlCfgFlags::StarBasic;
    else
        pImpl->nFlags &= ~HtmlCfgFlags::StarBasic;
    SetModified();
}

bool SvxHtmlOptions::IsSaveGraphicsLocal() const
{
    return bool(pImpl->nFlags & HtmlCfgFlags::LocalGrf) ;
}

void SvxHtmlOptions::SetSaveGraphicsLocal(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HtmlCfgFlags::LocalGrf;
    else
        pImpl->nFlags &= ~HtmlCfgFlags::LocalGrf;
    SetModified();
}

bool SvxHtmlOptions::IsPrintLayoutExtension() const
{
    bool bRet(pImpl->nFlags & HtmlCfgFlags::PrintLayoutExtension);
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
        pImpl->nFlags |=  HtmlCfgFlags::PrintLayoutExtension;
    else
        pImpl->nFlags &= ~HtmlCfgFlags::PrintLayoutExtension;
    SetModified();
}

bool SvxHtmlOptions::IsIgnoreFontFamily() const
{
    return bool(pImpl->nFlags & HtmlCfgFlags::IgnoreFontFamily) ;
}

void SvxHtmlOptions::SetIgnoreFontFamily(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HtmlCfgFlags::IgnoreFontFamily;
    else
        pImpl->nFlags &= ~HtmlCfgFlags::IgnoreFontFamily;
    SetModified();
}

bool SvxHtmlOptions::IsStarBasicWarning() const
{
    return bool(pImpl->nFlags & HtmlCfgFlags::IsBasicWarning) ;
}

void SvxHtmlOptions::SetStarBasicWarning(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HtmlCfgFlags::IsBasicWarning;
    else
        pImpl->nFlags &= ~HtmlCfgFlags::IsBasicWarning;
    SetModified();
}

rtl_TextEncoding SvxHtmlOptions::GetTextEncoding() const
{
    rtl_TextEncoding eRet;
    if(pImpl->bIsEncodingDefault)
        eRet = SvtSysLocale::GetBestMimeEncoding();
    else
        eRet = static_cast<rtl_TextEncoding>(pImpl->eEncoding);
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
    return bool(pImpl->nFlags & HtmlCfgFlags::NumbersEnglishUS) ;
}

void SvxHtmlOptions::SetNumbersEnglishUS(bool bSet)
{
    if(bSet)
        pImpl->nFlags |=  HtmlCfgFlags::NumbersEnglishUS;
    else
        pImpl->nFlags &= ~HtmlCfgFlags::NumbersEnglishUS;
    SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
