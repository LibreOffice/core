/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdclient.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:44:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "Client.hxx"

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif

#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#include <toolkit/helper/vclunohelper.hxx>


#include "misc.hxx"

#ifdef STARIMAGE_AVAILABLE
#ifndef _SIMDLL_HXX
#include <sim2/simdll.hxx>
#endif
#endif

#include "strings.hrc"

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
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

