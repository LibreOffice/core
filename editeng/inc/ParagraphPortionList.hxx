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

#include "ParagraphPortion.hxx"

#include <vector>

class EditDoc;

class ParaPortionList
{
    mutable sal_Int32 nLastCache = 0;
    typedef std::vector<std::unique_ptr<ParaPortion>> ParaPortionContainerType;
    ParaPortionContainerType maPortions;

public:
    ParaPortionList() = default;

    void Reset();
    tools::Long GetYOffset(const ParaPortion* pPPortion) const;
    sal_Int32 FindParagraph(tools::Long nYOffset) const;

    const ParaPortion* SafeGetObject(sal_Int32 nPos) const
    {
        return exists(nPos) ? maPortions[nPos].get() : nullptr;
    }

    ParaPortion* SafeGetObject(sal_Int32 nPos)
    {
        return exists(nPos) ? maPortions[nPos].get() : nullptr;
    }

    sal_Int32 GetPos(const ParaPortion* p) const;

    ParaPortion& getRef(sal_Int32 nPosition) { return *maPortions[nPosition]; }
    ParaPortion const& getRef(sal_Int32 nPosition) const { return *maPortions[nPosition]; }

    std::unique_ptr<ParaPortion> Release(sal_Int32 nPos);
    void Remove(sal_Int32 nPos);
    void Insert(sal_Int32 nPos, std::unique_ptr<ParaPortion> p);
    void Append(std::unique_ptr<ParaPortion> p);
    sal_Int32 Count() const;
    sal_Int32 lastIndex() const { return Count() - 1; }

    bool exists(sal_Int32 nPosition) const
    {
        return nPosition >= 0 && o3tl::make_unsigned(nPosition) < maPortions.size()
               && maPortions[nPosition];
    }

    ParaPortionContainerType::iterator begin() { return maPortions.begin(); }
    ParaPortionContainerType::iterator end() { return maPortions.end(); }
    ParaPortionContainerType::const_iterator cbegin() const { return maPortions.cbegin(); }
    ParaPortionContainerType::const_iterator cend() const { return maPortions.cend(); }

    void MarkAllSelectionsInvalid(sal_Int32 nStart)
    {
        for (auto& pParaPortion : maPortions)
            pParaPortion->MarkSelectionInvalid(nStart);
    }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    // temporary:
    static void DbgCheck(ParaPortionList const&, EditDoc const& rDoc);
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
