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

#ifndef INCLUDED_VCL_INC_IMPFONTCHARMAP_HXX
#define INCLUDED_VCL_INC_IMPFONTCHARMAP_HXX

#include <boost/intrusive_ptr.hpp>

class ImplFontCharMap;
typedef boost::intrusive_ptr< ImplFontCharMap > ImplFontCharMapPtr;

class CmapResult;

class VCL_PLUGIN_PUBLIC ImplFontCharMap
{
public:
    explicit            ImplFontCharMap( const CmapResult& );
    virtual             ~ImplFontCharMap();

private:
    friend class FontCharMap;
    friend void intrusive_ptr_add_ref(ImplFontCharMap* pImplFontCharMap);
    friend void intrusive_ptr_release(ImplFontCharMap* pImplFontCharMap);

                        ImplFontCharMap( const ImplFontCharMap& ) = delete;
    void                operator=( const ImplFontCharMap& ) = delete;

    static ImplFontCharMapPtr const & getDefaultMap( bool bSymbols=false);
    bool                isDefaultMap() const;

private:
    const sal_uInt32*   mpRangeCodes;     // pairs of StartCode/(EndCode+1)
    const int*          mpStartGlyphs;    // range-specific mapper to glyphs
    const sal_uInt16*   mpGlyphIds;       // individual glyphid mappings
    int                 mnRangeCount;
    int                 mnCharCount;      // covered codepoints
    mutable sal_uInt32  mnRefCount;
};

inline void intrusive_ptr_add_ref(ImplFontCharMap* pImplFontCharMap)
{
    ++pImplFontCharMap->mnRefCount;
}

inline void intrusive_ptr_release(ImplFontCharMap* pImplFontCharMap)
{
    if (--pImplFontCharMap->mnRefCount == 0)
        delete pImplFontCharMap;
}

bool ParseCMAP( const unsigned char* pRawData, int nRawLength, CmapResult& );

#endif // INCLUDED_VCL_INC_IMPFONTCHARMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
