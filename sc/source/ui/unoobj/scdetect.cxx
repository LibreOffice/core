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

// Tabelle mit Suchmustern
// Bedeutung der Sequenzen
// 0x00??: genau Byte 0x?? muss an dieser Stelle stehen
// 0x0100: ein Byte ueberlesen (don't care)
// 0x02nn: ein Byte aus 0xnn Alternativen folgt
// 0x8000: Erkennung abgeschlossen
//

#define M_DC        0x0100
#define M_ALT(ANZ)  (0x0200+(ANZ))
#define M_ENDE      0x8000

const sal_uInt16 pLotus[] =      // Lotus 1/1A/2
    { 0x0000, 0x0000, 0x0002, 0x0000,
    M_ALT(2), 0x0004, 0x0006,
    0x0004, M_ENDE };

const sal_uInt16 pLotusNew[] =   // Lotus >= 9.7
    { 0x0000, 0x0000, M_DC, 0x0000,     // Rec# + Len (0x1a)
      M_ALT(3), 0x0003, 0x0004, 0x0005, // File Revision Code 97->ME
      0x0010, 0x0004, 0x0000, 0x0000,
      M_ENDE };

const sal_uInt16 pLotus2[] =     // Lotus >3
    { 0x0000, 0x0000, 0x001A, 0x0000,   // Rec# + Len (26)
    M_ALT(2), 0x0000, 0x0002,         // File Revision Code
    0x0010,
    0x0004, 0x0000,                   // File Revision Subcode
    M_ENDE };

const sal_uInt16 pQPro[] =
       { 0x0000, 0x0000, 0x0002, 0x0000,
         M_ALT(4), 0x0001, 0x0002, // WB1, WB2
         0x0006, 0x0007,           // QPro 6/7 (?)
         0x0010,
         M_ENDE };

const sal_uInt16 pDIF1[] =       // DIF mit CR-LF
    {
    'T', 'A', 'B', 'L', 'E',
    M_DC, M_DC,
    '0', ',', '1',
    M_DC, M_DC,
    '\"',
    M_ENDE };

const sal_uInt16 pDIF2[] =       // DIF mit CR oder LF
    {
    'T', 'A', 'B', 'L', 'E',
    M_DC,
    '0', ',', '1',
    M_DC,
    '\"',
    M_ENDE };

const sal_uInt16 pSylk[] =       // Sylk
    {
    'I', 'D', ';',
    M_ALT(3), 'P', 'N', 'E',        // 'P' plus undocumented Excel extensions 'N' and 'E'
    M_ENDE };

