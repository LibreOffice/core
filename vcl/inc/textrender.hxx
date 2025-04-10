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

#pragma once

#include <sal/config.h>

#include "salgdi.hxx"

class TextRenderImpl
{
public:
    // can't call ReleaseFonts here, as the destructor just calls this classes SetFont (pure virtual)!
    virtual ~TextRenderImpl() {}

    virtual void                    SetTextColor( Color nColor ) = 0;
    virtual void                    SetFont(LogicalFontInstance*, int nFallbackLevel) = 0;
    void ReleaseFonts() { SetFont(nullptr, 0); }
    virtual void                    GetFontMetric( FontMetricDataRef&, int nFallbackLevel ) = 0;
    virtual FontCharMapRef          GetFontCharMap() const = 0;
    virtual bool                    GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const = 0;
    virtual void                    GetDevFontList( vcl::font::PhysicalFontCollection* ) = 0;
    virtual void                    ClearDevFontCache() = 0;
    virtual bool                    AddTempDevFont( vcl::font::PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) = 0;

    virtual std::unique_ptr<GenericSalLayout>
                                    GetTextLayout(int nFallbackLevel) = 0;
    virtual void                    DrawTextLayout(const GenericSalLayout&, const SalGraphics&) = 0;
};

/* vim:set tabstop=4 shiftwidth=4 softtabstop=4 expandtab: */
