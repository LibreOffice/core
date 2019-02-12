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

#ifndef INCLUDED_SVX_SOURCE_SVDRAW_SVDDRGM1_HXX
#define INCLUDED_SVX_SOURCE_SVDRAW_SVDDRGM1_HXX

#include <svx/xpoly.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrgv.hxx>
#include <svx/svddrgmt.hxx>

class SdrDragView;

class SdrDragMovHdl : public SdrDragMethod
{
protected:
    // define nothing, override to do so
    virtual void createSdrDragEntries() override;

public:
    explicit SdrDragMovHdl(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual void CancelSdrDrag() override;
    virtual PointerStyle GetSdrDragPointer() const override;
};

class SdrDragRotate : public SdrDragMethod
{
private:
    double                      nSin;
    double                      nCos;
    long                        nAngle0;
    long                        nAngle;
    bool                        bRight;

public:
    explicit SdrDragRotate(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual PointerStyle GetSdrDragPointer() const override;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() override;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) override;
};

class SdrDragShear : public SdrDragMethod
{
private:
    Fraction                    aFact;
    long                        nAngle0;
    long                        nAngle;
    double                      nTan;
    bool                        bVertical;   // contort vertically
    bool                        bResize;     // shear and resize
    bool                        bUpSideDown; // mirror and shear/slant
    bool const                  bSlant;

public:
    SdrDragShear(SdrDragView& rNewView,bool bSlant1);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual PointerStyle GetSdrDragPointer() const override;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() override;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) override;
};

class SdrDragMirror : public SdrDragMethod
{
private:
    Point                       aDif;
    long                        nAngle;
    bool                        bMirrored;
    bool                        bSide0;

    bool ImpCheckSide(const Point& rPnt) const;

public:
    explicit SdrDragMirror(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual PointerStyle GetSdrDragPointer() const override;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() override;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) override;
};

class SdrDragGradient : public SdrDragMethod
{
private:
    // Handles to work on
    SdrHdlGradient*             pIAOHandle;

    // is this for gradient (or for transparency)?
    bool const                  bIsGradient : 1;

public:
    SdrDragGradient(SdrDragView& rNewView, bool bGrad = true);

    bool IsGradient() const { return bIsGradient; }

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual PointerStyle GetSdrDragPointer() const override;
    virtual void CancelSdrDrag() override;
};

class SdrDragCrook : public SdrDragMethod
{
private:
    tools::Rectangle                   aMarkRect;
    Point                       aMarkCenter;
    Point                       aCenter;
    Point                       aStart;
    Fraction                    aFact;
    Point                       aRad;
    bool                        bContortionAllowed;
    bool                        bNoContortionAllowed;
    bool                        bContortion;
    bool                        bResizeAllowed;
    bool                        bResize;
    bool                        bRotateAllowed;
    bool                        bRotate;
    bool                        bVertical;
    bool                        bValid;
    bool                        bLft;
    bool                        bRgt;
    bool                        bUpr;
    bool                        bLwr;
    bool                        bAtCenter;
    long                        nAngle;
    long                        nMarkSize;
    SdrCrookMode                eMode;

    // helpers for applyCurrentTransformationToPolyPolygon
    void MovAllPoints(basegfx::B2DPolyPolygon& rTarget);
    void MovCrookPoint(Point& rPnt, Point* pC1, Point* pC2);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries() override;

public:
    explicit SdrDragCrook(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual PointerStyle GetSdrDragPointer() const override;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) override;
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget) override;
};

class SdrDragDistort : public SdrDragMethod
{
private:
    tools::Rectangle                   aMarkRect;
    XPolygon                    aDistortedRect;
    sal_uInt16                  nPolyPt;
    bool                        bContortionAllowed;
    bool                        bNoContortionAllowed;
    bool                        bContortion;

    // helper for applyCurrentTransformationToPolyPolygon
    void MovAllPoints(basegfx::B2DPolyPolygon& rTarget);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries() override;

public:
    explicit SdrDragDistort(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual PointerStyle GetSdrDragPointer() const override;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) override;
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget) override;
};

// derive from SdrDragObjOwn to have handles aligned to object when it
// is sheared or rotated
class SdrDragCrop : public SdrDragObjOwn
{
public:
    explicit SdrDragCrop(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const override;
    virtual bool BeginSdrDrag() override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual PointerStyle GetSdrDragPointer() const override;
};

#endif // INCLUDED_SVX_SOURCE_SVDRAW_SVDDRGM1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
