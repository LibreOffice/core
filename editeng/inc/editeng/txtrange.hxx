/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _MyTXTRANGE_HXX
#define _MyTXTRANGE_HXX

#ifndef _TXTRANGE_HXX
#define _SVSTDARR_sal_BoolS
#define _SVSTDARR_LONGS
#include <svl/svstdarr.hxx>
#endif
#include "editeng/editengdllapi.h"

class PolyPolygon;
class Range;
class Rectangle;

namespace basegfx {
    class B2DPolyPolygon;
}

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
class EDITENG_DLLPUBLIC TextRanger
{
    Range *pRangeArr;
    SvLongsPtr *pCache;
    PolyPolygon *mpPolyPolygon; // Flaechenpolygon
    PolyPolygon *mpLinePolyPolygon; // Linienpolygon
    Rectangle *pBound;  // Umfassendes Rechteck
    sal_uInt16 nCacheSize;  // Cache-Size
    sal_uInt16 nCacheIdx;   // Cache-Index
    sal_uInt16 nRight;      // Abstand Kontur-Text
    sal_uInt16 nLeft;       // Abstand Text-Kontur
    sal_uInt16 nUpper;      // Abstand Kontur-Text
    sal_uInt16 nLower;      // Abstand Text-Kontur
    sal_uInt32 nPointCount; // Anzahl der Polygonpunkte
    sal_Bool bSimple : 1;   // Nur Aussenkante
    sal_Bool bInner  : 1;   // sal_True: Objekt beschriften (EditEngine);
                        // sal_False: Objekt umfliessen (StarWriter);
    sal_Bool bVertical :1;  // for vertical writing mode
    sal_Bool bFlag3 :1;
    sal_Bool bFlag4 :1;
    sal_Bool bFlag5 :1;
    sal_Bool bFlag6 :1;
    sal_Bool bFlag7 :1;
    TextRanger( const TextRanger& ); // not implemented
    const Rectangle& _GetBoundRect();
public:
    TextRanger( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon,
                sal_uInt16 nCacheSize, sal_uInt16 nLeft, sal_uInt16 nRight,
                sal_Bool bSimple, sal_Bool bInner, sal_Bool bVert = sal_False );
    ~TextRanger();
    SvLongsPtr GetTextRanges( const Range& rRange );
    sal_uInt16 GetRight() const { return nRight; }
    sal_uInt16 GetLeft() const { return nLeft; }
    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    sal_uInt32 GetPointCount() const { return nPointCount; }
    sal_Bool IsSimple() const { return bSimple; }
    sal_Bool IsInner() const { return bInner; }
    sal_Bool IsVertical() const { return bVertical; }
    sal_Bool HasBorder() const { return nRight || nLeft; }
    const PolyPolygon& GetPolyPolygon() const { return *mpPolyPolygon; }
    const PolyPolygon* GetLinePolygon() const { return mpLinePolyPolygon; }
    const Rectangle& GetBoundRect()
        { return pBound ? static_cast< const Rectangle& >(*pBound) : _GetBoundRect(); }
    void SetUpper( sal_uInt16 nNew ){ nUpper = nNew; }
    void SetLower( sal_uInt16 nNew ){ nLower = nNew; }
    void SetVertical( sal_Bool bNew );
    sal_Bool IsFlag3() const { return bFlag3; }
    void SetFlag3( sal_Bool bNew ) { bFlag3 = bNew; }
    sal_Bool IsFlag4() const { return bFlag4; }
    void SetFlag4( sal_Bool bNew ) { bFlag4 = bNew; }
    sal_Bool IsFlag5() const { return bFlag5; }
    void SetFlag5( sal_Bool bNew ) { bFlag5 = bNew; }
    sal_Bool IsFlag6() const { return bFlag6; }
    void SetFlag6( sal_Bool bNew ) { bFlag6 = bNew; }
    sal_Bool IsFlag7() const { return bFlag7; }
    void SetFlag7( sal_Bool bNew ) { bFlag7 = bNew; }
};



#endif      // _TXTRANGE_HXX
