/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    
    if (rSlideSorter.GetModel().GetEditMode() != EM_PAGE)
        return;

    
    
    
    SdTransferable* pTransferable = SD_MOD()->pTransferDrag;
    SdPageObjsTLB::SdPageObjsTransferable* pTreeListBoxTransferable
        = dynamic_cast<SdPageObjsTLB::SdPageObjsTransferable*>(pTransferable);
    if (pTreeListBoxTransferable!=NULL && !TransferableData::GetFromTransferable(pTransferable))
    {
        pTransferable->AddUserData(
            rSlideSorter.GetController().GetClipboard().CreateTransferableUserData(pTransferable));
    }

    rSlideSorter.GetController().GetInsertionIndicatorHandler()->UpdateIndicatorIcon(pTransferable);
}




DragAndDropContext::~DragAndDropContext (void)
{
    SetTargetSlideSorter (NULL, Point(0,0), InsertionIndicatorHandler::UnknownMode, false);
}




void DragAndDropContext::Dispose (void)
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

    
    
    
    SharedSdWindow pWindow (mpTargetSlideSorter->GetContentWindow());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));
    ::boost::shared_ptr<InsertionIndicatorHandler> pInsertionIndicatorHandler (
        mpTargetSlideSorter->GetController().GetInsertionIndicatorHandler());

    bool bDoAutoScroll = bAllowAutoScroll
            && mpTargetSlideSorter->GetController().GetScrollBarManager().AutoScroll(
                rMousePosition,
                ::boost::bind(
                    &DragAndDropContext::UpdatePosition, this, rMousePosition, eMode, false));

    if (!bDoAutoScroll)
    {
        pInsertionIndicatorHandler->UpdatePosition(aMouseModelPosition, eMode);

        
        mnInsertionIndex = pInsertionIndicatorHandler->GetInsertionPageIndex();
        if (pInsertionIndicatorHandler->IsInsertionTrivial(mnInsertionIndex, eMode))
            mnInsertionIndex = -1;
    }
}




void DragAndDropContext::SetTargetSlideSorter (
    SlideSorter* pSlideSorter,
    const Point aMousePosition,
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
            aMousePosition,
            eMode);

    }
}


} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
