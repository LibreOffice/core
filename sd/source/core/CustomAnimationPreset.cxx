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

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
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
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
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

using ::rtl::OUString;
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

        xConfigAccess = Reference< XNameAccess >::query(
            xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationAccess" ,
                aArgs ));
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
            OUString aLabel( "Label" );
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
                    xNameAccess->getByName( aLabel ) >>= aUIName;
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

void CustomAnimationPreset::add( CustomAnimationEffectPtr pEffect )
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

Reference< XAnimationNode > CustomAnimationPreset::create( const rtl::OUString& rstrSubType )
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
    String aProperties( maProperty );
    sal_uInt16 nTokens = comphelper::string::getTokenCount(aProperties, ';');
    sal_uInt16 nToken;
    UStringList aPropertyList;
    for( nToken = 0; nToken < nTokens; nToken++ )
        aPropertyList.push_back( aProperties.GetToken( nToken ) );

    return aPropertyList;

}

bool CustomAnimationPreset::hasProperty( const OUString& rProperty )const
{
    String aProperties( maProperty );
    String aProperty( rProperty );
    sal_uInt16 nTokens = comphelper::string::getTokenCount(aProperties, ';');
    sal_uInt16 nToken;
    for( nToken = 0; nToken < nTokens; nToken++ )
    {
        if( aProperties.GetToken( nToken ) == aProperty )
            return true;
    }

    return false;
}

CustomAnimationPresets::CustomAnimationPresets()
{
}

CustomAnimationPresets::~CustomAnimationPresets()
{
}

void CustomAnimationPresets::init()
{
    importResources();
}

