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



#ifndef _SVDDRAG_HXX
#define _SVDDRAG_HXX

#include <sal/types.h>
#include "svx/svxdllapi.h"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <boost/utility.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrHdl;
class SdrView;
//class SdrPaintView;
//class SdrPageView;
class SdrDragMethod;

////////////////////////////////////////////////////////////////////////////////////////////////////

struct SVX_DLLPUBLIC SdrDragStatUserData {};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrDragStat : private boost::noncopyable
{
private:
    typedef ::std::vector< basegfx::B2DPoint > B2DPointVector;

    SdrHdl*                 mpHdl;      // Der Handle an dem der User zottelt
    SdrView&                mrSdrView;
//  SdrPageView*            mpPageView;
    SdrDragMethod*          mpDragMethod;
    SdrDragStatUserData*    mpUser;     // Userdata

    B2DPointVector          maPnts;    // Alle bisherigen Punkte: [0]=Start, [Count()-2]=Prev
    basegfx::B2DPoint       maRef1;     // Referenzpunkt: Resize-Fixpunkt, (Drehachse,
    basegfx::B2DPoint       maRef2;     // Spiegelachse, ...)
    basegfx::B2DPoint       maPos0;     // Position beim letzten Event
    basegfx::B2DPoint       maRealPos0; // Position beim letzten Event
    basegfx::B2DPoint       maRealNow;  // Aktuelle Dragposition ohne Snap, Ortho und Limit
    basegfx::B2DRange       maActionRange;
    double                  mfMinMov;   // Soviel muss erstmal minimal bewegt werden

    /// bitfield
    bool                    mbEndDragChangesAttributes : 1;
    bool                    mbEndDragChangesGeoAndAttributes : 1;
    bool                    mbMouseIsUp : 1;
    bool                    mbShown : 1;    // Xor sichrbar?
    bool                    mbMinMoved : 1; // MinMove durchbrochen?
    bool                    mbHorFixed : 1; // nur Vertikal draggen
    bool                    mbVerFixed : 1; // nur Horizontal draggen

    // true=Fuer die Entscheidung ob fuer pObj->MovCreate() NoSnapPos verwendet
                          // werden soll. Entsprechend wird auch NoSnapPos in den Buffer geschrieben.
    bool                    mbWantNoSnap : 1;

    bool                    mbOrtho4 : 1;
    bool                    mbOrtho8 : 1;

public:
    SdrDragStat(SdrView& rSdrView);
    ~SdrDragStat();

    void         Reset();
    void Reset(const basegfx::B2DPoint& rPnt);

    SdrView& GetSdrViewFromSdrDragStat() const { return mrSdrView; }

//  SdrPageView* GetPageView() const { return mpPageView; }
//  void SetPageView(SdrPageView* pPV) { if(mpPageView != pPV) mpPageView = pPV; }

    sal_uInt32 GetPointAnz() const { return maPnts.size(); }
    const basegfx::B2DPoint& GetPoint(sal_uInt32 nNum) const;

    const basegfx::B2DPoint& GetStart() const;
    void SetStart(const basegfx::B2DPoint& rNew);

    const basegfx::B2DPoint& GetPrev() const;
    void SetPrev(const basegfx::B2DPoint& rNew);

    const basegfx::B2DPoint& GetPos0() const { return maPos0;  }
    void SetPos0(const basegfx::B2DPoint& rNew) { if(maPos0 != rNew) maPos0 = rNew;  }

    const basegfx::B2DPoint& GetNow() const;
    void SetNow(const basegfx::B2DPoint& rNew);

    const basegfx::B2DPoint& GetRealNow() const { return maRealNow; }
    void SetRealNow(const basegfx::B2DPoint& rNew) { if(maRealNow != rNew) maRealNow = rNew; }

    const basegfx::B2DPoint& GetRef1() const { return maRef1;  }
    void SetRef1(const basegfx::B2DPoint& rNew) { if(maRef1 != rNew) maRef1 = rNew;  }

    const basegfx::B2DPoint& GetRef2() const { return maRef2;  }
    void SetRef2(const basegfx::B2DPoint& rNew) { if(maRef2 != rNew) maRef2 = rNew;  }

