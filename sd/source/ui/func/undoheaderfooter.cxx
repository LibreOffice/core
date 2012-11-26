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


#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "DrawViewShell.hxx"
#include "drawview.hxx"
#include "undoheaderfooter.hxx"

SdHeaderFooterUndoAction::SdHeaderFooterUndoAction( SdDrawDocument* pDoc, SdPage* pPage, const sd::HeaderFooterSettings& rNewSettings )
:   SdUndoAction(pDoc),
    mpPage(pPage),
    maOldSettings(pPage->getHeaderFooterSettings()),
    maNewSettings(rNewSettings)
{
}

SdHeaderFooterUndoAction::~SdHeaderFooterUndoAction()
{
}

void SdHeaderFooterUndoAction::Undo()
{
    mpPage->setHeaderFooterSettings( maOldSettings );
    SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}

void SdHeaderFooterUndoAction::Redo()
{
    mpPage->setHeaderFooterSettings( maNewSettings );
    SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}

