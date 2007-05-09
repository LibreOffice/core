/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpoev.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:29:55 $
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

#ifndef _SVDPOEV_HXX
#define _SVDPOEV_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _SVDEDTV_HXX
#include <svx/svdedtv.hxx>
#endif

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

class SVX_DLLPUBLIC SdrPolyEditView: public SdrEditView
{
    friend class                SdrEditView;

protected:
    BOOL                        bSetMarkedPointsSmoothPossible : 1;
    BOOL                        bSetMarkedSegmentsKindPossible : 1;

    SdrPathSmoothKind           eMarkedPointsSmooth;
    SdrPathSegmentKind          eMarkedSegmentsKind;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpResetPolyPossibilityFlags();
    SVX_DLLPRIVATE void ImpCheckPolyPossibilities();

    // Markierte Punkte kopieren und anstelle der alten markieren
    // ist noch nicht implementiert!
    SVX_DLLPRIVATE void ImpCopyMarkedPoints();
    typedef void (*PPolyTrFunc)(Point&, Point*, Point*, const void*, const void*, const void*, const void*, const void*);
    SVX_DLLPRIVATE void ImpTransformMarkedPoints(PPolyTrFunc pTrFunc, const void* p1=NULL, const void* p2=NULL, const void* p3=NULL, const void* p4=NULL, const void* p5=NULL);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrPolyEditView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrPolyEditView();

public:
    BOOL IsSetMarkedPointsSmoothPossible() const { ForcePossibilities(); return bSetMarkedPointsSmoothPossible; }
    SdrPathSmoothKind GetMarkedPointsSmooth() const { ForcePossibilities(); return eMarkedPointsSmooth; }
    void SetMarkedPointsSmooth(SdrPathSmoothKind eKind);

    // Ein PolySegment kann eine Strecke oder eine Bezierkurve sein.
    BOOL IsSetMarkedSegmentsKindPossible() const { ForcePossibilities(); return bSetMarkedSegmentsKindPossible; }
    SdrPathSegmentKind GetMarkedSegmentsKind() const { ForcePossibilities(); return eMarkedSegmentsKind; }
    void SetMarkedSegmentsKind(SdrPathSegmentKind eKind);

    // Moeglicherweise ist das Obj hinterher geloescht:
    void DeleteMarkedPoints();
    BOOL IsDeleteMarkedPointsPossible() const { return HasMarkedPoints(); }

    void MoveMarkedPoints(const Size& rSiz, bool bCopy=false);
    void ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false);
    void RotateMarkedPoints(const Point& rRef, long nWink, bool bCopy=false);

    // Hierbei entstehen eventuell beliebig viele neue Objekte:
    void RipUpAtMarkedPoints();
    bool IsRipUpAtMarkedPointsPossible() const;

    // Alle markierten Polylines werden zu Polygonen, alle offenen
    // Bezierkurven zu geschlossenen.
    void ShutMarkedObjects();
    void CloseMarkedObjects(BOOL bToggle=FALSE, BOOL bOpen=FALSE); // , long nOpenDistance=0);
    bool IsOpenCloseMarkedObjectsPossible() const;
    SdrObjClosedKind GetMarkedObjectsClosedState() const;
};

#endif //_SVDPOEV_HXX

