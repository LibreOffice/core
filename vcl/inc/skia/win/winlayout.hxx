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

#ifndef INCLUDED_VCL_INC_SKIA_WIN_WINLAYOUT_HXX
#define INCLUDED_VCL_INC_SKIA_WIN_WINLAYOUT_HXX

#include <win/winlayout.hxx>

#include <vector>

#include <skia/packedsurfaceatlas.hxx>

struct SkiaGlobalWinGlyphCache : public GlobalWinGlyphCache
{
    SkiaGlobalWinGlyphCache()
        : mPackedSurfaceAtlas(2048, 2048)
    {
    }
    SkiaPackedSurfaceAtlasManager mPackedSurfaceAtlas;
    virtual bool AllocateTexture(WinGlyphDrawElement& rElement, CompatibleDC* dc) override;
    virtual void Prune() override;
};

class SkiaWinGlyphCache : public WinGlyphCache
{
public:
    void RemoveSurfaces(const std::vector<sk_sp<SkSurface>>& surfaces);

private:
    // This class just "adds" RemoveSurfaces() to the base class, it's never instantiated.
    SkiaWinGlyphCache() = delete;
};

#endif // INCLUDED_VCL_INC_SKIA_WIN_WINLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
