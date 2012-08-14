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

#ifndef _SVDDRAG_HXX
#define _SVDDRAG_HXX


#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include "svx/svxdllapi.h"

// Status information for specialized object dragging. In order for the model
// to stay status free, the status data is kept on the View
// and handed over to the object at the appropriate time as a parameter.
// This also includes the status of the operation and Interactive
// Object creation. In this case, pHdl is null.
class SdrHdl;
class SdrView;
class SdrPageView;
class SdrDragMethod;

struct SVX_DLLPUBLIC SdrDragStatUserData
{
};

class SVX_DLLPUBLIC SdrDragStat {
protected:
    SdrHdl*  pHdl;      // Der Handle an dem der User zottelt
    SdrView* pView;
    SdrPageView* pPageView;
    std::vector<Point*> aPnts;    // Alle bisherigen Punkte: [0]=Start, [Count()-2]=Prev
    Point     aRef1;     // Referenzpunkt: Resize-Fixpunkt, (Drehachse,
    Point     aRef2;     // Spiegelachse, ...)
    Point     aPos0;     // Position beim letzten Event
    Point     aRealPos0; // Position beim letzten Event
    Point     aRealNow;  // Aktuelle Dragposition ohne Snap, Ortho und Limit
    Point     aRealLast; // RealPos des letzten Punkts (fuer MinMoved)
    Rectangle aActionRect;

    // Reserve fuer kompatible Erweiterungen, die sonst inkompatibel wuerden.
    Point     aReservePoint1;
    Point     aReservePoint2;
    Point     aReservePoint3;
    Point     aReservePoint4;
    Rectangle aReserveRect1;
    Rectangle aReserveRect2;
    bool      bEndDragChangesAttributes;
    bool      bEndDragChangesGeoAndAttributes;
    bool      bMouseIsUp;
    bool      aReserveBool3;
    bool      aReserveBool4;
    long      aReserveLong1;
    long      aReserveLong2;
    long      aReserveLong3;
    long      aReserveLong4;
    void*     aReservePtr1;
    void*     aReservePtr2;
    void*     aReservePtr3;
    void*     aReservePtr4;

    bool      bShown;    // Xor visible?
    sal_uInt16    nMinMov;   // So much has to be minimally moved first
    bool      bMinMoved; // MinMove surpassed?

    bool      bHorFixed; // Dragging only vertical
    bool      bVerFixed; // Dragging only horizontal
    bool      bWantNoSnap; // To decide if pObj-> MovCreate () should use NoSnapPos or not.
                          // Therefore, NoSnapPos is written into the buffer.
    bool  bOrtho4;
    bool  bOrtho8;

    SdrDragMethod* pDragMethod;

protected:
    void Clear(bool bLeaveOne);
    Point& Pnt(sal_uIntPtr nNum)                           { return *aPnts[nNum]; }
//public:
    SdrDragStatUserData*    pUser;     // Userdata
public:
    SdrDragStat(): aPnts()                           { pUser=NULL; Reset(); }
    ~SdrDragStat()                                   { Clear(sal_False); }
    void         Reset();
    SdrView*     GetView() const                     { return pView; }
    void         SetView(SdrView* pV)                { pView=pV; }
    SdrPageView* GetPageView() const                 { return pPageView; }
    void         SetPageView(SdrPageView* pPV)       { pPageView=pPV; }
    const Point& GetPoint(sal_uIntPtr nNum) const    { return *aPnts[nNum]; }
    sal_uIntPtr        GetPointAnz() const           { return aPnts.size(); }
    const Point& GetStart() const                    { return GetPoint(0); }
    Point&       Start()                             { return Pnt(0); }
    const Point& GetPrev() const                     { return GetPoint(GetPointAnz()-(GetPointAnz()>=2 ? 2:1)); }
    Point& Prev()                                    { return Pnt(GetPointAnz()-(GetPointAnz()>=2 ? 2:1)); }
    const Point& GetPos0() const                     { return aPos0;  }
    Point&       Pos0()                              { return aPos0;  }
    const Point& GetNow() const                      { return GetPoint(GetPointAnz()-1); }
    Point&       Now()                               { return Pnt(GetPointAnz()-1); }
    const Point& GetRealNow() const                  { return aRealNow; }
    Point&       RealNow()                           { return aRealNow; }
    const Point& GetRef1() const                     { return aRef1;  }
    Point&       Ref1()                              { return aRef1;  }
    const Point& GetRef2() const                     { return aRef2;  }
    Point&       Ref2()                              { return aRef2;  }
    const        SdrHdl* GetHdl() const              { return pHdl;   }
    void         SetHdl(SdrHdl* pH)                  { pHdl=pH;       }
    SdrDragStatUserData* GetUser() const             { return pUser;  }
    void SetUser(SdrDragStatUserData* pU)            { pUser=pU; }
    bool         IsShown() const                     { return bShown; }
    void         SetShown(bool bOn)                  { bShown=bOn; }

