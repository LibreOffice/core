/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fucushow.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:49:13 $
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

#include "fucushow.hxx"

#include <svx/svxids.hrc>


//CHINA001 #include "custsdlg.hxx"

#include "app.hrc"
#include "sdresid.hxx"

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"
#include "sdpage.hxx"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#include "sdabstdlg.hxx" //CHINA001
#include "custsdlg.hrc" //CHINA001

namespace sd {

TYPEINIT1( FuCustomShowDlg, FuPoor );


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

void FuCustomShowDlg::DoExecute( SfxRequest& rReq )
{
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
    AbstractSdCustomShowDlg* pDlg = pFact ? pFact->CreateSdCustomShowDlg(ResId( DLG_CUSTOMSHOW ), NULL, *pDoc ) : 0;
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    if( pDlg )
    {
        USHORT nRet = pDlg->Execute(); //CHINA001 nRet = aDlg.Execute();
        if( nRet != RET_CANCEL )
        {
            // wenn sich etwas geaendert hat, setzen wir das Modified-Flag,
            if( pDlg->IsModified() )//CHINA001 if( aDlg.IsModified() )
            {
                pDoc->SetChanged( TRUE );
                sd::PresentationSettings& rSettings = pDoc->getPresentationSettings();
                rSettings.mbCustomShow = pDlg->IsCustomShow();
            }

            if( nRet == RET_YES )
            {
                pViewShell->SetStartShowWithDialog();

                pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
            }
        }
        delete pDlg; //CHINA001
    }
}

} // end of namespace
