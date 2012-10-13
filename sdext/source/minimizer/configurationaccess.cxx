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


#include "configurationaccess.hxx"
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <sal/macros.h>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

static const OUString& GetConfigurationProviderServiceName (void)
{
    static const OUString sConfigurationProviderServiceName (
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.ConfigurationProvider"));
    return sConfigurationProviderServiceName;
}
static const OUString& GetPathToConfigurationRoot (void)
{
    static const OUString sPathToConfigurationRoot (
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.extension.SunPresentationMinimizer"));
    return sPathToConfigurationRoot;
}

void OptimizerSettings::LoadSettingsFromConfiguration( const Reference< XNameAccess >& rSettings )
{
    if ( rSettings.is() )
    {
        const Sequence< OUString > aElements( rSettings->getElementNames() );
        for ( int i = 0; i < aElements.getLength(); i++ )
        {
            try
            {
                const OUString aPropertyName( aElements[ i ] );
                Any aValue( rSettings->getByName( aPropertyName ) );
                switch( TKGet( aPropertyName ) )
                {
                    case TK_Name :                      aValue >>= maName; break;
                    case TK_JPEGCompression :           aValue >>= mbJPEGCompression; break;
                    case TK_JPEGQuality :               aValue >>= mnJPEGQuality; break;
                    case TK_RemoveCropArea :            aValue >>= mbRemoveCropArea; break;
                    case TK_ImageResolution :           aValue >>= mnImageResolution; break;
                    case TK_EmbedLinkedGraphics :       aValue >>= mbEmbedLinkedGraphics; break;
                    case TK_OLEOptimization :           aValue >>= mbOLEOptimization; break;
                    case TK_OLEOptimizationType :       aValue >>= mnOLEOptimizationType; break;
                    case TK_DeleteUnusedMasterPages :   aValue >>= mbDeleteUnusedMasterPages; break;
                    case TK_DeleteHiddenSlides :        aValue >>= mbDeleteHiddenSlides; break;
                    case TK_DeleteNotesPages :          aValue >>= mbDeleteNotesPages ;break;
                    case TK_SaveAs :                    aValue >>= mbSaveAs; break;
//                  case TK_SaveAsURL :                 aValue >>= maSaveAsURL; break;      // URL is not saved to configuration
//                  case TK_FilterName :                aValue >>= maFilterName; break;     // URL is not saved to configuration
                    case TK_OpenNewDocument :           aValue >>= mbOpenNewDocument; break;
                    default: break;
                }
            }
            catch (const Exception&)
            {
            }
        }
    }
}

void OptimizerSettings::SaveSettingsToConfiguration( const Reference< XNameReplace >& rSettings )
{
    if ( rSettings.is() )
    {
        OUString pNames[] = {
            TKGet( TK_Name ),
            TKGet( TK_JPEGCompression ),
            TKGet( TK_JPEGQuality ),
            TKGet( TK_RemoveCropArea ),
            TKGet( TK_ImageResolution ),
            TKGet( TK_EmbedLinkedGraphics ),
            TKGet( TK_OLEOptimization ),
            TKGet( TK_OLEOptimizationType ),
            TKGet( TK_DeleteUnusedMasterPages ),
            TKGet( TK_DeleteHiddenSlides ),
            TKGet( TK_DeleteNotesPages ),
            TKGet( TK_SaveAs ),
//          TKGet( TK_SaveAsURL ),
//          TKGet( TK_FilterName ),
            TKGet( TK_OpenNewDocument ) };

        Any pValues[] = {
            Any( maName ),
            Any( mbJPEGCompression ),
            Any( mnJPEGQuality ),
            Any( mbRemoveCropArea ),
            Any( mnImageResolution ),
            Any( mbEmbedLinkedGraphics ),
            Any( mbOLEOptimization ),
            Any( mnOLEOptimizationType ),
            Any( mbDeleteUnusedMasterPages ),
            Any( mbDeleteHiddenSlides ),
            Any( mbDeleteNotesPages ),
            Any( mbSaveAs ),
//          Any( maSaveAsURL ),
//          Any( maFilterName ),
            Any( mbOpenNewDocument ) };

        sal_Int32 i, nCount = SAL_N_ELEMENTS( pNames );

        for ( i = 0; i < nCount; i++ )
        {
            try
            {
                rSettings->replaceByName( pNames[ i ], pValues[ i ] );
            }
            catch (const Exception&)
            {
            }
        }
    }
}

sal_Bool OptimizerSettings::operator==( const OptimizerSettings& rOptimizerSettings ) const
{
    return ( rOptimizerSettings.mbJPEGCompression == mbJPEGCompression )
        && ( rOptimizerSettings.mnJPEGQuality == mnJPEGQuality )
        && ( rOptimizerSettings.mbRemoveCropArea == mbRemoveCropArea )
        && ( rOptimizerSettings.mnImageResolution == mnImageResolution )
        && ( rOptimizerSettings.mbEmbedLinkedGraphics == mbEmbedLinkedGraphics )
        && ( rOptimizerSettings.mbOLEOptimization == mbOLEOptimization )
        && ( rOptimizerSettings.mnOLEOptimizationType == mnOLEOptimizationType )
        && ( rOptimizerSettings.mbDeleteUnusedMasterPages == mbDeleteUnusedMasterPages )
        && ( rOptimizerSettings.mbDeleteHiddenSlides == mbDeleteHiddenSlides )
        && ( rOptimizerSettings.mbDeleteNotesPages == mbDeleteNotesPages );
//      && ( rOptimizerSettings.mbOpenNewDocument == mbOpenNewDocument );
}


ConfigurationAccess::ConfigurationAccess( const Reference< uno::XComponentContext >& rxMSF, OptimizerSettings* pDefaultSettings ) :
    mxMSF( rxMSF )
{
    LoadStrings();
    maSettings.push_back( pDefaultSettings ?
        *pDefaultSettings : OptimizerSettings() );
    maSettings.back().maName = TKGet( TK_LastUsedSettings );
    LoadConfiguration();
    maInitialSettings = maSettings;
};

ConfigurationAccess::~ConfigurationAccess()
{
}

rtl::OUString ConfigurationAccess::getPath( const PPPOptimizerTokenEnum eToken )
{
    rtl::OUString aPath;
    try
    {
        static const OUString sProtocol( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.expand:" ) );
        static const OUString stheMacroExpander( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander" ) );
        Reference< container::XNameAccess > xSet( OpenConfiguration( true ), UNO_QUERY_THROW );
        if ( xSet->hasByName( TKGet( eToken ) ) )
            xSet->getByName( TKGet( eToken ) ) >>= aPath;
        if ( aPath.match( sProtocol, 0 ) )
        {
            rtl::OUString aTmp( aPath.copy( 20 ) );
            Reference< util::XMacroExpander > xExpander;
            if ( mxMSF->getValueByName( stheMacroExpander ) >>= xExpander )
            {
                aPath = xExpander->expandMacros( aTmp );
            }
        }
    }
    catch (const Exception&)
    {
    }
    return aPath;
}

rtl::OUString ConfigurationAccess::getString( const PPPOptimizerTokenEnum eToken ) const
{
    std::map< PPPOptimizerTokenEnum, rtl::OUString, Compare >::const_iterator aIter( maStrings.find( eToken ) );
    return aIter != maStrings.end() ? ((*aIter).second) : rtl::OUString();
}

void ConfigurationAccess::LoadStrings()
{
    try
    {
        do
        {
            Reference< XInterface > xRoot( OpenConfiguration( true ) );
            if ( !xRoot.is() )
                break;
            Reference< container::XNameAccess > xSet( GetConfigurationNode( xRoot, TKGet( TK_Strings ) ), UNO_QUERY );
            if ( xSet.is() )
            {
                const Sequence< OUString > aElements( xSet->getElementNames() );
                for ( int i = 0; i < aElements.getLength(); i++ )
                {
                    try
                    {
                        OUString aString, aPropertyName( aElements[ i ] );
                        if ( xSet->getByName( aPropertyName ) >>= aString )
                            maStrings[ TKGet( aPropertyName ) ] = aString;
                    }
                    catch (const Exception&)
                    {
                    }
                }
            }
        }
        while( false );
    }
    catch (const Exception&)
    {
    }
}

void ConfigurationAccess::LoadConfiguration()
{
    try
    {
        do
        {
            Reference< XInterface > xRoot( OpenConfiguration( true ) );
            if ( !xRoot.is() )
                break;
            Reference< container::XNameAccess > xSet( GetConfigurationNode( xRoot, TKGet( TK_LastUsedSettings ) ), UNO_QUERY );
            if ( xSet.is() )
            {
                OptimizerSettings& rCurrent( maSettings.front() );
                rCurrent.LoadSettingsFromConfiguration( xSet );
            }
            xSet = Reference< container::XNameAccess >( GetConfigurationNode( xRoot, TKGet( TK_Settings_Templates ) ), UNO_QUERY );
            if ( xSet.is() )
            {
                const Sequence< OUString > aElements( xSet->getElementNames() );
                for ( int i = 0; i < aElements.getLength(); i++ )
                {
                    try
                    {
                        OUString aPath( TKGet( TK_Settings_Templates_ ).concat( aElements[ i ] ) );
                        Reference< container::XNameAccess > xTemplates( GetConfigurationNode( xRoot, aPath ), UNO_QUERY );
                        if ( xTemplates.is() )
                        {
                            maSettings.push_back( OptimizerSettings() );
                            maSettings.back().LoadSettingsFromConfiguration( xTemplates );
                        }
                    }
                    catch (const Exception&)
                    {
                    }
                }
            }
        }
        while( false );
    }
    catch (const Exception&)
    {
    }
}

void ConfigurationAccess::SaveConfiguration()
{
    try
    {
        do
        {
            int i;
            unsigned int k;
            Reference<util::XChangesBatch> xRoot( OpenConfiguration( false ), UNO_QUERY_THROW );

            // storing the last used settings
            Reference< container::XNameReplace > xSet( GetConfigurationNode( xRoot, TKGet( TK_LastUsedSettings ) ), UNO_QUERY_THROW );
            OptimizerSettings& rCurrent( maSettings.front() );
            rCurrent.SaveSettingsToConfiguration( xSet );

            // updating template elements
            xSet = Reference< container::XNameReplace >( GetConfigurationNode( xRoot, TKGet( TK_Settings_Templates ) ), UNO_QUERY_THROW );
            Reference< container::XNameContainer > xNameContainer( xSet, UNO_QUERY_THROW );

            const Sequence< OUString > aElements( xSet->getElementNames() );
            for( i = 0; i < aElements.getLength(); i++ )
                xNameContainer->removeByName( aElements[ i ] );

            for( k = 1; k < maSettings.size(); k++ )
            {
                OptimizerSettings& rSettings( maSettings[ k ] );
                OUString aElementName( TKGet( TK_Template ).concat( OUString::valueOf( static_cast< sal_Int32 >( k ) ) ) );
                Reference< lang::XSingleServiceFactory > xChildFactory ( xSet, UNO_QUERY_THROW );
                Reference< container::XNameReplace > xChild( xChildFactory->createInstance(), UNO_QUERY_THROW );
                xNameContainer->insertByName( aElementName, Any( xChild ) );

                OUString aPath( TKGet( TK_Settings_Templates_ ).concat( aElementName ) );
                Reference< container::XNameReplace > xTemplates( GetConfigurationNode( xRoot, aPath ), UNO_QUERY );
                rSettings.SaveSettingsToConfiguration( xTemplates );
            }
            xRoot->commitChanges();
        }
        while( false );
    }
    catch (const Exception&)
    {
    }
}

Reference< XInterface > ConfigurationAccess::OpenConfiguration( bool bReadOnly )
{
    Reference< XInterface > xRoot;
    try
    {
        Reference< lang::XMultiServiceFactory > xProvider( mxMSF->getServiceManager()->createInstanceWithContext( GetConfigurationProviderServiceName(), mxMSF ), UNO_QUERY );
        if ( xProvider.is() )
        {
            Sequence< Any > aCreationArguments( 2 );
            aCreationArguments[0] = makeAny( PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) ), 0,
                makeAny( GetPathToConfigurationRoot() ),
                PropertyState_DIRECT_VALUE ) );
            aCreationArguments[1] = makeAny(beans::PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "lazywrite" ) ), 0, makeAny( true ),
                PropertyState_DIRECT_VALUE ) );
            OUString sAccessService;
            if ( bReadOnly )
                sAccessService = OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationAccess" ) );
            else
                sAccessService = OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationUpdateAccess" ) );

            xRoot = xProvider->createInstanceWithArguments(
                sAccessService, aCreationArguments );
        }
    }
    catch (const Exception&)
    {
    }
    return xRoot;
}

