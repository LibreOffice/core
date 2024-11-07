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

#include "scdllapi.h"

#include <rtl/ustring.hxx>

#include <vector>

/**
 * Stores individual user-defined sort list.
 */
class SC_DLLPUBLIC ScUserListData final
{
private:
    struct SAL_DLLPRIVATE SubStr
    {
        OUString maReal;
        OUString maUpper;
        SubStr(OUString&& aReal);
    };

    std::vector<SubStr> maSubStrings;
    OUString aStr;

    SAL_DLLPRIVATE void InitTokens();

public:
    ScUserListData(OUString aStr);

    const OUString& GetString() const { return aStr; }
    void SetString(const OUString& rStr);
    size_t GetSubCount() const { return maSubStrings.size(); }
    bool GetSubIndex(const OUString& rSubStr, sal_uInt16& rIndex, bool& bMatchCase) const;
    const OUString& GetSubStr(sal_uInt16 nIndex) const;
    sal_Int32 Compare(const OUString& rSubStr1, const OUString& rSubStr2) const;
    sal_Int32 ICompare(const OUString& rSubStr1, const OUString& rSubStr2) const;
};

/**
 * Collection of user-defined sort lists.
 */
class SC_DLLPUBLIC ScUserList
{
    typedef std::vector<ScUserListData> DataType;
    DataType maData;

public:
    explicit ScUserList(bool initDefault = true);
    ScUserList(const ScUserList& r) = default;

    void AddDefaults();
    void EraseData(size_t nIndex) { maData.erase(maData.cbegin() + nIndex); }

    const ScUserListData* GetData(const OUString& rSubStr) const;
    /// If the list in rStr is already inserted
    bool HasEntry(std::u16string_view rStr) const;

    const ScUserListData& operator[](size_t nIndex) const { return maData[nIndex]; }
    ScUserListData& operator[](size_t nIndex) { return maData[nIndex]; }
    ScUserList& operator=(const ScUserList& r) = default;
    bool operator==(const ScUserList& r) const;
    bool operator!=(const ScUserList& r) const { return !operator==(r); }

    void clear() { maData.clear(); }
    size_t size() const { return maData.size(); }
    template <class... Args> void emplace_back(Args&&... args)
    {
        maData.emplace_back(std::forward<Args>(args)...);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
