/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"

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

#include <svtools/parhtml.hxx>
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
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/brokenpackageint.hxx>
#include <sot/storage.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

ScFilterDetect::ScFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& /* xFactory */ )
{
}

ScFilterDetect::~ScFilterDetect()
{
}

static const sal_Char pFilterSc50[]     = "StarCalc 5.0";
static const sal_Char pFilterSc50Temp[] = "StarCalc 5.0 Vorlage/Template";
static const sal_Char pFilterSc40[]     = "StarCalc 4.0";
static const sal_Char pFilterSc40Temp[] = "StarCalc 4.0 Vorlage/Template";
static const sal_Char pFilterSc30[]     = "StarCalc 3.0";
static const sal_Char pFilterSc30Temp[] = "StarCalc 3.0 Vorlage/Template";
static const sal_Char pFilterSc10[]     = "StarCalc 1.0";
static const sal_Char pFilterXML[]      = "StarOffice XML (Calc)";
static const sal_Char pFilterAscii[]        = "Text - txt - csv (StarCalc)";
static const sal_Char pFilterLotus[]        = "Lotus";
static const sal_Char pFilterQPro6[]        = "Quattro Pro 6.0";
static const sal_Char pFilterExcel4[]   = "MS Excel 4.0";
static const sal_Char pFilterEx4Temp[]  = "MS Excel 4.0 Vorlage/Template";
static const sal_Char pFilterExcel5[]   = "MS Excel 5.0/95";
static const sal_Char pFilterEx5Temp[]  = "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char pFilterExcel95[]  = "MS Excel 95";
static const sal_Char pFilterEx95Temp[] = "MS Excel 95 Vorlage/Template";
static const sal_Char pFilterExcel97[]  = "MS Excel 97";
static const sal_Char pFilterEx97Temp[] = "MS Excel 97 Vorlage/Template";
static const sal_Char pFilter2003XML[]   = "MS Excel 2003 XML";
static const sal_Char pFilterDBase[]        = "dBase";
static const sal_Char pFilterDif[]      = "DIF";
static const sal_Char pFilterSylk[]     = "SYLK";
static const sal_Char pFilterHtml[]     = "HTML (StarCalc)";
static const sal_Char pFilterHtmlWeb[]  = "calc_HTML_WebQuery";
static const sal_Char pFilterRtf[]      = "Rich Text Format (StarCalc)";


static BOOL lcl_MayBeAscii( SvStream& rStream )
{
    // ASCII/CSV is considered possible if there are no null bytes, or a Byte
    // Order Mark is present, or if, for Unicode UCS2/UTF-16, all null bytes
    // are on either even or uneven byte positions.

    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    const size_t nBufSize = 2048;
    sal_uInt16 aBuffer[ nBufSize ];
    sal_uInt8* pByte = reinterpret_cast<sal_uInt8*>(aBuffer);
    ULONG nBytesRead = rStream.Read( pByte, nBufSize*2);

    if ( nBytesRead >= 2 && (aBuffer[0] == 0xfffe || aBuffer[0] == 0xfeff) )
    {
        // Unicode BOM file may contain null bytes.
        return TRUE;
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

static BOOL lcl_MayBeDBase( SvStream& rStream )
{
    // Look for dbf marker, see connectivity/source/inc/dbase/DTable.hxx
    // DBFType for values.
    const BYTE nValidMarks[] = {
        0x03, 0x04, 0x05, 0x30, 0x43, 0xB3, 0x83, 0x8b, 0x8e, 0xf5 };
    BYTE nMark;
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream >> nMark;
    bool bValidMark = false;
    for (size_t i=0; i < SAL_N_ELEMENTS(nValidMarks) && !bValidMark; ++i)
    {
        if (nValidMarks[i] == nMark)
            bValidMark = true;
    }
    if ( !bValidMark )
        return FALSE;

    const size_t nHeaderBlockSize = 32;
    // Empty dbf is >= 32*2+1 bytes in size.
    const size_t nEmptyDbf = nHeaderBlockSize * 2 + 1;

    rStream.Seek(STREAM_SEEK_TO_END);
    ULONG nSize = rStream.Tell();
    if ( nSize < nEmptyDbf )
        return FALSE;

    // length of header starts at 8
    rStream.Seek(8);
    USHORT nHeaderLen;
    rStream >> nHeaderLen;

    if ( nHeaderLen < nEmptyDbf || nSize < nHeaderLen )
        return FALSE;

    // Last byte of header must be 0x0d, this is how it's specified.
    // #i9581#,#i26407# but some applications don't follow the specification
    // and pad the header with one byte 0x00 to reach an
    // even boundary. Some (#i88577# ) even pad more or pad using a 0x1a ^Z
    // control character (#i8857#). This results in:
    // Last byte of header must be 0x0d on 32 bytes boundary.
    USHORT nBlocks = (nHeaderLen - 1) / nHeaderBlockSize;
    BYTE nEndFlag = 0;
    while ( nBlocks > 1 && nEndFlag != 0x0d ) {
        rStream.Seek( nBlocks-- * nHeaderBlockSize );
        rStream >> nEndFlag;
    }

    return ( 0x0d == nEndFlag );
}

::rtl::OUString SAL_CALL ScFilterDetect::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
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
    bool bFakeXLS = false;

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
    SolarMutexGuard aGuard;
    //SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;

    SfxAllItemSet *pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, FALSE );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        String aPattern( aPrefix );
        aPattern += String::CreateFromAscii("scalc");
        if ( aURL.Match( aPattern ) >= aPattern.Len() )
            pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
    }
    else
    {
        // container for Calc filters
        SfxFilterMatcher aMatcher( String::CreateFromAscii("scalc") );
        if ( aPreselectedFilterName.Len() )
            pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
        else if( aTypeName.Len() )
            pFilter = aMatcher.GetFilter4EA( aTypeName );

        // ctor of SfxMedium uses owner transition of ItemSet
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, FALSE, NULL, pSet );
        aMedium.UseInteractionHandler( TRUE );

        BOOL bIsStorage = aMedium.IsStorage();
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
                uno::Reference < embed::XStorage > xStorage(aMedium.GetStorage( sal_False ));
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
                else if ( xStorage.is() )
                {
                    try
                    {
                        String aFilterName;
                        if ( pFilter )
                            aFilterName = pFilter->GetName();
                        aTypeName = SfxFilter::GetTypeFromStorage( xStorage, pFilter ? pFilter->IsOwnTemplateFormat() : FALSE, &aFilterName );
                    }
                    catch( lang::WrappedTargetException& aWrap )
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
                                    RequestPackageReparation* pRequest = new RequestPackageReparation( aDocumentTitle );
                                    uno::Reference< task::XInteractionRequest > xRequest ( pRequest );

                                    xInteraction->handle( xRequest );

                                    bRepairAllowed = pRequest->isApproved();
                                }

                                if ( !bRepairAllowed )
                                {
                                    // repair either not allowed or not successful
                                    NotifyBrokenPackage* pNotifyRequest = new NotifyBrokenPackage( aDocumentTitle );
                                    uno::Reference< task::XInteractionRequest > xRequest ( pNotifyRequest );
                                       xInteraction->handle( xRequest );
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
                           pFilter = SfxFilterMatcher( String::CreateFromAscii("scalc") ).GetFilter4EA( aTypeName );

                }
            }
            else
            {
                bool bIsXLS = false;
                SvStream* pStream = aMedium.GetInStream();
                const SfxFilter* pPreselectedFilter = pFilter;
                if ( pPreselectedFilter && ( ( pPreselectedFilter->GetName().SearchAscii("Excel") != STRING_NOTFOUND ) ||
                    ( !aPreselectedFilterName.Len() && pPreselectedFilter->GetFilterName().EqualsAscii( pFilterAscii ) ) ) )
                    bIsXLS = true;
                pFilter = 0;
                if ( pStream )
                {
                    SotStorageRef aStorage = new SotStorage ( pStream, FALSE );
                    if ( !aStorage->GetError() )
                    {
                        // Excel-5: detect through contained streams
                        // there are some "excel" formats from 3rd party vendors that need to be distinguished
                        String aStreamName(RTL_CONSTASCII_USTRINGPARAM("Workbook"));
                        BOOL bExcel97Stream = ( aStorage->IsStream( aStreamName ) );

                        aStreamName = String(RTL_CONSTASCII_USTRINGPARAM("Book"));
                        BOOL bExcel5Stream = ( aStorage->IsStream( aStreamName ) );
                        if ( bExcel97Stream || bExcel5Stream )
                        {
                            if ( bExcel97Stream )
                            {
                                String aOldName;
                                BOOL bIsCalcFilter = TRUE;
                                if ( pPreselectedFilter )
                                {
                                    // cross filter; now this should be a type detection only, not a filter detection
                                    // we can simulate it by preserving the preselected filter if the type matches
                                    // example: Excel filters for Writer
                                    aOldName = pPreselectedFilter->GetFilterName();
                                    bIsCalcFilter = pPreselectedFilter->GetServiceName().EqualsAscii("com.sun.star.sheet.SpreadsheetDocument");
                                }

                                if ( aOldName.EqualsAscii(pFilterEx97Temp) || !bIsCalcFilter )
                                {
                                    //  Excel 97 template selected -> keep selection
                                }
                                else if ( bExcel5Stream &&
                                            ( aOldName.EqualsAscii(pFilterExcel5) || aOldName.EqualsAscii(pFilterEx5Temp) ||
                                            aOldName.EqualsAscii(pFilterExcel95) || aOldName.EqualsAscii(pFilterEx95Temp) ) )
                                {
                                    //  dual format file and Excel 5 selected -> keep selection
                                }
                                else
                                {
                                    //  else use Excel 97 filter
                                    pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterExcel97) );
                                }
                            }
                            else if ( bExcel5Stream )
                            {
                                String aOldName;
                                BOOL bIsCalcFilter = TRUE;
                                if ( pPreselectedFilter )
                                {
                                    // cross filter; now this should be a type detection only, not a filter detection
                                    // we can simulate it by preserving the preselected filter if the type matches
                                    // example: Excel filters for Writer
                                    aOldName = pPreselectedFilter->GetFilterName();
                                    bIsCalcFilter = pPreselectedFilter->GetServiceName().EqualsAscii("com.sun.star.sheet.SpreadsheetDocument");
                                }

                                if ( aOldName.EqualsAscii(pFilterExcel95) || aOldName.EqualsAscii(pFilterEx95Temp) ||
                                        aOldName.EqualsAscii(pFilterEx5Temp) || !bIsCalcFilter )
                                {
                                    //  Excel 95 oder Vorlage (5 oder 95) eingestellt -> auch gut
                                }
                                else if ( aOldName.EqualsAscii(pFilterEx97Temp) )
                                {
                                    // auto detection has found template -> return Excel5 template
                                    pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterEx5Temp) );
                                }
                                else
                                {
                                    //  sonst wird als Excel 5-Datei erkannt
                                    pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterExcel5) );
                                }
                            }
                        }
                    }
                    else
                    {
                        SvStream &rStr = *pStream;

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

                        static const UINT16 pLotus[] =      // Lotus 1/1A/2
                            { 0x0000, 0x0000, 0x0002, 0x0000,
                            M_ALT(2), 0x0004, 0x0006,
                            0x0004, M_ENDE };

                        static const UINT16 pLotusNew[] =   // Lotus >= 9.7
                            { 0x0000, 0x0000, M_DC, 0x0000,     // Rec# + Len (0x1a)
                              M_ALT(3), 0x0003, 0x0004, 0x0005, // File Revision Code 97->ME
                              0x0010, 0x0004, 0x0000, 0x0000,
                              M_ENDE };

                        static const UINT16 pExcel1[] =     // Excel BIFF2, BIFF3, BIFF4
                            {   0x09,                                   // lobyte of BOF rec ID (0x0009, 0x0209, 0x0409)
                                M_ALT(3), 0x00, 0x02, 0x04,             // hibyte of BOF rec ID (0x0009, 0x0209, 0x0409)
                                M_ALT(3), 4, 6, 8,                      // lobyte of BOF rec size (4, 6, 8, 16)
                                0x00,                                   // hibyte of BOF rec size (4, 6, 8, 16)
                                M_DC, M_DC,                             // any version
                                M_ALT(3), 0x10, 0x20, 0x40,             // lobyte of data type (0x0010, 0x0020, 0x0040)
                                0x00,                                   // hibyte of data type (0x0010, 0x0020, 0x0040)
                                M_ENDE };

                        static const UINT16 pExcel2[] =     // Excel BIFF4 Workspace
                            {   0x09,                                   // lobyte of BOF rec ID (0x0409)
                                0x04,                                   // hibyte of BOF rec ID (0x0409)
                                M_ALT(3), 4, 6, 8,                      // lobyte of BOF rec size (4, 6, 8, 16)
                                0x00,                                   // hibyte of BOF rec size (4, 6, 8, 16)
                                M_DC, M_DC,                             // any version
                                0x00,                                   // lobyte of data type (0x0100)
                                0x01,                                   // hibyte of data type (0x0100)
                                M_ENDE };

                        static const UINT16 pExcel3[] =     // #i23425# Excel BIFF5, BIFF7, BIFF8 (simple book stream)
                            {   0x09,                                   // lobyte of BOF rec ID (0x0809)
                                0x08,                                   // hibyte of BOF rec ID (0x0809)
                                M_ALT(4), 4, 6, 8, 16,                  // lobyte of BOF rec size
                                0x00,                                   // hibyte of BOF rec size
                                M_DC, M_DC,                             // any version
                                M_ALT(5), 0x05, 0x06, 0x10, 0x20, 0x40, // lobyte of data type
                                0x00,                                   // hibyte of data type
                                M_ENDE };

                        static const UINT16 pSc10[] =       // StarCalc 1.0 Dokumente
                            { 'B', 'l', 'a', 'i', 's', 'e', '-', 'T', 'a', 'b', 'e', 'l', 'l',
                            'e', 0x000A, 0x000D, 0x0000,    // Sc10CopyRight[16]
                            M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC,
                            M_DC, M_DC,                   // Sc10CopyRight[29]
                            M_ALT(2), 0x0065, 0x0066,     // Versionsnummer 101 oder 102
                            0x0000,
                            M_ENDE };

                        static const UINT16 pLotus2[] =     // Lotus >3
                            { 0x0000, 0x0000, 0x001A, 0x0000,   // Rec# + Len (26)
                            M_ALT(2), 0x0000, 0x0002,         // File Revision Code
                            0x0010,
                            0x0004, 0x0000,                   // File Revision Subcode
                            M_ENDE };

                        static const UINT16 pQPro[] =
                               { 0x0000, 0x0000, 0x0002, 0x0000,
                                 M_ALT(4), 0x0001, 0x0002, // WB1, WB2
                                 0x0006, 0x0007,           // QPro 6/7 (?)
                                 0x0010,
                                 M_ENDE };

                        static const UINT16 pDIF1[] =       // DIF mit CR-LF
                            {
                            'T', 'A', 'B', 'L', 'E',
                            M_DC, M_DC,
                            '0', ',', '1',
                            M_DC, M_DC,
                            '\"',
                            M_ENDE };

                        static const UINT16 pDIF2[] =       // DIF mit CR oder LF
                            {
                            'T', 'A', 'B', 'L', 'E',
                            M_DC,
                            '0', ',', '1',
                            M_DC,
                            '\"',
                            M_ENDE };

                        static const UINT16 pSylk[] =       // Sylk
                            {
                            'I', 'D', ';',
                            M_ALT(3), 'P', 'N', 'E',        // 'P' plus undocumented Excel extensions 'N' and 'E'
                            M_ENDE };

                        static const UINT16 *ppFilterPatterns[] =      // Arrays mit Suchmustern
                            {
                            pLotus,
                            pExcel1,
                            pExcel2,
                            pExcel3,
                            pSc10,
                            pDIF1,
                            pDIF2,
                            pSylk,
                            pLotusNew,
                            pLotus2,
                            pQPro
                            };
                        const UINT16 nFilterCount = SAL_N_ELEMENTS(ppFilterPatterns);

                        static const sal_Char* const pFilterName[] =     // zugehoerige Filter
                            {
                            pFilterLotus,
                            pFilterExcel4,
                            pFilterExcel4,
                            pFilterExcel4,
                            pFilterSc10,
                            pFilterDif,
                            pFilterDif,
                            pFilterSylk,
                            pFilterLotus,
                            pFilterLotus,
                            pFilterQPro6
                            };

                        // suchen Sie jetzt!
                        // ... realisiert ueber 'Mustererkennung'

                        BYTE            nAkt;
                        BOOL            bSync;          // Datei und Muster stimmen ueberein
                        USHORT          nFilter;        // Zaehler ueber alle Filter
                        const UINT16    *pSearch;       // aktuelles Musterwort

                        for ( nFilter = 0 ; nFilter < nFilterCount ; nFilter++ )
                        {
                            rStr.Seek( 0 ); // am Anfang war alles Uebel...
                            rStr >> nAkt;
                            pSearch = ppFilterPatterns[ nFilter ];
                            bSync = TRUE;
                            while( !rStr.IsEof() && bSync )
                            {
                                register UINT16 nMuster = *pSearch;

                                if( nMuster < 0x0100 )
                                { //                                direkter Byte-Vergleich
                                    if( ( BYTE ) nMuster != nAkt )
                                        bSync = FALSE;
                                }
                                else if( nMuster & M_DC )
                                { //                                             don't care
                                }
                                else if( nMuster & M_ALT(0) )
                                { //                                      alternative Bytes
                                    BYTE nAnzAlt = ( BYTE ) nMuster;
                                    bSync = FALSE;          // zunaechst unsynchron
                                    while( nAnzAlt > 0 )
                                    {
                                        pSearch++;
                                        if( ( BYTE ) *pSearch == nAkt )
                                            bSync = TRUE;   // jetzt erst Synchronisierung
                                        nAnzAlt--;
                                    }
                                }
                                else if( nMuster & M_ENDE )
                                { //                                        Format detected
                                    if ( pFilterName[nFilter] == pFilterExcel4 && pPreselectedFilter &&
                                        ( (pPreselectedFilter)->GetFilterName().EqualsAscii(pFilterEx4Temp) || pPreselectedFilter->GetTypeName().EqualsAscii("calc_MS_Excel_40") ) )
                                    {
                                        //  Excel 4 erkannt, Excel 4 Vorlage eingestellt -> auch gut
                                        // oder Excel 4 Filter anderer Applikation (simulated type detection!)
                                    }
                                    else
                                    {   // gefundenen Filter einstellen
                                        pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterName[ nFilter ]) );
                                    }
                                    bSync = FALSE;              // leave inner loop
                                    nFilter = nFilterCount;     // leave outer loop
                                }
                                else
                                { //                                         Tabellenfehler
                                    DBG_ERROR( "-ScApplication::DetectFilter(): Fehler in Mustertabelle");
                                }

                                pSearch++;
                                rStr >> nAkt;
                            }
                        }

                        if ( pPreselectedFilter && !pFilter )
                        {
                            // further checks for filters only if they are preselected: ASCII, HTML, RTF, DBase
                            // without the preselection other filters (Writer) take precedence
                            // DBase can't be detected reliably, so it also needs preselection
                            bool bMaybeText = lcl_MayBeAscii( rStr );

                            // get file header
                            rStr.Seek( 0 );
                            const int nTrySize = 80;
                            ByteString aHeader;
                            for ( int j = 0; j < nTrySize && !rStr.IsEof(); j++ )
                            {
                                sal_Char c;
                                rStr >> c;
                                aHeader += c;
                            }
                            aHeader += '\0';

                            if ( HTMLParser::IsHTMLFormat( aHeader.GetBuffer() ) )
                            {
                                // test for HTML
                                if ( pPreselectedFilter->GetName().EqualsAscii(pFilterHtml) )
                                {
                                    pFilter = pPreselectedFilter;
                                }
                                else
                                {
                                    pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterHtmlWeb) );
                                    if ( bIsXLS )
                                        bFakeXLS = true;
                                }
                            }
                            else if ( aHeader.CompareTo( "{\\rtf", 5 ) == COMPARE_EQUAL )
                            {
                                // test for RTF
                                pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterRtf) );
                            }
                            else if ( bIsXLS && bMaybeText )
                            {
                                aHeader.EraseLeadingChars();
                                if( aHeader.CompareTo( "<?xml", 5 ) == COMPARE_EQUAL )
                                    pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilter2003XML) );
                                else
                                    pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterAscii) );
                                bFakeXLS = true;
                            }
                            else if ( pPreselectedFilter->GetName().EqualsAscii(pFilterDBase) && lcl_MayBeDBase( rStr ) )
                                pFilter = pPreselectedFilter;
                            else if ( pPreselectedFilter->GetFilterName().EqualsAscii(pFilterAscii) && bMaybeText )
                                pFilter = pPreselectedFilter;
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
        lDescriptor[nPropertyCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream"));
        lDescriptor[nPropertyCount].Value <<= xStream;
        nPropertyCount++;
    }

    if ( nIndexOfContent == -1 && xContent.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UCBContent"));
        lDescriptor[nPropertyCount].Value <<= xContent;
        nPropertyCount++;
    }

    if ( bReadOnly != bWasReadOnly )
    {
        if ( nIndexOfReadOnlyFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            lDescriptor[nPropertyCount].Value <<= bReadOnly;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfReadOnlyFlag].Value <<= bReadOnly;
    }

    if ( !bRepairPackage && bRepairAllowed )
    {
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RepairPackage"));
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
            lDescriptor[nPropertyCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
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
            lDescriptor[nPropertyCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentTitle"));
            lDescriptor[nPropertyCount].Value <<= aDocumentTitle;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfDocumentTitle].Value <<= aDocumentTitle;
    }

    if ( bFakeXLS )
    {
        if ( nIndexOfFilterName == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
            lDescriptor[nPropertyCount].Value <<= rtl::OUString(pFilter->GetName());
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfFilterName].Value <<= rtl::OUString(pFilter->GetName());
    }

    if ( pFilter )
        aTypeName = pFilter->GetTypeName();
    else
        aTypeName.Erase();
    return aTypeName;
}

SFX_IMPL_SINGLEFACTORY( ScFilterDetect )

/* XServiceInfo */
UNOOUSTRING SAL_CALL ScFilterDetect::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL ScFilterDetect::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames(getSupportedServiceNames());
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
UNOSEQUENCE< UNOOUSTRING > SAL_CALL ScFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > ScFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = UNOOUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.ExtendedTypeDetection"  ));
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING ScFilterDetect::impl_getStaticImplementationName()
{
    return UNOOUSTRING(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.calc.FormatDetector" ));
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL ScFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new ScFilterDetect( xServiceManager ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
