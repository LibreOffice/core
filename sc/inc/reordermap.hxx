/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_REORDERMAP_HXX
#define INCLUDED_SC_REORDERMAP_HXX

#include <types.hxx>
#include <unordered_map>

namespace sc {

class ColRowReorderMapType
{
    typedef std::unordered_map<SCCOLROW,SCCOLROW> DataType;
    DataType maData;
public:
    typedef DataType::value_type value_type;
    typedef DataType::const_iterator const_iterator;
    typedef DataType::iterator iterator;

    const_iterator end() const;

    std::pair<iterator, bool>
        insert( DataType::value_type const& val );

    const_iterator find( DataType::key_type const& key ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