bool detectThisFormat(SvStream& rStr, const sal_uInt16* pSearch)
{
    sal_uInt8 nByte;
    rStr.Seek( 0 ); // am Anfang war alles Uebel...
    rStr >> nByte;
    bool bSync = true;
    while( !rStr.IsEof() && bSync )
    {
        sal_uInt16 nMuster = *pSearch;

        if( nMuster < 0x0100 )
        { //                                direkter Byte-Vergleich
            if( ( sal_uInt8 ) nMuster != nByte )
                bSync = false;
        }
        else if( nMuster & M_DC )
        { //                                             don't care
        }
        else if( nMuster & M_ALT(0) )
        { //                                      alternative Bytes
            sal_uInt8 nAnzAlt = ( sal_uInt8 ) nMuster;
            bSync = false;          // zunaechst unsynchron
            while( nAnzAlt > 0 )
            {
                pSearch++;
                if( ( sal_uInt8 ) *pSearch == nByte )
                    bSync = true;   // jetzt erst Synchronisierung
                nAnzAlt--;
            }
        }
        else if( nMuster & M_ENDE )
        { //                                        Format detected
            return true;
        }

        pSearch++;
        rStr >> nByte;
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
// This method is no longer used, but I do want to keep this for now to see
// if we could transfer this check to the now centralized ascii detection
// code in the filter module.
static sal_Bool lcl_MayBeAscii( SvStream& rStream )
{
    // ASCII/CSV is considered possible if there are no null bytes, or a Byte
    // Order Mark is present, or if, for Unicode UCS2/UTF-16, all null bytes
    // are on either even or uneven byte positions.

    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    const size_t nBufSize = 2048;
    sal_uInt16 aBuffer[ nBufSize ];
    sal_uInt8* pByte = reinterpret_cast<sal_uInt8*>(aBuffer);
    sal_uLong nBytesRead = rStream.Read( pByte, nBufSize*2);

    if ( nBytesRead >= 2 && (aBuffer[0] == 0xfffe || aBuffer[0] == 0xfeff) )
    {
        // Unicode BOM file may contain null bytes.
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
    // Look for dbf marker, see connectivity/source/inc/dbase/DTable.hxx
    // DBFType for values.
    const sal_uInt8 nValidMarks[] = {
        0x03, 0x04, 0x05, 0x30, 0x43, 0xB3, 0x83, 0x8b, 0x8e, 0xf5 };
    sal_uInt8 nMark;
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream >> nMark;
    bool bValidMark = false;
    for (size_t i=0; i < sizeof(nValidMarks)/sizeof(nValidMarks[0]) && !bValidMark; ++i)
    {
        if (nValidMarks[i] == nMark)
            bValidMark = true;
    }
    if ( !bValidMark )
        return false;

    const size_t nHeaderBlockSize = 32;
    // Empty dbf is >= 32*2+1 bytes in size.
    const size_t nEmptyDbf = nHeaderBlockSize * 2 + 1;

    rStream.Seek(STREAM_SEEK_TO_END);
    sal_uLong nSize = rStream.Tell();
    if ( nSize < nEmptyDbf )
        return false;

    // length of header starts at 8
    rStream.Seek(8);
    sal_uInt16 nHeaderLen;
    rStream >> nHeaderLen;

    if ( nHeaderLen < nEmptyDbf || nSize < nHeaderLen )
        return false;

    // Last byte of header must be 0x0d, this is how it's specified.
    // #i9581#,#i26407# but some applications don't follow the specification
    // and pad the header with one byte 0x00 to reach an
    // even boundary. Some (#i88577# ) even pad more or pad using a 0x1a ^Z
    // control character (#i8857#). This results in:
    // Last byte of header must be 0x0d on 32 bytes boundary.
    sal_uInt16 nBlocks = (nHeaderLen - 1) / nHeaderBlockSize;
    sal_uInt8 nEndFlag = 0;
    while ( nBlocks > 1 && nEndFlag != 0x0d ) {
        rStream.Seek( nBlocks-- * nHeaderBlockSize );
        rStream >> nEndFlag;
    }

    return ( 0x0d == nEndFlag );
}

OUString SAL_CALL ScFilterDetect::detect( uno::Sequence<beans::PropertyValue>& lDescriptor )
    throw( uno::RuntimeException )
{
    uno::Reference< XInputStream > xStream;
    uno::Reference< XContent > xContent;
    uno::Reference< XInteractionHandler > xInteraction;
    String aURL;
    OUString sTemp;
    String aTypeName;            // a name describing the type (from MediaDescriptor, usually from flat detection)
    String aPreselectedFilterName;      // a name describing the filter to use (from MediaDescriptor, usually from UI action)

    OUString aDocumentTitle; // interesting only if set in this method

    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = false;
    sal_Bool bWasReadOnly = false, bReadOnly = false;

    sal_Bool bRepairPackage = false;
    sal_Bool bRepairAllowed = false;
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
        else if( !aURL.Len() && lDescriptor[nProperty].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("FileName")) )
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

    SfxAllItemSet *pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, false );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    String aPrefix = OUString( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        String aPattern( aPrefix );
        aPattern += OUString("scalc");
        if ( aURL.Match( aPattern ) >= aPattern.Len() )
            pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
    }
    else
    {
        // container for Calc filters
        SfxFilterMatcher aMatcher("scalc");
        if ( aPreselectedFilterName.Len() )
            pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
        else if( aTypeName.Len() )
            pFilter = aMatcher.GetFilter4EA( aTypeName );

        // ctor of SfxMedium uses owner transition of ItemSet
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, NULL, pSet );
        aMedium.UseInteractionHandler( sal_True );

        sal_Bool bIsStorage = aMedium.IsStorage();
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            // remember input stream and content and put them into the descriptor later
            // should be done here since later the medium can switch to a version
            xStream.set(aMedium.GetInputStream());
            xContent.set(aMedium.GetContent());
            bReadOnly = aMedium.IsReadOnly();

            // maybe that IsStorage() already created an error!
            if ( bIsStorage )
            {
                uno::Reference < embed::XStorage > xStorage(aMedium.GetStorage( false ));
                if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                {
                    // error during storage creation means _here_ that the medium
                    // is broken, but we can not handle it in medium since unpossibility
                    // to create a storage does not _always_ means that the medium is broken
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
                            // Bail out early unless it's a deep detection.
                            return OUString();

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
                        pFilter = SfxFilterMatcher("scalc").GetFilter4EA( aTypeName );
                }
            }
            else
            {
                // Non-storage format.

                if (aTypeName == "calc8_template" ||
                    aTypeName == "calc8" ||
                    aTypeName == "calc_StarOffice_XML_Calc" ||
                    aTypeName == "calc_StarOffice_XML_Calc_Template")
                    // These types require storage.  Bail out.
                    return OUString();

                SvStream* pStream = aMedium.GetInStream();
                if (!pStream)
                    // No stream, no detection.
                    return OUString();

                pFilter = NULL;

                pStream->Seek( STREAM_SEEK_TO_END);
                sal_Size nSize = pStream->Tell();
                pStream->Seek( 0);
                // Do not attempt to create an SotStorage on a
                // 0-length stream as that would create the compound
                // document header on the stream and effectively write to
                // disk!
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
        aTypeName.Erase();

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
    uno::Sequence<OUString> seqServiceNames(getSupportedServiceNames());
    const OUString* pArray = seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return false ;
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