Reference< XAnimationNode > implImportEffects( const Reference< XMultiServiceFactory >& xServiceFactory, const OUString& rPath )
{
    Reference< XAnimationNode > xRootNode;

    try
    {
        // create stream
        SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( rPath, STREAM_READ );
        Reference<XInputStream> xInputStream( new utl::OInputStreamWrapper( pIStm, sal_True ) );

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

#define EXPAND_PROTOCOL "vnd.sun.star.expand:"

void CustomAnimationPresets::importEffects()
{
    try
    {
        uno::Reference< uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager(), UNO_QUERY_THROW );

        uno::Reference< util::XMacroExpander > xMacroExpander(
            xContext->getValueByName("/singletons/com.sun.star.util.theMacroExpander"),
            UNO_QUERY );

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
        uno::Sequence< rtl::OUString > aFiles;
        xNameAccess->getByName( "EffectFiles" ) >>= aFiles;

        for( sal_Int32 i=0; i<aFiles.getLength(); ++i )
        {
            rtl::OUString aURL = aFiles[i];
            if( aURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( EXPAND_PROTOCOL )) == 0 )
            {
                // cut protocol
                rtl::OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
                // decode uric class chars
                aMacro = rtl::Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                // expand macro string
                aURL = xMacroExpander->expandMacros( aMacro );
            }

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
    String aMissedPresetIds;
#endif

    try
    {
        Reference< XNameAccess > xTypeAccess( getNodeAccess( xConfigProvider, rNodePath ) );
        if( xTypeAccess.is() )
        {
            Reference< XNameAccess > xCategoryAccess;
            const OUString aEffectsName( "Effects" );
            const OUString aLabelName( "Label" );

            Sequence< OUString > aNames( xTypeAccess->getElementNames() );
            const OUString* p = aNames.getConstArray();
            sal_Int32 n = aNames.getLength();
            while(n--)
            {
                xTypeAccess->getByName( *p ) >>= xCategoryAccess;

                if( xCategoryAccess.is() && xCategoryAccess->hasByName( aLabelName ) && xCategoryAccess->hasByName( aEffectsName ) )
                {
                    OUString aLabel;
                    xCategoryAccess->getByName( aLabelName ) >>= aLabel;

                    Sequence< OUString > aEffects;
                    xCategoryAccess->getByName( aEffectsName ) >>= aEffects;

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
                            aMissedPresetIds += String(*pEffectNames);
                            aMissedPresetIds += String( RTL_CONSTASCII_USTRINGPARAM("\n") );
                        }
#endif
                        pEffectNames++;
                    }
                    rPresetMap.push_back( PresetCategoryPtr( new PresetCategory( aLabel, aEffectsList ) ) );
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
    if( aMissedPresetIds.Len() )
    {
        rtl::OStringBuffer aTmp(RTL_CONSTASCII_STRINGPARAM("sd::CustomAnimationPresets::importPresets(), invalid preset id!\n"));
        aTmp.append(rtl::OUStringToOString(aMissedPresetIds,
            RTL_TEXTENCODING_ASCII_US));
        OSL_FAIL(aTmp.getStr());
    }
#endif
}

CustomAnimationPresetPtr CustomAnimationPresets::getEffectDescriptor( const rtl::OUString& rPresetId ) const
{
    EffectDescriptorMap::const_iterator aIter( maEffectDiscriptorMap.find( rPresetId ) );

    if( aIter != maEffectDiscriptorMap.end() )
    {
        return (*aIter).second;
    }
    else
    {
        return CustomAnimationPresetPtr((CustomAnimationPreset*)0);
    }
}

const rtl::OUString& CustomAnimationPresets::getUINameForPresetId( const rtl::OUString& rPresetId ) const
{
    return translateName( rPresetId, maEffectNameMap );
}

const rtl::OUString& CustomAnimationPresets::getUINameForProperty( const rtl::OUString& rPresetId ) const
{
    return translateName( rPresetId, maPropertyNameMap );
}

const rtl::OUString& CustomAnimationPresets::translateName( const rtl::OUString& rId, const UStringMap& rNameMap ) const
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
void CustomAnimationPresets::changePresetSubType( CustomAnimationEffectPtr pEffect, const rtl::OUString& rPresetSubType ) const
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

CustomAnimationPresets* CustomAnimationPresets::mpCustomAnimationPresets = 0;

const CustomAnimationPresets& CustomAnimationPresets::getCustomAnimationPresets()
{
    if( !mpCustomAnimationPresets )
    {
        SolarMutexGuard aGuard;

        if( !mpCustomAnimationPresets )
        {
            mpCustomAnimationPresets = new sd::CustomAnimationPresets();
            mpCustomAnimationPresets->init();
        }
    }

    return *mpCustomAnimationPresets;
}

Reference< XAnimationNode > CustomAnimationPresets::getRandomPreset( sal_Int16 nPresetClass ) const
{
    Reference< XAnimationNode > xNode;

    const PresetCategoryList* pCategoryList = 0;
    switch( nPresetClass )
    {
    case EffectPresetClass::ENTRANCE:   pCategoryList = &maEntrancePresets; break;
    case EffectPresetClass::EXIT:       pCategoryList = &maExitPresets; break;
    case EffectPresetClass::EMPHASIS:   pCategoryList = &maEmphasisPresets; break;
    case EffectPresetClass::MOTIONPATH: pCategoryList = &maMotionPathsPresets; break;
    default:
        pCategoryList = 0;
    }

    if( pCategoryList && pCategoryList->size() )
    {
        sal_Int32 nCategory = (rand() * pCategoryList->size() / RAND_MAX);

        PresetCategoryPtr pCategory = (*pCategoryList)[nCategory];
        if( pCategory.get() && !pCategory->maEffects.empty() )
        {
            sal_Int32 nDescriptor = (rand() * pCategory->maEffects.size() / RAND_MAX);
            CustomAnimationPresetPtr pPreset = pCategory->maEffects[nDescriptor];
            if( pPreset.get() )
            {
                UStringList aSubTypes = pPreset->getSubTypes();

                OUString aSubType;
                if( !aSubTypes.empty() )
                {
                    sal_Int32 nSubType = (rand() * aSubTypes.size() / RAND_MAX);
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
