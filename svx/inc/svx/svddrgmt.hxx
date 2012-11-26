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



#ifndef _SVDDRGMT_HXX
#define _SVDDRGMT_HXX

#include <svx/svdview.hxx>
#include "svx/svxdllapi.h"
#include <svx/sdr/contact/objectcontact.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrDragStat;

//////////////////////////////////////////////////////////////////////////////

class SdrDragEntry
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

    // #54102# Split createPrimitive2DSequenceInCurrentState in prepareCurrentState and processing,
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
    SdrView&                                mrSdrView;

    // bitfield
    bool                                    mbMoveOnly : 1;
    bool                                    mbSolidDraggingActive : 1;

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

    // access for derivated classes to mrSdrView
    SdrView& getSdrView() { return mrSdrView; }
    const SdrView& getSdrView() const { return mrSdrView; }

    // access for derivated classes for bools
    void setMoveOnly(bool bNew) { mbMoveOnly = bNew; }
    void setSolidDraggingActive(bool bNew) { mbSolidDraggingActive = bNew; }

    // internal helpers for creation of standard drag entries
    void createSdrDragEntries_SolidDrag();
    void createSdrDragEntries_PolygonDrag();
    void createSdrDragEntries_PointDrag();
    void createSdrDragEntries_GlueDrag();

    void               TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, String& rStr, sal_uInt16 nVal=0) const;
    SdrHdl*            GetDragHdl() const              { return getSdrView().GetDragHdl(); }
    SdrHdlKind         GetDragHdlKind() const          { return getSdrView().GetDragHdlKind(); }

    SdrDragStat&       DragStat()                      { return getSdrView().GetDragStat(); }
    const SdrDragStat& DragStat() const                { return getSdrView().GetDragStat(); }

    const basegfx::B2DPoint& GetRef1() const           { return getSdrView().GetRef1(); }
    void SetRef1(const basegfx::B2DPoint& rNew)        { getSdrView().SetRef1(rNew); }
    const basegfx::B2DPoint& GetRef2() const           { return getSdrView().GetRef2(); }
    void SetRef2(const basegfx::B2DPoint& rNew)        { getSdrView().SetRef2(rNew); }

    const SdrHdlList&  GetHdlList() const              { return getSdrView().GetHdlList(); }
    void               AddUndo(SdrUndoAction* pUndo)   { getSdrView().AddUndo(pUndo); }
    bool               IsDragLimit()                   { return getSdrView().IsDragLimit(); }
    const basegfx::B2DRange& GetMarkedRange() const;

    SdrObject*         GetDragObj() const;
    bool               IsDraggingPoints() const        { return getSdrView().IsDraggingPoints(); }
    bool               IsDraggingGluePoints() const    { return getSdrView().IsDraggingGluePoints(); }

    bool DoAddConnectorOverlays();
    drawinglayer::primitive2d::Primitive2DSequence AddConnectorOverlays();

public:
    void resetSdrDragEntries();
    basegfx::B2DRange getCurrentRange() const;

    // #i58950# also moved constructor implementation to cxx
    SdrDragMethod(SdrView& rNewView);

    // #i58950# virtual destructor was missing
    virtual ~SdrDragMethod();

    void Show();
    void Hide();
    virtual void TakeSdrDragComment(String& rStr) const=0;
    virtual bool BeginSdrDrag()=0;
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt)=0;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMove

class SVX_DLLPUBLIC SdrDragMove : public SdrDragMethod
{
private:
    // own local snap to allow single Top-Left snap
    basegfx::B2DVector          maBestSnap;

    /// bitfield
    bool                        mbXSnapped : 1;
    bool                        mbYSnapped : 1;

    void ImpCheckSnap(const basegfx::B2DPoint& rPt);

protected:
    virtual void createSdrDragEntryForSdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool bModify);

public:
    SdrDragMove(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragResize

class SVX_DLLPUBLIC SdrDragResize : public SdrDragMethod
{
protected:
    basegfx::B2DVector          maScale;
    Fraction                    aXFact;
    Fraction                    aYFact;

public:
    SdrDragResize(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
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
    SdrDragObjOwn(SdrView& rNewView);
    virtual ~SdrDragObjOwn();

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_SVDDRGMT_HXX

// eof
