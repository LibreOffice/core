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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svx/svdundo.hxx>

#include "undodraw.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"


// -----------------------------------------------------------------------

TYPEINIT1(ScUndoDraw, SfxUndoAction);

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



