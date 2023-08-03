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

#include <array>
#include <docmodel/dllapi.h>
#include <rtl/ustring.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <tools/color.hxx>

typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace model
{
class DOCMODEL_DLLPUBLIC ColorSet
{
    OUString maName;
    std::array<Color, 12> maColors;

public:
    ColorSet(OUString const& rName);

    void setName(OUString const& rName) { maName = rName; }

    void add(model::ThemeColorType Type, Color aColorData);

    const OUString& getName() const { return maName; }

    Color resolveColor(model::ComplexColor const& rComplexColor) const;

    Color getColor(model::ThemeColorType eType) const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
