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


//   Defines

enum class SdrPathSmoothKind
{
    DontCare,   // only for status query
    Angular,    // angular
    Asymmetric, // asymmetric, normal Smooth
    Symmetric   // symmetric
};
enum class SdrPathSegmentKind
{
    DontCare,  // only for status query
    Line,      // straight path segment
    Curve,     // curve path segment (Bezier)
    Toggle     // only for Set: Toggle
};
enum class SdrObjClosedKind
{
    DontCare,    // only for status query
    Open,        // non-closed object (line, polyline, ...)
    Closed       // closed object (polygon, ...)
};

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

    virtual bool IsOpenCloseMarkedObjectsPossible() const = 0;
    virtual SdrObjClosedKind GetMarkedObjectsClosedState() const = 0;

protected:
    ~IPolyPolygonEditorController() {}
};

#endif // INCLUDED_SVX_IPOLYPOLYGONEDITORCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
