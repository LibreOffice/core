/*************************************************************************
 *
 *  $RCSfile: sdclient.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 16:17:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "Client.hxx"

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

#pragma hdrstop

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

using namespace com::sun::star;

namespace sd {

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

Client::Client(SdrOle2Obj* pObj, ViewShell* pViewShell, ::Window* pWindow) :
    SfxInPlaceClient(pViewShell->GetViewShell(), pWindow, pObj->GetAspect() ),
    pViewShell(pViewShell),
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
    ::sd::View* pView = pViewShell->GetView();
    Rectangle aWorkArea( pView->GetWorkArea() );
    if (!aWorkArea.IsInside(aObjRect))
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
    ::sd::View* pView = pViewShell->GetView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        SdrObject* pObj = pMark->GetObj();

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
    //TODO/LATER: should we try to avoid the recalculation of the visareasize
    //if we know that it didn't change?
    if (pViewShell->GetActiveWindow())
    {
        ::sd::View* pView = pViewShell->GetView();
        if (pView)
        {
            // TODO/LEAN: maybe we can do this without requesting the VisualArea?
            // working with the visual area might need running state
            svt::EmbeddedObjectRef::TryRunningState( GetObject() );
            MapMode             aMap100( MAP_100TH_MM );
            MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( GetObject()->getMapUnit( GetAspect() ) );
            Rectangle           aVisArea;
            awt::Size aSz = GetObject()->getVisualAreaSize( GetAspect() );
            aVisArea.SetSize( Size( aSz.Width, aSz.Height ) );
            aVisArea = OutputDevice::LogicToLogic( aVisArea, aMapUnit, aMap100 );
            Rectangle           aLogicRect( pSdrOle2Obj->GetLogicRect() );
            Size                aScaledSize( static_cast< long >( GetScaleWidth() * Fraction( aVisArea.GetWidth() ) ),
                                                static_cast< long >( GetScaleHeight() * Fraction( aVisArea.GetHeight() ) ) );
            if( Application::GetDefaultDevice()->LogicToPixel( aScaledSize, aMap100 ) !=
                Application::GetDefaultDevice()->LogicToPixel( aLogicRect.GetSize(), aMap100 ) )
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
    if (pViewShell->ISA(DrawViewShell))
    {
        static_cast<DrawViewShell*>(pViewShell)->MakeVisible(
            pSdrOle2Obj->GetLogicRect(),
            *pViewShell->GetActiveWindow());
    }
}

} // end of namespace sd

