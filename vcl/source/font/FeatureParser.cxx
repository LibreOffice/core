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
#include <o3tl/string_view.hxx>

namespace vcl::font
{
OUString trimFontNameFeatures(OUString const& rFontName)
{
    const sal_Int32 nPrefixIdx{ rFontName.indexOf(vcl::font::FeaturePrefix) };

    if (nPrefixIdx < 0)
        return rFontName;

    return rFontName.copy(0, nPrefixIdx);
}

FeatureParser::FeatureParser(std::u16string_view rFontName)
{
    size_t nPrefixIdx{ rFontName.find(vcl::font::FeaturePrefix) };

    if (nPrefixIdx == std::u16string_view::npos)
        return;

    std::u16string_view sName(rFontName.substr(++nPrefixIdx));
    sal_Int32 nIndex = 0;
    do
    {
        std::u16string_view sToken = o3tl::getToken(sName, 0, vcl::font::FeatureSeparator, nIndex);

        sal_Int32 nInnerIdx{ 0 };
        std::u16string_view sID = o3tl::getToken(sToken, 0, '=', nInnerIdx);

        if (sID == u"lang")
        {
            m_sLanguage = o3tl::getToken(sToken, 0, '=', nInnerIdx);
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

std::unordered_map<uint32_t, int32_t> FeatureParser::getFeaturesMap() const
{
    std::unordered_map<uint32_t, int32_t> aResultMap;
    for (auto const& rFeat : m_aFeatures)
        aResultMap.emplace(rFeat.m_nTag, rFeat.m_nValue);
    return aResultMap;
}

} // end vcl::font namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
