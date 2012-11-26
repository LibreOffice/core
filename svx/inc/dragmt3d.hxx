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



#ifndef _E3D_DRAGMT3D_HXX
#define _E3D_DRAGMT3D_HXX

#include <svx/svddrgmt.hxx>
#include <svx/view3d.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <vcl/timer.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

class E3dScene;

/*************************************************************************
|*
|* Parameter fuer Interaktion eines 3D-Objektes
|*
\************************************************************************/
class E3dDragMethodUnit
{
public:
    E3dObject*                      mp3DObj;
    basegfx::B3DPolyPolygon         maWireframePoly;
    basegfx::B3DHomMatrix           maDisplayTransform;
    basegfx::B3DHomMatrix           maInvDisplayTransform;
    basegfx::B3DHomMatrix           maInitTransform;
    basegfx::B3DHomMatrix           maTransform;
    sal_Int32                       mnStartAngle;
    sal_Int32                       mnLastAngle;

    E3dDragMethodUnit()
    :   mp3DObj(0),
        maWireframePoly(),
        maDisplayTransform(),
        maInvDisplayTransform(),
        maInitTransform(),
        maTransform(),
        mnStartAngle(0),
        mnLastAngle(0)
    {}
};

/*************************************************************************
|*
|* Ableitung von SdrDragMethod fuer 3D-Objekte
|*
\************************************************************************/

class E3dDragMethod : public SdrDragMethod
{
protected:
    ::std::vector< E3dDragMethodUnit >  maGrp;
    E3dDragConstraint                   meConstraint;
    basegfx::B2DPoint                   maLastPos;
    basegfx::B2DRange                   maFullBound;
    bool                                mbMoveFull;
    bool                                mbMovedAtAll;

public:
    E3dDragMethod(
        SdrView &rView,
        const SdrObjectVector& rSelection,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        sal_Bool bFull = sal_False);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual void CancelSdrDrag();
    virtual bool EndSdrDrag(bool bCopy);

    E3dView& Get3DView()  { return (E3dView&)getSdrView();  }

    // for migration from XOR to overlay
    virtual void CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager);
};


/*************************************************************************
|*
|* Ableitung von SdrDragMethod zum Drehen von 3D-Objekten
|*
\************************************************************************/

class E3dDragRotate : public E3dDragMethod
{
    basegfx::B3DPoint                   maGlobalCenter;

public:
    E3dDragRotate(
        SdrView &rView,
        const SdrObjectVector& rSelection,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        sal_Bool bFull = sal_False);

    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual Pointer GetSdrDragPointer() const;
};


/*************************************************************************
|*
|* Ableitung von SdrDragMethod zum Verschieben von 3D-Subobjekten
|*
\************************************************************************/

class E3dDragMove : public E3dDragMethod
{
    SdrHdlKind              meWhatDragHdl;
    basegfx::B2DPoint       maScaleFixPos;

public:
    E3dDragMove(
        SdrView &rView,
        const SdrObjectVector& rSelection,
        SdrHdlKind eDrgHdl = HDL_MOVE,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        sal_Bool bFull = sal_False);

    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual Pointer GetSdrDragPointer() const;
};


#endif          // _E3D_DRAGMT3D_HXX
