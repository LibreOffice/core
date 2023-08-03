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

#include <functional>

#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <svx/svxdllapi.h>

#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/theme/ThemeColorType.hxx>

class SvxColorValueSet;

struct SVXCORE_DLLPUBLIC NamedColor
{
    Color m_aColor;
    OUString m_aName;
    sal_Int16 m_nThemeIndex = -1;
    sal_Int16 m_nLumMod = 10000;
    sal_Int16 m_nLumOff = 0;

    NamedColor() = default;

    NamedColor(Color const& rColor, OUString const& rName)
        : m_aColor(rColor)
        , m_aName(rName)
    {
    }

    model::ComplexColor getComplexColor()
    {
        model::ComplexColor aComplexColor;

        auto eThemeColorType = model::convertToThemeColorType(m_nThemeIndex);

        if (eThemeColorType != model::ThemeColorType::Unknown)
        {
            aComplexColor.setThemeColor(eThemeColorType);

            if (m_nLumMod != 10000)
                aComplexColor.addTransformation({ model::TransformationType::LumMod, m_nLumMod });

            if (m_nLumOff != 0)
                aComplexColor.addTransformation({ model::TransformationType::LumOff, m_nLumOff });

            aComplexColor.setFinalColor(m_aColor);
        }
        else
        {
            aComplexColor.setColor(m_aColor);
        }

        return aComplexColor;
    }
};

typedef std::function<void(const OUString&, const NamedColor&)> ColorSelectFunction;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
