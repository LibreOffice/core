/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuhhconv.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:43:18 $
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

#include <fuhhconv.hxx>
#include "drawdoc.hxx"
#include "Outliner.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"

#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

#include "sdresid.hxx"
#include "strings.hrc"

class SfxRequest;

namespace sd {

class ViewShell;

TYPEINIT1( FuHangulHanjaConversion, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuHangulHanjaConversion::FuHangulHanjaConversion (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDocument,
    SfxRequest& rReq )
       : FuPoor(pViewSh, pWin, pView, pDocument, rReq),
    pSdOutliner(NULL),
    bOwnOutliner(FALSE)
{
    if ( pViewShell->ISA(DrawViewShell) )
    {
        bOwnOutliner = TRUE;
        pSdOutliner = new Outliner( pDoc, OUTLINERMODE_TEXTOBJECT );
    }
    else if ( pViewShell->ISA(OutlineViewShell) )
    {
        bOwnOutliner = FALSE;
        pSdOutliner = pDoc->GetOutliner();
    }

    if (pSdOutliner)
       pSdOutliner->PrepareSpelling();
}



/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuHangulHanjaConversion::~FuHangulHanjaConversion()
{
    if (pSdOutliner)
        pSdOutliner->EndConversion();

    if (bOwnOutliner)
        delete pSdOutliner;
}


/*************************************************************************
|*
|* Suchen&Ersetzen
|*
\************************************************************************/

void FuHangulHanjaConversion::StartConversion( INT16 nSourceLanguage, INT16 nTargetLanguage,
        const Font *pTargetFont, INT32 nOptions, BOOL bIsInteractive )
{

    String aString( SdResId(STR_UNDO_HANGULHANJACONVERSION) );
    pView->BegUndo( aString );

    ViewShellBase* pBase = PTR_CAST(ViewShellBase, SfxViewShell::Current());
    if (pBase != NULL)
        pViewShell = pBase->GetMainViewShell();

    if( pViewShell )
    {
        if ( pSdOutliner && pViewShell->ISA(DrawViewShell) && !bOwnOutliner )
        {
            pSdOutliner->EndConversion();

            bOwnOutliner = TRUE;
            pSdOutliner = new Outliner( pDoc, OUTLINERMODE_TEXTOBJECT );
            pSdOutliner->BeginConversion();
        }
        else if ( pSdOutliner && pViewShell->ISA(OutlineViewShell) && bOwnOutliner )
        {
            pSdOutliner->EndConversion();
            delete pSdOutliner;

            bOwnOutliner = FALSE;
            pSdOutliner = pDoc->GetOutliner();
            pSdOutliner->BeginConversion();
        }

        if (pSdOutliner)
            pSdOutliner->StartConversion(nSourceLanguage, nTargetLanguage, pTargetFont, nOptions, bIsInteractive );
    }

    // Due to changing between edit mode, notes mode, and handout mode the
    // view has most likely changed.  Get the new one.
    pViewShell = pBase->GetMainViewShell();
    if (pViewShell != NULL)
    {
        pView = pViewShell->GetView();
        pWindow = pViewShell->GetActiveWindow();
    }
    else
    {
        pView;
        pWindow = NULL;
    }

    if (pView != NULL)
        pView->EndUndo();
}

} // end of namespace
