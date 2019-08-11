/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_FONT_FEATUREPASER_HXX
#define INCLUDED_VCL_FONT_FEATUREPASER_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <unordered_map>

#include <vcl/font/Feature.hxx>

namespace vcl
{
namespace font
{
// These must not conflict with font name lists which use ; and ,
constexpr const char FeaturePrefix = ':';
constexpr const char FeatureSeparator = '&';

VCL_DLLPUBLIC OUString trimFontNameFeatures(OUString const& rFontName);

class VCL_DLLPUBLIC FeatureParser
{
private:
    OUString m_sLanguage;
    std::vector<FeatureSetting> m_aFeatures;

public:
    FeatureParser(OUString const& sFontName);

    OUString const& getLanguage() const { return m_sLanguage; }

    std::vector<FeatureSetting> const& getFeatures() const { return m_aFeatures; }

    std::unordered_map<uint32_t, uint32_t> getFeaturesMap() const;
};

} // end font namespace
} // end vcl namespace

#endif // INCLUDED_VCL_FONT_FEATUREPASER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
