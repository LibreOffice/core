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

public:
    OpenTypeFeatureDefinitionListPrivate();
    FeatureDefinition getDefinition(const vcl::font::Feature& rFeature);
    bool isRequired(sal_uInt32 nFeatureCode);
};

VCL_DLLPUBLIC OpenTypeFeatureDefinitionListPrivate& OpenTypeFeatureDefinitionList();

} // namespace vcl::font

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
