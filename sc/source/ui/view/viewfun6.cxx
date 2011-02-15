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

#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>

#include "viewfunc.hxx"
#include "detfunc.hxx"
#include "detdata.hxx"
#include "viewdata.hxx"
#include "drwlayer.hxx"
#include "docsh.hxx"
#include "undocell.hxx"
#include "futext.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "sc.hrc"
#include "fusel.hxx"

//==================================================================

void ScViewFunc::DetectiveAddPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().
                    DetectiveAddPred( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();    //! use broadcast in DocFunc instead?
}

void ScViewFunc::DetectiveDelPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().
                    DetectiveDelPred( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveAddSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().
                    DetectiveAddSucc( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveDelSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().
                    DetectiveDelSucc( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveAddError()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().
                    DetectiveAddError( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveDelAll()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().
                    DetectiveDelAll( GetViewData()->GetTabNo() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveMarkInvalid()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().
                    DetectiveMarkInvalid( GetViewData()->GetTabNo() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveRefresh()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    sal_Bool bDone = pDocSh->GetDocFunc().DetectiveRefresh();
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

//---------------------------------------------------------------------------

void ScViewFunc::ShowNote( bool bShow )
{
    if( bShow )
        HideNoteMarker();
    const ScViewData& rViewData = *GetViewData();
    ScAddress aPos( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo() );
    // show note moved to ScDocFunc, to be able to use it in notesuno.cxx
    rViewData.GetDocShell()->GetDocFunc().ShowNote( aPos, bShow );
}

void ScViewFunc::EditNote()
{
    //  zum Editieren einblenden und aktivieren

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScAddress aPos( nCol, nRow, nTab );

    // start drawing undo to catch undo action for insertion of the caption object
    pDocSh->MakeDrawLayer();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    pDrawLayer->BeginCalcUndo();
    // generated undo action is processed in FuText::StopEditMode

    // get existing note or create a new note (including caption drawing object)
    if( ScPostIt* pNote = pDoc->GetOrCreateNote( aPos ) )
    {
        // hide temporary note caption
        HideNoteMarker();
        // show caption object without changing internal visibility state
        pNote->ShowCaptionTemp( aPos );

        /*  Drawing object has been created in ScDocument::GetOrCreateNote() or
            in ScPostIt::ShowCaptionTemp(), so ScPostIt::GetCaption() should
            return a caption object. */
        if( SdrCaptionObj* pCaption = pNote->GetCaption() )
        {
            // #i33764# enable the resize handles before starting edit mode
            if( FuPoor* pDraw = GetDrawFuncPtr() )
                static_cast< FuSelection* >( pDraw )->ActivateNoteHandles( pCaption );

            // activate object (as in FuSelection::TestComment)
            GetViewData()->GetDispatcher().Execute( SID_DRAW_NOTEEDIT, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
            // jetzt den erzeugten FuText holen und in den EditModus setzen
            FuPoor* pPoor = GetDrawFuncPtr();
            if ( pPoor && (pPoor->GetSlotID() == SID_DRAW_NOTEEDIT) )    // hat keine RTTI
            {
                ScrollToObject( pCaption );         // Objekt komplett sichtbar machen
                static_cast< FuText* >( pPoor )->SetInEditMode( pCaption );
            }
        }
    }
}
