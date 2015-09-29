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

#include "SlsDragAndDropContext.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsTransferableData.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "sdtreelb.hxx"
#include <sfx2/bindings.hxx>
#include <boost/bind.hpp>

namespace sd { namespace slidesorter { namespace controller {

DragAndDropContext::DragAndDropContext (SlideSorter& rSlideSorter)
    : mpTargetSlideSorter(&rSlideSorter),
      mnInsertionIndex(-1)
{
    // No Drag-and-Drop for master pages.
    if (rSlideSorter.GetModel().GetEditMode() != EM_PAGE)
        return;

    // For properly handling transferables created by the navigator we
    // need additional information.  For this a user data object is
    // created that contains the necessary information.
    SdTransferable* pTransferable = SD_MOD()->pTransferDrag;
    SdPageObjsTLB::SdPageObjsTransferable* pTreeListBoxTransferable
        = dynamic_cast<SdPageObjsTLB::SdPageObjsTransferable*>(pTransferable);
    if (pTreeListBoxTransferable!=NULL && !TransferableData::GetFromTransferable(pTransferable))
    {
        pTransferable->AddUserData(
            sd::slidesorter::controller::Clipboard::CreateTransferableUserData(pTransferable));
    }

    rSlideSorter.GetController().GetInsertionIndicatorHandler()->UpdateIndicatorIcon(pTransferable);
}

DragAndDropContext::~DragAndDropContext()
{
    SetTargetSlideSorter (NULL, Point(0,0), InsertionIndicatorHandler::UnknownMode);
}

void DragAndDropContext::Dispose()
{
    mnInsertionIndex = -1;
}

void DragAndDropContext::UpdatePosition (
    const Point& rMousePosition,
    const InsertionIndicatorHandler::Mode eMode,
    const bool bAllowAutoScroll)
{
    if (mpTargetSlideSorter == NULL)
        return;

    if (mpTargetSlideSorter->GetProperties()->IsUIReadOnly())
        return;

    // Convert window coordinates into model coordinates (we need the
    // window coordinates for auto-scrolling because that remains
    // constant while scrolling.)
    sd::Window *pWindow (mpTargetSlideSorter->GetContentWindow());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));
    std::shared_ptr<InsertionIndicatorHandler> pInsertionIndicatorHandler (
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler());

    bool bDoAutoScroll = bAllowAutoScroll
            && mpTargetSlideSorter->GetController().GetScrollBarManager().AutoScroll(
                rMousePosition,
                ::boost::bind(
                    &DragAndDropContext::UpdatePosition, this, rMousePosition, eMode, false));

    if (!bDoAutoScroll)
    {
        pInsertionIndicatorHandler->UpdatePosition(aMouseModelPosition, eMode);

        // Remember the new insertion index.
        mnInsertionIndex = pInsertionIndicatorHandler->GetInsertionPageIndex();
        if (pInsertionIndicatorHandler->IsInsertionTrivial(mnInsertionIndex, eMode))
            mnInsertionIndex = -1;
    }
}

void DragAndDropContext::SetTargetSlideSorter (
    SlideSorter* pSlideSorter,
    const Point& rMousePosition,
    const InsertionIndicatorHandler::Mode eMode,
    const bool bIsOverSourceView)
{
    if (mpTargetSlideSorter != NULL)
    {
        mpTargetSlideSorter->GetController().GetScrollBarManager().StopAutoScroll();
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->End(
            Animator::AM_Animated);
    }

    mpTargetSlideSorter = pSlideSorter;

    if (mpTargetSlideSorter != NULL)
    {
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->Start(
            bIsOverSourceView);
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler()->UpdatePosition(
            rMousePosition,
            eMode);

    }
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
