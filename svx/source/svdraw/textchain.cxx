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

#include <svx/textchain.hxx>
#include <svx/svdotext.hxx>

/*
 * Definition of Properties Interface
*/

IMPL_CHAIN_PROP_INTERFACE(CursorEvent, CursorChainingEvent)
IMPL_CHAIN_PROP_INTERFACE(NilChainingEvent, bool)
IMPL_CHAIN_PROP_INTERFACE(JustHandledUnderflow, bool)
IMPL_CHAIN_PROP_INTERFACE(PreChainingSel, ESelection)
IMPL_CHAIN_PROP_INTERFACE(PostChainingSel, ESelection)

/* End Definition of Properties Interface */

/* TextChain */

// NOTE: All getters in the class assume that the guy is in the chain

TextChain::TextChain()
{
}

TextChain::~TextChain()
{
    // XXX: Should free all LinkProperties
}

bool TextChain::IsLinkInChain(SdrTextObj *) const
{
    return true;    // XXX: Should make an actual check
}

void TextChain::AppendLink(SdrTextObj *)
{
    // XXX
}

SdrTextObj *TextChain::GetNextLink(SdrTextObj *) const
{
    return NULL; // XXX: To be changed. It'd be a mess to implement now
}


bool TextChain::GetLinksHaveMergeableFirstPara(SdrTextObj* /* pPrevLink */, SdrTextObj* /* pNextLink */)
{
    // XXX
    return false;
}


ImpChainLinkProperties *TextChain::GetLinkProperties(const SdrTextObj *pLink)
{
    // if the guy does not already have properties in the map make them
    ChainLinkId aLinkId = GetId(pLink);
    if (maLinkPropertiesMap.find(aLinkId) == maLinkPropertiesMap.end()) {
        maLinkPropertiesMap[aLinkId] = new ImpChainLinkProperties;
    }

    return maLinkPropertiesMap[aLinkId];
}

ChainLinkId TextChain::GetId(const SdrTextObj *pLink) const
{
    return pLink->GetName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
