/*************************************************************************
 *
 *  $RCSfile: viewfun6.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:26:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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


// STATIC DATA -----------------------------------------------------------

//==================================================================

void ScViewFunc::DetectiveAddPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveAddPred( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();
}

void ScViewFunc::DetectiveDelPred()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveDelPred( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();
}

void ScViewFunc::DetectiveAddSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveAddSucc( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();
}

void ScViewFunc::DetectiveDelSucc()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveDelSucc( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();
}

void ScViewFunc::DetectiveAddError()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveAddError( GetViewData()->GetCurPos() );
    if (!bDone)
        Sound::Beep();
}

void ScViewFunc::DetectiveDelAll()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveDelAll( GetViewData()->GetTabNo() );
    if (!bDone)
        Sound::Beep();
}

void ScViewFunc::DetectiveMarkInvalid()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().
                    DetectiveMarkInvalid( GetViewData()->GetTabNo() );
    if (!bDone)
        Sound::Beep();
}

void ScViewFunc::DetectiveRefresh()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bDone = pDocSh->GetDocFunc().DetectiveRefresh();
    if (!bDone)
        Sound::Beep();
}

//---------------------------------------------------------------------------

void ScViewFunc::ShowNote()
{
    //  permanent einblenden

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();

    ScPostIt aNote;
    if ( pDoc->GetNote( nCol, nRow, nTab, aNote ) &&
         !pDoc->HasNoteObject( nCol, nRow, nTab ) )
    {
        pDocSh->MakeDrawLayer();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();

        pModel->BeginCalcUndo();
        SdrObject* pObject = ScDetectiveFunc( pDoc,nTab ).ShowComment( nCol, nRow, FALSE );
        SdrUndoGroup* pUndo = pModel->GetCalcUndo();
        if (pObject)
        {
            aNote.SetShown( TRUE );
            pDoc->SetNote( nCol, nRow, nTab, aNote );
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

    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();

    ScPostIt aNote;
    if ( pDoc->GetNote( nCol, nRow, nTab, aNote ) &&
         pDoc->HasNoteObject( nCol, nRow, nTab ) )
    {
        pModel->BeginCalcUndo();
        BOOL bDone = ScDetectiveFunc( pDoc,nTab ).HideComment( nCol, nRow );
        SdrUndoGroup* pUndo = pModel->GetCalcUndo();
        if (bDone)
        {
            aNote.SetShown( FALSE );
            pDoc->SetNote( nCol, nRow, nTab, aNote );
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
    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();

    ScPostIt aNote;
    BOOL bFound = pDoc->GetNote( nCol, nRow, nTab, aNote );
    if ( !bFound || !pDoc->HasNoteObject( nCol, nRow, nTab ) )      // neu oder versteckt
    {
        pDocSh->MakeDrawLayer();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();

        pModel->BeginCalcUndo();
        //  TRUE -> auch neu anlegen
        SdrObject* pObject = ScDetectiveFunc( pDoc,nTab ).ShowComment( nCol, nRow, TRUE );

        //  Undo-Action (pModel->GetCalcUndo) wird beim StopEditMode abgeholt

        if (pObject)
        {
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



