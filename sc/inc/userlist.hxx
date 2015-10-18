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

#ifndef INCLUDED_SC_INC_USERLIST_HXX
#define INCLUDED_SC_INC_USERLIST_HXX

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
        OUString maReal;
        OUString maUpper;
        SubStr(const OUString& rReal, const OUString& rUpper);
    };
private:
    typedef ::boost::ptr_vector<SubStr> SubStringsType;
    SubStringsType maSubStrings;
    OUString aStr;

    SAL_DLLPRIVATE void  InitTokens();

public:
    ScUserListData(const OUString& rStr);
    ScUserListData(const ScUserListData& rData);
    ~ScUserListData();

    const OUString& GetString() const { return aStr; }
    void SetString(const OUString& rStr);
    size_t GetSubCount() const;
    bool GetSubIndex(const OUString& rSubStr, sal_uInt16& rIndex, bool& bMatchCase) const;
    OUString GetSubStr(sal_uInt16 nIndex) const;
    sal_Int32 Compare(const OUString& rSubStr1, const OUString& rSubStr2) const;
    sal_Int32 ICompare(const OUString& rSubStr1, const OUString& rSubStr2) const;
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

    const ScUserListData* GetData( const OUString& rSubStr ) const;
    /// If the list in rStr is already inserted
    bool HasEntry( const OUString& rStr ) const;

    const ScUserListData&  operator[](size_t nIndex) const;
    ScUserListData&  operator[](size_t nIndex);
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
