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

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_ 
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_ 
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_ 
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XExporter_HPP_ 
#include <com/sun/star/document/XExporter.hpp>
#endif

#ifndef _SFXDOCFILE_HXX
#include <bf_sfx2/docfile.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <bf_svtools/sfxecode.hxx>
#endif

#include "SchXMLWrapper.hxx"

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <bf_svx/xmlgrhlp.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX 
#include <bf_svtools/saveopt.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPIOEXCEPTION_HPP_
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#endif

#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {

#define XML_STRING(i, x) sal_Char __READONLY_DATA i[sizeof(x)] = x
#define MAP_LEN(x) x, sizeof(x) - 1

XML_STRING( sXML_metaStreamName, 		"meta.xml");
XML_STRING( sXML_styleStreamName,		"styles.xml" );
XML_STRING( sXML_contentStreamName,		"content.xml" );
XML_STRING( sXML_oldContentStreamName,	"Content.xml" );

XML_STRING( sXML_export_chart_meta_service, 		"com.sun.star.comp.Chart.XMLMetaExporter" );
XML_STRING( sXML_export_chart_styles_service, 		"com.sun.star.comp.Chart.XMLStylesExporter" );
XML_STRING( sXML_export_chart_content_service,		"com.sun.star.comp.Chart.XMLContentExporter" );
XML_STRING( sXML_export_chart_old_content_service,	"com.sun.star.office.sax.exporter.Chart" );

XML_STRING( sXML_import_chart_meta_service, 		"com.sun.star.comp.Chart.XMLMetaImporter" );
XML_STRING( sXML_import_chart_styles_service,		"com.sun.star.comp.Chart.XMLStylesImporter" );
XML_STRING( sXML_import_chart_content_service,		"com.sun.star.comp.Chart.XMLContentImporter" );
XML_STRING( sXML_import_chart_old_content_service,	"com.sun.star.office.sax.importer.Chart" );

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace comphelper;

/*N*/ SchXMLWrapper::SchXMLWrapper( uno::Reference< frame::XModel >& xModel,
/*N*/ 							  SvStorage& rStorage,
/*N*/ 							  sal_Bool bShowProgress ) :
/*N*/ 		mxModel( xModel ),
/*N*/ 		mrStorage( rStorage ),
/*N*/ 		mbShowProgress( bShowProgress )
/*N*/ {}

// -----------------------------------------------------------------------------

