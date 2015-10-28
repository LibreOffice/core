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

#ifndef INCLUDED_SD_INC_CUSTOMANIMATIONPRESET_HXX
#define INCLUDED_SD_INC_CUSTOMANIMATIONPRESET_HXX

#include <sal/config.h>

#include <memory>
#include <vector>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>

#include <CustomAnimationEffect.hxx>

#include <unordered_map>

namespace sd {

typedef std::unordered_map< OUString, CustomAnimationEffectPtr, OUStringHash > EffectsSubTypeMap;
typedef std::unordered_map< OUString, OUString, OUStringHash > UStringMap;
typedef std::vector< OUString > UStringList;

class CustomAnimationPreset
{
    friend class CustomAnimationPresets;

public:
    CustomAnimationPreset( CustomAnimationEffectPtr pEffect );

    void add( CustomAnimationEffectPtr pEffect );

    SD_DLLPUBLIC css::uno::Reference< css::animations::XAnimationNode > create( const OUString& rstrSubType );

    const OUString& getPresetId() const { return maPresetId; }
    const OUString& getLabel() const { return maLabel; }
    double getDuration() const { return mfDuration; }

    UStringList getSubTypes();
    UStringList getProperties() const;

    bool hasProperty( const OUString& rProperty ) const;
    bool isTextOnly() const { return mbIsTextOnly; }

private:
    OUString maPresetId;
    OUString maProperty;
    sal_Int16 mnPresetClass;
    OUString maLabel;
    OUString maDefaultSubTyp;
    double mfDuration;
    bool mbIsTextOnly;

    EffectsSubTypeMap maSubTypes;
};

typedef std::shared_ptr< CustomAnimationPreset > CustomAnimationPresetPtr;
typedef std::unordered_map<OUString, CustomAnimationPresetPtr, OUStringHash> EffectDescriptorMap;
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

class SD_DLLPUBLIC CustomAnimationPresets
{
public:
    SAL_DLLPRIVATE CustomAnimationPresets();
    SAL_DLLPRIVATE virtual ~CustomAnimationPresets();

    SAL_DLLPRIVATE void init();

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

    SAL_DLLPRIVATE void changePresetSubType( CustomAnimationEffectPtr pEffect, const OUString& rPresetSubType ) const;

private:
    SAL_DLLPRIVATE void importEffects();
    SAL_DLLPRIVATE void importResources();

    SAL_DLLPRIVATE void importPresets( const css::uno::Reference< css::lang::XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, PresetCategoryList& rPresetMap  );

    SAL_DLLPRIVATE const OUString& translateName( const OUString& rId, const UStringMap& rNameMap ) const;

private:
    css::uno::Reference< css::animations::XAnimationNode > mxRootNode;
    EffectDescriptorMap maEffectDiscriptorMap;
    UStringMap maEffectNameMap;
    UStringMap maPropertyNameMap;

    PresetCategoryList maEntrancePresets;
    PresetCategoryList maEmphasisPresets;
    PresetCategoryList maExitPresets;
    PresetCategoryList maMotionPathsPresets;
    PresetCategoryList maMiscPresets;

    SAL_DLLPRIVATE static CustomAnimationPresets*  mpCustomAnimationPresets;
};


css::uno::Reference< css::animations::XAnimationNode > implImportEffects( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceFactory, const OUString& rPath );

void implImportLabels( const css::uno::Reference< css::lang::XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, UStringMap& rStringMap );

}


#endif // _SD_CUSTOMANIMATIONEFFECTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
