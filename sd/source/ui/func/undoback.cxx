/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "undoback.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include <svl/itemset.hxx>

// ---------------------------
// - BackgroundObjUndoAction -
// ---------------------------

SdBackgroundObjUndoAction::SdBackgroundObjUndoAction(
    SdDrawDocument& rDoc,
    SdPage& rPage,
    const SfxItemSet& rItenSet)
:   SdUndoAction(&rDoc),
    mrPage(rPage),
    mpItemSet(new SfxItemSet(rItenSet))
{
    String aString( SdResId( STR_UNDO_CHANGE_PAGEFORMAT ) );
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

    // #110094#-15
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
