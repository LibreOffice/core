/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/font/FeatureParser.hxx>
#include <vcl/font/Feature.hxx>

namespace vcl
{
namespace font
{
OUString trimFontNameFeatures(OUString const& rFontName)
{
    const sal_Int32 nPrefixIdx{ rFontName.indexOf(vcl::font::FeaturePrefix) };

    if (nPrefixIdx < 0)
        return rFontName;

    return rFontName.copy(0, nPrefixIdx);
}

FeatureParser::FeatureParser(OUString const& rFontName)
{
    sal_Int32 nPrefixIdx{ rFontName.indexOf(vcl::font::FeaturePrefix) };

    if (nPrefixIdx < 0)
        return;

    OUString sName = rFontName.copy(++nPrefixIdx);
    sal_Int32 nIndex = 0;
    do
    {
        OUString sToken = sName.getToken(0, vcl::font::FeatureSeparator, nIndex);

        sal_Int32 nInnerIdx{ 0 };
        OUString sID = sToken.getToken(0, '=', nInnerIdx);

        if (sID == "lang")
        {
            m_sLanguage = sToken.getToken(0, '=', nInnerIdx);
        }
        else
        {
            OString sFeature = OUStringToOString(sToken, RTL_TEXTENCODING_ASCII_US);
            FeatureSetting aFeature(sFeature);
            if (aFeature.m_nTag != 0)
                m_aFeatures.push_back(aFeature);
        }
    } while (nIndex >= 0);
}

std::unordered_map<uint32_t, uint32_t> FeatureParser::getFeaturesMap() const
{
    std::unordered_map<uint32_t, uint32_t> aResultMap;
    for (auto const& rFeat : m_aFeatures)
    {
        if (rFeat.m_nValue != 0)
            aResultMap.emplace(rFeat.m_nTag, rFeat.m_nValue);
    }
    return aResultMap;
}

} // end font namespace

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
