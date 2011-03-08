/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "undopage.hxx"

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "drawview.hxx"
#include <svx/svdpagv.hxx>



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
