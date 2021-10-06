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

// nowadays these substitutions are needed for backward compatibility and tight platform integration:
// - substitutions from configuration entries (Tools->Options->FontReplacement and/or fontconfig)
// - device specific substitutions (e.g. for PS printer builtin fonts)
// - substitutions for missing fonts defined by configuration entries (generic and/or platform dependent fallbacks)
// - substitutions for missing fonts defined by multi-token fontnames (e.g. fontname="SpecialFont;FallbackA;FallbackB")
// - substitutions for incomplete fonts (implicit, generic, EUDC and/or platform dependent fallbacks)
// - substitutions for missing symbol fonts by translating code points into other symbol fonts

#pragma once

#include <sal/config.h>

#include <rtl/ustring.hxx>

#include <vcl/rendercontext/AddFontSubstituteFlags.hxx>

#include <font/FontSelectPattern.hxx>

namespace vcl::font
{
class FontSelectPattern;

class FontSubstitution
{
    // TODO: there is more commonality between the different substitutions
protected:
    virtual ~FontSubstitution() {}
};

/// Abstracts the concept of finding the best font to support an incomplete font
class GlyphFallbackFontSubstitution : public FontSubstitution
{
public:
    virtual bool FindFontSubstitute(vcl::font::FontSelectPattern&,
                                    LogicalFontInstance* pLogicalFont,
                                    OUString& rMissingCodes) const = 0;
};

/** Abstracts the concept of a configured font substitution before the
    availability of the originally selected font has been checked.
 */
class PreMatchFontSubstitution : public FontSubstitution
{
public:
    virtual bool FindFontSubstitute(vcl::font::FontSelectPattern&) const = 0;
};

void ImplFontSubstitute(OUString& rFontName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
