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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>

#include <tools/debug.hxx>
#include <rtl/uri.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <CustomAnimationPreset.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::rtl::OUString;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
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
        aPropValue.Name  = OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
        aPropValue.Value <<= rNodePath;
        aArgs[0] <<= aPropValue;

        xConfigAccess = Reference< XNameAccess >::query(
            xConfigProvider->createInstanceWithArguments(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" )),
                aArgs ));
    }
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL( "sd::getNodeAccess(), Exception catched!" );
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
            OUString aLabel( RTL_CONSTASCII_USTRINGPARAM( "Label" ) );
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
                    if( aUIName.getLength() )
                    {
                        rStringMap[ *p ] = aUIName;
                    }
                }

                p++;
            }
        }
    }
    catch( lang::WrappedTargetException& e )
    {
        (void)e;
        OSL_FAIL( "sd::implImportLabels(), WrappedTargetException catched!" );
    }
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL( "sd::implImportLabels(), Exception catched!" );
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
        if( p->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "text-only" ) ) )
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
        if( strSubType.getLength() == 0 )
            strSubType = maDefaultSubTyp;

        CustomAnimationEffectPtr pEffect = maSubTypes[strSubType];
        if( pEffect.get() )
        {
            Reference< XCloneable > xCloneable( pEffect->getNode(), UNO_QUERY_THROW );
            Reference< XAnimationNode > xNode( xCloneable->createClone(), UNO_QUERY_THROW );
            return xNode;
        }
    }
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL( "sd::CustomAnimationPresets::create(), exception catched!" );
    }

    Reference< XAnimationNode > xNode;
    return xNode;
}

UStringList CustomAnimationPreset::getProperties() const
{
    String aProperties( maProperty );
    USHORT nTokens = aProperties.GetTokenCount();
    USHORT nToken;
    UStringList aPropertyList;
    for( nToken = 0; nToken < nTokens; nToken++ )
        aPropertyList.push_back( aProperties.GetToken( nToken ) );

    return aPropertyList;

}

bool CustomAnimationPreset::hasProperty( const OUString& rProperty )const
{
    String aProperties( maProperty );
    String aProperty( rProperty );
    USHORT nTokens = aProperties.GetTokenCount();
    USHORT nToken;
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
        Reference< xml::sax::XParser > xParser(
            xServiceFactory->createInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser")) ),
            UNO_QUERY );

        DBG_ASSERT( xParser.is(), "Can't create parser" );
        if( !xParser.is() )
            return xRootNode;

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter(
            xServiceFactory->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Xmloff.AnimationsImport" ) ) ), UNO_QUERY );

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
    catch( xml::sax::SAXParseException& r )
    {
        (void)r;
        OSL_FAIL( "sd::implImportEffects(), SAXParseException catched!" );
    }
    catch( xml::sax::SAXException& r )
    {
        (void)r;
        OSL_FAIL( "sd::implImportEffects(), SAXException catched!" );
    }
    catch( io::IOException& r )
    {
        (void)r;
        OSL_FAIL( "sd::implImportEffects(), IOException catched!" );
    }
    catch( Exception& r )
    {
        (void)r;
        OSL_FAIL( "sd::importEffects(), Exception catched!" );
    }

    return xRootNode;
}

#define EXPAND_PROTOCOL "vnd.sun.star.expand:"

void CustomAnimationPresets::importEffects()
{
    try
    {
        // Get service factory
        Reference< XMultiServiceFactory > xServiceFactory( comphelper::getProcessServiceFactory() );
        DBG_ASSERT( xServiceFactory.is(), "sd::CustomAnimationPresets::import(), got no service manager" );
        if( !xServiceFactory.is() )
            return;

        uno::Reference< beans::XPropertySet > xProps( xServiceFactory, UNO_QUERY );
        uno::Reference< uno::XComponentContext > xContext;
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;

        uno::Reference< util::XMacroExpander > xMacroExpander;
        if( xContext.is() )
            xMacroExpander.set( xContext->getValueByName(
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander"))),
                                UNO_QUERY );

        Reference< XMultiServiceFactory > xConfigProvider(
            xServiceFactory->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ))),
            UNO_QUERY_THROW );

        // read path to transition effects files from config
        Any propValue = uno::makeAny(
            beans::PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" )), -1,
                uno::makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Impress/Misc") )),
                beans::PropertyState_DIRECT_VALUE ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")),
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );
        uno::Sequence< rtl::OUString > aFiles;
        xNameAccess->getByName(
            OUString( RTL_CONSTASCII_USTRINGPARAM("EffectFiles"))) >>= aFiles;

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

                    aIter++;
                }
            }
        }
    }
    catch( xml::sax::SAXParseException& r )
    {
        (void)r;
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), SAXParseException catched!" );
    }
    catch( xml::sax::SAXException& r )
    {
        (void)r;
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), SAXException catched!" );
    }
    catch( io::IOException& r )
    {
        (void)r;
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), IOException catched!" );
    }
    catch( Exception& r )
    {
        (void)r;
        OSL_FAIL( "sd::CustomAnimationPresets::importEffects(), Exception catched!" );
    }
}

