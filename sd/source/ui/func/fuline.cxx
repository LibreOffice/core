/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuline.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:19:00 $
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


#include "fuline.hxx"

#include <svx/svxids.hrc>
#ifndef _SVX_TAB_LINE_HXX //autogen
#include <svx/tabline.hxx>
#endif
#ifndef _XENUM_HXX //autogen
#include <svx/xenum.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <svx/xdef.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "app.hrc"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

namespace sd {

TYPEINIT1( FuLine, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuLine::FuLine (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuLine::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuLine( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLine::DoExecute( SfxRequest& rReq )
{
    BOOL        bHasMarked = mpView->AreObjectsMarked();

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        const SdrObject* pObj = NULL;
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        if( rMarkList.GetMarkCount() == 1 )
            pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

        SfxItemSet* pNewAttr = new SfxItemSet( mpDoc->GetPool() );
        mpView->GetAttributes( *pNewAttr );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        SfxAbstractTabDialog * pDlg = pFact ? pFact->CreateSvxLineTabDialog(NULL,pNewAttr,mpDoc,ResId(RID_SVXDLG_LINE),pObj,bHasMarked) : 0;
        if( pDlg && (pDlg->Execute() == RET_OK) )
        {
            mpView->SetAttributes (*(pDlg->GetOutputItemSet ()));
        }

        // Attribute wurden geaendert, Listboxes in Objectbars muessen aktualisiert werden
        static USHORT SidArray[] = {
                        SID_ATTR_LINE_STYLE,
                        SID_ATTR_LINE_DASH,
                        SID_ATTR_LINE_WIDTH,
                        SID_ATTR_LINE_COLOR,
                        0 };

        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

        delete pDlg;
        delete pNewAttr;
    }

    rReq.Ignore ();
}

void FuLine::Activate()
{
}

void FuLine::Deactivate()
{
}


} // end of namespace sd
