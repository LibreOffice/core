/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svl/stringpool.hxx"

namespace svl {

StringPool::StringPool() {}

rtl_uString* StringPool::intern( const OUString& rStr )
{
    StrHashType::iterator it = maStrPool.find(rStr);
    if (it == maStrPool.end())
    {
        // Not yet in the pool.
        std::pair<StrHashType::iterator, bool> r = maStrPool.insert(rStr.intern());
        if (!r.second)
            // Insertion failed.
            return NULL;

        it = r.first;
    }

    return it->pData;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
