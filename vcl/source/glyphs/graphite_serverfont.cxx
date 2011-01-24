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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Header files
//

// Platform
#include <sallayout.hxx>
// Module
#include "gcach_ftyp.hxx"
#include <graphite_features.hxx>
#include "graphite_textsrc.hxx"
#include <graphite_serverfont.hxx>

#ifndef WNT

//
// An implementation of the GraphiteLayout interface to enable Graphite enabled fonts to be used.
//

GraphiteServerFontLayout::GraphiteServerFontLayout(GraphiteFontAdaptor * pFont) throw()
  : ServerFontLayout(pFont->font()), mpFont(pFont),
    maImpl(*mpFont, mpFont->features(), pFont)
{
    // Nothing needed here
}

GraphiteServerFontLayout::~GraphiteServerFontLayout() throw()
{
    delete mpFont;
    mpFont = NULL;
}

const sal_Unicode* GraphiteServerFontLayout::getTextPtr() const
{
    return maImpl.textSrc()->getLayoutArgs().mpStr +
        maImpl.textSrc()->getLayoutArgs().mnMinCharPos;
}

sal_GlyphId GraphiteLayoutImpl::getKashidaGlyph(int & width)
{
    int nKashidaIndex = mpFont->font().GetGlyphIndex( 0x0640 );
    if( nKashidaIndex != 0 )
    {
        const GlyphMetric& rGM = mpFont->font().GetGlyphMetric( nKashidaIndex );
        width = rGM.GetCharWidth();
    }
    else
    {
        width = 0;
    }
    return nKashidaIndex;
}

#endif
