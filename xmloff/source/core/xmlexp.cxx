/*************************************************************************
 *
 *  $RCSfile: xmlexp.cxx,v $
 *
 *  $Revision: 1.77 $
 *
 *  last change: $Author: dvo $ $Date: 2001-08-03 16:24:00 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <rtl/uuid.h>
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XBINARYSTREAMRESOLVER_HPP_
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLNUMFE_HXX
#include "xmlnumfe.hxx"
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include "xmlmetae.hxx"
#endif

#ifndef _XMLOFF_XMLBASICE_HXX
#include "xmlscripte.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#include "XMLEventExport.hxx"
#endif

#ifndef _XMLOFF_XMLSTARBASICEXPORTHANDLER_HXX
#include "XMLStarBasicExportHandler.hxx"
#endif

#ifndef _XMLOFF_SETTINGSEXPORTHELPER_HXX
#include "SettingsExportHelper.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XVIEWDATASUPPLIER_HPP_
#include <com/sun/star/document/XViewDataSupplier.hpp>
#endif

#ifndef _XMLOFF_GRADIENTSTYLE_HXX
#include <GradientStyle.hxx>
#endif
#ifndef _XMLOFF_HATCHSTYLE_HXX
#include <HatchStyle.hxx>
#endif
#ifndef _XMLOFF_IMAGESTYLE_HXX
#include <ImageStyle.hxx>
#endif
#ifndef _XMLOFF_TRANSGRADIENTSTYLE_HXX
#include <TransGradientStyle.hxx>
#endif
#ifndef _XMLOFF_MARKERSTYLE_HXX
#include <MarkerStyle.hxx>
#endif
#ifndef _XMLOFF_DASHSTYLE_HXX
#include <DashStyle.hxx>
#endif
#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#include "XMLFontAutoStylePool"
#endif
#ifndef _XMLOFF_XMLIMAGEMAPEXPORT_HXX_
#include "XMLImageMapExport.hxx"
#endif
#ifndef _XMLOFF_XMLBASE64EXPORT_HXX_
#include "XMLBase64Export.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _XMLOFF_XMLFILTERSERVICENAMES_H
#include "XMLFilterServiceNames.h"
#endif
#ifndef _XMLOFF_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX
#include "XMLEmbeddedObjectExportFilter.hxx"
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;
using namespace ::xmloff::token;

sal_Char __READONLY_DATA sXML_1_0[] = "1.0";

#define LOGFILE_AUTHOR "mb93740"

#define XML_MODEL_SERVICE_WRITER    "com.sun.star.text.TextDocument"
#define XML_MODEL_SERVICE_CALC      "com.sun.star.sheet.SpreadsheetDocument"
#define XML_MODEL_SERVICE_DRAW      "com.sun.star.drawing.DrawingDocument"
#define XML_MODEL_SERVICE_IMPRESS   "com.sun.star.presentation.PresentationDocument"
#define XML_MODEL_SERVICE_MATH      "com.sun.star.formula.FormulaProperties"
#define XML_MODEL_SERVICE_CHART     "com.sun.star.chart.ChartDocument"

struct XMLServiceMapEntry_Impl
{
    const sal_Char *sModelService;
    sal_Int32      nModelServiceLen;
    const sal_Char *sFilterService;
    sal_Int32      nFilterServiceLen;
};

#define SERVICE_MAP_ENTRY( app ) \
    { XML_MODEL_SERVICE_##app, sizeof(XML_MODEL_SERVICE_##app)-1, \
      XML_EXPORT_FILTER_##app, sizeof(XML_EXPORT_FILTER_##app)-1 }

const XMLServiceMapEntry_Impl aServiceMap[] =
{
    SERVICE_MAP_ENTRY( WRITER ),
    SERVICE_MAP_ENTRY( CALC ),
    SERVICE_MAP_ENTRY( IMPRESS ),// Impress supports DrawingDocument, too, so
    SERVICE_MAP_ENTRY( DRAW ),   // it must appear before Draw
    SERVICE_MAP_ENTRY( MATH ),
    SERVICE_MAP_ENTRY( CHART ),
    { 0, 0, 0, 0 }
};

void SvXMLExport::_InitCtor()
{
    pNamespaceMap->Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO), XML_NAMESPACE_FO );
    }
    if( (getExportFlags() & (EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( (getExportFlags() & EXPORT_SETTINGS) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( (getExportFlags() & EXPORT_META) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_DC), GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
        pNamespaceMap->Add( GetXMLToken(XML_NP_META), GetXMLToken(XML_N_META), XML_NAMESPACE_META );
    }
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }

    // namespaces for documents
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_TEXT), GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT );
        pNamespaceMap->Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
        pNamespaceMap->Add( GetXMLToken(XML_NP_DR3D), GetXMLToken(XML_N_DR3D), XML_NAMESPACE_DR3D );
        pNamespaceMap->Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG),  XML_NAMESPACE_SVG );
        pNamespaceMap->Add( GetXMLToken(XML_NP_CHART), GetXMLToken(XML_N_CHART), XML_NAMESPACE_CHART );
        pNamespaceMap->Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
        pNamespaceMap->Add( GetXMLToken(XML_NP_NUMBER), GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );
    }
    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_MATH), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH );
        pNamespaceMap->Add( GetXMLToken(XML_NP_FORM), GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
    }
    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS) ) != 0 )
    {
        pNamespaceMap->Add( GetXMLToken(XML_NP_SCRIPT), GetXMLToken(XML_N_SCRIPT), XML_NAMESPACE_SCRIPT );
    }


    xAttrList = (xml::sax::XAttributeList*)pAttrList;

    sPicturesPath = OUString( RTL_CONSTASCII_USTRINGPARAM( "#Pictures/" ) );
    sObjectsPath = OUString( RTL_CONSTASCII_USTRINGPARAM( "#./" ) );
    sGraphicObjectProtocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
    sEmbeddedObjectProtocol = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ) );
}

SvXMLExport::SvXMLExport( MapUnit eDfltUnit, const enum XMLTokenEnum eClass, sal_uInt16 nExportFlags ) :
    pImpl( 0 ), meClass( eClass ),
    sCDATA( GetXMLToken(XML_CDATA) ),
    sWS( GetXMLToken(XML_WS) ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, eDfltUnit ) ),
    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    pNumExport(0L),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( nExportFlags )
{
    _InitCtor();
}

SvXMLExport::SvXMLExport(
        const OUString &rFileName,
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        MapUnit eDfltUnit   ) :
    pImpl( 0 ), meClass( XML_TOKEN_INVALID ),
    sCDATA( GetXMLToken(XML_CDATA) ),
    sWS( GetXMLToken(XML_WS) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, eDfltUnit ) ),
    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY ),
    pNumExport(0L),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( EXPORT_ALL )
{
    _InitCtor();

    if (xNumberFormatsSupplier.is())
        pNumExport = new SvXMLNumFmtExport(rHandler, xNumberFormatsSupplier);
}

SvXMLExport::SvXMLExport(
        const OUString &rFileName,
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const Reference< XModel >& rModel,
        sal_Int16 eDfltUnit ) :
    pImpl( 0 ), meClass( XML_TOKEN_INVALID ),
    sCDATA( GetXMLToken(XML_CDATA) ),
    sWS( GetXMLToken(XML_WS) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, SvXMLUnitConverter::GetMapUnit(eDfltUnit) ) ),
    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY ),
    xModel( rModel ),
    pNumExport(0L),
    xNumberFormatsSupplier (rModel, uno::UNO_QUERY),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( EXPORT_ALL )
{
    _InitCtor();

    if (xNumberFormatsSupplier.is())
        pNumExport = new SvXMLNumFmtExport(rHandler, xNumberFormatsSupplier);
}

SvXMLExport::SvXMLExport(
        const OUString &rFileName,
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const Reference< XModel >& rModel,
        const Reference< document::XGraphicObjectResolver >& rEmbeddedGraphicObjects,
        sal_Int16 eDfltUnit ) :
    pImpl( 0 ), meClass( XML_TOKEN_INVALID ),
    sCDATA( GetXMLToken(XML_CDATA) ),
    sWS( GetXMLToken(XML_WS) ),
    sOrigFileName( rFileName ),
    pNamespaceMap( new SvXMLNamespaceMap ),
    pUnitConv( new SvXMLUnitConverter( MAP_100TH_MM, SvXMLUnitConverter::GetMapUnit(eDfltUnit) ) ),
    pAttrList( new SvXMLAttributeList ),
    bExtended( sal_False ),
    xHandler( rHandler ),
    xExtHandler( rHandler, uno::UNO_QUERY ),
    xGraphicResolver( rEmbeddedGraphicObjects ),
    xModel( rModel ),
    pNumExport(0L),
    xNumberFormatsSupplier (rModel, uno::UNO_QUERY),
    pProgressBarHelper( NULL ),
    pEventExport( NULL ),
    pImageMapExport( NULL ),
    bSaveLinkedSections(sal_True),
    mnExportFlags( EXPORT_ALL )
{
    _InitCtor();

    if (xNumberFormatsSupplier.is())
        pNumExport = new SvXMLNumFmtExport(rHandler, xNumberFormatsSupplier);
}

SvXMLExport::~SvXMLExport()
{
    delete pImageMapExport;
    delete pEventExport;
    delete pNamespaceMap;
    delete pUnitConv;
    if (pProgressBarHelper || pNumExport)
    {
        if (xExportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xExportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                if (pProgressBarHelper)
                {
                    OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSMAX));
                    OUString sProgressCurrent(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSCURRENT));
                    if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                        xPropertySetInfo->hasPropertyByName(sProgressCurrent))
                    {
                        sal_Int32 nProgressMax(pProgressBarHelper->GetReference());
                        sal_Int32 nProgressCurrent(pProgressBarHelper->GetValue());
                        uno::Any aAny;
                        aAny <<= nProgressMax;
                        xExportInfo->setPropertyValue(sProgressMax, aAny);
                        aAny <<= nProgressCurrent;
                        xExportInfo->setPropertyValue(sProgressCurrent, aAny);
                    }
                }
                if (pNumExport && (mnExportFlags & (EXPORT_AUTOSTYLES | EXPORT_STYLES)))
                {
                    OUString sWrittenNumberFormats(RTL_CONSTASCII_USTRINGPARAM(XML_WRITTENNUMBERSTYLES));
                    if (xPropertySetInfo->hasPropertyByName(sWrittenNumberFormats))
                    {
                        uno::Sequence<sal_Int32> aWasUsed;
                        pNumExport->GetWasUsed(aWasUsed);
                        uno::Any aAny;
                        aAny <<= aWasUsed;
                        xExportInfo->setPropertyValue(sWrittenNumberFormats, aAny);
                    }
                }
            }
        }
        delete pProgressBarHelper;
        delete pNumExport;
    }

    xmloff::token::ResetTokens();
}

///////////////////////////////////////////////////////////////////////

// XExporter
void SAL_CALL SvXMLExport::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    xModel = uno::Reference< frame::XModel >::query( xDoc );
    if( !xModel.is() )
        throw lang::IllegalArgumentException();

    if(!xNumberFormatsSupplier.is() )
    {
        xNumberFormatsSupplier = xNumberFormatsSupplier.query( xModel );
        if(xNumberFormatsSupplier.is() && xHandler.is())
            pNumExport = new SvXMLNumFmtExport(xHandler, xNumberFormatsSupplier);
    }
    if (xExportInfo.is() && pNumExport && (mnExportFlags & (EXPORT_AUTOSTYLES | EXPORT_STYLES)))
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xExportInfo->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            OUString sWrittenNumberFormats(RTL_CONSTASCII_USTRINGPARAM(XML_WRITTENNUMBERSTYLES));
            if (xPropertySetInfo->hasPropertyByName(sWrittenNumberFormats))
            {
                uno::Any aAny = xExportInfo->getPropertyValue(sWrittenNumberFormats);
                uno::Sequence<sal_Int32> aWasUsed;
                if(aAny >>= aWasUsed)
                    pNumExport->SetWasUsed(aWasUsed);
            }
        }
    }
}

// XInitialize
void SAL_CALL SvXMLExport::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    const sal_Int32 nAnyCount = aArguments.getLength();
    const uno::Any* pAny = aArguments.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
    {
        if( pAny->getValueType() == ::getCppuType((const uno::Reference< task::XStatusIndicator >*)0) )
        {
            *pAny >>= xStatusIndicator;
        }
        else if( pAny->getValueType() == ::getCppuType((const uno::Reference< document::XGraphicObjectResolver >*)0) )
        {
            *pAny >>= xGraphicResolver;
        }
        else if( pAny->getValueType() == ::getCppuType((const uno::Reference< document::XEmbeddedObjectResolver >*)0) )
        {
            *pAny >>= xEmbeddedResolver;
        }
        else if( pAny->getValueType() == ::getCppuType((const uno::Reference< xml::sax::XDocumentHandler >*)0))
        {
            *pAny >>= xHandler;
            *pAny >>= xExtHandler;

            if (xNumberFormatsSupplier.is() && pNumExport == NULL)
                pNumExport = new SvXMLNumFmtExport(xHandler, xNumberFormatsSupplier);
        }
        else if( pAny->getValueType() == ::getCppuType((const uno::Reference< beans::XPropertySet >*)0))
        {
            *pAny >>= xExportInfo;
        }
    }
}

// XFilter
sal_Bool SAL_CALL SvXMLExport::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor ) throw(uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogContext, "xmloff", LOGFILE_AUTHOR,
                                "SvXMLExport::filter" );

    // check for xHandler first... should have been supplied in initialize
    if( !xHandler.is() )
        return sal_False;

    try
    {
        const sal_Int32 nPropCount = aDescriptor.getLength();
        const beans::PropertyValue* pProps = aDescriptor.getConstArray();

        for( sal_Int32 nIndex = 0; nIndex < nPropCount; nIndex++, pProps++ )
        {
            const OUString& rPropName = pProps->Name;
            const Any& rValue = pProps->Value;

            if( rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FileName" ) ) )
            {
                if( !(rValue >>= sOrigFileName ) )
                    return sal_False;
            }
        }

#ifdef TIMELOG
        // print a trace message with the URL
        ByteString aUrl( (String) GetModel()->getURL(),
                         RTL_TEXTENCODING_ASCII_US );
        RTL_LOGFILE_CONTEXT_TRACE1( aLogContext, "%s", aUrl.GetBuffer() );

        // we also want a trace message with the document class
        ByteString aClass( (String)GetXMLToken(meClass),
                           RTL_TEXTENCODING_ASCII_US );
        RTL_LOGFILE_CONTEXT_TRACE1( aLogContext, "class=\"%s\"",
                                    aClass.GetBuffer() );
#endif

        exportDoc( meClass );

        return sal_True;
    }
    catch( uno::Exception e )
    {
        // We must catch exceptions, because according to the
        // API definition export must not throw one!
#ifdef DEBUG
        ByteString aError( "uno Exception catched while exporting:\n" );
        aError += ByteString( String( e.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
    }

    return sal_False;
}

void SAL_CALL SvXMLExport::cancel() throw(uno::RuntimeException)
{
    DBG_ERROR( "not supported" );
}

// XServiceInfo
OUString SAL_CALL SvXMLExport::getImplementationName(  ) throw(uno::RuntimeException)
{
    OUString aStr;
    return aStr;
}

sal_Bool SAL_CALL SvXMLExport::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return sal_False;
}

uno::Sequence< OUString > SAL_CALL SvXMLExport::getSupportedServiceNames(  )
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq;
    return aSeq;
}

///////////////////////////////////////////////////////////////////////

void SvXMLExport::AddAttributeASCII( sal_uInt16 nPrefixKey,
                                     const sal_Char *pName,
                                       const sal_Char *pValue )
{
    OUString sName( OUString::createFromAscii( pName ) );
    OUString sValue( OUString::createFromAscii( pValue ) );

    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey,
                                                           sName ),
                             sCDATA, sValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const sal_Char *pName,
                              const OUString& rValue )
{
    OUString sName( OUString::createFromAscii( pName ) );

    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey,
                                                           sName ),
                             sCDATA, rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey, const OUString& rName,
                              const OUString& rValue )
{
    pAttrList->AddAttribute( pNamespaceMap->GetQNameByKey( nPrefixKey, rName ),
                             sCDATA, rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey,
                                enum XMLTokenEnum eName,
                                const OUString& rValue )
{
    pAttrList->AddAttribute(
        pNamespaceMap->GetQNameByKey( nPrefixKey, GetXMLToken(eName) ),
        sCDATA, rValue );
}

void SvXMLExport::AddAttribute( sal_uInt16 nPrefixKey,
                                enum XMLTokenEnum eName,
                                enum XMLTokenEnum eValue)
{
    pAttrList->AddAttribute(
        pNamespaceMap->GetQNameByKey( nPrefixKey, GetXMLToken(eName) ),
        sCDATA, GetXMLToken(eValue) );
}

void SvXMLExport::AddAttributeList( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    if( xAttrList.is())
        pAttrList->AppendAttributeList( xAttrList );
}

void SvXMLExport::ClearAttrList()
{
    pAttrList->Clear();
}

#ifndef PRODUCT
void SvXMLExport::CheckAttrList()
{
    DBG_ASSERT( !pAttrList->getLength(),
                "XMLExport::CheckAttrList: list is not empty" );
}
#endif

void SvXMLExport::ImplExportMeta()
{
    CheckAttrList();

    {
        // <office:meta>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_META,
                                sal_True, sal_True );

        _ExportMeta();
    }
}

void SvXMLExport::ImplExportSettings()
{
    CheckAttrList();

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_SETTINGS,
                                sal_True, sal_True );
        XMLSettingsExportHelper aSettingsExportHelper(*this);
        _ExportViewSettings(aSettingsExportHelper);
        _ExportConfigurationSettings(aSettingsExportHelper);
    }
}

void SvXMLExport::ImplExportStyles( sal_Bool bUsed )
{
    CheckAttrList();

//  AddAttribute( XML_NAMESPACE_NONE, XML_ID, XML_STYLES_ID );
    {
        // <style:styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_STYLES,
                                sal_True, sal_True );

        _ExportStyles( sal_False );
    }
}

void SvXMLExport::ImplExportAutoStyles( sal_Bool bUsed )
{
//  AddAttributeASCII( XML_NAMESPACE_NONE, XML_ID, XML_AUTO_STYLES_ID );
    {
        // <style:automatic-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                  XML_AUTOMATIC_STYLES, sal_True, sal_True );

#if 0
        AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, XML_STYLES_HREF );
        AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
        AddAttribute( XML_NAMESPACE_XLINK, XML_ROLE,
                         pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                                               GetXMLToken(XML_STYLESHEET)) );
        {
            // <style:use-styles>
            SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                      XML_USE_STYLES, sal_True, sal_True );
        }
#endif
        _ExportAutoStyles();
    }
}

void SvXMLExport::ImplExportMasterStyles( sal_Bool bUsed )
{
    {
        // <style:master-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_MASTER_STYLES,
                                sal_True, sal_True );

        _ExportMasterStyles();
    }

#if 0
    AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, XML_AUTO_STYLES_HREF );
    AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
    AddAttribute( XML_NAMESPACE_XLINK, XML_ROLE,
                  pNamespaceMap->GetQNameByKey( XML_NAMESPACE_OFFICE,
                                                GetXMLToken(XML_STYLESHEET) ) );
    {
        // <style:use-styles>
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE,
                                  XML_USE_STYLES, sal_True, sal_True );
    }
#endif
}

void SvXMLExport::ImplExportContent()
{
    CheckAttrList();

    CheckAttrList();

    {
        // <office:body ...>
        SetBodyAttributes();
        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_BODY,
                                  sal_True, sal_True );

        _ExportContent();
    }
}

void SvXMLExport::SetBodyAttributes()
{
}

sal_uInt32 SvXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum eClass )
{
    xHandler->startDocument();

    // <?xml version="1.0" encoding="UTF-8"?>
//  xHandler->processingInstruction( S2U( sXML_xml ), S2U( sXML_xml_pi ) );

    // <office:document ...>
    CheckAttrList();

    // namespace attributes
    // ( The namespace decls should be first attributes in the element;
    //   some faulty XML parsers (JAXP1.1) have a problem with this,
    //   also it's more elegant )
    sal_uInt16 nPos = pNamespaceMap->GetFirstKey();
    while( USHRT_MAX != nPos )
    {
        pAttrList->AddAttribute( pNamespaceMap->GetAttrNameByKey( nPos ),
                                 sCDATA,
                                 pNamespaceMap->GetNameByKey( nPos ) );
        nPos = pNamespaceMap->GetNextKey( nPos );
    }

    // office:class = ... (only for stream containing the content)
    if( (eClass != XML_TOKEN_INVALID) && ((mnExportFlags & EXPORT_CONTENT) != 0) )
        AddAttribute( XML_NAMESPACE_OFFICE, XML_CLASS, eClass );

    // office:version = ...
    if( !bExtended )
        AddAttribute( XML_NAMESPACE_OFFICE, XML_VERSION,
                      OUString::createFromAscii(sXML_1_0) );

    {
        enum XMLTokenEnum eRootService = XML_TOKEN_INVALID;
        const sal_Int32 nExportMode = mnExportFlags & (EXPORT_META|EXPORT_STYLES|EXPORT_CONTENT|EXPORT_SETTINGS);
        if( EXPORT_META == nExportMode )
        {
            // export only meta
            eRootService = XML_DOCUMENT_META;
        }
        else if ( EXPORT_SETTINGS == nExportMode )
        {
            // export only settings
            eRootService = XML_DOCUMENT_SETTINGS;
        }
        else if( EXPORT_STYLES == nExportMode )
        {
            // export only styles
            eRootService = XML_DOCUMENT_STYLES;
        }
        else if( EXPORT_CONTENT == nExportMode )
        {
            // export only content
            eRootService = XML_DOCUMENT_CONTENT;
        }
        else
        {
            // the god'ol one4all element
            eRootService = XML_DOCUMENT;
        }

        if( (getExportFlags() & EXPORT_NODOCTYPE) == 0 &&
            xExtHandler.is() )
        {
            OUStringBuffer aDocType(
                GetXMLToken(XML_XML_DOCTYPE_PREFIX).getLength() +
                GetXMLToken(XML_XML_DOCTYPE_SUFFIX).getLength() + 30 );

            aDocType.append( GetXMLToken(XML_XML_DOCTYPE_PREFIX) );
            aDocType.append( GetNamespaceMap().GetQNameByKey(
                           XML_NAMESPACE_OFFICE, GetXMLToken(eRootService) ) );
            aDocType.append( GetXMLToken(XML_XML_DOCTYPE_SUFFIX) );
            xExtHandler->unknown( aDocType.makeStringAndClear() );
        }

        SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, eRootService, sal_True, sal_True );

        // meta information
        if( mnExportFlags & EXPORT_META )
            ImplExportMeta();

        // settings
        if( mnExportFlags & EXPORT_SETTINGS )
            ImplExportSettings();

        // scripts
        if( mnExportFlags & EXPORT_SCRIPTS )
            _ExportScripts();

        // font declerations
        if( mnExportFlags & EXPORT_FONTDECLS )
            _ExportFontDecls();

        // styles
        if( mnExportFlags & EXPORT_STYLES )
            ImplExportStyles( sal_False );

        // autostyles
        if( mnExportFlags & EXPORT_AUTOSTYLES )
            ImplExportAutoStyles( sal_False );

        // masterstyles
        if( mnExportFlags & EXPORT_MASTERSTYLES )
            ImplExportMasterStyles( sal_False );

        // contnt
        if( mnExportFlags & EXPORT_CONTENT )
            ImplExportContent();
    }


    xHandler->endDocument();

    return 0;
}

void SvXMLExport::_ExportMeta()
{
    SfxXMLMetaExport aMeta( GetDocHandler(), xModel );
    aMeta.Export( GetNamespaceMap() );
}

void SvXMLExport::_ExportViewSettings(const XMLSettingsExportHelper& rSettingsExportHelper)
{
    uno::Sequence<beans::PropertyValue> aProps;
    GetViewSettings(aProps);
    uno::Reference<document::XViewDataSupplier> xViewDataSupplier(GetModel(), uno::UNO_QUERY);
    if(xViewDataSupplier.is())
    {
        uno::Reference<container::XIndexAccess> xIndexAccess;
        xViewDataSupplier->setViewData( xIndexAccess ); // make sure we get a newly created sequence
        xIndexAccess = xViewDataSupplier->getViewData();
        if(xIndexAccess.is())
        {
            sal_Int32 nOldLength(aProps.getLength());
            aProps.realloc(nOldLength + 1);
            beans::PropertyValue aProp;
            aProp.Name = OUString(RTL_CONSTASCII_USTRINGPARAM("Views"));
            aProp.Value <<= xIndexAccess;
            aProps[nOldLength] = aProp;
        }
    }
    OUString sViewSettings(GetXMLToken(XML_VIEW_SETTINGS));
    rSettingsExportHelper.exportSettings(aProps, sViewSettings);
}

void SvXMLExport::_ExportConfigurationSettings(const XMLSettingsExportHelper& rSettingsExportHelper)
{
    uno::Sequence<beans::PropertyValue> aProps;
    GetConfigurationSettings(aProps);
    OUString sConfigurationSettings(GetXMLToken(XML_CONFIGURATION_SETTINGS));
    rSettingsExportHelper.exportSettings(aProps, sConfigurationSettings);
}

void SvXMLExport::_ExportScripts()
{
    // <office:script>
    SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_SCRIPT,
                            sal_True, sal_True );

    XMLBasicExport aBasicExp( *this );
    aBasicExp.Export();

    // export document events
    Reference<document::XEventsSupplier> xEvents(GetModel(), UNO_QUERY);
    GetEventExport().Export(xEvents, sal_True);
}

void SvXMLExport::_ExportFontDecls()
{
    if( mxFontAutoStylePool.is() )
        mxFontAutoStylePool->exportXML();
}

void SvXMLExport::_ExportStyles( sal_Bool bUsed )
{
    uno::Reference< lang::XMultiServiceFactory > xFact( GetModel(), uno::UNO_QUERY );
    if( xFact.is())
    {
        // export (fill-)gradient-styles
        try
        {
            uno::Reference< container::XNameAccess > xGradient( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GradientTable") ) ), uno::UNO_QUERY );
            if( xGradient.is() )
            {
                XMLGradientStyle aGradientStyle( &xHandler, *pNamespaceMap, *pUnitConv );

                if( xGradient->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xGradient->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xGradient->getByName( rStrName );

                            aGradientStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export (fill-)hatch-styles
        try
        {
            uno::Reference< container::XNameAccess > xHatch( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.HatchTable") ) ), uno::UNO_QUERY );
            if( xHatch.is() )
            {
                XMLHatchStyle aHatchStyle( xHandler, *pNamespaceMap, *pUnitConv );

                if( xHatch->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xHatch->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xHatch->getByName( rStrName );

                            aHatchStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export (fill-)bitmap-styles
        try
        {
            uno::Reference< container::XNameAccess > xBitmap( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.BitmapTable") ) ), uno::UNO_QUERY );
            if( xBitmap.is() )
            {
                XMLImageStyle aImageStyle;

                if( xBitmap->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xBitmap->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xBitmap->getByName( rStrName );

                            aImageStyle.exportXML( rStrName, aValue, *this );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export transparency-gradient -styles
        try
        {
            uno::Reference< container::XNameAccess > xTransGradient( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.TransparencyGradientTable") ) ), uno::UNO_QUERY );
            if( xTransGradient.is() )
            {
                XMLTransGradientStyle aTransGradientstyle( xHandler, *pNamespaceMap, *pUnitConv );

                if( xTransGradient->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xTransGradient->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xTransGradient->getByName( rStrName );

                            aTransGradientstyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export marker-styles
        try
        {
            uno::Reference< container::XNameAccess > xMarker( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.MarkerTable") ) ), uno::UNO_QUERY );
            if( xMarker.is() )
            {
                XMLMarkerStyle aMarkerStyle( xHandler, *pNamespaceMap, *pUnitConv );

                if( xMarker->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xMarker->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xMarker->getByName( rStrName );

                            aMarkerStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}

        // export dash-styles
        try
        {
            uno::Reference< container::XNameAccess > xDashes( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DashTable") ) ), uno::UNO_QUERY );
            if( xDashes.is() )
            {
                XMLDashStyle aDashStyle( &xHandler, *pNamespaceMap, *pUnitConv );

                if( xDashes->hasElements() )
                {
                    uno::Sequence< OUString > aNamesSeq ( xDashes->getElementNames() );
                    sal_Int32 nCount = aNamesSeq.getLength();
                    for( sal_Int32 i=0; i<nCount; i++ )
                    {
                        const OUString& rStrName = aNamesSeq[ i ];

                        try
                        {
                            uno::Any aValue = xDashes->getByName( rStrName );

                            aDashStyle.exportXML( rStrName, aValue );
                        }
                        catch( container::NoSuchElementException& )
                        {}
                    }
                }
            }
        }
        catch( lang::ServiceNotRegisteredException& )
        {}
    }
}

XMLTextParagraphExport* SvXMLExport::CreateTextParagraphExport()
{
    return new XMLTextParagraphExport( *this, *(GetAutoStylePool().get()) );
}

XMLShapeExport* SvXMLExport::CreateShapeExport()
{
    return new XMLShapeExport(*this);
}

SvXMLAutoStylePoolP* SvXMLExport::CreateAutoStylePool()
{
    return new SvXMLAutoStylePoolP();
}

XMLPageExport* SvXMLExport::CreatePageExport()
{
    return new XMLPageExport( *this );
}

SchXMLExportHelper* SvXMLExport::CreateChartExport()
{
    return new SchXMLExportHelper(*this,*GetAutoStylePool().get());
}

XMLFontAutoStylePool* SvXMLExport::CreateFontAutoStylePool()
{
    return new XMLFontAutoStylePool( *this );
}

xmloff::OFormLayerXMLExport* SvXMLExport::CreateFormExport()
{
    return new xmloff::OFormLayerXMLExport(*this);
}

void SvXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& aProps)
{
}

void SvXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>& aProps)
{
}

void SvXMLExport::addDataStyle(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat )
{
    if(pNumExport)
        pNumExport->SetUsed(nNumberFormat);
}

void SvXMLExport::exportDataStyles()
{
    if(pNumExport)
        pNumExport->Export(*pNamespaceMap, sal_False);
}

void SvXMLExport::exportAutoDataStyles()
{
    if(pNumExport)
        pNumExport->Export(*pNamespaceMap, sal_True);

    if (mxFormExport.is())
        mxFormExport->exportAutoControlNumberStyles();
}

OUString SvXMLExport::getDataStyleName(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat ) const
{
    OUString sTemp;
    if(pNumExport)
        sTemp = pNumExport->GetStyleName(nNumberFormat);
    return sTemp;
}

OUString SvXMLExport::AddEmbeddedGraphicObject( const OUString& rGraphicObjectURL )
{
    OUString sRet( rGraphicObjectURL );
    if( 0 == rGraphicObjectURL.compareTo( sGraphicObjectProtocol,
                                          sGraphicObjectProtocol.getLength() ) &&
        xGraphicResolver.is() )
    {
        if( (getExportFlags() & EXPORT_EMBEDDED) == 0 )
            sRet = xGraphicResolver->resolveGraphicObjectURL( rGraphicObjectURL );
        else
            sRet = OUString();
    }
    else
        sRet = INetURLObject::AbsToRel( sRet );

    return sRet;
}

sal_Bool SvXMLExport::AddEmbeddedGraphicObjectAsBase64( const OUString& rGraphicObjectURL )
{
    sal_Bool bRet = sal_False;

    if( (getExportFlags() & EXPORT_EMBEDDED) != 0 &&
        0 == rGraphicObjectURL.compareTo( sGraphicObjectProtocol,
                                          sGraphicObjectProtocol.getLength() ) &&
        xGraphicResolver.is() )
    {
        Reference< XBinaryStreamResolver > xStmResolver( xGraphicResolver, UNO_QUERY );

        if( xStmResolver.is() )
        {
            Reference< XInputStream > xIn( xStmResolver->getInputStream( rGraphicObjectURL ) );

            if( xIn.is() )
            {
                XMLBase64Export aBase64Exp( *this );
                bRet = aBase64Exp.exportOfficeBinaryDataElement( xIn );
            }
        }
    }

    return bRet;
}

OUString SvXMLExport::AddEmbeddedObject( const OUString& rEmbeddedObjectURL )
{
    OUString sRet;
    if( 0 == rEmbeddedObjectURL.compareTo( sEmbeddedObjectProtocol,
                sEmbeddedObjectProtocol.getLength() ) &&
        xEmbeddedResolver.is() )
    {
        sRet =
            xEmbeddedResolver->resolveEmbeddedObjectURL( rEmbeddedObjectURL );
    }

    return sRet;
}

sal_Bool SvXMLExport::AddEmbeddedObjectAsBase64( const OUString& rEmbeddedObjectURL )
{
    sal_Bool bRet = sal_False;
    if( 0 == rEmbeddedObjectURL.compareTo( sEmbeddedObjectProtocol,
                sEmbeddedObjectProtocol.getLength() ) &&
        xEmbeddedResolver.is() )
    {
        Reference < XNameAccess > xNA( xEmbeddedResolver, UNO_QUERY );
        if( xNA.is() )
        {
            Any aAny = xNA->getByName( rEmbeddedObjectURL );
            Reference < XInputStream > xIn;
            aAny >>= xIn;
            if( xIn.is() )
            {
                XMLBase64Export aBase64Exp( *this );
                bRet = aBase64Exp.exportOfficeBinaryDataElement( xIn );
            }
        }
    }

    return bRet;
}

ProgressBarHelper*  SvXMLExport::GetProgressBarHelper()
{
    if (!pProgressBarHelper)
    {
        pProgressBarHelper = new ProgressBarHelper(xStatusIndicator, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "XML Export" )));

        if (pProgressBarHelper && xExportInfo.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xExportInfo->getPropertySetInfo();
            if (xPropertySetInfo.is())
            {
                OUString sProgressRange(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSRANGE));
                OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSMAX));
                OUString sProgressCurrent(RTL_CONSTASCII_USTRINGPARAM(XML_PROGRESSCURRENT));
                if (xPropertySetInfo->hasPropertyByName(sProgressMax) &&
                    xPropertySetInfo->hasPropertyByName(sProgressCurrent) &&
                    xPropertySetInfo->hasPropertyByName(sProgressRange))
                {
                    uno::Any aAny;
                    sal_Int32 nProgressMax(0);
                    sal_Int32 nProgressCurrent(0);
                    sal_Int32 nProgressRange(0);
                    aAny = xExportInfo->getPropertyValue(sProgressRange);
                    if (aAny >>= nProgressRange)
                        pProgressBarHelper->SetRange(nProgressRange);
                    aAny = xExportInfo->getPropertyValue(sProgressMax);
                    if (aAny >>= nProgressMax)
                        pProgressBarHelper->SetReference(nProgressMax);
                    aAny = xExportInfo->getPropertyValue(sProgressCurrent);
                    if (aAny >>= nProgressCurrent)
                        pProgressBarHelper->SetValue(nProgressCurrent);
                }
            }
        }
    }
    return pProgressBarHelper;
}

XMLEventExport& SvXMLExport::GetEventExport()
{
    if( NULL == pEventExport)
    {
        // create EventExport on demand
        pEventExport = new XMLEventExport(*this, NULL);

        // and register standard handlers + names
        OUString sStarBasic(RTL_CONSTASCII_USTRINGPARAM("StarBasic"));
        pEventExport->AddHandler(sStarBasic, new XMLStarBasicExportHandler());
        pEventExport->AddTranslationTable(aStandardEventTable);
    }

    return *pEventExport;
}

XMLImageMapExport& SvXMLExport::GetImageMapExport()
{
    // image map export, create on-demand
    if( NULL == pImageMapExport )
    {
        pImageMapExport = new XMLImageMapExport(*this);
    }

    return *pImageMapExport;
}

// XUnoTunnel & co
const uno::Sequence< sal_Int8 > & SvXMLExport::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

SvXMLExport* SvXMLExport::getImplementation( uno::Reference< uno::XInterface > xInt ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, uno::UNO_QUERY );
    if( xUT.is() )
        return (SvXMLExport*)xUT->getSomething( SvXMLExport::getUnoTunnelId() );
    else
        return NULL;
}

// XUnoTunnel
sal_Int64 SAL_CALL SvXMLExport::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw( uno::RuntimeException )
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

sal_Bool SvXMLExport::ExportEmbeddedOwnObject( Reference< XComponent >& rComp )
{
    OUString sFilterService;

    Reference < lang::XServiceInfo > xServiceInfo( rComp, UNO_QUERY );
    if( xServiceInfo.is() )
    {
        const XMLServiceMapEntry_Impl *pEntry = aServiceMap;
        while( pEntry->sModelService )
        {
            OUString sModelService( pEntry->sModelService,
                                    pEntry->nModelServiceLen,
                                       RTL_TEXTENCODING_ASCII_US );
            if( xServiceInfo->supportsService( sModelService ) )
            {
                sFilterService = OUString( pEntry->sFilterService,
                                           pEntry->nFilterServiceLen,
                                              RTL_TEXTENCODING_ASCII_US );
                break;
            }
            pEntry++;
        }
    }

    OSL_ENSURE( sFilterService.getLength(), "no export filter for own object" );

    if( !sFilterService.getLength() )
        return sal_False;

    Reference < XDocumentHandler > xHdl =
        new XMLEmbeddedObjectExportFilter( xHandler );

    Sequence < Any > aArgs( 1 );
    aArgs[0] <<= xHdl;

    Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
    Reference< document::XExporter > xExporter(
        xServiceFactory->createInstanceWithArguments( sFilterService, aArgs),
                                               UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
                "can't instantiate export filter component for own object" );
    if( !xExporter.is() )
        return sal_False;

    xExporter->setSourceDocument( rComp );

    Reference<XFilter> xFilter( xExporter, UNO_QUERY );

    Sequence < PropertyValue > aMediaDesc( 0 );
    return xFilter->filter( aMediaDesc );
}

OUString SvXMLExport::GetRelativeReference(const OUString& rValue)
{
    return INetURLObject::AbsToRel( rValue );
}

void SvXMLElementExport::StartElement( SvXMLExport& rExp,
                                       sal_uInt16 nPrefixKey,
                                       const OUString& rLName,
                                       sal_Bool bIWSOutside )
{
    aName = rExp.GetNamespaceMap().GetQNameByKey( nPrefixKey, rLName );

    if( bIWSOutside )
        rExport.GetDocHandler()->ignorableWhitespace( rExport.sWS );
    rExport.GetDocHandler()->startElement( aName, rExport.GetXAttrList() );
    rExport.ClearAttrList();
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        const sal_Char *pLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( sal_True )
{
    OUString sLName( OUString::createFromAscii(pLName) );
    StartElement( rExp, nPrefixKey, sLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        const OUString& rLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( sal_True )
{
    StartElement( rExp, nPrefixKey, rLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_uInt16 nPrefixKey,
                                        enum XMLTokenEnum eLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( sal_True )
{
    StartElement( rExp, nPrefixKey, GetXMLToken(eLName), bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_Bool bDoSth,
                                        sal_uInt16 nPrefixKey,
                                        const sal_Char *pLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( bDoSth )
{
    if( bDoSomething )
    {
        OUString sLName( OUString::createFromAscii(pLName) );
        StartElement( rExp, nPrefixKey, sLName, bIWSOutside );
    }
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_Bool bDoSth,
                                        sal_uInt16 nPrefixKey,
                                        const OUString& rLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( bDoSth )
{
    if( bDoSomething )
        StartElement( rExp, nPrefixKey, rLName, bIWSOutside );
}

SvXMLElementExport::SvXMLElementExport( SvXMLExport& rExp,
                                        sal_Bool bDoSth,
                                        sal_uInt16 nPrefixKey,
                                        enum XMLTokenEnum eLName,
                                        sal_Bool bIWSOutside,
                                        sal_Bool bIWSInside ) :
    rExport( rExp ),
    bIgnWS( bIWSInside ),
    bDoSomething( bDoSth )
{
    if( bDoSomething )
        StartElement( rExp, nPrefixKey, GetXMLToken(eLName), bIWSOutside );
}

SvXMLElementExport::~SvXMLElementExport()
{
    if( bDoSomething )
    {
        if( bIgnWS )
            rExport.GetDocHandler()->ignorableWhitespace( rExport.sWS );
        rExport.GetDocHandler()->endElement( aName );
    }
}

