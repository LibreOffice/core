/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "configurationaccess.hxx"
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/resource/XResourceBundleLoader.hpp>

#include <rtl/ustrbuf.hxx>

#include "minimizer.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

using rtl::OUString;
using rtl::OUStringBuffer;


#include <rtl/instance.hxx>

typedef std::map < sal_Int32, rtl::OUString > StringResourceMap;

struct StaticResourceMap
    : public rtl::StaticWithInit< StringResourceMap, StaticResourceMap  >
{
    StringResourceMap &
    operator()() const;
};

StringResourceMap &
StaticResourceMap::operator()() const
{
    static StringResourceMap aMap;
    return aMap;
}

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
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Impress/PresentationMinimizer"));
    return sPathToConfigurationRoot;
}

static OUString lcl_loadString(
    const Reference< resource::XResourceBundle > xResourceBundle,
    sal_Int32 nResourceId )
{
    OUString sString;
    OUStringBuffer sKey;
    sKey.appendAscii( RTL_CONSTASCII_STRINGPARAM( "string:" ) );
    sKey.append( nResourceId );

    try
    {
        OSL_VERIFY( xResourceBundle->getByName( sKey.makeStringAndClear() ) >>= sString );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "OptimizerSettings: missing resource!" );
    }

    return sString;
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
            catch( Exception& )
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

        sal_Int32 i, nCount = sizeof( pNames ) / sizeof( OUString );

        for ( i = 0; i < nCount; i++ )
        {
            try
            {
                rSettings->replaceByName( pNames[ i ], pValues[ i ] );
            }
            catch( Exception& /* rException */ )
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


ConfigurationAccess::ConfigurationAccess( const Reference< uno::XComponentContext >& rxContext, OptimizerSettings* pDefaultSettings ) :
    m_xContext( rxContext )
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

rtl::OUString ConfigurationAccess::getString( sal_Int32 nResId )
{
    const StringResourceMap &aStrings = StaticResourceMap::get();
    StringResourceMap::const_iterator aIter( aStrings.find( nResId ) );
    return aIter != aStrings.end() ? ((*aIter).second) : rtl::OUString();
}

void ConfigurationAccess::LoadStrings()
{
    static bool bLoaded = false;
    if ( bLoaded )
        return;
    else
        bLoaded = true;
    try
    {

        Reference< resource::XResourceBundleLoader > xResourceBundleLoader(
            m_xContext->getValueByName(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/com.sun.star.resource.OfficeResourceLoader" ) ) ),
                        UNO_QUERY_THROW );

        Reference< resource::XResourceBundle > xResourceBundle(
            xResourceBundleLoader->loadBundle_Default(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "minimizer" ) ) ),
                    UNO_SET_THROW );

        StringResourceMap &aStrings = StaticResourceMap::get();
        aStrings[ STR_PRESENTATION_MINIMIZER   ] = lcl_loadString( xResourceBundle, STR_PRESENTATION_MINIMIZER );
        aStrings[ STR_STEPS                    ] = lcl_loadString( xResourceBundle, STR_STEPS );
        aStrings[ STR_HELP                     ] = lcl_loadString( xResourceBundle, STR_HELP );
        aStrings[ STR_BACK                     ] = lcl_loadString( xResourceBundle, STR_BACK );
        aStrings[ STR_NEXT                     ] = lcl_loadString( xResourceBundle, STR_NEXT );
        aStrings[ STR_FINISH                   ] = lcl_loadString( xResourceBundle, STR_FINISH );
        aStrings[ STR_CANCEL                   ] = lcl_loadString( xResourceBundle, STR_CANCEL );
        aStrings[ STR_INTRODUCTION             ] = lcl_loadString( xResourceBundle, STR_INTRODUCTION );
        aStrings[ STR_INTRODUCTION_T           ] = lcl_loadString( xResourceBundle, STR_INTRODUCTION_T );
        aStrings[ STR_CHOSE_SETTINGS           ] = lcl_loadString( xResourceBundle, STR_CHOSE_SETTINGS );
        aStrings[ STR_REMOVE                   ] = lcl_loadString( xResourceBundle, STR_REMOVE );
        aStrings[ STR_GRAPHIC_OPTIMIZATION     ] = lcl_loadString( xResourceBundle, STR_GRAPHIC_OPTIMIZATION );
        aStrings[ STR_IMAGE_OPTIMIZATION       ] = lcl_loadString( xResourceBundle, STR_IMAGE_OPTIMIZATION );
        aStrings[ STR_LOSSLESS_COMPRESSION     ] = lcl_loadString( xResourceBundle, STR_LOSSLESS_COMPRESSION );
        aStrings[ STR_JPEG_COMPRESSION         ] = lcl_loadString( xResourceBundle, STR_JPEG_COMPRESSION );
        aStrings[ STR_QUALITY                  ] = lcl_loadString( xResourceBundle, STR_QUALITY );
        aStrings[ STR_REMOVE_CROP_AREA         ] = lcl_loadString( xResourceBundle, STR_REMOVE_CROP_AREA );
        aStrings[ STR_IMAGE_RESOLUTION         ] = lcl_loadString( xResourceBundle, STR_IMAGE_RESOLUTION );
        aStrings[ STR_IMAGE_RESOLUTION_0       ] = lcl_loadString( xResourceBundle, STR_IMAGE_RESOLUTION_0 );
        aStrings[ STR_IMAGE_RESOLUTION_1       ] = lcl_loadString( xResourceBundle, STR_IMAGE_RESOLUTION_1 );
        aStrings[ STR_IMAGE_RESOLUTION_2       ] = lcl_loadString( xResourceBundle, STR_IMAGE_RESOLUTION_2 );
        aStrings[ STR_IMAGE_RESOLUTION_3       ] = lcl_loadString( xResourceBundle, STR_IMAGE_RESOLUTION_3 );
        aStrings[ STR_EMBED_LINKED_GRAPHICS    ] = lcl_loadString( xResourceBundle, STR_EMBED_LINKED_GRAPHICS );
        aStrings[ STR_OLE_OBJECTS              ] = lcl_loadString( xResourceBundle, STR_OLE_OBJECTS );
        aStrings[ STR_OLE_OPTIMIZATION         ] = lcl_loadString( xResourceBundle, STR_OLE_OPTIMIZATION );
        aStrings[ STR_OLE_REPLACE              ] = lcl_loadString( xResourceBundle, STR_OLE_REPLACE );
        aStrings[ STR_ALL_OLE_OBJECTS          ] = lcl_loadString( xResourceBundle, STR_ALL_OLE_OBJECTS );
        aStrings[ STR_ALIEN_OLE_OBJECTS_ONLY   ] = lcl_loadString( xResourceBundle, STR_ALIEN_OLE_OBJECTS_ONLY );
        aStrings[ STR_OLE_OBJECTS_DESC         ] = lcl_loadString( xResourceBundle, STR_OLE_OBJECTS_DESC );
        aStrings[ STR_NO_OLE_OBJECTS_DESC      ] = lcl_loadString( xResourceBundle, STR_NO_OLE_OBJECTS_DESC );
        aStrings[ STR_SLIDES                   ] = lcl_loadString( xResourceBundle, STR_SLIDES );
        aStrings[ STR_CHOOSE_SLIDES            ] = lcl_loadString( xResourceBundle, STR_CHOOSE_SLIDES );
        aStrings[ STR_MASTER_PAGES             ] = lcl_loadString( xResourceBundle, STR_MASTER_PAGES );
        aStrings[ STR_DELETE_MASTER_PAGES      ] = lcl_loadString( xResourceBundle, STR_DELETE_MASTER_PAGES );
        aStrings[ STR_DELETE_NOTES_PAGES       ] = lcl_loadString( xResourceBundle, STR_DELETE_NOTES_PAGES );
        aStrings[ STR_DELETE_HIDDEN_SLIDES     ] = lcl_loadString( xResourceBundle, STR_DELETE_HIDDEN_SLIDES );
        aStrings[ STR_CUSTOM_SHOW              ] = lcl_loadString( xResourceBundle, STR_CUSTOM_SHOW );
        aStrings[ STR_SUMMARY                  ] = lcl_loadString( xResourceBundle, STR_SUMMARY );
        aStrings[ STR_SUMMARY_TITLE            ] = lcl_loadString( xResourceBundle, STR_SUMMARY_TITLE );
        aStrings[ STR_PROGRESS                 ] = lcl_loadString( xResourceBundle, STR_PROGRESS );
        aStrings[ STR_OBJECTS_OPTIMIZED        ] = lcl_loadString( xResourceBundle, STR_OBJECTS_OPTIMIZED );
        aStrings[ STR_APPLY_TO_CURRENT         ] = lcl_loadString( xResourceBundle, STR_APPLY_TO_CURRENT );
        aStrings[ STR_AUTOMATICALLY_OPEN       ] = lcl_loadString( xResourceBundle, STR_AUTOMATICALLY_OPEN );
        aStrings[ STR_SAVE_SETTINGS            ] = lcl_loadString( xResourceBundle, STR_SAVE_SETTINGS );
        aStrings[ STR_SAVE_AS                  ] = lcl_loadString( xResourceBundle, STR_SAVE_AS );
        aStrings[ STR_DELETE_SLIDES            ] = lcl_loadString( xResourceBundle, STR_DELETE_SLIDES );
        aStrings[ STR_OPTIMIZE_IMAGES          ] = lcl_loadString( xResourceBundle, STR_OPTIMIZE_IMAGES );
        aStrings[ STR_CREATE_REPLACEMENT       ] = lcl_loadString( xResourceBundle, STR_CREATE_REPLACEMENT );
        aStrings[ STR_CURRENT_FILESIZE         ] = lcl_loadString( xResourceBundle, STR_CURRENT_FILESIZE );
        aStrings[ STR_ESTIMATED_FILESIZE       ] = lcl_loadString( xResourceBundle, STR_ESTIMATED_FILESIZE );
        aStrings[ STR_MB                       ] = lcl_loadString( xResourceBundle, STR_MB );
        aStrings[ STR_MY_SETTINGS              ] = lcl_loadString( xResourceBundle, STR_MY_SETTINGS );
        aStrings[ STR_DEFAULT_SESSION          ] = lcl_loadString( xResourceBundle, STR_DEFAULT_SESSION );
        aStrings[ STR_MODIFY_WARNING           ] = lcl_loadString( xResourceBundle, STR_MODIFY_WARNING );
        aStrings[ STR_YES                      ] = lcl_loadString( xResourceBundle, STR_YES );
        aStrings[ STR_OK                       ] = lcl_loadString( xResourceBundle, STR_OK );
        aStrings[ STR_INFO_1                   ] = lcl_loadString( xResourceBundle, STR_INFO_1 );
        aStrings[ STR_INFO_2                   ] = lcl_loadString( xResourceBundle, STR_INFO_2 );
        aStrings[ STR_INFO_3                   ] = lcl_loadString( xResourceBundle, STR_INFO_3 );
        aStrings[ STR_INFO_4                   ] = lcl_loadString( xResourceBundle, STR_INFO_4 );
        aStrings[ STR_DUPLICATING_PRESENTATION ] = lcl_loadString( xResourceBundle, STR_DUPLICATING_PRESENTATION );
        aStrings[ STR_DELETING_SLIDES          ] = lcl_loadString( xResourceBundle, STR_DELETING_SLIDES );
        aStrings[ STR_OPTIMIZING_GRAPHICS      ] = lcl_loadString( xResourceBundle, STR_OPTIMIZING_GRAPHICS );
        aStrings[ STR_CREATING_OLE_REPLACEMENTS] = lcl_loadString( xResourceBundle, STR_CREATING_OLE_REPLACEMENTS );
        aStrings[ STR_FILESIZESEPARATOR        ] = lcl_loadString( xResourceBundle, STR_FILESIZESEPARATOR );
        aStrings[ STR_INFO_DIALOG              ] = lcl_loadString( xResourceBundle, STR_INFO_DIALOG );
    }
    catch( Exception& )
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
                    catch( Exception& /* rException */ )
                    {
                    }
                }
            }
        }
        while( false );
    }
    catch( Exception& )
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
    catch( Exception& /* rException */ )
    {

    }
}

Reference< XInterface > ConfigurationAccess::OpenConfiguration( bool bReadOnly )
{
    Reference< XInterface > xRoot;
    try
    {
        Reference< lang::XMultiServiceFactory > xProvider( m_xContext->getServiceManager()->createInstanceWithContext( GetConfigurationProviderServiceName(), m_xContext ), UNO_QUERY );
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
    catch ( Exception& /* rException */ )
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
        if ( !sPathToNode.getLength() )
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
    catch ( Exception& rException )
    {
        OSL_TRACE ("caught exception while getting configuration node %s: %s",
            ::rtl::OUStringToOString(sPathToNode,
                RTL_TEXTENCODING_UTF8).getStr(),
            ::rtl::OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
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
    catch( Exception& /* rException */ )
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
    catch( Exception& /* rException */ )
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
