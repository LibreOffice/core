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

#ifndef INCLUDED_SVX_IPOLYPOLYGONEDITORCONTROLLER_HXX
#define INCLUDED_SVX_IPOLYPOLYGONEDITORCONTROLLER_HXX

#include <svx/svxdllapi.h>
#include <svx/svdedtv.hxx>


//   Defines


enum SdrPathSmoothKind  {SDRPATHSMOOTH_DONTCARE,   // only for status query
                         SDRPATHSMOOTH_ANGULAR,    // angular
                         SDRPATHSMOOTH_ASYMMETRIC, // asymmetric, normal Smooth
                         SDRPATHSMOOTH_SYMMETRIC}; // symmetric

enum SdrPathSegmentKind {SDRPATHSEGMENT_DONTCARE,  // only for status query
                         SDRPATHSEGMENT_LINE,      // straight path segment
                         SDRPATHSEGMENT_CURVE,     // curve path segment (Bezier)
                         SDRPATHSEGMENT_TOGGLE};   // only for Set: Toggle

enum SdrObjClosedKind   {SDROBJCLOSED_DONTCARE,    // only for status query
                         SDROBJCLOSED_OPEN,        // non-closed object (line, polyline, ...)
                         SDROBJCLOSED_CLOSED,      // closed object (polygon, ...)
                         SDROBJCLOSED_TOGGLE};     // only for Set: Toggle (not implemented yet)

class IPolyPolygonEditorController
{
public:
    virtual void DeleteMarkedPoints() = 0;
    virtual sal_Bool IsDeleteMarkedPointsPossible() const = 0;

    virtual void RipUpAtMarkedPoints() = 0;
    virtual bool IsRipUpAtMarkedPointsPossible() const = 0;

    virtual sal_Bool IsSetMarkedSegmentsKindPossible() const = 0;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const = 0;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind) = 0;

    virtual sal_Bool IsSetMarkedPointsSmoothPossible() const = 0;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const = 0;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind) = 0;

    virtual void CloseMarkedObjects(sal_Bool bToggle, sal_Bool bOpen ) = 0;
    virtual bool IsOpenCloseMarkedObjectsPossible() const = 0;
    virtual SdrObjClosedKind GetMarkedObjectsClosedState() const = 0;

protected:
    ~IPolyPolygonEditorController() {}
};

#endif // INCLUDED_SVX_IPOLYPOLYGONEDITORCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
