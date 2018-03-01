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

#ifndef __SC_QUERYENTRY_HXX__
#define __SC_QUERYENTRY_HXX__

#include "address.hxx"
#include "global.hxx"

#include <vector>

namespace utl {
    class SearchParam;
    class TextSearch;
}

/**
 * Each instance of this struct represents a single filtering criteria.
 */
struct SC_DLLPUBLIC ScQueryEntry
{
    enum QueryType { ByValue, ByString, ByDate, ByEmpty };

    struct Item
    {
        QueryType     meType;
        double        mfVal;
        OUString maString;

        Item() : meType(ByValue), mfVal(0.0) {}

        bool operator== (const Item& r) const;
    };
    typedef std::vector<Item> QueryItemsType;

    bool            bDoQuery;
    SCCOLROW        nField;
    ScQueryOp       eOp;
    ScQueryConnect  eConnect;
    mutable utl::SearchParam* pSearchParam;       // if RegExp, not saved
    mutable utl::TextSearch*  pSearchText;        // if RegExp, not saved

    ScQueryEntry();
    ScQueryEntry(const ScQueryEntry& r);
    ~ScQueryEntry();

    // creates pSearchParam and pSearchText if necessary, always RegExp!
    utl::TextSearch* GetSearchTextPtr( bool bCaseSens ) const;

    QueryItemsType& GetQueryItems();
    const QueryItemsType& GetQueryItems() const;
    void SetQueryByEmpty();
    bool IsQueryByEmpty() const;
    void SetQueryByNonEmpty();
    bool IsQueryByNonEmpty() const;
    const Item& GetQueryItem() const;
    Item& GetQueryItem();
    void            Clear();
    ScQueryEntry&   operator=( const ScQueryEntry& r );
    bool            operator==( const ScQueryEntry& r ) const;

private:
    /**
     * Stores all query items.  It must contain at least one item at all times
     * (for single equality match queries or comparative queries).  It may
     * contain multiple items for multi-equality match queries.
     */
    mutable QueryItemsType maQueryItems;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
