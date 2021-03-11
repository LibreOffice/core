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

#include "global.hxx"
#include <svl/sharedstring.hxx>
#include <unotools/textsearch.hxx>

#include <memory>
#include <vector>

/**
 * Each instance of this struct represents a single filtering criteria.
 */
struct SC_DLLPUBLIC ScQueryEntry
{
    enum QueryType { ByValue, ByString, ByDate, ByEmpty };

    struct SAL_DLLPRIVATE Item
    {
        QueryType     meType;
        double        mfVal;
        svl::SharedString maString;
        bool              mbMatchEmpty;
        bool              mbFormattedValue;

        Item() : meType(ByValue), mfVal(0.0), mbMatchEmpty(false), mbFormattedValue(false) {}

        bool operator== (const Item& r) const;
    };
    typedef std::vector<Item> QueryItemsType;

    bool            bDoQuery;
    SCCOLROW        nField;
    ScQueryOp       eOp;
    ScQueryConnect  eConnect;
    mutable std::unique_ptr<utl::SearchParam> pSearchParam;       ///< if Wildcard or RegExp, not saved
    mutable std::unique_ptr<utl::TextSearch>  pSearchText;        ///< if Wildcard or RegExp, not saved

    ScQueryEntry();
    ScQueryEntry(const ScQueryEntry& r);
    ~ScQueryEntry();

    /// creates pSearchParam and pSearchText if necessary
    utl::TextSearch* GetSearchTextPtr( utl::SearchParam::SearchType eSearchType, bool bCaseSens,
            bool bWildMatchSel ) const;

    QueryItemsType& GetQueryItems() { return maQueryItems;}
    const QueryItemsType& GetQueryItems() const { return maQueryItems;}
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
