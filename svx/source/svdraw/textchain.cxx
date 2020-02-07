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

#include <textchain.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>

/*
 * Definition of Properties Interface
*/

CursorChainingEvent const & TextChain::GetCursorEvent(const SdrTextObj *pTarget)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    return pLinkProperties->aCursorEvent;
}
void TextChain::SetCursorEvent(const SdrTextObj *pTarget, CursorChainingEvent const & rPropParam)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    pLinkProperties->aCursorEvent = rPropParam;
}

bool TextChain::GetNilChainingEvent(const SdrTextObj *pTarget)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    return pLinkProperties->aNilChainingEvent;
}
void TextChain::SetNilChainingEvent(const SdrTextObj *pTarget, bool b)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    pLinkProperties->aNilChainingEvent = b;
}

ESelection const & TextChain::GetPreChainingSel(const SdrTextObj *pTarget)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    return pLinkProperties->aPreChainingSel;
}
void TextChain::SetPreChainingSel(const SdrTextObj *pTarget, ESelection const & rPropParam)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    pLinkProperties->aPreChainingSel = rPropParam;
}

ESelection const & TextChain::GetPostChainingSel(const SdrTextObj *pTarget)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    return pLinkProperties->aPostChainingSel;
}
void TextChain::SetPostChainingSel(const SdrTextObj *pTarget, ESelection const & rPropParam)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    pLinkProperties->aPostChainingSel = rPropParam;
}

bool TextChain::GetIsPartOfLastParaInNextLink(const SdrTextObj *pTarget)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    return pLinkProperties->aIsPartOfLastParaInNextLink;
}
void TextChain::SetIsPartOfLastParaInNextLink(const SdrTextObj *pTarget, bool b)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    pLinkProperties->aIsPartOfLastParaInNextLink = b;
}

bool TextChain::GetSwitchingToNextBox(const SdrTextObj *pTarget)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    return pLinkProperties->aSwitchingToNextBox;
}
void TextChain::SetSwitchingToNextBox(const SdrTextObj *pTarget, bool b)
{
    ImpChainLinkProperties *pLinkProperties = GetLinkProperties(pTarget);
    pLinkProperties->aSwitchingToNextBox = b;
}

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

namespace {

ChainLinkId GetId(const SdrTextObj *pLink)
{
    return pLink->GetName();
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
