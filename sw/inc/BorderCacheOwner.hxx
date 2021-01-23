/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_CACHEOWNER_HXX
#define INCLUDED_SW_INC_CACHEOWNER_HXX

#include <sal/types.h>
#include "swdllapi.h"

class SwBorderAttrs;
class SwBorderAttrAccess;

namespace sw
{
/// Bookkeeping helper for SwCache caching writer borders.
class SW_DLLPUBLIC BorderCacheOwner
{
private:
    friend SwBorderAttrs;
    friend SwBorderAttrAccess;
    bool m_bInCache;

public:
    BorderCacheOwner()
        : m_bInCache(false)
    {
    }
    BorderCacheOwner(BorderCacheOwner&)
        : m_bInCache(false)
    {
    }
    BorderCacheOwner& operator=(const BorderCacheOwner&)
    {
        m_bInCache = false;
        return *this;
    }
    ~BorderCacheOwner();
    bool IsInCache() const { return m_bInCache; }
    void InvalidateInSwCache(const sal_uInt16);
};
}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
