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

#include "controller/SlsTransferableData.hxx"

#include "SlideSorterViewShell.hxx"
#include "View.hxx"

namespace sd { namespace slidesorter { namespace controller {

SdTransferable* TransferableData::CreateTransferable (
    SdDrawDocument* pSrcDoc,
    ::sd::View* pWorkView,
    bool bInitOnGetData,
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
    if (pTransferable)
    {
        for (sal_Int32 nIndex=0,nCount=pTransferable->GetUserDataCount(); nIndex<nCount; ++nIndex)
        {
            ::boost::shared_ptr<TransferableData> xData =
                ::boost::dynamic_pointer_cast<TransferableData>(pTransferable->GetUserData(nIndex));
            if (xData)
                return xData;
        }
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

TransferableData::~TransferableData()
{
    if (mpViewShell != NULL)
        EndListening(*mpViewShell);
}

void TransferableData::Notify (SfxBroadcaster&, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (pSimpleHint && mpViewShell)
    {
        if (pSimpleHint->GetId() == SFX_HINT_DYING)
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

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
