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

class SVX_DLLPUBLIC SdrPolyEditView : public SdrEditView, public IPolyPolygonEditorController
{
    friend class                SdrEditView;

    bool                        bSetMarkedPointsSmoothPossible : 1;
    bool                        bSetMarkedSegmentsKindPossible : 1;

    SdrPathSmoothKind           eMarkedPointsSmooth;
    SdrPathSegmentKind          eMarkedSegmentsKind;

private:
    SVX_DLLPRIVATE void ImpResetPolyPossibilityFlags();
    SVX_DLLPRIVATE void ImpCheckPolyPossibilities();

    typedef void (*PPolyTrFunc)(Point&, Point*, Point*, const void*, const void*, const void*, const void*);
    SVX_DLLPRIVATE void ImpTransformMarkedPoints(PPolyTrFunc pTrFunc, const void* p1, const void* p2=nullptr, const void* p3=nullptr, const void* p4=nullptr);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrPolyEditView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~SdrPolyEditView() override;

public:
    bool IsSetMarkedPointsSmoothPossible() const override;
    SdrPathSmoothKind GetMarkedPointsSmooth() const override;
    void SetMarkedPointsSmooth(SdrPathSmoothKind eKind) override;

    // A PolySegment can either be a segment or a Bézier curve
    bool IsSetMarkedSegmentsKindPossible() const override;
    SdrPathSegmentKind GetMarkedSegmentsKind() const override;
    void SetMarkedSegmentsKind(SdrPathSegmentKind eKind) override;

    // Probably Obj has been deleted afterwards
    void DeleteMarkedPoints() override;
    bool IsDeleteMarkedPointsPossible() const override;

    void MoveMarkedPoints(const Size& rSiz);
    void ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    void RotateMarkedPoints(const Point& rRef, long nAngle);

    // This probably creates an abitrary amount of new objects
    void RipUpAtMarkedPoints() override;
    bool IsRipUpAtMarkedPointsPossible() const override;

    // All marked Polylines are turned into polygons
    // All open Bézier curves are turned into closed ones
    bool IsOpenCloseMarkedObjectsPossible() const override;
    SdrObjClosedKind GetMarkedObjectsClosedState() const override;

    void CheckPolyPossibilitiesHelper( SdrMark* pM, bool& b1stSmooth, bool& b1stSegm, bool& bCurve, bool& bSmoothFuz, bool& bSegmFuz, basegfx::B2VectorContinuity& eSmooth );
};

#endif // INCLUDED_SVX_SVDPOEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
