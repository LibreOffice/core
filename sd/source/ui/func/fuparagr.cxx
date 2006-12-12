/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuparagr.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:21:40 $
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


#include "fuparagr.hxx"

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#include <svx/svxids.hrc>
#include <svx/editdata.hxx>

#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#include "app.hrc"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"
#include "sdabstdlg.hxx"
#include "paragr.hrc"
namespace sd {

TYPEINIT1( FuParagraph, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuParagraph::FuParagraph (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuParagraph::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuParagraph( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuParagraph::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );
        SfxItemPool *pPool =  aEditAttr.GetPool();
        SfxItemSet aNewAttr( *pPool,
                             EE_ITEMS_START, EE_ITEMS_END,
                             SID_ATTR_TABSTOP_OFFSET, SID_ATTR_TABSTOP_OFFSET,
                             0 );

        aNewAttr.Put( aEditAttr );

        // linker Rand als Offset
        const long nOff = ( (SvxLRSpaceItem&)aNewAttr.Get( EE_PARA_LRSPACE ) ).GetTxtLeft();
        // Umrechnung, da TabulatorTabPage immer von Twips ausgeht !
        SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
        aNewAttr.Put( aOff );

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        SfxAbstractTabDialog* pDlg = pFact ? pFact->CreateSdParagraphTabDlg(NULL, &aNewAttr ) : 0;
        if( pDlg )
        {
            USHORT nResult = pDlg->Execute();

            switch( nResult )
            {
                case RET_OK:
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );

                    pArgs = rReq.GetArgs();
                }
                break;

                default:
                {
                    delete pDlg;
                }
                return; // Abbruch
            }
            delete( pDlg );
        }
    }
    mpView->SetAttributes( *pArgs );

    // invalidieren der Slots
    static USHORT SidArray[] = {
        SID_ATTR_TABSTOP,
        SID_ATTR_PARA_ADJUST_LEFT,
        SID_ATTR_PARA_ADJUST_RIGHT,
        SID_ATTR_PARA_ADJUST_CENTER,
        SID_ATTR_PARA_ADJUST_BLOCK,
        SID_ATTR_PARA_LINESPACE_10,
        SID_ATTR_PARA_LINESPACE_15,
        SID_ATTR_PARA_LINESPACE_20,
        SID_ATTR_PARA_LRSPACE,
        SID_ATTR_PARA_LEFT_TO_RIGHT,
        SID_ATTR_PARA_RIGHT_TO_LEFT,
        SID_RULER_TEXT_RIGHT_TO_LEFT,
        SID_PARASPACE_INCREASE,
        SID_PARASPACE_DECREASE,
        0 };

    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );
}

void FuParagraph::Activate()
{
}

void FuParagraph::Deactivate()
{
}

} // end of namespace sd
