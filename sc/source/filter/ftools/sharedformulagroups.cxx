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

SharedFormulaGroups::Key::Key(size_t nId, SCCOL nCol) : mnId(nId), mnCol(nCol) {}

bool SharedFormulaGroups::Key::operator== ( const Key& rOther ) const
{
    return mnId == rOther.mnId && mnCol == rOther.mnCol;
}

bool SharedFormulaGroups::Key::operator!= ( const Key& rOther ) const
{
    return !operator==(rOther);
}

size_t SharedFormulaGroups::KeyHash::operator ()( const Key& rKey ) const
{
    double nVal = rKey.mnId;
    nVal *= 256.0;
    nVal += rKey.mnCol;
    return static_cast<size_t>(nVal);
}

void SharedFormulaGroups::set( size_t nSharedId, SCCOL nCol, const ScFormulaCellGroupRef& xGroup )
{
    Key aKey(nSharedId, nCol);
    maStore.insert(StoreType::value_type(aKey, xGroup));
}

ScFormulaCellGroupRef SharedFormulaGroups::get( size_t nSharedId, SCCOL nCol ) const
{
    Key aKey(nSharedId, nCol);
    StoreType::const_iterator it = maStore.find(aKey);
    return it == maStore.end() ? ScFormulaCellGroupRef() : it->second;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
