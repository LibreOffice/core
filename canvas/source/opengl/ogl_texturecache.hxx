/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <unordered_map>

namespace com::sun::star::geometry { struct IntegerSize2D; }


namespace oglcanvas
{
    class TextureCache
    {
    public:
        TextureCache();
        ~TextureCache();

        /// clear whole cache, reset statistic counters
        void flush();

        /** prune old entries from cache

            Every time this method is called, all cache entries are set
            to "old". If subsequently not used by getTexture(),
            they'll be entitled for expunge on the next prune()
            call. Resets statistic counters.
         */
        void prune();

        /// Statistics
        size_t getCacheSize() const { return maCache.size(); };
        sal_uInt32 getCacheMissCount() const { return mnMissCount; }
        sal_uInt32 getCacheHitCount() const { return mnHitCount; }

        unsigned int getTexture( const css::geometry::IntegerSize2D& rPixelSize,
                                 const sal_Int8*                                  pPixel,
                                 sal_uInt32                                       nPixelCrc32) const;
    private:
        struct CacheEntry
        {
            CacheEntry() : nTexture(0), bOld(false) {}
            unsigned int nTexture;
            bool bOld;
        };
        typedef std::unordered_map<sal_uInt32,CacheEntry> TextureCacheMapT;
        mutable TextureCacheMapT maCache;
        mutable sal_uInt32       mnMissCount;
        mutable sal_uInt32       mnHitCount;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
