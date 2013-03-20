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
#if 1

#include <tools/stream.hxx>
#include "scdllapi.h"

#include <boost/ptr_container/ptr_vector.hpp>

/**
 * Stores individual user-defined sort list.
 */
class SC_DLLPUBLIC ScUserListData
{
public:
    struct SubStr
    {
        ::rtl::OUString maReal;
        ::rtl::OUString maUpper;
        SubStr(const ::rtl::OUString& rReal, const ::rtl::OUString& rUpper);
    };
private:
    typedef ::boost::ptr_vector<SubStr> SubStringsType;
    SubStringsType maSubStrings;
    ::rtl::OUString aStr;

    SC_DLLPRIVATE void  InitTokens();

public:
    ScUserListData(const ::rtl::OUString& rStr);
    ScUserListData(const ScUserListData& rData);
    ~ScUserListData();

    const ::rtl::OUString& GetString() const { return aStr; }
    void SetString(const ::rtl::OUString& rStr);
    size_t GetSubCount() const;
    bool GetSubIndex(const ::rtl::OUString& rSubStr, sal_uInt16& rIndex) const;
    ::rtl::OUString GetSubStr(sal_uInt16 nIndex) const;
    StringCompare Compare(const ::rtl::OUString& rSubStr1, const ::rtl::OUString& rSubStr2) const;
    StringCompare ICompare(const ::rtl::OUString& rSubStr1, const ::rtl::OUString& rSubStr2) const;
};

/**
 * Collection of user-defined sort lists.
 */
class SC_DLLPUBLIC ScUserList
{
    typedef ::boost::ptr_vector<ScUserListData> DataType;
    DataType maData;
public:
    typedef DataType::iterator iterator;
    typedef DataType::const_iterator const_iterator;

    ScUserList();
    ScUserList(const ScUserList& r);

    const ScUserListData* GetData( const ::rtl::OUString& rSubStr ) const;
    /// If the list in rStr is already inserted
    bool HasEntry( const ::rtl::OUString& rStr ) const;

    const ScUserListData*  operator[](size_t nIndex) const;
    ScUserListData*  operator[](size_t nIndex);
    ScUserList&     operator= ( const ScUserList& r );
    bool            operator==( const ScUserList& r ) const;
    bool            operator!=( const ScUserList& r ) const;

    iterator begin();
    const_iterator begin() const;
    void clear();
    size_t size() const;
    void push_back(ScUserListData* p);
    void erase(iterator itr);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
