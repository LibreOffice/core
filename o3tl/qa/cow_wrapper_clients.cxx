/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cow_wrapper_clients.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 10:15:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
