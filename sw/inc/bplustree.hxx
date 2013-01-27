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

#ifndef SW_BPLUSTREE_HXX
#define SW_BPLUSTREE_HXX

#include <tools/solar.h>
#include <osl/diagnose.h>
#include <swdllapi.h>

/** B+ Tree implementation (to replace the original BigPtrArray).

For more information about B+ Tree, please see eg. wikipedia:
http://en.wikipedia.org/wiki/B%2B_tree

As part of the removal of BigPtrArray (and consequent refactor of the related
code), this BPlusTree is supposed to be a drop-in replacement, with some of
the functionality templatized for easier use.

Key is sal_uLong in the BigPtrArray implementation.
Value is supposed to be SwNodePtr initially.
*/
template < class Key, class Value >
class SW_DLLPUBLIC BPlusTree
{
public:
    /// Callback function to be called during ForEach.
    typedef bool (*FnForEach)( const Value&, void* pArgs );

public:
    BPlusTree();
    ~BPlusTree();

    /// Number of elements.
    Key Count() const;

    /// Insert entry at the specified position.
    void Insert( const Value& r, Key pos );

    /// Remove n entries starting with the position pos.
    void Remove( Key pos, Key n = 1 );

    /// Insert the value of 'from' to the position 'to', and remove the original value.
    void Move( Key from, Key to );

    /// Exchange the value on position pos with the new one.
    void Replace( Key pos, const Value& r);

    /// Field access.
    const Value& operator[]( Key ) const;

    /// Traverse over the entire data, and call fn on the data.
    void ForEach( FnForEach fn, void* pArgs = NULL );

    /// Traverse over the specified range, and call fn on the data.
    void ForEach( Key nStart, Key nEnd, FnForEach fn, void* pArgs = NULL );
};

#endif // SW_BPLUSTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
