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


#include "fulink.hxx"

#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/svxdlg.hxx>

#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "app.hrc"


class SfxRequest;

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuLink::FuLink (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuLink::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuLink( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLink::DoExecute( SfxRequest& )
{
    sfx2::LinkManager* pLinkManager = mpDoc->GetLinkManager();

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractLinksDialog* pDlg = pFact->CreateLinksDialog( mpViewShell->GetActiveWindow(), pLinkManager );
    if ( pDlg )
    {
        pDlg->Execute();
        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_MANAGE_LINKS );
        delete pDlg;
    }
}


} // end of namespace sd
