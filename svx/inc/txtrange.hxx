/*************************************************************************
 *
 *  $RCSfile: txtrange.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-16 16:06:31 $
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

#ifndef _MyTXTRANGE_HXX
#define _MyTXTRANGE_HXX

#ifndef _TXTRANGE_HXX
#define _SVSTDARR_BOOLS
#define _SVSTDARR_LONGS
#include <svtools/svstdarr.hxx>
#endif

class PolyPolygon;
class XPolyPolygon;
class Range;
class Rectangle;

typedef SvLongs* SvLongsPtr;

/*************************************************************************
|*
|*    class TextRanger
|*
|*    Beschreibung
|*    Ersterstellung       20.01.97
|*    Letzte Aenderung AMA 20.01.97
|*
*************************************************************************/
class TextRanger
{
    Range *pRangeArr;
    SvLongsPtr *pCache;
    PolyPolygon *pPoly; // Flaechenpolygon
    PolyPolygon *pLine; // Linienpolygon
    Rectangle *pBound;  // Umfassendes Rechteck
    USHORT nCacheSize;  // Cache-Size
    USHORT nCacheIdx;   // Cache-Index
    USHORT nRight;      // Abstand Kontur-Text
    USHORT nLeft;       // Abstand Text-Kontur
    USHORT nUpper;      // Abstand Kontur-Text
    USHORT nLower;      // Abstand Text-Kontur
    USHORT nPointCount; // Anzahl der Polygonpunkte
    BOOL bSimple : 1;   // Nur Aussenkante
    BOOL bInner  : 1;   // TRUE: Objekt beschriften (EditEngine);
                        // FALSE: Objekt umfliessen (StarWriter);
    BOOL bVertical :1;  // for vertical writing mode
    BOOL bFlag3 :1;
    BOOL bFlag4 :1;
    BOOL bFlag5 :1;
    BOOL bFlag6 :1;
    BOOL bFlag7 :1;
    TextRanger( const TextRanger& ); // not implemented
    const Rectangle& _GetBoundRect();
public:
    TextRanger( const XPolyPolygon& rXPoly, const XPolyPolygon* pXLine,
                USHORT nCacheSize, USHORT nLeft, USHORT nRight,
                BOOL bSimple, BOOL bInner, BOOL bVert = sal_False );
    ~TextRanger();
    SvLongsPtr GetTextRanges( const Range& rRange );
    USHORT GetRight() const { return nRight; }
    USHORT GetLeft() const { return nLeft; }
    USHORT GetUpper() const { return nUpper; }
    USHORT GetLower() const { return nLower; }
    USHORT GetPointCount() const { return nPointCount; }
    BOOL IsSimple() const { return bSimple; }
    BOOL IsInner() const { return bInner; }
    BOOL IsVertical() const { return bVertical; }
    BOOL HasBorder() const { return nRight || nLeft; }
    const PolyPolygon& GetPolyPolygon() const { return *pPoly; }
    const PolyPolygon* GetLinePolygon() const { return pLine; }
    const Rectangle& GetBoundRect()
        { return pBound ? *pBound : _GetBoundRect(); }
    void SetUpper( USHORT nNew ){ nUpper = nNew; }
    void SetLower( USHORT nNew ){ nLower = nNew; }
    BOOL IsFlag3() const { return bFlag3; }
    void SetFlag3( BOOL bNew ) { bFlag3 = bNew; }
    BOOL IsFlag4() const { return bFlag4; }
    void SetFlag4( BOOL bNew ) { bFlag4 = bNew; }
    BOOL IsFlag5() const { return bFlag5; }
    void SetFlag5( BOOL bNew ) { bFlag5 = bNew; }
    BOOL IsFlag6() const { return bFlag6; }
    void SetFlag6( BOOL bNew ) { bFlag6 = bNew; }
    BOOL IsFlag7() const { return bFlag7; }
    void SetFlag7( BOOL bNew ) { bFlag7 = bNew; }
};



#endif      // _TXTRANGE_HXX
