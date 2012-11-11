/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <SwPortionHandler.hxx>
#include "viewopt.hxx"  // SwViewOptions

#include "portox.hxx"
#include "inftxt.hxx"       // GetTxtSize()

/*************************************************************************
 *               virtual SwToxPortion::Paint()
 *************************************************************************/

void SwToxPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, POR_TOX );
        SwTxtPortion::Paint( rInf );
    }
}

/*************************************************************************
 *                      class SwIsoToxPortion
 *************************************************************************/

SwLinePortion *SwIsoToxPortion::Compress() { return this; }

SwIsoToxPortion::SwIsoToxPortion() : nViewWidth(0)
{
    SetLen(1);
    SetWhichPor( POR_ISOTOX );
}

/*************************************************************************
 *               virtual SwIsoToxPortion::GetViewWidth()
 *************************************************************************/

KSHORT SwIsoToxPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Although we are const, nViewWidth should be calculated in the last
    // moment possible
    SwIsoToxPortion* pThis = (SwIsoToxPortion*)this;
    // nViewWidth need to be calculated
    if( !Width() && rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() &&
            !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings()   )
    {
        if( !nViewWidth )
            pThis->nViewWidth = rInf.GetTxtSize(OUString(' ')).Width();
    }
    else
        pThis->nViewWidth = 0;
    return nViewWidth;
}

/*************************************************************************
 *                 virtual SwIsoToxPortion::Format()
 *************************************************************************/

sal_Bool SwIsoToxPortion::Format( SwTxtFormatInfo &rInf )
{
    const sal_Bool bFull = SwLinePortion::Format( rInf );
    return bFull;
}

/*************************************************************************
 *               virtual SwIsoToxPortion::Paint()
 *************************************************************************/

void SwIsoToxPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
        rInf.DrawViewOpt( *this, POR_TOX );
}

/*************************************************************************
 *              virtual SwIsoToxPortion::HandlePortion()
 *************************************************************************/

void SwIsoToxPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    String aString;
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
