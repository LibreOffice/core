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


#include <EditLine.hxx>
#include <editdoc.hxx>
#include <algorithm>

EditLine* EditLine::Clone() const
{
    EditLine* pLine = new EditLine;
    pLine->maPositions = maPositions;
    pLine->mnStartPosX = mnStartPosX;
    pLine->mnNextLinePosXDiff = mnNextLinePosXDiff;
    pLine->mnStart = mnStart;
    pLine->mnEnd = mnEnd;
    pLine->mnStartPortion = mnStartPortion;
    pLine->mnEndPortion = mnEndPortion;
    pLine->mnHeight = mnHeight;
    pLine->mnTextWidth = mnTextWidth;
    pLine->mnTextHeight = mnTextHeight;
    pLine->mnMaxAscent = mnMaxAscent;

    return pLine;
}

Size EditLine::CalcTextSize(ParaPortion& rParaPortion)
{
    Size aSize;

    DBG_ASSERT(rParaPortion.GetTextPortions().Count(), "GetTextSize before CreatePortions !");

    for (sal_Int32 nPosition = mnStartPortion; nPosition <= mnEndPortion; nPosition++)
    {
        TextPortion& rPortion = rParaPortion.GetTextPortions()[nPosition];
        switch (rPortion.GetKind())
        {
            case PortionKind::TEXT:
            case PortionKind::FIELD:
            case PortionKind::HYPHENATOR:
            {
                Size aTmpSize = rPortion.GetSize();
                aSize.AdjustWidth(aTmpSize.Width());
                if (aSize.Height() < aTmpSize.Height() )
                    aSize.setHeight( aTmpSize.Height() );
            }
            break;
            case PortionKind::TAB:
            {
                aSize.AdjustWidth(rPortion.GetSize().Width());
            }
            break;
            case PortionKind::LINEBREAK:
            break;
        }
    }

    SetHeight(sal_uInt16(aSize.Height()));
    return aSize;
}

void EditLine::SetHeight(sal_uInt16 nHeight, sal_uInt16 nTextHeight)
{
    mnHeight = nHeight;

    if (nTextHeight != 0)
        mnTextHeight = nTextHeight;
    else
        mnTextHeight = nHeight;
}

void EditLine::SetStartPosX(sal_Int32 nStart)
{
    if (nStart > 0)
        mnStartPosX = nStart;
    else
        mnStartPosX = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
