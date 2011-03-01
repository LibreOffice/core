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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

Client::~Client()
{
}


/*************************************************************************
|*
|* Wenn IP-aktiv, dann kommt diese Anforderung um Vergroesserung des
|* sichtbaren Ausschnitts des Objektes
|*
\************************************************************************/

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

        // no need to check for changes, this method is called only if the area really changed
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
        // correct position
        Point aPos = aObjRect.TopLeft();
        Size  aSize = aObjRect.GetSize();
        Point aWorkAreaTL = aWorkArea.TopLeft();
        Point aWorkAreaBR = aWorkArea.BottomRight();

        aPos.X() = Max(aPos.X(), aWorkAreaTL.X());
        aPos.X() = Min(aPos.X(), aWorkAreaBR.X()-aSize.Width());
        aPos.Y() = Max(aPos.Y(), aWorkAreaTL.Y());
        aPos.Y() = Min(aPos.Y(), aWorkAreaBR.Y()-aSize.Height());

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
        SdrObject* pObj = pMark->GetMarkedSdrObj();

        // no need to check for changes, this method is called only if the area really changed
        pObj->SetLogicRect( GetScaledObjArea() );
    }
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
    if (mpViewShell->GetActiveWindow())
    {
        ::sd::View* pView = mpViewShell->GetView();
        if (pView)
        {
            Rectangle aLogicRect( pSdrOle2Obj->GetLogicRect() );
            Size aLogicSize( aLogicRect.GetWidth(), aLogicRect.GetHeight() );

            if( pSdrOle2Obj->IsChart() )
            {
                //charts never should be stretched see #i84323# for example
                pSdrOle2Obj->SetLogicRect( Rectangle( aLogicRect.TopLeft(), aLogicSize ) );
                pSdrOle2Obj->BroadcastObjectChange();
                return;
            }

            // TODO/LEAN: maybe we can do this without requesting the VisualArea?
            // working with the visual area might need running state, so the object may switch itself to this state
            MapMode             aMap100( MAP_100TH_MM );
            Rectangle           aVisArea;
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
                pSdrOle2Obj->SetLogicRect( Rectangle( aLogicRect.TopLeft(), aScaledSize ) );
                pSdrOle2Obj->BroadcastObjectChange();
            }
            else
                pSdrOle2Obj->ActionChanged();
        }
    }
}


/*************************************************************************
|*
|* Objekt in den sichtbaren Breich scrollen
|*
\************************************************************************/

void Client::MakeVisible()
{
    if (mpViewShell->ISA(DrawViewShell))
    {
        static_cast<DrawViewShell*>(mpViewShell)->MakeVisible(
            pSdrOle2Obj->GetLogicRect(),
            *mpViewShell->GetActiveWindow());
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
