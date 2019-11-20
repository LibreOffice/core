/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <sharedformulagroups.hxx>
#include <tokenarray.hxx>

namespace sc {

void SharedFormulaGroups::set( size_t nSharedId, std::unique_ptr<ScTokenArray> pArray )
{
    m_Store.try_emplace(nSharedId, std::move(pArray), ScAddress(ScAddress::INITIALIZE_INVALID));
}

void SharedFormulaGroups::set( size_t nSharedId, std::unique_ptr<ScTokenArray> pArray, const ScAddress& rOrigin )
{
    m_Store.try_emplace(nSharedId, std::move(pArray), rOrigin);
}

const ScTokenArray* SharedFormulaGroups::get( size_t nSharedId ) const
{
    StoreType::const_iterator const it = m_Store.find(nSharedId);
    return it == m_Store.end() ? nullptr : it->second.getTokenArray();
}

const SharedFormulaGroupEntry* SharedFormulaGroups::getEntry( size_t nSharedId ) const
{
    StoreType::const_iterator const it = m_Store.find(nSharedId);
    return it == m_Store.end() ? nullptr : &(it->second);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
