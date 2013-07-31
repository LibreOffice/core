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
    SdrHdl*  pHdl;      // The Handle for the User
    SdrView* pView;
    SdrPageView* pPageView;
    std::vector<Point*> aPnts; // All previous Points: [0]=Start, [Count()-2]=Prev
    Point     aRef1;     // Referencepoint: Resize fixed point, (axis of rotation,
    Point     aRef2;     // axis of reflection, ...)
    Point     aPos0;     // Position at the last Event
    Point     aRealPos0; // Position at the last Event
    Point     aRealNow;  // Current dragging position without Snap, Ortho and Limit
    Point     aRealLast; // RealPos of the last Point (for MinMoved)
    Rectangle aActionRect;

    // Backup for compatible extensions which otherwise would become incompatible
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

    // And here the Obj say which Ortho (if there is one) can be usefully applied to him.
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

    // Also considering 1stPointAsCenter
    void TakeCreateRect(Rectangle& rRect) const;
};

#endif //_SVDDRAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
