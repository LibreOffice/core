/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undopage.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:29:16 $
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
    Rectangle aOldBorderRect(mnOldLeft, mnOldUpper, mnOldRight, mnOldLower);
    mpPage->ScaleObjects(maOldSize, aOldBorderRect, mbNewScale);
    mpPage->SetSize(maOldSize);
    mpPage->SetLftBorder(mnOldLeft);
    mpPage->SetRgtBorder(mnOldRight);
    mpPage->SetUppBorder(mnOldUpper);
    mpPage->SetLwrBorder(mnOldLower);
    mpPage->SetOrientation(meOldOrientation);
    mpPage->SetPaperBin( mnOldPaperBin );

    mpPage->SetBackgroundFullSize( mbOldFullSize );
    if( !mpPage->IsMasterPage() )
        ( (SdPage&) mpPage->TRG_GetMasterPage() ).SetBackgroundFullSize( mbOldFullSize );

}

void SdPageFormatUndoAction::Redo()
{
    Rectangle aNewBorderRect(mnNewLeft, mnNewUpper, mnNewRight, mnNewLower);
    mpPage->ScaleObjects(maNewSize, aNewBorderRect, mbNewScale);
    mpPage->SetSize(maNewSize);
    mpPage->SetLftBorder(mnNewLeft);
    mpPage->SetRgtBorder(mnNewRight);
    mpPage->SetUppBorder(mnNewUpper);
    mpPage->SetLwrBorder(mnNewLower);
    mpPage->SetOrientation(meNewOrientation);
    mpPage->SetPaperBin( mnNewPaperBin );

    mpPage->SetBackgroundFullSize( mbNewFullSize );
    if( !mpPage->IsMasterPage() )
        ( (SdPage&) mpPage->TRG_GetMasterPage() ).SetBackgroundFullSize( mbNewFullSize );

}

SdPageLRUndoAction::~SdPageLRUndoAction()
{
}

void SdPageLRUndoAction::Undo()
{
    mpPage->SetLftBorder(mnOldLeft);
    mpPage->SetRgtBorder(mnOldRight);
}

void SdPageLRUndoAction::Redo()
{
    mpPage->SetLftBorder(mnNewLeft);
    mpPage->SetRgtBorder(mnNewRight);
}

SdPageULUndoAction::~SdPageULUndoAction()
{
}

void SdPageULUndoAction::Undo()
{
    mpPage->SetUppBorder(mnOldUpper);
    mpPage->SetLwrBorder(mnOldLower);
}

/*************************************************************************
|*
|* UL-Redo()
|*
\************************************************************************/

void SdPageULUndoAction::Redo()
{
    mpPage->SetUppBorder(mnNewUpper);
    mpPage->SetLwrBorder(mnNewLower);
}

