/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_FONT_FEATURE_HXX
#define INCLUDED_VCL_FONT_FEATURE_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <vector>

namespace vcl
{
namespace font
{
constexpr uint32_t featureCode(const char sFeature[4])
{
    return static_cast<uint32_t>(sFeature[0]) << 24U | static_cast<uint32_t>(sFeature[1]) << 16U
           | static_cast<uint32_t>(sFeature[2]) << 8U | static_cast<uint32_t>(sFeature[3]);
}

VCL_DLLPUBLIC OUString featureCodeAsString(uint32_t nFeature);

enum class FeatureParameterType
{
    BOOL,
    ENUM
};

enum class FeatureType
{
    OpenType,
    Graphite
};

struct VCL_DLLPUBLIC FeatureParameter
{
private:
    uint32_t m_nCode;
    OUString m_sDescription;
    const char* m_pDescriptionID;

public:
    FeatureParameter(uint32_t nCode, OUString aDescription);
    FeatureParameter(uint32_t nCode, const char* pDescriptionID);

    uint32_t getCode() const;
    OUString getDescription() const;
};

class VCL_DLLPUBLIC FeatureDefinition
{
private:
    OUString m_sDescription;
    const char* m_pDescriptionID;
    OUString m_sNumericPart;
    uint32_t m_nCode;
    FeatureParameterType m_eType;
    // the index of the parameter defines the enum value, string is the description
    std::vector<FeatureParameter> m_aEnumParameters;

public:
    FeatureDefinition();
    FeatureDefinition(uint32_t nCode, OUString const& rDescription,
                      FeatureParameterType eType = FeatureParameterType::BOOL,
                      std::vector<FeatureParameter> const& rEnumParameters
                      = std::vector<FeatureParameter>{});
    FeatureDefinition(uint32_t nCode, const char* pDescriptionID,
                      OUString const& rNumericPart = OUString());
    FeatureDefinition(uint32_t nCode, const char* pDescriptionID,
                      std::vector<FeatureParameter> aEnumParameters);

    const std::vector<FeatureParameter>& getEnumParameters() const;
    OUString getDescription() const;
    uint32_t getCode() const;
    FeatureParameterType getType() const;

    operator bool() const;
};

struct VCL_DLLPUBLIC FeatureID
{
    uint32_t m_aFeatureCode;
    uint32_t m_aScriptCode;
    uint32_t m_aLanguageCode;
};

struct VCL_DLLPUBLIC Feature
{
    Feature();
    Feature(FeatureID const& rID, FeatureType eType);

    FeatureID m_aID;
    FeatureType const m_eType;
    FeatureDefinition m_aDefinition;
};

// This is basically duplicates hb_feature_t to avoid including HarfBuzz
// headers here, so the member types should remain compatible.
struct VCL_DLLPUBLIC FeatureSetting
{
    FeatureSetting(OString feature);

    uint32_t m_nTag;
    uint32_t m_nValue;
    unsigned int m_nStart;
    unsigned int m_nEnd;
};

} // end font namespace

} // end vcl namespace

#endif // INCLUDED_VCL_FONT_FEATURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
