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


// predeclarations

class SdrDragView;

class SdrDragMovHdl : public SdrDragMethod
{
protected:
    // define nothing, overload to do so
    virtual void createSdrDragEntries() SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();
    SdrDragMovHdl(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual void MoveSdrDrag(const Point& rPnt) SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual void CancelSdrDrag() SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;
};

class SdrDragRotate : public SdrDragMethod
{
private:
    double                      nSin;
    double                      nCos;
    long                        nWink0;
    long                        nWink;
    bool                        bRight;

public:
    TYPEINFO_OVERRIDE();
    SdrDragRotate(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual void MoveSdrDrag(const Point& rPnt) SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() SAL_OVERRIDE;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) SAL_OVERRIDE;
};

class SdrDragShear : public SdrDragMethod
{
private:
    boost::rational<long>       aFact;
    long                        nWink0;
    long                        nWink;
    double                      nTan;
    bool                        bVertical;   // contort vertically
    bool                        bResize;     // shear and resize
    bool                        bUpSideDown; // mirror and shear/slant
    bool                        bSlant;

public:
    TYPEINFO_OVERRIDE();
    SdrDragShear(SdrDragView& rNewView,bool bSlant1);

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual void MoveSdrDrag(const Point& rPnt) SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() SAL_OVERRIDE;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) SAL_OVERRIDE;
};

class SdrDragMirror : public SdrDragMethod
{
private:
    Point                       aDif;
    long                        nWink;
    bool                        bMirrored;
    bool                        bSide0;

    bool ImpCheckSide(const Point& rPnt) const;

public:
    TYPEINFO_OVERRIDE();
    SdrDragMirror(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual void MoveSdrDrag(const Point& rPnt) SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;

    virtual basegfx::B2DHomMatrix getCurrentTransformation() SAL_OVERRIDE;
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) SAL_OVERRIDE;
};

class SdrDragGradient : public SdrDragMethod
{
private:
    // Handles to work on
    SdrHdlGradient*             pIAOHandle;

    // is this for gradient (or for transparency)?
    bool                        bIsGradient : 1;

public:
    TYPEINFO_OVERRIDE();
    SdrDragGradient(SdrDragView& rNewView, bool bGrad = true);

    bool IsGradient() const { return bIsGradient; }

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual void MoveSdrDrag(const Point& rPnt) SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;
    virtual void CancelSdrDrag() SAL_OVERRIDE;
};

class SdrDragCrook : public SdrDragMethod
{
private:
    Rectangle                   aMarkRect;
    Point                       aMarkCenter;
    Point                       aCenter;
    Point                       aStart;
    boost::rational<long>       aFact;
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
    long                        nWink;
    long                        nMarkSize;
    SdrCrookMode                eMode;

    // helpers for applyCurrentTransformationToPolyPolygon
    void _MovAllPoints(basegfx::B2DPolyPolygon& rTarget);
    void _MovCrookPoint(Point& rPnt, Point* pC1, Point* pC2);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries() SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();
    SdrDragCrook(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual void MoveSdrDrag(const Point& rPnt) SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) SAL_OVERRIDE;
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget) SAL_OVERRIDE;
};

class SdrDragDistort : public SdrDragMethod
{
private:
    Rectangle                   aMarkRect;
    XPolygon                    aDistortedRect;
    sal_uInt16                  nPolyPt;
    bool                        bContortionAllowed;
    bool                        bNoContortionAllowed;
    bool                        bContortion;

    // helper for applyCurrentTransformationToPolyPolygon
    void _MovAllPoints(basegfx::B2DPolyPolygon& rTarget);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries() SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();
    SdrDragDistort(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual void MoveSdrDrag(const Point& rPnt) SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget) SAL_OVERRIDE;
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget) SAL_OVERRIDE;
};

// derive from SdrDragObjOwn to have handles aligned to object when it
// is sheared or rotated
class SdrDragCrop : public SdrDragObjOwn
{
public:
    TYPEINFO_OVERRIDE();
    SdrDragCrop(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const SAL_OVERRIDE;
    virtual bool BeginSdrDrag() SAL_OVERRIDE;
    virtual bool EndSdrDrag(bool bCopy) SAL_OVERRIDE;
    virtual Pointer GetSdrDragPointer() const SAL_OVERRIDE;
};


#endif // INCLUDED_SVX_SOURCE_SVDRAW_SVDDRGM1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
