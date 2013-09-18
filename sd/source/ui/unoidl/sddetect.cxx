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


#include "sddetect.hxx"

#include <framework/interaction.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <vcl/graphicfilter.hxx>
#include <rtl/ustring.h>
#include <rtl/logfile.hxx>
#include <svl/itemset.hxx>
#include <vcl/window.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <tools/urlobj.hxx>
#include <osl/mutex.hxx>
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
#include <vcl/FilterConfigItem.hxx>
#include <sot/storage.hxx>
#include <unotools/moduleoptions.hxx>

#include "strmname.h"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;

namespace {

bool isZipStorageType(const OUString& rTypeName)
{
    if (rTypeName == "impress8" || rTypeName == "impress8_template")
        return true;

    if (rTypeName == "draw8" || rTypeName == "draw8_template")
        return true;

    if (rTypeName == "impress_StarOffice_XML_Impress" || rTypeName == "impress_StarOffice_XML_Impress_Template")
        return true;

    if (rTypeName == "draw_StarOffice_XML_Draw" || rTypeName == "draw_StarOffice_XML_Draw_Template")
        return true;

    return false;
}

}

SdFilterDetect::SdFilterDetect( const Reference < XMultiServiceFactory >&  )
{
}

SdFilterDetect::~SdFilterDetect()
{
}

