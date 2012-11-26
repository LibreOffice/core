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


#include "unprlout.hxx"

#include "strings.hrc"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdPresentationLayoutUndoAction::SdPresentationLayoutUndoAction(
                            SdDrawDocument* pTheDoc,
                            String          aTheOldLayoutName,
                            String          aTheNewLayoutName,
                            AutoLayout      eTheOldAutoLayout,
                            AutoLayout      eTheNewAutoLayout,
                            bool            bSet,
                            SdPage*         pThePage):
                      SdUndoAction(pTheDoc)
{
    aOldLayoutName = aTheOldLayoutName;
    aNewLayoutName = aTheNewLayoutName;
    eOldAutoLayout = eTheOldAutoLayout;
    eNewAutoLayout = eTheNewAutoLayout;
    bSetAutoLayout = bSet;

    DBG_ASSERT(pThePage, "keine Page gesetzt!");
    pPage = pThePage;
    aComment = String(SdResId(STR_UNDO_SET_PRESLAYOUT));
}

/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void SdPresentationLayoutUndoAction::Undo()
{
    pPage->SetPresentationLayout(aOldLayoutName, true, true, true);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eOldAutoLayout, true);
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void SdPresentationLayoutUndoAction::Redo()
{
    pPage->SetPresentationLayout(aNewLayoutName);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eNewAutoLayout, true);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdPresentationLayoutUndoAction::~SdPresentationLayoutUndoAction()
{
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String SdPresentationLayoutUndoAction::GetComment() const
{
    return aComment;
}



