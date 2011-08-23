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

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

namespace binfilter {
// Statushalter fuer objektspeziefisches Draggen. Damit das Model
// Statusfrei bleibt werden die Statusdaten an der View gehalten
// und dem Objekt zu gegebener Zeit als Parameter uebergeben.
// Ausserdem auch Statushalter fuer den Vorgang der Interaktiven
// Objekterzeugung. pHdl ist in diesem Fall NULL.
class SdrHdl;
class SdrView;
class SdrPageView;
class SdrDragMethod;
class SdrDragStat {
protected:
    SdrHdl*  pHdl;      // Der Handle an dem der User zottelt
    SdrView* pView;
    SdrPageView* pPageView;
    Container aPnts;    // Alle bisherigen Punkte: [0]=Start, [Count()-2]=Prev
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
    FASTBOOL  bEndDragChangesAttributes;
    FASTBOOL  bEndDragChangesGeoAndAttributes;
    FASTBOOL  bMouseIsUp;
    FASTBOOL  aReserveBool3;
    FASTBOOL  aReserveBool4;
    long      aReserveLong1;
    long      aReserveLong2;
    long      aReserveLong3;
    long      aReserveLong4;
    void*     aReservePtr1;
    void*     aReservePtr2;
    void*     aReservePtr3;
    void*     aReservePtr4;

    FASTBOOL  bShown;    // Xor sichrbar?
    USHORT    nMinMov;   // Soviel muss erstmal minimal bewegt werden
    FASTBOOL  bMinMoved; // MinMove durchbrochen?

    FASTBOOL  bHorFixed; // nur Vertikal draggen
    FASTBOOL  bVerFixed; // nur Horizontal draggen
    FASTBOOL  bWantNoSnap; // TRUE=Fuer die Entscheidung ob fuer pObj->MovCreate() NoSnapPos verwendet
                          // werden soll. Entsprechend wird auch NoSnapPos in den Buffer geschrieben.
    FASTBOOL  bOrtho4;
    FASTBOOL  bOrtho8;

    SdrDragMethod* pDragMethod;

protected:
    void Clear(FASTBOOL bLeaveOne);
    Point& Pnt(ULONG nNum)                           { return *((Point*)aPnts.GetObject(nNum)); }
public:
    void*    pUser;     // Userdata
public:
    SdrDragStat(): aPnts(1024,16,16)                 { pUser=NULL; Reset(); }
    ~SdrDragStat()                                   { Clear(FALSE); }
    void         Reset();
    SdrView*     GetView() const                     { return pView; }
    void         SetView(SdrView* pV)                { pView=pV; }
    SdrPageView* GetPageView() const                 { return pPageView; }
    void         SetPageView(SdrPageView* pPV)       { pPageView=pPV; }
    const Point& GetPoint(ULONG nNum) const          { return *((Point*)aPnts.GetObject(nNum)); }
    ULONG        GetPointAnz() const                 { return aPnts.Count(); }
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
    void*        GetUser() const                     { return pUser;  }
    void         SetUser(void* pU)                   { pUser=pU;      }
    FASTBOOL     IsShown() const                     { return bShown; }
    void         SetShown(FASTBOOL bOn)              { bShown=bOn; }

    FASTBOOL     IsMinMoved() const                  { return bMinMoved; }
    void         SetMinMoved()                       { bMinMoved=TRUE; }
    void         ResetMinMoved()                     { bMinMoved=FALSE; }
    void         SetMinMove(USHORT nDist)            { nMinMov=nDist; if (nMinMov<1) nMinMov=1; }
    USHORT       GetMinMove() const                  { return nMinMov; }

    FASTBOOL     IsHorFixed() const                  { return bHorFixed; }
    void         SetHorFixed(FASTBOOL bOn)           { bHorFixed=bOn; }
    FASTBOOL     IsVerFixed() const                  { return bVerFixed; }
    void         SetVerFixed(FASTBOOL bOn)           { bVerFixed=bOn; }

    // Hier kann das Obj sagen: "Ich will keinen Koordinatenfang!"
    // z.B. fuer den Winkel des Kreisbogen...
    FASTBOOL     IsNoSnap() const                     { return bWantNoSnap; }
    void         SetNoSnap(FASTBOOL bOn=TRUE)         { bWantNoSnap=bOn; }

    // Und hier kann das Obj sagen welches Ortho (wenn ueberhaupt eins)
    // sinnvoll auf ihm angewendet werden kann.
    // Ortho4 bedeutet Ortho in 4 Richtungen (fuer Rect und Cirt)
    FASTBOOL     IsOrtho4Possible() const             { return bOrtho4; }
    void         SetOrtho4Possible(FASTBOOL bOn=TRUE) { bOrtho4=bOn; }
    // Ortho8 bedeutet Ortho in 8 Richtungen (fuer Linien)
    FASTBOOL     IsOrtho8Possible() const             { return bOrtho8; }
    void         SetOrtho8Possible(FASTBOOL bOn=TRUE) { bOrtho8=bOn; }

    // Wird vom gedraggten Objekt gesetzt
    FASTBOOL     IsEndDragChangesAttributes() const   { return bEndDragChangesAttributes; }
    void         SetEndDragChangesAttributes(FASTBOOL bOn) { bEndDragChangesAttributes=bOn; }
    FASTBOOL     IsEndDragChangesGeoAndAttributes() const   { return bEndDragChangesGeoAndAttributes; }
    void         SetEndDragChangesGeoAndAttributes(FASTBOOL bOn) { bEndDragChangesGeoAndAttributes=bOn; }

    // Wird von der View gesetzt und kann vom Obj ausgewertet werden
    FASTBOOL     IsMouseDown() const                  { return !bMouseIsUp; }
    void         SetMouseDown(FASTBOOL bDown)         { bMouseIsUp=!bDown; }

    long  GetDX() const                     { return GetNow().X()-GetPrev().X(); }
    long  GetDY() const                     { return GetNow().Y()-GetPrev().Y(); }

    SdrDragMethod* GetDragMethod() const               { return pDragMethod; }
    void           SetDragMethod(SdrDragMethod* pMth)  { pDragMethod=pMth; }

    const Rectangle& GetActionRect() const             { return aActionRect; }
    void             SetActionRect(const Rectangle& rR) { aActionRect=rR; }

    // Unter Beruecksichtigung von 1stPointAsCenter
};

}//end of namespace binfilter
#endif //_SVDDRAG_HXX

