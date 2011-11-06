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



#ifndef _GOODIES_IMAPPOLY_HXX
#define _GOODIES_IMAPPOLY_HXX

#include "svtools/svtdllapi.h"
#include <svtools/imapobj.hxx>
#include <tools/poly.hxx>

class Fraction;

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class SVT_DLLPUBLIC IMapPolygonObject : public IMapObject
{
    Polygon             aPoly;
    Rectangle           aEllipse;
    sal_Bool                bEllipse;

    SVT_DLLPRIVATE void             ImpConstruct( const Polygon& rPoly, sal_Bool bPixel );

protected:

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const;
    virtual void        ReadIMapObject(  SvStream& rIStm );

    sal_Bool                ReadPreProLine( SvStream& rIStm, String& rStr );

public:
                        IMapPolygonObject() {};
                        IMapPolygonObject( const Polygon& rPoly,
                                           const String& rURL,
                                           const String& rAltText,
                                           const String& rDesc,
                                           const String& rTarget,
                                           const String& rName,
                                           sal_Bool bActive = sal_True,
                                           sal_Bool bPixelCoords = sal_True );
    virtual             ~IMapPolygonObject() {};

    virtual sal_uInt16      GetType() const;
    virtual sal_Bool        IsHit( const Point& rPoint ) const;

    Polygon             GetPolygon( sal_Bool bPixelCoords = sal_True ) const;

    // liefert das BoundRect des Polygon-Objektes in 1/100mm
    virtual Rectangle   GetBoundRect() const {  return aPoly.GetBoundRect(); }

    sal_Bool                HasExtraEllipse() const { return bEllipse; }
    const Rectangle&    GetExtraEllipse() const { return aEllipse; }
    void                SetExtraEllipse( const Rectangle& rEllipse );

    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    sal_Bool                IsEqual( const IMapPolygonObject& rEqObj );

    // Im-/Export
    void                WriteCERN( SvStream& rOStm, const String& rBaseURL  ) const;
    void                WriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const;
};

#endif
