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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
