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

SmFilterDetect::SmFilterDetect( const Reference < XMultiServiceFactory >& /*xFactory*/ )
{
}

SmFilterDetect::~SmFilterDetect()
{
}

OUString SAL_CALL SmFilterDetect::detect( Sequence< PropertyValue >& lDescriptor ) throw( RuntimeException )
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
        if( lDescriptor[nProperty].Name == "URL" )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( aURL.isEmpty() && lDescriptor[nProperty].Name == "FileName" )
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
        else if (lDescriptor[nProperty].Name == "DeepDetection")
            bDeepDetection = lDescriptor[nProperty].Value.get<sal_Bool>();
    }

    // can't check the type for external filters, so set the "dont" flag accordingly
    SolarMutexGuard aGuard;

    SfxApplication* pApp = SFX_APP();
    SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, sal_False );

    bWasReadOnly = pItem && pItem->GetValue();

    OUString aFilterName;
    OUString aPrefix( "private:factory/" );
    if( aURL.startsWith( aPrefix ) )
    {
        const SfxFilter* pFilter = 0;
        OUString aPattern( aPrefix );
        aPattern += "smath";
        if ( aURL.startsWith( aPattern ) )
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
                Reference < embed::XStorage > xStorage = aMedium.GetStorage( sal_False );
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
                    aFilterName = OUString();

                    try
                    {
                        const SfxFilter* pFilter = !aPreselectedFilterName.isEmpty() ?
                                SfxFilterMatcher().GetFilter4FilterName( aPreselectedFilterName ) : !aTypeName.isEmpty() ?
                                SfxFilterMatcher( "smath" ).GetFilter4EA( aTypeName ) : 0;
                        OUString aTmpFilterName;
                        if ( pFilter )
                            aTmpFilterName = pFilter->GetName();
                        aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pFilter ? pFilter->IsAllowedAsTemplate() : sal_False, &aTmpFilterName );
                    }
                    catch( const WrappedTargetException& aWrap )
                    {
                        if (!bDeepDetection)
                            // Bail out early unless it's a deep detection.
                            return OUString();

                        packages::zip::ZipIOException aZipException;

                        // repairing is done only if this type is requested from outside
                        if ( ( aWrap.TargetException >>= aZipException ) && !aTypeName.isEmpty() )
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
                                aTypeName = OUString();
                        }
                    }
                    catch( RuntimeException& )
                    {
                        throw;
                    }
                    catch( Exception& )
                    {
                        aTypeName = OUString();
                    }

                       if ( !aTypeName.isEmpty() )
                    {
                           const SfxFilter* pFilter =
                                    SfxFilterMatcher( "smath" ).GetFilter4EA( aTypeName );
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
                aTypeName = OUString();
                if (pStrm && !pStrm->GetError())
                {
                    SotStorageRef aStorage = new SotStorage ( pStrm, sal_False );
                    if ( !aStorage->GetError() )
                    {
                        if (aStorage->IsStream("Equation Native"))
                        {
                            sal_uInt8 nVersion;
                            if (GetMathTypeVersion( aStorage, nVersion ) && nVersion <=3)
                                aTypeName = "math_MathType_3x";
                        }
                    }
                    else
                    {
                        // 200 should be enough for the XML
                        // version, encoding and !DOCTYPE
                        // stuff I hope?
                        const sal_uInt16 nSize = 200;
                        sal_Char aBuffer[nSize+1];
                        aBuffer[nSize] = 0;
                        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                        pStrm->StartReadingUnicodeText(RTL_TEXTENCODING_DONTKNOW); // avoid BOM marker
                        sal_uLong nBytesRead = pStrm->Read( aBuffer, nSize );
                        if (nBytesRead >= 6)
                        {
                            bool bIsMathType = false;
                            if (0 == strncmp( "<?xml", aBuffer, 5))
                            {
                                if (strstr( aBuffer, "<math>" ) ||
                                        strstr( aBuffer, "<math " ) ||
                                        strstr( aBuffer, "<math:math " ))
                                    bIsMathType = true;
                            }
                            // this is the old <math tag to MathML in the beginning of the XML file
                            else if (0 == strncmp( "<math ", aBuffer, 6) ||
                                         0 == strncmp( "<math> ", aBuffer, 7) ||
                                         0 == strncmp( "<math:math> ", aBuffer, 12))
                                bIsMathType = true;

                            if (bIsMathType){
                                aFilterName = OUString( MATHML_XML );
                                aTypeName = "math_MathML_XML_Math";
                            }
                        }
                    }

                    if ( !aTypeName.isEmpty() )
                    {
                        const SfxFilter* pFilt = SfxFilterMatcher( "smath" ).GetFilter4EA( aTypeName );
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

    if ( aFilterName.isEmpty() )
        aTypeName = OUString();

    return aTypeName;
}

/* XServiceInfo */
OUString SAL_CALL SmFilterDetect::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SmFilterDetect::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    Sequence< OUString > seqServiceNames = getSupportedServiceNames();
    const OUString*      pArray          = seqServiceNames.getConstArray();
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
Sequence< OUString > SAL_CALL SmFilterDetect::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< OUString > SmFilterDetect::impl_getStaticSupportedServiceNames()
{
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = "com.sun.star.frame.ExtendedTypeDetection";
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
OUString SmFilterDetect::impl_getStaticImplementationName()
{
    return OUString("com.sun.star.comp.math.FormatDetector");
}

/* Helper for registry */
Reference< XInterface > SAL_CALL SmFilterDetect::impl_createInstance( const Reference< XMultiServiceFactory >& xServiceManager ) throw( Exception )
{
    return Reference< XInterface >( *new SmFilterDetect( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
