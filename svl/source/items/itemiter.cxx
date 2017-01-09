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


#include <svl/itemiter.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>

SfxItemIter::SfxItemIter( const SfxItemSet& rItemSet )
    : m_rSet( rItemSet )
{
    // store the set of keys because client code likes modifying the map
    // while iterating over it
    m_keys.resize(rItemSet.m_aItems.size());
    size_t idx = 0;
    for (auto const & rPair : rItemSet.m_aItems) {
        m_keys[idx++] = rPair.first;
    }
    m_iter = m_keys.begin();
}

SfxItemIter::~SfxItemIter()
{
}

SfxPoolItem const * SfxItemIter::FirstItem()
{
    m_iter = m_keys.begin();
    return GetCurItem();
}

SfxPoolItem const * SfxItemIter::GetCurItem()
{
    if (m_keys.empty())
        return nullptr;
    auto it = m_rSet.m_aItems.find(*m_iter);
    if (it == m_rSet.m_aItems.end())
        return nullptr;
    return it->second;
}

SfxPoolItem const * SfxItemIter::NextItem()
{
    if (m_iter == m_keys.end())
        return nullptr;
    ++m_iter;
    if (m_iter == m_keys.end())
        return nullptr;
    return GetCurItem();
}

bool SfxItemIter::IsAtEnd() const
{
    return m_iter == m_keys.end() || std::next(m_iter) == m_keys.end();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
