/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/font/Feature.hxx>
#include <rtl/instance.hxx>
#include <vector>
#include <unordered_map>

namespace vcl::font
{
class OpenTypeFeatureDefinitionListPrivate
{
private:
    std::vector<FeatureDefinition> m_aFeatureDefinition;
    std::unordered_map<sal_uInt32, size_t> m_aCodeToIndex;
    std::vector<sal_uInt32> m_aRequiredFeatures;

    void init();

    static bool isSpecialFeatureCode(sal_uInt32 nFeatureCode);
    static FeatureDefinition handleSpecialFeatureCode(sal_uInt32 nFeatureCode);

public:
    OpenTypeFeatureDefinitionListPrivate();
    FeatureDefinition getDefinition(sal_uInt32 nFeatureCode);
    bool isRequired(sal_uInt32 nFeatureCode);
};

class VCL_DLLPUBLIC OpenTypeFeatureDefinitionList
    : public rtl::Static<OpenTypeFeatureDefinitionListPrivate, OpenTypeFeatureDefinitionList>
{
};

} // namespace vcl::font

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
