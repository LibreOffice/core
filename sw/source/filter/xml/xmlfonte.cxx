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
#include "xmlexp.hxx"
#include "xmlimp.hxx"
#include <IDocumentSettingAccess.hxx>

namespace {

class SwXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
public:
    SwXMLFontAutoStylePool_Impl(SwXMLExport& rExport, bool bFontEmbedding);
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

SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl(SwXMLExport& _rExport, bool bFontEmbedding)
    : XMLFontAutoStylePool(_rExport, bFontEmbedding)
{
    sal_uInt16 const aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                      RES_CHRATR_CTL_FONT };

    const SfxItemPool& rPool = _rExport.getDoc()->GetAttrPool();
    std::vector<const SvxFontItem *> aFonts;
    for(sal_uInt16 nWhichId : aWhichIds)
    {
        const SvxFontItem& rFont =
            static_cast<const SvxFontItem&>(rPool.GetUserOrPoolDefaultItem( nWhichId ));
        aFonts.push_back(&rFont);
        ItemSurrogates aSurrogates;
        rPool.GetItemSurrogates(aSurrogates, nWhichId);
        for (const SfxPoolItem* pItem : aSurrogates)
        {
            auto pFont = static_cast<const SvxFontItem *>(pItem);
            aFonts.push_back(pFont);
        }
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

    auto const & pDocument = _rExport.getDoc();

    m_bEmbedUsedOnly = pDocument->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_USED_FONTS);
    m_bEmbedLatinScript = pDocument->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_LATIN_SCRIPT_FONTS);
    m_bEmbedAsianScript = pDocument->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_ASIAN_SCRIPT_FONTS);
    m_bEmbedComplexScript = pDocument->getIDocumentSettingAccess().get(DocumentSettingId::EMBED_COMPLEX_SCRIPT_FONTS);

}

XMLFontAutoStylePool* SwXMLExport::CreateFontAutoStylePool()
{
    bool blockFontEmbedding = false;
    // We write font info to both content.xml and styles.xml, but they are both
    // written by different SwXMLExport instance, and would therefore write each
    // font file twice without complicated checking for duplicates, so handle
    // the embedding only in one of them.
    if( !( getExportFlags() & SvXMLExportFlags::CONTENT) )
        blockFontEmbedding = true;
    if( !getDoc()->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS ))
        blockFontEmbedding = true;
    return new SwXMLFontAutoStylePool_Impl( *this, !blockFontEmbedding );
}

void SwXMLImport::NotifyContainsEmbeddedFont()
{
    getDoc()->getIDocumentSettingAccess().set( DocumentSettingId::EMBED_FONTS, true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
