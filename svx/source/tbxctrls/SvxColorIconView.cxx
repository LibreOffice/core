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

#include <svx/SvxColorIconView.hxx>
#include <svx/xtable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <osl/diagnose.h>

sal_uInt32 SvxColorIconView::getEntryEdgeLength()
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return rStyleSettings.GetListBoxPreviewDefaultPixelSize().Height() + 1;
}

void SvxColorIconView::addEntriesForXColorList(weld::IconView& pIconView,
                                               const XColorList& rXColorList,
                                               sal_uInt32 nStartIndex)
{
    const sal_uInt32 nColorCount(rXColorList.Count());

    for (sal_uInt32 nIndex(0); nIndex < nColorCount; nIndex++, nStartIndex++)
    {
        const XColorEntry* pEntry = rXColorList.GetColor(nIndex);

        if (pEntry)
        {
            VclPtr<VirtualDevice> pColorVDev = createColorVirtualDevice(pEntry->GetColor());
            OUString sColorName = pEntry->GetName();
            OUString sId = OUString::number(nIndex);
            pIconView.insert(nIndex, &sColorName, &sId, pColorVDev, nullptr);
        }
        else
        {
            OSL_ENSURE(false, "OOps, XColorList with empty entries (!)");
        }
    }
}

void SvxColorIconView::addEntriesForColorSet(weld::IconView& pIconView,
                                             const std::set<Color>& rColorSet,
                                             std::u16string_view rNamePrefix)
{
    sal_uInt32 nStartIndex = 0;
    if (!rNamePrefix.empty())
    {
        for (const auto& rColor : rColorSet)
        {
            VclPtr<VirtualDevice> pColorVDev = createColorVirtualDevice(rColor);
            OUString sName = OUString::Concat(rNamePrefix) + OUString::number(nStartIndex);
            OUString sId = OUString::number(nStartIndex);
            // InsertEntry(sName, aColorImage, aColorImage);
            pIconView.insert(nStartIndex, &sName, &sId, pColorVDev, nullptr);
            nStartIndex++;
        }
    }
    else
    {
        for (const auto& rColor : rColorSet)
        {
            VclPtr<VirtualDevice> pColorVDev = createColorVirtualDevice(rColor);
            OUString sId = OUString::number(nStartIndex);
            OUString sName = u""_ustr;
            // InsertEntry(u""_ustr, aColorImage, aColorImage);
            pIconView.insert(nStartIndex, &sName, &sId, pColorVDev, nullptr);
            nStartIndex++;
        }
    }
}

VclPtr<VirtualDevice> SvxColorIconView::createColorVirtualDevice(const Color& rColor)
{
    const sal_uInt32 nEdgeLength = getEntryEdgeLength() - 2;
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    pVDev->SetOutputSizePixel(Size(nEdgeLength, nEdgeLength));

    // Fill with the color
    pVDev->SetFillColor(rColor);
    pVDev->SetLineColor(COL_BLACK);
    pVDev->DrawRect(tools::Rectangle(Point(0, 0), Size(nEdgeLength, nEdgeLength)));

    BitmapEx aPreviewBitmap = pVDev->GetBitmapEx(Point(0, 0), Size(nEdgeLength, nEdgeLength));
    const Point aNull(0, 0);
    if (pVDev->GetDPIScaleFactor() > 1)
        aPreviewBitmap.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());
    const Size aSize(aPreviewBitmap.GetSizePixel());
    pVDev->SetOutputSizePixel(aSize);
    pVDev->DrawBitmapEx(aNull, aPreviewBitmap);

    return pVDev;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */