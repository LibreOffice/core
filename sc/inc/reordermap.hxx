/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "types.hxx"
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

    const_iterator end() const { return maData.end(); }

    template<class... Args>
    std::pair<iterator,bool> emplace(Args&&... args) { return maData.emplace(std::forward<Args>(args)...); }

    const_iterator find( DataType::key_type key ) const { return maData.find(key); }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
