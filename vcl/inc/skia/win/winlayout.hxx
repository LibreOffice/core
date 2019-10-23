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

struct SkiaGlobalWinGlyphCache : public GlobalWinGlyphCache
{
    virtual bool AllocateTexture(WinGlyphDrawElement& rElement, int nWidth, int nHeight) override;
    virtual void NotifyElementUsed(WinGlyphDrawElement& rElement) override;
    virtual void Prune() override;
    // The least recently used SkBitmap order, identified by SkBitmap::getPixels().
    std::vector<void*> mLRUOrder;
};

class SkiaWinGlyphCache : public WinGlyphCache
{
public:
    void RemoveTextures(const std::vector<void*>& pixels);

private:
    // This class just "adds" RemoveTexture() to the base class, it's never instantiatied.
    SkiaWinGlyphCache() = delete;
};

#endif // INCLUDED_VCL_INC_SKIA_WIN_WINLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
