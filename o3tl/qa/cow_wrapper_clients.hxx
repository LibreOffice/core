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

#ifndef INCLUDED_COW_WRAPPER_CLIENTS_HXX
#define INCLUDED_COW_WRAPPER_CLIENTS_HXX

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

/** test opaque impl type - need to explicitely declare lifetime
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

} // namespace o3tltests

#endif /* INCLUDED_COW_WRAPPER_CLIENTS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
