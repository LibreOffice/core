/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
