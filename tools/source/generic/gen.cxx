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

#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

// =======================================================================

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

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const Pair& rPair )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Pair::<< - Solar-Version not set on rOStream" );

    //39428 SvStream no longer supports operator<<(long)
    rOStream << sal::static_int_cast<sal_Int32>(rPair.nA) << sal::static_int_cast<sal_Int32>(rPair.nB);

    return rOStream;
}

/*************************************************************************
|*
|*    Rectangle::SetSize()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    Rectangle::Union()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    Rectangle::Intersection()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    Rectangle::Justify()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    Rectangle::IsInside()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    Rectangle::IsInside()
|*
*************************************************************************/

sal_Bool Rectangle::IsInside( const Rectangle& rRect ) const
{
    if ( IsInside( rRect.TopLeft() ) && IsInside( rRect.BottomRight() ) )
        return sal_True;
    else
        return sal_False;
}

/*************************************************************************
|*
|*    Rectangle::IsOver()
|*
*************************************************************************/

sal_Bool Rectangle::IsOver( const Rectangle& rRect ) const
{
    // Wenn sie sich nicht schneiden, ueberlappen sie auch nicht
    return !GetIntersection( rRect ).IsEmpty();
}

// =======================================================================

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

// -----------------------------------------------------------------------

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
