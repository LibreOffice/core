/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "Client.hxx"
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include "strings.hrc"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include <vcl/svapp.hxx>

using namespace com::sun::star;

namespace sd {


Client::Client(SdrOle2Obj* pObj, ViewShell* pViewShell, ::Window* pWindow) :
    SfxInPlaceClient(pViewShell->GetViewShell(), pWindow, pObj->GetAspect() ),
    mpViewShell(pViewShell),
    pSdrOle2Obj(pObj),
    pSdrGrafObj(NULL),
    pOutlinerParaObj (NULL)
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
void Client::RequestNewObjectArea( Rectangle& aObjRect )
{
    ::sd::View* pView = mpViewShell->GetView();

    sal_Bool bSizeProtect = sal_False;
    sal_Bool bPosProtect = sal_False;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        SdrObject* pObj = pMark->GetMarkedSdrObj();

        
        bSizeProtect = pObj->IsResizeProtect();
        bPosProtect = pObj->IsMoveProtect();
    }

    Rectangle aOldRect = GetObjArea();
    if ( bPosProtect )
        aObjRect.SetPos( aOldRect.TopLeft() );

    if ( bSizeProtect )
        aObjRect.SetSize( aOldRect.GetSize() );

    Rectangle aWorkArea( pView->GetWorkArea() );
    if ( !aWorkArea.IsInside(aObjRect) && !bPosProtect && aObjRect != aOldRect )
    {
        
        Point aPos = aObjRect.TopLeft();
        Size  aSize = aObjRect.GetSize();
        Point aWorkAreaTL = aWorkArea.TopLeft();
        Point aWorkAreaBR = aWorkArea.BottomRight();

        aPos.X() = std::max(aPos.X(), aWorkAreaTL.X());
        aPos.X() = std::min(aPos.X(), aWorkAreaBR.X()-aSize.Width());
        aPos.Y() = std::max(aPos.Y(), aWorkAreaTL.Y());
        aPos.Y() = std::min(aPos.Y(), aWorkAreaBR.Y()-aSize.Height());

        aObjRect.SetPos(aPos);
    }
}

void Client::ObjectAreaChanged()
{
    ::sd::View* pView = mpViewShell->GetView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrMark* pMark = rMarkList.GetMark(0);
		SdrOle2Obj* pObj = dynamic_cast< SdrOle2Obj* >(pMark->GetMarkedSdrObj());

        if(pObj)
        {
            
            Rectangle aNewRectangle(GetScaledObjArea());

            
            pObj->setSuppressSetVisAreaSize(true);

            if(pObj->GetGeoStat().nDrehWink || pObj->GetGeoStat().nShearWink)
            {
                pObj->SetLogicRect( aNewRectangle );

                const Rectangle& rBoundRect = pObj->GetCurrentBoundRect();
                const Point aDelta(aNewRectangle.Center() - rBoundRect.Center());

                aNewRectangle.Move(aDelta.X(), aDelta.Y());
            }

            pObj->SetLogicRect( aNewRectangle );
            pObj->setSuppressSetVisAreaSize(false);
        }
    }
}

void Client::ViewChanged()
{
    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
    {
        
        
        

        pSdrOle2Obj->ActionChanged(); 
        return;
    }

    
    
    if (mpViewShell->GetActiveWindow())
    {
        ::sd::View* pView = mpViewShell->GetView();
        if (pView)
        {
            Rectangle aLogicRect( pSdrOle2Obj->GetLogicRect() );
            Size aLogicSize( aLogicRect.GetWidth(), aLogicRect.GetHeight() );

            if( pSdrOle2Obj->IsChart() )
            {
                
                pSdrOle2Obj->SetLogicRect( Rectangle( aLogicRect.TopLeft(), aLogicSize ) );
                pSdrOle2Obj->BroadcastObjectChange();
                return;
            }

            
            
            MapMode             aMap100( MAP_100TH_MM );
            Rectangle           aVisArea;
            Size aSize = pSdrOle2Obj->GetOrigObjSize( &aMap100 );

            aVisArea.SetSize( aSize );
            Size                aScaledSize( static_cast< long >( GetScaleWidth() * Fraction( aVisArea.GetWidth() ) ),
                                                static_cast< long >( GetScaleHeight() * Fraction( aVisArea.GetHeight() ) ) );

            
            Size aPixelDiff =
                Application::GetDefaultDevice()->LogicToPixel(
                    Size( aLogicRect.GetWidth() - aScaledSize.Width(),
                          aLogicRect.GetHeight() - aScaledSize.Height() ),
                    aMap100 );
            if( aPixelDiff.Width() || aPixelDiff.Height() )
            {
                pSdrOle2Obj->SetLogicRect( Rectangle( aLogicRect.TopLeft(), aScaledSize ) );
                pSdrOle2Obj->BroadcastObjectChange();
            }
            else
                pSdrOle2Obj->ActionChanged();
        }
    }
}


/**
 * Scroll object into visible area.
 */
void Client::MakeVisible()
{
    if (mpViewShell->ISA(DrawViewShell))
    {
        static_cast<DrawViewShell*>(mpViewShell)->MakeVisible(
            pSdrOle2Obj->GetLogicRect(),
            *mpViewShell->GetActiveWindow());
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
