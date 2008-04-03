/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: presvish.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:21:02 $
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

#include <com/sun/star/presentation/XSlideShowController.hpp>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "PresentationViewShell.hxx"

#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#ifndef _SDDLL_HXX
#include "sddll.hxx"
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX
#include <sfx2/topfrm.hxx>
#endif
#ifndef _SFX_DISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#include <sfx2/objface.hxx>

#include <svx/svxids.hrc>
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_FACTORY_IDS_HXX
#include "FactoryIds.hxx"
#endif

// #110496#
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
//    SwitchPage (nPageNumber);
    WriteFrameViewData();

    GetActiveWindow()->GrabFocus();
}


SvxRuler* PresentationViewShell::CreateHRuler(::sd::Window*, BOOL)
{
    return NULL;
}

SvxRuler* PresentationViewShell::CreateVRuler(::sd::Window*)
{
    return NULL;
}


void PresentationViewShell::Activate( BOOL bIsMDIActivate )
{
    DrawViewShell::Activate( bIsMDIActivate );

    if( bIsMDIActivate )
    {
        //HMH::sd::View*     pView = GetView();
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, TRUE );

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

} // end of namespace sd
