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

#include <com/sun/star/presentation/XSlideShowController.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>

#include "PresentationViewShell.hxx"
#include "optsitem.hxx"
#include "sddll.hxx"
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <sfx2/objface.hxx>

#include <svx/svxids.hrc>
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "slideshow.hxx"
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#include "ViewShellBase.hxx"
#include "FactoryIds.hxx"

#include "slideshow.hxx"
#include "fupoor.hxx"
#include "Window.hxx"

#define PresentationViewShell
using namespace sd;
#include "sdslots.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::presentation;

namespace sd {

// -------------------
// - PresentationViewShell -
// -------------------

SFX_IMPL_INTERFACE( PresentationViewShell, DrawViewShell, SdResId( STR_PRESVIEWSHELL ) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER | SFX_VISIBILITY_READONLYDOC,
                                SdResId(RID_DRAW_VIEWER_TOOLBOX) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OPTIONS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_OPTIONS_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_COMMONTASK | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_COMMONTASK_TOOLBOX));
}


TYPEINIT1( PresentationViewShell, DrawViewShell );

PresentationViewShell::PresentationViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, ::Window* pParentWindow, FrameView* pFrameView)
: DrawViewShell( pFrame, rViewShellBase, pParentWindow, PK_STANDARD, pFrameView)
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        maOldVisArea = GetDocSh()->GetVisArea( ASPECT_CONTENT );
    meShellType = ST_PRESENTATION;
}

PresentationViewShell::~PresentationViewShell (void)
{
    if( GetDocSh() && GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED && !maOldVisArea.IsEmpty() )
        GetDocSh()->SetVisArea( maOldVisArea );
}

void PresentationViewShell::FinishInitialization( FrameView* pFrameView )
{
    DrawViewShell::Init(true);

    // Use the frame view that comes form the view shell that initiated our
    // creation.
    if (pFrameView != NULL)
    {
        GetFrameView()->Disconnect();
        SetFrameView (pFrameView);
        pFrameView->Connect();
    }
    SetRuler(false);
    WriteFrameViewData();

    GetActiveWindow()->GrabFocus();
}


SvxRuler* PresentationViewShell::CreateHRuler(::sd::Window*, sal_Bool)
{
    return NULL;
}

SvxRuler* PresentationViewShell::CreateVRuler(::sd::Window*)
{
    return NULL;
}


void PresentationViewShell::Activate( sal_Bool bIsMDIActivate )
{
    DrawViewShell::Activate( bIsMDIActivate );

    if( bIsMDIActivate )
    {
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, sal_True );

        GetViewFrame()->GetDispatcher()->Execute( SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
        if( xSlideShow.is() )
            xSlideShow->activate(GetViewShellBase());

        if( HasCurrentFunction() )
            GetCurrentFunction()->Activate();
    }

    if( bIsMDIActivate )
        ReadFrameViewData( mpFrameView );
    GetDocSh()->Connect( this );
}

void PresentationViewShell::Paint( const Rectangle& rRect, ::sd::Window* )
{
    rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( GetViewShellBase() ) );
    if( xSlideShow.is() )
        xSlideShow->paint(rRect);
}

void PresentationViewShell::Resize (void)
{
    ViewShell::Resize(); // do not call DrawViewShell here!

    rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetViewShellBase() ) );
    if( xSlideshow.is() )
        xSlideshow->resize(maViewSize);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
