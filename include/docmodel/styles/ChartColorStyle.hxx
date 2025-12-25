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

#include <com/sun/star/uno/Reference.hxx>

#include <docmodel/color/ComplexColor.hxx>
#include <comphelper/string.hxx>
#include <rtl/ustring.hxx>
#include <memory>
#include <forward_list>

namespace com::sun::star::beans
{
class XPropertySet;
}

using namespace com::sun::star;

namespace model
{
enum class ColorStyleMethod
{
    CYCLE,
    WITHIN_LINEAR,
    ACROSS_LINEAR,
    WITHIN_LINEAR_REVERSED,
    ACROSS_LINEAR_REVERSED
};

struct DOCMODEL_DLLPUBLIC ColorStyleEntry
{
    std::vector<model::ComplexColor> maComplexColors;
    ColorStyleMethod meMethod;
    sal_Int32 mnId;

    ColorStyleEntry(std::vector<model::ComplexColor>& aCColors, ColorStyleMethod eMethod,
                    sal_Int32 nId)
        : maComplexColors(std::move(aCColors))
        , meMethod(eMethod)
        , mnId(nId)
    {
    }
};

struct DOCMODEL_DLLPUBLIC ColorStyleSet
{
    std::forward_list<ColorStyleEntry> maEntryList;

    void addEntry(std::vector<model::ComplexColor>& aComplexColors, ColorStyleMethod eMethod,
                  sal_Int32 nId);
};

} // namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
