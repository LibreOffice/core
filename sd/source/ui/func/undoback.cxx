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


#include "undoback.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include <svl/itemset.hxx>

TYPEINIT1( SdBackgroundObjUndoAction, SdUndoAction );

// -----------------------------------------------------------------------------

SdBackgroundObjUndoAction::SdBackgroundObjUndoAction(
    SdDrawDocument& rDoc,
    SdPage& rPage,
    const SfxItemSet& rItenSet)
:   SdUndoAction(&rDoc),
    mrPage(rPage),
    mpItemSet(new SfxItemSet(rItenSet))
{
    OUString aString( SdResId( STR_UNDO_CHANGE_PAGEFORMAT ) );
    SetComment( aString );
}

// -----------------------------------------------------------------------------

SdBackgroundObjUndoAction::~SdBackgroundObjUndoAction()
{
    delete mpItemSet;
}

// -----------------------------------------------------------------------------

void SdBackgroundObjUndoAction::ImplRestoreBackgroundObj()
{
    SfxItemSet* pNew = new SfxItemSet(mrPage.getSdrPageProperties().GetItemSet());
    mrPage.getSdrPageProperties().ClearItem();
    mrPage.getSdrPageProperties().PutItemSet(*mpItemSet);
    delete mpItemSet;
    mpItemSet = pNew;

    // tell the page that it's visualization has changed
    mrPage.ActionChanged();
}

// -----------------------------------------------------------------------------

void SdBackgroundObjUndoAction::Undo()
{
    ImplRestoreBackgroundObj();
}

// -----------------------------------------------------------------------------

void SdBackgroundObjUndoAction::Redo()
{
    ImplRestoreBackgroundObj();
}

// -----------------------------------------------------------------------------

SdUndoAction* SdBackgroundObjUndoAction::Clone() const
{
    return new SdBackgroundObjUndoAction(*mpDoc, mrPage, *mpItemSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
