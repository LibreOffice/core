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

#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

SvStream& operator>>( SvStream& rIStream, Pair& rPair )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "Pair::>> - Solar-Version not set on rIStream" );

    //39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nTmpA(0), nTmpB(0);
    rIStream >> nTmpA >> nTmpB;
    rPair.nA = nTmpA;
    rPair.nB = nTmpB;

    return rIStream;
}

SvStream& operator<<( SvStream& rOStream, const Pair& rPair )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Pair::<< - Solar-Version not set on rOStream" );

    //39428 SvStream no longer supports operator<<(long)
    rOStream << sal::static_int_cast<sal_Int32>(rPair.nA) << sal::static_int_cast<sal_Int32>(rPair.nB);

    return rOStream;
}

void Rectangle::SetSize( const Size& rSize )
{
    if ( rSize.Width() < 0 )
        nRight  = nLeft + rSize.Width() +1;
    else if ( rSize.Width() > 0 )
        nRight  = nLeft + rSize.Width() -1;
    else
        nRight = RECT_EMPTY;

    if ( rSize.Height() < 0 )
        nBottom  = nTop + rSize.Height() +1;
    else if ( rSize.Height() > 0 )
        nBottom  = nTop + rSize.Height() -1;
    else
        nBottom = RECT_EMPTY;
}

Rectangle& Rectangle::Union( const Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
        return *this;

    if ( IsEmpty() )
        *this = rRect;
    else
    {
        nLeft  =  Min( Min( nLeft, rRect.nLeft ), Min( nRight, rRect.nRight )   );
        nRight  = Max( Max( nLeft, rRect.nLeft ), Max( nRight, rRect.nRight )   );
        nTop    = Min( Min( nTop, rRect.nTop ),   Min( nBottom, rRect.nBottom ) );
        nBottom = Max( Max( nTop, rRect.nTop ),   Max( nBottom, rRect.nBottom ) );
    }

    return *this;
}

Rectangle& Rectangle::Intersection( const Rectangle& rRect )
{
    if ( IsEmpty() )
        return *this;
    if ( rRect.IsEmpty() )
    {
        *this = Rectangle();
        return *this;
    }

    // nicht mit umgedrehten Rechtecken arbeiten
    Rectangle aTmpRect( rRect );
    Justify();
    aTmpRect.Justify();

    // Schnitt bilden
    nLeft  = Max( nLeft, aTmpRect.nLeft );
    nRight = Min( nRight, aTmpRect.nRight );
    nTop   = Max( nTop, aTmpRect.nTop );
    nBottom= Min( nBottom, aTmpRect.nBottom );

    // Feststellen ob Schnitt leer
    if ( nRight < nLeft || nBottom < nTop )
        *this = Rectangle();

    return *this;
}

void Rectangle::Justify()
{
    long nHelp;

    // Abfrage, ob Right kleiner Left
    if ( (nRight < nLeft) && (nRight != RECT_EMPTY) )
    {
        nHelp = nLeft;
        nLeft = nRight;
        nRight = nHelp;
    }

    // Abfrage, ob Bottom kleiner Top
    if ( (nBottom < nTop) && (nBottom != RECT_EMPTY) )
    {
        nHelp = nBottom;
        nBottom = nTop;
        nTop = nHelp;
    }
}

sal_Bool Rectangle::IsInside( const Point& rPoint ) const
{
    if ( IsEmpty() )
        return sal_False;

    sal_Bool bRet = sal_True;
    if ( nLeft <= nRight )
    {
        if ( (rPoint.X() < nLeft) || (rPoint.X() > nRight) )
            bRet = sal_False;
    }
    else
    {
        if ( (rPoint.X() > nLeft) || (rPoint.X() < nRight) )
            bRet = sal_False;
    }
    if ( nTop <= nBottom )
    {
        if ( (rPoint.Y() < nTop) || (rPoint.Y() > nBottom) )
            bRet = sal_False;
    }
    else
    {
        if ( (rPoint.Y() > nTop) || (rPoint.Y() < nBottom) )
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool Rectangle::IsInside( const Rectangle& rRect ) const
{
    if ( IsInside( rRect.TopLeft() ) && IsInside( rRect.BottomRight() ) )
        return sal_True;
    else
        return sal_False;
}

sal_Bool Rectangle::IsOver( const Rectangle& rRect ) const
{
    // Wenn sie sich nicht schneiden, ueberlappen sie auch nicht
    return !GetIntersection( rRect ).IsEmpty();
}

SvStream& operator>>( SvStream& rIStream, Rectangle& rRect )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "Rectangle::>> - Solar-Version not set on rIStream" );

    //fdo#39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nTmpL(0), nTmpT(0), nTmpR(0), nTmpB(0);

    rIStream >> nTmpL >> nTmpT >> nTmpR >> nTmpB;

    rRect.nLeft = nTmpL;
    rRect.nTop = nTmpT;
    rRect.nRight = nTmpR;
    rRect.nBottom = nTmpB;

    return rIStream;
}

SvStream& operator<<( SvStream& rOStream, const Rectangle& rRect )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Rectangle::<< - Solar-Version not set on rOStream" );

    //fdo#39428 SvStream no longer supports operator<<(long)
    rOStream << sal::static_int_cast<sal_Int32>(rRect.nLeft)
             << sal::static_int_cast<sal_Int32>(rRect.nTop)
             << sal::static_int_cast<sal_Int32>(rRect.nRight)
             << sal::static_int_cast<sal_Int32>(rRect.nBottom);

    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
