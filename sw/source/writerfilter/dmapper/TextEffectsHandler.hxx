/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "LoggedResources.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>

#include <docmodel/color/ComplexColor.hxx>

#include "PropertyIds.hxx"

#include <oox/helper/grabbagstack.hxx>
#include <vcl/font/Feature.hxx>

#include <memory>
#include <optional>

namespace writerfilter::dmapper
{
/// Class to process all text effects like glow, textOutline, ...
class TextEffectsHandler : public LoggedProperties
{
protected:
    std::optional<PropertyIds> maPropertyId;
    OUString maElementName;
    std::unique_ptr<oox::GrabBagStack> mpGrabBagStack;
    std::vector<vcl::font::FeatureSetting> maFontFeatures;
    uint32_t mnLastStyleSetTag = 0;

    void convertElementIdToPropertyId(sal_Int32 aElementId);
    void addFontFeature(const char* pTag, uint32_t nValue);
    void addLigatureFeatures(std::u16string_view rValue);

public:
    explicit TextEffectsHandler(sal_uInt32 aElementId);
    virtual ~TextEffectsHandler() override;

    const std::optional<PropertyIds>& getGrabBagPropertyId() const { return maPropertyId; }

    css::beans::PropertyValue getInteropGrabBag();

    /// The OpenType features the w14 smart typography elements ask for.
    const std::vector<vcl::font::FeatureSetting>& getFontFeatures() const { return maFontFeatures; }
    void addContextualAlternates() { addFontFeature("calt", 1); }

    static OUString getSchemeColorValTypeString(sal_Int32 nType);
    static OUString getRectAlignmentString(sal_Int32 nType);
    static OUString getLineCapString(sal_Int32 nType);
    static OUString getCompoundLineString(sal_Int32 nType);
    static OUString getPenAlignmentString(sal_Int32 nType);
    static OUString getOnOffString(sal_Int32 nType);
    static OUString getPathShadeTypeString(sal_Int32 nType);
    static OUString getPresetLineDashValString(sal_Int32 nType);
    static OUString getPresetCameraTypeString(sal_Int32 nType);
    static OUString getLightRigTypeString(sal_Int32 nType);
    static OUString getLightRigDirectionString(sal_Int32 nType);
    static OUString getBevelPresetTypeString(sal_Int32 nType);
    static OUString getPresetMaterialTypeString(sal_Int32 nType);
    static OUString getLigaturesString(sal_Int32 nType);
    static OUString getNumFormString(sal_Int32 nType);
    static OUString getNumSpacingString(sal_Int32 nType);

    static sal_uInt8 GetTextFillSolidFillAlpha(const css::beans::PropertyValue& rValue);

    // LoggedProperties
    virtual void lcl_attribute(Id aName, const Value& aValue) override;
    virtual void lcl_sprm(Sprm& sprm) override;
};

class TextFillHandler : public TextEffectsHandler
{
    void updateComplexColor();
    model::ComplexColor& maComplexColor;

public:
    bool mbIsHandled;
    TextFillHandler(sal_uInt32 aElementId, model::ComplexColor& aComplexColor);
    // LoggedProperties
    virtual void lcl_sprm(Sprm& rSprm) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
