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

#include <seclabelmarking.hxx>

#include <attrib.hxx>
#include <docsh.hxx>
#include <docuno.hxx>
#include <document.hxx>
#include <scitems.hxx>

#include <editeng/editobj.hxx>
#include <svl/eitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/seclabel/SecLabelStore.hxx>

namespace sc::seclabel
{
bool isMarking(const ScDocument& rDocument, const EditTextObject* pArea)
{
    ScDocShell* pDocShell = rDocument.GetDocumentShell();
    return pArea && pArea->GetParagraphCount() == 1 && pDocShell
           && svx::seclabel::isMarking(
               cpo::uno::Reference<css::frame::XModel>(pDocShell->GetModel()),
               pArea->GetText(0));
}

std::optional<SfxItemSet> keepMarking(const ScDocument& rDocument, const SfxItemSet& rStyleSet,
                                      const SfxItemSet& rChanges)
{
    std::optional<SfxItemSet> oKept;
    static constexpr std::pair<TypedWhichId<ScPageHFItem>, TypedWhichId<SvxSetItem>> aAreas[]
        = { { ATTR_PAGE_HEADERRIGHT, ATTR_PAGE_HEADERSET },
            { ATTR_PAGE_FOOTERRIGHT, ATTR_PAGE_FOOTERSET } };
    for (const auto& [nContent, nSet] : aAreas)
    {
        const ScPageHFItem& rOldContent = rStyleSet.Get(nContent);
        if (!isMarking(rDocument, rOldContent.GetCenterArea()))
            continue;

        if (const ScPageHFItem* pNewContent = rChanges.GetItemIfSet(nContent, false);
            pNewContent && !isMarking(rDocument, pNewContent->GetCenterArea()))
        {
            if (!oKept)
                oKept.emplace(rChanges);
            ScPageHFItem aContent(*pNewContent);
            aContent.SetCenterArea(*rOldContent.GetCenterArea());
            oKept->Put(aContent);
        }

        const SvxSetItem* pNewSet = rChanges.GetItemIfSet(nSet, false);
        if (!pNewSet)
            continue;
        const SfxItemSet& rOldAreaSet = rStyleSet.Get(nSet).GetItemSet();
        SfxItemSet aAreaSet(pNewSet->GetItemSet());
        bool bChanged = false;
        for (TypedWhichId<SfxBoolItem> nWhich :
             { ATTR_PAGE_ON, ATTR_PAGE_SHARED, ATTR_PAGE_SHARED_FIRST })
        {
            if (aAreaSet.Get(nWhich).GetValue() != rOldAreaSet.Get(nWhich).GetValue())
            {
                aAreaSet.Put(rOldAreaSet.Get(nWhich));
                bChanged = true;
            }
        }
        if (bChanged)
        {
            if (!oKept)
                oKept.emplace(rChanges);
            oKept->Put(SvxSetItem(nSet, aAreaSet));
        }
    }
    return oKept;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
