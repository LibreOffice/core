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

#include "cow_wrapper_clients.hxx"

namespace o3tltests {

class cow_wrapper_client2_impl
{
public:
    cow_wrapper_client2_impl() : mnValue(0) {}
    explicit cow_wrapper_client2_impl( int nVal ) : mnValue(nVal) {}
    void setValue( int nVal ) { mnValue = nVal; }
    int  getValue() const { return mnValue; }

    bool operator==( const cow_wrapper_client2_impl& rRHS ) const { return mnValue == rRHS.mnValue; }
    bool operator!=( const cow_wrapper_client2_impl& rRHS ) const { return mnValue != rRHS.mnValue; }
    bool operator<( const cow_wrapper_client2_impl& rRHS ) const { return mnValue < rRHS.mnValue; }

private:
    int mnValue;
};

cow_wrapper_client2::cow_wrapper_client2() : maImpl()
{
}

cow_wrapper_client2::cow_wrapper_client2( int nVal ) :
    maImpl( cow_wrapper_client2_impl(nVal) )
{
}

cow_wrapper_client2::~cow_wrapper_client2()
{
}

cow_wrapper_client2::cow_wrapper_client2( const cow_wrapper_client2& rSrc ) :
    maImpl(rSrc.maImpl)
{
}

cow_wrapper_client2& cow_wrapper_client2::operator=( const cow_wrapper_client2& rSrc )
{
    maImpl = rSrc.maImpl;

    return *this;
}

void cow_wrapper_client2::modify( int nVal )
{
    maImpl->setValue( nVal );
}

int  cow_wrapper_client2::queryUnmodified() const
{
    return maImpl->getValue();
}

void cow_wrapper_client2::makeUnique()
{
    maImpl.make_unique();
}
bool cow_wrapper_client2::is_unique() const
{
    return maImpl.is_unique();
}
oslInterlockedCount cow_wrapper_client2::use_count() const
{
    return maImpl.use_count();
}
void cow_wrapper_client2::swap( cow_wrapper_client2& r )
{
    o3tl::swap(maImpl, r.maImpl);
}

bool cow_wrapper_client2::operator==( const cow_wrapper_client2& rRHS ) const
{
    return maImpl == rRHS.maImpl;
}
bool cow_wrapper_client2::operator!=( const cow_wrapper_client2& rRHS ) const
{
    return maImpl != rRHS.maImpl;
}
bool cow_wrapper_client2::operator<( const cow_wrapper_client2& rRHS ) const
{
    return maImpl < rRHS.maImpl;
}

// ---------------------------------------------------------------------------

cow_wrapper_client3::cow_wrapper_client3() : maImpl()
{
}

cow_wrapper_client3::cow_wrapper_client3( int nVal ) :
    maImpl( cow_wrapper_client2_impl(nVal) )
{
}

cow_wrapper_client3::~cow_wrapper_client3()
{
}

cow_wrapper_client3::cow_wrapper_client3( const cow_wrapper_client3& rSrc ) :
    maImpl(rSrc.maImpl)
{
}

cow_wrapper_client3& cow_wrapper_client3::operator=( const cow_wrapper_client3& rSrc )
{
    maImpl = rSrc.maImpl;

    return *this;
}

void cow_wrapper_client3::modify( int nVal )
{
    maImpl->setValue( nVal );
}

int  cow_wrapper_client3::queryUnmodified() const
{
    return maImpl->getValue();
}

void cow_wrapper_client3::makeUnique()
{
    maImpl.make_unique();
}
bool cow_wrapper_client3::is_unique() const
{
    return maImpl.is_unique();
}
oslInterlockedCount cow_wrapper_client3::use_count() const
{
    return maImpl.use_count();
}
void cow_wrapper_client3::swap( cow_wrapper_client3& r )
{
    o3tl::swap(maImpl, r.maImpl);
}

bool cow_wrapper_client3::operator==( const cow_wrapper_client3& rRHS ) const
{
    return maImpl == rRHS.maImpl;
}
bool cow_wrapper_client3::operator!=( const cow_wrapper_client3& rRHS ) const
{
    return maImpl != rRHS.maImpl;
}
bool cow_wrapper_client3::operator<( const cow_wrapper_client3& rRHS ) const
{
    return maImpl < rRHS.maImpl;
}

} // namespace o3tltests

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
