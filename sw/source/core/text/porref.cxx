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

#include <SwPortionHandler.hxx>
#include <viewopt.hxx>

#include "porref.hxx"
#include "inftxt.hxx"

void SwRefPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, PortionType::Ref );
        SwTextPortion::Paint( rInf );
    }
}

SwLinePortion *SwIsoRefPortion::Compress() { return this; }

SwIsoRefPortion::SwIsoRefPortion() : nViewWidth(0)
{
    SetLen(TextFrameIndex(1));
    SetWhichPor( PortionType::IsoRef );
}

sal_uInt16 SwIsoRefPortion::GetViewWidth( const SwTextSizeInfo &rInf ) const
{
    // Although we are const, nViewWidth should be calculated in the last
    // moment possible
    SwIsoRefPortion* pThis = const_cast<SwIsoRefPortion*>(this);
    if( !Width() && rInf.OnWin() && SwViewOption::IsFieldShadings() &&
            !rInf.GetOpt().IsReadonly() && !rInf.GetOpt().IsPagePreview() )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTextSize(OUString(' ')).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

bool SwIsoRefPortion::Format( SwTextFormatInfo &rInf )
{
    return SwLinePortion::Format( rInf );
}

void SwIsoRefPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
        rInf.DrawViewOpt( *this, PortionType::Ref );
}

void SwIsoRefPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString(), GetWhichPor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
