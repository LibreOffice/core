/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX
#define _SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX

#include "svx/svxdllapi.h"
#include <svx/svdedtv.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Defines

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

class IPolyPolygonEditorController
{
public:
    virtual void DeleteMarkedPoints() = 0;
    virtual bool IsDeleteMarkedPointsPossible() const = 0;

    virtual void RipUpAtMarkedPoints() = 0;
    virtual bool IsRipUpAtMarkedPointsPossible() const = 0;

    virtual bool IsSetMarkedSegmentsKindPossible() const = 0;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const = 0;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind) = 0;

    virtual bool IsSetMarkedPointsSmoothPossible() const = 0;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const = 0;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind) = 0;

    virtual void CloseMarkedObjects(bool bToggle, bool bOpen ) = 0;
    virtual bool IsOpenCloseMarkedObjectsPossible() const = 0;
    virtual SdrObjClosedKind GetMarkedObjectsClosedState() const = 0;
};

#endif //_SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
