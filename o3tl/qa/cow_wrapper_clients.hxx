/*************************************************************************
 *
 *  $RCSfile: cow_wrapper_clients.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-03-23 15:25:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
