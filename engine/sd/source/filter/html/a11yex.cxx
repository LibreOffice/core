/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <algorithm>
#include <vector>

#include <sal/log.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdoashp.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdograf.hxx>
#include <tools/debug.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <Outliner.hxx>
#include <sdpage.hxx>

#include "htmlex.hxx"
#include <sdhtmlfilter.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;

using namespace sdr::table;

namespace
{
OUString GetObjectName(const SdrObject* pObject)
{
    OUString aName = pObject->GetName();
    if (!aName.isEmpty())
        return aName;

    if (pObject->GetObjIdentifier() == SdrObjKind::Graphic)
        return SvxResId(STR_ObjNameSingulGRAF);

    if (pObject->GetObjIdentifier() == SdrObjKind::CustomShape)
    {
        const SdrObjCustomShape* pCustomShape
            = static_cast<const SdrObjCustomShape*>(pObject);
        if (pCustomShape->IsTextPath())
            return SvxResId(STR_ObjNameSingulFONTWORK);

        OUString aShapeName = pCustomShape->GetCustomShapeName();
        if (!aShapeName.isEmpty())
            return aShapeName;
    }

    return pObject->TakeObjNameSingul();
}

// A screen reader announces the slide in the order the objects are exported, so they have to be
// exported in visual reading order: top to bottom, and left to right for objects sharing a row.
// The order the objects have in the page is the z-order, which often differs from what the user
// sees - e.g. a text box at the bottom of the slide can be created before the content placeholder
// above it.
std::vector<SdrObject*> GetObjectsInReadingOrder(SdPage* pPage)
{
    std::vector<SdrObject*> aObjects;
    aObjects.reserve(pPage->GetObjCount());
    for (const rtl::Reference<SdrObject>& pObject : *pPage)
        aObjects.push_back(pObject.get());

    std::stable_sort(aObjects.begin(), aObjects.end(),
                     [](const SdrObject* pA, const SdrObject* pB) {
                         return pA->GetSnapRect().Top() < pB->GetSnapRect().Top();
                     });

    // Objects that overlap vertically belong to the same row, which is ordered left to right. A
    // row ends at the highest bottom edge in it, so that a tall object does not pull in every
    // object next to it.
    auto aRowStart = aObjects.begin();
    while (aRowStart != aObjects.end())
    {
        tools::Long nRowBottom = (*aRowStart)->GetSnapRect().Bottom();
        auto aRowEnd = aRowStart + 1;
        for (; aRowEnd != aObjects.end(); ++aRowEnd)
        {
            const tools::Rectangle& rRect = (*aRowEnd)->GetSnapRect();
            if (rRect.Top() >= nRowBottom)
                break;
            nRowBottom = std::min(nRowBottom, rRect.Bottom());
        }
        std::stable_sort(aRowStart, aRowEnd, [](const SdrObject* pA, const SdrObject* pB) {
            return pA->GetSnapRect().Left() < pB->GetSnapRect().Left();
        });
        aRowStart = aRowEnd;
    }

    return aObjects;
}

void CreateTitleDescription(const SdrObject* pObject, OUStringBuffer& rHtml, bool bWithName)
{
    if (bWithName)
    {
        rHtml.append("<h2>");
        rHtml.append(GetObjectName(pObject));
        rHtml.append("</h2>\n\r");
    }
    rHtml.append("<h2>");
    rHtml.append(pObject->GetTitle());
    rHtml.append("</h2>\n\r<p>");
    rHtml.append(pObject->GetDescription());
    rHtml.append("</p>\n\r");
}
}

void SdHTMLFilter::ExportPage(SdrOutliner* pOutliner, SdPage* pPage, OUStringBuffer& rHtml)
{
    if (!pPage || !pOutliner)
    {
        return;
    }

    // page title
    OUString sTitleText(HtmlExport::CreateTextForTitle(pOutliner, pPage));

    rHtml.append("<h1>" + sTitleText + "</h1>\r\n");

    for (SdrObject* pObject : GetObjectsInReadingOrder(pPage))
    {
        if (pObject->IsDecorative())
            continue;

        PresObjKind eKind = pPage->GetPresObjKind(pObject);

        switch (eKind)
        {
            case PresObjKind::NONE:
            {
                if (pObject->GetObjIdentifier() == SdrObjKind::Group)
                {
                    SdrObjGroup* pObjectGroup = static_cast<SdrObjGroup*>(pObject);
                    HtmlExport::WriteObjectGroup(rHtml, pObjectGroup, pOutliner, false);
                }
                else if (pObject->GetObjIdentifier() == SdrObjKind::Table)
                {
                    SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject);
                    HtmlExport::WriteTable(rHtml, pTableObject, pOutliner);
                }
                else
                {
                    if (pObject->GetOutlinerParaObject())
                    {
                        HtmlExport::WriteOutlinerParagraph(rHtml, pOutliner,
                                                           pObject->GetOutlinerParaObject(), false);
                    }
                    else
                    {
                        CreateTitleDescription(pObject, rHtml, true);
                    }
                }
            }
            break;

            case PresObjKind::Table:
            {
                SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject);
                HtmlExport::WriteTable(rHtml, pTableObject, pOutliner);
            }
            break;

            case PresObjKind::Text:
            case PresObjKind::Outline:
            {
                SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject);
                if (pTextObject->IsEmptyPresObj())
                    continue;
                HtmlExport::WriteOutlinerParagraph(rHtml, pOutliner,
                                                   pTextObject->GetOutlinerParaObject(), true);
            }
            break;

            default:
                CreateTitleDescription(pObject, rHtml, false);
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
