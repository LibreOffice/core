/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewfun6.cxx,v $
 * $Revision: 1.11 $
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

#ifdef WIN
#define _MENUBTN_HXX
#endif

//#define _SFX_DOCFILT_HXX
#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXAPPWIN_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXDLGCFG_HXX
//#define _SFXDISPATCH_HXX
#define _SFXDOCFILE_HXX
#define _SFXDOCMAN_HXX
#define _SFXDOCMGR_HXX
#define _SFXDOCTDLG_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXOBJFAC_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NOCONTROL
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
#define _VCATTR_HXX
#define _VCONT_HXX

#define _SDR_NOTRANSFORM
#define _SDR_NOITEMS
#define _SDR_NOOBJECTS
#define _SVDXOUT_HXX

//------------------------------------------------------------------

#include <svx/svdundo.hxx>
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


// STATIC DATA -----------------------------------------------------------

//==================================================================

void ScViewFunc::DetectiveAddPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveAddPred( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();    //! use broadcast in DocFunc instead?
}

void ScViewFunc::DetectiveDelPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveDelPred( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveAddSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveAddSucc( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveDelSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveDelSucc( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveAddError()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveAddError( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveDelAll()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveDelAll( GetViewData()->GetTabNo() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveMarkInvalid()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveMarkInvalid( GetViewData()->GetTabNo() );
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

void ScViewFunc::DetectiveRefresh()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().DetectiveRefresh();
    if (!bDone)
        Sound::Beep();

    RecalcPPT();
}

//---------------------------------------------------------------------------

void ScViewFunc::ShowNote()
{
    //  permanent einblenden

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();
    BOOL bUndo (pDoc->IsUndoEnabled());

    ScPostIt aNote(pDoc);
    if ( pDoc->GetNote( nCol, nRow, nTab, aNote ) &&
         !pDoc->HasNoteObject( nCol, nRow, nTab ) )
    {
        HideNoteMarker();

        pDocSh->MakeDrawLayer();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();

        pModel->BeginCalcUndo();
        SdrObject* pObject = ScDetectiveFunc( pDoc,nTab ).ShowComment( nCol, nRow, FALSE );
        SdrUndoGroup* pUndo = NULL;
        if (bUndo)
            pUndo = pModel->GetCalcUndo();
        if (pObject)
        {
            aNote.SetShown( TRUE );
            pDoc->SetNote( nCol, nRow, nTab, aNote );
            // This repaint should not be neccessary.
            // But it solves a problem where following an
            // insertion of more note text, the note sometimes
            // displays the height to the previous note position.
            // A similar problem is also in ScUndoEditNote::Undo().
                        ScRange aDrawRange(pDoc->GetRange(nTab, aNote.GetRectangle()));
            pDocSh->PostPaint( aDrawRange, PAINT_GRID| PAINT_EXTRAS);
            if (pUndo)
                pDocSh->GetUndoManager()->AddUndoAction( new ScUndoNote( pDocSh,
                                                TRUE, ScAddress(nCol,nRow,nTab), pUndo ) );



            pDocSh->SetDocumentModified();
        }
        else
        {
            delete pUndo;
            Sound::Beep();
        }
    }
}

void ScViewFunc::HideNote()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return;         // da is nix
    BOOL bUndo (pDoc->IsUndoEnabled());

    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();

    ScPostIt aNote(pDoc);
    if ( pDoc->GetNote( nCol, nRow, nTab, aNote ) &&
         pDoc->HasNoteObject( nCol, nRow, nTab ) )
    {
        pModel->BeginCalcUndo();
        BOOL bDone = ScDetectiveFunc( pDoc,nTab ).HideComment( nCol, nRow );
        SdrUndoGroup* pUndo = NULL;
        if (bUndo)
            pUndo = pModel->GetCalcUndo();
        if (bDone)
        {
            aNote.SetShown( FALSE );
            pDoc->SetNote( nCol, nRow, nTab, aNote );
            // This repaint should not be neccessary.
            // But it solves a problem where following an
            // insertion of more note text, the note sometimes
            // continues to displays the additional height to
            // the previous note height position - despite the fact
            // that we have chosen to hide the note.
            // A similar problem is also in ScUndoEditNote::Undo().
                        ScRange aDrawRange(pDoc->GetRange(nTab, aNote.GetRectangle()));
            pDocSh->PostPaint( aDrawRange, PAINT_GRID| PAINT_EXTRAS);
            if (pUndo)
                pDocSh->GetUndoManager()->AddUndoAction( new ScUndoNote( pDocSh,
                                                FALSE, ScAddress(nCol,nRow,nTab), pUndo ) );

            pDocSh->SetDocumentModified();
        }
        else
        {
            delete pUndo;
            Sound::Beep();
        }
    }
}

void ScViewFunc::EditNote()
{
    //  zum Editieren einblenden und aktivieren

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();

    ScPostIt aNote(pDoc);
    BOOL bFound = pDoc->GetNote( nCol, nRow, nTab, aNote );
    if ( !bFound || !pDoc->HasNoteObject( nCol, nRow, nTab ) )      // neu oder versteckt
    {
        HideNoteMarker();

        pDocSh->MakeDrawLayer();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();

        pModel->BeginCalcUndo();
        //  TRUE -> auch neu anlegen
        SdrObject* pObject = ScDetectiveFunc( pDoc,nTab ).ShowComment( nCol, nRow, TRUE );

        //  Undo-Action (pModel->GetCalcUndo) wird beim StopEditMode abgeholt

        if (pObject)
        {
            FuPoor* pDraw = GetDrawFuncPtr();
            if ( pDraw )
            {
                FuSelection* pSel = static_cast<FuSelection*>(pDraw);
                // #i33764# Enable the resize handles before editing.
                pSel->ActivateNoteHandles(pObject);
            }
            //  Shown-Flag nicht veraendern

            //  Objekt aktivieren (wie in FuSelection::TestComment)
            GetViewData()->GetDispatcher().Execute(SID_DRAW_NOTEEDIT, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
            // jetzt den erzeugten FuText holen und in den EditModus setzen
            FuPoor* pPoor = GetDrawFuncPtr();
            if ( pPoor && pPoor->GetSlotID() == SID_DRAW_NOTEEDIT )  // hat keine RTTI
            {
                ScrollToObject( pObject );          // Objekt komplett sichtbar machen
                FuText* pText = (FuText*)pPoor;
                pText->SetInEditMode( pObject );
            }
        }
    }
}



