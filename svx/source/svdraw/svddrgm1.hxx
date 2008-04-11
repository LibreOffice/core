/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svddrgm1.hxx,v $
 * $Revision: 1.11 $
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

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrDragView;
class SdrDragStat;

//************************************************************
//   SdrDragMovHdl
//************************************************************

class SdrDragMovHdl : public SdrDragMethod
{
    FASTBOOL                    bMirrObjShown;

public:
    TYPEINFO();
    SdrDragMovHdl(SdrDragView& rNewView): SdrDragMethod(rNewView), bMirrObjShown(FALSE) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual void Brk();
    virtual Pointer GetPointer() const;
    virtual void Show();
    virtual void Hide();
};

//************************************************************
//   SdrDragRotate
//************************************************************

class SdrDragRotate : public SdrDragMethod
{
protected:
    double                      nSin;
    double                      nCos;
    long                        nWink0;
    long                        nWink;
    FASTBOOL                    bRight;

public:
    TYPEINFO();
    SdrDragRotate(SdrDragView& rNewView): SdrDragMethod(rNewView),nSin(0.0),nCos(1.0),nWink0(0),nWink(0),bRight(FALSE) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovPoint(Point& rPnt);
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
};

//************************************************************
//   SdrDragShear
//************************************************************

class SdrDragShear : public SdrDragMethod
{
    Fraction                    aFact;
    long                        nWink0;
    long                        nWink;
    double                      nTan;
    FASTBOOL                    bVertical;   // Vertikales verzerren
    FASTBOOL                    bResize;     // Shear mit Resize
    FASTBOOL                    bUpSideDown; // Beim Shear/Slant gespiegelt
    FASTBOOL                    bSlant;

public:
    TYPEINFO();
    SdrDragShear(SdrDragView& rNewView,FASTBOOL bSlant1): SdrDragMethod(rNewView),
        aFact(1,1),nWink0(0),nWink(0),
        nTan(0.0),
        bVertical(FALSE),bResize(FALSE),bUpSideDown(FALSE),
        bSlant(bSlant1) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovPoint(Point& rPnt);
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
};

//************************************************************
//   SdrDragMirror
//************************************************************

class SdrDragMirror : public SdrDragMethod
{
    Point                       aDif;
    long                        nWink;
    bool                        bMirrored;
    bool                        bSide0;

private:
    FASTBOOL ImpCheckSide(const Point& rPnt) const;

public:
    TYPEINFO();
    SdrDragMirror(SdrDragView& rNewView): SdrDragMethod(rNewView),nWink(0),bMirrored(FALSE),bSide0(FALSE) { }

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovPoint(Point& rPnt);
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
};

//************************************************************
//   SdrDragGradient
//************************************************************

class SdrDragGradient : public SdrDragMethod
{
    // Handles to work on
    SdrHdlGradient*             pIAOHandle;

    // is this for gradient (or for transparence) ?
    unsigned                    bIsGradient : 1;

public:
    TYPEINFO();
    SdrDragGradient(SdrDragView& rNewView, BOOL bGrad = TRUE);

    BOOL IsGradient() const { return bIsGradient; }

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
    virtual void Brk();
};

//************************************************************
//   SdrDragCrook
//************************************************************

class SdrDragCrook : public SdrDragMethod
{
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

public:
    TYPEINFO();
    SdrDragCrook(SdrDragView& rNewView): SdrDragMethod(rNewView),aFact(1,1),
        bContortionAllowed(FALSE),bNoContortionAllowed(FALSE),bContortion(FALSE),
        bResizeAllowed(FALSE),bResize(FALSE),bRotateAllowed(FALSE),bRotate(FALSE),
        bVertical(FALSE),bValid(FALSE),bLft(FALSE),bRgt(FALSE),bUpr(FALSE),bLwr(FALSE),bAtCenter(FALSE),
        nWink(0),nMarkSize(0),eMode(SDRCROOK_ROTATE) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovAllPoints();
    void MovCrookPoint(Point& rPnt, Point* pC1, Point* pC2);
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
};

//************************************************************
//   SdrDragDistort
//************************************************************

class SdrDragDistort : public SdrDragMethod
{
    Rectangle                   aMarkRect;
    XPolygon                    aDistortedRect;
    USHORT                      nPolyPt;
    bool                        bContortionAllowed;
    bool                        bNoContortionAllowed;
    bool                        bContortion;

public:
    TYPEINFO();
    SdrDragDistort(SdrDragView& rNewView): SdrDragMethod(rNewView),nPolyPt(0),
        bContortionAllowed(FALSE),bNoContortionAllowed(FALSE),bContortion(FALSE) {}

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovAllPoints();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
};

//************************************************************
//   SdrDragCrop
//************************************************************

class SdrDragCrop : public SdrDragResize
{
public:
    TYPEINFO();
    SdrDragCrop(SdrDragView& rNewView);

    virtual void TakeComment(String& rStr) const;
    virtual FASTBOOL End(FASTBOOL bCopy);
    virtual Pointer GetPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDDRGM1_HXX

