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

#ifndef INCLUDED_SVX_SVDDRAG_HXX
#define INCLUDED_SVX_SVDDRAG_HXX


#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <svx/svxdllapi.h>

#include <memory>
#include <vector>

// Status information for specialized object dragging. In order for the model
// to stay status free, the status data is kept on the View
// and handed over to the object at the appropriate time as a parameter.
// This also includes the status of the operation and Interactive
// Object creation. In this case, pHdl is null.
class SdrHdl;
class SdrView;
class SdrPageView;
class SdrDragMethod;

struct SdrDragStatUserData
{
    virtual ~SdrDragStatUserData() = 0;
};

class SVXCORE_DLLPUBLIC SdrDragStat final
{
    SdrHdl*  m_pHdl;      // The Handle for the User
    SdrView* m_pView;
    SdrPageView* m_pPageView;
    std::vector<Point> mvPnts; // All previous Points: [0]=Start, [Count()-2]=Prev
    Point     m_aRef1;     // Referencepoint: Resize fixed point, (axis of rotation,
    Point     m_aRef2;     // axis of reflection, ...)
    Point     m_aPos0;     // Position at the last Event
    Point     m_aRealNow;  // Current dragging position without Snap, Ortho and Limit
    tools::Rectangle m_aActionRect;

    bool      m_bEndDragChangesAttributes;
    bool      m_bEndDragChangesGeoAndAttributes;
    /// Table row drag: table will re-layout itself later.
    bool      mbEndDragChangesLayout;
    bool      m_bMouseIsUp;

    bool      m_bShown;    // Xor visible?
    sal_uInt16    m_nMinMov;   // So much has to be minimally moved first
    bool      m_bMinMoved; // MinMove surpassed?

    bool      m_bHorFixed; // Dragging only vertical
    bool      m_bVerFixed; // Dragging only horizontal
    bool      m_bWantNoSnap; // To decide if pObj-> MovCreate () should use NoSnapPos or not.
                          // Therefore, NoSnapPos is written into the buffer.
    bool  m_bOrtho4;
    bool  m_bOrtho8;

    SdrDragMethod* m_pDragMethod;
    std::unique_ptr<SdrDragStatUserData>  mpUserData;     // Userdata

    SAL_DLLPRIVATE void Clear();

    sal_Int32 GetPrevPos() const { return mvPnts.size()-(mvPnts.size()>1 ? 2 : 1); }

    // This is passed all the way through to ApplySpecialDrag of the Edge Object
    // For LOK, we cannot really specify which gluepoint to select by default
    // It selects the nearest gluepoints after DragEnd event.
    // When multiple objects are on top of each other or somehow their gluepoints
    // collide, the gluepoint is selected from the lowest order numbered object
    // We can pass the ord number information inside the draginfo and choose the correct shape
    struct {
        sal_Int32 objectOrdNum = -1;
    } mGlueOptions;

public:
    SdrDragStat()                                    { Reset(); }
    SAL_DLLPRIVATE ~SdrDragStat();
    void         Reset();
    SdrView*     GetView() const                     { return m_pView; }
    void         SetView(SdrView* pV)                { m_pView=pV; }
    SdrPageView* GetPageView() const                 { return m_pPageView; }
    void         SetPageView(SdrPageView* pPV)       { m_pPageView=pPV; }
    const Point& GetPoint(sal_Int32 nNum) const      { return mvPnts[nNum]; }
    sal_Int32    GetPointCount() const               { return mvPnts.size(); }
    const Point& GetStart() const                    { return mvPnts[0]; }
    const Point& GetPrev() const                     { return mvPnts[GetPrevPos()]; }
    const Point& GetPos0() const                     { return m_aPos0;  }
    const Point& GetNow() const                      { return mvPnts.back(); }
    void         SetNow(Point const &pt)             { mvPnts.back() = pt; }
    const Point& GetRef1() const                     { return m_aRef1;  }
    void         SetRef1(const Point &pt)            { m_aRef1 = pt;  }
    const Point& GetRef2() const                     { return m_aRef2;  }
    void         SetRef2(const Point &pt)            { m_aRef2 = pt;  }
    const        SdrHdl* GetHdl() const              { return m_pHdl;   }
    void         SetHdl(SdrHdl* pH)                  { m_pHdl=pH;       }
    SdrDragStatUserData* GetUser() const             { return mpUserData.get();  }
    void         SetUser(std::unique_ptr<SdrDragStatUserData> pU) { mpUserData = std::move(pU); }
    bool         IsShown() const                     { return m_bShown; }
    void         SetShown(bool bOn)                  { m_bShown=bOn; }

