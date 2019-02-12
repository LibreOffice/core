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
#include <vcl/ptrstyle.hxx>


PointerStyle SdrHelpLine::GetPointer() const
{
    switch (eKind) {
        case SdrHelpLineKind::Vertical  : return PointerStyle::ESize;
        case SdrHelpLineKind::Horizontal: return PointerStyle::SSize;
        default                    : return PointerStyle::Move;
    } // switch
}

bool SdrHelpLine::IsHit(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const
{
    Size a1Pix(rOut.PixelToLogic(Size(1,1)));
    bool bXHit=rPnt.X()>=aPos.X()-nTolLog && rPnt.X()<=aPos.X()+nTolLog+a1Pix.Width();
    bool bYHit=rPnt.Y()>=aPos.Y()-nTolLog && rPnt.Y()<=aPos.Y()+nTolLog+a1Pix.Height();
    switch (eKind) {
        case SdrHelpLineKind::Vertical  : return bXHit;
        case SdrHelpLineKind::Horizontal: return bYHit;
        case SdrHelpLineKind::Point: {
            if (bXHit || bYHit) {
                Size aRad(rOut.PixelToLogic(Size(SDRHELPLINE_POINT_PIXELSIZE,SDRHELPLINE_POINT_PIXELSIZE)));
                return rPnt.X()>=aPos.X()-aRad.Width() && rPnt.X()<=aPos.X()+aRad.Width()+a1Pix.Width() &&
                       rPnt.Y()>=aPos.Y()-aRad.Height() && rPnt.Y()<=aPos.Y()+aRad.Height()+a1Pix.Height();
            }
        } break;
    } // switch
    return false;
}

tools::Rectangle SdrHelpLine::GetBoundRect(const OutputDevice& rOut) const
{
    tools::Rectangle aRet(aPos,aPos);
    Point aOfs(rOut.GetMapMode().GetOrigin());
    Size aSiz(rOut.GetOutputSize());
    switch (eKind) {
        case SdrHelpLineKind::Vertical  : aRet.SetTop(-aOfs.Y() ); aRet.SetBottom(-aOfs.Y()+aSiz.Height() ); break;
        case SdrHelpLineKind::Horizontal: aRet.SetLeft(-aOfs.X() ); aRet.SetRight(-aOfs.X()+aSiz.Width() );  break;
        case SdrHelpLineKind::Point     : {
            Size aRad(rOut.PixelToLogic(Size(SDRHELPLINE_POINT_PIXELSIZE,SDRHELPLINE_POINT_PIXELSIZE)));
            aRet.AdjustLeft( -(aRad.Width()) );
            aRet.AdjustRight(aRad.Width() );
            aRet.AdjustTop( -(aRad.Height()) );
            aRet.AdjustBottom(aRad.Height() );
        } break;
    } // switch
    return aRet;
}

void SdrHelpLineList::Clear()
{
    aList.clear();
}

SdrHelpLineList& SdrHelpLineList::operator=(const SdrHelpLineList& rSrcList)
{
    Clear();
    sal_uInt16 nCount=rSrcList.GetCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        Insert(rSrcList[i]);
    }
    return *this;
}

bool SdrHelpLineList::operator==(const SdrHelpLineList& rSrcList) const
{
    bool bEqual = false;
    sal_uInt16 nCount=GetCount();
    if (nCount==rSrcList.GetCount()) {
        bEqual = true;
        for (sal_uInt16 i=0; i<nCount && bEqual; i++) {
            if (*aList[i]!=*rSrcList.aList[i]) {
                bEqual = false;
            }
        }
    }
    return bEqual;
}

sal_uInt16 SdrHelpLineList::HitTest(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 i=nCount; i>0;) {
        i--;
        if (aList[i]->IsHit(rPnt,nTolLog,rOut)) return i;
    }
    return SDRHELPLINE_NOTFOUND;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
