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


#include "undopage.hxx"

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "drawview.hxx"
#include <svx/svdpagv.hxx>



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
    mpPage->ScaleObjects(maOldSize, mfOldLeft, mfOldTop, mfOldRight, mfOldBottom, mbNewScale);
    mpPage->SetPageScale(maOldSize);
    mpPage->SetLeftPageBorder(mfOldLeft);
    mpPage->SetRightPageBorder(mfOldRight);
    mpPage->SetTopPageBorder(mfOldTop);
    mpPage->SetBottomPageBorder(mfOldBottom);
    mpPage->SetOrientation(meOldOrientation);
    mpPage->SetPaperBin( mnOldPaperBin );

    mpPage->SetBackgroundFullSize( mbOldFullSize );
    if( !mpPage->IsMasterPage() )
        ( (SdPage&) mpPage->TRG_GetMasterPage() ).SetBackgroundFullSize( mbOldFullSize );

}

void SdPageFormatUndoAction::Redo()
{
    mpPage->ScaleObjects(maNewSize, mfNewLeft, mfNewTop, mfNewRight, mfNewBottom, mbNewScale);
    mpPage->SetPageScale(maNewSize);
    mpPage->SetLeftPageBorder(mfNewLeft);
    mpPage->SetRightPageBorder(mfNewRight);
    mpPage->SetTopPageBorder(mfNewTop);
    mpPage->SetBottomPageBorder(mfNewBottom);
    mpPage->SetOrientation(meNewOrientation);
    mpPage->SetPaperBin( mnNewPaperBin );

    mpPage->SetBackgroundFullSize( mbNewFullSize );
    if( !mpPage->IsMasterPage() )
        ( (SdPage&) mpPage->TRG_GetMasterPage() ).SetBackgroundFullSize( mbNewFullSize );

}

/*************************************************************************
|*
|* LR-Redo()
|*
\************************************************************************/

SdPageLRUndoAction::~SdPageLRUndoAction()
{
}

void SdPageLRUndoAction::Undo()
{
    mpPage->SetLeftPageBorder(mfOldLeft);
    mpPage->SetRightPageBorder(mfOldRight);
}

void SdPageLRUndoAction::Redo()
{
    mpPage->SetLeftPageBorder(mfNewLeft);
    mpPage->SetRightPageBorder(mfNewRight);
}

/*************************************************************************
|*
|* UL-Redo()
|*
\************************************************************************/

SdPageULUndoAction::~SdPageULUndoAction()
{
}

void SdPageULUndoAction::Undo()
{
    mpPage->SetTopPageBorder(mfOldTop);
    mpPage->SetBottomPageBorder(mfOldBottom);
}

void SdPageULUndoAction::Redo()
{
    mpPage->SetTopPageBorder(mfNewTop);
    mpPage->SetBottomPageBorder(mfNewBottom);
}

