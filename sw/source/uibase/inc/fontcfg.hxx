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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FONTCFG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FONTCFG_HXX

#include <rtl/ustring.hxx>
#include <unotools/configitem.hxx>
#include <i18nlangtag/lang.h>
#include <osl/diagnose.h>
#include "swdllapi.h"

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

//pt-size of fonts
#define FONTSIZE_DEFAULT            240
#define FONTSIZE_CJK_DEFAULT        210
#define FONTSIZE_OUTLINE            280

class SW_DLLPUBLIC SwStdFontConfig : public utl::ConfigItem
{
    OUString    sDefaultFonts[DEF_FONT_COUNT];
    sal_Int32   nDefaultFontHeight[DEF_FONT_COUNT];

    SAL_DLLPRIVATE css::uno::Sequence<OUString>    GetPropertyNames();

    void ChangeString(sal_uInt16 nFontType, const OUString& rSet)
        {
            if(sDefaultFonts[nFontType] != rSet)
            {
                SetModified();
                sDefaultFonts[nFontType] = rSet;
            }
        }

    void ChangeInt( sal_uInt16 nFontType, sal_Int32 nHeight );

    virtual void    ImplCommit() override;

public:
    SwStdFontConfig();
    virtual ~SwStdFontConfig();

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    const OUString&   GetFontStandard(sal_uInt8 nFontGroup) const {return sDefaultFonts[FONT_STANDARD + FONT_PER_GROUP * nFontGroup];}
    const OUString&   GetFontOutline(sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_OUTLINE + FONT_PER_GROUP * nFontGroup];}
    const OUString&   GetFontList   (sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_LIST + FONT_PER_GROUP * nFontGroup];}
    const OUString&   GetFontCaption(sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_CAPTION + FONT_PER_GROUP * nFontGroup];}
    const OUString&   GetFontIndex  (sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_INDEX + FONT_PER_GROUP * nFontGroup];}

    const OUString&   GetFontFor(sal_uInt16 nFontType)  const {return sDefaultFonts[nFontType];}
    bool            IsFontDefault(sal_uInt16 nFontType) const;

    void     SetFontStandard(const OUString& rSet, sal_uInt8 nFontGroup)
                    {ChangeString(FONT_STANDARD + FONT_PER_GROUP * nFontGroup, rSet);}

    void     SetFontOutline(const OUString& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_OUTLINE + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontList   (const OUString& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_LIST + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontCaption(const OUString& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_CAPTION + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontIndex  (const OUString& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_INDEX + FONT_PER_GROUP * nFontGroup, rSet);}

    void     SetFontHeight( sal_Int32 nHeight, sal_uInt8 nFont, sal_uInt8 nScriptType )
                    {    ChangeInt(nFont + FONT_PER_GROUP * nScriptType, nHeight);}

    sal_Int32 GetFontHeight( sal_uInt8 nFont, sal_uInt8 nScriptType, LanguageType eLang );

    static OUString    GetDefaultFor(sal_uInt16 nFontType, LanguageType eLang);
    static sal_Int32 GetDefaultHeightFor(sal_uInt16 nFontType, LanguageType eLang);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
