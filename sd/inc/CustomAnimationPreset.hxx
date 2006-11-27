/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CustomAnimationPreset.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-11-27 09:39:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#define _SD_CUSTOMANIMATIONPRESET_HXX

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONNODETYPE_HPP_
#include <com/sun/star/animations/AnimationNodeType.hpp>
#endif

#ifndef _UTL_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _SD_CUSTOMANIMATIONEFFECT_HXX
#include <CustomAnimationEffect.hxx>
#endif

#include <hash_map>

namespace sd {

typedef std::hash_map<rtl::OUString, CustomAnimationEffectPtr, comphelper::UStringHash, comphelper::UStringEqual> EffectsSubTypeMap;
typedef std::hash_map<rtl::OUString, rtl::OUString, comphelper::UStringHash, comphelper::UStringEqual> UStringMap;
typedef std::vector< rtl::OUString > UStringList;

class CustomAnimationPreset
{
    friend class CustomAnimationPresets;

public:
    CustomAnimationPreset( CustomAnimationEffectPtr pEffect );

    void add( CustomAnimationEffectPtr pEffect );

    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > create( const rtl::OUString& rstrSubType );

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
typedef std::hash_map<rtl::OUString, CustomAnimationPresetPtr, comphelper::UStringHash, comphelper::UStringEqual> EffectDescriptorMap;
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

    static const CustomAnimationPresets& getCustomAnimationPresets();

    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > getRandomPreset( sal_Int16 nPresetClass ) const;

    CustomAnimationPresetPtr getEffectDescriptor( const rtl::OUString& rPresetId ) const;
//  const AnimationEffect* getEffect( const rtl::OUString& rPresetId ) const;
//  const AnimationEffect* getEffect( const rtl::OUString& rPresetId, const rtl::OUString& rPresetSubType ) const;

    const rtl::OUString& getUINameForPresetId( const rtl::OUString& rPresetId ) const;
    const rtl::OUString& getUINameForProperty( const rtl::OUString& rProperty ) const;

    const PresetCategoryList& getEntrancePresets() const { return maEntrancePresets; }
    const PresetCategoryList& getEmphasisPresets() const { return maEmphasisPresets; }
    const PresetCategoryList& getExitPresets() const { return maExitPresets; }
    const PresetCategoryList& getMotionPathsPresets() const { return maMotionPathsPresets; }

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

    static CustomAnimationPresets*  mpCustomAnimationPresets;
};

typedef boost::shared_ptr< CustomAnimationPresets > CustomAnimationPresetsPtr;

}

#endif // _SD_CUSTOMANIMATIONEFFECTS_HXX

