/*************************************************************************
 *
 *  $RCSfile: sgvspln.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
