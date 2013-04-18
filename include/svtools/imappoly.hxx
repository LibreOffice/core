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

#ifndef _GOODIES_IMAPPOLY_HXX
#define _GOODIES_IMAPPOLY_HXX

#include "svtools/svtdllapi.h"
#include <svtools/imapobj.hxx>
#include <tools/poly.hxx>

class Fraction;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
