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

#include <premac.h>
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#include <osx/osxvcltypes.h>
#include <osx/salframe.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#endif
#include <postmac.h>

#ifdef IOS
// iOS defines a different Point class so include salgeom.hxx after postmac.h
// so that it will use the Point class in tools/gen.hxx
#include "salgeom.hxx"
#endif

#include <font/LogicalFontInstance.hxx>
#include <font/FontMetricData.hxx>
#include <salgdi.hxx>

#include <quartz/salgdicommon.hxx>

#include <quartz/CGHelpers.hxx>
#include <quartz/CoreTextFontFace.hxx>

class CoreTextFont final : public LogicalFontInstance
{
    friend rtl::Reference<LogicalFontInstance>
    CoreTextFontFace::CreateFontInstance(const vcl::font::FontSelectPattern&) const;

public:
    ~CoreTextFont() override;

    void GetFontMetric(FontMetricDataRef const&);
    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override;

    CTFontRef GetCTFont() const { return mpCTFont; }

    /// <1.0: font is squeezed, >1.0 font is stretched, else 1.0
    float mfFontStretch;
    /// text rotation in radian
    float mfFontRotation;

private:
    explicit CoreTextFont(const CoreTextFontFace&, const vcl::font::FontSelectPattern&);

    CTFontRef mpCTFont;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
