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

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <unotools/streamwrap.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>

#include <tools/debug.hxx>
#include <rtl/uri.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <CustomAnimationPreset.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::io::XInputStream;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::beans::NamedValue;

namespace sd {

static Reference< XNameAccess > getNodeAccess( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath )
{
    Reference< XNameAccess > xConfigAccess;

    try
    {
        Sequence< Any > aArgs( 1 );
        PropertyValue   aPropValue;
        aPropValue.Name  = "nodepath";
        aPropValue.Value <<= rNodePath;
        aArgs[0] <<= aPropValue;

        xConfigAccess.set(
            xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess", aArgs ),
            UNO_QUERY);
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::getNodeAccess(), Exception caught!" );
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
            Sequence< OUString > aNames( xConfigAccess->getElementNames() );
            const OUString* p = aNames.getConstArray();
            sal_Int32 n = aNames.getLength();
            while(n--)
            {
                xConfigAccess->getByName( *p ) >>= xNameAccess;
                if( xNameAccess.is() )
                {
                    OUString aUIName;
                    xNameAccess->getByName( "Label" ) >>= aUIName;
                    if( !aUIName.isEmpty() )
                    {
                        rStringMap[ *p ] = aUIName;
                    }
                }

                p++;
            }
        }
    }
    catch (const lang::WrappedTargetException&)
    {
        OSL_FAIL( "sd::implImportLabels(), WrappedTargetException caught!" );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::implImportLabels(), Exception caught!" );
    }
}

CustomAnimationPreset::CustomAnimationPreset( CustomAnimationEffectPtr pEffect )
{
    maPresetId = pEffect->getPresetId();
    maProperty = pEffect->getProperty();
    mnPresetClass = pEffect->getPresetClass();

    add( pEffect );

    mfDuration = pEffect->getDuration();
    maDefaultSubTyp = pEffect->getPresetSubType();

    mbIsTextOnly = false;

    Sequence< NamedValue > aUserData( pEffect->getNode()->getUserData() );
    sal_Int32 nLength = aUserData.getLength();
    const NamedValue* p = aUserData.getConstArray();

    while( nLength-- )
    {
        if ( p->Name == "text-only" )
        {
            mbIsTextOnly = true;
            break;
        }
        p++;
    }

}

void CustomAnimationPreset::add( const CustomAnimationEffectPtr& pEffect )
{
    maSubTypes[ pEffect->getPresetSubType() ] = pEffect;
}

