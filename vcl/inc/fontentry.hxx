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

#ifndef INCLUDED_VCL_INC_FONTENTRY_HXX
#define INCLUDED_VCL_INC_FONTENTRY_HXX

#include <sal/types.h>
#include <vcl/dllapi.h>

#include "fontselect.hxx"
#include "fontattributes.hxx"
#include "outdev.h"

#include <unordered_map>

class ImplFontCache;
class ConvertChar;

// TODO: rename ImplFontEntry to LogicalFontInstance
// TODO: allow sharing of metrics for related fonts

class VCL_PLUGIN_PUBLIC ImplFontEntry
{
public:
    explicit        ImplFontEntry( const FontSelectPattern& );
    virtual         ~ImplFontEntry();

public: // TODO: make data members private
    ImplFontCache * m_pFontCache;
    FontSelectPattern  maFontSelData;          // FontSelectionData
    ImplFontAttributes  maFontAttributes;       // Font attributes
    const ConvertChar* mpConversion;           // used e.g. for StarBats->StarSymbol

    long            mnLineHeight;
    sal_uLong       mnRefCount;
    sal_uInt16      mnSetFontFlags;         // Flags returned by SalGraphics::SetFont()
    short           mnOwnOrientation;       // text angle if lower layers don't rotate text themselves
    short           mnOrientation;          // text angle in 3600 system
    bool            mbInit;                 // true if maFontAttributes member is valid

    void            AddFallbackForUnicode( sal_UCS4, FontWeight eWeight, const OUString& rFontName );
    bool            GetFallbackForUnicode( sal_UCS4, FontWeight eWeight, OUString* pFontName ) const;
    void            IgnoreFallbackForUnicode( sal_UCS4, FontWeight eWeight, const OUString& rFontName );

private:
    // cache of Unicode characters and replacement font names
    // TODO: a fallback map can be shared with many other ImplFontEntries
    // TODO: at least the ones which just differ in orientation, stretching or height
    typedef ::std::pair<sal_UCS4,FontWeight> GFBCacheKey;
    struct GFBCacheKey_Hash{ size_t operator()( const GFBCacheKey& ) const; };
    typedef ::std::unordered_map<GFBCacheKey,OUString,GFBCacheKey_Hash> UnicodeFallbackList;
    UnicodeFallbackList* mpUnicodeFallbackList;
};

#endif // INCLUDED_VCL_INC_FONTENTRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
