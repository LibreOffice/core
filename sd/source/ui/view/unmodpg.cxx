/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/svdlayer.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdviter.hxx>
#include <svx/svdview.hxx>

#include <strings.hrc>
#include <strings.hxx>
#include <glob.hxx>
#include <app.hrc>

#include <unmodpg.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <drawdoc.hxx>


ModifyPageUndoAction::ModifyPageUndoAction(
    SdDrawDocument* pTheDoc,
    SdPage* pThePage,
    const OUString& aTheNewName,
    AutoLayout  eTheNewAutoLayout,
    bool bTheNewBckgrndVisible,
    bool bTheNewBckgrndObjsVisible)
:   SdUndoAction(pTheDoc)
{
    DBG_ASSERT(pThePage, "Undo without a page???");

    mpPage                  = pThePage;
    maNewName               = aTheNewName;
    meNewAutoLayout         = eTheNewAutoLayout;
    mbNewBckgrndVisible     = bTheNewBckgrndVisible;
    mbNewBckgrndObjsVisible = bTheNewBckgrndObjsVisible;

    meOldAutoLayout         = mpPage->GetAutoLayout();

    if (!mpPage->IsMasterPage())
    {
        maOldName = mpPage->GetName();
        SdrLayerAdmin& rLayerAdmin = mpDoc->GetLayerAdmin();
        SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
        SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
        SdrLayerIDSet aVisibleLayers = mpPage->TRG_GetMasterPageVisibleLayers();

        mbOldBckgrndVisible = aVisibleLayers.IsSet(aBckgrnd);
        mbOldBckgrndObjsVisible = aVisibleLayers.IsSet(aBckgrndObj);
    }
    else
    {
        mbOldBckgrndVisible = false;
        mbOldBckgrndObjsVisible = false;
    }

    SetComment( SdResId(STR_UNDO_MODIFY_PAGE) );
}

void ModifyPageUndoAction::Undo()
{
    // invalidate Selection, there could be objects deleted in this UNDO
    // which are no longer allowed to be selected then.
    SdrViewIter aIter(mpPage);
    SdrView* pView = aIter.FirstView();

    while(pView)
    {
        if(pView->AreObjectsMarked())
            pView->UnmarkAll();
        pView = aIter.NextView();
    }

    mpPage->SetAutoLayout( meOldAutoLayout );

    if (!mpPage->IsMasterPage())
    {
        if (mpPage->GetName() != maOldName)
        {
            mpPage->SetName(maOldName);

            if (mpPage->GetPageKind() == PageKind::Standard)
            {
                SdPage* pNotesPage = static_cast<SdPage*>(mpDoc->GetPage(mpPage->GetPageNum() + 1));
                pNotesPage->SetName(maOldName);
            }
        }

        SdrLayerAdmin& rLayerAdmin = mpDoc->GetLayerAdmin();
        SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
        SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
        SdrLayerIDSet aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, mbOldBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, mbOldBckgrndObjsVisible);
        mpPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }

    // Redisplay
    SfxViewFrame::Current()->GetDispatcher()->Execute(
        SID_SWITCHPAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
}

void ModifyPageUndoAction::Redo()
{
    // invalidate Selection, there could be objects deleted in this UNDO
    // which are no longer allowed to be selected then.
    SdrViewIter aIter(mpPage);
    SdrView* pView = aIter.FirstView();

    while(pView)
    {
        if(pView->AreObjectsMarked())
            pView->UnmarkAll();
        pView = aIter.NextView();
    }

    mpPage->meAutoLayout = meNewAutoLayout;

    if (!mpPage->IsMasterPage())
    {
        if (mpPage->GetName() != maNewName)
        {
            mpPage->SetName(maNewName);

            if (mpPage->GetPageKind() == PageKind::Standard)
            {
                SdPage* pNotesPage = static_cast<SdPage*>(mpDoc->GetPage(mpPage->GetPageNum() + 1));
                pNotesPage->SetName(maNewName);
            }
        }

        SdrLayerAdmin& rLayerAdmin = mpDoc->GetLayerAdmin();
        SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
        SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
        SdrLayerIDSet aVisibleLayers;
        aVisibleLayers.Set(aBckgrnd, mbNewBckgrndVisible);
        aVisibleLayers.Set(aBckgrndObj, mbNewBckgrndObjsVisible);
        mpPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }

    // Redisplay
    SfxViewFrame::Current()->GetDispatcher()->Execute(
        SID_SWITCHPAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
}

ModifyPageUndoAction::~ModifyPageUndoAction()
{
}

RenameLayoutTemplateUndoAction::RenameLayoutTemplateUndoAction(
    SdDrawDocument* pDocument,
    const OUString& rOldLayoutName,
    const OUString& rNewLayoutName)
    : SdUndoAction(pDocument)
    , maOldName(rOldLayoutName)
    , maNewName(rNewLayoutName)
    , maComment(SdResId(STR_TITLE_RENAMESLIDE))
{
    sal_Int32 nPos = maOldName.indexOf(SD_LT_SEPARATOR);
    if (nPos != -1)
        maOldName = maOldName.copy(0, nPos);
}

void RenameLayoutTemplateUndoAction::Undo()
{
    OUString aLayoutName(maNewName + SD_LT_SEPARATOR STR_LAYOUT_OUTLINE);
    mpDoc->RenameLayoutTemplate( aLayoutName, maOldName );
}

void RenameLayoutTemplateUndoAction::Redo()
{
    OUString aLayoutName(maOldName + SD_LT_SEPARATOR STR_LAYOUT_OUTLINE);
    mpDoc->RenameLayoutTemplate( aLayoutName, maNewName );
}

OUString RenameLayoutTemplateUndoAction::GetComment() const
{
    return  maComment;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
