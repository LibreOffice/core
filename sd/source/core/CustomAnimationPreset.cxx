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

#include <sal/config.h>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <unotools/streamwrap.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/random.hxx>
#include <comphelper/lok.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <tools/stream.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>

#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <CustomAnimationPreset.hxx>

#include <algorithm>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::io::XInputStream;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::beans::NamedValue;

namespace sd {

static Reference< XNameAccess > getNodeAccess( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath )
{
    Reference< XNameAccess > xConfigAccess;

    try
    {
        Sequence<Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(rNodePath)}
        }));

        xConfigAccess.set(
            xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess", aArgs ),
            UNO_QUERY);
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::getNodeAccess()" );
    }

    return xConfigAccess;
}

void implImportLabels( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, UStringMap& rStringMap )
{
    try
    {
        Reference< XNameAccess > xConfigAccess( getNodeAccess( xConfigProvider, rNodePath ) );
        if( xConfigAccess.is() )
        {
            Reference< XNameAccess > xNameAccess;
            const Sequence< OUString > aNames( xConfigAccess->getElementNames() );
            for(const OUString& rName : aNames)
            {
                xConfigAccess->getByName( rName ) >>= xNameAccess;
                if( xNameAccess.is() )
                {
                    OUString aUIName;
                    xNameAccess->getByName( "Label" ) >>= aUIName;
                    if( !aUIName.isEmpty() )
                    {
                        rStringMap[ rName ] = aUIName;
                    }
                }
            }
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::implImportLabels()" );
    }
}

CustomAnimationPreset::CustomAnimationPreset( const CustomAnimationEffectPtr& pEffect )
{
    maPresetId = pEffect->getPresetId();
    maProperty = pEffect->getProperty();

    add( pEffect );

    mfDuration = pEffect->getDuration();
    maDefaultSubTyp = pEffect->getPresetSubType();

    const Sequence< NamedValue > aUserData( pEffect->getNode()->getUserData() );

    mbIsTextOnly = std::any_of(aUserData.begin(), aUserData.end(),
        [](const NamedValue& rProp) { return rProp.Name == "text-only"; });
}

void CustomAnimationPreset::add( const CustomAnimationEffectPtr& pEffect )
{
    maSubTypes[ pEffect->getPresetSubType() ] = pEffect;
}

std::vector<OUString> CustomAnimationPreset::getSubTypes()
{
    std::vector<OUString> aSubTypes;

    if( maSubTypes.size() > 1 )
    {
        std::transform(maSubTypes.begin(), maSubTypes.end(), std::back_inserter(aSubTypes),
            [](EffectsSubTypeMap::value_type& rEntry) -> OUString { return rEntry.first; });
    }

    return aSubTypes;
}

Reference< XAnimationNode > CustomAnimationPreset::create( const OUString& rstrSubType )
{
    try
    {
        OUString strSubType( rstrSubType );
        if( strSubType.isEmpty() )
            strSubType = maDefaultSubTyp;

        CustomAnimationEffectPtr pEffect = maSubTypes[strSubType];
        if( pEffect )
        {
            Reference< XCloneable > xCloneable( pEffect->getNode(), UNO_QUERY_THROW );
            Reference< XAnimationNode > xNode( xCloneable->createClone(), UNO_QUERY_THROW );
            return xNode;
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPresets::create()" );
    }

    Reference< XAnimationNode > xNode;
    return xNode;
}

std::vector<OUString> CustomAnimationPreset::getProperties() const
{
    std::vector<OUString> aPropertyList;
    if (!maProperty.isEmpty())
    {
        sal_Int32 nPos = 0;
        do
        {
            aPropertyList.push_back(maProperty.getToken(0, ';', nPos));
        }
        while (nPos >= 0);
    }
    return aPropertyList;
}

bool CustomAnimationPreset::hasProperty( std::u16string_view rProperty )const
{
    if (maProperty.isEmpty())
        return false;

    sal_Int32 nPos = 0;
    do
    {
        if (o3tl::getToken(maProperty, 0, ';', nPos) == rProperty)
            return true;
    }
    while (nPos >= 0);

    return false;
}

CustomAnimationPresets::CustomAnimationPresets()
{
}

CustomAnimationPresets::~CustomAnimationPresets()
{
}

Reference< XAnimationNode > implImportEffects( const Reference< XMultiServiceFactory >& xServiceFactory, const OUString& rPath )
{
    Reference< XAnimationNode > xRootNode;

    try
    {
        // create stream
        std::unique_ptr<SvStream> pIStm = ::utl::UcbStreamHelper::CreateStream( rPath, StreamMode::READ );

        // prepare ParserInputSource
        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = rPath;
        aParserInput.aInputStream.set(new utl::OInputStreamWrapper(std::move(pIStm)));

        // get filter
        Reference< xml::sax::XFastParser > xFilter( xServiceFactory->createInstance("com.sun.star.comp.Xmloff.AnimationsImport" ), UNO_QUERY_THROW );

        xFilter->parseStream( aParserInput );

        Reference< XAnimationNodeSupplier > xAnimationNodeSupplier( xFilter, UNO_QUERY_THROW );
        xRootNode = xAnimationNodeSupplier->getAnimationNode();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("sd", "");
    }

    return xRootNode;
}

void CustomAnimationPresets::importEffects()
{
    try
    {
        uno::Reference< uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager(), UNO_QUERY_THROW );

        Reference< XMultiServiceFactory > xConfigProvider =
            configuration::theDefaultProvider::get( xContext );

        // read path to transition effects files from config
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(OUString("/org.openoffice.Office.Impress/Misc"))}
        }));
        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                aArgs ), UNO_QUERY_THROW );
        uno::Sequence< OUString > aFiles;
        xNameAccess->getByName( "EffectFiles" ) >>= aFiles;

        for (const auto& rFile : aFiles)
        {
            OUString aURL = comphelper::getExpandedUri(xContext, rFile);

            mxRootNode = implImportEffects( xServiceFactory, aURL );

            if( mxRootNode.is() )
            {
                Reference< XTimeContainer > xRootContainer( mxRootNode, UNO_QUERY_THROW );
                EffectSequenceHelper aSequence( xRootContainer );

                EffectSequence::iterator aIter( aSequence.getBegin() );
                const EffectSequence::iterator aEnd( aSequence.getEnd() );

                while( aIter != aEnd )
                {
                    CustomAnimationEffectPtr pEffect = *aIter;

                    const OUString aPresetId( pEffect->getPresetId() );
                    CustomAnimationPresetPtr pDescriptor = getEffectDescriptor( aPresetId );
                    if( pDescriptor )
                        pDescriptor->add( pEffect );
                    else
                    {
                        pDescriptor = std::make_shared<CustomAnimationPreset>( pEffect );
                        pDescriptor->maLabel = getUINameForPresetId( pEffect->getPresetId() );
                        maEffectDescriptorMap[aPresetId] = pDescriptor;
                    }

                    ++aIter;
                }
            }
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPresets::importEffects()" );
    }
}

