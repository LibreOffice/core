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

#include "scdetect.hxx"

#include <sal/macros.h>

#include <framework/interaction.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
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

#include <svtools/parhtml.hxx>
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
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/brokenpackageint.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;

namespace {

const sal_Char pFilterLotus[]        = "Lotus";
const sal_Char pFilterQPro6[]        = "Quattro Pro 6.0";
const sal_Char pFilterDBase[]        = "dBase";
const sal_Char pFilterDif[]      = "DIF";
const sal_Char pFilterSylk[]     = "SYLK";







//

#define M_DC        0x0100
#define M_ALT(ANZ)  (0x0200+(ANZ))
#define M_ENDE      0x8000

const sal_uInt16 pLotus[] =      
    { 0x0000, 0x0000, 0x0002, 0x0000,
    M_ALT(2), 0x0004, 0x0006,
    0x0004, M_ENDE };

const sal_uInt16 pLotusNew[] =   
    { 0x0000, 0x0000, M_DC, 0x0000,     
      M_ALT(3), 0x0003, 0x0004, 0x0005, 
      0x0010, 0x0004, 0x0000, 0x0000,
      M_ENDE };

const sal_uInt16 pLotus2[] =     
    { 0x0000, 0x0000, 0x001A, 0x0000,   
    M_ALT(2), 0x0000, 0x0002,         
    0x0010,
    0x0004, 0x0000,                   
    M_ENDE };

const sal_uInt16 pQPro[] =
       { 0x0000, 0x0000, 0x0002, 0x0000,
         M_ALT(4), 0x0001, 0x0002, 
         0x0006, 0x0007,           
         0x0010,
         M_ENDE };

const sal_uInt16 pDIF1[] =       
    {
    'T', 'A', 'B', 'L', 'E',
    M_DC, M_DC,
    '0', ',', '1',
    M_DC, M_DC,
    '\"',
    M_ENDE };

const sal_uInt16 pDIF2[] =       
    {
    'T', 'A', 'B', 'L', 'E',
    M_DC,
    '0', ',', '1',
    M_DC,
    '\"',
    M_ENDE };

const sal_uInt16 pSylk[] =       
    {
    'I', 'D', ';',
    M_ALT(3), 'P', 'N', 'E',        
    M_ENDE };

bool detectThisFormat(SvStream& rStr, const sal_uInt16* pSearch)
{
    sal_uInt8 nByte;
    rStr.Seek( 0 ); 
    rStr.ReadUChar( nByte );
    bool bSync = true;
    while( !rStr.IsEof() && bSync )
    {
        sal_uInt16 nMuster = *pSearch;

        if( nMuster < 0x0100 )
        { 
            if( ( sal_uInt8 ) nMuster != nByte )
                bSync = false;
        }
        else if( nMuster & M_DC )
        { 
        }
        else if( nMuster & M_ALT(0) )
        { 
            sal_uInt8 nAnzAlt = ( sal_uInt8 ) nMuster;
            bSync = false;          
            while( nAnzAlt > 0 )
            {
                pSearch++;
                if( ( sal_uInt8 ) *pSearch == nByte )
                    bSync = true;   
                nAnzAlt--;
            }
        }
        else if( nMuster & M_ENDE )
        { 
            return true;
        }

        pSearch++;
        rStr.ReadUChar( nByte );
    }

    return false;
}

}

ScFilterDetect::ScFilterDetect( const uno::Reference<uno::XComponentContext>& /*xContext*/ )
{
}

ScFilterDetect::~ScFilterDetect()
{
}

#if 0



static sal_Bool lcl_MayBeAscii( SvStream& rStream )
{
    
    
    

    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    const size_t nBufSize = 2048;
    sal_uInt16 aBuffer[ nBufSize ];
    sal_uInt8* pByte = reinterpret_cast<sal_uInt8*>(aBuffer);
    sal_uLong nBytesRead = rStream.Read( pByte, nBufSize*2);

    if ( nBytesRead >= 2 && (aBuffer[0] == 0xfffe || aBuffer[0] == 0xfeff) )
    {
        
        return sal_True;
    }

    const sal_uInt16* p = aBuffer;
    sal_uInt16 nMask = 0xffff;
    nBytesRead /= 2;
    while( nBytesRead-- && nMask )
    {
        sal_uInt16 nVal = *p++ & nMask;
        if (!(nVal & 0x00ff))
            nMask &= 0xff00;
        if (!(nVal & 0xff00))
            nMask &= 0x00ff;
    }

    return nMask != 0;
}
#endif

