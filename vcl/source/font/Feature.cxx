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
#include <svdata.hxx>

#include <hb.h>

namespace vcl::font
{
OUString featureCodeAsString(uint32_t nFeature)
{
    std::vector<char> aString(5, 0);
    aString[0] = char(nFeature >> 24 & 0xff);
    aString[1] = char(nFeature >> 16 & 0xff);
    aString[2] = char(nFeature >> 8 & 0xff);
    aString[3] = char(nFeature >> 0 & 0xff);

    return OStringToOUString(aString.data(), RTL_TEXTENCODING_ASCII_US);
}

// Feature
Feature::Feature()
    : m_nCode(0)
    , m_eType(FeatureType::OpenType)
{
}

Feature::Feature(uint32_t nCode, FeatureType eType)
    : m_nCode(nCode)
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
{
}

FeatureParameter::FeatureParameter(uint32_t nCode, TranslateId pDescriptionID)
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
    : m_nCode(0)
    , m_nDefault(-1)
    , m_eType(FeatureParameterType::BOOL)
{
}

FeatureDefinition::FeatureDefinition(uint32_t nCode, OUString aDescription,
                                     FeatureParameterType eType,
                                     std::vector<FeatureParameter>&& rEnumParameters,
                                     int32_t nDefault)
    : m_sDescription(std::move(aDescription))
    , m_nCode(nCode)
    , m_nDefault(nDefault)
    , m_eType(eType)
    , m_aEnumParameters(std::move(rEnumParameters))
{
}

FeatureDefinition::FeatureDefinition(uint32_t nCode, TranslateId pDescriptionID,
                                     OUString aNumericPart)
    : m_pDescriptionID(pDescriptionID)
    , m_sNumericPart(std::move(aNumericPart))
    , m_nCode(nCode)
    , m_nDefault(-1)
    , m_eType(FeatureParameterType::BOOL)
{
}

FeatureDefinition::FeatureDefinition(uint32_t nCode, TranslateId pDescriptionID,
                                     std::vector<FeatureParameter> aEnumParameters)
    : m_pDescriptionID(pDescriptionID)
    , m_nCode(nCode)
    , m_nDefault(-1)
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

int32_t FeatureDefinition::getDefault() const { return m_nDefault; }
} // end vcl::font namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
