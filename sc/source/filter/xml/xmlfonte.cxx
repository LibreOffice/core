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

#include <scitems.hxx>

#include <editeng/eeitem.hxx>

#include <xmloff/XMLFontAutoStylePool.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/editeng.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include "xmlexprt.hxx"
#include <stlpool.hxx>
#include <attrib.hxx>

namespace {

class ScXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
private:
    // #i120077# remember owned pool
    rtl::Reference<SfxItemPool>  mpEditEnginePool;

    void AddFontItems(const sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pItemPool, const bool bExportDefaults);

public:
    ScXMLFontAutoStylePool_Impl(ScDocument* pDoc, ScXMLExport& rExport, bool bEmbedFonts);
};

}

void ScXMLFontAutoStylePool_Impl::AddFontItems(const sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pItemPool, const bool bExportDefaults)
{
    for( sal_uInt16 i=0; i < nIdCount; ++i )
    {
        sal_uInt16 nWhichId(pWhichIds[i]);
        if (bExportDefaults)
        {
            const SfxPoolItem* pItem = &pItemPool->GetUserOrPoolDefaultItem(nWhichId);
            const SvxFontItem *pFont(static_cast<const SvxFontItem *>(pItem));
            Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                    pFont->GetFamily(), pFont->GetPitch(),
                    pFont->GetCharSet() );
        }
        ItemSurrogates aSurrogates;
        pItemPool->GetItemSurrogates( aSurrogates, nWhichId );
        for (const SfxPoolItem* pItem : aSurrogates)
        {
            const SvxFontItem *pFont(static_cast<const SvxFontItem *>(pItem));
            Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                 pFont->GetFamily(), pFont->GetPitch(),
                 pFont->GetCharSet() );
        }
    }
}

ScXMLFontAutoStylePool_Impl::ScXMLFontAutoStylePool_Impl(ScDocument* pDoc, ScXMLExport& rExportP, bool bEmbedFonts)
    : XMLFontAutoStylePool(rExportP, bEmbedFonts)
{
    if (!pDoc)
        return;

    sal_uInt16 const aWhichIds[]     { ATTR_FONT, ATTR_CJK_FONT,
                                       ATTR_CTL_FONT };
    sal_uInt16 const aEditWhichIds[] { EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK,
                                       EE_CHAR_FONTINFO_CTL };
    sal_uInt16 const aPageWhichIds[] { ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERLEFT,
                                       ATTR_PAGE_HEADERRIGHT, ATTR_PAGE_FOOTERRIGHT,
                                       ATTR_PAGE_HEADERFIRST, ATTR_PAGE_FOOTERFIRST };

    const SfxItemPool* pItemPool(pDoc->GetPool());
    AddFontItems(aWhichIds, 3, pItemPool, true);
    const SfxItemPool* pEditPool(pDoc->GetEditPool());
    AddFontItems(aEditWhichIds, 3, pEditPool, false);

    std::unique_ptr<SfxStyleSheetIterator> pItr = pDoc->GetStyleSheetPool()->CreateIterator(SfxStyleFamily::Page);

    m_bEmbedUsedOnly = pDoc->IsEmbedUsedFontsOnly();
    m_bEmbedLatinScript = pDoc->IsEmbedFontScriptLatin();
    m_bEmbedAsianScript = pDoc->IsEmbedFontScriptAsian();
    m_bEmbedComplexScript = pDoc->IsEmbedFontScriptComplex();

    if(!pItr)
        return;

    SfxStyleSheetBase* pStyle(pItr->First());

    if(!pStyle)
        return;

    // #i120077# remember the SfxItemPool in member variable before usage. The
    // local EditEngine will not take over ownership of the pool.
    mpEditEnginePool = EditEngine::CreatePool();
    EditEngine aEditEngine(mpEditEnginePool.get());

    while (pStyle)
    {
        const SfxItemPool& rPagePool(pStyle->GetPool()->GetPool());

        for (sal_uInt16 nPageWhichId : aPageWhichIds)
        {
            ItemSurrogates aSurrogates;
            rPagePool.GetItemSurrogates( aSurrogates, nPageWhichId );
            for (const SfxPoolItem* pItem : aSurrogates)
            {
                const ScPageHFItem* pPageItem = static_cast<const ScPageHFItem*>(pItem);
                const EditTextObject* pLeftArea(pPageItem->GetLeftArea());
                if (pLeftArea)
                {
                    aEditEngine.SetText(*pLeftArea);
                    AddFontItems(aEditWhichIds, 3, mpEditEnginePool.get(), false);
                }
                const EditTextObject* pCenterArea(pPageItem->GetCenterArea());
                if (pCenterArea)
                {
                    aEditEngine.SetText(*pCenterArea);
                    AddFontItems(aEditWhichIds, 3, mpEditEnginePool.get(), false);
                }
                const EditTextObject* pRightArea(pPageItem->GetRightArea());
                if (pRightArea)
                {
                    aEditEngine.SetText(*pRightArea);
                    AddFontItems(aEditWhichIds, 3, mpEditEnginePool.get(), false);
                }
            }
        }

        pStyle = pItr->Next();
    }
}

XMLFontAutoStylePool* ScXMLExport::CreateFontAutoStylePool()
{
    bool blockFontEmbedding = false;
    // We write font info to both content.xml and styles.xml, but they are both
    // written by different ScXMLExport instance, and would therefore write each
    // font file twice without complicated checking for duplicates, so handle
    // the embedding only in one of them.
    if(!( getExportFlags() & SvXMLExportFlags::CONTENT ))
        blockFontEmbedding = true;
    ScDocument* pDoc = GetDocument();
    if (pDoc && !pDoc->IsEmbedFonts())
        blockFontEmbedding = true;
    return new ScXMLFontAutoStylePool_Impl(pDoc, *this, !blockFontEmbedding);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