UStringList CustomAnimationPreset::getSubTypes()
{
    UStringList aSubTypes;

    if( maSubTypes.size() > 1 )
    {
        EffectsSubTypeMap::iterator aIter( maSubTypes.begin() );
        const EffectsSubTypeMap::iterator aEnd( maSubTypes.end() );
        while( aIter != aEnd )
            aSubTypes.push_back( (*aIter++).first );
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
        if( pEffect.get() )
        {
            Reference< XCloneable > xCloneable( pEffect->getNode(), UNO_QUERY_THROW );
            Reference< XAnimationNode > xNode( xCloneable->createClone(), UNO_QUERY_THROW );
            return xNode;
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::create(), exception caught!" );
    }

    Reference< XAnimationNode > xNode;
    return xNode;
}

UStringList CustomAnimationPreset::getProperties() const
{
    UStringList aPropertyList;
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

bool CustomAnimationPreset::hasProperty( const OUString& rProperty )const
{
    if (maProperty.isEmpty())
        return false;

    sal_Int32 nPos = 0;
    do
    {
        if (maProperty.getToken(0, ';', nPos) == rProperty)
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
        SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( rPath, StreamMode::READ );
        Reference<XInputStream> xInputStream( new utl::OInputStreamWrapper( pIStm, true ) );

        // prepare ParserInputSrouce
        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = rPath;
        aParserInput.aInputStream = xInputStream;

        // get parser
        Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( comphelper::getComponentContext(xServiceFactory) );

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter( xServiceFactory->createInstance("com.sun.star.comp.Xmloff.AnimationsImport" ), UNO_QUERY );

        DBG_ASSERT( xFilter.is(), "Can't instantiate filter component." );
        if( !xFilter.is() )
            return xRootNode;

        // connect parser and filter
        xParser->setDocumentHandler( xFilter );

        // finally, parser the stream
        xParser->parseStream( aParserInput );

        Reference< XAnimationNodeSupplier > xAnimationNodeSupplier( xFilter, UNO_QUERY );
        if( xAnimationNodeSupplier.is() )
            xRootNode = xAnimationNodeSupplier->getAnimationNode();
    }
    catch (const xml::sax::SAXParseException&)
    {
        OSL_FAIL( "sd::implImportEffects(), SAXParseException caught!" );
    }
    catch (const xml::sax::SAXException&)
    {
        OSL_FAIL( "sd::implImportEffects(), SAXException caught!" );
    }
    catch (const io::IOException&)
    {
        OSL_FAIL( "sd::implImportEffects(), IOException caught!" );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::importEffects(), Exception caught!" );
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

        uno::Reference< util::XMacroExpander > xMacroExpander =
            util::theMacroExpander::get(xContext);

        Reference< XMultiServiceFactory > xConfigProvider =
            configuration::theDefaultProvider::get( xContext );

        // read path to transition effects files from config
        Any propValue = uno::makeAny(
            beans::PropertyValue(
                "nodepath", -1,
                uno::makeAny( OUString( "/org.openoffice.Office.Impress/Misc" )),
                beans::PropertyState_DIRECT_VALUE ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );
        uno::Sequence< OUString > aFiles;
        xNameAccess->getByName( "EffectFiles" ) >>= aFiles;

        for( sal_Int32 i=0; i<aFiles.getLength(); ++i )
        {
            OUString aURL = comphelper::getExpandedUri(xContext, aFiles[i]);

            mxRootNode = implImportEffects( xServiceFactory, aURL );

            if( mxRootNode.is() )
            {
                Reference< XTimeContainer > xRootContainer( mxRootNode, UNO_QUERY_THROW );
                EffectSequenceHelper aSequence( xRootContainer );

                EffectSequence::iterator aIter( aSequence.getBegin() );
                const EffectSequence::iterator aEnd( aSequence.getEnd() );

                while( aIter != aEnd )
                {
                    CustomAnimationEffectPtr pEffect = (*aIter);

                    const OUString aPresetId( pEffect->getPresetId() );
                    CustomAnimationPresetPtr pDescriptor = getEffectDescriptor( aPresetId );
                    if( pDescriptor.get() )
                        pDescriptor->add( pEffect );
                    else
                    {
                        pDescriptor.reset( new CustomAnimationPreset( pEffect ) );
                        pDescriptor->maLabel = getUINameForPresetId( pEffect->getPresetId() );
                        maEffectDiscriptorMap[aPresetId] = pDescriptor;
                    }

                    ++aIter;
                }
            }
        }
    }
    catch (const xml::sax::SAXParseException&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), SAXParseException caught!" );
    }
    catch (const xml::sax::SAXException&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), SAXException caught!" );
    }
    catch (const io::IOException&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), IOException caught!" );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), Exception caught!" );
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

        const OUString aPropertyPath("/org.openoffice.Office.UI.Effects/UserInterface/Properties" );
        implImportLabels( xConfigProvider, aPropertyPath, maPropertyNameMap );

        const OUString aEffectsPath( "/org.openoffice.Office.UI.Effects/UserInterface/Effects" );
        implImportLabels( xConfigProvider, aEffectsPath, maEffectNameMap );

        importEffects();

        const OUString aEntrancePath( "/org.openoffice.Office.UI.Effects/Presets/Entrance" );
        importPresets( xConfigProvider, aEntrancePath, maEntrancePresets );

        const OUString aEmphasisPath( "/org.openoffice.Office.UI.Effects/Presets/Emphasis" );
        importPresets( xConfigProvider, aEmphasisPath, maEmphasisPresets );

        const OUString aExitPath( "/org.openoffice.Office.UI.Effects/Presets/Exit" );
        importPresets( xConfigProvider, aExitPath, maExitPresets );

        const OUString aMotionPathsPath( "/org.openoffice.Office.UI.Effects/Presets/MotionPaths" );
        importPresets( xConfigProvider, aMotionPathsPath, maMotionPathsPresets );

        const OUString aMiscPath( "/org.openoffice.Office.UI.Effects/Presets/Misc" );
        importPresets( xConfigProvider, aMiscPath, maMiscPresets );
    }
    catch (const lang::WrappedTargetException&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::importResources(), WrappedTargetException caught!" );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::importResources(), Exception caught!" );
    }
}

