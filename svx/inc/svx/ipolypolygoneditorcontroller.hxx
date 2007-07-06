/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ipolypolygoneditorcontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 13:16:59 $
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

#ifndef _SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX
#define _SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX

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

class IPolyPolygonEditorController
{
public:
    virtual void DeleteMarkedPoints() = 0;
    virtual BOOL IsDeleteMarkedPointsPossible() const = 0;

    virtual void RipUpAtMarkedPoints() = 0;
    virtual bool IsRipUpAtMarkedPointsPossible() const = 0;

    virtual BOOL IsSetMarkedSegmentsKindPossible() const = 0;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const = 0;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind) = 0;

    virtual BOOL IsSetMarkedPointsSmoothPossible() const = 0;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const = 0;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind) = 0;

    virtual void CloseMarkedObjects(BOOL bToggle, BOOL bOpen ) = 0;
    virtual bool IsOpenCloseMarkedObjectsPossible() const = 0;
    virtual SdrObjClosedKind GetMarkedObjectsClosedState() const = 0;
};

#endif //_SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX

