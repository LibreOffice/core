/*************************************************************************
 *
 *  $RCSfile: fuscale.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:11:56 $
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

#pragma hdrstop

#include <svx/dialogs.hrc>

#include "app.hrc"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "slidvish.hxx"
#include "outlnvsh.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "drviewsh.hxx"
#include "viewshel.hxx"
#include "fuscale.hxx"
#include "fuzoom.hxx" // wegen SidArrayZoom[]

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_ZOOM_HXX //autogen
#include <svx/zoom.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

TYPEINIT1( FuScale, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuScale::FuScale(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq)
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    INT16 nValue;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aNewAttr( pDoc->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM );
        SvxZoomItem* pZoomItem;
        USHORT nZoomValues = SVX_ZOOM_ENABLE_ALL;

        nValue = (INT16) pWindow->GetZoom();

        // Zoom auf Seitengroesse ?
        if( pViewSh->ISA( SdDrawViewShell ) &&
            ( (SdDrawViewShell*)pViewSh )->IsZoomOnPage() )
        {
            pZoomItem = new SvxZoomItem( SVX_ZOOM_WHOLEPAGE, nValue );
        }
        else
        {
            pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nValue );
        }

        // Bereich einschraenken
        if( pViewSh->ISA( SdDrawViewShell ) )
        {
            SdrPageView* pPageView = pView->GetPageViewPvNum( 0 );
            if( ( pPageView && pPageView->GetObjList()->GetObjCount() == 0 ) )
                // || ( pView->GetMarkList().GetMarkCount() == 0 ) )
            {
                nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
            }
        }
        else if( pViewSh->ISA( SdOutlineViewShell ) )
        {
            nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
            nZoomValues &= ~SVX_ZOOM_ENABLE_WHOLEPAGE;
            nZoomValues &= ~SVX_ZOOM_ENABLE_PAGEWIDTH;
        }
        else if( pViewSh->ISA( SdSlideViewShell ) )
        {
            nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
            nZoomValues &= ~SVX_ZOOM_ENABLE_PAGEWIDTH;
            nZoomValues &= ~(SVX_ZOOM_ENABLE_100|SVX_ZOOM_ENABLE_150|SVX_ZOOM_ENABLE_200);
        }

        pZoomItem->SetValueSet( nZoomValues );
        aNewAttr.Put( *pZoomItem );

        SvxZoomDialog* pDlg = new SvxZoomDialog( NULL, aNewAttr );
        pDlg->SetLimits( pWin->GetMinZoom(), pWin->GetMaxZoom() );
        USHORT nResult = pDlg->Execute();

        switch( nResult )
        {
            case RET_CANCEL:
            {
                delete pDlg;
                delete pZoomItem;
                rReq.Ignore ();
                return; // Abbruch
            }

            default:
            {
                rReq.Ignore ();
/*
                rReq.Done( *( pDlg->GetOutputItemSet() ) );
                pArgs = rReq.GetArgs();*/
            }
            break;
        }

        const SfxItemSet aArgs (*(pDlg->GetOutputItemSet ()));

        delete pDlg;
        delete pZoomItem;

//      SvxZoomType eZT = ((const SvxZoomItem &) aArgs.Get (RID_SVXDLG_ZOOM)).GetType ();
        switch (((const SvxZoomItem &) aArgs.Get (RID_SVXDLG_ZOOM)).GetType ())
        {
            case SVX_ZOOM_PERCENT:
            {
                nValue = ((const SvxZoomItem &) aArgs.Get (RID_SVXDLG_ZOOM)).GetValue ();

                pViewShell->SetZoom( nValue );

                pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
            }
            break;

            case SVX_ZOOM_OPTIMAL:
            {
                if( pViewShell->ISA( SdDrawViewShell ) )
                {
                    // Namensverwirrung: SID_SIZE_ALL -> Zoom auf alle Objekte
                    // --> Wird als Optimal im Programm angeboten
                    pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                }
                else if( pViewShell->ISA( SdSlideViewShell ) )
                    pViewShell->SetZoom( 20 );
                    // Hier sollte sich noch etwas besseres ueberlegt werden !!!
                // ???!!
                /*
                    aNewAttr.Put( SvxZoomItem( SVX_ZOOM_PERCENT, 20 ) );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
                 */
            }
            break;

            case SVX_ZOOM_PAGEWIDTH:
                pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                break;

            case SVX_ZOOM_WHOLEPAGE:
                pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                break;
        }
    }
    else if (pArgs->Count () == 1)
    {
        SFX_REQUEST_ARG (rReq, pScale, SfxUInt32Item, ID_VAL_ZOOM, FALSE);
        pViewShell->SetZoom (pScale->GetValue ());

        pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
    }

}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuScale::~FuScale()
{
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuScale::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuScale::Deactivate()
{
    FuPoor::Deactivate();
}



