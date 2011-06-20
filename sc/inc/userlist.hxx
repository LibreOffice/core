/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_USERLIST_HXX
#define SC_USERLIST_HXX

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
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    void clear();
    size_t size() const;
    void push_back(ScUserListData* p);
    void erase(iterator itr);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
