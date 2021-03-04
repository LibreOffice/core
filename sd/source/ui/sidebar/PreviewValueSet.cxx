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

#include "PreviewValueSet.hxx"
#include <vcl/commandevent.hxx>

namespace sd::sidebar {

const int gnBorderWidth(3);
const int gnBorderHeight(3);

PreviewValueSet::PreviewValueSet()
    : ValueSet(nullptr)
    , maPreviewSize(10,10)
{
    SetStyle (
        GetStyle()
        & ~(WB_ITEMBORDER)// | WB_MENUSTYLEVALUESET)
        //        | WB_FLATVALUESET);
        );
}

void PreviewValueSet::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    ValueSet::SetDrawingArea(pDrawingArea);

    SetColCount(2);
    SetExtraSpacing (2);
}

PreviewValueSet::~PreviewValueSet()
{
}

void PreviewValueSet::SetPreviewSize (const Size& rSize)
{
    maPreviewSize = rSize;
}

void PreviewValueSet::SetContextMenuHandler(const Link<const Point*, void>& rLink)
{
    maContextMenuHandler = rLink;
}

bool PreviewValueSet::Command(const CommandEvent& rEvent)
{
    if (rEvent.GetCommand() != CommandEventId::ContextMenu)
        return ValueSet::Command(rEvent);
    maContextMenuHandler.Call(rEvent.IsMouseEvent() ? &rEvent.GetMousePosPixel() : nullptr);
    return true;
}

void PreviewValueSet::Resize()
{
    ValueSet::Resize();

    Size aWindowSize (GetOutputSizePixel());
    if (!aWindowSize.IsEmpty())
    {
        Rearrange();
    }
}

void PreviewValueSet::Rearrange()
{
    sal_uInt16 nNewColumnCount (CalculateColumnCount (
        GetOutputSizePixel().Width()));
    sal_uInt16 nNewRowCount (CalculateRowCount (nNewColumnCount));

    SetFormat();
    SetColCount(nNewColumnCount);
    SetLineCount(nNewRowCount);
}

sal_uInt16 PreviewValueSet::CalculateColumnCount (int nWidth) const
{
    int nColumnCount = 0;
    if (nWidth > 0)
    {
        nColumnCount = nWidth / (maPreviewSize.Width() + 2*gnBorderWidth);
        if (nColumnCount < 1)
            nColumnCount = 1;
    }
    return static_cast<sal_uInt16>(nColumnCount);
}

sal_uInt16 PreviewValueSet::CalculateRowCount (sal_uInt16 nColumnCount) const
{
    int nRowCount = 0;
    int nItemCount = GetItemCount();
    if (nColumnCount > 0)
    {
        nRowCount = (nItemCount+nColumnCount-1) / nColumnCount;
        if (nRowCount < 1)
            nRowCount = 1;
    }

    return static_cast<sal_uInt16>(nRowCount);
}

sal_Int32 PreviewValueSet::GetPreferredHeight (sal_Int32 nWidth)
{
    int nRowCount (CalculateRowCount(CalculateColumnCount(nWidth)));
    int nItemHeight (maPreviewSize.Height());

    return nRowCount * (nItemHeight + 2*gnBorderHeight);
}

} // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
