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


#include "controller/SlsTransferableData.hxx"

#include "SlideSorterViewShell.hxx"
#include "View.hxx"

namespace sd { namespace slidesorter { namespace controller {

SdTransferable* TransferableData::CreateTransferable (
    SdDrawDocument* pSrcDoc,
    ::sd::View* pWorkView,
    sal_Bool bInitOnGetData,
    SlideSorterViewShell* pViewShell,
    const ::std::vector<Representative>& rRepresentatives)
{
    SdTransferable* pTransferable = new SdTransferable (pSrcDoc, pWorkView, bInitOnGetData);
    ::boost::shared_ptr<TransferableData> pData (new TransferableData(pViewShell, rRepresentatives));
    pTransferable->AddUserData(pData);
    return pTransferable;
}




::boost::shared_ptr<TransferableData> TransferableData::GetFromTransferable (const SdTransferable* pTransferable)
{
    ::boost::shared_ptr<TransferableData> pData;
    for (sal_Int32 nIndex=0,nCount=pTransferable->GetUserDataCount(); nIndex<nCount; ++nIndex)
    {
        pData = ::boost::dynamic_pointer_cast<TransferableData>(pTransferable->GetUserData(nIndex));
        if (pData)
            return pData;
    }
    return ::boost::shared_ptr<TransferableData>();
}




TransferableData::TransferableData (
    SlideSorterViewShell* pViewShell,
    const ::std::vector<Representative>& rRepresentatives)
    : mpViewShell(pViewShell),
      maRepresentatives(rRepresentatives)
{
    if (mpViewShell != NULL)
        StartListening(*mpViewShell);
}




TransferableData::~TransferableData (void)
{
    if (mpViewShell != NULL)
        EndListening(*mpViewShell);
}




void TransferableData::DragFinished (sal_Int8 nDropAction)
{
    if (mpViewShell != NULL)
        mpViewShell->DragFinished(nDropAction);
    /*
    for (CallbackContainer::const_iterator
             iCallback(maDragFinishCallbacks.begin()),
             iEnd(maDragFinishCallbacks.end());
         iCallback!=iEnd;
         ++iCallback)
    {
        if (*iCallback)
            (*iCallback)(nDropAction);
    }
    maDragFinishCallbacks.clear();
    */
}




void TransferableData::Notify (SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.ISA(SfxSimpleHint) && mpViewShell!=NULL)
    {
        SfxSimpleHint& rSimpleHint (*PTR_CAST(SfxSimpleHint, &rHint));
        if (rSimpleHint.GetId() == SFX_HINT_DYING)
        {
            
            
            
            
            EndListening(*mpViewShell);
            mpViewShell = NULL;
        }
    }
}




const ::std::vector<TransferableData::Representative>& TransferableData::GetRepresentatives (void) const
{
    return maRepresentatives;
}




SlideSorterViewShell* TransferableData::GetSourceViewShell (void) const
{
    return mpViewShell;
}

} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
