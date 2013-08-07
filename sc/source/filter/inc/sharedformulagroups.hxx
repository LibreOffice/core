/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_FILTER_SHAREDFORMULAGROUPS_HXX
#define SC_FILTER_SHAREDFORMULAGROUPS_HXX

#include "address.hxx"
#include "formulacell.hxx"

#include <boost/unordered_map.hpp>

namespace sc {

class SharedFormulaGroups
{
    struct Key
    {
        size_t mnId;
        SCCOL mnCol;

        Key(size_t nId, SCCOL nCol);

        bool operator== ( const Key& rOther ) const;
        bool operator!= ( const Key& rOther ) const;
    };

    struct KeyHash
    {
        size_t operator() ( const Key& rKey ) const;
    };

    typedef boost::unordered_map<Key, ScFormulaCellGroupRef, KeyHash> StoreType;
    StoreType maStore;
public:

    void set( size_t nSharedId, SCCOL nCol, const ScFormulaCellGroupRef& xGroup );
    ScFormulaCellGroupRef get( size_t nSharedId, SCCOL nCol ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
