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
constexpr sal_uInt32 featureCode(const char sFeature[4])
{
    return static_cast<sal_uInt32>(sFeature[0]) << 24U | static_cast<sal_uInt32>(sFeature[1]) << 16U
           | static_cast<sal_uInt32>(sFeature[2]) << 8U | static_cast<sal_uInt32>(sFeature[3]);
}

VCL_DLLPUBLIC OUString featureCodeAsString(sal_uInt32 nFeature);

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
    sal_uInt32 m_nCode;
    OUString m_sDescription;
    const char* m_pDescriptionID;

public:
    FeatureParameter(sal_uInt32 nCode, OUString aDescription);
    FeatureParameter(sal_uInt32 nCode, const char* pDescriptionID);

    sal_uInt32 getCode() const;
    OUString getDescription() const;
};

class VCL_DLLPUBLIC FeatureDefinition
{
private:
    sal_uInt32 m_nCode;
    OUString m_sDescription;
    const char* m_pDescriptionID;
    OUString m_sNumericPart;
    FeatureParameterType m_eType;
    // the index of the parameter defines the enum value, string is the description
    std::vector<FeatureParameter> m_aEnumParameters;

public:
    FeatureDefinition();
    FeatureDefinition(sal_uInt32 nCode, OUString const& rDescription,
                      FeatureParameterType eType = FeatureParameterType::BOOL,
                      std::vector<FeatureParameter> const& rEnumParameters
                      = std::vector<FeatureParameter>{});
    FeatureDefinition(sal_uInt32 nCode, const char* pDescriptionID,
                      OUString const& rNumericPart = OUString());
    FeatureDefinition(sal_uInt32 nCode, const char* pDescriptionID,
                      std::vector<FeatureParameter> aEnumParameters);

    const std::vector<FeatureParameter>& getEnumParameters() const;
    OUString getDescription() const;
    sal_uInt32 getCode() const;
    FeatureParameterType getType() const;

    operator bool() const;
};

struct VCL_DLLPUBLIC FeatureID
{
    sal_uInt32 m_aFeatureCode;
    sal_uInt32 m_aScriptCode;
    sal_uInt32 m_aLanguageCode;
};

struct VCL_DLLPUBLIC Feature
{
    Feature();
    Feature(FeatureID const& rID, FeatureType eType);

    FeatureID m_aID;
    FeatureType const m_eType;
    FeatureDefinition m_aDefinition;
};

} // end font namespace

} // end vcl namespace

#endif // INCLUDED_VCL_FONT_FEATURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
