/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "rtl/ustring.hxx"
#include "osl/module.h"
#include "osl/thread.h"

#include "unx/saldisp.hxx"
#include "unx/saldata.hxx"
#include "unx/salgdi.h"

#include "gcach_xpeer.hxx"
#include "xrender_peer.hxx"

// ===========================================================================

X11GlyphPeer::X11GlyphPeer()
{
}

// ---------------------------------------------------------------------------

X11GlyphPeer::~X11GlyphPeer()
{
    if( !ImplGetSVData() )
        return;

    //Why do this here, move into dtor/shutdown of display?
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    Display* const pX11Disp = pSalDisp->GetDisplay();
    int nMaxScreens = pSalDisp->GetXScreenCount();
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();

    for( int i = 0; i < nMaxScreens; i++ )
    {
        SalDisplay::RenderEntryMap& rMap = pSalDisp->GetRenderEntries( SalX11Screen (i) );
        for( SalDisplay::RenderEntryMap::iterator it = rMap.begin(); it != rMap.end(); ++it )
        {
            if( it->second.m_aPixmap )
                ::XFreePixmap( pX11Disp, it->second.m_aPixmap );
            if( it->second.m_aPicture )
                rRenderPeer.FreePicture( it->second.m_aPicture );
        }
        rMap.clear();
    }
}

// ===========================================================================

X11GlyphCache::X11GlyphCache( X11GlyphPeer& rPeer )
:   GlyphCache( rPeer )
{
}

// ---------------------------------------------------------------------------

namespace
{
    struct GlyphCacheHolder
    {
    private:
        X11GlyphPeer* m_pX11GlyphPeer;
        X11GlyphCache* m_pX11GlyphCache;
    public:
        GlyphCacheHolder()
        {
            m_pX11GlyphPeer = new X11GlyphPeer();
            m_pX11GlyphCache = new X11GlyphCache( *m_pX11GlyphPeer );
        }
        void release()
        {
            delete m_pX11GlyphCache;
            delete m_pX11GlyphPeer;
            m_pX11GlyphCache = NULL;
            m_pX11GlyphPeer = NULL;
        }
        X11GlyphCache& getGlyphCache()
        {
            return *m_pX11GlyphCache;
        }
        ~GlyphCacheHolder()
        {
            release();
        }
    };

    struct theGlyphCacheHolder :
        public rtl::Static<GlyphCacheHolder, theGlyphCacheHolder>
    {};
}

X11GlyphCache& X11GlyphCache::GetInstance()
{
    return theGlyphCacheHolder::get().getGlyphCache();
}

// ---------------------------------------------------------------------------

void X11GlyphCache::KillInstance()
{
    return theGlyphCacheHolder::get().release();
}

// ===========================================================================

void X11SalGraphics::releaseGlyphPeer()
{
    X11GlyphCache::KillInstance();
}

// ===========================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
