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

#ifndef INCLUDED_SVX_SVDPOEV_HXX
#define INCLUDED_SVX_SVDPOEV_HXX

#include <svx/svxdllapi.h>
#include <svx/svdedtv.hxx>

#include <svx/ipolypolygoneditorcontroller.hxx>

class SVX_DLLPUBLIC SdrPolyEditView: public SdrEditView, public IPolyPolygonEditorController
{
    friend class                SdrEditView;

protected:
    bool                        bSetMarkedPointsSmoothPossible : 1;
    bool                        bSetMarkedSegmentsKindPossible : 1;

    SdrPathSmoothKind           eMarkedPointsSmooth;
    SdrPathSegmentKind          eMarkedSegmentsKind;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    SVX_DLLPRIVATE void ImpResetPolyPossibilityFlags();
    SVX_DLLPRIVATE void ImpCheckPolyPossibilities();

    typedef void (*PPolyTrFunc)(Point&, Point*, Point*, const void*, const void*, const void*, const void*, const void*);
    SVX_DLLPRIVATE void ImpTransformMarkedPoints(PPolyTrFunc pTrFunc, const void* p1=NULL, const void* p2=NULL, const void* p3=NULL, const void* p4=NULL, const void* p5=NULL);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrPolyEditView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrPolyEditView();

public:
    bool IsSetMarkedPointsSmoothPossible() const SAL_OVERRIDE;
    SdrPathSmoothKind GetMarkedPointsSmooth() const SAL_OVERRIDE;
    void SetMarkedPointsSmooth(SdrPathSmoothKind eKind) SAL_OVERRIDE;

    // A PolySegment can either be a segment or a Bézier curve
    bool IsSetMarkedSegmentsKindPossible() const SAL_OVERRIDE;
    SdrPathSegmentKind GetMarkedSegmentsKind() const SAL_OVERRIDE;
    void SetMarkedSegmentsKind(SdrPathSegmentKind eKind) SAL_OVERRIDE;

    // Probably Obj has been deleted afterwards
    void DeleteMarkedPoints() SAL_OVERRIDE;
    bool IsDeleteMarkedPointsPossible() const SAL_OVERRIDE;

    void MoveMarkedPoints(const Size& rSiz);
    void ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    void RotateMarkedPoints(const Point& rRef, long nAngle);

    // This probably creates an abitrary amount of new objects
    void RipUpAtMarkedPoints() SAL_OVERRIDE;
    bool IsRipUpAtMarkedPointsPossible() const SAL_OVERRIDE;

    // All marked Polylines are turned into polygons
    // All open Bézier curves are turned into closed ones
    bool IsOpenCloseMarkedObjectsPossible() const SAL_OVERRIDE;
    SdrObjClosedKind GetMarkedObjectsClosedState() const SAL_OVERRIDE;

    void CheckPolyPossibilitiesHelper( SdrMark* pM, bool& b1stSmooth, bool& b1stSegm, bool& bCurve, bool& bSmoothFuz, bool& bSegmFuz, basegfx::B2VectorContinuity& eSmooth );
};

#endif // INCLUDED_SVX_SVDPOEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
