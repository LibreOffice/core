/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "FieldSlot.hxx"
#include "porfld.hxx"

FieldSlot::FieldSlot(const SwTextFormatInfo* pNew, SwTextPortion* pPor)
    : pOldText(nullptr)
    , nIdx(0)
    , nLen(0)
    , pInf(nullptr)
{
    bOn = pPor->GetExpText(*pNew, aText);

    // The text will be replaced ...
    if (!bOn)
        return;

    pInf = const_cast<SwTextFormatInfo*>(pNew);
    nIdx = pInf->GetIdx();
    nLen = pInf->GetLen();
    pOldText = &(pInf->GetText());
    m_pOldCachedVclData = pInf->GetCachedVclData();
    pInf->SetLen(TextFrameIndex(aText.getLength()));
    pInf->SetCachedVclData(nullptr);

    SwFieldPortion* pFieldPor = dynamic_cast<SwFieldPortion*>(pPor);
    if (pFieldPor && pFieldPor->IsFollow())
    {
        pInf->SetFakeLineStart(nIdx > pInf->GetLineStart());
        pInf->SetIdx(TextFrameIndex(0));
    }
    else if (nIdx < TextFrameIndex(pOldText->getLength()))
    {
        aText = (*pOldText).replaceAt(sal_Int32(nIdx), 1, aText);
    }
    pInf->SetText(aText);
}

FieldSlot::~FieldSlot()
{
    if (bOn)
    {
        pInf->SetCachedVclData(m_pOldCachedVclData);
        pInf->SetText(*pOldText);
        pInf->SetIdx(nIdx);
        pInf->SetLen(nLen);
        pInf->SetFakeLineStart(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
