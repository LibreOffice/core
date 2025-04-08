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

#pragma once

#include <editeng/editdata.hxx>
#include <map>

class SdrTextObj;

typedef OUString ChainLinkId;

enum class CursorChainingEvent : sal_uInt8
{
    TO_NEXT_LINK,
    TO_PREV_LINK,
    UNCHANGED,
    NULL_EVENT
};

class ImpChainLinkProperties
{
protected:
    friend class TextChain;

    ImpChainLinkProperties() = default;

private:
    // NOTE: Remember to set default value in constructor when adding field
    ESelection aPreChainingSel;
    ESelection aPostChainingSel;
    CursorChainingEvent aCursorEvent = CursorChainingEvent::NULL_EVENT;
    bool aNilChainingEvent = false;
    bool aIsPartOfLastParaInNextLink = false;
    bool aSwitchingToNextBox = false;
};

class TextChain
{
public:
    ~TextChain();

    ImpChainLinkProperties* GetLinkProperties(const SdrTextObj*);

    // Specific Link Properties
    CursorChainingEvent const& GetCursorEvent(const SdrTextObj*);
    void SetCursorEvent(const SdrTextObj*, CursorChainingEvent const&);

    bool GetNilChainingEvent(const SdrTextObj*);
    void SetNilChainingEvent(const SdrTextObj*, bool);

    ESelection const& GetPreChainingSel(const SdrTextObj*);
    void SetPreChainingSel(const SdrTextObj*, ESelection const&);

    ESelection const& GetPostChainingSel(const SdrTextObj*);
    void SetPostChainingSel(const SdrTextObj*, ESelection const&);

    // return whether a paragraph is split between this box and the next
    bool GetIsPartOfLastParaInNextLink(const SdrTextObj*);
    void SetIsPartOfLastParaInNextLink(const SdrTextObj*, bool);

    // return whether we are currently moving the cursor to the next box (useful to know whether we should prevent SetOutlinerParaObject invocations in SdrTextObj::EndTextEdit)
    bool GetSwitchingToNextBox(const SdrTextObj*);
    void SetSwitchingToNextBox(const SdrTextObj*, bool);

protected:
    TextChain();

private:
    std::map<ChainLinkId, ImpChainLinkProperties*> maLinkPropertiesMap;

    friend class SdrModel;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