    bool         IsMinMoved() const                  { return bMinMoved; }
    void         SetMinMoved()                       { bMinMoved=sal_True; }
    void         ResetMinMoved()                     { bMinMoved=sal_False; }
    void         SetMinMove(sal_uInt16 nDist)            { nMinMov=nDist; if (nMinMov<1) nMinMov=1; }
    sal_uInt16       GetMinMove() const                  { return nMinMov; }

    bool         IsHorFixed() const                  { return bHorFixed; }
    void         SetHorFixed(bool bOn)               { bHorFixed=bOn; }
    bool         IsVerFixed() const                  { return bVerFixed; }
    void         SetVerFixed(bool bOn)               { bVerFixed=bOn; }

    // Here, the object can say: "I do not want to snap to coordinates!"
    // for example, the angle of the arc ...
    bool         IsNoSnap() const                     { return bWantNoSnap; }
    void         SetNoSnap(bool bOn = true)           { bWantNoSnap=bOn; }

    // And here the Obj say which Ortho (if there is one)
    // can be usefully applied to him.
    // Ortho4 means Ortho in four directions (for Rect and CIRT)
    bool         IsOrtho4Possible() const             { return bOrtho4; }
    void         SetOrtho4Possible(bool bOn = true)   { bOrtho4=bOn; }
    // Ortho8 means Ortho in 8 directions (for lines)
    bool         IsOrtho8Possible() const             { return bOrtho8; }
    void         SetOrtho8Possible(bool bOn = true)   { bOrtho8=bOn; }

    // Is set by an object that was dragged.
    bool         IsEndDragChangesAttributes() const    { return bEndDragChangesAttributes; }
    void         SetEndDragChangesAttributes(bool bOn) { bEndDragChangesAttributes=bOn; }
    bool         IsEndDragChangesGeoAndAttributes() const   { return bEndDragChangesGeoAndAttributes; }
    void         SetEndDragChangesGeoAndAttributes(bool bOn) { bEndDragChangesGeoAndAttributes=bOn; }

    // Is set by the view and can be evaluated by Obj
    bool         IsMouseDown() const                  { return !bMouseIsUp; }
    void         SetMouseDown(bool bDown)         { bMouseIsUp=!bDown; }

    Point KorregPos(const Point& rNow, const Point& rPrev) const;
    void  Reset(const Point& rPnt);
    void  NextMove(const Point& rPnt);
    void  NextPoint(bool bSaveReal=sal_False);
    void  PrevPoint();
    bool CheckMinMoved(const Point& rPnt);
    long  GetDX() const                     { return GetNow().X()-GetPrev().X(); }
    long  GetDY() const                     { return GetNow().Y()-GetPrev().Y(); }
    Fraction GetXFact() const;
    Fraction GetYFact() const;

    SdrDragMethod* GetDragMethod() const               { return pDragMethod; }
    void           SetDragMethod(SdrDragMethod* pMth)  { pDragMethod=pMth; }

    const Rectangle& GetActionRect() const             { return aActionRect; }
    void             SetActionRect(const Rectangle& rR) { aActionRect=rR; }

    // also considering 1stPointAsCenter
    void TakeCreateRect(Rectangle& rRect) const;
};

#endif //_SVDDRAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
