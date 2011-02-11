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

#include "sddetect.hxx"

#include <framework/interaction.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <framework/interaction.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <svtools/filter.hxx>
#include <rtl/ustring.h>
#include <rtl/logfile.hxx>
#include <svl/itemset.hxx>
#include <vcl/window.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <sot/storinfo.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/brokenpackageint.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <sot/storage.hxx>
#include <unotools/moduleoptions.hxx>
#include <com/sun/star/util/XArchiver.hpp>
#include <comphelper/processfactory.hxx>

#include "strmname.h"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

SdFilterDetect::SdFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >&  )
{
}

SdFilterDetect::~SdFilterDetect()
{
}

::rtl::OUString SAL_CALL SdFilterDetect::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    REFERENCE< XInputStream > xStream;
    REFERENCE< XContent > xContent;
    REFERENCE< XInteractionHandler > xInteraction;
    String aURL;
    ::rtl::OUString sTemp;
    String aTypeName;            // a name describing the type (from MediaDescriptor, usually from flat detection)
    String aPreselectedFilterName;      // a name describing the filter to use (from MediaDescriptor, usually from UI action)

    ::rtl::OUString aDocumentTitle; // interesting only if set in this method

    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Bool bWasReadOnly = sal_False, bReadOnly = sal_False;

    sal_Bool bRepairPackage = sal_False;
    sal_Bool bRepairAllowed = sal_False;

    // now some parameters that can already be in the array, but may be overwritten or new inserted here
    // remember their indices in the case new values must be added to the array
    sal_Int32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfFilterName = -1;
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
    sal_Int32 nIndexOfDocumentTitle = -1;

    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // extract properties
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("URL")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( !aURL.Len() && lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("TypeName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aTypeName = sTemp;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aPreselectedFilterName = sTemp;

            // if the preselected filter name is not correct, it must be erased after detection
            // remember index of property to get access to it later
            nIndexOfFilterName = nProperty;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream")) )
            nIndexOfInputStream = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly")) )
            nIndexOfReadOnlyFlag = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("UCBContent")) )
            nIndexOfContent = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate")) )
        {
            lDescriptor[nProperty].Value >>= bOpenAsTemplate;
            nIndexOfTemplateFlag = nProperty;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("InteractionHandler")) )
            lDescriptor[nProperty].Value >>= xInteraction;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("RepairPackage")) )
            lDescriptor[nProperty].Value >>= bRepairPackage;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentTitle")) )
            nIndexOfDocumentTitle = nProperty;
    }

    // can't check the type for external filters, so set the "dont" flag accordingly
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    //SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;

    SfxApplication* pApp = SFX_APP();
    SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, sal_False );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    String aFilterName;
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        if( SvtModuleOptions().IsImpress() )
        {
            String aPattern( aPrefix );
            aPattern += String::CreateFromAscii("simpress");
            if ( aURL.Match( aPattern ) >= aPattern.Len() )
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
        }

        if( !pFilter && SvtModuleOptions().IsDraw() )
        {
            String aPattern( aPrefix );
            aPattern += String::CreateFromAscii("sdraw");
            if ( aURL.Match( aPattern ) >= aPattern.Len() )
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
        }
    }
    else
    {
        // ctor of SfxMedium uses owner transition of ItemSet
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, sal_False, NULL, pSet );
        aMedium.UseInteractionHandler( sal_True );
        if ( aPreselectedFilterName.Len() )
            pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
        else if( aTypeName.Len() )
        {
            SfxFilterMatcher aMatch;
            pFilter = aMatch.GetFilter4EA( aTypeName );
        }

        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            // remember input stream and content and put them into the descriptor later
            // should be done here since later the medium can switch to a version
            xStream = aMedium.GetInputStream();
            xContent = aMedium.GetContent();
            bReadOnly = aMedium.IsReadOnly();
            sal_Bool bIsStorage = aMedium.IsStorage();

            if (aMedium.GetError() == SVSTREAM_OK)
            {
                if ( bIsStorage )
                {
                    // PowerPoint needs to be detected via StreamName, all other storage based formats are our own and can
                    // be detected by the ClipboardId, so except for the PPT filter all filters must have a ClipboardId set
                    uno::Reference < embed::XStorage > xStorage = aMedium.GetStorage( sal_False );

                    //TODO/LATER: move error handling to central place! (maybe even complete own filters)
                    if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                    {
                        // error during storage creation means _here_ that the medium
                        // is broken, but we can not handle it in medium since unpossibility
                        // to create a storage does not _always_ means that the medium is broken
                        aMedium.SetError( aMedium.GetLastStorageCreationState(), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                        if ( xInteraction.is() )
                        {
                            OUString empty;
                            try
                            {
                                InteractiveAppException xException( empty,
                                                                REFERENCE< XInterface >(),
                                                                InteractionClassification_ERROR,
                                                                aMedium.GetError() );

                                REFERENCE< XInteractionRequest > xRequest(
                                    new ucbhelper::SimpleInteractionRequest( makeAny( xException ),
                                                                          ucbhelper::CONTINUATION_APPROVE ) );
                                xInteraction->handle( xRequest );
                            }
                            catch ( Exception & ) {};
                        }
                    }
                    else
                    {
                        if ( pFilter && !pFilter->GetFormat() )
                            // preselected Filter has no ClipboardId -> doesn't match (see comment above)
                            pFilter = 0;

                        // the storage must be checked even if filter is already found, since it is deep type detection
                        // the storage can be corrupted and it will be detected here
                        try
                        {
                            String sFilterName;
                            if ( pFilter )
                                sFilterName = pFilter->GetName();
                            aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pFilter ? pFilter->IsOwnTemplateFormat() : sal_False, &sFilterName );
                        }
                        catch( lang::WrappedTargetException& aWrap )
                        {
                            packages::zip::ZipIOException aZipException;
                            if ( ( aWrap.TargetException >>= aZipException ) && aTypeName.Len() )
                            {
                                if ( xInteraction.is() )
                                {
                                    // the package is broken one
                                       aDocumentTitle = aMedium.GetURLObject().getName(
                                                                INetURLObject::LAST_SEGMENT,
                                                                true,
                                                                INetURLObject::DECODE_WITH_CHARSET );

                                    if ( !bRepairPackage )
                                    {
                                        // ask the user whether he wants to try to repair
                                        RequestPackageReparation aRequest( aDocumentTitle );
                                        xInteraction->handle( aRequest.GetRequest() );
                                        bRepairAllowed = aRequest.isApproved();
                                    }

                                    if ( !bRepairAllowed )
                                    {
                                        // repair either not allowed or not successful
                                        NotifyBrokenPackage aNotifyRequest( aDocumentTitle );
                                        xInteraction->handle( aNotifyRequest.GetRequest() );
                                    }
                                }

                                if ( !bRepairAllowed )
                                {
                                    aTypeName.Erase();
                                    pFilter = 0;
                                }
                            }
                        }
                        catch( uno::RuntimeException& )
                        {
                            throw;
                        }
                        catch( uno::Exception& )
                        {
                            aTypeName.Erase();
                            pFilter = 0;
                        }

                        if ( !pFilter && aTypeName.Len() )
                        {
                            //TODO/LATER: using this method impress is always preferred if no flat detecion has been made
                            // this should been discussed!
                            if ( SvtModuleOptions().IsImpress() )
                                pFilter = SfxFilterMatcher( String::CreateFromAscii("simpress") ).GetFilter4EA( aTypeName );
                            else if ( SvtModuleOptions().IsDraw() )
                                pFilter = SfxFilterMatcher( String::CreateFromAscii("sdraw") ).GetFilter4EA( aTypeName );
                        }
                    }
                }
                else
                {
                    SvStream* pStm = aMedium.GetInStream();
                    if ( !pStm )
                    {
                        pFilter = 0;
                    }
                    else
                    {
                        SotStorageRef aStorage = new SotStorage ( pStm, sal_False );
                        if ( !aStorage->GetError() )
                        {
                            String aStreamName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PowerPoint Document" ) );
                            if ( aStorage->IsStream( aStreamName ) && SvtModuleOptions().IsImpress() )
                            {
                                String aFileName(aMedium.GetName());
                                aFileName.ToUpperAscii();

                                if( aFileName.SearchAscii( ".POT" ) == STRING_NOTFOUND )
                                    pFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97);
                                else
                                    pFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97Template );
                            }
                        }
                        else
                        {
                            // Vektorgraphik?
                            pStm->Seek( STREAM_SEEK_TO_BEGIN );

                            const String        aFileName( aMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
                            GraphicDescriptor   aDesc( *pStm, &aFileName );
                            GraphicFilter*      pGrfFilter = GraphicFilter::GetGraphicFilter();
                            if( !aDesc.Detect( sal_False ) )
                            {
                                pFilter = 0;
                                if( SvtModuleOptions().IsImpress() )
                                {
                                    INetURLObject aCheckURL( aFileName );
                                    if( aCheckURL.getExtension().equalsIgnoreAsciiCaseAscii( "cgm" ) )
                                    {
                                        sal_uInt8 n8;
                                        pStm->Seek( STREAM_SEEK_TO_BEGIN );
                                        *pStm >> n8;
                                        if ( ( n8 & 0xf0 ) == 0 )       // we are supporting binary cgm format only, so
                                        {                               // this is a small test to exclude cgm text
                                            const String aName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "CGM - Computer Graphics Metafile" ) );
                                            SfxFilterMatcher aMatch( String::CreateFromAscii("simpress") );
                                            pFilter = aMatch.GetFilter4FilterName( aName );
                                        }
                                    }
                                }
                            }
                            else
                            {
                                String aShortName( aDesc.GetImportFormatShortName( aDesc.GetFileFormat() ) );
                                const String aName( pGrfFilter->GetImportFormatTypeName( pGrfFilter->GetImportFormatNumberForShortName( aShortName ) ) );

                                if ( pFilter && aShortName.EqualsIgnoreCaseAscii( "PCD" ) )    // there is a multiple pcd selection possible
                                {
                                    sal_Int32 nBase = 2;    // default Base0
                                    String aFilterTypeName( pFilter->GetRealTypeName() );
                                    if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base4" ) == COMPARE_EQUAL )
                                        nBase = 1;
                                    else if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base16" ) == COMPARE_EQUAL )
                                        nBase = 0;
                                    String aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Import/PCD" ) );
                                    FilterConfigItem aFilterConfigItem( aFilterConfigPath );
                                    aFilterConfigItem.WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ), nBase );
                                }

                                SfxFilterMatcher aMatch( String::CreateFromAscii("sdraw") );
                                pFilter = aMatch.GetFilter4FilterName( aName );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( nIndexOfInputStream == -1 && xStream.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("InputStream");
        lDescriptor[nPropertyCount].Value <<= xStream;
        nPropertyCount++;
    }

    if ( nIndexOfContent == -1 && xContent.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("UCBContent");
        lDescriptor[nPropertyCount].Value <<= xContent;
        nPropertyCount++;
    }

    if ( bReadOnly != bWasReadOnly )
    {
        if ( nIndexOfReadOnlyFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("ReadOnly");
            lDescriptor[nPropertyCount].Value <<= bReadOnly;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfReadOnlyFlag].Value <<= bReadOnly;
    }

    if ( !bRepairPackage && bRepairAllowed )
    {
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("RepairPackage");
        lDescriptor[nPropertyCount].Value <<= bRepairAllowed;
        nPropertyCount++;

        bOpenAsTemplate = sal_True;

        // TODO/LATER: set progress bar that should be used
    }

    if ( bOpenAsTemplate )
    {
        if ( nIndexOfTemplateFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("AsTemplate");
            lDescriptor[nPropertyCount].Value <<= bOpenAsTemplate;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfTemplateFlag].Value <<= bOpenAsTemplate;
    }

    if ( aDocumentTitle.getLength() )
    {
        // the title was set here
        if ( nIndexOfDocumentTitle == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("DocumentTitle");
            lDescriptor[nPropertyCount].Value <<= aDocumentTitle;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfDocumentTitle].Value <<= aDocumentTitle;
    }

    if ( pFilter )
        aTypeName = pFilter->GetTypeName();
    else
        aTypeName.Erase();

    return aTypeName;
}

SFX_IMPL_SINGLEFACTORY( SdFilterDetect )

/* XServiceInfo */
UNOOUSTRING SAL_CALL SdFilterDetect::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SdFilterDetect::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SAL_CALL SdFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SdFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.ExtendedTypeDetection"  );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING SdFilterDetect::impl_getStaticImplementationName()
{
    return UNOOUSTRING::createFromAscii( "com.sun.star.comp.draw.FormatDetector" );
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL SdFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new SdFilterDetect( xServiceManager ) );
}

