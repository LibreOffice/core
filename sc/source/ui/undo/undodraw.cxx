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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
#define _DYNARY_HXX
#define _SVMEMPOOL_HXX
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX

//SV
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
#define _HELP_HXX
#define _MDIWIN_HXX
#define _SPIN_HXX
#define _FILDLG_HXX
#define _COLDLG_HXX
#define _SOUND_HXX


#define SI_NOITEMS
#define _SI_NOSBXCONTROLS
#define _SI_NOOTHERFORMS
#define _SI_NOCONTROL
#define _SI_NOSBXCONTROLS
#define _SIDLL_HXX

// SFX
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

// INCLUDE ---------------------------------------------------------------

#include <svx/svdundo.hxx>

#include "undodraw.hxx"
#include "docsh.hxx"


// -----------------------------------------------------------------------

TYPEINIT1(ScUndoDraw, SfxUndoAction);

// -----------------------------------------------------------------------

ScUndoDraw::ScUndoDraw( SfxUndoAction* pUndo, ScDocShell* pDocSh ) :
    pDrawUndo( pUndo ),
    pDocShell( pDocSh )
{
}

ScUndoDraw::~ScUndoDraw()
{
    delete pDrawUndo;
}

void ScUndoDraw::ForgetDrawUndo()
{
    pDrawUndo = NULL;   // nicht loeschen (Draw-Undo muss dann von aussen gemerkt werden)
}

String ScUndoDraw::GetComment() const
{
    if (pDrawUndo)
        return pDrawUndo->GetComment();
    else
        return String();
}

String ScUndoDraw::GetRepeatComment(SfxRepeatTarget& rTarget) const
{
    if (pDrawUndo)
        return pDrawUndo->GetRepeatComment(rTarget);
    else
        return String();
}

sal_uInt16 ScUndoDraw::GetId() const
{
    if (pDrawUndo)
        return pDrawUndo->GetId();
    else
        return 0;
}

sal_Bool ScUndoDraw::IsLinked()
{
    if (pDrawUndo)
        return pDrawUndo->IsLinked();
    else
        return false;
}

void ScUndoDraw::SetLinked( sal_Bool bIsLinked )
{
    if (pDrawUndo)
        pDrawUndo->SetLinked(bIsLinked);
}

sal_Bool  ScUndoDraw::Merge( SfxUndoAction* pNextAction )
{
    if (pDrawUndo)
        return pDrawUndo->Merge(pNextAction);
    else
        return false;
}

void ScUndoDraw::Undo()
{
    if (pDrawUndo)
    {
        pDrawUndo->Undo();
        pDocShell->SetDrawModified();
    }
}

void ScUndoDraw::Redo()
{
    if (pDrawUndo)
    {
        pDrawUndo->Redo();
        pDocShell->SetDrawModified();
    }
}

void ScUndoDraw::Repeat(SfxRepeatTarget& rTarget)
{
    if (pDrawUndo)
        pDrawUndo->Repeat(rTarget);
}

sal_Bool ScUndoDraw::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (pDrawUndo)
        return pDrawUndo->CanRepeat(rTarget);
    else
        return false;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
