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

#include <hintids.hxx>
#include <xmloff/XMLFontAutoStylePool.hxx>
#include <editeng/fontitem.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include "xmlexp.hxx"
#include "xmlimp.hxx"
#include <IDocumentSettingAccess.hxx>

namespace {

class SwXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
public:
    SwXMLFontAutoStylePool_Impl(SwXMLExport& rExport);
};

}

namespace
{
sal_Int32 CompareTo(sal_Int32 nA, sal_Int32 nB)
{
    if (nA < nB)
    {
        return -1;
    }

    if (nA > nB)
    {
        return 1;
    }

    return 0;
}
}

SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl(SwXMLExport& _rExport)
    : XMLFontAutoStylePool(_rExport)
{
    TypedWhichId<SvxFontItem> const aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                      RES_CHRATR_CTL_FONT };

    const SfxItemPool& rPool = _rExport.getDoc()->GetAttrPool();
    std::vector<const SvxFontItem *> aFonts;
    for(const TypedWhichId<SvxFontItem> & nWhichId : aWhichIds)
    {
        const SvxFontItem& rFont = rPool.GetUserOrPoolDefaultItem( nWhichId );
        aFonts.push_back(&rFont);
        _rExport.getDoc()->ForEachCharacterFontItem(nWhichId, /*bIgnoreAutoStyles*/true,
            [&aFonts] (const SvxFontItem& rFontItem) -> bool
            {
                aFonts.push_back(&rFontItem);
                return true;
            });
    }

    std::sort(aFonts.begin(), aFonts.end(),
        [](const SvxFontItem* pA, const SvxFontItem* pB) -> bool
        {
            sal_Int32 nRet = pA->GetFamilyName().compareTo(pB->GetFamilyName());
            if (nRet != 0)
            {
                return nRet < 0;
            }

            nRet = pA->GetStyleName().compareTo(pB->GetStyleName());
            if (nRet != 0)
            {
                return nRet < 0;
            }

            nRet = CompareTo(pA->GetFamily(), pB->GetFamily());
            if (nRet != 0)
            {
                return nRet < 0;
            }

            nRet = CompareTo(pA->GetPitch(), pB->GetPitch());
            if (nRet != 0)
            {
                return nRet < 0;
            }

            return pA->GetCharSet() < pB->GetCharSet();
        });
    for (const auto& pFont : aFonts)
    {
        Add(pFont->GetFamilyName(), pFont->GetStyleName(), pFont->GetFamily(), pFont->GetPitch(),
            pFont->GetCharSet());
    }
}

XMLFontAutoStylePool* SwXMLExport::CreateFontAutoStylePool()
{
    return new SwXMLFontAutoStylePool_Impl(*this);
}

bool SwXMLExport::getEmbedFonts()
{
    return getDoc()->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_FONTS);
}
bool SwXMLExport::getEmbedOnlyUsedFonts()
{
    return getDoc()->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_USED_FONTS);
}
bool SwXMLExport::getEmbedLatinScript()
{
    return getDoc()->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_LATIN_SCRIPT_FONTS);
}
bool SwXMLExport::getEmbedAsianScript()
{
    return getDoc()->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_ASIAN_SCRIPT_FONTS);
}
bool SwXMLExport::getEmbedComplexScript()
{
    return getDoc()->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_COMPLEX_SCRIPT_FONTS);
}

void SwXMLImport::NotifyContainsEmbeddedFont()
{
    getDoc()->getIDocumentSettingAccess().set( DocumentSettingId::EMBED_FONTS, true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
