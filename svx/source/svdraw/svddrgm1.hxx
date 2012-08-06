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
private:
    bool                    bMirrObjShown;

protected:
    // define nothing, overload to do so
    virtual void createSdrDragEntries();

public:
    TYPEINFO();
    SdrDragMovHdl(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
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

    virtual void TakeSdrDragComment(String& rStr) const;
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

    virtual void TakeSdrDragComment(String& rStr) const;
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

    virtual void TakeSdrDragComment(String& rStr) const;
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

    virtual void TakeSdrDragComment(String& rStr) const;
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

    virtual void TakeSdrDragComment(String& rStr) const;
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

    virtual void TakeSdrDragComment(String& rStr) const;
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

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_SVDDRGM1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
