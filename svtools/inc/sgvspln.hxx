/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sgvspln.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:48:46 $
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

#ifndef _SGVSPLN_HXX
#define _SGVSPLN_HXX

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

#endif //_SGVSPLN_HXX