static sal_Bool lcl_MayBeDBase( SvStream& rStream )
{
    
    
    const sal_uInt8 nValidMarks[] = {
        0x03, 0x04, 0x05, 0x30, 0x43, 0xB3, 0x83, 0x8b, 0x8e, 0xf5 };
    sal_uInt8 nMark;
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ReadUChar( nMark );
    bool bValidMark = false;
    for (size_t i=0; i < sizeof(nValidMarks)/sizeof(nValidMarks[0]) && !bValidMark; ++i)
    {
        if (nValidMarks[i] == nMark)
            bValidMark = true;
    }
    if ( !bValidMark )
        return false;

    const size_t nHeaderBlockSize = 32;
    
    const size_t nEmptyDbf = nHeaderBlockSize * 2 + 1;

    rStream.Seek(STREAM_SEEK_TO_END);
    sal_uLong nSize = rStream.Tell();
    if ( nSize < nEmptyDbf )
        return false;

    
    rStream.Seek(8);
    sal_uInt16 nHeaderLen;
    rStream.ReadUInt16( nHeaderLen );

    if ( nHeaderLen < nEmptyDbf || nSize < nHeaderLen )
        return false;

    
    
    
    
    
    
    sal_uInt16 nBlocks = (nHeaderLen - 1) / nHeaderBlockSize;
    sal_uInt8 nEndFlag = 0;
    while ( nBlocks > 1 && nEndFlag != 0x0d ) {
        rStream.Seek( nBlocks-- * nHeaderBlockSize );
        rStream.ReadUChar( nEndFlag );
    }

    return ( 0x0d == nEndFlag );
}

