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
|* Parameters for the interaction of a 3D object
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
|* Derivative of SdrDragMethod for 3D objects
|*
\************************************************************************/

class E3dDragMethod : public SdrDragMethod
{
protected:
    ::std::vector< E3dDragMethodUnit >  maGrp;
    E3dDragConstraint                   meConstraint;
    Point                               maLastPos;
    Rectangle                           maFullBound;
    bool                                mbMoveFull;
    bool                                mbMovedAtAll;

public:
    TYPEINFO();
    E3dDragMethod(
        SdrDragView &rView,
        const SdrMarkList& rMark,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        sal_Bool bFull = sal_False);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual void CancelSdrDrag();
    virtual bool EndSdrDrag(bool bCopy);

    E3dView& Get3DView()  { return (E3dView&)getSdrDragView();  }

    // for migration from XOR to overlay
    virtual void CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager);
};


/*************************************************************************
|*
|* Derivative of SdrDragMethod for spinning 3D objects
|*
\************************************************************************/

class E3dDragRotate : public E3dDragMethod
{
    basegfx::B3DPoint                   maGlobalCenter;

public:
    TYPEINFO();
    E3dDragRotate(
        SdrDragView &rView,
        const SdrMarkList& rMark,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        sal_Bool bFull = sal_False);

    virtual void MoveSdrDrag(const Point& rPnt);
    virtual Pointer GetSdrDragPointer() const;
};


/*************************************************************************
|*
|* Derivative of SdrDragMethod for moving 3D sub-objects
|*
\************************************************************************/

class E3dDragMove : public E3dDragMethod
{
    SdrHdlKind              meWhatDragHdl;
    Point                   maScaleFixPos;

public:
    TYPEINFO();
    E3dDragMove(
        SdrDragView &rView,
        const SdrMarkList& rMark,
        SdrHdlKind eDrgHdl = HDL_MOVE,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        sal_Bool bFull = sal_False);

    virtual void MoveSdrDrag(const Point& rPnt);
    virtual Pointer GetSdrDragPointer() const;
};


#endif          // _E3D_DRAGMT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