void CustomAnimationPresets::importResources()
{
    try
    {
        // Get service factory
        Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

        Reference< XMultiServiceFactory > xConfigProvider =
             configuration::theDefaultProvider::get( xContext );

        implImportLabels( xConfigProvider, "/org.openoffice.Office.UI.Effects/UserInterface/Properties", maPropertyNameMap );

        implImportLabels( xConfigProvider, "/org.openoffice.Office.UI.Effects/UserInterface/Effects", maEffectNameMap );

        importEffects();

        importPresets( xConfigProvider, "/org.openoffice.Office.UI.Effects/Presets/Entrance", maEntrancePresets );

        importPresets( xConfigProvider, "/org.openoffice.Office.UI.Effects/Presets/Emphasis", maEmphasisPresets );

        importPresets( xConfigProvider, "/org.openoffice.Office.UI.Effects/Presets/Exit", maExitPresets );

        importPresets( xConfigProvider, "/org.openoffice.Office.UI.Effects/Presets/MotionPaths", maMotionPathsPresets );

        importPresets( xConfigProvider, "/org.openoffice.Office.UI.Effects/Presets/Misc", maMiscPresets );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPresets::importResources()" );
    }
}

void CustomAnimationPresets::importPresets( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, PresetCategoryList& rPresetMap  )
{
#if OSL_DEBUG_LEVEL >= 2
    OUString aMissedPresetIds;
#endif

    try
    {
        Reference< XNameAccess > xTypeAccess( getNodeAccess( xConfigProvider, rNodePath ) );
        if( xTypeAccess.is() )
        {
            Reference< XNameAccess > xCategoryAccess;

            const Sequence< OUString > aNames( xTypeAccess->getElementNames() );
            for(const OUString& rName : aNames)
            {
                xTypeAccess->getByName( rName ) >>= xCategoryAccess;

                if( xCategoryAccess.is() && xCategoryAccess->hasByName( "Label" ) && xCategoryAccess->hasByName( "Effects" ) )
                {
                    OUString aLabel;
                    xCategoryAccess->getByName( "Label" ) >>= aLabel;

                    Sequence< OUString > aEffects;
                    xCategoryAccess->getByName( "Effects" ) >>= aEffects;

                    EffectDescriptorList aEffectsList;

                    for (const OUString& rEffectName : aEffects)
                    {
                        CustomAnimationPresetPtr pEffect = getEffectDescriptor( rEffectName );
                        if( pEffect )
                        {
                            aEffectsList.push_back( pEffect );
                        }
#if OSL_DEBUG_LEVEL >= 2
                        else
                        {
                            aMissedPresetIds += OUString(rEffectName);
                            aMissedPresetIds += "\n";
                        }
#endif
                    }
                    rPresetMap.push_back( std::make_shared<PresetCategory>( aLabel, std::move(aEffectsList) ) );
                }
            }
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPresets::importPresets()" );
    }

#if OSL_DEBUG_LEVEL >= 2
    SAL_WARN_IF(!aMissedPresetIds.isEmpty(), "sd", "sd::CustomAnimationPresets::importPresets(), invalid preset id: "
                    << aMissedPresetIds);
#endif
}

CustomAnimationPresetPtr CustomAnimationPresets::getEffectDescriptor( const OUString& rPresetId ) const
{
    EffectDescriptorMap::const_iterator aIter( maEffectDescriptorMap.find( rPresetId ) );

    if( aIter != maEffectDescriptorMap.end() )
    {
        return (*aIter).second;
    }
    else
    {
        return CustomAnimationPresetPtr(nullptr);
    }
}

const OUString& CustomAnimationPresets::getUINameForPresetId( const OUString& rPresetId ) const
{
    return translateName( rPresetId, maEffectNameMap );
}

const OUString& CustomAnimationPresets::getUINameForProperty( const OUString& rPresetId ) const
{
    return translateName( rPresetId, maPropertyNameMap );
}

const OUString& CustomAnimationPresets::translateName( const OUString& rId, const UStringMap& rNameMap )
{
    UStringMap::const_iterator aIter( rNameMap.find( rId ) );

    if( aIter != rNameMap.end() )
    {
        return (*aIter).second;
    }
    else
    {
        return rId;
    }
}
void CustomAnimationPresets::changePresetSubType( const CustomAnimationEffectPtr& pEffect, const OUString& rPresetSubType ) const
{
    if( pEffect && pEffect->getPresetSubType() != rPresetSubType )
    {
        CustomAnimationPresetPtr pDescriptor( getEffectDescriptor( pEffect->getPresetId() ) );

        if( pDescriptor )
        {
            Reference< XAnimationNode > xNewNode( pDescriptor->create( rPresetSubType ) );
            if( xNewNode.is() )
                pEffect->replaceNode( xNewNode );
        }
    }
}

std::map<OUString, CustomAnimationPresets>  CustomAnimationPresets::mPresetsMap;

const CustomAnimationPresets& CustomAnimationPresets::getCustomAnimationPresets()
{
    // Support localization per-view. Currently not useful for Desktop
    // but very much critical for LOK. The cache now is per-language.
    const OUString aLang = comphelper::LibreOfficeKit::isActive()
                               ? comphelper::LibreOfficeKit::getLanguageTag().getBcp47()
                               : SvtSysLocaleOptions().GetLanguageTag().getBcp47();

    SolarMutexGuard aGuard;
    const auto it = mPresetsMap.find(aLang);
    if (it != mPresetsMap.end())
        return it->second;

    CustomAnimationPresets& rPresets = mPresetsMap[aLang];
    rPresets.importResources();
    return rPresets;
}

Reference< XAnimationNode > CustomAnimationPresets::getRandomPreset( sal_Int16 nPresetClass ) const
{
    Reference< XAnimationNode > xNode;

    const PresetCategoryList* pCategoryList = nullptr;
    switch( nPresetClass )
    {
    case EffectPresetClass::ENTRANCE:   pCategoryList = &maEntrancePresets; break;
    case EffectPresetClass::EXIT:       pCategoryList = &maExitPresets; break;
    case EffectPresetClass::EMPHASIS:   pCategoryList = &maEmphasisPresets; break;
    case EffectPresetClass::MOTIONPATH: pCategoryList = &maMotionPathsPresets; break;
    default:
        pCategoryList = nullptr;
    }

    if( pCategoryList && !pCategoryList->empty() )
    {
        sal_Int32 nCategory = comphelper::rng::uniform_size_distribution(0, pCategoryList->size()-1);

        PresetCategoryPtr pCategory = (*pCategoryList)[nCategory];
        if( pCategory && !pCategory->maEffects.empty() )
        {
            sal_Int32 nDescriptor = comphelper::rng::uniform_size_distribution(0, pCategory->maEffects.size()-1);
            CustomAnimationPresetPtr pPreset = pCategory->maEffects[nDescriptor];
            if( pPreset )
            {
                std::vector<OUString> aSubTypes = pPreset->getSubTypes();

                OUString aSubType;
                if( !aSubTypes.empty() )
                {
                    size_t nSubType = comphelper::rng::uniform_size_distribution(0, aSubTypes.size()-1);
                    aSubType = aSubTypes[nSubType];
                }
                xNode = pPreset->create( aSubType );
            }
        }
    }

    return xNode;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
