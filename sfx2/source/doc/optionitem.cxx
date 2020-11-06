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

#include <sfx2/optionitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>

#include <sfx2/sfxsids.hrc>

SfxPoolItem* SfxOptionItem::CreateDefault()
{
    return new SfxOptionItem(0);
}

SfxOptionItem::SfxOptionItem(const sal_uInt16 nWhich) :
    SfxPoolItem(nWhich)
{

}

SfxOptionItem::SfxOptionItem( const SfxOptionItem& rItem ) :
    SfxPoolItem ( rItem )
{
    for (auto it = rItem.m_aOptions.begin(); it != rItem.m_aOptions.end(); ++it)
    {
        m_aOptions.emplace(std::make_pair(it->first, it->second));
    }
}

bool SfxOptionItem::QueryValue( css::uno::Any& rValue, sal_uInt8 nMemberId ) const
{
    if (m_aOptions.size() == 0)
        return false;

    nMemberId &= ~CONVERT_TWIPS;
    if (nMemberId == 0)
    {
        int nIndex = 0;
        css::uno::Sequence< css::uno::Any > aSeq( static_cast<sal_Int32>(m_aOptions.size()) );

        for (auto it = m_aOptions.begin(); it != m_aOptions.end(); ++it, ++nIndex)
        {
            aSeq[nIndex] = css::uno::makeAny(it->second);
        }
        rValue <<= aSeq;

        return true;
    }
    else
    {
        auto found = m_aOptions.find(nMemberId);
        if (found != m_aOptions.end())
        {
            rValue <<= found->second;
            return true;
        }
    }

    return false;
}

bool SfxOptionItem::PutValue( const css::uno::Any& rValue, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    css::beans::NamedValue aProp;

    switch (nMemberId)
    {
    case MID_MACRO_SECURITYLEVEL:
        aProp.Name = "MacroSecurityLevel";
        aProp.Value = rValue;
        break;
    }

    if (!aProp.Name.isEmpty())
    {
        m_aOptions[nMemberId] = aProp;
        return true;
    }

    return false;
}

SfxPoolItem* SfxOptionItem::Clone( SfxItemPool * ) const
{
    return new SfxOptionItem( *this );
}

bool SfxOptionItem::operator==( const SfxPoolItem& rItem) const
{
    assert( SfxPoolItem::operator==(rItem));
    const SfxOptionItem &rSec = static_cast<const SfxOptionItem &>(rItem);
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
