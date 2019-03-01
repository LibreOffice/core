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
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/propertysequence.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

static OUString GetPathToConfigurationRoot()
{
    return OUString("org.openoffice.Office.PresentationMinimizer");
}

void OptimizerSettings::LoadSettingsFromConfiguration( const Reference< XNameAccess >& rSettings )
{
    if ( !rSettings.is() )
        return;

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

void OptimizerSettings::SaveSettingsToConfiguration( const Reference< XNameReplace >& rSettings )
{
    if ( !rSettings.is() )
        return;

    OUString pNames[] = {
        OUString("Name"),
        OUString("JPEGCompression"),
        OUString("JPEGQuality"),
        OUString("RemoveCropArea"),
        OUString("ImageResolution"),
        OUString("EmbedLinkedGraphics"),
        OUString("OLEOptimization"),
        OUString("OLEOptimizationType"),
        OUString("DeleteUnusedMasterPages"),
        OUString("DeleteHiddenSlides"),
        OUString("DeleteNotesPages"),
        OUString("SaveAs"),
//          OUString("SaveAsURL"),
//          OUString("FilterName"),
        OUString("OpenNewDocument") };

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

    for ( int i = 0; i < int(SAL_N_ELEMENTS( pNames )); i++ )
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

bool OptimizerSettings::operator==( const OptimizerSettings& rOptimizerSettings ) const
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


ConfigurationAccess::ConfigurationAccess( const Reference< uno::XComponentContext >& rxContext ) :
    mxContext( rxContext )
{
    LoadStrings();
    maSettings.emplace_back( );
    maSettings.back().maName = "LastUsedSettings";
    LoadConfiguration();
};

ConfigurationAccess::~ConfigurationAccess()
{
}

OUString ConfigurationAccess::getString( const PPPOptimizerTokenEnum eToken ) const
{
    std::map< PPPOptimizerTokenEnum, OUString >::const_iterator aIter( maStrings.find( eToken ) );
    return aIter != maStrings.end() ? ((*aIter).second) : OUString();
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
            Reference< container::XNameAccess > xSet( GetConfigurationNode( xRoot, "Strings" ), UNO_QUERY );
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
            Reference< container::XNameAccess > xSet( GetConfigurationNode( xRoot, "LastUsedSettings" ), UNO_QUERY );
            if ( xSet.is() )
            {
                OptimizerSettings& rCurrent( maSettings.front() );
                rCurrent.LoadSettingsFromConfiguration( xSet );
            }
            xSet.set( GetConfigurationNode( xRoot, "Settings/Templates" ), UNO_QUERY );
            if ( xSet.is() )
            {
                const Sequence< OUString > aElements( xSet->getElementNames() );
                for ( int i = 0; i < aElements.getLength(); i++ )
                {
                    try
                    {
                        OUString aPath( "Settings/Templates/" + aElements[ i ] );
                        Reference< container::XNameAccess > xTemplates( GetConfigurationNode( xRoot, aPath ), UNO_QUERY );
                        if ( xTemplates.is() )
                        {
                            maSettings.emplace_back( );
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
            Reference<util::XChangesBatch> xRoot( OpenConfiguration( false ), UNO_QUERY_THROW );

            // storing the last used settings
            Reference< container::XNameReplace > xSet( GetConfigurationNode( xRoot, "LastUsedSettings" ), UNO_QUERY_THROW );
            OptimizerSettings& rCurrent( maSettings.front() );
            rCurrent.SaveSettingsToConfiguration( xSet );

            // updating template elements
            xSet.set( GetConfigurationNode( xRoot, "Settings/Templates" ), UNO_QUERY_THROW );
            Reference< container::XNameContainer > xNameContainer( xSet, UNO_QUERY_THROW );

            const Sequence< OUString > aElements( xSet->getElementNames() );
            for( i = 0; i < aElements.getLength(); i++ )
                xNameContainer->removeByName( aElements[ i ] );

            for( std::vector<OptimizerSettings>::size_type k = 1; k < maSettings.size(); k++ )
            {
                OptimizerSettings& rSettings( maSettings[ k ] );
                OUString aElementName( "Template" + OUString::number( k ) );
                Reference< lang::XSingleServiceFactory > xChildFactory ( xSet, UNO_QUERY_THROW );
                Reference< container::XNameReplace > xChild( xChildFactory->createInstance(), UNO_QUERY_THROW );
                xNameContainer->insertByName( aElementName, Any( xChild ) );

                OUString aPath( "Settings/Templates/" + aElementName );
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
        Reference< lang::XMultiServiceFactory > xProvider = configuration::theDefaultProvider::get( mxContext );
        uno::Sequence<uno::Any> aCreationArguments(comphelper::InitAnyPropertySequence(
        {
            {"nodepath",  uno::Any(GetPathToConfigurationRoot())}
        }));
        OUString sAccessService;
        if ( bReadOnly )
            sAccessService = "com.sun.star.configuration.ConfigurationAccess";
        else
            sAccessService =
                "com.sun.star.configuration.ConfigurationUpdateAccess";

        xRoot = xProvider->createInstanceWithArguments(
            sAccessService, aCreationArguments );
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
    catch (const Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sdext.minimizer", "caught exception while getting configuration node "
                  << sPathToNode << " : " << exceptionToString(ex));
    }
    return xNode;
}

css::uno::Any ConfigurationAccess::GetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken ) const
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

void ConfigurationAccess::SetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken, const css::uno::Any& rValue )
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

bool ConfigurationAccess::GetConfigProperty( const PPPOptimizerTokenEnum ePropertyToken, const bool bDefault ) const
{
    bool bRetValue = bDefault;
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
    aRet[ 0 ].Name  = "JPEGCompression";
    aRet[ 0 ].Value <<= rSettings.mbJPEGCompression;
    aRet[ 1 ].Name  = "JPEGQuality";
    aRet[ 1 ].Value <<= rSettings.mnJPEGQuality;
    aRet[ 2 ].Name  = "RemoveCropArea";
    aRet[ 2 ].Value <<= rSettings.mbRemoveCropArea;
    aRet[ 3 ].Name  = "ImageResolution";
    aRet[ 3 ].Value <<= rSettings.mnImageResolution;
    aRet[ 4 ].Name  = "EmbedLinkedGraphics";
    aRet[ 4 ].Value <<= rSettings.mbEmbedLinkedGraphics;
    aRet[ 5 ].Name  = "OLEOptimization";
    aRet[ 5 ].Value <<= rSettings.mbOLEOptimization;
    aRet[ 6 ].Name  = "OLEOptimizationType";
    aRet[ 6 ].Value <<= rSettings.mnOLEOptimizationType;
    aRet[ 7 ].Name  = "DeleteUnusedMasterPages";
    aRet[ 7 ].Value <<= rSettings.mbDeleteUnusedMasterPages;
    aRet[ 8 ].Name  = "DeleteHiddenSlides";
    aRet[ 8 ].Value <<= rSettings.mbDeleteHiddenSlides;
    aRet[ 9 ].Name  = "DeleteNotesPages";
    aRet[ 9 ].Value <<= rSettings.mbDeleteNotesPages;
    aRet[ 10].Name  = "CustomShowName";
    aRet[ 10].Value <<= rSettings.maCustomShowName;
    aRet[ 11].Name  = "SaveAsURL";
    aRet[ 11].Value <<= rSettings.maSaveAsURL;
    aRet[ 12].Name  = "FilterName";
    aRet[ 12].Value <<= rSettings.maFilterName;
    aRet[ 13].Name  = "OpenNewDocument";
    aRet[ 13].Value <<= rSettings.mbOpenNewDocument;
    aRet[ 14].Name  = "EstimatedFileSize";
    aRet[ 14].Value <<= rSettings.mnEstimatedFileSize;
    return aRet;
}

std::vector< OptimizerSettings >::iterator ConfigurationAccess::GetOptimizerSettingsByName( const OUString& rName )
{
    return std::find_if(maSettings.begin() + 1, maSettings.end(),
        [&rName](const OptimizerSettings& rSettings) { return rSettings.maName == rName; });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
