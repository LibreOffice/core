/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undopage.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:32:12 $
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


#include "undopage.hxx"

#include <svx/svxids.hrc>
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#include "sdpage.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif



TYPEINIT1(SdPageFormatUndoAction, SdUndoAction);
TYPEINIT1(SdPageLRUndoAction, SdUndoAction);
TYPEINIT1(SdPageULUndoAction, SdUndoAction);


/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdPageFormatUndoAction::~SdPageFormatUndoAction()
{
}

/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void SdPageFormatUndoAction::Undo()
{
    Rectangle aOldBorderRect(nOldLeft, nOldUpper, nOldRight, nOldLower);
    pPage->ScaleObjects(aOldSize, aOldBorderRect, bNewScale);
    pPage->SetSize(aOldSize);
    pPage->SetLftBorder(nOldLeft);
    pPage->SetRgtBorder(nOldRight);
    pPage->SetUppBorder(nOldUpper);
    pPage->SetLwrBorder(nOldLower);
    pPage->SetOrientation(eOldOrientation);
    pPage->SetPaperBin( nOldPaperBin );

    pPage->SetBackgroundFullSize( bOldFullSize );
    if( !pPage->IsMasterPage() )
        ( (SdPage&) pPage->TRG_GetMasterPage() ).SetBackgroundFullSize( bOldFullSize );

    SfxViewShell* pViewShell = SfxViewShell::Current();

    /*  if ( pViewShell->ISA(::sd::DrawViewShell))
    {
        ::sd::DrawViewShell* pDrViewShell =
              static_cast< ::sd::DrawViewShell*>(pViewShell);
        long nWidth = pPage->GetSize().Width();
        long nHeight = pPage->GetSize().Height();

        Point aPageOrg = Point(nWidth, nHeight / 2);
        Size aViewSize = Size(nWidth * 3, nHeight * 2);

        pDrViewShell->InitWindows(aPageOrg, aViewSize, Point(-1, -1), TRUE);
        pDrViewShell->GetView()->SetWorkArea(Rectangle(Point(0,0) - aPageOrg, aViewSize));

        pDrViewShell->UpdateScrollBars();
        pDrViewShell->GetView()->GetPageViewByIndex(0)->SetPageOrigin(Point(0,0));
        pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }
    */
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void SdPageFormatUndoAction::Redo()
{
    Rectangle aNewBorderRect(nNewLeft, nNewUpper, nNewRight, nNewLower);
    pPage->ScaleObjects(aNewSize, aNewBorderRect, bNewScale);
    pPage->SetSize(aNewSize);
    pPage->SetLftBorder(nNewLeft);
    pPage->SetRgtBorder(nNewRight);
    pPage->SetUppBorder(nNewUpper);
    pPage->SetLwrBorder(nNewLower);
    pPage->SetOrientation(eNewOrientation);
    pPage->SetPaperBin( nNewPaperBin );

    pPage->SetBackgroundFullSize( bNewFullSize );
    if( !pPage->IsMasterPage() )
        ( (SdPage&) pPage->TRG_GetMasterPage() ).SetBackgroundFullSize( bNewFullSize );

    SfxViewShell* pViewShell = SfxViewShell::Current();

    /*  if ( pViewShell->ISA(::sd::DrawViewShell))
    {
        ::sd::DrawViewShell* pDrViewShell =
              static_cast< ::sd::DrawViewShell*>(pViewShell);
        long nWidth = pPage->GetSize().Width();
        long nHeight = pPage->GetSize().Height();

        Point aPageOrg = Point(nWidth, nHeight / 2);
        Size aViewSize = Size(nWidth * 3, nHeight * 2);

        pDrViewShell->InitWindows(aPageOrg, aViewSize, Point(-1, -1), TRUE);
        pDrViewShell->GetView()->SetWorkArea(Rectangle(Point(0,0) - aPageOrg, aViewSize));

        pDrViewShell->UpdateScrollBars();
        pDrViewShell->GetView()->GetPageViewByIndex(0)->SetPageOrigin(Point(0,0));
        pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }
    */
}

/*************************************************************************
|*
|* Repeat()
|*
\************************************************************************/

void SdPageFormatUndoAction::Repeat()
{
    Redo();
}

/*************************************************************************
|*
|* LR-Destruktor
|*
\************************************************************************/

SdPageLRUndoAction::~SdPageLRUndoAction()
{
}

/*************************************************************************
|*
|* LR-Undo()
|*
\************************************************************************/

void SdPageLRUndoAction::Undo()
{
    pPage->SetLftBorder(nOldLeft);
    pPage->SetRgtBorder(nOldRight);
}

/*************************************************************************
|*
|* LR-Redo()
|*
\************************************************************************/

void SdPageLRUndoAction::Redo()
{
    pPage->SetLftBorder(nNewLeft);
    pPage->SetRgtBorder(nNewRight);
}

/*************************************************************************
|*
|* LR-Repeat()
|*
\************************************************************************/

void SdPageLRUndoAction::Repeat()
{
    Redo();
}

/*************************************************************************
|*
|* UL-Destruktor
|*
\************************************************************************/

SdPageULUndoAction::~SdPageULUndoAction()
{
}

/*************************************************************************
|*
|* UL-Undo()
|*
\************************************************************************/

void SdPageULUndoAction::Undo()
{
    pPage->SetUppBorder(nOldUpper);
    pPage->SetLwrBorder(nOldLower);
}

/*************************************************************************
|*
|* UL-Redo()
|*
\************************************************************************/

void SdPageULUndoAction::Redo()
{
    pPage->SetUppBorder(nNewUpper);
    pPage->SetLwrBorder(nNewLower);
}

/*************************************************************************
|*
|* UL-Repeat()
|*
\************************************************************************/

void SdPageULUndoAction::Repeat()
{
    Redo();
}



