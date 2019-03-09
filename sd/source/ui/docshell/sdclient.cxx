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

#include <Client.hxx>
#include <svx/svdoole2.hxx>

#include <ViewShell.hxx>
#include <View.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

namespace sd {

Client::Client(SdrOle2Obj* pObj, ViewShell* pViewShell, vcl::Window* pWindow) :
    SfxInPlaceClient(pViewShell->GetViewShell(), pWindow, pObj->GetAspect() ),
    mpViewShell(pViewShell),
    pSdrOle2Obj(pObj)
{
    SetObject( pObj->GetObjRef() );
    DBG_ASSERT( GetObject().is(), "No object connected!" );
}

Client::~Client()
{
}

/**
 * If IP active, then we get this request to increase the visible section of the
 * object.
 */
void Client::RequestNewObjectArea( ::tools::Rectangle& aObjRect )
{
    ::sd::View* pView = mpViewShell->GetView();

    bool bSizeProtect = false;
    bool bPosProtect = false;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        SdrObject* pObj = pMark->GetMarkedSdrObj();

        // no need to check for changes, this method is called only if the area really changed
        bSizeProtect = pObj->IsResizeProtect();
        bPosProtect = pObj->IsMoveProtect();
    }

    ::tools::Rectangle aOldRect = GetObjArea();
    if ( bPosProtect )
        aObjRect.SetPos( aOldRect.TopLeft() );

    if ( bSizeProtect )
        aObjRect.SetSize( aOldRect.GetSize() );

    ::tools::Rectangle aWorkArea( pView->GetWorkArea() );
    if ( aWorkArea.IsInside(aObjRect) || bPosProtect || aObjRect == aOldRect )
        return;

    // correct position
    Point aPos = aObjRect.TopLeft();
    Size  aSize = aObjRect.GetSize();
    Point aWorkAreaTL = aWorkArea.TopLeft();
    Point aWorkAreaBR = aWorkArea.BottomRight();

    aPos.setX( std::max(aPos.X(), aWorkAreaTL.X()) );
    aPos.setX( std::min(aPos.X(), aWorkAreaBR.X()-aSize.Width()) );
    aPos.setY( std::max(aPos.Y(), aWorkAreaTL.Y()) );
    aPos.setY( std::min(aPos.Y(), aWorkAreaBR.Y()-aSize.Height()) );

    aObjRect.SetPos(aPos);
}

void Client::ObjectAreaChanged()
{
    ::sd::View* pView = mpViewShell->GetView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() != 1)
        return;

    SdrMark* pMark = rMarkList.GetMark(0);
    SdrOle2Obj* pObj = dynamic_cast< SdrOle2Obj* >(pMark->GetMarkedSdrObj());

    if(!pObj)
        return;

    // no need to check for changes, this method is called only if the area really changed
    ::tools::Rectangle aNewRectangle(GetScaledObjArea());

    // #i118524# if sheared/rotated, center to non-rotated LogicRect
    pObj->setSuppressSetVisAreaSize(true);

    if(pObj->GetGeoStat().nRotationAngle || pObj->GetGeoStat().nShearAngle)
    {
        pObj->SetLogicRect( aNewRectangle );

        const ::tools::Rectangle& rBoundRect = pObj->GetCurrentBoundRect();
        const Point aDelta(aNewRectangle.Center() - rBoundRect.Center());

        aNewRectangle.Move(aDelta.X(), aDelta.Y());
    }

    pObj->SetLogicRect( aNewRectangle );
    pObj->setSuppressSetVisAreaSize(false);
}

void Client::ViewChanged()
{
    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
    {
        // the iconified object seems not to need such a scaling handling
        // since the replacement image and the size a completely controlled by the container
        // TODO/LATER: when the icon exchange is implemented the scaling handling might be required again here

        pSdrOle2Obj->ActionChanged(); // draw needs it to remove lines in slide preview
        return;
    }

    //TODO/LATER: should we try to avoid the recalculation of the visareasize
    //if we know that it didn't change?
    if (!mpViewShell->GetActiveWindow())
        return;

    ::sd::View* pView = mpViewShell->GetView();
    if (!pView)
        return;

    ::tools::Rectangle aLogicRect( pSdrOle2Obj->GetLogicRect() );
    Size aLogicSize( aLogicRect.GetWidth(), aLogicRect.GetHeight() );

    if( pSdrOle2Obj->IsChart() )
    {
        //charts never should be stretched see #i84323# for example
        pSdrOle2Obj->SetLogicRect( ::tools::Rectangle( aLogicRect.TopLeft(), aLogicSize ) );
        pSdrOle2Obj->BroadcastObjectChange();
        return;
    }

    // TODO/LEAN: maybe we can do this without requesting the VisualArea?
    // working with the visual area might need running state, so the object may switch itself to this state
    MapMode             aMap100( MapUnit::Map100thMM );
    ::tools::Rectangle           aVisArea;
    Size aSize = pSdrOle2Obj->GetOrigObjSize( &aMap100 );

    aVisArea.SetSize( aSize );
    Size                aScaledSize( static_cast< long >( GetScaleWidth() * Fraction( aVisArea.GetWidth() ) ),
                                        static_cast< long >( GetScaleHeight() * Fraction( aVisArea.GetHeight() ) ) );

    // react to the change if the difference is bigger than one pixel
    Size aPixelDiff =
        Application::GetDefaultDevice()->LogicToPixel(
            Size( aLogicRect.GetWidth() - aScaledSize.Width(),
                  aLogicRect.GetHeight() - aScaledSize.Height() ),
            aMap100 );
    if( aPixelDiff.Width() || aPixelDiff.Height() )
    {
        pSdrOle2Obj->SetLogicRect( ::tools::Rectangle( aLogicRect.TopLeft(), aScaledSize ) );
        pSdrOle2Obj->BroadcastObjectChange();
    }
    else
        pSdrOle2Obj->ActionChanged();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
