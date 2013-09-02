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

#ifndef _SVDDRGMT_HXX
#define _SVDDRGMT_HXX

#include <svx/svddrgv.hxx>
#include "svx/svxdllapi.h"
#include <svx/sdr/contact/objectcontact.hxx>

class SdrDragView;
class SdrDragStat;

class SVX_DLLPUBLIC SdrDragEntry
{
private:
    // bitfield
    unsigned                    mbAddToTransparent : 1;

protected:
    // access for derived classes
    void setAddToTransparent(bool bNew) { mbAddToTransparent = bNew; }

public:
    SdrDragEntry();
    virtual ~SdrDragEntry();

    virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod) = 0;

    // data read access
    bool getAddToTransparent() const { return mbAddToTransparent; }
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrDragEntryPolyPolygon : public SdrDragEntry
{
private:
    basegfx::B2DPolyPolygon             maOriginalPolyPolygon;

public:
    SdrDragEntryPolyPolygon(const basegfx::B2DPolyPolygon& rOriginalPolyPolygon);
    virtual ~SdrDragEntryPolyPolygon();

    virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod);
};

//////////////////////////////////////////////////////////////////////////////

class SdrDragEntrySdrObject : public SdrDragEntry
{
private:
    const SdrObject&                                maOriginal;
    SdrObject*                                      mpClone;
    sdr::contact::ObjectContact&                    mrObjectContact;
    bool                                            mbModify;

public:
    SdrDragEntrySdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool bModify);
    virtual ~SdrDragEntrySdrObject();

    // #i54102# Split createPrimitive2DSequenceInCurrentState in prepareCurrentState and processing,
    // added accessors to original and clone
    void prepareCurrentState(SdrDragMethod& rDragMethod);
    const SdrObject& getOriginal() const { return maOriginal; }
    SdrObject* getClone() { return mpClone; }

    virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod);
};

//////////////////////////////////////////////////////////////////////////////

class SdrDragEntryPrimitive2DSequence : public SdrDragEntry
{
private:
    drawinglayer::primitive2d::Primitive2DSequence  maPrimitive2DSequence;

public:
    SdrDragEntryPrimitive2DSequence(
        const drawinglayer::primitive2d::Primitive2DSequence& rSequence,
        bool bAddToTransparent);
    virtual ~SdrDragEntryPrimitive2DSequence();

    virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod);
};

//////////////////////////////////////////////////////////////////////////////

class SdrDragEntryPointGlueDrag : public SdrDragEntry
{
private:
    std::vector< basegfx::B2DPoint >                maPositions;
    bool                                            mbIsPointDrag;

public:
    SdrDragEntryPointGlueDrag(const std::vector< basegfx::B2DPoint >& rPositions, bool bIsPointDrag);
    virtual ~SdrDragEntryPointGlueDrag();

    virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod);
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrDragMethod
{
private:
    std::vector< SdrDragEntry* >            maSdrDragEntries;
    sdr::overlay::OverlayObjectList         maOverlayObjectList;
    SdrDragView&                            mrSdrDragView;

    // bitfield
    unsigned                                mbMoveOnly : 1;
    unsigned                                mbSolidDraggingActive : 1;

protected:
    // access for derivated classes to maSdrDragEntries
    void clearSdrDragEntries();
    void addSdrDragEntry(SdrDragEntry* pNew);
    virtual void createSdrDragEntries();
    virtual void createSdrDragEntryForSdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool bModify);

    // access for derivated classes to maOverlayObjectList
    void clearOverlayObjectList() { maOverlayObjectList.clear(); }
    void addToOverlayObjectList(sdr::overlay::OverlayObject& rNew) { maOverlayObjectList.append(rNew); }
    basegfx::B2DRange getB2DRangeFromOverlayObjectList() const { return maOverlayObjectList.getBaseRange(); }

    // access for derivated classes to mrSdrDragView
    SdrDragView& getSdrDragView() { return mrSdrDragView; }
    const SdrDragView& getSdrDragView() const { return mrSdrDragView; }

    // access for derivated classes for bools
    void setMoveOnly(bool bNew) { mbMoveOnly = bNew; }
    void setSolidDraggingActive(bool bNew) { mbSolidDraggingActive = bNew; }

    // internal helpers for creation of standard drag entries
    void createSdrDragEntries_SolidDrag();
    void createSdrDragEntries_PolygonDrag();
    void createSdrDragEntries_PointDrag();
    void createSdrDragEntries_GlueDrag();

    // old call forwarders to the SdrDragView
    void               ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 nVal=0) const;
    SdrHdl*            GetDragHdl() const              { return getSdrDragView().pDragHdl; }
    SdrHdlKind         GetDragHdlKind() const          { return getSdrDragView().eDragHdl; }
    SdrDragStat&       DragStat()                      { return getSdrDragView().aDragStat; }
    const SdrDragStat& DragStat() const                { return getSdrDragView().aDragStat; }
    Point&             Ref1() const                    { return mrSdrDragView.aRef1; }
    Point&             Ref2() const                    { return mrSdrDragView.aRef2; }
    const SdrHdlList&  GetHdlList() const              { return getSdrDragView().aHdl; }
    void               AddUndo(SdrUndoAction* pUndo)   { getSdrDragView().AddUndo(pUndo); }
    bool               IsDragLimit()                   { return getSdrDragView().bDragLimit; }
    const Rectangle&   GetDragLimitRect()              { return getSdrDragView().aDragLimit; }
    const SdrMarkList& GetMarkedObjectList()                   { return getSdrDragView().GetMarkedObjectList(); }
    Point              GetSnapPos(const Point& rPt) const { return getSdrDragView().GetSnapPos(rPt,getSdrDragView().pMarkedPV); }
    sal_uInt16         SnapPos(Point& rPt) const       { return getSdrDragView().SnapPos(rPt,getSdrDragView().pMarkedPV); }
    inline const Rectangle& GetMarkedRect() const;
    SdrPageView*       GetDragPV() const;
    SdrObject*         GetDragObj() const;
    OutputDevice*      GetDragWin() const              { return getSdrDragView().pDragWin; }
    bool               IsDraggingPoints() const        { return getSdrDragView().IsDraggingPoints(); }
    bool               IsDraggingGluePoints() const    { return getSdrDragView().IsDraggingGluePoints(); }

    bool DoAddConnectorOverlays();
    drawinglayer::primitive2d::Primitive2DSequence AddConnectorOverlays();

public:
    TYPEINFO();

    void resetSdrDragEntries();
    basegfx::B2DRange getCurrentRange() const;

    // #i58950# also moved constructor implementation to cxx
    SdrDragMethod(SdrDragView& rNewView);

    // #i58950# virtual destructor was missing
    virtual ~SdrDragMethod();

    void Show();
    void Hide();
    virtual void TakeSdrDragComment(OUString& rStr) const=0;
    virtual bool BeginSdrDrag()=0;
    virtual void MoveSdrDrag(const Point& rPnt)=0;
    virtual bool EndSdrDrag(bool bCopy)=0;
    virtual void CancelSdrDrag();
    virtual Pointer GetSdrDragPointer() const=0;

    virtual void CreateOverlayGeometry(sdr::overlay::OverlayManager& rOverlayManager);
    void destroyOverlayGeometry();

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget);

    // data read access
    bool getMoveOnly() const { return mbMoveOnly; }
    bool getSolidDraggingActive() const { return mbSolidDraggingActive; }
};

inline const Rectangle& SdrDragMethod::GetMarkedRect() const
{
    return getSdrDragView().eDragHdl==HDL_POLY ? getSdrDragView().GetMarkedPointsRect() :
           getSdrDragView().eDragHdl==HDL_GLUE ? getSdrDragView().GetMarkedGluePointsRect() :
           getSdrDragView().GetMarkedObjRect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMove

class SVX_DLLPUBLIC SdrDragMove : public SdrDragMethod
{
private:
    long                        nBestXSnap;
    long                        nBestYSnap;
    bool                        bXSnapped;
    bool                        bYSnapped;

    void ImpCheckSnap(const Point& rPt);

protected:
    virtual void createSdrDragEntryForSdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool bModify);

public:
    TYPEINFO();
    SdrDragMove(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragResize

class SVX_DLLPUBLIC SdrDragResize : public SdrDragMethod
{
protected:
    Fraction                    aXFact;
    Fraction                    aYFact;

public:
    TYPEINFO();
    SdrDragResize(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragObjOwn

class SVX_DLLPUBLIC SdrDragObjOwn : public SdrDragMethod
{
private:
    // SdrDragObjOwn always works on a clone since it has no transformation
    // mechanism to modify wireframe visualisations, but uses the
    // SdrObject::applySpecialDrag() method to change a clone of the
    // SdrObject
    SdrObject*                  mpClone;

protected:
    virtual void createSdrDragEntries();

public:
    TYPEINFO();
    SdrDragObjOwn(SdrDragView& rNewView);
    virtual ~SdrDragObjOwn();

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
};

#endif //_SVDDRGMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
