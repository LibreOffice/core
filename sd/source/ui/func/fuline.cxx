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
            SID_ATTR_LINE_STYLE,                // ( SID_SVX_START + 169 )
            SID_ATTR_LINE_DASH,                 // ( SID_SVX_START + 170 )
            SID_ATTR_LINE_WIDTH,                // ( SID_SVX_START + 171 )
            SID_ATTR_LINE_COLOR,                // ( SID_SVX_START + 172 )
            SID_ATTR_LINE_START,                // ( SID_SVX_START + 173 )
            SID_ATTR_LINE_END,                  // ( SID_SVX_START + 174 )
            SID_ATTR_LINE_TRANSPARENCE,         // (SID_SVX_START+1107)
            SID_ATTR_LINE_JOINT,                // (SID_SVX_START+1110)
            SID_ATTR_LINE_CAP,                  // (SID_SVX_START+1111)
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
