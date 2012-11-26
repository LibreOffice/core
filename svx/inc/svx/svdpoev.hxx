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



#ifndef _SVDPOEV_HXX
#define _SVDPOEV_HXX

#include "svx/svxdllapi.h"
#include <svx/svdedtv.hxx>
#include "svx/ipolypolygoneditorcontroller.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPolyEditView: public SdrEditView, public IPolyPolygonEditorController
{
protected:
    /// bitfield
    bool                        mbSetMarkedPointsSmoothPossible : 1;
    bool                        mbSetMarkedSegmentsKindPossible : 1;

    SdrPathSmoothKind           meMarkedPointsSmooth;
    SdrPathSegmentKind          meMarkedSegmentsKind;

    virtual void ImpCheckPolyPossibilities();

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrPolyEditView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrPolyEditView();

public:
    void CheckPolyPossibilitiesHelper(
        const SdrPathObj& rMarkedObject,
        const sdr::selection::Indices& rMarkedPoints,
        bool& b1stSmooth, bool& b1stSegm, bool& bCurve, bool& bSmoothFuz,
        bool& bSegmFuz, basegfx::B2VectorContinuity& eSmooth);

    bool IsSetMarkedPointsSmoothPossible() const;
    SdrPathSmoothKind GetMarkedPointsSmooth() const;
    void SetMarkedPointsSmooth(SdrPathSmoothKind eKind);

    // Ein PolySegment kann eine Strecke oder eine Bezierkurve sein.
    bool IsSetMarkedSegmentsKindPossible() const;
    SdrPathSegmentKind GetMarkedSegmentsKind() const;
    void SetMarkedSegmentsKind(SdrPathSegmentKind eKind);

    // Moeglicherweise ist das Obj hinterher geloescht:
    void DeleteMarkedPoints();
    bool IsDeleteMarkedPointsPossible() const;

    // central selected points manipulator
    void TransformMarkedPoints(const basegfx::B2DHomMatrix& rTransformation, const SdrRepeatFunc aRepFunc, bool bCopy = false);

    // Hierbei entstehen eventuell beliebig viele neue Objekte:
    void RipUpAtMarkedPoints();
    bool IsRipUpAtMarkedPointsPossible() const;

    // Alle markierten Polylines werden zu Polygonen, alle offenen
    // Bezierkurven zu geschlossenen.
    void CloseMarkedObjects(bool bToggle = false, bool bOpen = false);
    bool IsOpenCloseMarkedObjectsPossible() const;
    SdrObjClosedKind GetMarkedObjectsClosedState() const;
};

#endif //_SVDPOEV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
