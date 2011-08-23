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

#ifndef _B3D_B3DCOLOR_HXX
#define _B3D_B3DCOLOR_HXX

#include <tools/color.hxx>

/*************************************************************************
|*
|* Farbklasse fuer 3D. Besitzt einige Funktionen mehr, als die normale
|* Farbe
|*
\************************************************************************/

namespace binfilter {
class B3dColor : public Color
{
public:
    B3dColor() : Color() {}
    B3dColor( ColorData nColor ) : Color(nColor) {}
    B3dColor( UINT8 nRed, UINT8 nGreen, UINT8 nBlue )
        : Color(nRed, nGreen, nBlue) {}
    B3dColor( UINT8 nTransparency, UINT8 nRed, UINT8 nGreen, UINT8 nBlue )
        : Color(nTransparency, nRed, nGreen, nBlue) {}
    B3dColor( const ResId& rResId ) : Color(rResId) {}
    B3dColor( const Color& rCol ) : Color(rCol) {}

    void CalcInBetween(Color& rOld1, Color& rOld2, double t);
    void CalcMiddle(Color& rOld1, Color& rOld2);

    // Addition, Subtraktion mit clamping
    B3dColor&	operator+=	(const B3dColor&);
    B3dColor&	operator-=	(const B3dColor&);
    B3dColor	operator+ 	(const B3dColor&) const;
    B3dColor	operator-	(const B3dColor&) const;

    // Multiplikation als Gewichtung, Anwendung einer Lampe
    // auf eine Farbe, Lampe als 2.Faktor
    B3dColor&	operator*=	(const B3dColor&);
    B3dColor	operator* 	(const B3dColor&) const;

    // Multiplikation mit Faktor im Bereich [0.0 .. 1.0]
    B3dColor&	operator*=	(const double);
    B3dColor	operator* 	(const double) const;

    // Zuweisung
    void operator=(const Color& rCol) { mnColor = rCol.GetColor(); }
};
}//end of namespace binfilter

#endif          // _B3D_B3DCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
