/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sharedformulagroups.hxx"

namespace sc {

void SharedFormulaGroups::set( size_t nSharedId, ScTokenArray* pArray )
{
    maStore.insert(nSharedId, pArray);
}

const ScTokenArray* SharedFormulaGroups::get( size_t nSharedId ) const
{
    StoreType::const_iterator it = maStore.find(nSharedId);
    return it == maStore.end() ? nullptr : it->second;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
