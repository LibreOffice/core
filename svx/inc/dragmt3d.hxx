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

#ifndef INCLUDED_SVX_INC_DRAGMT3D_HXX
#define INCLUDED_SVX_INC_DRAGMT3D_HXX

#include <svx/svddrgmt.hxx>
#include <svx/view3d.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <vcl/timer.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

// Parameters for the interaction of a 3D object
class E3dDragMethodUnit
{
public:
    E3dObject&                      mr3DObj;
    basegfx::B3DPolyPolygon         maWireframePoly;
    basegfx::B3DHomMatrix           maDisplayTransform;
    basegfx::B3DHomMatrix           maInvDisplayTransform;
    basegfx::B3DHomMatrix           maInitTransform;
    basegfx::B3DHomMatrix           maTransform;
    sal_Int32                       mnStartAngle;
    sal_Int32                       mnLastAngle;

    E3dDragMethodUnit(E3dObject& r3DObj)
    :   mr3DObj(r3DObj),
        maWireframePoly(),
        maDisplayTransform(),
        maInvDisplayTransform(),
        maInitTransform(),
        maTransform(),
        mnStartAngle(0),
        mnLastAngle(0)
    {
    }
};

// Derivative of SdrDragMethod for 3D objects
class E3dDragMethod : public SdrDragMethod
{
protected:
    ::std::vector< E3dDragMethodUnit >  maGrp;
    E3dDragConstraint const             meConstraint;
    Point                               maLastPos;
    tools::Rectangle                           maFullBound;
    bool                                mbMoveFull;
    bool                                mbMovedAtAll;

public:
    E3dDragMethod(
        SdrDragView &rView,
        const SdrMarkList& rMark,
        E3dDragConstraint eConstr,
        bool bFull);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual void CancelSdrDrag() override;
    virtual bool EndSdrDrag(bool bCopy) override;

    // for migration from XOR to overlay
    virtual void CreateOverlayGeometry(
        sdr::overlay::OverlayManager& rOverlayManager,
        const sdr::contact::ObjectContact& rObjectContact) override;
};

// Derivative of SdrDragMethod for spinning 3D objects
class E3dDragRotate : public E3dDragMethod
{
    basegfx::B3DPoint                   maGlobalCenter;

public:
    E3dDragRotate(
        SdrDragView &rView,
        const SdrMarkList& rMark,
        E3dDragConstraint eConstr,
        bool bFull);

    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual PointerStyle GetSdrDragPointer() const override;
};

// Derivative of SdrDragMethod for moving 3D sub-objects
class E3dDragMove : public E3dDragMethod
{
    SdrHdlKind              meWhatDragHdl;
    Point                   maScaleFixPos;

public:
    E3dDragMove(
        SdrDragView &rView,
        const SdrMarkList& rMark,
        SdrHdlKind eDrgHdl,
        E3dDragConstraint eConstr,
        bool bFull);

    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual PointerStyle GetSdrDragPointer() const override;
};


#endif // INCLUDED_SVX_INC_DRAGMT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
