/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX
#define _SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX

#include "svx/svxdllapi.h"
#include <svx/svdedtv.hxx>

//************************************************************
//   Defines
//************************************************************

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
};

#endif //_SDR_IPOLYPOLYGONEDITORCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
