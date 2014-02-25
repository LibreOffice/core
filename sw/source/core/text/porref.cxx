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
#include "viewopt.hxx"

#include "porref.hxx"
#include "inftxt.hxx"

/*************************************************************************
 *               virtual SwRefPortion::Paint()
 *************************************************************************/

void SwRefPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, POR_REF );
        SwTxtPortion::Paint( rInf );
    }
}

/*************************************************************************
 *                      class SwIsoRefPortion
 *************************************************************************/

SwLinePortion *SwIsoRefPortion::Compress() { return this; }

SwIsoRefPortion::SwIsoRefPortion() : nViewWidth(0)
{
    SetLen(1);
    SetWhichPor( POR_ISOREF );
}

/*************************************************************************
 *               virtual SwIsoRefPortion::GetViewWidth()
 *************************************************************************/

KSHORT SwIsoRefPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Although we are const, nViewWidth should be calculated in the last
    // moment possible
    SwIsoRefPortion* pThis = (SwIsoRefPortion*)this;
    if( !Width() && rInf.OnWin() && SwViewOption::IsFieldShadings() &&
            !rInf.GetOpt().IsReadonly() && !rInf.GetOpt().IsPagePreview() )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTxtSize(OUString(' ')).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

/*************************************************************************
 *                 virtual SwIsoRefPortion::Format()
 *************************************************************************/

bool SwIsoRefPortion::Format( SwTxtFormatInfo &rInf )
{
    return SwLinePortion::Format( rInf );
}

/*************************************************************************
 *               virtual SwIsoRefPortion::Paint()
 *************************************************************************/

void SwIsoRefPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
        rInf.DrawViewOpt( *this, POR_REF );
}

/*************************************************************************
 *              virtual SwIsoRefPortion::HandlePortion()
 *************************************************************************/

void SwIsoRefPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    OUString aString;
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
