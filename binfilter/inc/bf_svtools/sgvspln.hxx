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

#ifndef _SGVSPLN_HXX
#define _SGVSPLN_HXX

namespace binfilter
{

/*************************************************************************
|*
|*    CalcSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines parametrischen
|*                      natÅrlichen oder periodischen kubischen
|*                      Polynomsplines. Die Eckpunkte des öbergebenen
|*                      Polygons werden als StÅtzstellen angenommen.
|*                      n liefert die Anzahl der Teilpolynome.
|*                      Ist die Berechnung fehlerfrei verlaufen, so
|*                      liefert die Funktion TRUE. Nur in diesem Fall
|*                      ist Speicher fÅr die Koeffizientenarrays
|*                      allokiert, der dann spÑter vom Aufrufer mittels
|*                      delete freizugeben ist.
|*    Ersterstellung    JOE 17-08.93
|*    Letzte Aenderung  JOE 17-08.93
|*
*************************************************************************/

BOOL CalcSpline(Polygon& rPoly, BOOL Periodic, USHORT& n,
                double*& ax, double*& ay, double*& bx, double*& by,
                double*& cx, double*& cy, double*& dx, double*& dy, double*& T);

/*************************************************************************
|*
|*    Poly2Spline()
|*
|*    Beschreibung      Konvertiert einen parametrichen kubischen
|*                      Polynomspline Spline (natÅrlich oder periodisch)
|*                      in ein angenÑhertes Polygon.
|*                      Die Funktion liefert FALSE, wenn ein Fehler bei
|*                      der Koeffizientenberechnung aufgetreten ist oder
|*                      das Polygon zu gro· wird (>PolyMax=16380). Im 1.
|*                      Fall hat das Polygon 0, im 2. Fall PolyMax Punkte.
|*                      Um KoordinatenÅberlÑufe zu vermeiden werden diese
|*                      auf +/-32000 begrenzt.
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL Spline2Poly(Polygon& rSpln, BOOL Periodic, Polygon& rPoly);

}

#endif //_SGVSPLN_HXX
