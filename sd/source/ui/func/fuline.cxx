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


#include "fuline.hxx"

#include <svx/svxids.hrc>
#include <svx/tabline.hxx>
#include <svx/xenum.hxx>
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <svx/xdef.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include "ViewShell.hxx"
#include "View.hxx"
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
    sal_Bool        bHasMarked = mpView->AreObjectsMarked();

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
        SfxAbstractTabDialog * pDlg = pFact ? pFact->CreateSvxLineTabDialog(NULL,pNewAttr,mpDoc,pObj,bHasMarked) : 0;
        if( pDlg && (pDlg->Execute() == RET_OK) )
        {
            mpView->SetAttributes (*(pDlg->GetOutputItemSet ()));
        }

        // Attribute wurden geaendert, Listboxes in Objectbars muessen aktualisiert werden
        static sal_uInt16 SidArray[] = {
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
