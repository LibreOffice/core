/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MultipleItemConverter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:30:11 $
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
#include "MultipleItemConverter.hxx"
#include "ItemPropertyMap.hxx"

#include <algorithm>

using namespace ::com::sun::star;

namespace comphelper
{

MultipleItemConverter::MultipleItemConverter( SfxItemPool& rItemPool )
        : ItemConverter( NULL, rItemPool )
{
}
MultipleItemConverter::~MultipleItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     DeleteItemConverterPtr() );
}

void MultipleItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::vector< ItemConverter* >::const_iterator       aIter = m_aConverters.begin();
    const ::std::vector< ItemConverter* >::const_iterator aEnd  = m_aConverters.end();
    if( aIter != aEnd )
    {
        (*aIter)->FillItemSet( rOutItemSet );
        aIter++;
    }
    for( ; aIter != aEnd; aIter++ )
    {
        SfxItemSet aSet = this->CreateEmptyItemSet();
        (*aIter)->FillItemSet( aSet );
        InvalidateUnequalItems( rOutItemSet, aSet );
    }
    // no own items
}

bool MultipleItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ApplyItemSetFunc( rItemSet, bResult ));

    // no own items
    return bResult;
}

bool MultipleItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    return false;
}

} //  namespace comphelper
