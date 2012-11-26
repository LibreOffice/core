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



#ifndef _SVDGLEV_HXX
#define _SVDGLEV_HXX

#include "svx/svxdllapi.h"
#include <svx/svdpoev.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrGluePoint;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrGlueEditView: public SdrPolyEditView
{
private:
    // Markierte Klebepunkte kopieren und anstelle der alten markieren
    void ImpCopyMarkedGluePoints();
    typedef void (*PGlueDoFunc)(SdrGluePoint&, const SdrObject* pObj, const void*, const void*, const void*, const void*, const void*);
    typedef void (*PGlueTrFunc)(basegfx::B2DPoint&, const void*, const void*, const void*, const void*, const void*);
    void ImpDoMarkedGluePoints(PGlueDoFunc pDoFunc, bool bConst, const void* p1 = 0, const void* p2 = 0, const void* p3 = 0, const void* p4 = 0, const void* p5 = 0);
    void ImpTransformMarkedGluePoints(PGlueTrFunc pTrFunc, const void* p1 = 0, const void* p2 = 0, const void* p3 = 0, const void* p4 = 0, const void* p5 = 0);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrGlueEditView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrGlueEditView();

public:
    // Durch den Parameter nThisEsc uebergibt man die Richtung, die man
    // checken bzw. setzen/loeschen will.
    // Moegliche Werte fuer nThisEsc sind z.Zt.
    // SDRESC_LEFT, SDRESC_RIGHT, SDRESC_TOP und SDRESC_BOTTOM
    TRISTATE IsMarkedGluePointsEscDir(sal_uInt16 nThisEsc) const;
    void SetMarkedGluePointsEscDir(sal_uInt16 nThisEsc, bool bOn);
    bool IsSetMarkedGluePointsEscDirPossible() const { return !IsReadOnly() && areGluesSelected(); }

    // Checken/setzen, ob die Klebepunktpositionen relativ zur
    // Objektgroesse sind (Percent=true) oder nicht (Percent=false)
    TRISTATE IsMarkedGluePointsPercent() const;
    void SetMarkedGluePointsPercent(bool bOn);
    bool IsSetMarkedGluePointsPercentPossible() const { return !IsReadOnly() && areGluesSelected(); }

    // bVert=false: Horizontales Alignment checken/setzen
    //      SDRHORZALIGN_CENTER
    //      SDRHORZALIGN_LEFT
    //      SDRHORZALIGN_RIGHT
    //      SDRHORZALIGN_DONTCARE (nur bei Get())
    // bVert=true: Vertikales Alignment checken/setzen
    //      SDRVERTALIGN_CENTER
    //      SDRVERTALIGN_TOP
    //      SDRVERTALIGN_BOTTOM
    //      SDRVERTALIGN_DONTCARE (nur bei Get())
    sal_uInt16 GetMarkedGluePointsAlign(bool bVert) const;
    void SetMarkedGluePointsAlign(bool bVert, sal_uInt16 nAlign);
    bool IsSetMarkedGluePointsAlignPossible() const { return !IsReadOnly() && areGluesSelected(); }

    // Alle merkierten Klebepunkte entfernen
    void DeleteMarkedGluePoints();

    void MoveMarkedGluePoints(const basegfx::B2DVector& rDelta, bool bCopy = false);
    void ResizeMarkedGluePoints(const basegfx::B2DPoint& rRef, const basegfx::B2DVector& rScale, bool bCopy = false);
    void RotateMarkedGluePoints(const basegfx::B2DPoint& rRef, double fAngle, bool bCopy = false);
};

#endif //_SVDGLEV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
