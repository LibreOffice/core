/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fuspell.cxx,v $
 * $Revision: 1.12 $
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


#include "fuspell.hxx"

#include <editeng/outliner.hxx>
#include <sfx2/bindings.hxx>

#include <sfx2/viewfrm.hxx>
#include "fupoor.hxx"
#include "Outliner.hxx"
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "ViewShellBase.hxx"

#include "app.hrc"

class SfxRequest;

namespace sd {

USHORT SidArraySpell[] = {
                SID_DRAWINGMODE,
                SID_OUTLINEMODE,
                SID_DIAMODE,
                SID_NOTESMODE,
                SID_HANDOUTMODE,
                0 };

TYPEINIT1( FuSpell, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSpell::FuSpell (
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

FunctionReference FuSpell::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSpell( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSpell::DoExecute( SfxRequest& )
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

FuSpell::~FuSpell()
{
    mpDocSh->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SidArraySpell );

    if (pSdOutliner)
        pSdOutliner->EndSpelling();

    if (bOwnOutliner)
        delete pSdOutliner;
}

/*************************************************************************
|*
|* Pruefung starten
|*
\************************************************************************/

void FuSpell::StartSpelling()
{
    // Get current main view shell.
    ViewShellBase* pBase (ViewShellBase::GetViewShellBase (
        mpDocSh->GetViewShell()->GetViewFrame()));
    if (pBase != NULL)
        mpViewShell = pBase->GetMainViewShell().get();
    else
        mpViewShell = NULL;
    if (mpViewShell != NULL)
    {
        if ( pSdOutliner && mpViewShell->ISA(DrawViewShell) && !bOwnOutliner )
        {
            pSdOutliner->EndSpelling();

            bOwnOutliner = TRUE;
            pSdOutliner = new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );
            pSdOutliner->PrepareSpelling();
        }
        else if ( pSdOutliner && mpViewShell->ISA(OutlineViewShell) && bOwnOutliner )
        {
            pSdOutliner->EndSpelling();
            delete pSdOutliner;

            bOwnOutliner = FALSE;
            pSdOutliner = mpDoc->GetOutliner();
            pSdOutliner->PrepareSpelling();
        }

        if (pSdOutliner)
            pSdOutliner->StartSpelling();
    }
}



} // end of namespace sd
