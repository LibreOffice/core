/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FONTCFG_HXX
#define _FONTCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
namespace binfilter {

#define FONT_STANDARD       0
#define FONT_OUTLINE        1
#define FONT_LIST           2
#define FONT_CAPTION        3
#define FONT_INDEX          4
#define FONT_STANDARD_CJK   5
#define FONT_OUTLINE_CJK    6
#define FONT_LIST_CJK       7
#define FONT_CAPTION_CJK    8
#define FONT_INDEX_CJK      9
#define FONT_STANDARD_CTL   10
#define FONT_OUTLINE_CTL    11
#define FONT_LIST_CTL       12
#define FONT_CAPTION_CTL    13
#define FONT_INDEX_CTL      14
#define DEF_FONT_COUNT      15

#define FONT_PER_GROUP      5

#define FONT_GROUP_DEFAULT  0
#define FONT_GROUP_CJK      1
#define FONT_GROUP_CTL      2

class SwStdFontConfig : public ::utl::ConfigItem
{
    String      sDefaultFonts[DEF_FONT_COUNT];

    ::com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

    void ChangeString(USHORT nFontType, const String& rSet)
        {
            if(sDefaultFonts[nFontType] != rSet)
            {
                SetModified();
                sDefaultFonts[nFontType] = rSet;
            }
        }
public:
    SwStdFontConfig();
    ~SwStdFontConfig();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

    const String&   GetFontStandard(sal_uInt8 nFontGroup) const {return sDefaultFonts[FONT_STANDARD + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontOutline(sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_OUTLINE + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontList   (sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_LIST + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontCaption(sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_CAPTION + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontIndex  (sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_INDEX + FONT_PER_GROUP * nFontGroup];}

    const String&   GetFontFor(USHORT nFontType)  const {return sDefaultFonts[nFontType];}
    BOOL            IsFontDefault(USHORT nFontType) const;

    void     SetFontStandard(const String& rSet, sal_uInt8 nFontGroup)
                    {ChangeString(FONT_STANDARD + FONT_PER_GROUP * nFontGroup, rSet);}

    void     SetFontOutline(const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_OUTLINE + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontList   (const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_LIST + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontCaption(const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_CAPTION + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontIndex  (const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_INDEX + FONT_PER_GROUP * nFontGroup, rSet);}

    static String   GetDefaultFor(USHORT nFontType, LanguageType eLang);
};
} //namespace binfilter
#endif


