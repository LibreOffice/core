/*************************************************************************
 *
 *  $RCSfile: cow_wrapper_clients.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-02-01 10:55:36 $
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

} // namespace o3tltests
