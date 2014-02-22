/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "swdetect.hxx"

#include <framework/interaction.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <rtl/ustring.h>
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
#include <unotools/moduleoptions.hxx>
#include <comphelper/ihwrapnofilter.hxx>
#include <iodetect.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;

SwFilterDetect::SwFilterDetect( const Reference < XMultiServiceFactory >& /*xFactory*/ )
{
}

SwFilterDetect::~SwFilterDetect()
{
}

OUString SAL_CALL SwFilterDetect::detect( Sequence< PropertyValue >& lDescriptor ) throw( RuntimeException )
{
    Reference< XInputStream > xStream;
    Reference< XContent > xContent;
    Reference< XInteractionHandler > xInteraction;
    OUString aURL;
    OUString sTemp;
    OUString aTypeName;            
    OUString aPreselectedFilterName;      

    OUString aDocumentTitle; 

    
    
    
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Bool bWasReadOnly = sal_False, bReadOnly = sal_False;

    sal_Bool bRepairPackage = sal_False;
    sal_Bool bRepairAllowed = sal_False;
    bool bDeepDetection = false;

    
    
    sal_Int32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
    sal_Int32 nIndexOfDocumentTitle = -1;
    sal_Int32 nIndexOfInteractionHandler = -1;

    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        
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
        {
            lDescriptor[nProperty].Value >>= xInteraction;
            nIndexOfInteractionHandler = nProperty;
        }
        else if ( lDescriptor[nProperty].Name == "RepairPackage" )
            lDescriptor[nProperty].Value >>= bRepairPackage;
        else if ( lDescriptor[nProperty].Name == "DocumentTitle" )
            nIndexOfDocumentTitle = nProperty;
        else if (lDescriptor[nProperty].Name == "DeepDetection")
            bDeepDetection = lDescriptor[nProperty].Value.get<sal_Bool>();
    }

    SolarMutexGuard aGuard;

    SfxApplication* pApp = SFX_APP();
    SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, false );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    OUString aPrefix = "private:factory/";
    if( aURL.startsWith( aPrefix ) )
    {
        if( SvtModuleOptions().IsWriter() )
        {
            OUString aPattern = aPrefix + "swriter";
            if ( aURL.startsWith( aPattern ) )
                return aTypeName;
        }
    }
    else
    {
        
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, NULL, pSet );
        aMedium.UseInteractionHandler( sal_True );
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            
            
            xStream = aMedium.GetInputStream();
            xContent = aMedium.GetContent();
            bReadOnly = aMedium.IsReadOnly();

            sal_Bool bIsStorage = aMedium.IsStorage();
            if ( bIsStorage )
            {
                Reference< embed::XStorage > xStorage = aMedium.GetStorage( sal_False );
                if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                {
                    
                    
                    
                    aMedium.SetError( aMedium.GetLastStorageCreationState(), OUString( OSL_LOG_PREFIX  ) );
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
                        catch (const Exception&)
                        {
                        }
                    }
                }
                else
                {
                    OSL_ENSURE( xStorage.is(), "At this point storage must exist!" );

                    try
                    {
                        const SfxFilter* pPreFilter = !aPreselectedFilterName.isEmpty() ?
                                SfxFilterMatcher().GetFilter4FilterName( aPreselectedFilterName ) : !aTypeName.isEmpty() ?
                                SfxFilterMatcher(OUString("swriter")).GetFilter4EA( aTypeName ) : 0;
                        if (!pPreFilter)
                            pPreFilter = SfxFilterMatcher(OUString("sweb")).GetFilter4EA( aTypeName );
                        OUString aFilterName;
                        if ( pPreFilter )
                        {
                            aFilterName = pPreFilter->GetName();
                            aTypeName = pPreFilter->GetTypeName();
                        }

                        aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pPreFilter ? pPreFilter->IsOwnTemplateFormat() : sal_False, &aFilterName );
                    }
                    catch (const WrappedTargetException& aWrap)
                    {
                        if (!bDeepDetection)
                            
                            return OUString();

                        packages::zip::ZipIOException aZipException;

                        
                        
                        
                        if ( ( aWrap.TargetException >>= aZipException ) && ( !aTypeName.isEmpty() || !aPreselectedFilterName.isEmpty() ) )
                        {
                            if ( xInteraction.is() )
                            {
                                
                                   aDocumentTitle = aMedium.GetURLObject().getName(
                                                            INetURLObject::LAST_SEGMENT,
                                                            true,
                                                            INetURLObject::DECODE_WITH_CHARSET );

                                if ( !bRepairPackage )
                                {
                                    
                                    RequestPackageReparation aRequest( aDocumentTitle );
                                    xInteraction->handle( aRequest.GetRequest() );
                                    bRepairAllowed = aRequest.isApproved();
                                }

                                if ( !bRepairAllowed )
                                {
                                    
                                    
                                    NotifyBrokenPackage aNotifyRequest( aDocumentTitle );
                                    xInteraction->handle( aNotifyRequest.GetRequest() );

                                    Reference< ::comphelper::OIHWrapNoFilterDialog > xHandler = new ::comphelper::OIHWrapNoFilterDialog( xInteraction );
                                    if ( nIndexOfInteractionHandler != -1 )
                                        lDescriptor[nIndexOfInteractionHandler].Value <<= Reference< XInteractionHandler >( static_cast< XInteractionHandler* >( xHandler.get() ) );

                                    aMedium.SetError( ERRCODE_ABORT, OUString( OSL_LOG_PREFIX  ) );
                                }
                            }
                            else
                                
                                aMedium.SetError( ERRCODE_IO_BROKENPACKAGE, OUString( OSL_LOG_PREFIX  ) );

                            if ( !bRepairAllowed )
                            {
                                aTypeName = "";
                                aPreselectedFilterName = "";
                            }
                        }
                    }
                    catch (const RuntimeException&)
                    {
                        throw;
                    }
                    catch (const Exception&)
                    {
                        aTypeName = "";
                        aPreselectedFilterName = "";
                    }
                }
            }
            else
            {
                aMedium.GetInStream();
                if ( aMedium.GetErrorCode() == ERRCODE_NONE )
                {
                    if ( !aPreselectedFilterName.isEmpty() )
                        pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
                    else
                        pFilter = SfxFilterMatcher().GetFilter4EA( aTypeName );

                    bool bTestWriter = !pFilter || pFilter->GetServiceName() == "com.sun.star.text.TextDocument" ||
                        pFilter->GetServiceName() == "com.sun.star.text.WebDocument";
                    bool bTestGlobal = !pFilter || pFilter->GetServiceName() == "com.sun.star.text.GlobalDocument";

                    const SfxFilter* pOrigFilter = NULL;
                    if ( !bTestWriter && !bTestGlobal && pFilter )
                    {
                        
                        
                        
                        pOrigFilter = pFilter;
                        pFilter = SfxFilterMatcher().GetFilter4EA( pFilter->GetTypeName() );
                        bTestWriter = true;
                    }

                    sal_uLong nErr = ERRCODE_NONE;
                    if ( pFilter || bTestWriter )
                        nErr = DetectFilter( aMedium, &pFilter );
                    if ( nErr != ERRCODE_NONE )
                        pFilter = NULL;
                    else if ( pOrigFilter && pFilter && pFilter->GetTypeName() == pOrigFilter->GetTypeName() )
                        
                        pFilter = pOrigFilter;
                }

                if ( pFilter )
                    aTypeName = pFilter->GetTypeName();
                else
                    aTypeName = "";
            }
        }
    }

    if ( nIndexOfInputStream == -1 && xStream.is() )
    {
        
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = "InputStream";
        lDescriptor[nPropertyCount].Value <<= xStream;
        nPropertyCount++;
    }

    if ( nIndexOfContent == -1 && xContent.is() )
    {
        
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


    return aTypeName;
}

