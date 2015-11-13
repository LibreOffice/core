/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>

#include <cassert>

#include <rtl/instance.hxx>
#include <tools/debug.hxx>

#include "generic/geninst.h"
#include "generic/glyphcache.hxx"
#include "headless/svpgdi.hxx"

class SvpGlyphPeer : public GlyphCachePeer
{
public:
    SvpGlyphPeer() {}
};

namespace
{
    struct GlyphCacheHolder
    {
    private:
        SvpGlyphPeer* m_pSvpGlyphPeer;
        GlyphCache* m_pSvpGlyphCache;
    public:
        GlyphCacheHolder()
        {
            m_pSvpGlyphPeer = new SvpGlyphPeer();
            m_pSvpGlyphCache = new GlyphCache( *m_pSvpGlyphPeer );
        }
        GlyphCache& getGlyphCache()
        {
            return *m_pSvpGlyphCache;
        }
        ~GlyphCacheHolder()
        {
            delete m_pSvpGlyphCache;
            delete m_pSvpGlyphPeer;
        }
    };

    struct theGlyphCacheHolder :
        public rtl::Static<GlyphCacheHolder, theGlyphCacheHolder>
    {};
}

GlyphCache& SvpSalGraphics::getPlatformGlyphCache()
{
    return theGlyphCacheHolder::get().getGlyphCache();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
