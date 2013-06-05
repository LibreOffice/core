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
|*                      liefert die Funktion true. Nur in diesem Fall
|*                      ist Speicher fÅr die Koeffizientenarrays
|*                      allokiert, der dann spÑter vom Aufrufer mittels
|*                      delete freizugeben ist.
|*
*************************************************************************/

bool CalcSpline(Polygon& rPoly, bool Periodic, sal_uInt16& n,
                double*& ax, double*& ay, double*& bx, double*& by,
                double*& cx, double*& cy, double*& dx, double*& dy, double*& T);

/*************************************************************************
|*
|*    Poly2Spline()
|*
|*    Beschreibung      Konvertiert einen parametrichen kubischen
|*                      Polynomspline Spline (natÅrlich oder periodisch)
|*                      in ein angenÑhertes Polygon.
|*                      Die Funktion liefert false, wenn ein Fehler bei
|*                      der Koeffizientenberechnung aufgetreten ist oder
|*                      das Polygon zu gro?wird (>PolyMax=16380). Im 1.
|*                      Fall hat das Polygon 0, im 2. Fall PolyMax Punkte.
|*                      Um KoordinatenÅberlÑufe zu vermeiden werden diese
|*                      auf +/-32000 begrenzt.
|*
*************************************************************************/
bool Spline2Poly(Polygon& rSpln, bool Periodic, Polygon& rPoly);

#endif //_SGVSPLN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
