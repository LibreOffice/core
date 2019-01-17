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

#include "portox.hxx"
#include "inftxt.hxx"

void SwToxPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, PortionType::Tox );
        SwTextPortion::Paint( rInf );
    }
}

SwLinePortion *SwIsoToxPortion::Compress() { return this; }

SwIsoToxPortion::SwIsoToxPortion() : nViewWidth(0)
{
    SetLen(TextFrameIndex(1));
    SetWhichPor( PortionType::IsoTox );
}

sal_uInt16 SwIsoToxPortion::GetViewWidth( const SwTextSizeInfo &rInf ) const
{
    // Although we are const, nViewWidth should be calculated in the last
    // moment possible
    SwIsoToxPortion* pThis = const_cast<SwIsoToxPortion*>(this);
    // nViewWidth need to be calculated
    if( !Width() && rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() &&
            !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings()   )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTextSize(OUString(' ')).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

bool SwIsoToxPortion::Format( SwTextFormatInfo &rInf )
{
    return SwLinePortion::Format( rInf );
}

void SwIsoToxPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
        rInf.DrawViewOpt( *this, PortionType::Tox );
}

void SwIsoToxPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString(), GetWhichPor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
