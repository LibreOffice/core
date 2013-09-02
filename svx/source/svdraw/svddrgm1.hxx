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

#ifndef _SVDDRGM1_HXX
#define _SVDDRGM1_HXX

#include <svx/xpoly.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrgv.hxx>
#include <svx/svddrgmt.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrDragView;

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMovHdl

class SdrDragMovHdl : public SdrDragMethod
{
protected:
    // define nothing, overload to do so
    virtual void createSdrDragEntries();

public:
    TYPEINFO();
    SdrDragMovHdl(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual void CancelSdrDrag();
    virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragRotate

class SdrDragRotate : public SdrDragMethod
{
private:
    double                      nSin;
    double                      nCos;
    long                        nWink0;
    long                        nWink;
    bool                        bRight;

public:
    TYPEINFO();
    SdrDragRotate(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragShear

class SdrDragShear : public SdrDragMethod
{
private:
    Fraction                    aFact;
    long                        nWink0;
    long                        nWink;
    double                      nTan;
    bool                        bVertical;   // contort vertically
    bool                        bResize;     // shear and resize
    bool                        bUpSideDown; // mirror and shear/slant
    bool                        bSlant;

public:
    TYPEINFO();
    SdrDragShear(SdrDragView& rNewView,bool bSlant1);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMirror

class SdrDragMirror : public SdrDragMethod
{
private:
    Point                       aDif;
    long                        nWink;
    bool                        bMirrored;
    bool                        bSide0;

    bool ImpCheckSide(const Point& rPnt) const;

public:
    TYPEINFO();
    SdrDragMirror(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragGradient

class SdrDragGradient : public SdrDragMethod
{
private:
    // Handles to work on
    SdrHdlGradient*             pIAOHandle;

    // is this for gradient (or for transparency)?
    unsigned                    bIsGradient : 1;

public:
    TYPEINFO();
    SdrDragGradient(SdrDragView& rNewView, bool bGrad = true);

    bool IsGradient() const { return bIsGradient; }

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
    virtual void CancelSdrDrag();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragCrook

class SdrDragCrook : public SdrDragMethod
{
private:
    Rectangle                   aMarkRect;
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
    long                        nWink;
    long                        nMarkSize;
    SdrCrookMode                eMode;

    // helpers for applyCurrentTransformationToPolyPolygon
    void _MovAllPoints(basegfx::B2DPolyPolygon& rTarget);
    void _MovCrookPoint(Point& rPnt, Point* pC1, Point* pC2);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries();

public:
    TYPEINFO();
    SdrDragCrook(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
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
    virtual void createSdrDragEntries();

public:
    TYPEINFO();
    SdrDragDistort(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
    virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragCrop

class SdrDragCrop : public SdrDragResize
{
public:
    TYPEINFO();
    SdrDragCrop(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(OUString& rStr) const;
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_SVDDRGM1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
