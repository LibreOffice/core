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
#include <svx/svdpage.hxx>

/*
 * Definition of Properties Interface
*/

IMPL_CHAIN_PROP_INTERFACE(CursorEvent, CursorChainingEvent)
IMPL_CHAIN_PROP_INTERFACE(NilChainingEvent, bool)
IMPL_CHAIN_PROP_INTERFACE(PreChainingSel, ESelection)
IMPL_CHAIN_PROP_INTERFACE(PostChainingSel, ESelection)
IMPL_CHAIN_PROP_INTERFACE(IsPartOfLastParaInNextLink, bool)

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

SdrTextObj *TextChain::GetNextLink(const SdrTextObj *pTextObj) const
{
    return impGetNextLink(pTextObj);
}

SdrTextObj *TextChain::GetPrevLink(const SdrTextObj *pTextObj) const
{
    return impGetPrevLink(pTextObj);
}

SdrTextObj *TextChain::impGetNextLink(const SdrTextObj *pTextObj) const
{
    // XXX: Temporary implementation based on index number

    SdrTextObj *pNextTextObj = NULL;
    SdrPage *pPage = pTextObj->pPage;

    if ( pPage && pPage->GetObjCount() > 1) {

        sal_uInt32 nextIndex = (pTextObj->GetOrdNum()+1);

        if (nextIndex < pPage->GetObjCount())
            pNextTextObj =  dynamic_cast< SdrTextObj * >( pPage->GetObj( nextIndex ) );

        return pNextTextObj;
    } else {
        fprintf(stderr, "Make New Object please\n");
        return NULL;
    }
}

SdrTextObj *TextChain::impGetPrevLink(const SdrTextObj *pTextObj) const
{
    return NULL;
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
