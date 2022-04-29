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

#include "PropertyIds.hxx"

#include <oox/helper/grabbagstack.hxx>

#include <memory>
#include <optional>

namespace writerfilter::dmapper
{
/// Class to process all text effects like glow, textOutline, ...
class TextEffectsHandler : public LoggedProperties
{
private:
    std::optional<PropertyIds> maPropertyId;
    OUString maElementName;
    std::unique_ptr<oox::GrabBagStack> mpGrabBagStack;

    void convertElementIdToPropertyId(sal_Int32 aElementId);

public:
    explicit TextEffectsHandler(sal_uInt32 aElementId);
    virtual ~TextEffectsHandler() override;

    const std::optional<PropertyIds>& getGrabBagPropertyId() const { return maPropertyId; }

    css::beans::PropertyValue getInteropGrabBag();

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
    virtual void lcl_attribute(Id aName, Value& aValue) override;
    virtual void lcl_sprm(Sprm& sprm) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
