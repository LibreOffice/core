/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuconnct.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:15:43 $
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



#include "fuconnct.hxx"

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

namespace sd {

TYPEINIT1( FuConnectionDlg, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConnectionDlg::FuConnectionDlg (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuConnectionDlg::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuConnectionDlg( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuConnectionDlg::DoExecute( SfxRequest& rReq )
{
    SfxItemSet aNewAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aNewAttr );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ::std::auto_ptr<AbstractSfxSingleTabDialog> pDlg( pFact ? pFact->CreateSfxSingleTabDialog( NULL, aNewAttr, mpView, ResId(RID_SVXPAGE_CONNECTION)) : 0);

        if( pDlg.get() && (pDlg->Execute() == RET_OK) )
        {
            rReq.Done( *pDlg->GetOutputItemSet() );
            pArgs = rReq.GetArgs();
        }
    }
    if( pArgs )
        mpView->SetAttributes( *pArgs );
}

} // end of namespace sd
