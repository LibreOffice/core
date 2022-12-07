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
#ifndef INCLUDED_SVX_PALETTE_HXX
#define INCLUDED_SVX_PALETTE_HXX

#include <sal/config.h>

#include <functional>

#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <svx/svxdllapi.h>

class SvxColorValueSet;

typedef std::pair<Color, OUString> NamedColor;

namespace svx
{
/// A color with an optional name and other theming-related properties.
struct SVXCORE_DLLPUBLIC NamedThemedColor
{
    Color m_aColor;
    OUString m_aName;
    sal_Int16 m_nThemeIndex = -1;
    sal_Int16 m_nLumMod = 10000;
    sal_Int16 m_nLumOff = 0;

    static NamedThemedColor FromNamedColor(const NamedColor& rNamedColor);

    NamedColor ToNamedColor() const;
};
}

typedef std::function<void(const OUString&, const svx::NamedThemedColor&)> ColorSelectFunction;

class Palette
{
protected:
    Palette(const Palette&) = default;
public:
    Palette() = default;
    virtual ~Palette();

    virtual const OUString&     GetName() = 0;
    virtual const OUString&     GetPath() = 0;
    virtual void                LoadColorSet(SvxColorValueSet& rColorSet) = 0;

    virtual bool                IsValid() = 0;

    virtual Palette*            Clone() const = 0;
};

#endif // INCLUDED_SVX_PALETTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
