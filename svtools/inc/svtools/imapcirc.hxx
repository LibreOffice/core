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


#ifndef _GOODIES_IMAPCIRC_HXX
#define _GOODIES_IMAPCIRC_HXX

#include "svtools/svtdllapi.h"
#include <tools/gen.hxx>
#include <svtools/imapobj.hxx>

class Fraction;

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class SVT_DLLPUBLIC IMapCircleObject : public IMapObject
{
    Point               aCenter;
    sal_uLong               nRadius;

    void                ImpConstruct( const Point& rCenter, sal_uLong nRad, sal_Bool bPixel );

protected:

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const;
    virtual void        ReadIMapObject(  SvStream& rIStm );

public:

                        IMapCircleObject() {};
                        IMapCircleObject( const Point& rCenter, sal_uLong nRad,
                                          const String& rURL,
                                          const String& rAltText,
                                          const String& rDesc,
                                          const String& rTarget,
                                          const String& rName,
                                          sal_Bool bActive = sal_True,
                                          sal_Bool bPixelCoords = sal_True );
    virtual             ~IMapCircleObject() {};

    virtual sal_uInt16      GetType() const;
    virtual sal_Bool        IsHit( const Point& rPoint ) const;

    Point               GetCenter( sal_Bool bPixelCoords = sal_True ) const;
    sal_uLong               GetRadius( sal_Bool bPixelCoords = sal_True ) const;

    // liefert das BoundRect des Kreis-Objektes in 1/100mm
    virtual Rectangle   GetBoundRect() const;

    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    sal_Bool                IsEqual( const IMapCircleObject& rEqObj );

    // Im-/Export
    void                WriteCERN( SvStream& rOStm, const String& rBaseURL  ) const;
    void                WriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const;
};

#endif
