/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

SvStream& ReadPair( SvStream& rIStream, Pair& rPair )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "Pair::>> - Solar-Version not set on rIStream" );

    
    sal_Int32 nTmpA(0), nTmpB(0);
    rIStream.ReadInt32( nTmpA ).ReadInt32( nTmpB );
    rPair.nA = nTmpA;
    rPair.nB = nTmpB;

    return rIStream;
}

SvStream& WritePair( SvStream& rOStream, const Pair& rPair )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Pair::<< - Solar-Version not set on rOStream" );

    
    rOStream.WriteInt32( sal::static_int_cast<sal_Int32>(rPair.nA) ).WriteInt32( sal::static_int_cast<sal_Int32>(rPair.nB) );

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
        nLeft  =  std::min( std::min( nLeft, rRect.nLeft ), std::min( nRight, rRect.nRight )   );
        nRight  = std::max( std::max( nLeft, rRect.nLeft ), std::max( nRight, rRect.nRight )   );
        nTop    = std::min( std::min( nTop, rRect.nTop ),   std::min( nBottom, rRect.nBottom ) );
        nBottom = std::max( std::max( nTop, rRect.nTop ),   std::max( nBottom, rRect.nBottom ) );
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

    
    Rectangle aTmpRect( rRect );
    Justify();
    aTmpRect.Justify();

    
    nLeft  = std::max( nLeft, aTmpRect.nLeft );
    nRight = std::min( nRight, aTmpRect.nRight );
    nTop   = std::max( nTop, aTmpRect.nTop );
    nBottom= std::min( nBottom, aTmpRect.nBottom );

    
    if ( nRight < nLeft || nBottom < nTop )
        *this = Rectangle();

    return *this;
}

void Rectangle::Justify()
{
    long nHelp;

    if ( (nRight < nLeft) && (nRight != RECT_EMPTY) )
    {
        nHelp = nLeft;
        nLeft = nRight;
        nRight = nHelp;
    }

    if ( (nBottom < nTop) && (nBottom != RECT_EMPTY) )
    {
        nHelp = nBottom;
        nBottom = nTop;
        nTop = nHelp;
    }
}

bool Rectangle::IsInside( const Point& rPoint ) const
{
    if ( IsEmpty() )
        return false;

    bool bRet = true;
    if ( nLeft <= nRight )
    {
        if ( (rPoint.X() < nLeft) || (rPoint.X() > nRight) )
            bRet = false;
    }
    else
    {
        if ( (rPoint.X() > nLeft) || (rPoint.X() < nRight) )
            bRet = false;
    }
    if ( nTop <= nBottom )
    {
        if ( (rPoint.Y() < nTop) || (rPoint.Y() > nBottom) )
            bRet = false;
    }
    else
    {
        if ( (rPoint.Y() > nTop) || (rPoint.Y() < nBottom) )
            bRet = false;
    }
    return bRet;
}

bool Rectangle::IsInside( const Rectangle& rRect ) const
{
    if ( IsInside( rRect.TopLeft() ) && IsInside( rRect.BottomRight() ) )
        return true;
    else
        return false;
}

bool Rectangle::IsOver( const Rectangle& rRect ) const
{
    
    return !GetIntersection( rRect ).IsEmpty();
}

SvStream& ReadRectangle( SvStream& rIStream, Rectangle& rRect )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "Rectangle::>> - Solar-Version not set on rIStream" );

    
    sal_Int32 nTmpL(0), nTmpT(0), nTmpR(0), nTmpB(0);

    rIStream.ReadInt32( nTmpL ).ReadInt32( nTmpT ).ReadInt32( nTmpR ).ReadInt32( nTmpB );

    rRect.nLeft = nTmpL;
    rRect.nTop = nTmpT;
    rRect.nRight = nTmpR;
    rRect.nBottom = nTmpB;

    return rIStream;
}

SvStream& WriteRectangle( SvStream& rOStream, const Rectangle& rRect )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Rectangle::<< - Solar-Version not set on rOStream" );

    
    rOStream.WriteInt32( sal::static_int_cast<sal_Int32>(rRect.nLeft) )
            .WriteInt32( sal::static_int_cast<sal_Int32>(rRect.nTop) )
            .WriteInt32( sal::static_int_cast<sal_Int32>(rRect.nRight) )
            .WriteInt32( sal::static_int_cast<sal_Int32>(rRect.nBottom) );

    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
