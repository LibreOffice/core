/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <GL/glew.h>

#include <com/sun/star/geometry/IntegerSize2D.hpp>

#include "ogl_texturecache.hxx"


using namespace ::com::sun::star;

namespace oglcanvas
{
    TextureCache::TextureCache() :
        maCache(101),
        mnMissCount(0),
        mnHitCount(0)
    {}

    TextureCache::~TextureCache()
    {
        flush();
    }

    void TextureCache::flush()
    {
        // un-bind any texture
        glBindTexture(GL_TEXTURE_2D, 0);

        // delete all cached textures
        for( const auto& rCache : maCache )
        {
            glDeleteTextures( 1, &rCache.second.nTexture );
        }

        maCache.clear();
        mnMissCount = 0;
        mnHitCount  = 0;
    }

    void TextureCache::prune()
    {
        // un-bind any texture
        glBindTexture(GL_TEXTURE_2D, 0);

        // delete already "old" textures, mark "new" entries "old"
        const TextureCacheMapT::const_iterator aEnd = maCache.end();
        for( auto aCurr = maCache.begin(); aCurr != aEnd; /* increment managed in loop */)
        {
            if( aCurr->second.bOld )
            {
                glDeleteTextures( 1, &aCurr->second.nTexture );
                aCurr = maCache.erase( aCurr );
            }
            else
            {
                aCurr->second.bOld = true;
                ++aCurr;
            }
        }

        mnMissCount = 0;
        mnHitCount  = 0;
    }

    unsigned int TextureCache::getTexture( const geometry::IntegerSize2D& rPixelSize,
                                           const sal_Int8*                pPixel,
                                           sal_uInt32                     nPixelCrc32) const
    {
        unsigned int nTexture(0);

        // texture already cached?
        TextureCacheMapT::iterator aCacheEntry;
        if( (aCacheEntry=maCache.find(nPixelCrc32)) == maCache.end() )
        {
            // nope, insert new entry
            glGenTextures(1, &nTexture);
            glBindTexture(GL_TEXTURE_2D, nTexture);

            // TODO(E3): handle limited texture sizes -
            // glGetIntegerv(GL_MAX_TEXTURE_SIZE)
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         4,
                         rPixelSize.Width,
                         rPixelSize.Height,
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_INT_8_8_8_8_REV,
                         pPixel);

            maCache[nPixelCrc32].nTexture = nTexture;
            ++mnMissCount;

            return nTexture;
        }
        else
        {
            nTexture = aCacheEntry->second.nTexture;
            aCacheEntry->second.bOld = false;
            ++mnHitCount;
        }

        return nTexture;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
