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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdhlpln.hxx>
#include <tools/color.hxx>

#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

Pointer SdrHelpLine::GetPointer() const
{
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  : return Pointer(POINTER_ESIZE);
        case SDRHELPLINE_HORIZONTAL: return Pointer(POINTER_SSIZE);
        default                    : return Pointer(POINTER_MOVE);
    } // switch
}

FASTBOOL SdrHelpLine::IsHit(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const
{
    Size a1Pix(rOut.PixelToLogic(Size(1,1)));
    FASTBOOL bXHit=rPnt.X()>=aPos.X()-nTolLog && rPnt.X()<=aPos.X()+nTolLog+a1Pix.Width();
    FASTBOOL bYHit=rPnt.Y()>=aPos.Y()-nTolLog && rPnt.Y()<=aPos.Y()+nTolLog+a1Pix.Height();
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  : return bXHit;
        case SDRHELPLINE_HORIZONTAL: return bYHit;
        case SDRHELPLINE_POINT: {
            if (bXHit || bYHit) {
                Size aRad(rOut.PixelToLogic(Size(SDRHELPLINE_POINT_PIXELSIZE,SDRHELPLINE_POINT_PIXELSIZE)));
                return rPnt.X()>=aPos.X()-aRad.Width() && rPnt.X()<=aPos.X()+aRad.Width()+a1Pix.Width() &&
                       rPnt.Y()>=aPos.Y()-aRad.Height() && rPnt.Y()<=aPos.Y()+aRad.Height()+a1Pix.Height();
            }
        } break;
    } // switch
    return sal_False;
}

Rectangle SdrHelpLine::GetBoundRect(const OutputDevice& rOut) const
{
    Rectangle aRet(aPos,aPos);
    Point aOfs(rOut.GetMapMode().GetOrigin());
    Size aSiz(rOut.GetOutputSize());
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  : aRet.Top()=-aOfs.Y(); aRet.Bottom()=-aOfs.Y()+aSiz.Height(); break;
        case SDRHELPLINE_HORIZONTAL: aRet.Left()=-aOfs.X(); aRet.Right()=-aOfs.X()+aSiz.Width();  break;
        case SDRHELPLINE_POINT     : {
            Size aRad(rOut.PixelToLogic(Size(SDRHELPLINE_POINT_PIXELSIZE,SDRHELPLINE_POINT_PIXELSIZE)));
            aRet.Left()  -=aRad.Width();
            aRet.Right() +=aRad.Width();
            aRet.Top()   -=aRad.Height();
            aRet.Bottom()+=aRad.Height();
        } break;
    } // switch
    return aRet;
}

bool SdrHelpLine::IsVisibleEqual( const SdrHelpLine& rHelpLine, const OutputDevice& rOut ) const
{
    if( eKind == rHelpLine.eKind)
    {
        Point aPt1(rOut.LogicToPixel(aPos)), aPt2(rOut.LogicToPixel(rHelpLine.aPos));
        switch( eKind )
        {
            case SDRHELPLINE_POINT:
                return aPt1 == aPt2;
            case SDRHELPLINE_VERTICAL:
                return aPt1.X() == aPt2.X();
            case SDRHELPLINE_HORIZONTAL:
                return aPt1.Y() == aPt2.Y();
        }
    }
    return false;
}

void SdrHelpLineList::Clear()
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

void SdrHelpLineList::operator=(const SdrHelpLineList& rSrcList)
{
    Clear();
    sal_uInt16 nAnz=rSrcList.GetCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        Insert(rSrcList[i]);
    }
}

bool SdrHelpLineList::operator==(const SdrHelpLineList& rSrcList) const
{
    FASTBOOL bEqual=sal_False;
    sal_uInt16 nAnz=GetCount();
    if (nAnz==rSrcList.GetCount()) {
        bEqual=sal_True;
        for (sal_uInt16 i=0; i<nAnz && bEqual; i++) {
            if (*GetObject(i)!=*rSrcList.GetObject(i)) {
                bEqual=sal_False;
            }
        }
    }
    return bEqual;
}

sal_uInt16 SdrHelpLineList::HitTest(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 i=nAnz; i>0;) {
        i--;
        if (GetObject(i)->IsHit(rPnt,nTolLog,rOut)) return i;
    }
    return SDRHELPLINE_NOTFOUND;
}

// eof