Reference< XInterface > ConfigurationAccess::GetConfigurationNode(
    const Reference< XInterface >& xRoot,
    const OUString& sPathToNode )
{
    Reference< XInterface > xNode;
    try
    {
        if ( sPathToNode.isEmpty() )
            xNode = xRoot;
        else
        {
            Reference< XHierarchicalNameAccess > xHierarchy( xRoot, UNO_QUERY );
            if ( xHierarchy.is() )
            {
                xHierarchy->getByHierarchicalName( sPathToNode ) >>= xNode;
            }
        }
    }
    catch (const Exception& rException)
    {
        OSL_TRACE ("caught exception while getting configuration node %s: %s",
            ::rtl::OUStringToOString(sPathToNode,
                RTL_TEXTENCODING_UTF8).getStr(),
            ::rtl::OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
        (void)rException;
    }
    return xNode;
}

com::sun::star::uno::Any ConfigurationAccess::GetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken ) const
{
    Any aRetValue;
    const OptimizerSettings& rSettings( maSettings.front() );
    try
    {
        switch( ePropertyToken )
        {
            case TK_Name :                      aRetValue <<= rSettings.maName; break;
            case TK_JPEGCompression :           aRetValue <<= rSettings.mbJPEGCompression; break;
            case TK_JPEGQuality :               aRetValue <<= rSettings.mnJPEGQuality; break;
            case TK_RemoveCropArea :            aRetValue <<= rSettings.mbRemoveCropArea; break;
            case TK_ImageResolution :           aRetValue <<= rSettings.mnImageResolution; break;
            case TK_EmbedLinkedGraphics :       aRetValue <<= rSettings.mbEmbedLinkedGraphics; break;
            case TK_OLEOptimization :           aRetValue <<= rSettings.mbOLEOptimization; break;
            case TK_OLEOptimizationType :       aRetValue <<= rSettings.mnOLEOptimizationType; break;
            case TK_DeleteUnusedMasterPages :   aRetValue <<= rSettings.mbDeleteUnusedMasterPages; break;
            case TK_DeleteHiddenSlides :        aRetValue <<= rSettings.mbDeleteHiddenSlides; break;
            case TK_DeleteNotesPages :          aRetValue <<= rSettings.mbDeleteNotesPages; break;
            case TK_SaveAs :                    aRetValue <<= rSettings.mbSaveAs; break;
            case TK_SaveAsURL :                 aRetValue <<= rSettings.maSaveAsURL; break;
            case TK_FilterName :                aRetValue <<= rSettings.maFilterName; break;
            case TK_OpenNewDocument :           aRetValue <<= rSettings.mbOpenNewDocument; break;
            case TK_EstimatedFileSize :         aRetValue <<= rSettings.mnEstimatedFileSize; break;
            default:
                break;
        }
    }
    catch (const Exception&)
    {
    }
    return aRetValue;
}

