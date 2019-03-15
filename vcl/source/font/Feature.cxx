/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <utility>
#include <vcl/font/Feature.hxx>
#include <strings.hrc>
#include <svdata.hxx>

#include <hb.h>

namespace vcl
{
namespace font
{
OUString featureCodeAsString(uint32_t nFeature)
{
    std::vector<sal_Char> aString(5, 0);
    aString[0] = sal_Char(nFeature >> 24 & 0xff);
    aString[1] = sal_Char(nFeature >> 16 & 0xff);
    aString[2] = sal_Char(nFeature >> 8 & 0xff);
    aString[3] = sal_Char(nFeature >> 0 & 0xff);

    return OStringToOUString(aString.data(), RTL_TEXTENCODING_ASCII_US);
}

// Feature
Feature::Feature()
    : m_aID({ 0, 0, 0 })
    , m_eType(FeatureType::OpenType)
{
}

Feature::Feature(FeatureID const& rID, FeatureType eType)
    : m_aID(rID)
    , m_eType(eType)
{
}

// FeatureSetting
FeatureSetting::FeatureSetting(OString feature)
    : m_nTag(0)
    , m_nValue(0)
    , m_nStart(0)
    , m_nEnd(0)
{
    hb_feature_t aFeat;
    if (hb_feature_from_string(feature.getStr(), feature.getLength(), &aFeat))
    {
        m_nTag = aFeat.tag;
        m_nValue = aFeat.value;
        m_nStart = aFeat.start;
        m_nEnd = aFeat.end;
    }
}

// FeatureParameter

FeatureParameter::FeatureParameter(uint32_t nCode, OUString aDescription)
    : m_nCode(nCode)
    , m_sDescription(std::move(aDescription))
    , m_pDescriptionID(nullptr)
{
}

FeatureParameter::FeatureParameter(uint32_t nCode, const char* pDescriptionID)
    : m_nCode(nCode)
    , m_pDescriptionID(pDescriptionID)
{
}

OUString FeatureParameter::getDescription() const
{
    OUString aReturnString;

    if (m_pDescriptionID)
        aReturnString = VclResId(m_pDescriptionID);
    else if (!m_sDescription.isEmpty())
        aReturnString = m_sDescription;

    return aReturnString;
}

uint32_t FeatureParameter::getCode() const { return m_nCode; }

// FeatureDefinition

FeatureDefinition::FeatureDefinition()
    : m_pDescriptionID(nullptr)
    , m_nCode(0)
    , m_eType(FeatureParameterType::BOOL)
{
}

FeatureDefinition::FeatureDefinition(uint32_t nCode, OUString const& rDescription,
                                     FeatureParameterType eType,
                                     std::vector<FeatureParameter> const& rEnumParameters)
    : m_sDescription(rDescription)
    , m_pDescriptionID(nullptr)
    , m_nCode(nCode)
    , m_eType(eType)
    , m_aEnumParameters(rEnumParameters)
{
}

FeatureDefinition::FeatureDefinition(uint32_t nCode, const char* pDescriptionID,
                                     OUString const& rNumericPart)
    : m_pDescriptionID(pDescriptionID)
    , m_sNumericPart(rNumericPart)
    , m_nCode(nCode)
    , m_eType(FeatureParameterType::BOOL)
{
}

FeatureDefinition::FeatureDefinition(uint32_t nCode, const char* pDescriptionID,
                                     std::vector<FeatureParameter> aEnumParameters)
    : m_pDescriptionID(pDescriptionID)
    , m_nCode(nCode)
    , m_eType(FeatureParameterType::ENUM)
    , m_aEnumParameters(std::move(aEnumParameters))
{
}

const std::vector<FeatureParameter>& FeatureDefinition::getEnumParameters() const
{
    return m_aEnumParameters;
}

OUString FeatureDefinition::getDescription() const
{
    if (m_pDescriptionID)
    {
        OUString sTranslatedDescription = VclResId(m_pDescriptionID);
        if (!m_sNumericPart.isEmpty())
            return sTranslatedDescription.replaceFirst("%1", m_sNumericPart);
        return sTranslatedDescription;
    }
    else if (!m_sDescription.isEmpty())
    {
        return m_sDescription;
    }
    else
    {
        return vcl::font::featureCodeAsString(m_nCode);
    }
}

uint32_t FeatureDefinition::getCode() const { return m_nCode; }

FeatureParameterType FeatureDefinition::getType() const { return m_eType; }

FeatureDefinition::operator bool() const { return m_nCode != 0; }

} // end font namespace
} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