    const SdrHdl* GetActiveHdl() const { return mpHdl;   }
    void SetActiveHdl(SdrHdl* pH) { if(mpHdl != pH) mpHdl = pH; }

    SdrDragStatUserData* GetUser() const { return mpUser;  }
    void SetUser(SdrDragStatUserData* pU) { if(mpUser != pU) mpUser = pU; }

    bool IsShown() const { return mbShown; }
    void SetShown(bool bOn) { if(mbShown != bOn) mbShown = bOn; }

    bool IsMinMoved() const { return mbMinMoved; }
    void SetMinMoved() { mbMinMoved = true; }
    void ResetMinMoved() { mbMinMoved = false; }
    void SetMinMove(double fDist) { mfMinMov = std::min(fDist, 1.0); }
    double GetMinMove() const { return mfMinMov; }

    bool IsHorFixed() const { return mbHorFixed; }
    void SetHorFixed(bool bOn) { if(mbHorFixed != bOn) mbHorFixed = bOn; }
    bool IsVerFixed() const { return mbVerFixed; }
    void SetVerFixed(bool bOn) { if(mbVerFixed != bOn) mbVerFixed = bOn; }

    // Hier kann das Obj sagen: "Ich will keinen Koordinatenfang!"
    // z.B. fuer den Winkel des Kreisbogen...
    bool IsNoSnap() const { return mbWantNoSnap; }
    void SetNoSnap(bool bOn = true) { if(mbWantNoSnap != bOn) mbWantNoSnap = bOn; }

    // Ortho4 bedeutet Ortho in 4 Richtungen (fuer Rect und Cirt)
    bool IsOrtho4Possible() const { return mbOrtho4; }
    void SetOrtho4Possible(bool bOn = true) { if(mbOrtho4 != bOn) mbOrtho4 = bOn; }

    // Ortho8 bedeutet Ortho in 8 Richtungen (fuer Linien)
    bool IsOrtho8Possible() const { return mbOrtho8; }
    void SetOrtho8Possible(bool bOn = true) { if(mbOrtho8 != bOn) mbOrtho8 = bOn; }

    // Wird vom gedraggten Objekt gesetzt
    bool IsEndDragChangesAttributes() const { return mbEndDragChangesAttributes; }
    void SetEndDragChangesAttributes(bool bOn) { if(mbEndDragChangesAttributes != bOn) mbEndDragChangesAttributes = bOn; }
    bool IsEndDragChangesGeoAndAttributes() const { return mbEndDragChangesGeoAndAttributes; }
    void SetEndDragChangesGeoAndAttributes(bool bOn) { if(mbEndDragChangesGeoAndAttributes != bOn) mbEndDragChangesGeoAndAttributes = bOn; }

    // Wird von der View gesetzt und kann vom Obj ausgewertet werden
    bool IsMouseDown() const { return !mbMouseIsUp; }
    void SetMouseDown(bool bDown) { if(mbMouseIsUp == bDown) mbMouseIsUp = !bDown; }

    basegfx::B2DPoint KorregPos(const basegfx::B2DPoint & rNow, const basegfx::B2DPoint & rPrev) const;
    void NextMove(const basegfx::B2DPoint& rPnt);
    void NextPoint(bool bSaveReal = false);
    void  PrevPoint();
    bool CheckMinMoved(const basegfx::B2DPoint& rPnt);
    double GetDX() const { return GetNow().getX() - GetPrev().getX(); }
    double GetDY() const { return GetNow().getY() - GetPrev().getY(); }
    double GetXFact() const;
    double GetYFact() const;

    SdrDragMethod* GetDragMethod() const { return mpDragMethod; }
    void SetDragMethod(SdrDragMethod* pMth) { if(mpDragMethod != pMth) mpDragMethod = pMth; }

    const basegfx::B2DRange& GetActionRange() const { return maActionRange; }
    void SetActionRange(const basegfx::B2DRange& rR) { if(maActionRange != rR) maActionRange = rR; }

    // Unter Beruecksichtigung von 1stPointAsCenter
    basegfx::B2DRange TakeCreateRange() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDDRAG_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
