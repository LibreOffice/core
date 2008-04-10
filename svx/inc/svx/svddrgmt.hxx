/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svddrgmt.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SVDDRGMT_HXX
#define _SVDDRGMT_HXX

#include <svx/svddrgv.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@  @@@@@   @@@@   @@@@   @@   @@ @@@@@ @@@@@@ @@  @@  @@@@  @@@@@   @@@@
//  @@  @@ @@  @@ @@  @@ @@  @@  @@@ @@@ @@      @@   @@  @@ @@  @@ @@  @@ @@  @@
//  @@  @@ @@  @@ @@  @@ @@      @@@@@@@ @@      @@   @@  @@ @@  @@ @@  @@ @@
//  @@  @@ @@@@@  @@@@@@ @@ @@@  @@@@@@@ @@@@    @@   @@@@@@ @@  @@ @@  @@  @@@@
//  @@  @@ @@  @@ @@  @@ @@  @@  @@ @ @@ @@      @@   @@  @@ @@  @@ @@  @@     @@
//  @@  @@ @@  @@ @@  @@ @@  @@  @@   @@ @@      @@   @@  @@ @@  @@ @@  @@ @@  @@
//  @@@@@  @@  @@ @@  @@  @@@@@  @@   @@ @@@@@   @@   @@  @@  @@@@  @@@@@   @@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrDragView;
class SdrDragStat;

class SVX_DLLPUBLIC SdrDragMethod {
protected:
    SdrDragView& rView;
    bool         bMoveOnly;
protected:
//#if 0 // _SOLAR__PRIVATE
    void               ImpTakeDescriptionStr(USHORT nStrCacheID, String& rStr, USHORT nVal=0) const;
//#endif // __PRIVATE
    SdrHdl*            GetDragHdl() const              { return rView.pDragHdl; }
    SdrHdlKind         GetDragHdlKind() const          { return rView.eDragHdl; }
    SdrDragStat&       DragStat()                      { return rView.aDragStat; }
    const SdrDragStat& DragStat() const                { return rView.aDragStat; }
    Point&             Ref1() const                    { return rView.aRef1; }
    Point&             Ref2() const                    { return rView.aRef2; }
    const SdrHdlList&  GetHdlList() const              { return rView.aHdl; }
    void               AddUndo(SdrUndoAction* pUndo)   { rView.AddUndo(pUndo); }
    void               SetDragPolys()                  { rView.SetDragPolys(); }
    FASTBOOL           IsDragLimit()                   { return rView.bDragLimit; }
    const Rectangle&   GetDragLimitRect()              { return rView.aDragLimit; }
    const SdrMarkList& GetMarkedObjectList()                   { return rView.GetMarkedObjectList(); }
    Point              GetSnapPos(const Point& rPt) const { return rView.GetSnapPos(rPt,rView.pMarkedPV); }
    USHORT             SnapPos(Point& rPt) const       { return rView.SnapPos(rPt,rView.pMarkedPV); }
    inline const Rectangle& GetMarkedRect() const;
    SdrPageView*       GetDragPV() const;
    SdrObject*         GetDragObj() const;
    OutputDevice*      GetDragWin() const              { return rView.pDragWin; }
    FASTBOOL           IsDraggingPoints() const        { return rView.IsDraggingPoints(); }
    FASTBOOL           IsDraggingGluePoints() const    { return rView.IsDraggingGluePoints(); }

    void CreateOverlayGeometryLines(basegfx::B2DPolyPolygon& rResult);
    void CreateOverlayGeometryPoints(basegfx::B2DPolyPolygon& rResult, const Size& rLogicSize);
    sal_Bool DoAddConnectorOverlays();
    void AddConnectorOverlays(basegfx::B2DPolyPolygon& rResult);
    sal_Bool DoAddDragStripeOverlay();

public:
    TYPEINFO();

    // #i58950# also moved constructor implementation to cxx
    SdrDragMethod(SdrDragView& rNewView);

    // #i58950# virtual destructor was missing
    virtual ~SdrDragMethod();

    virtual void Draw() const;
    virtual void Show();
    virtual void Hide();
    virtual void TakeComment(String& rStr) const=0;
    virtual FASTBOOL Beg()=0;
    virtual void MovAllPoints();
    virtual void MovPoint(Point& rPnt);
    virtual void Mov(const Point& rPnt)=0;
    virtual FASTBOOL End(FASTBOOL bCopy)=0;
    virtual void Brk();
    virtual Pointer GetPointer() const=0;
    virtual FASTBOOL IsMoveOnly() const; // TRUE, wenn nur verschoben wird

    // for migration from XOR to overlay
    virtual void CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager, ::sdr::overlay::OverlayObjectList& rOverlayList);
};

inline const Rectangle& SdrDragMethod::GetMarkedRect() const
{
    return rView.eDragHdl==HDL_POLY ? rView.GetMarkedPointsRect() :
           rView.eDragHdl==HDL_GLUE ? rView.GetMarkedGluePointsRect() :
           rView.GetMarkedObjRect();
}

//************************************************************
//   SdrDragMove
//************************************************************

class SVX_DLLPUBLIC SdrDragMove : public SdrDragMethod
{
    long                        nBestXSnap;
    long                        nBestYSnap;
    FASTBOOL                    bXSnapped;
    FASTBOOL                    bYSnapped;

private:
    void ImpCheckSnap(const Point& rPt);

public:
    TYPEINFO();
    SdrDragMove(SdrDragView& rNewView): SdrDragMethod(rNewView) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovAllPoints();
    virtual void MovPoint(Point& rPnt);
//  virtual void MovPoint(Point& rPnt, const Point& rPvOfs);
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual FASTBOOL IsMoveOnly() const;
    virtual Pointer GetPointer() const;
};

//************************************************************
//   SdrDragResize
//************************************************************

class SVX_DLLPUBLIC SdrDragResize : public SdrDragMethod
{
protected:
    Fraction                    aXFact;
    Fraction                    aYFact;

public:
    TYPEINFO();
    SdrDragResize(SdrDragView& rNewView): SdrDragMethod(rNewView), aXFact(1,1), aYFact(1,1) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovPoint(Point& rPnt);
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
};

//************************************************************
//   SdrDragObjOwn
//************************************************************

class SVX_DLLPUBLIC SdrDragObjOwn : public SdrDragMethod
{
public:
    TYPEINFO();
    SdrDragObjOwn(SdrDragView& rNewView): SdrDragMethod(rNewView) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;

    // for migration from XOR to overlay
    virtual void CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager, ::sdr::overlay::OverlayObjectList& rOverlayList);

    //SJ: added following method, otherwise the object won't be
    //able to notice that dragging has been cancelled.
    virtual void Brk();
};


////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDDRGMT_HXX

