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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svx/svdundo.hxx>

#include "undodraw.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"


// -----------------------------------------------------------------------

ScUndoDraw::ScUndoDraw( SfxUndoAction* pUndo, ScDocShell* pDocSh ) :
    pDrawUndo( pUndo ),
    pDocShell( pDocSh )
{
}

__EXPORT ScUndoDraw::~ScUndoDraw()
{
    delete pDrawUndo;
}

void ScUndoDraw::ForgetDrawUndo()
{
    pDrawUndo = NULL;   // nicht loeschen (Draw-Undo muss dann von aussen gemerkt werden)
}

String __EXPORT ScUndoDraw::GetComment() const
{
    if (pDrawUndo)
        return pDrawUndo->GetComment();
    else
        return String();
}

String __EXPORT ScUndoDraw::GetRepeatComment(SfxRepeatTarget& rTarget) const
{
    if (pDrawUndo)
        return pDrawUndo->GetRepeatComment(rTarget);
    else
        return String();
}

sal_uInt16 __EXPORT ScUndoDraw::GetId() const
{
    if (pDrawUndo)
        return pDrawUndo->GetId();
    else
        return 0;
}

sal_Bool __EXPORT ScUndoDraw::IsLinked()
{
    if (pDrawUndo)
        return pDrawUndo->IsLinked();
    else
        return sal_False;
}

void __EXPORT ScUndoDraw::SetLinked( sal_Bool bIsLinked )
{
    if (pDrawUndo)
        pDrawUndo->SetLinked(bIsLinked);
}

sal_Bool  __EXPORT ScUndoDraw::Merge( SfxUndoAction* pNextAction )
{
    if (pDrawUndo)
        return pDrawUndo->Merge(pNextAction);
    else
        return sal_False;
}

void ScUndoDraw::UpdateSubShell()
{
    // #i26822# remove the draw shell if the selected object has been removed
    ScTabViewShell* pViewShell = pDocShell->GetBestViewShell();
    if (pViewShell)
        pViewShell->UpdateDrawShell();
}

void __EXPORT ScUndoDraw::Undo()
{
    if (pDrawUndo)
    {
        pDrawUndo->Undo();
        pDocShell->SetDrawModified();
        UpdateSubShell();
    }
}

void __EXPORT ScUndoDraw::Redo()
{
    if (pDrawUndo)
    {
        pDrawUndo->Redo();
        pDocShell->SetDrawModified();
        UpdateSubShell();
    }
}

void __EXPORT ScUndoDraw::Repeat(SfxRepeatTarget& rTarget)
{
    if (pDrawUndo)
        pDrawUndo->Repeat(rTarget);
}

sal_Bool __EXPORT ScUndoDraw::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (pDrawUndo)
        return pDrawUndo->CanRepeat(rTarget);
    else
        return sal_False;
}



