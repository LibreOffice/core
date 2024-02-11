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

#include "TextPortion.hxx"
#include <vector>

class TextPortionList
{
    typedef std::vector<std::unique_ptr<TextPortion>> PortionsType;
    PortionsType maPortions;

public:
    TextPortionList() = default;

    void Reset() { maPortions.clear(); }

    sal_Int32 FindPortion(sal_Int32 nCharPos, sal_Int32& rPortionStart,
                          bool bPreferStartingPortion = false) const;
    sal_Int32 GetStartPos(sal_Int32 nPortion);
    void DeleteFromPortion(sal_Int32 nDelFrom);

    sal_Int32 Count() const { return sal_Int32(maPortions.size()); }

    const TextPortion& operator[](sal_Int32 nPosition) const { return *maPortions[nPosition]; }

    TextPortion& operator[](sal_Int32 nPosition) { return *maPortions[nPosition]; }

    void Append(TextPortion* pTextPortion)
    {
        maPortions.push_back(std::unique_ptr<TextPortion>(pTextPortion));
    }

    void Insert(sal_Int32 nPosition, TextPortion* pTextPortion)
    {
        maPortions.insert(maPortions.begin() + nPosition,
                          std::unique_ptr<TextPortion>(pTextPortion));
    }

    void Remove(sal_Int32 nPosition) { maPortions.erase(maPortions.begin() + nPosition); }

    sal_Int32 GetPos(const TextPortion* p) const;

    bool isEmpty() { return Count() == 1 && maPortions[0]->GetLen() == 0; }

    PortionsType::iterator begin() { return maPortions.begin(); }
    PortionsType::iterator end() { return maPortions.end(); }
    PortionsType::const_iterator cbegin() const { return maPortions.cbegin(); }
    PortionsType::const_iterator cend() const { return maPortions.cend(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
