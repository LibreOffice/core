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

#include "stringlistitem.hxx"

namespace dbaui
{

using namespace ::com::sun::star::uno;

// OStringListItem
TYPEINIT1(OStringListItem, SfxPoolItem);
OStringListItem::OStringListItem(sal_Int16 _nWhich, const Sequence< OUString >& _rList)
    :SfxPoolItem(_nWhich)
    ,m_aList(_rList)
{
}

OStringListItem::OStringListItem(const OStringListItem& _rSource)
    :SfxPoolItem(_rSource)
    ,m_aList(_rSource.m_aList)
{
}

int OStringListItem::operator==(const SfxPoolItem& _rItem) const
{
    const OStringListItem* pCompare = PTR_CAST(OStringListItem, &_rItem);
    if ((!pCompare) || (pCompare->m_aList.getLength() != m_aList.getLength()))
        return 0;

    // compare all strings individually
    const OUString* pMyStrings = m_aList.getConstArray();
    const OUString* pCompareStrings = pCompare->m_aList.getConstArray();

    for (sal_Int32 i=0; i<m_aList.getLength(); ++i, ++pMyStrings, ++pCompareStrings)
        if (!pMyStrings->equals(*pCompareStrings))
            return 0;

    return 1;
}

SfxPoolItem* OStringListItem::Clone(SfxItemPool* /* _pPool */) const
{
    return new OStringListItem(*this);
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