    bool         IsMinMoved() const                  { return m_bMinMoved; }
    void         SetMinMoved()                       { m_bMinMoved=true; }
    void         ResetMinMoved()                     { m_bMinMoved=false; }
    void         SetMinMove(sal_uInt16 nDist)        { m_nMinMov=nDist; if (m_nMinMov<1) m_nMinMov=1; }

    bool         IsHorFixed() const                  { return m_bHorFixed; }
    void         SetHorFixed(bool bOn)               { m_bHorFixed=bOn; }
    bool         IsVerFixed() const                  { return m_bVerFixed; }
    void         SetVerFixed(bool bOn)               { m_bVerFixed=bOn; }

    // Here, the object can say: "I do not want to snap to coordinates!"
    // for example, the angle of the arc ...
    bool         IsNoSnap() const                     { return m_bWantNoSnap; }
    void         SetNoSnap(bool bOn = true)           { m_bWantNoSnap=bOn; }

    // And here the Obj say which Ortho (if there is one) can be usefully applied to him.
    // Ortho4 means Ortho in four directions (for Rect and CIRT)
    bool         IsOrtho4Possible() const             { return m_bOrtho4; }
    void         SetOrtho4Possible(bool bOn = true)   { m_bOrtho4=bOn; }
    // Ortho8 means Ortho in 8 directions (for lines)
    bool         IsOrtho8Possible() const             { return m_bOrtho8; }
    void         SetOrtho8Possible(bool bOn = true)   { m_bOrtho8=bOn; }

    // Is set by an object that was dragged.
    bool         IsEndDragChangesAttributes() const    { return m_bEndDragChangesAttributes; }
    void         SetEndDragChangesAttributes(bool bOn) { m_bEndDragChangesAttributes=bOn; }
    bool         IsEndDragChangesGeoAndAttributes() const   { return m_bEndDragChangesGeoAndAttributes; }
    void         SetEndDragChangesGeoAndAttributes(bool bOn) { m_bEndDragChangesGeoAndAttributes=bOn; }
    bool         IsEndDragChangesLayout() const   { return mbEndDragChangesLayout; }
    void         SetEndDragChangesLayout(bool bOn) { mbEndDragChangesLayout=bOn; }

    // Is set by the view and can be evaluated by Obj
    bool         IsMouseDown() const                  { return !m_bMouseIsUp; }
    void         SetMouseDown(bool bDown)         { m_bMouseIsUp=!bDown; }

    SAL_DLLPRIVATE void         Reset(const Point& rPnt);
    void         NextMove(const Point& rPnt);
    SAL_DLLPRIVATE void         NextPoint();
    SAL_DLLPRIVATE void         PrevPoint();
    bool         CheckMinMoved(const Point& rPnt);
    tools::Long         GetDX() const                     { return GetNow().X()-GetPrev().X(); }
    tools::Long         GetDY() const                     { return GetNow().Y()-GetPrev().Y(); }
    Fraction     GetXFact() const;
    Fraction     GetYFact() const;

    SdrDragMethod* GetDragMethod() const             { return m_pDragMethod; }
    void         SetDragMethod(SdrDragMethod* pMth)  { m_pDragMethod=pMth; }

    const tools::Rectangle& GetActionRect() const          { return m_aActionRect; }
    void         SetActionRect(const tools::Rectangle& rR) { m_aActionRect=rR; }

    // Also considering 1stPointAsCenter
    SAL_DLLPRIVATE void         TakeCreateRect(tools::Rectangle& rRect) const;

    auto&        GetGlueOptions() { return mGlueOptions; }
};

#endif // INCLUDED_SVX_SVDDRAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
