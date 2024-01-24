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

EditLine* EditLine::Clone() const
{
    EditLine* pL = new EditLine;
    pL->aPositions = aPositions;
    pL->nStartPosX      = nStartPosX;
    pL->nNextLinePosXDiff = nNextLinePosXDiff;
    pL->nStart          = nStart;
    pL->nEnd            = nEnd;
    pL->nStartPortion   = nStartPortion;
    pL->nEndPortion     = nEndPortion;
    pL->nHeight         = nHeight;
    pL->nTxtWidth       = nTxtWidth;
    pL->nTxtHeight      = nTxtHeight;
    pL->nMaxAscent      = nMaxAscent;

    return pL;
}

Size EditLine::CalcTextSize( ParaPortion& rParaPortion )
{
    Size aSz;
    Size aTmpSz;

    DBG_ASSERT( rParaPortion.GetTextPortions().Count(), "GetTextSize before CreatePortions !" );

    for ( sal_Int32 n = nStartPortion; n <= nEndPortion; n++ )
    {
        TextPortion& rPortion = rParaPortion.GetTextPortions()[n];
        switch ( rPortion.GetKind() )
        {
            case PortionKind::TEXT:
            case PortionKind::FIELD:
            case PortionKind::HYPHENATOR:
            {
                aTmpSz = rPortion.GetSize();
                aSz.AdjustWidth(aTmpSz.Width() );
                if ( aSz.Height() < aTmpSz.Height() )
                    aSz.setHeight( aTmpSz.Height() );
            }
            break;
            case PortionKind::TAB:
            {
                aSz.AdjustWidth(rPortion.GetSize().Width() );
            }
            break;
            case PortionKind::LINEBREAK: break;
        }
    }

    SetHeight( static_cast<sal_uInt16>(aSz.Height()) );
    return aSz;
}

void EditLine::SetHeight( sal_uInt16 nH, sal_uInt16 nTxtH )
{
    nHeight = nH;
    nTxtHeight = ( nTxtH ? nTxtH : nH );
}

void EditLine::SetStartPosX( sal_Int32 start )
{
    if (start > 0)
        nStartPosX = start;
    else
        nStartPosX = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
