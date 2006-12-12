/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fusearch.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:22:51 $
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


#include "fusearch.hxx"

#include <sfx2/viewfrm.hxx>

#define ITEMID_SEARCH           SID_SEARCH_ITEM
#include <svx/svxids.hrc>
#include <sfx2/srchitem.hxx>
#include <svx/srchdlg.hxx>
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#ifndef SD_FU_SPELL_HXX
#include "fuspell.hxx"  // wegen SidArraySpell[]
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "app.hrc"
#include "app.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

class SfxRequest;

namespace sd {

TYPEINIT1( FuSearch, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSearch::FuSearch (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq),
      pSdOutliner(NULL),
      bOwnOutliner(FALSE)
{
}

FunctionReference FuSearch::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSearch( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSearch::DoExecute( SfxRequest& )
{
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArraySpell );

    if ( mpViewShell->ISA(DrawViewShell) )
    {
        bOwnOutliner = TRUE;
        pSdOutliner = new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );
    }
    else if ( mpViewShell->ISA(OutlineViewShell) )
    {
        bOwnOutliner = FALSE;
        pSdOutliner = mpDoc->GetOutliner();
    }

    if (pSdOutliner)
       pSdOutliner->PrepareSpelling();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuSearch::~FuSearch()
{
    if ( ! mpDocSh->IsInDestruction() && mpDocSh->GetViewShell()!=NULL)
        mpDocSh->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SidArraySpell );

    if (pSdOutliner)
        pSdOutliner->EndSpelling();

    if (bOwnOutliner)
        delete pSdOutliner;
}


/*************************************************************************
|*
|* Suchen&Ersetzen
|*
\************************************************************************/

void FuSearch::SearchAndReplace( const SvxSearchItem* pSearchItem )
{
    ViewShellBase* pBase = PTR_CAST(ViewShellBase, SfxViewShell::Current());
    ViewShell* pViewShell = NULL;
    if (pBase != NULL)
        pViewShell = pBase->GetMainViewShell();

    if (pViewShell != NULL)
    {
        if ( pSdOutliner && pViewShell->ISA(DrawViewShell) && !bOwnOutliner )
        {
            pSdOutliner->EndSpelling();

            bOwnOutliner = TRUE;
            pSdOutliner = new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );
            pSdOutliner->PrepareSpelling();
        }
        else if ( pSdOutliner && pViewShell->ISA(OutlineViewShell) && bOwnOutliner )
        {
            pSdOutliner->EndSpelling();
            delete pSdOutliner;

            bOwnOutliner = FALSE;
            pSdOutliner = mpDoc->GetOutliner();
            pSdOutliner->PrepareSpelling();
        }

        if (pSdOutliner)
        {
            BOOL bEndSpelling = pSdOutliner->StartSearchAndReplace(pSearchItem);

            if (bEndSpelling)
            {
                pSdOutliner->EndSpelling();
                pSdOutliner->PrepareSpelling();
            }
        }
    }
}



} // end of namespace sd
