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

#include <font/PhysicalFontFace.hxx>
#ifdef IOS
#include <font/PhysicalFontCollection.hxx>
#endif

#include <unordered_map>

// TODO: move into cross-platform headers

class CoreTextFontFace;

class SystemFontList
{
public:
    SystemFontList();
    ~SystemFontList();

    bool Init();
    void AddFont(CoreTextFontFace*);

    void AnnounceFonts(vcl::font::PhysicalFontCollection&) const;
    CoreTextFontFace* GetFontDataFromId(sal_IntPtr nFontId) const;

    CTFontCollectionRef fontCollection() { return mpCTFontCollection; }

private:
    CTFontCollectionRef mpCTFontCollection;
    CFArrayRef mpCTFontArray;

    std::unordered_map<sal_IntPtr, rtl::Reference<CoreTextFontFace>> maFontContainer;
};

FontAttributes DevFontFromCTFontDescriptor(CTFontDescriptorRef, bool*);
// True for fonts that have an 'hvgl' but no 'glyf' table (the new PingFang
// fonts in macOS 15+), which we cannot render, see tdf#163000.
bool IsHvglOnlyFont(CTFontDescriptorRef);
// True for fonts that can be used by VCL, i.e. that are in our font list: not a
// private system font (family name starting with '.') and with a supported,
// renderable format.
bool IsFontUsableByVCL(CTFontDescriptorRef);
std::unique_ptr<SystemFontList> GetCoretextFontList();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
