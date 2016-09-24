/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "undopage.hxx"

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "drawview.hxx"
#include <svx/svdpagv.hxx>


SdPageFormatUndoAction::~SdPageFormatUndoAction()
{
}

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
        static_cast<SdPage&>( mpPage->TRG_GetMasterPage() ).SetBackgroundFullSize( mbOldFullSize );

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
        static_cast<SdPage&>( mpPage->TRG_GetMasterPage() ).SetBackgroundFullSize( mbNewFullSize );

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

/**
 * UL-Redo()
 */
void SdPageULUndoAction::Redo()
{
    mpPage->SetUppBorder(mnNewUpper);
    mpPage->SetLwrBorder(mnNewLower);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
