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

#ifndef XCHAR_HXX
#define XCHAR_HXX


#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _XPOLY_HXX //autogen
#include <bf_svx/xpoly.hxx>
#endif


#include "rect.hxx"
namespace binfilter {




////////////////////////////////////////
// SmPolygon
//

class SmPolygonLoader;

class SmPolygon
{
    friend class SmPolygonLoader;

    XPolyPolygon	aPoly;
    Rectangle	  	aBoundRect;	// das umgebende Rechteck (ohne Rand!)
                                // wie von OutputDevice::GetBoundRect
    Size			aFontSize;  // vom Originalzeichen (Width != 0 aus FontMetric)
    Point			aPos;
    Size		  	aOrigSize;	// Originalgröße (des BoundRect)
    Point			aOrigPos;	// Offset des BoundRect im Originalzeichen

    double		  	fScaleX,    // Skalierungsfaktoren der aktuellen (gewollten)
                    fScaleY,	// Größe gegenüber der Originalgröße

                    fDelayedFactorX,	// dienen zum sammeln der Änderungen
                    fDelayedFactorY;	// bis tatsächlich skaliert wird

    sal_Unicode     cChar;
    BOOL			bDelayedScale,
                    bDelayedBoundRect;

    void			Scale();

public:
    SmPolygon();
    SmPolygon(sal_Unicode cChar);
    // default copy-constructor verwenden

    const Size &  	GetOrigFontSize() const { return aFontSize; };
    const Size &	GetOrigSize() const { return aOrigSize; }
    const Point	&	GetOrigPos() const { return aOrigPos; }

    void 			ScaleBy(double fFactorX, double fFactorY);
    double			GetScaleX() const { return fScaleX * fDelayedFactorX; }
    double			GetScaleY() const { return fScaleY * fDelayedFactorY; }

    void 			AdaptToX(const OutputDevice &rDev, ULONG nWidth);
    void 			AdaptToY(const OutputDevice &rDev, ULONG nHeight);

    void 			Move  (const Point &rPoint);
    void 			MoveTo(const Point &rPoint) { Move(rPoint - aPos); }

    const sal_Unicode       GetChar() const { return cChar; }
    const Rectangle & 		GetBoundRect(const OutputDevice &rDev) const;


    // default assignment-operator verwenden

    friend SvStream & operator >> (SvStream &rIStream, SmPolygon &rPoly);
    friend SvStream & operator << (SvStream &rOStream, const SmPolygon &rPoly);
};



} //namespace binfilter
#endif