void CustomAnimationPresets::importResources()
{
    try
    {
        // Get service factory
        Reference< XMultiServiceFactory > xServiceFactory( comphelper::getProcessServiceFactory() );
        DBG_ASSERT( xServiceFactory.is(), "sd::CustomAnimationPresets::import(), got no service manager" );
        if( !xServiceFactory.is() )
            return;

        Reference< XMultiServiceFactory > xConfigProvider(
            xServiceFactory->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ))),
            UNO_QUERY );

        const OUString aPropertyPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/UserInterface/Properties" ) );
        implImportLabels( xConfigProvider, aPropertyPath, maPropertyNameMap );

        const OUString aEffectsPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/UserInterface/Effects" ) );
        implImportLabels( xConfigProvider, aEffectsPath, maEffectNameMap );

        importEffects();

        const OUString aEntrancePath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/Presets/Entrance" ) );
        importPresets( xConfigProvider, aEntrancePath, maEntrancePresets );

        const OUString aEmphasisPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/Presets/Emphasis" ) );
        importPresets( xConfigProvider, aEmphasisPath, maEmphasisPresets );

        const OUString aExitPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/Presets/Exit" ) );
        importPresets( xConfigProvider, aExitPath, maExitPresets );

        const OUString aMotionPathsPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/Presets/MotionPaths" ) );
        importPresets( xConfigProvider, aMotionPathsPath, maMotionPathsPresets );

        const OUString aMiscPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/Presets/Misc" ) );
        importPresets( xConfigProvider, aMiscPath, maMiscPresets );
    }
    catch( lang::WrappedTargetException& e )
    {
        (void)e;
        OSL_FAIL( "sd::CustomAnimationPresets::importResources(), WrappedTargetException catched!" );
    }
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL( "sd::CustomAnimationPresets::importResources(), Exception catched!" );
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
            const OUString aEffectsName( RTL_CONSTASCII_USTRINGPARAM( "Effects" ) );
            const OUString aLabelName( RTL_CONSTASCII_USTRINGPARAM( "Label" ) );

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
    catch( Exception& e )
    {
        (void)e;
        OSL_FAIL( "sd::CustomAnimationPresets::importPresets(), Exception catched!" );
    }

#ifdef DEBUG
    if( aMissedPresetIds.Len() )
    {
        ByteString aTmp( "sd::CustomAnimationPresets::importPresets(), invalid preset id!\n" );
        aTmp += ByteString( aMissedPresetIds, RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aTmp.GetBuffer() );
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
        if( pCategory.get() && pCategory->maEffects.size() )
        {
            sal_Int32 nDescriptor = (rand() * pCategory->maEffects.size() / RAND_MAX);
            CustomAnimationPresetPtr pPreset = pCategory->maEffects[nDescriptor];
            if( pPreset.get() )
            {
                UStringList aSubTypes = pPreset->getSubTypes();

                OUString aSubType;
                if( aSubTypes.size() )
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
