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
#include "cppunit/extensions/HelperMacros.h"

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

/** test default-object comparison - have default-stored-client4 share
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

// singleton ref-counting policy used to keep track of when
// incrementing and decrementing occurs
struct BogusRefCountPolicy
{
    static bool s_bShouldIncrement;
    static bool s_bShouldDecrement;
    static sal_uInt32 s_nEndOfScope;
    typedef sal_uInt32 ref_count_t;
    static void incrementCount( ref_count_t& rCount ) {
        if(s_bShouldIncrement)
        {
            ++rCount;
            s_bShouldIncrement = false;
        }
        else
            CPPUNIT_FAIL("Ref-counting policy incremented when it should not have.");
    }
    static bool decrementCount( ref_count_t& rCount ) {
        if(s_nEndOfScope)
        {
            --rCount;
            --s_nEndOfScope;
        }
        else if(s_bShouldDecrement)
        {
            --rCount;
            s_bShouldDecrement = false;
        }
        else
            CPPUNIT_FAIL("Ref-counting policy decremented when it should not have.");
        return rCount != 0;
    }
};

class cow_wrapper_client5
{
public:
    cow_wrapper_client5();
    explicit cow_wrapper_client5(int);
    ~cow_wrapper_client5();

    cow_wrapper_client5( const cow_wrapper_client5& );
    cow_wrapper_client5( cow_wrapper_client5&& );
    cow_wrapper_client5& operator=( const cow_wrapper_client5& );
    cow_wrapper_client5& operator=( cow_wrapper_client5&& );

    sal_uInt32 use_count() const { return maImpl.use_count(); }

    bool operator==( const cow_wrapper_client5& rRHS ) const;
    bool operator!=( const cow_wrapper_client5& rRHS ) const;

private:
    o3tl::cow_wrapper< int, BogusRefCountPolicy > maImpl;
};
} // namespace o3tltests

#endif // INCLUDED_O3TL_QA_COW_WRAPPER_CLIENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
