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

#include <sfx2/securityitem.hxx>

SfxPoolItem* SfxSecurityItem::CreateDefault()
{
     return new SfxSecurityItem;
}

SfxSecurityItem::SfxSecurityItem()
{

}

SfxSecurityItem::SfxSecurityItem( const SfxSecurityItem& rItem ) :
    SfxPoolItem ( rItem )
{
    for (auto it = rItem.m_aOptions.begin(); it != rItem.m_aOptions.end(); ++it)
    {
        m_aOptions.emplace(std::make_pair(it->first, css::uno::Any(it->second)));
    }
}

bool SfxSecurityItem::QueryValue( css::uno::Any& rValue, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    std::map<sal_uInt8, css::uno::Any>::const_iterator aIter( m_aOptions.find( nMemberId ) );

    if ( m_aOptions.end() != aIter )
    {
        rValue = (*aIter).second;
        return true;
    }

    return false;
}

bool SfxSecurityItem::PutValue( const css::uno::Any& rValue, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    m_aOptions[ nMemberId ] = rValue;
    return true;
}

SfxPoolItem* SfxSecurityItem::Clone( SfxItemPool * ) const
{
    return new SfxSecurityItem( *this );
}

bool SfxSecurityItem::operator==( const SfxPoolItem& rItem) const
{
    assert( SfxPoolItem::operator==(rItem));
    const SfxSecurityItem &rSec = static_cast<const SfxSecurityItem &>(rItem);
    if (rSec.m_aOptions.size() == m_aOptions.size())
    {
        auto itFirst = m_aOptions.begin();
        auto itSecond = rSec.m_aOptions.begin();
        for ( ; itFirst != m_aOptions.end(); ++itFirst)
        {
            itSecond = rSec.m_aOptions.find(itFirst->first);
            if (itSecond != rSec.m_aOptions.end() &&
                itSecond->second != itFirst->second)
                break;
        }

        return itFirst == m_aOptions.end();
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
