/*************************************************************************
 *
 *  $RCSfile: scdetect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:25:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "scdetect.hxx"

#include <framework/interaction.hxx>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAPPEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#include <framework/interaction.hxx>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#include <ucbhelper/simpleinteractionrequest.hxx>
#endif

#include <svtools/parhtml.hxx>
#include <rtl/ustring.h>
#include <rtl/logfile.hxx>
#include <svtools/itemset.hxx>
#include <vcl/window.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>
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

//#include "document.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

ScFilterDetect::ScFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
{
}

ScFilterDetect::~ScFilterDetect()
{
}

static const sal_Char __FAR_DATA pFilterSc50[]      = "StarCalc 5.0";
static const sal_Char __FAR_DATA pFilterSc50Temp[]  = "StarCalc 5.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc40[]      = "StarCalc 4.0";
static const sal_Char __FAR_DATA pFilterSc40Temp[]  = "StarCalc 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc30[]      = "StarCalc 3.0";
static const sal_Char __FAR_DATA pFilterSc30Temp[]  = "StarCalc 3.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc10[]      = "StarCalc 1.0";
static const sal_Char __FAR_DATA pFilterXML[]       = "StarOffice XML (Calc)";
static const sal_Char __FAR_DATA pFilterAscii[]     = "Text - txt - csv (StarCalc)";
static const sal_Char __FAR_DATA pFilterLotus[]     = "Lotus";
static const sal_Char __FAR_DATA pFilterExcel4[]    = "MS Excel 4.0";
static const sal_Char __FAR_DATA pFilterEx4Temp[]   = "MS Excel 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel5[]    = "MS Excel 5.0/95";
static const sal_Char __FAR_DATA pFilterEx5Temp[]   = "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel95[]   = "MS Excel 95";
static const sal_Char __FAR_DATA pFilterEx95Temp[]  = "MS Excel 95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel97[]   = "MS Excel 97";
static const sal_Char __FAR_DATA pFilterEx97Temp[]  = "MS Excel 97 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterDBase[]     = "dBase";
static const sal_Char __FAR_DATA pFilterDif[]       = "DIF";
static const sal_Char __FAR_DATA pFilterSylk[]      = "SYLK";
static const sal_Char __FAR_DATA pFilterHtml[]      = "HTML (StarCalc)";
static const sal_Char __FAR_DATA pFilterHtmlWeb[]   = "calc_HTML_WebQuery";
static const sal_Char __FAR_DATA pFilterRtf[]       = "Rich Text Format (StarCalc)";


BOOL lcl_MayBeAscii( SvStream& rStream )
{
    //  ASCII is considered possible if there are no null bytes

    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    BOOL bNullFound = FALSE;
    BYTE aBuffer[ 4097 ];
    const BYTE* p = aBuffer;
    ULONG nBytesRead = rStream.Read( aBuffer, 4096 );

    if ( nBytesRead >= 2 &&
            ( ( aBuffer[0] == 0xff && aBuffer[1] == 0xfe ) ||
              ( aBuffer[0] == 0xfe && aBuffer[1] == 0xff ) ) )
    {
        //  unicode file may contain null bytes
        return TRUE;
    }

    while( nBytesRead-- )
        if( !*p++ )
        {
            bNullFound = TRUE;
            break;
        }

    return !bNullFound;
}

BOOL lcl_MayBeDBase( SvStream& rStream )
{
    //  for dBase, look for the 0d character at the end of the header

    rStream.Seek(STREAM_SEEK_TO_END);
    ULONG nSize = rStream.Tell();

    // length of header starts at 8

    if ( nSize < 10 )
        return FALSE;
    rStream.Seek(8);
    USHORT nHeaderLen;
    rStream >> nHeaderLen;

    if ( nHeaderLen < 32 || nSize < nHeaderLen )
        return FALSE;

    // last byte of header must be 0d

    rStream.Seek( nHeaderLen - 1 );
    BYTE nEndFlag;
    rStream >> nEndFlag;

    return ( nEndFlag == 0x0d );
}

BOOL lcl_IsAnyXMLFilter( const SfxFilter* pFilter )
{
    if ( !pFilter )
        return FALSE;

    //  TRUE for XML file or template
    //  (template filter has no internal name -> allow configuration key names)

    String aName = pFilter->GetFilterName();
    return aName.EqualsAscii(pFilterXML) ||
           aName.EqualsAscii("calc_StarOffice_XML_Calc") ||
           aName.EqualsAscii("calc_StarOffice_XML_Calc_Template");
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

    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Bool bWasReadOnly = sal_False, bReadOnly = sal_False;

    // now some parameters that can already be in the array, but may be overwritten or new inserted here
    // remember their indices in the case new values must be added to the array
    sal_Int32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfFilterName = -1;
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
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
    }

    // can't check the type for external filters, so set the "dont" flag accordingly
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    //SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;

    SfxAllItemSet *pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, FALSE );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    String aFilterName;
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
        String aFilterName;
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            // remember input stream and content and put them into the descriptor later
            // should be done here since later the medium can switch to a version
            xStream = aMedium.GetInputStream();
            xContent = aMedium.GetContent();
            bReadOnly = aMedium.IsReadOnly();

            // maybe that IsStorage() already created an error!
            if ( bIsStorage )
            {
                SotStorageRef xStorage = aMedium.GetStorage();
                if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                {
                    // error during storage creation means _here_ that the medium
                    // is broken, but we can not handle it in medium since unpossibility
                    // to create a storage does not _always_ means that the medium is broken
                    aMedium.SetError( aMedium.GetLastStorageCreationState() );
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
                else if ( xStorage.Is() )
                {
                    // Excel-5: detect through contained streams
                    // there are some "excel" formats from 3rd party vendors that need to be distinguished
                    String aStreamName;
                    aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Workbook"));
                    BOOL bExcel97Stream = ( xStorage->IsContained( aStreamName ) && xStorage->IsStream( aStreamName ) );

                    aStreamName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Book"));
                    BOOL bExcel5Stream = ( xStorage->IsContained( aStreamName ) && xStorage->IsStream( aStreamName ) );

                    if ( !bExcel97Stream && !bExcel5Stream )
                    {
                        // if it's not Excel-5/97, it must be our own format
                        if ( pFilter && !pFilter->GetFormat() )
                            // preselected Filter has no ClipboardId -> doesn't match to own format
                            pFilter = 0;

                        // now the "real" type detection: check if the filter has the right ClipboardId
                        if ( pFilter && pFilter->GetFormat() != xStorage->GetFormat() )
                            // preselected Filter has different ClipboardId -> doesn't match
                            pFilter = 0;

                        if ( !pFilter )
                            // not found until now, check all own formats for ClipboardId
                            pFilter = aMatcher.GetFilter4ClipBoardId( xStorage->GetFormat() );
                    }
                    else
                    {
                        if ( bExcel97Stream )
                        {
                            String aOldName;
                            BOOL bIsCalcFilter = TRUE;
                            if ( pFilter )
                            {
                                // cross filter; now this should be a type detection only, not a filter detection
                                // we can simulate it by preserving the preselected filter if the type matches
                                // example: Excel filters for Writer
                                aOldName = pFilter->GetFilterName();
                                bIsCalcFilter = pFilter->GetServiceName().EqualsAscii("com.sun.star.sheet.SpreadsheetDocument");
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
                            if ( pFilter )
                            {
                                // cross filter; now this should be a type detection only, not a filter detection
                                // we can simulate it by preserving the preselected filter if the type matches
                                // example: Excel filters for Writer
                                aOldName = pFilter->GetFilterName();
                                bIsCalcFilter = pFilter->GetServiceName().EqualsAscii("com.sun.star.sheet.SpreadsheetDocument");
                            }

                            if ( aOldName.EqualsAscii(pFilterExcel95) || aOldName.EqualsAscii(pFilterEx95Temp) ||
                                    aOldName.EqualsAscii(pFilterEx5Temp) || !bIsCalcFilter )
                            {
                                //  Excel 95 oder Vorlage (5 oder 95) eingestellt -> auch gut
                            }
                            else if ( aOldName.EqualsAscii(pFilterEx97Temp) )
                            {
                                // #101923# auto detection has found template -> return Excel5 template
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
            }
            else
            {
                SvStream &rStr = *aMedium.GetInStream();

                // Tabelle mit Suchmustern
                // Bedeutung der Sequenzen
                // 0x00??: genau Byte 0x?? muss an dieser Stelle stehen
                // 0x0100: ein Byte ueberlesen (don't care)
                // 0x02nn: ein Byte aus 0xnn Alternativen folgt
                // 0x8000: Erkennung abgeschlossen
                //

#define M_DC        0x0100
#define M_ALT(ANZ)  0x0200+ANZ
#define M_ENDE      0x8000

                const UINT16 pLotus[] =         // Lotus 1/1A/2
                    { 0x0000, 0x0000, 0x0002, 0x0000,
                      M_ALT(2), 0x0004, 0x0006,
                      0x0004, M_ENDE };

                const UINT16 pExcel1[] =        // Excel Biff/3/4 Tabellen
                    { 0x0009,
                      M_ALT(2), 0x0002, 0x0004,
                      0x0006, 0x0000, M_DC, M_DC, 0x0010, 0x0000,
                      M_DC, M_DC, M_ENDE };

                const UINT16 pExcel2[] =        // Excel Biff3/4 Workbooks
                    { 0x0009,
                      M_ALT(2), 0x0002, 0x0004,
                      0x0006, 0x0000, M_DC, M_DC, 0x0000, 0x0001,
                      M_DC, M_DC, M_ENDE };

                const UINT16 pExcel3[] =        // Excel Biff2 Tabellen
                    { 0x0009, 0x0000, 0x0004, 0x0000,
                      M_DC, M_DC, 0x0010, 0x0000, M_ENDE };

                const UINT16 pSc10[] =          // StarCalc 1.0 Dokumente
                    { 'B', 'l', 'a', 'i', 's', 'e', '-', 'T', 'a', 'b', 'e', 'l', 'l',
                      'e', 0x000A, 0x000D, 0x0000,    // Sc10CopyRight[16]
                      M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC, M_DC,
                      M_DC, M_DC,                   // Sc10CopyRight[29]
                      M_ALT(2), 0x0065, 0x0066,     // Versionsnummer 101 oder 102
                      0x0000,
                      M_ENDE };

                const UINT16 pLotus2[] =        // Lotus >3
                    { 0x0000, 0x0000, 0x001A, 0x0000,   // Rec# + Len (26)
                      M_ALT(2), 0x0000, 0x0002,         // File Revision Code
                      0x0010,
                      0x0004, 0x0000,                   // File Revision Subcode
                      M_ENDE };

                const UINT16 pDIF1[] =          // DIF mit CR-LF
                    {
                    'T', 'A', 'B', 'L', 'E',
                    M_DC, M_DC,
                    '0', ',', '1',
                    M_DC, M_DC,
                    '\"',
                    M_ENDE };

                const UINT16 pDIF2[] =          // DIF mit CR oder LF
                    {
                    'T', 'A', 'B', 'L', 'E',
                    M_DC,
                    '0', ',', '1',
                    M_DC,
                    '\"',
                    M_ENDE };

                const UINT16 pSylk[] =          // Sylk
                    {
                    'I', 'D', ';', 'P',
                    M_ENDE };

#ifdef SINIX
                const UINT16 nAnzMuster = 9;    // sollte fuer indiz. Zugriff stimmen...
                UINT16 *ppMuster[ nAnzMuster ];         // Arrays mit Suchmustern
                ppMuster[ 0 ] = pLotus;
                ppMuster[ 1 ] = pExcel1;
                ppMuster[ 2 ] = pExcel2;
                ppMuster[ 3 ] = pExcel3;
                ppMuster[ 4 ] = pSc10;
                ppMuster[ 5 ] = pDIF1;
                ppMuster[ 6 ] = pDIF2;
                ppMuster[ 7 ] = pSylk;
                ppMuster[ 8 ] = pLotus2;                // Lotus immer ganz hinten wegen Ini-Eintrag
#else
                const UINT16 *ppMuster[] =      // Arrays mit Suchmustern
                    {
                    pLotus,
                    pExcel1,
                    pExcel2,
                    pExcel3,
                    pSc10,
                    pDIF1,
                    pDIF2,
                    pSylk,
                    pLotus2
                    };
                const UINT16 nAnzMuster = sizeof(ppMuster) / sizeof(ppMuster[0]);
#endif

                const sal_Char* pFilterName[ nAnzMuster ] =     // zugehoerige Filter
                    {
                    pFilterLotus,
                    pFilterExcel4,
                    pFilterExcel4,
                    pFilterExcel4,
                    pFilterSc10,
                    pFilterDif,
                    pFilterDif,
                    pFilterSylk,
                    pFilterLotus
                    };

                const UINT16 nByteMask = 0xFF;

                // suchen Sie jetzt!
                // ... realisiert ueber 'Mustererkennung'

                BYTE            nAkt;
                BOOL            bSync;          // Datei und Muster stimmen ueberein
                USHORT          nFilter;        // Zaehler ueber alle Filter
                const UINT16    *pSearch;       // aktuelles Musterwort
                UINT16          nFilterLimit = nAnzMuster;

                // nur solange, bis es etwas Globales gibt
                // funzt nur, solange Eintraege fuer WK3 letzte Muster-Tabelle ist!
//!MBA              //ScLibOptions aLibOpt;
                //if( !aLibOpt.GetWK3Flag() )
                //  nFilterLimit--;

                for ( nFilter = 0 ; nFilter < nFilterLimit ; nFilter++ )
                {
                    rStr.Seek( 0 ); // am Anfang war alles Uebel...
                    rStr >> nAkt;
                    pSearch = ppMuster[ nFilter ];
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
                            if ( pFilterName[nFilter] == pFilterExcel4 && pFilter &&
                                ( (pFilter)->GetFilterName().EqualsAscii(pFilterEx4Temp) || pFilter->GetTypeName().EqualsAscii("calc_MS_Excel_40") ) )
                            {
                                //  Excel 4 erkannt, Excel 4 Vorlage eingestellt -> auch gut
                                // oder Excel 4 Filter anderer Applikation (simulated type detection!)
                            }
                            else
                            {   // gefundenen Filter einstellen
                                pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterName[ nFilter ]) );
                            }
                        }
                        else
                        { //                                         Tabellenfehler
                            DBG_ERROR( "-ScApplication::DetectFilter(): Fehler in Mustertabelle");
                        }

                        pSearch++;
                        rStr >> nAkt;
                    }
                }

                // ASCII cannot be recognized.
                // #i3341# But if the Text/CSV filter was set (either by the user or
                // file extension) it takes precedence over HTML and RTF and dBase
                // detection. Otherwise something like, for example, "lala <SUP> gugu"
                // would trigger HTML to be recognized.

                if ( aPreselectedFilterName.EqualsAscii(pFilterAscii) && lcl_MayBeAscii( rStr ) )
                {
                    pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
                }
                else
                {
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

                    // test for HTML

                    if ( HTMLParser::IsHTMLFormat( aHeader.GetBuffer() ) )
                    {
                        if ( aPreselectedFilterName.EqualsAscii(pFilterHtml) )
                        {
                             pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
                        }
                        else
                        {
                            pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterHtmlWeb) );
                        }
                    }

                    // test for RTF

                    if ( aHeader.CompareTo( "{\\rtf", 5 ) == COMPARE_EQUAL )
                    {
                        pFilter = aMatcher.GetFilter4FilterName( String::CreateFromAscii(pFilterRtf) );
                    }

                    // #97832#; we don't have a flat xml filter
            /*      if ( aHeader.CompareTo( "<?xml", 5 ) == COMPARE_EQUAL )
                    {
                        //  if XML template is set, don't modify
                        if (!lcl_IsAnyXMLFilter(pFilter))
                            pFilter = SFX_APP()->GetFilter( ScDocShell::Factory(),
                                                              String::CreateFromAscii(pFilterXML) );
                        return ERRCODE_NONE;
                    }*/

                    // dBase cannot safely be recognized - only test if the filter was set
                    if ( aPreselectedFilterName.EqualsAscii(pFilterDBase) && lcl_MayBeDBase( rStr ) )
                        pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
                }
            }
        }
    }

    if ( pFilter )
    {
        aTypeName = pFilter->GetTypeName();
        aFilterName = pFilter->GetName();
    }

    if ( aFilterName.Len() )
    {
        // successful detection, get the filter name (without prefix)
        if ( nIndexOfFilterName != -1 )
            // convert to format with factory ( makes load more easy to implement )
            lDescriptor[nIndexOfFilterName].Value <<= ::rtl::OUString( aFilterName );
        else
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("FilterName");
            lDescriptor[nPropertyCount].Value <<= ::rtl::OUString( aFilterName );
            nPropertyCount++;
        }
/*
        if ( pFilter->IsOwnTemplateFormat() && nIndexOfTemplateFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("AsTemplate");
            lDescriptor[nPropertyCount].Value <<= sal_True;
            nPropertyCount++;
        }
*/
    }
    else
    {
        aFilterName.Erase();
        aTypeName.Erase();
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
UNOSEQUENCE< UNOOUSTRING > SAL_CALL ScFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > ScFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.ExtendedTypeDetection"  );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING ScFilterDetect::impl_getStaticImplementationName()
{
    return UNOOUSTRING::createFromAscii( "com.sun.star.comp.calc.FormatDetector" );
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL ScFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new ScFilterDetect( xServiceManager ) );
}

