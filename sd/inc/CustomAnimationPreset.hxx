/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/config.h>

#include <vector>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "CustomAnimationEffect.hxx"

#include <unordered_map>

namespace sd {

typedef std::unordered_map< OUString, CustomAnimationEffectPtr > EffectsSubTypeMap;
typedef std::unordered_map< OUString, OUString > UStringMap;

class CustomAnimationPreset
{
    friend class CustomAnimationPresets;

public:
    CustomAnimationPreset( const CustomAnimationEffectPtr& pEffect );

    void add( const CustomAnimationEffectPtr& pEffect );

    SD_DLLPUBLIC css::uno::Reference< css::animations::XAnimationNode > create( const OUString& rstrSubType );

    const OUString& getPresetId() const { return maPresetId; }
    const OUString& getLabel() const { return maLabel; }
    double getDuration() const { return mfDuration; }

    std::vector<OUString> getSubTypes();
    std::vector<OUString> getProperties() const;

    bool hasProperty( std::u16string_view rProperty ) const;
    bool isTextOnly() const { return mbIsTextOnly; }

private:
    OUString maPresetId;
    OUString maProperty;
    OUString maLabel;
    OUString maDefaultSubTyp;
    double mfDuration;
    bool mbIsTextOnly;

    EffectsSubTypeMap maSubTypes;
};

typedef std::shared_ptr< CustomAnimationPreset > CustomAnimationPresetPtr;
typedef std::unordered_map<OUString, CustomAnimationPresetPtr> EffectDescriptorMap;
typedef std::vector< CustomAnimationPresetPtr > EffectDescriptorList;

struct PresetCategory
{
    OUString maLabel;
    EffectDescriptorList maEffects;

    PresetCategory( const OUString& rLabel, const EffectDescriptorList& rEffects )
        : maLabel( rLabel ), maEffects( rEffects ) {}
};
typedef std::shared_ptr< PresetCategory > PresetCategoryPtr;
typedef std::vector< PresetCategoryPtr > PresetCategoryList;

class SD_DLLPUBLIC CustomAnimationPresets final
{
public:
    SAL_DLLPRIVATE CustomAnimationPresets();
    SAL_DLLPRIVATE ~CustomAnimationPresets();

    /** This method gets presets instance, which is localized
     * for the current user's locale.
    */
    static const CustomAnimationPresets& getCustomAnimationPresets();

    SAL_DLLPRIVATE css::uno::Reference< css::animations::XAnimationNode > getRandomPreset( sal_Int16 nPresetClass ) const;

    CustomAnimationPresetPtr getEffectDescriptor( const OUString& rPresetId ) const;

    SAL_DLLPRIVATE const OUString& getUINameForPresetId( const OUString& rPresetId ) const;
    SAL_DLLPRIVATE const OUString& getUINameForProperty( const OUString& rProperty ) const;

    SAL_DLLPRIVATE const PresetCategoryList& getEntrancePresets() const { return maEntrancePresets; }
    SAL_DLLPRIVATE const PresetCategoryList& getEmphasisPresets() const { return maEmphasisPresets; }
    SAL_DLLPRIVATE const PresetCategoryList& getExitPresets() const { return maExitPresets; }
    SAL_DLLPRIVATE const PresetCategoryList& getMotionPathsPresets() const { return maMotionPathsPresets; }
    SAL_DLLPRIVATE const PresetCategoryList& getMiscPresets() const { return maMiscPresets; }

    SAL_DLLPRIVATE void changePresetSubType( const CustomAnimationEffectPtr& pEffect, const OUString& rPresetSubType ) const;

private:
    SAL_DLLPRIVATE void importEffects();
    SAL_DLLPRIVATE void importResources();

    SAL_DLLPRIVATE void importPresets( const css::uno::Reference< css::lang::XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, PresetCategoryList& rPresetMap  );

    SAL_DLLPRIVATE static const OUString& translateName( const OUString& rId, const UStringMap& rNameMap );

private:
    css::uno::Reference< css::animations::XAnimationNode > mxRootNode;
    EffectDescriptorMap maEffectDescriptorMap;
    UStringMap maEffectNameMap;
    UStringMap maPropertyNameMap;

    PresetCategoryList maEntrancePresets;
    PresetCategoryList maEmphasisPresets;
    PresetCategoryList maExitPresets;
    PresetCategoryList maMotionPathsPresets;
    PresetCategoryList maMiscPresets;

    //! Maps per-language the animation presets.
    SAL_DLLPRIVATE static std::map<OUString, CustomAnimationPresets>  mPresetsMap;
};


css::uno::Reference< css::animations::XAnimationNode > implImportEffects( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceFactory, const OUString& rPath );

void implImportLabels( const css::uno::Reference< css::lang::XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, UStringMap& rStringMap );

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
