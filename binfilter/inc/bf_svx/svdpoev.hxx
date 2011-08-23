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

#ifndef _SVDPOEV_HXX
#define _SVDPOEV_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#ifndef _SVDHDL_HXX
#include <bf_svx/svdhdl.hxx>
#endif

#include <bf_svx/svdedtv.hxx>
namespace binfilter {

//************************************************************
//   Defines
//************************************************************

enum SdrPathSmoothKind  {SDRPATHSMOOTH_DONTCARE,   // nur fuer Statusabfrage
                         SDRPATHSMOOTH_ANGULAR,    // Eckig
                         SDRPATHSMOOTH_ASYMMETRIC, // unsymmetrisch, normales Smooth
                         SDRPATHSMOOTH_SYMMETRIC}; // symmetrisch

enum SdrPathSegmentKind {SDRPATHSEGMENT_DONTCARE,  // nur fuer Statusabfrage
                         SDRPATHSEGMENT_LINE,      // gerader Streckenabschnitt
                         SDRPATHSEGMENT_CURVE,     // Kurvenabschnitt (Bezier)
                         SDRPATHSEGMENT_TOGGLE};   // nur fuer Set: Toggle

enum SdrObjClosedKind   {SDROBJCLOSED_DONTCARE,    // nur fuer Statusabfrage
                         SDROBJCLOSED_OPEN,        // Objekte geoeffnet (Linie, Polyline, ...)
                         SDROBJCLOSED_CLOSED,      // Objekte geschlossen (Polygon, ...)
                         SDROBJCLOSED_TOGGLE};     // nur fuer Set: Toggle (not implemented yet)

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// @@@@@   @@@@  @@  @@  @@  @@@@@ @@@@@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
// @@  @@ @@  @@ @@  @@  @@  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@   @@
// @@  @@ @@  @@ @@  @@  @@  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@ @ @@
// @@@@@  @@  @@ @@   @@@@   @@@@  @@  @@ @@   @@    @@@@@ @@ @@@@  @@@@@@@
// @@     @@  @@ @@    @@    @@    @@  @@ @@   @@     @@@  @@ @@    @@@@@@@
// @@     @@  @@ @@    @@    @@    @@  @@ @@   @@     @@@  @@ @@    @@@ @@@
// @@      @@@@  @@@@@ @@    @@@@@ @@@@@  @@   @@      @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPolyEditView: public SdrEditView
{
    friend class				SdrEditView;

protected:
    BOOL						bSetMarkedPointsSmoothPossible : 1;
    BOOL						bSetMarkedSegmentsKindPossible : 1;

    SdrPathSmoothKind			eMarkedPointsSmooth;
    SdrPathSegmentKind			eMarkedSegmentsKind;

private:
    void ImpClearVars();
    void ImpResetPolyPossibilityFlags();
    void ImpCheckPolyPossibilities();

    // Markierte Punkte kopieren und anstelle der alten markieren
    // ist noch nicht implementiert!
    typedef void (*PPolyTrFunc)(Point&, Point*, Point*, const void*, const void*, const void*, const void*, const void*);

public:
    SdrPolyEditView(SdrModel* pModel1, OutputDevice* pOut=NULL);
    virtual ~SdrPolyEditView();

    BOOL IsSetMarkedPointsSmoothPossible() const { ForcePossibilities(); return bSetMarkedPointsSmoothPossible; }
    SdrPathSmoothKind GetMarkedPointsSmooth() const { ForcePossibilities(); return eMarkedPointsSmooth; }

    // Ein PolySegment kann eine Strecke oder eine Bezierkurve sein.
    BOOL IsSetMarkedSegmentsKindPossible() const { ForcePossibilities(); return bSetMarkedSegmentsKindPossible; }
    SdrPathSegmentKind GetMarkedSegmentsKind() const { ForcePossibilities(); return eMarkedSegmentsKind; }

    // Moeglicherweise ist das Obj hinterher geloescht:
    BOOL IsDeleteMarkedPointsPossible() const { return HasMarkedPoints(); }


    // Hierbei entstehen eventuell beliebig viele neue Objekte:

    // Alle markierten Polylines werden zu Polygonen, alle offenen
    // Bezierkurven zu geschlossenen.
};

}//end of namespace binfilter
#endif //_SVDPOEV_HXX