OUString SAL_CALL SdFilterDetect::detect( Sequence< beans::PropertyValue >& lDescriptor ) throw( RuntimeException )
{
    Reference< XInputStream > xStream;
    Reference< XContent > xContent;
    Reference< XInteractionHandler > xInteraction;
    OUString aURL;
    OUString sTemp;
    OUString aTypeName;            // a name describing the type (from MediaDescriptor, usually from flat detection)
    OUString aPreselectedFilterName;      // a name describing the filter to use (from MediaDescriptor, usually from UI action)

    OUString aDocumentTitle; // interesting only if set in this method

    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Bool bWasReadOnly = sal_False, bReadOnly = sal_False;

    sal_Bool bRepairPackage = sal_False;
    sal_Bool bRepairAllowed = sal_False;
    bool bDeepDetection = false;

    // now some parameters that can already be in the array, but may be overwritten or new inserted here
    // remember their indices in the case new values must be added to the array
    sal_Int32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
    sal_Int32 nIndexOfDocumentTitle = -1;

    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // extract properties
        if ( lDescriptor[nProperty].Name == "URL" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( aURL.isEmpty() && lDescriptor[nProperty].Name == "FileName" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if ( lDescriptor[nProperty].Name == "TypeName" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aTypeName = sTemp;
        }
        else if ( lDescriptor[nProperty].Name == "FilterName" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aPreselectedFilterName = sTemp;
        }
        else if ( lDescriptor[nProperty].Name == "InputStream" )
            nIndexOfInputStream = nProperty;
        else if ( lDescriptor[nProperty].Name == "ReadOnly" )
            nIndexOfReadOnlyFlag = nProperty;
        else if ( lDescriptor[nProperty].Name == "UCBContent" )
            nIndexOfContent = nProperty;
        else if ( lDescriptor[nProperty].Name == "AsTemplate" )
        {
            lDescriptor[nProperty].Value >>= bOpenAsTemplate;
            nIndexOfTemplateFlag = nProperty;
        }
        else if ( lDescriptor[nProperty].Name == "InteractionHandler" )
            lDescriptor[nProperty].Value >>= xInteraction;
        else if ( lDescriptor[nProperty].Name == "RepairPackage" )
            lDescriptor[nProperty].Value >>= bRepairPackage;
        else if ( lDescriptor[nProperty].Name == "DocumentTitle" )
            nIndexOfDocumentTitle = nProperty;
        else if (lDescriptor[nProperty].Name == "DeepDetection")
            bDeepDetection = lDescriptor[nProperty].Value.get<sal_Bool>();
    }

    // can't check the type for external filters, so set the "dont" flag accordingly
    SolarMutexGuard aGuard;
    //SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;

    SfxApplication* pApp = SFX_APP();
    SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, sal_False );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    OUString aPrefix = OUString("private:factory/");
    if( aURL.startsWith( aPrefix ) )
    {
        if( SvtModuleOptions().IsImpress() )
        {
            OUString aPattern( aPrefix );
            aPattern += "simpress";
            if ( aURL.startsWith( aPattern ) )
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
        }

        if( !pFilter && SvtModuleOptions().IsDraw() )
        {
            OUString aPattern( aPrefix );
            aPattern += "sdraw";
            if ( aURL.startsWith( aPattern ) )
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
        }
    }
    else
    {
        // ctor of SfxMedium uses owner transition of ItemSet
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, NULL, pSet );
        aMedium.UseInteractionHandler( sal_True );
        if ( !aPreselectedFilterName.isEmpty() )
            pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
        else if (!aTypeName.isEmpty())
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
            bool bIsZipStorage = aMedium.IsStorage();

            if (aMedium.GetError() == SVSTREAM_OK)
            {
                if (bIsZipStorage)
                {
                    // PowerPoint needs to be detected via StreamName, all other storage based formats are our own and can
                    // be detected by the ClipboardId, so except for the PPT filter all filters must have a ClipboardId set
                    Reference < embed::XStorage > xStorage = aMedium.GetStorage( sal_False );

                    //TODO/LATER: move error handling to central place! (maybe even complete own filters)
                    if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                    {
                        // error during storage creation means _here_ that the medium
                        // is broken, but we can not handle it in medium since unpossibility
                        // to create a storage does not _always_ means that the medium is broken
                        aMedium.SetError( aMedium.GetLastStorageCreationState(),  OSL_LOG_PREFIX  );
                        if ( xInteraction.is() )
                        {
                            OUString empty;
                            try
                            {
                                InteractiveAppException xException( empty,
                                                                Reference< XInterface >(),
                                                                InteractionClassification_ERROR,
                                                                aMedium.GetError() );

                                Reference< XInteractionRequest > xRequest(
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
                            OUString sFilterName;
                            if ( pFilter )
                                sFilterName = pFilter->GetName();
                            aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pFilter ? pFilter->IsOwnTemplateFormat() : sal_False, &sFilterName );
                        }
                        catch( const WrappedTargetException& aWrap )
                        {
                            if (!bDeepDetection)
                                // Bail out early unless it's a deep detection.
                                return OUString();

                            packages::zip::ZipIOException aZipException;
                            if ((aWrap.TargetException >>= aZipException) && !aTypeName.isEmpty())
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
                                    aTypeName = OUString();
                                    pFilter = 0;
                                }
                            }
                        }
                        catch( RuntimeException& )
                        {
                            throw;
                        }
                        catch( Exception& )
                        {
                            aTypeName = OUString();
                            pFilter = 0;
                        }

                        if (!pFilter && !aTypeName.isEmpty())
                        {
                            //TODO/LATER: using this method impress is always preferred if no flat detecion has been made
                            // this should been discussed!
                            if ( SvtModuleOptions().IsImpress() )
                                pFilter = SfxFilterMatcher( OUString("simpress") ).GetFilter4EA( aTypeName );
                            else if ( SvtModuleOptions().IsDraw() )
                                pFilter = SfxFilterMatcher( OUString("sdraw") ).GetFilter4EA( aTypeName );
                        }
                    }
                }
                else
                {
                    if (isZipStorageType(aTypeName))
                        // This stream is a not zip archive, but a zip archive type is specified.
                        pFilter = NULL;

                    SvStream* pStm = NULL;
                    if (pFilter)
                    {
                        pStm = aMedium.GetInStream();
                        if (!pStm)
                            pFilter = NULL;
                    }

                    if (pFilter && pStm)
                    {
                        SotStorageRef aStorage = new SotStorage ( pStm, sal_False );
                        if ( !aStorage->GetError() )
                        {
                            OUString aStreamName("PowerPoint Document");
                            if ( aStorage->IsStream( aStreamName ) && SvtModuleOptions().IsImpress() )
                            {
                                OUString aFileName(aMedium.GetName());
                                aFileName = aFileName.toAsciiUpperCase();

                                if( aFileName.indexOf( ".POT" ) != -1 )
                                    pFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97Template );
                                else if( aFileName.indexOf( ".PPS" ) != -1 )
                                    pFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97AutoPlay );
                                else
                                    pFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97);
                            }
                        }
                        else
                        {
                            // vector graphic?
                            pStm->Seek( STREAM_SEEK_TO_BEGIN );

                            const OUString      aFileName( aMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
                            GraphicDescriptor   aDesc( *pStm, &aFileName );
                            if( !aDesc.Detect( sal_False ) )
                            {
                                pFilter = 0;
                                if( SvtModuleOptions().IsImpress() )
                                {
                                    INetURLObject aCheckURL( aFileName );
                                    if( aCheckURL.getExtension().equalsIgnoreAsciiCase("cgm") )
                                    {
                                        sal_uInt8 n8;
                                        pStm->Seek( STREAM_SEEK_TO_BEGIN );
                                        *pStm >> n8;
                                        if ( ( n8 & 0xf0 ) == 0 )       // we are supporting binary cgm format only, so
                                        {                               // this is a small test to exclude cgm text
                                            SfxFilterMatcher aMatch(OUString("simpress"));
                                            pFilter = aMatch.GetFilter4FilterName(OUString("CGM - Computer Graphics Metafile"));
                                        }
                                    }
                                }
                            }
                            else
                            {
                                OUString aShortName( aDesc.GetImportFormatShortName( aDesc.GetFileFormat() ) );
                                GraphicFilter &rGrfFilter = GraphicFilter::GetGraphicFilter();
                                const OUString aName( rGrfFilter.GetImportFormatTypeName( rGrfFilter.GetImportFormatNumberForShortName( aShortName ) ) );

                                if ( pFilter && aShortName.equalsIgnoreAsciiCase( "PCD" ) )    // there is a multiple pcd selection possible
                                {
                                    sal_Int32 nBase = 2;    // default Base0
                                    OUString aFilterTypeName( pFilter->GetRealTypeName() );
                                    if ( aFilterTypeName == "pcd_Photo_CD_Base4" )
                                        nBase = 1;
                                    else if ( aFilterTypeName == "pcd_Photo_CD_Base16" )
                                        nBase = 0;
                                    FilterConfigItem aFilterConfigItem( "Office.Common/Filter/Graphic/Import/PCD" );
                                    aFilterConfigItem.WriteInt32( "Resolution" , nBase );
                                }

                                SfxFilterMatcher aMatch(OUString("sdraw"));
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
        lDescriptor[nPropertyCount].Name = "InputStream";
        lDescriptor[nPropertyCount].Value <<= xStream;
        nPropertyCount++;
    }

    if ( nIndexOfContent == -1 && xContent.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = "UCBContent";
        lDescriptor[nPropertyCount].Value <<= xContent;
        nPropertyCount++;
    }

    if ( bReadOnly != bWasReadOnly )
    {
        if ( nIndexOfReadOnlyFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = "ReadOnly";
            lDescriptor[nPropertyCount].Value <<= bReadOnly;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfReadOnlyFlag].Value <<= bReadOnly;
    }

    if ( !bRepairPackage && bRepairAllowed )
    {
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = "RepairPackage";
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
            lDescriptor[nPropertyCount].Name = "AsTemplate";
            lDescriptor[nPropertyCount].Value <<= bOpenAsTemplate;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfTemplateFlag].Value <<= bOpenAsTemplate;
    }

    if ( !aDocumentTitle.isEmpty() )
    {
        // the title was set here
        if ( nIndexOfDocumentTitle == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = "DocumentTitle";
            lDescriptor[nPropertyCount].Value <<= aDocumentTitle;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfDocumentTitle].Value <<= aDocumentTitle;
    }

    if (!pFilter)
        aTypeName = OUString();

    return aTypeName;
}

/* XServiceInfo */
OUString SAL_CALL SdFilterDetect::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SdFilterDetect::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    Sequence< OUString > seqServiceNames = getSupportedServiceNames();
    const OUString*         pArray          = seqServiceNames.getConstArray();
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
Sequence< OUString > SAL_CALL SdFilterDetect::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< OUString > SdFilterDetect::impl_getStaticSupportedServiceNames()
{
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = "com.sun.star.frame.ExtendedTypeDetection"  ;
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
OUString SdFilterDetect::impl_getStaticImplementationName()
{
    return OUString( "com.sun.star.comp.draw.FormatDetector" );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL SdFilterDetect::impl_createInstance( const Reference< XMultiServiceFactory >& xServiceManager ) throw( Exception )
{
    return static_cast< cppu::OWeakObject * >(
        new SdFilterDetect( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