/*N*/ sal_Bool SchXMLWrapper::ExportStream(
/*N*/ 	const ::rtl::OUString& rsStreamName,
/*N*/ 	const ::rtl::OUString& rsServiceName,
/*N*/ 	uno::Reference< io::XActiveDataSource >& xDataSource,
/*N*/ 	uno::Reference< lang::XMultiServiceFactory>& xServiceFactory,
/*N*/ 	uno::Sequence< uno::Any >& aArgs )
/*N*/ {
/*N*/ 	sal_Bool bRet = sal_False;
/*N*/ 
/*N*/ 	try
/*N*/ 	{
/*N*/ 		// create output stream
/*N*/ 		SvStorageStreamRef rOutputStream( mrStorage.OpenStream(
/*N*/ 			String( rsStreamName ), STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC ));
/*N*/ 
/*N*/ 		::rtl::OUString sMIMEType( RTL_CONSTASCII_USTRINGPARAM( "text/xml" ) );
/*N*/ 		uno::Any aAny;
/*N*/ 		aAny <<= sMIMEType;
/*N*/ 		rOutputStream->SetProperty( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" )), aAny );
/*N*/ 
/*N*/         // "Encrypted" means "Encryptable". Has to be set for all but the meta stream (which doesn't exist in charts)
/*N*/         aAny <<= (sal_Bool)(sal_True);
/*N*/         rOutputStream->SetProperty( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" )), aAny );
/*N*/ 
/*N*/ 		rOutputStream->SetBufferSize( 0x4000 );	   // 16*1024
/*N*/ 
/*N*/ 		if( xDataSource.is())
/*N*/ 			xDataSource->setOutputStream( new ::utl::OOutputStreamWrapper( *rOutputStream ));
/*N*/ 
/*N*/ 		uno::Reference< document::XFilter > xFilter(
/*N*/ 			xServiceFactory->createInstanceWithArguments( rsServiceName, aArgs ),
/*N*/ 			uno::UNO_QUERY );
/*N*/ 
/*N*/ 		if( xFilter.is())
/*N*/ 		{
/*N*/ 			uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
/*N*/ 			if( xExporter.is())
/*N*/ 			{
/*N*/ 				uno::Reference< lang::XComponent > xModelComponent( mxModel, uno::UNO_QUERY );
/*N*/ 				xExporter->setSourceDocument( xModelComponent );
/*N*/ 			}
/*N*/ 
/*N*/ 			// empty Descriptior (formerly FileName was given)
/*N*/ 			uno::Sequence< beans::PropertyValue > aEmptyDescriptor( 0 );
/*N*/ 			bRet = xFilter->filter( aEmptyDescriptor );
/*N*/ 
/*N*/ 			if( bRet && rOutputStream.Is())
/*N*/ 				rOutputStream->Commit();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	catch( uno::Exception aEx )
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*N*/ 		// convert ::rtl::OUString => tools String => ByteString
/*?*/ 		String aStr( aEx.Message );
/*?*/ 		ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
/*?*/ 		ByteString aBStrStreamName = ByteString( String( rsStreamName ), RTL_TEXTENCODING_ASCII_US );
/*?*/ 		DBG_ERROR2( "Exception caught during Export of \"%s\" stream(): %s",
/*?*/ 					aBStrStreamName.GetBuffer(),
/*?*/ 					aBStr.GetBuffer());
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ sal_Bool SchXMLWrapper::Export()
/*N*/ {
/*N*/ 	sal_Bool bRet = sal_False;
/*N*/ 
/*N*/ 	try
/*N*/ 	{
/*N*/ 		if( !mxModel.is() )
/*N*/ 		{
/*N*/ 			DBG_ERROR("Got NO Model in XMLExport");
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 
/*N*/ 		uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY );
/*N*/ 
/*N*/ 		if( ! xServiceInfo.is() || !xServiceInfo->supportsService(
/*N*/ 			OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.ChartDocument" ) ) ) )
/*N*/ 		{
/*N*/ 			DBG_ERROR( "Model is no ChartDocument in XMLExport" );
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 
/*N*/ 		uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/ 	
/*N*/ 		if( !xServiceFactory.is() )
/*N*/ 		{
/*N*/ 			DBG_ERROR( "got no service manager" );
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 
/*N*/ 		uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance(
/*N*/ 			OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );
/*N*/ 	
/*N*/ 		if( !xWriter.is() )
/*N*/ 		{
/*N*/ 			DBG_ERROR( "com.sun.star.xml.sax.Writer service missing" );
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 		uno::Reference<xml::sax::XDocumentHandler > xHandler( xWriter, uno::UNO_QUERY );
/*N*/ 		uno::Sequence< beans::PropertyValue > aEmptyDescriptor( 0 );
/*N*/ 
/*N*/ 
/*N*/ 		/** property map for export info set */
/*N*/ 		PropertyMapEntry aExportInfoMap[] =
/*N*/ 		{
/*N*/ 			{ MAP_LEN( "UsePrettyPrinting"),0, &::getBooleanCppuType(),				::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
/*N*/ 			{ NULL, 0, 0, NULL, 0, 0 }
/*N*/ 		};
/*N*/ 
/*N*/ 		uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aExportInfoMap ) ) );
/*N*/ 
/*N*/ 		SvtSaveOptions aSaveOpt;
/*N*/ 		OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting"));
/*N*/ 		sal_Bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
/*N*/ 		xInfoSet->setPropertyValue( sUsePrettyPrinting, uno::makeAny( bUsePrettyPrinting ) );
/*N*/ 
/*N*/ 
/*N*/ 		SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create( mrStorage, GRAPHICHELPER_MODE_WRITE, sal_False );
/*N*/ 		uno::Reference< document::XGraphicObjectResolver > xGraphObjResolver( pGraphicHelper );
/*N*/ 		uno::Reference< io::XActiveDataSource > xDataSource( xWriter, uno::UNO_QUERY );
/*N*/ 
/*N*/ 		// there is no meta info needed in charts - they are always OLE objects
/*N*/ 		// export meta
/*N*/ //  		uno::Sequence< uno::Any > aMetaArgs( 1 );
/*N*/ //  		aMetaArgs[ 0 ] <<= xHandler;
/*N*/ //  		bRet = ExportStream(
/*N*/ //  			::rtl::OUString::createFromAscii( sXML_metaStreamName ),
/*N*/ //  			::rtl::OUString::createFromAscii( sXML_export_chart_meta_service ),
/*N*/ //  			xDataSource, xServiceFactory, aMetaArgs );
/*N*/ 
/*N*/ 		// prepare arguments: document handler / status indicator and graphics resolver
/*N*/ 		sal_Int32 nArgs = 2;
/*N*/ 		if( mxStatusIndicator.is())
/*N*/ 			nArgs++;
/*N*/ 		if( xGraphObjResolver.is())
/*N*/ 			nArgs++;
/*N*/ 
/*N*/ 		uno::Sequence< uno::Any > aArgs( nArgs );
/*N*/ 
/*N*/ 		nArgs = 0;
/*N*/ 		aArgs[ nArgs++ ] <<= xHandler;
/*N*/ 		aArgs[ nArgs++ ] <<= xInfoSet;
/*N*/ 
/*N*/ 		if( mxStatusIndicator.is())
/*?*/ 			aArgs[ nArgs++ ] <<= mxStatusIndicator;
/*N*/ 		if( xGraphObjResolver.is())
/*N*/ 			aArgs[ nArgs++ ] <<= xGraphObjResolver;
/*N*/ 
/*N*/ 		// styles export
/*N*/ 		bRet = ExportStream(
/*N*/ 			::rtl::OUString::createFromAscii( sXML_styleStreamName ),
/*N*/ 			::rtl::OUString::createFromAscii( sXML_export_chart_styles_service ),
/*N*/ 			xDataSource, xServiceFactory, aArgs );
/*N*/ 
/*N*/ 		// content export
/*N*/ 		bRet = ExportStream(
/*N*/ 			::rtl::OUString::createFromAscii( sXML_contentStreamName ),
/*N*/ 			::rtl::OUString::createFromAscii( sXML_export_chart_content_service ),
/*N*/ 			xDataSource, xServiceFactory, aArgs );
/*N*/ 
/*N*/ 		// graphics resolver has to be destroyed this way!
/*N*/ 		SvXMLGraphicHelper::Destroy( pGraphicHelper );
/*N*/ 	}
/*N*/ 	catch( uno::Exception aEx )
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*?*/ 		// convert ::rtl::OUString => tools String => ByteString
/*?*/ 		String aStr( aEx.Message );
/*?*/ 		ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
/*?*/ 		DBG_ERROR1( "Exception caught during Export of : %s", aBStr.GetBuffer());
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }
}
