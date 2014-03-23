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



#ifndef _SVDDRGM1_HXX
#define _SVDDRGM1_HXX

#include <svx/svdhdl.hxx>
#include <svx/svdview.hxx>
#include <svx/svddrgmt.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrDragStat;

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMovHdl

class SdrDragMovHdl : public SdrDragMethod
{
private:
protected:
    // define nothing, overload to do so
    virtual void createSdrDragEntries();

public:
    SdrDragMovHdl(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual void CancelSdrDrag();
    virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragRotate

class SdrDragRotate : public SdrDragMethod
{
private:
    double                      mfStartRotation;
    double                      mfDeltaRotation;

public:
    SdrDragRotate(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragShear

class SdrDragShear : public SdrDragMethod
{
private:
    double                      mfStartAngle;
    double                      mfDeltaAngle;
    double                      mfFactor;

    /// bitfield
    bool                        mbVertical : 1;   // Vertikales verzerren

public:
    SdrDragShear(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMirror

class SdrDragMirror : public SdrDragMethod
{
private:
public:
    SdrDragMirror(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragGradient

class SdrDragGradient : public SdrDragMethod
{
private:
    // Handles to work on
    SdrHdlGradient*             pIAOHandle;

    // is this for gradient (or for transparence) ?
    bool                        bIsGradient : 1;

public:
    SdrDragGradient(SdrView& rNewView, bool bGrad = true);

    bool IsGradient() const { return bIsGradient; }

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
    virtual void CancelSdrDrag();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragCrook

class SdrDragCrook : public SdrDragMethod
{
private:
    basegfx::B2DRange       maMarkedRange;

    basegfx::B2DPoint       maMarkedCenter;
    basegfx::B2DPoint       maCenter;
    basegfx::B2DPoint       maStart;
    basegfx::B2DPoint       maRadius;

    double                  mfFactor;
    double                  mfAngle;
    double                  mfMarkedSize;

    SdrCrookMode            meMode;

    /// bitfield
    bool                    mbContortionAllowed : 1;
    bool                    mbNoContortionAllowed : 1;
    bool                    mbContortion : 1;
    bool                    mbResizeAllowed : 1;
    bool                    mbResize : 1;
    bool                    mbRotateAllowed : 1;
    bool                    mbRotate : 1;
    bool                    mbVertical : 1;
    bool                    mbValid : 1;
    bool                    mbLeft : 1;
    bool                    mbRight : 1;
    bool                    mbTop : 1;
    bool                    mbBottom : 1;
    bool                    mbAtCenter : 1;

    // helpers for applyCurrentTransformationToPolyPolygon
    void _MovAllPoints(basegfx::B2DPolyPolygon& rTarget);
    void _MovCrookPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries();

public:
    SdrDragCrook(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragDistort

class SdrDragDistort : public SdrDragMethod
{
private:
    basegfx::B2DRange       maMarkedRange;
    basegfx::B2DPolygon     maDistortedRangePolygon;
    sal_uInt32              mnPointIndex;

    /// bitfield
    bool                    mbContortionAllowed : 1;
    bool                    mbNoContortionAllowed : 1;
    bool                    mbContortion : 1;

    // helper for applyCurrentTransformationToPolyPolygon
    void _MovAllPoints(basegfx::B2DPolyPolygon& rTarget);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries();

public:
    SdrDragDistort(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragCrop

// derive from SdrDragObjOwn to have handles aligned to object when it
// is sheared or rotated
class SdrDragCrop : public SdrDragObjOwn
{
public:
    SdrDragCrop(SdrView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_SVDDRGM1_HXX

// eof
