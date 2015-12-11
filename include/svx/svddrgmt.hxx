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

#ifndef INCLUDED_SVX_SVDDRGMT_HXX
#define INCLUDED_SVX_SVDDRGMT_HXX

#include <svx/svddrgv.hxx>
#include <svx/svxdllapi.h>
#include <svx/sdr/contact/objectcontact.hxx>

class SdrDragView;
class SdrDragStat;

class SVX_DLLPUBLIC SdrDragEntry
{
private:
    // bitfield
    bool                        mbAddToTransparent : 1;

protected:
    // access for derived classes
    void setAddToTransparent(bool bNew) { mbAddToTransparent = bNew; }

public:
    SdrDragEntry();
    virtual ~SdrDragEntry();

    virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod) = 0;

    // data read access
    bool getAddToTransparent() const { return mbAddToTransparent; }
};



class SVX_DLLPUBLIC SdrDragEntryPolyPolygon : public SdrDragEntry
{
private:
    basegfx::B2DPolyPolygon             maOriginalPolyPolygon;

public:
    SdrDragEntryPolyPolygon(const basegfx::B2DPolyPolygon& rOriginalPolyPolygon);
    virtual ~SdrDragEntryPolyPolygon();

    virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod) override;
};



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

    virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod) override;
};



class SdrDragEntryPrimitive2DSequence : public SdrDragEntry
{
private:
    drawinglayer::primitive2d::Primitive2DContainer  maPrimitive2DSequence;

public:
    SdrDragEntryPrimitive2DSequence(
        const drawinglayer::primitive2d::Primitive2DContainer& rSequence,
        bool bAddToTransparent);
    virtual ~SdrDragEntryPrimitive2DSequence();

    virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod) override;
};



class SdrDragEntryPointGlueDrag : public SdrDragEntry
{
private:
    std::vector< basegfx::B2DPoint >                maPositions;
    bool                                            mbIsPointDrag;

public:
    SdrDragEntryPointGlueDrag(const std::vector< basegfx::B2DPoint >& rPositions, bool bIsPointDrag);
    virtual ~SdrDragEntryPointGlueDrag();

    virtual drawinglayer::primitive2d::Primitive2DContainer createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod) override;
};



class SVX_DLLPUBLIC SdrDragMethod
{
private:
    std::vector< SdrDragEntry* >            maSdrDragEntries;
    sdr::overlay::OverlayObjectList         maOverlayObjectList;
    SdrDragView&                            mrSdrDragView;

    // bitfield
    bool                                    mbMoveOnly : 1;
    bool                                    mbSolidDraggingActive : 1;
    bool                                    mbShiftPressed : 1;

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
    SdrHdl*            GetDragHdl() const              { return getSdrDragView().mpDragHdl; }
    SdrHdlKind         GetDragHdlKind() const          { return getSdrDragView().meDragHdl; }
    SdrDragStat&       DragStat()                      { return getSdrDragView().maDragStat; }
    const SdrDragStat& DragStat() const                { return getSdrDragView().maDragStat; }
    Point&             Ref1() const                    { return mrSdrDragView.maRef1; }
    Point&             Ref2() const                    { return mrSdrDragView.maRef2; }
    const SdrHdlList&  GetHdlList() const              { return getSdrDragView().GetHdlList(); }
    void               AddUndo(SdrUndoAction* pUndo)   { getSdrDragView().AddUndo(pUndo); }
    bool               IsDragLimit()                   { return getSdrDragView().mbDragLimit; }
    const Rectangle&   GetDragLimitRect()              { return getSdrDragView().maDragLimit; }
    const SdrMarkList& GetMarkedObjectList()                   { return getSdrDragView().GetMarkedObjectList(); }
    Point              GetSnapPos(const Point& rPt) const { return getSdrDragView().GetSnapPos(rPt,getSdrDragView().mpMarkedPV); }
    SdrSnap            SnapPos(Point& rPt) const       { return getSdrDragView().SnapPos(rPt,getSdrDragView().mpMarkedPV); }
    inline const Rectangle& GetMarkedRect() const;
    SdrPageView*       GetDragPV() const;
    SdrObject*         GetDragObj() const;
    bool               IsDraggingPoints() const        { return getSdrDragView().IsDraggingPoints(); }
    bool               IsDraggingGluePoints() const    { return getSdrDragView().IsDraggingGluePoints(); }

    bool DoAddConnectorOverlays();
    drawinglayer::primitive2d::Primitive2DContainer AddConnectorOverlays();

public:

    void resetSdrDragEntries();
    basegfx::B2DRange getCurrentRange() const;

    // #i58950# also moved constructor implementation to cxx
    SdrDragMethod(SdrDragView& rNewView);

    // #i58950# virtual destructor was missing
    virtual ~SdrDragMethod();

    void Show();
    void Hide();
    bool IsShiftPressed() { return mbShiftPressed; }
    void SetShiftPressed(bool bShiftPressed) { mbShiftPressed = bShiftPressed; }
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
    return getSdrDragView().meDragHdl==HDL_POLY ? getSdrDragView().GetMarkedPointsRect() :
           getSdrDragView().meDragHdl==HDL_GLUE ? getSdrDragView().GetMarkedGluePointsRect() :
           getSdrDragView().GetMarkedObjRect();
}


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
    virtual void createSdrDragEntryForSdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool bModify) override;

public:
    SdrDragMove(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual Pointer GetSdrDragPointer() const override;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() override;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) override;
};


//   SdrDragResize

class SVX_DLLPUBLIC SdrDragResize : public SdrDragMethod
{
protected:
    Fraction                    aXFact;
    Fraction                    aYFact;

public:
    SdrDragResize(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual Pointer GetSdrDragPointer() const override;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() override;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) override;
};


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
    virtual void createSdrDragEntries() override;

public:
    SdrDragObjOwn(SdrDragView& rNewView);
    virtual ~SdrDragObjOwn();

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual Pointer GetSdrDragPointer() const override;
};

#endif // INCLUDED_SVX_SVDDRGMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