void ConfigurationAccess::SetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken, const com::sun::star::uno::Any& rValue )
{
    OptimizerSettings& rSettings( maSettings.front() );
    try
    {
        switch( ePropertyToken )
        {
            case TK_Name :                      rValue >>= rSettings.maName; break;
            case TK_JPEGCompression :           rValue >>= rSettings.mbJPEGCompression; break;
            case TK_JPEGQuality :               rValue >>= rSettings.mnJPEGQuality; break;
            case TK_RemoveCropArea :            rValue >>= rSettings.mbRemoveCropArea; break;
            case TK_ImageResolution :           rValue >>= rSettings.mnImageResolution; break;
            case TK_EmbedLinkedGraphics :       rValue >>= rSettings.mbEmbedLinkedGraphics; break;
            case TK_OLEOptimization :           rValue >>= rSettings.mbOLEOptimization; break;
            case TK_OLEOptimizationType :       rValue >>= rSettings.mnOLEOptimizationType; break;
            case TK_DeleteUnusedMasterPages :   rValue >>= rSettings.mbDeleteUnusedMasterPages; break;
            case TK_DeleteHiddenSlides :        rValue >>= rSettings.mbDeleteHiddenSlides; break;
            case TK_DeleteNotesPages :          rValue >>= rSettings.mbDeleteNotesPages; break;
            case TK_CustomShowName :            rValue >>= rSettings.maCustomShowName; break;
            case TK_SaveAs :                    rValue >>= rSettings.mbSaveAs; break;
            case TK_SaveAsURL :                 rValue >>= rSettings.maSaveAsURL; break;
            case TK_FilterName :                rValue >>= rSettings.maFilterName; break;
            case TK_OpenNewDocument :           rValue >>= rSettings.mbOpenNewDocument; break;
            case TK_EstimatedFileSize :         rValue >>= rSettings.mnEstimatedFileSize; break;
            default:
                break;
        }
    }
    catch (const Exception&)
    {
    }
}

