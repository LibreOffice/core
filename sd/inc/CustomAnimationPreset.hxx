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

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#define _SD_CUSTOMANIMATIONPRESET_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>

#include <comphelper/stl_types.hxx>
#include <CustomAnimationEffect.hxx>

#include <boost/unordered_map.hpp>

namespace sd {

typedef boost::unordered_map< OUString, CustomAnimationEffectPtr, OUStringHash, comphelper::UStringEqual > EffectsSubTypeMap;
typedef boost::unordered_map< OUString, OUString, OUStringHash, comphelper::UStringEqual > UStringMap;
typedef std::vector< OUString > UStringList;

class CustomAnimationPreset
{
    friend class CustomAnimationPresets;

public:
    CustomAnimationPreset( CustomAnimationEffectPtr pEffect );

    void add( CustomAnimationEffectPtr pEffect );

    SD_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > create( const OUString& rstrSubType );

    const OUString& getPresetId() const { return maPresetId; }
    const OUString& getProperty() const { return maProperty; }
    const OUString& getLabel() const { return maLabel; }
    sal_Int16 getPresetClass() const { return mnPresetClass; }
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

typedef boost::shared_ptr< CustomAnimationPreset > CustomAnimationPresetPtr;
typedef boost::unordered_map<OUString, CustomAnimationPresetPtr, OUStringHash, comphelper::UStringEqual> EffectDescriptorMap;
typedef std::vector< CustomAnimationPresetPtr > EffectDescriptorList;

struct PresetCategory
{
    OUString maLabel;
    EffectDescriptorList maEffects;

    PresetCategory( const OUString& rLabel, const EffectDescriptorList& rEffects )
        : maLabel( rLabel ), maEffects( rEffects ) {}
};
typedef boost::shared_ptr< PresetCategory > PresetCategoryPtr;
typedef std::vector< PresetCategoryPtr > PresetCategoryList;

class CustomAnimationPresets
{
public:
    CustomAnimationPresets();
    virtual ~CustomAnimationPresets();

    void init();

    SD_DLLPUBLIC static const CustomAnimationPresets& getCustomAnimationPresets();

    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > getRandomPreset( sal_Int16 nPresetClass ) const;

    SD_DLLPUBLIC CustomAnimationPresetPtr getEffectDescriptor( const OUString& rPresetId ) const;

    const OUString& getUINameForPresetId( const OUString& rPresetId ) const;
    const OUString& getUINameForProperty( const OUString& rProperty ) const;

    const PresetCategoryList& getEntrancePresets() const { return maEntrancePresets; }
    const PresetCategoryList& getEmphasisPresets() const { return maEmphasisPresets; }
    const PresetCategoryList& getExitPresets() const { return maExitPresets; }
    const PresetCategoryList& getMotionPathsPresets() const { return maMotionPathsPresets; }
    const PresetCategoryList& getMiscPresets() const { return maMiscPresets; }

    void changePresetSubType( CustomAnimationEffectPtr pEffect, const OUString& rPresetSubType ) const;

private:
    void importEffects();
    void importResources();

    void importPresets( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, PresetCategoryList& rPresetMap  );

    const OUString& translateName( const OUString& rId, const UStringMap& rNameMap ) const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxRootNode;
    EffectDescriptorMap maEffectDiscriptorMap;
    UStringMap maEffectNameMap;
    UStringMap maPropertyNameMap;

    PresetCategoryList maEntrancePresets;
    PresetCategoryList maEmphasisPresets;
    PresetCategoryList maExitPresets;
    PresetCategoryList maMotionPathsPresets;
    PresetCategoryList maMiscPresets;

    static CustomAnimationPresets*  mpCustomAnimationPresets;
};

typedef boost::shared_ptr< CustomAnimationPresets > CustomAnimationPresetsPtr;

}

#endif // _SD_CUSTOMANIMATIONEFFECTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
