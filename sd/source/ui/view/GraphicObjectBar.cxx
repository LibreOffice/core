/*************************************************************************
 *
 *  $RCSfile: GraphicObjectBar.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:13:50 $
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

#include "GraphicObjectBar.hxx"

#include <limits.h>
#include <vcl/msgbox.hxx>
#include <svtools/whiter.hxx>
#include <svtools/itempool.hxx>
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVX_GRFFLT_HXX //autogen
#include <svx/grfflt.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVX_GRAFCTRL_HXX
#include <svx/grafctrl.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "DrawDocShell.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "sdresid.hxx"
#include "drawdoc.hxx"

using namespace sd;
#define GraphicObjectBar
#include "sdslots.hxx"

namespace sd {


SFX_DECL_TYPE( 13 );


// -----------------------
// - GraphicObjectBar -
// -----------------------

SFX_IMPL_INTERFACE( GraphicObjectBar, SfxShell, SdResId( STR_GRAFOBJECTBARSHELL ) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SdResId( RID_DRAW_GRAF_TOOLBOX ) );
}

// -----------------------------------------------------------------------------

TYPEINIT1( GraphicObjectBar, SfxShell );

// -----------------------------------------------------------------------------

GraphicObjectBar::GraphicObjectBar (
    ViewShell* pSdViewShell,
    ::sd::View* pSdView )
    : SfxShell (pSdViewShell->GetViewShell()),
      pView  ( pSdView ),
      pViewSh ( pSdViewShell ),
      nMappedSlotFilter ( SID_GRFFILTER_INVERT )
{
    DrawDocShell* pDocShell = pViewSh->GetDocSh();

    SetPool( &pDocShell->GetPool() );
    SetUndoManager( pDocShell->GetUndoManager() );
    SetRepeatTarget( pView );
    SetHelpId( SD_IF_SDDRAWGRAFOBJECTBAR );
    SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Graphic objectbar" )));
}

// -----------------------------------------------------------------------------

GraphicObjectBar::~GraphicObjectBar()
{
    SetRepeatTarget( NULL );
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::GetAttrState( SfxItemSet& rSet )
{
    if( pView )
        SvxGrafAttrHelper::GetGrafAttrState( rSet, *pView );
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::Execute( SfxRequest& rReq )
{
    if( pView )
    {
        SvxGrafAttrHelper::ExecuteGrafAttr( rReq, *pView );
        Invalidate();
    }
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::GetFilterState( SfxItemSet& rSet )
{
    const SdrMarkList&  rMarkList = pView->GetMarkedObjectList();
    BOOL                bEnable = FALSE;

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

        if( pObj && pObj->ISA( SdrGrafObj ) && ( ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP ) )
            bEnable = TRUE;
    }

    if( !bEnable )
        SvxGraphicFilter::DisableGraphicFilterSlots( rSet );
}

// -----------------------------------------------------------------------------

void GraphicObjectBar::ExecuteFilter( SfxRequest& rReq )
{
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

        if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
        {
            GraphicObject aFilterObj( ( (SdrGrafObj*) pObj )->GetGraphicObject() );

            if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
                SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ) )
            {
                SdrPageView* pPageView = pView->GetPageViewPvNum( 0 );

                if( pPageView )
                {
                    SdrGrafObj* pFilteredObj = (SdrGrafObj*) pObj->Clone();
                    String      aStr( pView->GetDescriptionOfMarkedObjects() );

                    aStr.Append( sal_Unicode(' ') );
                    aStr.Append( String( SdResId( STR_UNDO_GRAFFILTER ) ) );
                    pView->BegUndo( aStr );
                    pFilteredObj->SetGraphicObject( aFilterObj );
                    pView->ReplaceObject( pObj, *pPageView, pFilteredObj );
                    pView->EndUndo();
                }
            }
        }
    }

    Invalidate();
}

} // end of namespace sd
