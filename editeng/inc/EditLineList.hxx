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

#include "EditLine.hxx"

#include <memory>
#include <vector>
#include <sal/types.h>
#include <tools/debug.hxx>

class EditLineList
{
    typedef std::vector<std::unique_ptr<EditLine>> LinesType;
    LinesType maLines;

public:
    EditLineList() = default;

    void Reset() { maLines.clear(); }

    void DeleteFromLine(sal_Int32 nDelFrom)
    {
        assert(nDelFrom <= (static_cast<sal_Int32>(maLines.size()) - 1));
        LinesType::iterator it = maLines.begin();
        std::advance(it, nDelFrom);
        maLines.erase(it, maLines.end());
    }

    sal_Int32 FindLine(sal_Int32 nChar, bool bInclEnd)
    {
        sal_Int32 n = maLines.size();
        for (sal_Int32 i = 0; i < n; ++i)
        {
            const EditLine& rLine = *maLines[i];
            if ((bInclEnd && (rLine.GetEnd() >= nChar)) || (rLine.GetEnd() > nChar))
            {
                return i;
            }
        }

        DBG_ASSERT(!bInclEnd, "Line not found: FindLine");
        return n - 1;
    }

    sal_Int32 Count() const { return maLines.size(); }
    const EditLine& operator[](sal_Int32 nPos) const { return *maLines[nPos]; }
    EditLine& operator[](sal_Int32 nPos) { return *maLines[nPos]; }

    void Append(std::unique_ptr<EditLine> pEditLine) { maLines.push_back(std::move(pEditLine)); }

    void Insert(sal_Int32 nPos, std::unique_ptr<EditLine> pEditLine)
    {
        maLines.insert(maLines.begin() + nPos, std::move(pEditLine));
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
