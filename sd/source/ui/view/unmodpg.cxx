/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unmodpg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:44:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SVDLAYER
#include <svx/svdlayer.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif


#include "strings.hrc"
#include "glob.hrc"         // STR_BCKGRND, STR_BCKGRNDOBJ
#include "app.hrc"          // SID_SWITCHPAGE

#include "unmodpg.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"


TYPEINIT1(ModifyPageUndoAction, SdUndoAction);


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

ModifyPageUndoAction::ModifyPageUndoAction(
    SfxUndoManager* pTheManager, // #67720#
    SdDrawDocument* pTheDoc,
    SdPage* pThePage,
    String aTheNewName,
    AutoLayout  eTheNewAutoLayout,
    BOOL bTheNewBckgrndVisible,
    BOOL bTheNewBckgrndObjsVisible)
:   SdUndoAction(pTheDoc),
    mpManager(pTheManager)
{
    DBG_ASSERT(pThePage, "Undo ohne Seite ???");

    pPage                   = pThePage;
    aNewName                = aTheNewName;
    eNewAutoLayout          = eTheNewAutoLayout;
    bNewBckgrndVisible      = bTheNewBckgrndVisible;
    bNewBckgrndObjsVisible  = bTheNewBckgrndObjsVisible;

    eOldAutoLayout          = pPage->GetAutoLayout();

    if (!pPage->IsMasterPage())
    {
        aOldName = pPage->GetName();
        SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
        BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
        BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
        SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();

        bOldBckgrndVisible = aVisibleLayers.IsSet(aBckgrnd);
        bOldBckgrndObjsVisible = aVisibleLayers.IsSet(aBckgrndObj);
    }

    aComment = String(SdResId(STR_UNDO_MODIFY_PAGE));
}

/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

#ifndef _SVDVITER_HXX
#include <svx/svdviter.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
void ModifyPageUndoAction::Undo()
{
    // #94637# invalidate Selection, there could be objects deleted in tis UNDO
    // which are no longer allowed to be selected then.
      SdrViewIter aIter(pPage);
    SdrView* pView = aIter.FirstView();

    while(pView)
    {
        if(pView->AreObjectsMarked())
            pView->UnmarkAll();
        pView = aIter.NextView();
    }

    pPage->SetAutoLayout( eOldAutoLayout );

    if (!pPage->IsMasterPage())
    {
        if (pPage->GetName() != aOldName)
        {
            pPage->SetName(aOldName);

            if (pPage->GetPageKind() == PK_STANDARD)
            {
                SdPage* pNotesPage = (SdPage*)pDoc->GetPage(pPage->GetPageNum() + 1);
                pNotesPage->SetName(aOldName);
            }
        }

        SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
        BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
        BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
        SetOfByte aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, bOldBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, bOldBckgrndObjsVisible);
        pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }

    // Redisplay
    SfxViewFrame::Current()->GetDispatcher()->Execute(
        SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void ModifyPageUndoAction::Redo()
{
    // #94637# invalidate Selection, there could be objects deleted in tis UNDO
    // which are no longer allowed to be selected then.
      SdrViewIter aIter(pPage);
    SdrView* pView = aIter.FirstView();

    while(pView)
    {
        if(pView->AreObjectsMarked())
            pView->UnmarkAll();
        pView = aIter.NextView();
    }

    // pPage->SetAutoLayout(eNewAutoLayout, TRUE);
    pPage->eAutoLayout = eNewAutoLayout;

    if (!pPage->IsMasterPage())
    {
        if (pPage->GetName() != aNewName)
        {
            pPage->SetName(aNewName);

            if (pPage->GetPageKind() == PK_STANDARD)
            {
                SdPage* pNotesPage = (SdPage*)pDoc->GetPage(pPage->GetPageNum() + 1);
                pNotesPage->SetName(aNewName);
            }
        }

        SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
        BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
        BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
        SetOfByte aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, bNewBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, bNewBckgrndObjsVisible);
        pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }

    // Redisplay
    SfxViewFrame::Current()->GetDispatcher()->Execute(
        SID_SWITCHPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}

/*************************************************************************
|*
|* Repeat()
|*
\************************************************************************/

void ModifyPageUndoAction::Repeat()
{
    DBG_ASSERT(FALSE, "ModifyPageUndoAction::Repeat: nicht implementiert");
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

ModifyPageUndoAction::~ModifyPageUndoAction()
{
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String ModifyPageUndoAction::GetComment() const
{
    return aComment;
}


