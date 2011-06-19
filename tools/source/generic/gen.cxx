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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>

// =======================================================================

SvStream& operator>>( SvStream& rIStream, Pair& rPair )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "Pair::>> - Solar-Version not set on rIStream" );

    if ( rIStream.GetCompressMode() == COMPRESSMODE_FULL )
    {
        unsigned char   cId;
        unsigned char   cAry[8];
        int             i;
        int             i1;
        int             i2;
        sal_uInt32          nNum;

        rIStream >> cId;
        i1 = (cId & 0x70) >> 4;
        i2 = cId & 0x07;
        rIStream.Read( cAry, i1+i2 );

        nNum = 0;
        i = i1;
        while ( i )
        {
            i--;
            nNum <<= 8;
            nNum |= cAry[i];
        }
        if ( cId & 0x80 )
            nNum ^= 0xFFFFFFFF;
        rPair.nA = (sal_Int32)nNum;

        nNum = 0;
        i = i1+i2;
        while ( i > i1 )
        {
            i--;
            nNum <<= 8;
            nNum |= cAry[i];
        }
        if ( cId & 0x08 )
            nNum ^= 0xFFFFFFFF;
        rPair.nB = (sal_Int32)nNum;
    }
    else
    {
        rIStream >> rPair.nA >> rPair.nB;
    }

    return rIStream;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const Pair& rPair )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Pair::<< - Solar-Version not set on rOStream" );

    if ( rOStream.GetCompressMode() == COMPRESSMODE_FULL )
    {
        unsigned char   cAry[9];
        int             i = 1;
        sal_uInt32          nNum;

        cAry[0] = 0;

        nNum = (sal_uInt32)(sal_Int32)rPair.nA;
        if ( rPair.nA < 0 )
        {
            cAry[0] |= 0x80;
            nNum ^= 0xFFFFFFFF;
        }
        if ( nNum )
        {
            cAry[i] = (unsigned char)(nNum & 0xFF);
            nNum >>= 8;
            i++;

            if ( nNum )
            {
                cAry[i] = (unsigned char)(nNum & 0xFF);
                nNum >>= 8;
                i++;

                if ( nNum )
                {
                    cAry[i] = (unsigned char)(nNum & 0xFF);
                    nNum >>= 8;
                    i++;

                    if ( nNum )
                    {
                        cAry[i] = (unsigned char)(nNum & 0xFF);
                        nNum >>= 8;
                        i++;
                        cAry[0] |= 0x40;
                    }
                    else
                        cAry[0] |= 0x30;
                }
                else
                    cAry[0] |= 0x20;
            }
            else
                cAry[0] |= 0x10;
        }

        nNum = (sal_uInt32)(sal_Int32)rPair.nB;
        if ( rPair.nB < 0 )
        {
            cAry[0] |= 0x08;
            nNum ^= 0xFFFFFFFF;
        }
        if ( nNum )
        {
            cAry[i] = (unsigned char)(nNum & 0xFF);
            nNum >>= 8;
            i++;

            if ( nNum )
            {
                cAry[i] = (unsigned char)(nNum & 0xFF);
                nNum >>= 8;
                i++;

                if ( nNum )
                {
                    cAry[i] = (unsigned char)(nNum & 0xFF);
                    nNum >>= 8;
                    i++;

                    if ( nNum )
                    {
                        cAry[i] = (unsigned char)(nNum & 0xFF);
                        nNum >>= 8;
                        i++;
                        cAry[0] |= 0x04;
                    }
                    else
                        cAry[0] |= 0x03;
                }
                else
                    cAry[0] |= 0x02;
            }
            else
                cAry[0] |= 0x01;
        }

        rOStream.Write( cAry, i );
    }
    else
    {
        rOStream << rPair.nA << rPair.nB;
    }

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

    if ( rIStream.GetCompressMode() == COMPRESSMODE_FULL )
    {
        unsigned char   cIdAry[2];
        unsigned char   cAry[16];
        int             i;
        int             iLast;
        int             i1;
        int             i2;
        int             i3;
        int             i4;
        sal_uInt32          nNum;

        rIStream.Read( cIdAry, 2 );
        i1 = (cIdAry[0] & 0x70) >> 4;
        i2 = cIdAry[0] & 0x07;
        i3 = (cIdAry[1] & 0x70) >> 4;
        i4 = cIdAry[1] & 0x07;
        rIStream.Read( cAry, i1+i2+i3+i4 );

        nNum = 0;
        i = i1;
        iLast = i;
        while ( i )
        {
            i--;
            nNum <<= 8;
            nNum |= cAry[i];
        }
        iLast = i1;
        if ( cIdAry[0] & 0x80 )
            nNum ^= 0xFFFFFFFF;
        rRect.nLeft = (sal_Int32)nNum;

        nNum = 0;
        i = iLast+i2;
        while ( i > iLast )
        {
            i--;
            nNum <<= 8;
            nNum |= cAry[i];
        }
        iLast += i2;
        if ( cIdAry[0] & 0x08 )
            nNum ^= 0xFFFFFFFF;
        rRect.nTop = (sal_Int32)nNum;

        nNum = 0;
        i = iLast+i3;
        while ( i > iLast )
        {
            i--;
            nNum <<= 8;
            nNum |= cAry[i];
        }
        iLast += i3;
        if ( cIdAry[1] & 0x80 )
            nNum ^= 0xFFFFFFFF;
        rRect.nRight = (sal_Int32)nNum;

        nNum = 0;
        i = iLast+i4;
        while ( i > iLast )
        {
            i--;
            nNum <<= 8;
            nNum |= cAry[i];
        }
        if ( cIdAry[1] & 0x08 )
            nNum ^= 0xFFFFFFFF;
        rRect.nBottom = (sal_Int32)nNum;
    }
    else
    {
        rIStream >> rRect.nLeft >> rRect.nTop >> rRect.nRight >> rRect.nBottom;
    }

    return rIStream;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const Rectangle& rRect )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Rectangle::<< - Solar-Version not set on rOStream" );

    if ( rOStream.GetCompressMode() == COMPRESSMODE_FULL )
    {
        unsigned char   cAry[18];
        int             i = 2;
        sal_uInt32          nNum;

        cAry[0] = 0;
        cAry[1] = 0;

        nNum = (sal_uInt32)(sal_Int32)rRect.nLeft;
        if ( rRect.nLeft < 0 )
        {
            cAry[0] |= 0x80;
            nNum ^= 0xFFFFFFFF;
        }
        if ( nNum )
        {
            cAry[i] = (unsigned char)(nNum & 0xFF);
            nNum >>= 8;
            i++;

            if ( nNum )
            {
                cAry[i] = (unsigned char)(nNum & 0xFF);
                nNum >>= 8;
                i++;

                if ( nNum )
                {
                    cAry[i] = (unsigned char)(nNum & 0xFF);
                    nNum >>= 8;
                    i++;

                    if ( nNum )
                    {
                        cAry[i] = (unsigned char)(nNum & 0xFF);
                        nNum >>= 8;
                        i++;
                        cAry[0] |= 0x40;
                    }
                    else
                        cAry[0] |= 0x30;
                }
                else
                    cAry[0] |= 0x20;
            }
            else
                cAry[0] |= 0x10;
        }

        nNum = (sal_uInt32)(sal_Int32)rRect.nTop;
        if ( rRect.nTop < 0 )
        {
            cAry[0] |= 0x08;
            nNum ^= 0xFFFFFFFF;
        }
        if ( nNum )
        {
            cAry[i] = (unsigned char)(nNum & 0xFF);
            nNum >>= 8;
            i++;

            if ( nNum )
            {
                cAry[i] = (unsigned char)(nNum & 0xFF);
                nNum >>= 8;
                i++;

                if ( nNum )
                {
                    cAry[i] = (unsigned char)(nNum & 0xFF);
                    nNum >>= 8;
                    i++;

                    if ( nNum )
                    {
                        cAry[i] = (unsigned char)(nNum & 0xFF);
                        nNum >>= 8;
                        i++;
                        cAry[0] |= 0x04;
                    }
                    else
                        cAry[0] |= 0x03;
                }
                else
                    cAry[0] |= 0x02;
            }
            else
                cAry[0] |= 0x01;
        }

        nNum = (sal_uInt32)(sal_Int32)rRect.nRight;
        if ( rRect.nRight < 0 )
        {
            cAry[1] |= 0x80;
            nNum ^= 0xFFFFFFFF;
        }
        if ( nNum )
        {
            cAry[i] = (unsigned char)(nNum & 0xFF);
            nNum >>= 8;
            i++;

            if ( nNum )
            {
                cAry[i] = (unsigned char)(nNum & 0xFF);
                nNum >>= 8;
                i++;

                if ( nNum )
                {
                    cAry[i] = (unsigned char)(nNum & 0xFF);
                    nNum >>= 8;
                    i++;

                    if ( nNum )
                    {
                        cAry[i] = (unsigned char)(nNum & 0xFF);
                        nNum >>= 8;
                        i++;
                        cAry[1] |= 0x40;
                    }
                    else
                        cAry[1] |= 0x30;
                }
                else
                    cAry[1] |= 0x20;
            }
            else
                cAry[1] |= 0x10;
        }

        nNum = (sal_uInt32)(sal_Int32)rRect.nBottom;
        if ( rRect.nBottom < 0 )
        {
            cAry[1] |= 0x08;
            nNum ^= 0xFFFFFFFF;
        }
        if ( nNum )
        {
            cAry[i] = (unsigned char)(nNum & 0xFF);
            nNum >>= 8;
            i++;

            if ( nNum )
            {
                cAry[i] = (unsigned char)(nNum & 0xFF);
                nNum >>= 8;
                i++;

                if ( nNum )
                {
                    cAry[i] = (unsigned char)(nNum & 0xFF);
                    nNum >>= 8;
                    i++;

                    if ( nNum )
                    {
                        cAry[i] = (unsigned char)(nNum & 0xFF);
                        nNum >>= 8;
                        i++;
                        cAry[1] |= 0x04;
                    }
                    else
                        cAry[1] |= 0x03;
                }
                else
                    cAry[1] |= 0x02;
            }
            else
                cAry[1] |= 0x01;
        }

        rOStream.Write( cAry, i );
    }
    else
    {
        rOStream << rRect.nLeft << rRect.nTop << rRect.nRight << rRect.nBottom;
    }

    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
