/*************************************************************************
 *
 *  $RCSfile: gen.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <gen.hxx>
#endif
#ifndef _STREAM_HXX
#include <stream.hxx>
#endif

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
        UINT32          nNum;

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
        rPair.nA = (INT32)nNum;

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
        rPair.nB = (INT32)nNum;
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
        UINT32          nNum;

        cAry[0] = 0;

        nNum = (UINT32)(INT32)rPair.nA;
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

        nNum = (UINT32)(INT32)rPair.nB;
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
|*    Beschreibung      GEN.SDW
|*    Ersterstellung    DV 29.10.91
|*    Letzte Aenderung  MM 21.04.94
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
|*    Beschreibung      GEN.SDW
|*    Ersterstellung    TH 20.10.92
|*    Letzte Aenderung  MM 21.04.94
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
|*    Beschreibung      GEN.SDW
|*    Ersterstellung    TH 20.10.92
|*    Letzte Aenderung  MM 21.04.94
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
|*    Beschreibung      GEN.SDW
|*    Ersterstellung    DV 07.03.91
|*    Letzte Aenderung  DV 07.03.91
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
|*    Beschreibung      GEN.SDW
|*    Ersterstellung    TH 19.03.90
|*    Letzte Aenderung  MM 21.04.94
|*
*************************************************************************/

BOOL Rectangle::IsInside( const Point& rPoint ) const
{
    if ( IsEmpty() )
        return FALSE;

    BOOL bRet = TRUE;
    if ( nLeft <= nRight )
    {
        if ( (rPoint.X() < nLeft) || (rPoint.X() > nRight) )
            bRet = FALSE;
    }
    else
    {
        if ( (rPoint.X() > nLeft) || (rPoint.X() < nRight) )
            bRet = FALSE;
    }
    if ( nTop <= nBottom )
    {
        if ( (rPoint.Y() < nTop) || (rPoint.Y() > nBottom) )
            bRet = FALSE;
    }
    else
    {
        if ( (rPoint.Y() > nTop) || (rPoint.Y() < nBottom) )
            bRet = FALSE;
    }
    return bRet;
}

/*************************************************************************
|*
|*    Rectangle::IsInside()
|*
|*    Beschreibung      GEN.SDW
|*    Ersterstellung    TH 19.03.90
|*    Letzte Aenderung  MM 21.04.94
|*
*************************************************************************/

BOOL Rectangle::IsInside( const Rectangle& rRect ) const
{
    if ( IsInside( rRect.TopLeft() ) && IsInside( rRect.BottomRight() ) )
        return TRUE;
    else
        return FALSE;
}

/*************************************************************************
|*
|*    Rectangle::IsOver()
|*
|*    Beschreibung      GEN.SDW
|*    Ersterstellung    TH 19.03.90
|*    Letzte Aenderung  MM 21.04.94
|*
*************************************************************************/

BOOL Rectangle::IsOver( const Rectangle& rRect ) const
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
        UINT32          nNum;

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
        rRect.nLeft = (INT32)nNum;

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
        rRect.nTop = (INT32)nNum;

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
        rRect.nRight = (INT32)nNum;

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
        rRect.nBottom = (INT32)nNum;
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
        UINT32          nNum;

        cAry[0] = 0;
        cAry[1] = 0;

        nNum = (UINT32)(INT32)rRect.nLeft;
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

        nNum = (UINT32)(INT32)rRect.nTop;
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

        nNum = (UINT32)(INT32)rRect.nRight;
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

        nNum = (UINT32)(INT32)rRect.nBottom;
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
