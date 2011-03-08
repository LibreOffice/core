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




void TransferableData::Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
{
    if (rHint.ISA(SfxSimpleHint) && mpViewShell!=NULL)
    {
        SfxSimpleHint& rSimpleHint (*PTR_CAST(SfxSimpleHint, &rHint));
        if (rSimpleHint.GetId() == SFX_HINT_DYING)
        {
            // This hint may come either from the ViewShell or from the
            // document (registered by SdTransferable).  We do not know
            // which but both are sufficient to disconnect from the
            // ViewShell.
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

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
