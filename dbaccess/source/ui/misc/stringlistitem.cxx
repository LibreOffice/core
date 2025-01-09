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

#include <stringlistitem.hxx>

namespace dbaui
{
using namespace ::com::sun::star::uno;

// OStringListItem
OStringListItem::OStringListItem(sal_Int16 _nWhich, const Sequence<OUString>& _rList)
    : SfxPoolItem(_nWhich)
    , m_aList(_rList)
{
}

OStringListItem::OStringListItem(const OStringListItem& _rSource)
    : SfxPoolItem(_rSource)
    , m_aList(_rSource.m_aList)
{
}

bool OStringListItem::operator==(const SfxPoolItem& _rItem) const
{
    if (!SfxPoolItem::operator==(_rItem))
        return false;
    const OStringListItem* pCompare = static_cast<const OStringListItem*>(&_rItem);
    if (pCompare->m_aList.getLength() != m_aList.getLength())
        return false;

    // compare all strings individually
    for (sal_Int32 i = 0; i < m_aList.getLength(); ++i)
        if (m_aList[i] != pCompare->m_aList[i])
            return false;

    return true;
}

OStringListItem* OStringListItem::Clone(SfxItemPool* /* _pPool */) const
{
    return new OStringListItem(*this);
}

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
