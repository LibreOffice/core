/*************************************************************************
 *
 *  $RCSfile: scrrect.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _SCRRECT_HXX
#define _SCRRECT_HXX


#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#include "swrect.hxx"
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

class SwStripe
{
    SwTwips nY;
    SwTwips nHeight;
public:
    inline SwStripe( SwTwips nPos, SwTwips nHght )
        : nY( nPos ), nHeight( nHght ) {}
    inline SwTwips GetY() const { return nY; }
    inline SwTwips& Y() { return nY; }
    inline SwTwips GetHeight() const { return nHeight; }
    inline SwTwips& Height() { return nHeight; }
    inline SwTwips GetBottom() const { return nHeight ? nY + nHeight - 1 : nY; }
    inline BOOL operator<( const SwStripe &rTst ) const
        { return nY < rTst.nY || ( nY == rTst.nY && nHeight > rTst.nHeight ); }
    inline BOOL operator==( const SwStripe& rTst ) const
        { return nY == rTst.nY && nHeight == rTst.nHeight; }
};

SV_DECL_VARARR( SwStripeArr, SwStripe, 1, 4 );

class SwStripes : public SwStripeArr, public SwStripe
{
    SwTwips nMin;       // Left minimum
    SwTwips nMax;       // Right maximum
public:
    SwStripes( const SwRect& rRect ) : SwStripe( rRect.Top(), rRect.Height() ),
        nMin(rRect.Left()), nMax(rRect.Left() + rRect.Width()) {}
    SwStripes& operator+=( const SwStripes& rOther );
    BOOL Recalc();
    inline SwTwips GetMin() const { return nMin; }
    inline SwTwips GetMax() const { return nMax; }
    inline void SetMin( const SwTwips nNew ) { nMin = nNew; }
    inline void SetMax( const SwTwips nNew ) { nMax = nNew; }
    inline void ChkMin( const SwTwips nNew ) { if( nNew < nMin ) nMin = nNew; }
    inline void ChkMax( const SwTwips nNew ) { if( nNew > nMax ) nMax = nNew; }
    inline SwTwips GetRight() const { return nMax > nMin ? nMax - 1 : nMax; }
    inline SwTwips GetWidth() const { return nMax - nMin; }
};

class SwScrollColumn
{
    SwTwips nX;
    SwTwips nWidth;
    SwTwips nOffs;
public:
    inline SwScrollColumn( const SwRect& rRect, SwTwips nOff )
        : nX( rRect.Left() ), nWidth( rRect.Width() ), nOffs( nOff ) {}
    inline SwScrollColumn( const SwScrollColumn& rCol )
        : nX( rCol.nX ), nWidth( rCol.nWidth ), nOffs( rCol.nOffs ) {}
    inline SwTwips GetX() const { return nX; }
    inline SwTwips GetWidth() const { return nWidth; }
    inline SwTwips GetOffs() const { return nOffs; }
    inline void ClrOffs() { nOffs = 0; }
    inline SwTwips GetRight() const { return nWidth ? nX + nWidth - 1 : nX; }
    inline BOOL operator<( const SwScrollColumn &rTst ) const
        { return nX < rTst.nX || ( nX == rTst.nX && ( nWidth < rTst.nWidth ||
          ( nWidth == rTst.nWidth && nOffs < rTst.nOffs ) ) ); }
    inline BOOL operator==( const SwScrollColumn &rTst ) const
        { return nX == rTst.nX && nWidth == rTst.nWidth && nOffs == rTst.nOffs;}
};

typedef SwStripes* SwStripesPtr;
SV_DECL_PTRARR_SORT(SwScrollStripes, SwStripesPtr, 1, 4)

class SwScrollArea : public SwScrollColumn, public SwScrollStripes
{
public:
    void SmartInsert( SwStripes* pStripes );
    void Add( SwScrollArea *pScroll );
    inline SwScrollArea( const SwScrollColumn &rCol, SwStripes* pStripes )
        : SwScrollColumn( rCol )
        { Insert( pStripes ); }
    BOOL Compress();
};

typedef SwScrollArea* SwScrollAreaPtr;
SV_DECL_PTRARR_SORT(SScrAreas,SwScrollAreaPtr,1,2);

class SwScrollAreas : public SScrAreas
{
public:
    void InsertCol( const SwScrollColumn &rCol, SwStripes *pStripes );
};


#endif //_SCRRECT_HXX
