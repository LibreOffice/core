/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dcolor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:31:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _B3D_B3DCOLOR_HXX
#define _B3D_B3DCOLOR_HXX

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

/*************************************************************************
|*
|* Farbklasse fuer 3D. Besitzt einige Funktionen mehr, als die normale
|* Farbe
|*
\************************************************************************/

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
    void CalcMiddle(Color& rOld1, Color& rOld2, Color& rOld3);
    ULONG GetDistance(Color& rOld);

    // Addition, Subtraktion mit clamping
    B3dColor&   operator+=  (const B3dColor&);
    B3dColor&   operator-=  (const B3dColor&);
    B3dColor    operator+   (const B3dColor&) const;
    B3dColor    operator-   (const B3dColor&) const;

    // Multiplikation als Gewichtung, Anwendung einer Lampe
    // auf eine Farbe, Lampe als 2.Faktor
    B3dColor&   operator*=  (const B3dColor&);
    B3dColor    operator*   (const B3dColor&) const;

    // Multiplikation mit Faktor im Bereich [0.0 .. 1.0]
    B3dColor&   operator*=  (const double);
    B3dColor    operator*   (const double) const;

    // Zuweisung
    void operator=(const Color& rCol) { mnColor = rCol.GetColor(); }
};

#endif          // _B3D_B3DCOLOR_HXX