sal_uLong SwFilterDetect::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter )
{
    sal_uLong nRet = ERRCODE_NONE;
    if( *ppFilter )
    {
        
        OUString aPrefFlt = (*ppFilter)->GetUserData();

        
        sal_Bool bDetected = SwIoSystem::IsFileFilter(rMedium, aPrefFlt);
        return bDetected ? nRet : ERRCODE_ABORT;
    }

    
    OUString aPrefFlt;
    const SfxFilter* pTmp = SwIoSystem::GetFileFilter( rMedium.GetPhysicalName(), aPrefFlt, &rMedium );
    if( !pTmp )
        return ERRCODE_ABORT;

    else
    {
        
        SfxFilterContainer aFilterContainer( OUString("swriter/web") );
        if( !pTmp->GetUserData().equals(sHTML) ||
            pTmp->GetServiceName() == "com.sun.star.text.WebDocument" ||
            0 == ( (*ppFilter) = SwIoSystem::GetFilterOfFormat( OUString(sHTML),
                    &aFilterContainer ) ) )
            *ppFilter = pTmp;
    }

    return nRet;
}

/* XServiceInfo */
OUString SAL_CALL SwFilterDetect::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SwFilterDetect::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SwFilterDetect::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< OUString > SwFilterDetect::impl_getStaticSupportedServiceNames()
{
    Sequence< OUString > seqServiceNames( 3 );
    seqServiceNames.getArray() [0] = "com.sun.star.frame.ExtendedTypeDetection";
    seqServiceNames.getArray() [1] = "com.sun.star.text.FormatDetector";
    seqServiceNames.getArray() [2] = "com.sun.star.text.W4WFormatDetector";
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
OUString SwFilterDetect::impl_getStaticImplementationName()
{
    return OUString("com.sun.star.comp.writer.FormatDetector" );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL SwFilterDetect::impl_createInstance( const Reference< XMultiServiceFactory >& xServiceManager ) throw( Exception )
{
    return Reference< XInterface >( *new SwFilterDetect( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
