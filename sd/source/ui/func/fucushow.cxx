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

#include "fucushow.hxx"

#include <svx/svxids.hrc>

#include "app.hrc"
#include "sdresid.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "sdabstdlg.hxx"

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuCustomShowDlg::FuCustomShowDlg (
    ViewShell* pViewSh,
    ::sd::Window*    pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor( pViewSh, pWin, pView, pDoc, rReq )
{
}

FunctionReference FuCustomShowDlg::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuCustomShowDlg( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuCustomShowDlg::DoExecute( SfxRequest& )
{
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    AbstractSdCustomShowDlg* pDlg = pFact ? pFact->CreateSdCustomShowDlg( NULL, *mpDoc ) : 0;
    if( pDlg )
    {
        sal_uInt16 nRet = pDlg->Execute();
        if( pDlg->IsModified() )
        {
            mpDoc->SetChanged( true );
            sd::PresentationSettings& rSettings = mpDoc->getPresentationSettings();
            rSettings.mbCustomShow = pDlg->IsCustomShow();
        }
        delete pDlg;

        if( nRet == RET_YES )
        {
            mpViewShell->SetStartShowWithDialog();

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }
    }
}

} // end of namespace
