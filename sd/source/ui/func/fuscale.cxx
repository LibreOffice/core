/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuscale.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:48:52 $
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


#include "fuscale.hxx"

#include <svx/dialogs.hrc>

#include "app.hrc"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_FU_ZOOM_HXX
#include "fuzoom.hxx" // wegen SidArrayZoom[]
#endif

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
#ifndef _SVX_ZOOM_DEF_HXX //autogen
#include <svx/zoom_def.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

namespace sd {

TYPEINIT1( FuScale, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuScale::FuScale (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuScale::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuScale( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuScale::DoExecute( SfxRequest& rReq )
{
    INT16 nValue;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aNewAttr( mpDoc->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM );
        SvxZoomItem* pZoomItem;
        USHORT nZoomValues = SVX_ZOOM_ENABLE_ALL;

        nValue = (INT16) mpWindow->GetZoom();

        // Zoom auf Seitengroesse ?
        if( mpViewShell && mpViewShell->ISA( DrawViewShell ) &&
            static_cast<DrawViewShell*>(mpViewShell)->IsZoomOnPage() )
        {
            pZoomItem = new SvxZoomItem( SVX_ZOOM_WHOLEPAGE, nValue );
        }
        else
        {
            pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nValue );
        }

        // Bereich einschraenken
        if( mpViewShell )
        {
            if( mpViewShell->ISA( DrawViewShell ) )
            {
                SdrPageView* pPageView = mpView->GetSdrPageView();
                if( ( pPageView && pPageView->GetObjList()->GetObjCount() == 0 ) )
                    // || ( mpView->GetMarkedObjectList().GetMarkCount() == 0 ) )
                {
                    nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
                }
            }
            else if( mpViewShell->ISA( OutlineViewShell ) )
            {
                nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
                nZoomValues &= ~SVX_ZOOM_ENABLE_WHOLEPAGE;
                nZoomValues &= ~SVX_ZOOM_ENABLE_PAGEWIDTH;
            }
        }

        pZoomItem->SetValueSet( nZoomValues );
        aNewAttr.Put( *pZoomItem );

        AbstractSvxZoomDialog* pDlg=NULL;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            pDlg = pFact->CreateSvxZoomDialog(NULL, aNewAttr, RID_SVXDLG_ZOOM);
        }

        if( pDlg )
        {
            pDlg->SetLimits( (USHORT)mpWindow->GetMinZoom(), (USHORT)mpWindow->GetMaxZoom() );
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

    //      SvxZoomType eZT = ((const SvxZoomItem &) aArgs.Get (RID_SVXDLG_ZOOM)).GetType ();
            switch (((const SvxZoomItem &) aArgs.Get (RID_SVXDLG_ZOOM)).GetType ())
            {
                case SVX_ZOOM_PERCENT:
                {
                    nValue = ((const SvxZoomItem &) aArgs.Get (RID_SVXDLG_ZOOM)).GetValue ();

                    mpViewShell->SetZoom( nValue );

                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
                }
                break;

                case SVX_ZOOM_OPTIMAL:
                {
                    if( mpViewShell->ISA( DrawViewShell ) )
                    {
                        // Namensverwirrung: SID_SIZE_ALL -> Zoom auf alle Objekte
                        // --> Wird als Optimal im Programm angeboten
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                    }
                }
                break;

                case SVX_ZOOM_PAGEWIDTH:
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                    break;

                case SVX_ZOOM_WHOLEPAGE:
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                    break;
                default:
                    break;
            }
        }

        delete pZoomItem;
    }
    else if(mpViewShell && (pArgs->Count () == 1))
    {
        SFX_REQUEST_ARG (rReq, pScale, SfxUInt32Item, ID_VAL_ZOOM, FALSE);
        mpViewShell->SetZoom (pScale->GetValue ());

        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
    }

}

} // end of namespace sd