void CustomAnimationPresets::importPresets( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, PresetCategoryList& rPresetMap  )
{
#ifdef DEBUG
    OUString aMissedPresetIds;
#endif

    try
    {
        Reference< XNameAccess > xTypeAccess( getNodeAccess( xConfigProvider, rNodePath ) );
        if( xTypeAccess.is() )
        {
            Reference< XNameAccess > xCategoryAccess;

            Sequence< OUString > aNames( xTypeAccess->getElementNames() );
            const OUString* p = aNames.getConstArray();
            sal_Int32 n = aNames.getLength();
            while(n--)
            {
                xTypeAccess->getByName( *p ) >>= xCategoryAccess;

                if( xCategoryAccess.is() && xCategoryAccess->hasByName( "Label" ) && xCategoryAccess->hasByName( "Effects" ) )
                {
                    OUString aLabel;
                    xCategoryAccess->getByName( "Label" ) >>= aLabel;

                    Sequence< OUString > aEffects;
                    xCategoryAccess->getByName( "Effects" ) >>= aEffects;

                    EffectDescriptorList aEffectsList;

                    const OUString* pEffectNames = aEffects.getConstArray();
                    sal_Int32 nEffectCount = aEffects.getLength();
                    while( nEffectCount-- )
                    {
                        CustomAnimationPresetPtr pEffect = getEffectDescriptor( *pEffectNames );
                        if( pEffect.get() )
                        {
                            aEffectsList.push_back( pEffect );
                        }
#ifdef DEBUG
                        else
                        {
                            aMissedPresetIds += OUString(*pEffectNames);
                            aMissedPresetIds += "\n";
                        }
#endif
                        pEffectNames++;
                    }
                    rPresetMap.push_back( std::make_shared<PresetCategory>( aLabel, aEffectsList ) );
                }

                p++;
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL( "sd::CustomAnimationPresets::importPresets(), Exception caught!" );
    }

#ifdef DEBUG
    if( !aMissedPresetIds.isEmpty() )
    {
        OStringBuffer aTmp("sd::CustomAnimationPresets::importPresets(), invalid preset id!\n");
        aTmp.append(OUStringToOString(aMissedPresetIds,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aTmp.getStr());
    }
#endif
}

CustomAnimationPresetPtr CustomAnimationPresets::getEffectDescriptor( const OUString& rPresetId ) const
{
    EffectDescriptorMap::const_iterator aIter( maEffectDiscriptorMap.find( rPresetId ) );

    if( aIter != maEffectDiscriptorMap.end() )
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
    if( pEffect.get() && pEffect->getPresetSubType() != rPresetSubType )
    {
        CustomAnimationPresetPtr pDescriptor( getEffectDescriptor( pEffect->getPresetId() ) );

        if( pDescriptor.get() )
        {
            Reference< XAnimationNode > xNewNode( pDescriptor->create( rPresetSubType ) );
            if( xNewNode.is() )
                pEffect->replaceNode( xNewNode );
        }
    }
}

CustomAnimationPresets* CustomAnimationPresets::mpCustomAnimationPresets = nullptr;

const CustomAnimationPresets& CustomAnimationPresets::getCustomAnimationPresets()
{
    if( !mpCustomAnimationPresets )
    {
        SolarMutexGuard aGuard;

        if( !mpCustomAnimationPresets )
        {
            mpCustomAnimationPresets = new sd::CustomAnimationPresets();
            mpCustomAnimationPresets->importResources();
        }
    }

    return *mpCustomAnimationPresets;
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

    if( pCategoryList && pCategoryList->size() )
    {
        sal_Int32 nCategory = comphelper::rng::uniform_size_distribution(0, pCategoryList->size()-1);

        PresetCategoryPtr pCategory = (*pCategoryList)[nCategory];
        if( pCategory.get() && !pCategory->maEffects.empty() )
        {
            sal_Int32 nDescriptor = comphelper::rng::uniform_size_distribution(0, pCategory->maEffects.size()-1);
            CustomAnimationPresetPtr pPreset = pCategory->maEffects[nDescriptor];
            if( pPreset.get() )
            {
                UStringList aSubTypes = pPreset->getSubTypes();

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