sal_Bool ConfigurationAccess::GetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken, const sal_Bool bDefault ) const
{
    sal_Bool bRetValue = bDefault;
    if ( ! ( GetConfigProperty( ePropertyToken ) >>= bRetValue ) )
        bRetValue = bDefault;
    return bRetValue;
}

sal_Int16 ConfigurationAccess::GetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken, const sal_Int16 nDefault ) const
{
    sal_Int16 nRetValue = nDefault;
    if ( ! ( GetConfigProperty( ePropertyToken ) >>= nRetValue ) )
        nRetValue = nDefault;
    return nRetValue;
}

sal_Int32 ConfigurationAccess::GetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken, const sal_Int32 nDefault ) const
{
    sal_Int32 nRetValue = nDefault;
    if ( ! ( GetConfigProperty( ePropertyToken ) >>= nRetValue ) )
        nRetValue = nDefault;
    return nRetValue;
}

Sequence< PropertyValue > ConfigurationAccess::GetConfigurationSequence()
{
    Sequence< PropertyValue > aRet( 15 );
    OptimizerSettings& rSettings( maSettings.front() );
    aRet[ 0 ].Name = TKGet( TK_JPEGCompression );
    aRet[ 0 ].Value= Any( rSettings.mbJPEGCompression );
    aRet[ 1 ].Name = TKGet( TK_JPEGQuality );
    aRet[ 1 ].Value= Any( rSettings.mnJPEGQuality );
    aRet[ 2 ].Name = TKGet( TK_RemoveCropArea );
    aRet[ 2 ].Value= Any( rSettings.mbRemoveCropArea );
    aRet[ 3 ].Name = TKGet( TK_ImageResolution );
    aRet[ 3 ].Value= Any( rSettings.mnImageResolution );
    aRet[ 4 ].Name = TKGet( TK_EmbedLinkedGraphics );
    aRet[ 4 ].Value= Any( rSettings.mbEmbedLinkedGraphics );
    aRet[ 5 ].Name = TKGet( TK_OLEOptimization );
    aRet[ 5 ].Value= Any( rSettings.mbOLEOptimization );
    aRet[ 6 ].Name = TKGet( TK_OLEOptimizationType );
    aRet[ 6 ].Value= Any( rSettings.mnOLEOptimizationType );
    aRet[ 7 ].Name = TKGet( TK_DeleteUnusedMasterPages );
    aRet[ 7 ].Value= Any( rSettings.mbDeleteUnusedMasterPages );
    aRet[ 8 ].Name = TKGet( TK_DeleteHiddenSlides );
    aRet[ 8 ].Value= Any( rSettings.mbDeleteHiddenSlides );
    aRet[ 9 ].Name = TKGet( TK_DeleteNotesPages );
    aRet[ 9 ].Value= Any( rSettings.mbDeleteNotesPages );
    aRet[ 10].Name = TKGet( TK_CustomShowName );
    aRet[ 10].Value= Any( rSettings.maCustomShowName );
    aRet[ 11].Name = TKGet( TK_SaveAsURL );
    aRet[ 11].Value= Any( rSettings.maSaveAsURL );
    aRet[ 12].Name = TKGet( TK_FilterName );
    aRet[ 12].Value= Any( rSettings.maFilterName );
    aRet[ 13].Name = TKGet( TK_OpenNewDocument );
    aRet[ 13].Value= Any( rSettings.mbOpenNewDocument );
    aRet[ 14].Name = TKGet( TK_EstimatedFileSize );
    aRet[ 14].Value= Any( rSettings.mnEstimatedFileSize );
    return aRet;
}

std::vector< OptimizerSettings >::iterator ConfigurationAccess::GetOptimizerSettingsByName( const rtl::OUString& rName )
{
    std::vector< OptimizerSettings >::iterator aIter( maSettings.begin() + 1 );
    while ( aIter != maSettings.end() )
    {
        if ( aIter->maName == rName )
            break;
        aIter++;
    }
    return aIter;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
