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


#include <svx/svdhlpln.hxx>

#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>



Pointer SdrHelpLine::GetPointer() const
{
    switch (eKind) {
        case SDRHELPLINE_VERTICAL  : return Pointer(POINTER_ESIZE);
        case SDRHELPLINE_HORIZONTAL: return Pointer(POINTER_SSIZE);
        default                    : return Pointer(POINTER_MOVE);
    } // switch
}

bool SdrHelpLine::IsHit(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const
{
    Size a1Pix(rOut.PixelToLogic(Size(1,1)));
    bool bXHit=rPnt.X()>=aPos.X()-nTolLog && rPnt.X()<=aPos.X()+nTolLog+a1Pix.Width();
    bool bYHit=rPnt.Y()>=aPos.Y()-nTolLog && rPnt.Y()<=aPos.Y()+nTolLog+a1Pix.Height();
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
    return false;
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

void SdrHelpLineList::Clear()
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.clear();
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
    bool bEqual = false;
    sal_uInt16 nAnz=GetCount();
    if (nAnz==rSrcList.GetCount()) {
        bEqual = true;
        for (sal_uInt16 i=0; i<nAnz && bEqual; i++) {
            if (*GetObject(i)!=*rSrcList.GetObject(i)) {
                bEqual = false;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
