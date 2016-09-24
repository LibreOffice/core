/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <reordermap.hxx>

namespace sc {

ColRowReorderMapType::const_iterator ColRowReorderMapType::end() const
{
    return maData.end();
}

std::pair<ColRowReorderMapType::iterator, bool>
ColRowReorderMapType::insert( ColRowReorderMapType::value_type const& val )
{
    return maData.insert(val);
}

ColRowReorderMapType::const_iterator
ColRowReorderMapType::find( DataType::key_type key ) const
{
    return maData.find(key);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
