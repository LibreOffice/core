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

#include <vector>

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

#include <font/LogicalFontInstance.hxx>
#include <font/PhysicalFontFace.hxx>
#include <salgdi.hxx>

#include <quartz/salgdicommon.hxx>
#include <hb.h>

class AquaSalFrame;
class FontAttributes;

// CoreText-specific physically available font face
class CoreTextFontFace final : public vcl::font::PhysicalFontFace
{
public:
    CoreTextFontFace(const FontAttributes&, CTFontDescriptorRef xRef);
    ~CoreTextFontFace() override;

    sal_IntPtr GetFontId() const override;

    CTFontDescriptorRef GetFontDescriptorRef() const { return mxFontDescriptor; }

    rtl::Reference<LogicalFontInstance>
    CreateFontInstance(const vcl::font::FontSelectPattern&) const override;

    hb_blob_t* GetHbTable(hb_tag_t nTag) const override;

    const std::vector<hb_variation_t>& GetVariations(const LogicalFontInstance&) const override;

private:
    CTFontDescriptorRef mxFontDescriptor;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
