/*************************************************************************
 *
 *  $RCSfile: unmodpg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:12:21 $
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

#ifndef _SVDLAYER
#include <svx/svdlayer.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#pragma hdrstop

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

ModifyPageUndoAction::ModifyPageUndoAction(SdDrawDocument* pTheDoc,
                                           SdPage*         pThePage,
                                           String          aTheNewName,
                                           AutoLayout      eTheNewAutoLayout,
                                           BOOL            bTheNewBckgrndVisible,
                                           BOOL            bTheNewBckgrndObjsVisible):
                      SdUndoAction(pTheDoc)
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
        USHORT nPos = 0;
        SetOfByte aVisibleLayers = pPage->GetMasterPageVisibleLayers(nPos);

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

void __EXPORT ModifyPageUndoAction::Undo()
{
    pPage->SetAutoLayout(eOldAutoLayout, TRUE);

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
        USHORT nPos = 0;
        SetOfByte aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, bOldBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, bOldBckgrndObjsVisible);

        nPos = 0;
        pPage->SetMasterPageVisibleLayers(aVisibleLayers, nPos);
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

void __EXPORT ModifyPageUndoAction::Redo()
{
    pPage->SetAutoLayout(eNewAutoLayout, TRUE);

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
        USHORT nPos = 0;
        SetOfByte aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, bNewBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, bNewBckgrndObjsVisible);

        nPos = 0;
        pPage->SetMasterPageVisibleLayers(aVisibleLayers, nPos);
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

void __EXPORT ModifyPageUndoAction::Repeat()
{
    DBG_ASSERT(FALSE, "ModifyPageUndoAction::Repeat: nicht implementiert");
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT ModifyPageUndoAction::~ModifyPageUndoAction()
{
}

/*************************************************************************
|*
|* Kommentar liefern
|*
\************************************************************************/

String __EXPORT ModifyPageUndoAction::GetComment() const
{
    return aComment;
}


