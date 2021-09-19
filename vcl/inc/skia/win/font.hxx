/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/config.h>

#include <win/winlayout.hxx>

#include <SkTypeface.h>

// This class only adds SkTypeface in order to allow its caching.
class SkiaWinFontInstance : public WinFontInstance
{
    friend rtl::Reference<LogicalFontInstance>
    WinFontFace::CreateFontInstance(const vcl::font::FontSelectPattern&) const;

public:
    sk_sp<SkTypeface> GetSkiaTypeface() const { return m_skiaTypeface; }
    bool GetSkiaDWrite() const { return m_skiaDWrite; }
    void SetSkiaTypeface(const sk_sp<SkTypeface>& typeface, bool dwrite)
    {
        m_skiaTypeface = typeface;
        m_skiaDWrite = dwrite;
    }

private:
    using WinFontInstance::WinFontInstance;
    sk_sp<SkTypeface> m_skiaTypeface;
    bool m_skiaDWrite;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
