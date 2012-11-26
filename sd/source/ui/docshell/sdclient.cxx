/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "Client.hxx"
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include <toolkit/helper/vclunohelper.hxx>


#include "misc.hxx"

#ifdef STARIMAGE_AVAILABLE
#ifndef _SIMDLL_HXX
#include <sim2/simdll.hxx>
#endif
#endif

#include "strings.hrc"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include <vcl/svapp.hxx>
#include <svx/svdlegacy.hxx>

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

void Client::RequestNewObjectArea( Rectangle& rRectangle )
{
    basegfx::B2DRange aNewRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());
    ::sd::View* pView = mpViewShell->GetView();
    const SdrObject* pSelected = pView->getSelectedIfSingle();
    const bool bSizeProtect(pSelected ? pSelected->IsResizeProtect() : false);
    const bool bPosProtect(pSelected ? pSelected->IsMoveProtect() : false);
    const Rectangle aOldRectangle(GetObjArea());
    const basegfx::B2DRange aOldRange(aOldRectangle.Left(), aOldRectangle.Top(), aOldRectangle.Right(), aOldRectangle.Bottom());

    if ( bPosProtect )
    {
        aNewRange = basegfx::B2DRange(
            aOldRange.getMinimum(),
            aOldRange.getMinimum() + aNewRange.getRange());
    }

    if ( bSizeProtect )
    {
        aNewRange = basegfx::B2DRange(
            aNewRange.getMinimum(),
            aNewRange.getMinimum() + aOldRange.getRange());
    }

    const basegfx::B2DRange& rWorkArea(pView->GetWorkArea());

    if(!rWorkArea.isInside(aNewRange) && !bPosProtect && !aNewRange.equal(aOldRange) )
    {
        // correct position
        const basegfx::B2DRange aClampRange(rWorkArea.getMinimum(), rWorkArea.getMaximum() - aNewRange.getRange());
        const basegfx::B2DPoint aNewTopLeft(aClampRange.clamp(aNewRange.getMinimum()));

        if(!aNewTopLeft.equal(aNewRange.getMinimum()))
        {
            aNewRange = basegfx::B2DRange(aNewTopLeft, aNewTopLeft + aNewRange.getRange());
        }
    }

    rRectangle = Rectangle(
        basegfx::fround(aNewRange.getMinX()), basegfx::fround(aNewRange.getMinY()),
        basegfx::fround(aNewRange.getMaxX()), basegfx::fround(aNewRange.getMaxY()));
}

void Client::ObjectAreaChanged()
{
    ::sd::View* pView = mpViewShell->GetView();
    SdrOle2Obj* pSelected = dynamic_cast< SdrOle2Obj* >(pView->getSelectedIfSingle());

    if (pSelected)
    {
        // no need to check for changes, this method is called only if the area really changed
        const Rectangle aNewRectangle(GetScaledObjArea());
        basegfx::B2DRange aNewRange(aNewRectangle.Left(), aNewRectangle.Top(), aNewRectangle.Right(), aNewRectangle.Bottom());

        // #i118524# if sheared/rotated, center to non-rotated LogicRect
        pSelected->setSuppressSetVisAreaSize(true);

        if(pSelected->isRotatedOrSheared())
        {
            sdr::legacy::SetLogicRange(*pSelected, aNewRange);

            const basegfx::B2DRange& rObjectRange = pSelected->getObjectRange(pView);

            aNewRange.transform(
                basegfx::tools::createTranslateB2DHomMatrix(
                    aNewRange.getCenterX() - rObjectRange.getCenterX(),
                    aNewRange.getCenterY() - rObjectRange.getCenterY()));
        }

        sdr::legacy::SetLogicRange(*pSelected, aNewRange);
        pSelected->setSuppressSetVisAreaSize(false);
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

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
            const Rectangle aLogicRect( sdr::legacy::GetLogicRect(*pSdrOle2Obj) );
            Size aLogicSize( aLogicRect.GetWidth(), aLogicRect.GetHeight() );

            if( pSdrOle2Obj->IsChart() )
            {
                // should not be needed; SetLogicRect should already broadcast the change as needed
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pSdrOle2Obj);

                //charts never should be stretched see #i84323# for example
                sdr::legacy::SetLogicRect(*pSdrOle2Obj, Rectangle( aLogicRect.TopLeft(), aLogicSize ) );
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
                // should not be needed; SetLogicRect should already broadcast the change as needed
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pSdrOle2Obj);

                sdr::legacy::SetLogicRect(*pSdrOle2Obj, Rectangle( aLogicRect.TopLeft(), aScaledSize ) );
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
    DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

    if (pDrawViewShell)
    {
        pDrawViewShell->MakeVisibleAtView(
            sdr::legacy::GetLogicRange(*pSdrOle2Obj),
            *mpViewShell->GetActiveWindow());
    }
}

} // end of namespace sd

