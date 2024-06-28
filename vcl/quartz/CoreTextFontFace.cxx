/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>
#include <sal/log.hxx>

#ifdef MACOSX
#include <osx/saldata.hxx>
#include <osx/salinst.h>
#endif
#include <font/LogicalFontInstance.hxx>
#include <quartz/CoreTextFont.hxx>
#include <quartz/CoreTextFontFace.hxx>
#include <quartz/salgdi.h>
#include <quartz/utils.h>

CoreTextFontFace::CoreTextFontFace(const FontAttributes& rDFA, CTFontDescriptorRef xFontDescriptor)
    : vcl::font::PhysicalFontFace(rDFA)
    , mxFontDescriptor(xFontDescriptor)
{
    CFRetain(mxFontDescriptor);
}

CoreTextFontFace::~CoreTextFontFace() { CFRelease(mxFontDescriptor); }

sal_IntPtr CoreTextFontFace::GetFontId() const
{
    return reinterpret_cast<sal_IntPtr>(mxFontDescriptor);
}

const std::vector<hb_variation_t>& CoreTextFontFace::GetVariations(const LogicalFontInstance&) const
{
    if (!mxVariations)
    {
        mxVariations.emplace();
        CTFontRef pFont = CTFontCreateWithFontDescriptor(mxFontDescriptor, 0.0, nullptr);
        if (pFont)
        {
            CFArrayRef pAxes = CTFontCopyVariationAxes(pFont);
            if (pAxes)
            {
                CFDictionaryRef pVariations = CTFontCopyVariation(pFont);
                if (pVariations)
                {
                    CFIndex nAxes = CFArrayGetCount(pAxes);
                    for (CFIndex i = 0; i < nAxes; ++i)
                    {
                        auto pAxis = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(pAxes, i));
                        if (pAxis)
                        {
                            hb_tag_t nTag;
                            auto pTag = static_cast<CFNumberRef>(
                                CFDictionaryGetValue(pAxis, kCTFontVariationAxisIdentifierKey));
                            if (!pTag)
                                continue;
                            CFNumberGetValue(pTag, kCFNumberIntType, &nTag);

                            float fValue;
                            auto pValue
                                = static_cast<CFNumberRef>(CFDictionaryGetValue(pVariations, pTag));
                            if (!pValue)
                                continue;
                            CFNumberGetValue(pValue, kCFNumberFloatType, &fValue);

                            mxVariations->push_back({ nTag, fValue });
                        }
                    }
                    CFRelease(pVariations);
                }
                CFRelease(pAxes);
            }
            CFRelease(pFont);
        }
    }

    return *mxVariations;
}

rtl::Reference<LogicalFontInstance>
CoreTextFontFace::CreateFontInstance(const vcl::font::FontSelectPattern& rFSD) const
{
    return new CoreTextFont(*this, rFSD);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
