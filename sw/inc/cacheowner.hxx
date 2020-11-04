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
#include <swdllapi.h>

namespace sw
{
    class SW_DLLPUBLIC CacheOwner
    {
        private:
            bool m_bInCache;
            bool m_bInSwFntCache;
        public:
            CacheOwner() : m_bInCache(false), m_bInSwFntCache(false) {};
            void SetInCache(bool bInCache)
                    { m_bInCache = bInCache; }
            void SetInSwFntCache(bool bInSwFntCache)
                    { m_bInSwFntCache = bInSwFntCache; }
            bool IsInCache() const
                    { return m_bInCache; }
            bool IsInSwFntCache() const
                    { return m_bInSwFntCache; }
            void CheckCaching(const sal_uInt16 nWhich);
            void ClearFromCaches();
    };
}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
