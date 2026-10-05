/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawdoc.hxx>
#include <stlpool.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/svxenum.hxx>
#include <svl/style.hxx>
#include <svx/documentbullets.hxx>
#include <svx/svditer.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdtext.hxx>
#include <svx/svdoutl.hxx>
#include <vcl/font.hxx>

#include <memory>
#include <set>

namespace
{
/// Appends every level of rRule that draws a character or an image to rBullets. A bullet whose
/// identifier is already in rSeenIdentifiers is left out, so each one is listed once.
void lcl_AddBulletsOfNumRule(const SvxNumRule& rRule, std::vector<SvxDocumentBullet>& rBullets,
                             std::set<OUString>& rSeenIdentifiers)
{
    for (sal_uInt16 nLevel = 0; nLevel < rRule.GetLevelCount(); ++nLevel)
    {
        const SvxNumberFormat& rFormat = rRule.GetLevel(nLevel);
        SvxDocumentBullet aBullet;

        if (rFormat.GetNumberingType() == SVX_NUM_CHAR_SPECIAL)
        {
            if (!rFormat.GetBulletFont().has_value())
                continue;

            const sal_UCS4 cBullet = rFormat.GetBulletChar();
            aBullet.aBulletCharacter = OUString(&cBullet, 1);
            aBullet.aFontName = rFormat.GetBulletFont()->GetFamilyName();
        }
        else if (rFormat.GetNumberingType() == SVX_NUM_BITMAP)
        {
            const SvxBrushItem* pBrush = rFormat.GetBrush();
            if (!pBrush || !pBrush->GetGraphic())
                continue;

            aBullet.aGraphic = *pBrush->GetGraphic();
            aBullet.aGraphicSize = rFormat.GetGraphicSize();
        }
        else
            continue;

        if (rSeenIdentifiers.insert(aBullet.GetIdentifier()).second)
            rBullets.push_back(aBullet);
    }
}

/// Appends the bullets of the numbering rule that rItemSet carries directly. An item that the set
/// only inherits is left alone, so a pool default does not count as a bullet of the document.
void lcl_AddBulletsOfItemSet(const SfxItemSet& rItemSet, std::vector<SvxDocumentBullet>& rBullets,
                             std::set<OUString>& rSeenIdentifiers)
{
    const SvxNumBulletItem* pItem = rItemSet.GetItemIfSet(EE_PARA_NUMBULLET, false);
    if (pItem)
        lcl_AddBulletsOfNumRule(pItem->GetNumRule(), rBullets, rSeenIdentifiers);
}

/// Appends the bullets of every paragraph that rOutliner holds.
void lcl_AddBulletsOfOutliner(const SdrOutliner& rOutliner,
                              std::vector<SvxDocumentBullet>& rBullets,
                              std::set<OUString>& rSeenIdentifiers)
{
    for (sal_Int32 nParagraph = 0; nParagraph < rOutliner.GetParagraphCount(); ++nParagraph)
        lcl_AddBulletsOfItemSet(rOutliner.GetParaAttribs(nParagraph), rBullets, rSeenIdentifiers);
}

/// Appends the bullets of every text object on rPage, including the text objects inside groups.
void lcl_AddBulletsOfPage(const SdrPage& rPage, std::vector<SvxDocumentBullet>& rBullets,
                          std::set<OUString>& rSeenIdentifiers)
{
    SdrObjListIter aIter(&rPage, SdrIterMode::DeepWithGroups);
    while (aIter.IsMore())
    {
        const SdrTextObj* pTextObject = DynCastSdrTextObj(aIter.Next());
        if (!pTextObject)
            continue;

        lcl_AddBulletsOfItemSet(pTextObject->GetMergedItemSet(), rBullets, rSeenIdentifiers);

        // A table holds one text per cell, where a plain text object holds a single one.
        for (sal_Int32 nText = 0; nText < pTextObject->getTextCount(); ++nText)
        {
            const SdrText* pText = pTextObject->getText(nText);
            if (!pText)
                continue;

            lcl_AddBulletsOfItemSet(pText->GetItemSet(), rBullets, rSeenIdentifiers);

            const OutlinerParaObject* pParagraphs = pText->GetOutlinerParaObject();
            if (!pParagraphs)
                continue;

            const EditTextObject& rTextObject = pParagraphs->GetTextObject();
            for (sal_Int32 nParagraph = 0; nParagraph < rTextObject.GetParagraphCount();
                 ++nParagraph)
            {
                lcl_AddBulletsOfItemSet(rTextObject.GetParaAttribs(nParagraph), rBullets,
                                        rSeenIdentifiers);
            }
        }
    }
}
}

std::vector<SvxDocumentBullet>
SdDrawDocument::GetUsedBullets(const SdrOutliner* pActiveOutliner) const
{
    std::vector<SvxDocumentBullet> aBullets;
    std::set<OUString> aSeenIdentifiers;

    // The outline styles of every slide layout live in the style sheet pool, so a bullet that a
    // master slide defines is found here whether or not a slide goes on to use it. A style sheet
    // reaches the document as changed only through a shape that follows it, so a style no shape
    // follows would otherwise answer from a list made before it gained its bullet. There are tens
    // of style sheets where there are thousands of paragraphs, so these are read every time.
    SdStyleSheetPool* pStyleSheetPool = GetSdStyleSheetPool();
    if (pStyleSheetPool)
    {
        // The pool's own First and Next share one cached iterator, so walking with them here would
        // disturb a walk already under way in the caller. This iterator belongs to us alone.
        std::unique_ptr<SfxStyleSheetIterator> pIterator
            = pStyleSheetPool->CreateIterator(SfxStyleFamily::All);
        for (SfxStyleSheetBase* pStyle = pIterator->First(); pStyle; pStyle = pIterator->Next())
            lcl_AddBulletsOfItemSet(pStyle->GetItemSet(), aBullets, aSeenIdentifiers);
    }

    if (!moUsedBullets)
    {
        moUsedBullets = WalkPagesForUsedBullets();
        ++mnUsedBulletWalks;
    }

    for (const SvxDocumentBullet& rBullet : *moUsedBullets)
    {
        if (aSeenIdentifiers.insert(rBullet.GetIdentifier()).second)
            aBullets.push_back(rBullet);
    }

    // A shape under edit keeps its text in the outliner rather than on the shape, so a bullet
    // applied a moment ago is found here and nowhere else. That text changes under the typing,
    // so it is read afresh every time rather than remembered with the rest.
    if (pActiveOutliner)
        lcl_AddBulletsOfOutliner(*pActiveOutliner, aBullets, aSeenIdentifiers);

    return aBullets;
}

std::vector<SvxDocumentBullet> SdDrawDocument::WalkPagesForUsedBullets() const
{
    std::vector<SvxDocumentBullet> aBullets;
    std::set<OUString> aSeenIdentifiers;

    for (sal_uInt16 nPage = 0; nPage < GetPageCount(); ++nPage)
    {
        const SdrPage* pPage = GetPage(nPage);
        if (pPage)
            lcl_AddBulletsOfPage(*pPage, aBullets, aSeenIdentifiers);
    }

    for (sal_uInt16 nPage = 0; nPage < GetMasterPageCount(); ++nPage)
    {
        const SdrPage* pPage = GetMasterPage(nPage);
        if (pPage)
            lcl_AddBulletsOfPage(*pPage, aBullets, aSeenIdentifiers);
    }

    return aBullets;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
