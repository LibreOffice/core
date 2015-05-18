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

#ifndef INCLUDED_O3TL_QA_COW_WRAPPER_CLIENTS_HXX
#define INCLUDED_O3TL_QA_COW_WRAPPER_CLIENTS_HXX

#include "o3tl/cow_wrapper.hxx"

/* Definition of Cow_Wrapper_Clients classes */

namespace o3tltests {

/** This is a header and a separate compilation unit on purpose -
    cow_wrapper needs destructor, copy constructor and assignment
    operator to be outline, when pimpl idiom is used
 */

/// test non-opaque impl type
class cow_wrapper_client1
{
public:
    cow_wrapper_client1() : maImpl() {}
    explicit cow_wrapper_client1( int nVal ) : maImpl(nVal) {}

    void modify( int nVal ) { *maImpl = nVal; }
    int  queryUnmodified() const { return *maImpl; }

    void makeUnique() { maImpl.make_unique(); }
    bool is_unique() const { return maImpl.is_unique(); }
    oslInterlockedCount use_count() const { return maImpl.use_count(); }
    void swap( cow_wrapper_client1& r ) { o3tl::swap(maImpl, r.maImpl); }

    bool operator==( const cow_wrapper_client1& rRHS ) const { return maImpl == rRHS.maImpl; }
    bool operator!=( const cow_wrapper_client1& rRHS ) const { return maImpl != rRHS.maImpl; }
    bool operator<( const cow_wrapper_client1& rRHS ) const { return maImpl < rRHS.maImpl; }

private:
    o3tl::cow_wrapper< int > maImpl;
};


class cow_wrapper_client2_impl;

/** test opaque impl type - need to explicitly declare lifetime
    methods
 */
class cow_wrapper_client2
{
public:
    cow_wrapper_client2();
    explicit cow_wrapper_client2( int nVal );
    ~cow_wrapper_client2();

    cow_wrapper_client2( const cow_wrapper_client2& );
    cow_wrapper_client2& operator=( const cow_wrapper_client2& );

    void modify( int nVal );
    int  queryUnmodified() const;

    void makeUnique();
    bool is_unique() const;
    oslInterlockedCount use_count() const;
    void swap( cow_wrapper_client2& r );

    bool operator==( const cow_wrapper_client2& rRHS ) const;
    bool operator!=( const cow_wrapper_client2& rRHS ) const;
    bool operator<( const cow_wrapper_client2& rRHS ) const;

private:
    o3tl::cow_wrapper< cow_wrapper_client2_impl > maImpl;
};

/** test MT-safe cow_wrapper - basically the same as
    cow_wrapper_client2, only with different refcounting policy
 */
class cow_wrapper_client3
{
public:
    cow_wrapper_client3();
    explicit cow_wrapper_client3( int nVal );
    ~cow_wrapper_client3();

    cow_wrapper_client3( const cow_wrapper_client3& );
    cow_wrapper_client3& operator=( const cow_wrapper_client3& );

    void modify( int nVal );
    int  queryUnmodified() const;

    void makeUnique();
    bool is_unique() const;
    oslInterlockedCount use_count() const;
    void swap( cow_wrapper_client3& r );

    bool operator==( const cow_wrapper_client3& rRHS ) const;
    bool operator!=( const cow_wrapper_client3& rRHS ) const;
    bool operator<( const cow_wrapper_client3& rRHS ) const;

private:
    o3tl::cow_wrapper< cow_wrapper_client2_impl, o3tl::ThreadSafeRefCountingPolicy > maImpl;
};

/** test default-object comparison - have default-ctored-client4 share
    the same static impl instance, check if isDefault does the right
    thing
 */
class cow_wrapper_client4
{
public:
    cow_wrapper_client4();
    explicit cow_wrapper_client4(int);
    ~cow_wrapper_client4();

    cow_wrapper_client4( const cow_wrapper_client4& );
    cow_wrapper_client4& operator=( const cow_wrapper_client4& );

    bool is_default() const;

    bool operator==( const cow_wrapper_client4& rRHS ) const;
    bool operator!=( const cow_wrapper_client4& rRHS ) const;
    bool operator<( const cow_wrapper_client4& rRHS ) const;

private:
    o3tl::cow_wrapper< int > maImpl;
};

}

#endif // INCLUDED_O3TL_QA_COW_WRAPPER_CLIENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
