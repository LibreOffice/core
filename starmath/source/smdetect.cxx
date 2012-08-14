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


#include "smdetect.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
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

#include "document.hxx"
#include "eqnolefilehdr.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

SmFilterDetect::SmFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& /*xFactory*/ )
{
}

SmFilterDetect::~SmFilterDetect()
{
}

::rtl::OUString SAL_CALL SmFilterDetect::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
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
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
    sal_Int32 nIndexOfDocumentTitle = -1;

    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // extract properties
        if( lDescriptor[nProperty].Name == "URL" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( !aURL.Len() && lDescriptor[nProperty].Name == "FileName" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( lDescriptor[nProperty].Name == "TypeName" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aTypeName = sTemp;
        }
        else if( lDescriptor[nProperty].Name == "FilterName" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aPreselectedFilterName = sTemp;
        }
        else if( lDescriptor[nProperty].Name == "InputStream" )
            nIndexOfInputStream = nProperty;
        else if( lDescriptor[nProperty].Name == "ReadOnly" )
            nIndexOfReadOnlyFlag = nProperty;
        else if( lDescriptor[nProperty].Name == "UCBContent" )
            nIndexOfContent = nProperty;
        else if( lDescriptor[nProperty].Name == "AsTemplate" )
        {
            lDescriptor[nProperty].Value >>= bOpenAsTemplate;
            nIndexOfTemplateFlag = nProperty;
        }
        else if( lDescriptor[nProperty].Name == "InteractionHandler" )
            lDescriptor[nProperty].Value >>= xInteraction;
        else if( lDescriptor[nProperty].Name == "RepairPackage" )
            lDescriptor[nProperty].Value >>= bRepairPackage;
        else if( lDescriptor[nProperty].Name == "DocumentTitle" )
            nIndexOfDocumentTitle = nProperty;
    }

    // can't check the type for external filters, so set the "dont" flag accordingly
    SolarMutexGuard aGuard;

    SfxApplication* pApp = SFX_APP();
    SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, sal_False );

    bWasReadOnly = pItem && pItem->GetValue();

    String aFilterName;
    String aPrefix = rtl::OUString( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        const SfxFilter* pFilter = 0;
        String aPattern( aPrefix );
        aPattern += rtl::OUString("smath");
        if ( aURL.Match( aPattern ) >= aPattern.Len() )
        {
            pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
            aTypeName = pFilter->GetTypeName();
            aFilterName = pFilter->GetName();
        }
    }
    else
    {
        // ctor of SfxMedium uses owner transition of ItemSet
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, NULL, pSet );
        aMedium.UseInteractionHandler( true );

        bool bIsStorage = aMedium.IsStorage();
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            // remember input stream and content and put them into the descriptor later
            // should be done here since later the medium can switch to a version
            xStream = aMedium.GetInputStream();
            xContent = aMedium.GetContent();
            bReadOnly = aMedium.IsReadOnly();

            if ( bIsStorage )
            {
                //TODO/LATER: factor this out!
                uno::Reference < embed::XStorage > xStorage = aMedium.GetStorage( sal_False );
                if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                {
                    // error during storage creation means _here_ that the medium
                    // is broken, but we can not handle it in medium since unpossibility
                    // to create a storage does not _always_ means that the medium is broken
                    aMedium.SetError( aMedium.GetLastStorageCreationState(), OSL_LOG_PREFIX );
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
                    aFilterName.Erase();

                    try
                    {
                        const SfxFilter* pFilter = aPreselectedFilterName.Len() ?
                                SfxFilterMatcher().GetFilter4FilterName( aPreselectedFilterName ) : aTypeName.Len() ?
                                SfxFilterMatcher(rtl::OUString("smath")).GetFilter4EA( aTypeName ) : 0;
                        String aTmpFilterName;
                        if ( pFilter )
                            aTmpFilterName = pFilter->GetName();
                        aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pFilter ? pFilter->IsAllowedAsTemplate() : sal_False, &aTmpFilterName );
                    }
                    catch( const lang::WrappedTargetException& aWrap )
                    {
                        packages::zip::ZipIOException aZipException;

                        // repairing is done only if this type is requested from outside
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
                                aTypeName.Erase();
                        }
                    }
                    catch( uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch( uno::Exception& )
                    {
                        aTypeName.Erase();
                    }

                       if ( aTypeName.Len() )
                    {
                           const SfxFilter* pFilter =
                                    SfxFilterMatcher( rtl::OUString("smath") ).GetFilter4EA( aTypeName );
                        if ( pFilter )
                            aFilterName = pFilter->GetName();
                    }
                }
            }
            else
            {
                //Test to see if this begins with xml and if so run it through
                //the MathML filter. There are all sorts of things wrong with
                //this approach, to be fixed at a better level than here
                SvStream *pStrm = aMedium.GetInStream();
                aTypeName.Erase();
                if (pStrm && !pStrm->GetError())
                {
                    SotStorageRef aStorage = new SotStorage ( pStrm, sal_False );
                    if ( !aStorage->GetError() )
                    {
                        if (aStorage->IsStream(rtl::OUString("Equation Native")))
                        {
                            sal_uInt8 nVersion;
                            if (GetMathTypeVersion( aStorage, nVersion ) && nVersion <=3)
                                aTypeName.AssignAscii( "math_MathType_3x" );
                        }
                    }
                    else
                    {
                        const sal_uInt16 nSize = 5;
                        sal_Char aBuffer[nSize+1];
                        aBuffer[nSize] = 0;
                        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                        sal_uLong nBytesRead = pStrm->Read( aBuffer, nSize );
                        if (nBytesRead == nSize)
                        {
                            if (0 == strncmp( "<?xml",aBuffer,nSize))
                            {
                                // 200 should be enough for the XML
                                // version, encoding and !DOCTYPE
                                // stuff I hope?
                                sal_Char aBuffer2[200];
                                nBytesRead = pStrm->Read( aBuffer2, sizeof(aBuffer2) - 1);
                                aBuffer2[nBytesRead] = 0;
                                if (strstr( aBuffer2, "<math>" ) != NULL ||
                                    strstr( aBuffer2, "<math " ) != NULL)
                                {
                                    static const sal_Char sFltrNm_2[] = MATHML_XML;
                                    static const sal_Char sTypeNm_2[] = "math_MathML_XML_Math";
                                    aFilterName.AssignAscii( sFltrNm_2 );
                                    aTypeName.AssignAscii( sTypeNm_2 );
                                }
                            }
                        }
                    }

                    if ( aTypeName.Len() )
                    {
                        const SfxFilter* pFilt = SfxFilterMatcher( rtl::OUString("smath") ).GetFilter4EA( aTypeName );
                        if ( pFilt )
                            aFilterName = pFilt->GetName();
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

    if ( !aFilterName.Len() )
        aTypeName.Erase();

    return aTypeName;
}

/* XServiceInfo */
rtl::OUString SAL_CALL SmFilterDetect::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SmFilterDetect::supportsService( const rtl::OUString& sServiceName ) throw( UNORUNTIMEEXCEPTION )
{
    UNOSEQUENCE< rtl::OUString > seqServiceNames = getSupportedServiceNames();
    const rtl::OUString*         pArray          = seqServiceNames.getConstArray();
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
UNOSEQUENCE< rtl::OUString > SAL_CALL SmFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< rtl::OUString > SmFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOSEQUENCE< rtl::OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = "com.sun.star.frame.ExtendedTypeDetection";
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
rtl::OUString SmFilterDetect::impl_getStaticImplementationName()
{
    return rtl::OUString("com.sun.star.comp.math.FormatDetector");
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL SmFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new SmFilterDetect( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
