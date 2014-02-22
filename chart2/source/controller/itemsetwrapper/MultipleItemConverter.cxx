/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
    
}

bool MultipleItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ApplyItemSetFunc( rItemSet, bResult ));

    
    return bResult;
}

bool MultipleItemConverter::GetItemProperty( tWhichIdType /*nWhichId*/, tPropertyNameWithMemberId & /*rOutProperty*/ ) const
{
    return false;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
