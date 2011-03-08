/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#define _SD_CUSTOMANIMATIONPRESET_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>

#include <comphelper/stl_types.hxx>
#include <CustomAnimationEffect.hxx>

#include <boost/unordered_map.hpp>

namespace sd {

typedef boost::unordered_map< rtl::OUString, CustomAnimationEffectPtr, comphelper::UStringHash, comphelper::UStringEqual > EffectsSubTypeMap;
typedef boost::unordered_map< rtl::OUString, rtl::OUString, comphelper::UStringHash, comphelper::UStringEqual > UStringMap;
typedef std::vector< rtl::OUString > UStringList;

class CustomAnimationPreset
{
    friend class CustomAnimationPresets;

public:
    CustomAnimationPreset( CustomAnimationEffectPtr pEffect );

    void add( CustomAnimationEffectPtr pEffect );

    SD_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > create( const rtl::OUString& rstrSubType );

    const rtl::OUString& getPresetId() const { return maPresetId; }
    const rtl::OUString& getProperty() const { return maProperty; }
    const rtl::OUString& getLabel() const { return maLabel; }
    sal_Int16 getPresetClass() const { return mnPresetClass; }
    double getDuration() const { return mfDuration; }

    UStringList getSubTypes();
    UStringList getProperties() const;

    bool hasProperty( const rtl::OUString& rProperty ) const;
    bool isTextOnly() const { return mbIsTextOnly; }

private:
    rtl::OUString maPresetId;
    rtl::OUString maProperty;
    sal_Int16 mnPresetClass;
    rtl::OUString maLabel;
    rtl::OUString maDefaultSubTyp;
    double mfDuration;
    bool mbIsTextOnly;

    EffectsSubTypeMap maSubTypes;
};

typedef boost::shared_ptr< CustomAnimationPreset > CustomAnimationPresetPtr;
typedef boost::unordered_map<rtl::OUString, CustomAnimationPresetPtr, comphelper::UStringHash, comphelper::UStringEqual> EffectDescriptorMap;
typedef std::vector< CustomAnimationPresetPtr > EffectDescriptorList;

struct PresetCategory
{
    rtl::OUString maLabel;
    EffectDescriptorList maEffects;

    PresetCategory( const rtl::OUString& rLabel, const EffectDescriptorList& rEffects )
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

    SD_DLLPUBLIC CustomAnimationPresetPtr getEffectDescriptor( const rtl::OUString& rPresetId ) const;

    const rtl::OUString& getUINameForPresetId( const rtl::OUString& rPresetId ) const;
    const rtl::OUString& getUINameForProperty( const rtl::OUString& rProperty ) const;

    const PresetCategoryList& getEntrancePresets() const { return maEntrancePresets; }
    const PresetCategoryList& getEmphasisPresets() const { return maEmphasisPresets; }
    const PresetCategoryList& getExitPresets() const { return maExitPresets; }
    const PresetCategoryList& getMotionPathsPresets() const { return maMotionPathsPresets; }
    const PresetCategoryList& getMiscPresets() const { return maMiscPresets; }

    void changePresetSubType( CustomAnimationEffectPtr pEffect, const rtl::OUString& rPresetSubType ) const;

private:
    void importEffects();
    void importResources();

    void importPresets( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xConfigProvider, const rtl::OUString& rNodePath, PresetCategoryList& rPresetMap  );

    const rtl::OUString& translateName( const rtl::OUString& rId, const UStringMap& rNameMap ) const;

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
