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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
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
        ++aIter;
    }
    for( ; aIter != aEnd; ++aIter )
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

bool MultipleItemConverter::GetItemProperty( tWhichIdType /*nWhichId*/, tPropertyNameWithMemberId & /*rOutProperty*/ ) const
{
    return false;
}

} //  namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
