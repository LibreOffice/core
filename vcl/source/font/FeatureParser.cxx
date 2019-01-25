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

    OUString sName = rFontName.getToken(0, vcl::font::FeaturePrefix, ++nPrefixIdx);
    sal_Int32 nIndex = 0;
    do
    {
        OUString sToken = sName.getToken(0, vcl::font::FeatureSeparator, nIndex);

        sal_Int32 nInnerIdx{ 0 };
        OUString sID = sToken.getToken(0, '=', nInnerIdx);
        OUString sValue = sToken.getToken(0, '=', nInnerIdx);

        if (sID.getLength() == 4 && sValue != "0")
        {
            if (sID == "lang")
            {
                m_sLanguage = sValue;
            }
            else
            {
                OString sFeatureCodeAscii = OUStringToOString(sID, RTL_TEXTENCODING_ASCII_US);
                sal_uInt32 nCode = vcl::font::featureCode(sFeatureCodeAscii.getStr());
                sal_uInt32 nValue = sValue.isEmpty() ? 1 : sValue.toUInt32();

                if (nValue != 0)
                    m_aFeatures.emplace_back(nCode, nValue);
            }
        }
    } while (nIndex >= 0);
}

std::unordered_map<sal_uInt32, sal_uInt32> FeatureParser::getFeaturesMap() const
{
    std::unordered_map<sal_uInt32, sal_uInt32> aResultMap;
    for (auto const& rPair : m_aFeatures)
    {
        aResultMap.emplace(rPair);
    }
    return aResultMap;
}

} // end font namespace

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
