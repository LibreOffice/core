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
#include <vcl/bitmap.hxx>
#include <osl/diagnose.h>
#include <comphelper/lok.hxx>

sal_uInt32 SvxColorIconView::getEntryEdgeLength()
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        return 26;
    }

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return rStyleSettings.GetListBoxPreviewDefaultPixelSize().Height() + 1;
}

void SvxColorIconView::addEntriesForXColorList(weld::IconView& pIconView,
                                               const XColorList& rXColorList,
                                               sal_uInt32 nStartIndex)
{
    const sal_uInt32 nColorCount(rXColorList.Count());
    ScopedVclPtr<VirtualDevice> pVDev = createColorDevice();

    for (sal_uInt32 nIndex(0); nIndex < nColorCount; nIndex++, nStartIndex++)
    {
        const XColorEntry* pEntry = rXColorList.GetColor(nIndex);

        if (pEntry)
        {
            drawColor(*pVDev, pEntry->GetColor());
            Bitmap aBmp = pVDev->GetBitmap(Point(), pVDev->GetOutputSizePixel());
            OUString sColorName = pEntry->GetName();
            OUString sId = OUString::number(nIndex);
            pIconView.insert(nIndex, &sColorName, &sId, &aBmp, nullptr);
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
    ScopedVclPtr<VirtualDevice> pVDev = createColorDevice();

    if (!rNamePrefix.empty())
    {
        for (const auto& rColor : rColorSet)
        {
            drawColor(*pVDev, rColor);
            Bitmap aBmp = pVDev->GetBitmap(Point(), pVDev->GetOutputSizePixel());
            OUString sName = OUString::Concat(rNamePrefix) + OUString::number(nStartIndex);
            OUString sId = OUString::number(nStartIndex);
            pIconView.insert(nStartIndex, &sName, &sId, &aBmp, nullptr);
            nStartIndex++;
        }
    }
    else
    {
        for (const auto& rColor : rColorSet)
        {
            drawColor(*pVDev, rColor);
            Bitmap aBmp = pVDev->GetBitmap(Point(), pVDev->GetOutputSizePixel());
            OUString sId = OUString::number(nStartIndex);
            OUString sName = u""_ustr;
            pIconView.insert(nStartIndex, &sName, &sId, &aBmp, nullptr);
            nStartIndex++;
        }
    }
}

// [-loplugin:scopedvclptr]
VclPtr<VirtualDevice> SvxColorIconView::createColorDevice()
{
    const sal_uInt32 nEdgeLength = getEntryEdgeLength() - 2;
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    const sal_Int32 nScaleFactor = pVDev->GetDPIScaleFactor();
    const sal_uInt32 nScaledEdge = nEdgeLength * nScaleFactor;
    pVDev->SetOutputSizePixel(Size(nScaledEdge, nScaledEdge));
    return pVDev;
}

void SvxColorIconView::drawColor(VirtualDevice& rDev, const Color& rColor)
{
    const Size aSize = rDev.GetOutputSizePixel();
    rDev.SetFillColor(rColor);
    rDev.SetLineColor(COL_BLACK);
    rDev.DrawRect(tools::Rectangle(Point(0, 0), aSize));
}

ScopedVclPtr<VirtualDevice> SvxColorIconView::createColorVirtualDevice(const Color& rColor)
{
    VclPtr<VirtualDevice> pVDev = createColorDevice();
    drawColor(*pVDev, rColor);
    return pVDev;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
