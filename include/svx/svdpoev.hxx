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

#ifndef _SVDPOEV_HXX
#define _SVDPOEV_HXX

#include "svx/svxdllapi.h"
#include <svx/svdedtv.hxx>

#include "svx/ipolypolygoneditorcontroller.hxx"

class SVX_DLLPUBLIC SdrPolyEditView: public SdrEditView, public IPolyPolygonEditorController
{
    friend class                SdrEditView;

protected:
    sal_Bool                        bSetMarkedPointsSmoothPossible : 1;
    sal_Bool                        bSetMarkedSegmentsKindPossible : 1;

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
    sal_Bool IsSetMarkedPointsSmoothPossible() const;
    SdrPathSmoothKind GetMarkedPointsSmooth() const;
    void SetMarkedPointsSmooth(SdrPathSmoothKind eKind);

    // Ein PolySegment kann eine Strecke oder eine Bezierkurve sein.
    sal_Bool IsSetMarkedSegmentsKindPossible() const;
    SdrPathSegmentKind GetMarkedSegmentsKind() const;
    void SetMarkedSegmentsKind(SdrPathSegmentKind eKind);

    // Moeglicherweise ist das Obj hinterher geloescht:
    void DeleteMarkedPoints();
    sal_Bool IsDeleteMarkedPointsPossible() const;

    void MoveMarkedPoints(const Size& rSiz, bool bCopy=false);
    void ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false);
    void RotateMarkedPoints(const Point& rRef, long nWink, bool bCopy=false);

    // Hierbei entstehen eventuell beliebig viele neue Objekte:
    void RipUpAtMarkedPoints();
    bool IsRipUpAtMarkedPointsPossible() const;

    // Alle markierten Polylines werden zu Polygonen, alle offenen
    // Bezierkurven zu geschlossenen.
    void CloseMarkedObjects(sal_Bool bToggle=sal_False, sal_Bool bOpen=sal_False); // , long nOpenDistance=0);
    bool IsOpenCloseMarkedObjectsPossible() const;
    SdrObjClosedKind GetMarkedObjectsClosedState() const;

    void CheckPolyPossibilitiesHelper( SdrMark* pM, bool& b1stSmooth, bool& b1stSegm, bool& bCurve, bool& bSmoothFuz, bool& bSegmFuz, basegfx::B2VectorContinuity& eSmooth );
};

#endif //_SVDPOEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
