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

#ifndef INCLUDED_VCL_INC_IMPFONTCACHE_HXX
#define INCLUDED_VCL_INC_IMPFONTCACHE_HXX

#include <unordered_map>

#include "fontselect.hxx"

class Size;
namespace vcl { class Font; }
class PhysicalFontCollection;


// TODO: closely couple with PhysicalFontCollection

class ImplFontCache
{
    // For access to Acquire and Release
    friend class LogicalFontInstance;
private:
    LogicalFontInstance* mpFirstEntry;
    int                  mnRef0Count;    // number of unreferenced LogicalFontInstances

    // cache of recently used font instances
    struct IFSD_Equal { bool operator()( const FontSelectPattern&, const FontSelectPattern& ) const; };
    struct IFSD_Hash { size_t operator()( const FontSelectPattern& ) const; };
    typedef std::unordered_map<FontSelectPattern,LogicalFontInstance*,IFSD_Hash,IFSD_Equal > FontInstanceList;
    FontInstanceList    maFontInstanceList;

    int                 CountUnreferencedEntries() const;
    bool                IsFontInList(const LogicalFontInstance* pFont) const;

    /// Increase the refcount of the given LogicalFontInstance.
    void                Acquire(LogicalFontInstance*);
    /// Decrease the refcount and potentially cleanup the entries with zero refcount from the cache.
    void                Release(LogicalFontInstance*);

public:
                        ImplFontCache();
                        ~ImplFontCache();

    LogicalFontInstance* GetFontInstance( PhysicalFontCollection const *,
                             const vcl::Font&, const Size& rPixelSize, float fExactHeight);
    LogicalFontInstance* GetFontInstance( PhysicalFontCollection const *, FontSelectPattern& );
    LogicalFontInstance* GetGlyphFallbackFont( PhysicalFontCollection const *, FontSelectPattern&,
                            int nFallbackLevel, OUString& rMissingCodes );

    void                Invalidate();
};

#endif // INCLUDED_VCL_INC_IMPFONTCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