OUString SAL_CALL ScFilterDetect::detect( uno::Sequence<beans::PropertyValue>& lDescriptor )
    throw( uno::RuntimeException )
{
    uno::Reference< XInputStream > xStream;
    uno::Reference< XContent > xContent;
    uno::Reference< XInteractionHandler > xInteraction;
    OUString aURL;
    OUString sTemp;
    OUString aTypeName;            
    OUString aPreselectedFilterName;      

    OUString aDocumentTitle; 

    
    
    
    sal_Bool bOpenAsTemplate = false;
    sal_Bool bWasReadOnly = false, bReadOnly = false;

    sal_Bool bRepairPackage = false;
    sal_Bool bRepairAllowed = false;
    bool bDeepDetection = false;

    
    
    sal_Int32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfFilterName = -1;
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
    sal_Int32 nIndexOfDocumentTitle = -1;

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

            
            
            nIndexOfFilterName = nProperty;
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

    
    SolarMutexGuard aGuard;
    

    SfxAllItemSet *pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, false );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    OUString aPrefix = "private:factory/";
    if( aURL.startsWith( aPrefix ) )
    {
        OUString aPattern = aPrefix + "scalc";
        if ( aURL.startsWith( aPattern ) )
            pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
    }
    else
    {
        
        SfxFilterMatcher aMatcher("scalc");
        if ( !aPreselectedFilterName.isEmpty() )
            pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
        else if( !aTypeName.isEmpty() )
            pFilter = aMatcher.GetFilter4EA( aTypeName );

        
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, NULL, pSet );
        aMedium.UseInteractionHandler( sal_True );

        sal_Bool bIsStorage = aMedium.IsStorage();
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            
            
            xStream.set(aMedium.GetInputStream());
            xContent.set(aMedium.GetContent());
            bReadOnly = aMedium.IsReadOnly();

            
            if ( bIsStorage )
            {
                uno::Reference < embed::XStorage > xStorage(aMedium.GetStorage( false ));
                if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                {
                    
                    
                    
                    aMedium.SetError(aMedium.GetLastStorageCreationState(), OUString(OSL_LOG_PREFIX));
                    if ( xInteraction.is() )
                    {
                        OUString empty;
                        try
                        {
                            InteractiveAppException xException( empty,
                                                            uno::Reference< XInterface >(),
                                                            InteractionClassification_ERROR,
                                                            aMedium.GetError() );

                            uno::Reference< XInteractionRequest > xRequest(
                                new ucbhelper::SimpleInteractionRequest( makeAny( xException ),
                                                                      ucbhelper::CONTINUATION_APPROVE ) );
                            xInteraction->handle( xRequest );
                        }
                        catch ( Exception & ) {};
                    }
                }
                else if ( xStorage.is() )
                {
                    try
                    {
                        OUString aFilterName;
                        if ( pFilter )
                            aFilterName = pFilter->GetName();
                        aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pFilter ? pFilter->IsOwnTemplateFormat() : false, &aFilterName );
                    }
                    catch( const lang::WrappedTargetException& aWrap )
                    {
                        if (!bDeepDetection)
                            
                            return OUString();

                        packages::zip::ZipIOException aZipException;

                        
                        if ( ( aWrap.TargetException >>= aZipException ) && !aTypeName.isEmpty() )
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
                                }
                            }

                            if ( !bRepairAllowed )
                                aTypeName = "";
                        }
                    }
                    catch( uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch( uno::Exception& )
                    {
                        aTypeName = "";
                    }

                    if ( !aTypeName.isEmpty() )
                        pFilter = SfxFilterMatcher("scalc").GetFilter4EA( aTypeName );
                }
            }
            else
            {
                

                if (aTypeName == "calc8_template" ||
                    aTypeName == "calc8" ||
                    aTypeName == "calc_StarOffice_XML_Calc" ||
                    aTypeName == "calc_StarOffice_XML_Calc_Template")
                    
                    return OUString();

                SvStream* pStream = aMedium.GetInStream();
                if (!pStream)
                    
                    return OUString();

                pFilter = NULL;

                pStream->Seek( STREAM_SEEK_TO_END);
                sal_Size nSize = pStream->Tell();
                pStream->Seek( 0);
                
                
                
                
                if (nSize > 0)
                {
                    const char* pSearchFilterName = NULL;
                    if (aTypeName == "calc_Lotus")
                    {
                        if (!detectThisFormat(*pStream, pLotus) && !detectThisFormat(*pStream, pLotusNew) && !detectThisFormat(*pStream, pLotus2))
                            return OUString();

                        pSearchFilterName = pFilterLotus;
                    }
                    else if (aTypeName == "calc_QPro")
                    {
                        if (!detectThisFormat(*pStream, pQPro))
                            return OUString();

                        pSearchFilterName = pFilterQPro6;
                    }
                    else if (aTypeName == "calc_SYLK")
                    {
                        if (!detectThisFormat(*pStream, pSylk))
                            return OUString();

                        pSearchFilterName = pFilterSylk;
                    }
                    else if (aTypeName == "calc_DIF")
                    {
                        if (!detectThisFormat(*pStream, pDIF1) && !detectThisFormat(*pStream, pDIF2))
                            return OUString();

                        pSearchFilterName = pFilterDif;
                    }
                    else if (aTypeName == "calc_dBase")
                    {
                        if (!lcl_MayBeDBase(*pStream))
                            return OUString();

                        pSearchFilterName = pFilterDBase;
                    }

                    if (!pSearchFilterName)
                        return OUString();

                    pFilter = aMatcher.GetFilter4FilterName(OUString::createFromAscii(pSearchFilterName));
                }
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

    if (!pFilter)
        return OUString();

    if (nIndexOfFilterName == -1)
    {
        lDescriptor.realloc(nPropertyCount + 1);
        lDescriptor[nPropertyCount].Name = "FilterName";
        lDescriptor[nPropertyCount].Value <<= pFilter->GetName();
        ++nPropertyCount;
    }
    else
        lDescriptor[nIndexOfFilterName].Value <<= pFilter->GetName();

    return aTypeName;
}

OUString SAL_CALL ScFilterDetect::getImplementationName() throw (uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

sal_Bool ScFilterDetect::supportsService( const OUString& sServiceName )
    throw (uno::RuntimeException)
{
    return cppu::supportsService(this, sServiceName);
}

com::sun::star::uno::Sequence<OUString> ScFilterDetect::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

uno::Sequence<OUString> ScFilterDetect::impl_getStaticSupportedServiceNames()
{
    uno::Sequence<OUString> seqServiceNames(1);
    seqServiceNames.getArray()[0] = "com.sun.star.frame.ExtendedTypeDetection";
    return seqServiceNames;
}

OUString ScFilterDetect::impl_getStaticImplementationName()
{
    return OUString("com.sun.star.comp.calc.FormatDetector");
}

uno::Reference<uno::XInterface> ScFilterDetect::impl_createInstance(
    const uno::Reference<uno::XComponentContext>& xContext ) throw (uno::Exception)
{
    return static_cast<cppu::OWeakObject*>(new ScFilterDetect(xContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
