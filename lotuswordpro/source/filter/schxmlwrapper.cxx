/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#include "schxmlwrapper.hxx"

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <svx/xmlgrhlp.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XERRORHANDLER_HPP_
#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XENTITYRESOLVER_HPP_
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDTDHANDLER_HPP_
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPIOEXCEPTION_HPP_
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#endif

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

XML_STRING( sXML_export_chart_allinone_service, 		"com.sun.star.comp.Chart.XMLExporter" );



XML_STRING( sXML_import_chart_meta_service, 		"com.sun.star.comp.Chart.XMLMetaImporter" );
XML_STRING( sXML_import_chart_styles_service,		"com.sun.star.comp.Chart.XMLStylesImporter" );
XML_STRING( sXML_import_chart_content_service,		"com.sun.star.comp.Chart.XMLContentImporter" );
XML_STRING( sXML_import_chart_old_content_service,	"com.sun.star.office.sax.importer.Chart" );

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace comphelper;

SchXMLWrapper::SchXMLWrapper( uno::Reference< frame::XModel >& xModel,
                              SvStorage& rStorage,
                              sal_Bool bShowProgress ) :
        mxModel( xModel ),
        mrStorage( rStorage ),
        mbShowProgress( bShowProgress )
{}


sal_Int32 SchXMLWrapper::ImportStream(
    const ::rtl::OUString& rsStreamName,
    const ::rtl::OUString& rsServiceName,
    uno::Reference< xml::sax::XParser >& xParser,
    uno::Reference< lang::XMultiServiceFactory >& xServiceFactory,
    uno::Reference< document::XGraphicObjectResolver >& xGraphObjResolver )
{
    xml::sax::InputSource aParserInput;
    SvStorageStreamRef rInpStream;
    uno::Reference< io::XActiveDataSource > xSource;
    sal_Bool bEncrypted = sal_False;

    try
    {
        String sStreamName( rsStreamName );
        if( ! mrStorage.IsStream( String( rsStreamName )))
            return sal_False;

        rInpStream = mrStorage.OpenStream( sStreamName, STREAM_READ | STREAM_NOCREATE );
        if( ! rInpStream.Is())
            return sal_False;

        uno::Any aAny;
        bEncrypted = rInpStream->GetProperty( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" )), aAny )
            && aAny.getValueType() == ::getBooleanCppuType()
            && *(sal_Bool *)aAny.getValue();

        aParserInput.aInputStream = rInpStream->GetXInputStream();

        if( aParserInput.aInputStream.is() )
        {
            sal_Int32 nArgs = 0;
            if( mxStatusIndicator.is())
                nArgs++;
            if( xGraphObjResolver.is())
                nArgs++;

            uno::Sequence< uno::Any > aArgs( nArgs );

            nArgs = 0;
            if( mxStatusIndicator.is())
                aArgs[ nArgs++ ] <<= mxStatusIndicator;

            if( xGraphObjResolver.is())
                aArgs[ nArgs++ ] <<= xGraphObjResolver;

            uno::Reference< xml::sax::XDocumentHandler > xDocHandler(
                xServiceFactory->createInstanceWithArguments( rsServiceName, aArgs ),
                uno::UNO_QUERY );

            if( xDocHandler.is() )
            {
                uno::Reference< document::XImporter > xImporter( xDocHandler, uno::UNO_QUERY );
                uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY );

                if( xImporter.is() )
                {
                    xImporter->setTargetDocument( xComponent );
                    xParser->setDocumentHandler( xDocHandler );

                    xParser->parseStream( aParserInput );
                }
            }
        }
    }
    catch( xml::sax::SAXParseException&)
    {
        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;
        return ERRCODE_SFX_GENERAL;
    }
    catch( xml::sax::SAXException&)
    {
        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;
        return ERRCODE_SFX_GENERAL;
    }
    catch( io::IOException&)
    {
        return ERRCODE_SFX_GENERAL;
    }
    catch( packages::zip::ZipIOException& )
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch( uno::Exception&)
    {
        return ERRCODE_SFX_GENERAL;
    }

    return 0;
}

sal_Int32 SchXMLWrapper::Import()
{
    sal_Int32 nWarning = 0;

    if( !mxModel.is() )
    {
        DBG_ERROR("Got NO Model in XMLImport");
        return sal_False;
    }

    uno::Reference<lang::XServiceInfo> xServiceInfo( mxModel, uno::UNO_QUERY );

    if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.ChartDocument" ) ) ) )
    {
        DBG_ERROR( "Model is no ChartDocument in XMLImport" );
        return sal_False;
    }

    uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );

    if( !xServiceFactory.is() )
    {
        DBG_ERROR( "XMLReader::Read: got no service manager" );
        return sal_False;
    }

    // get the sax parser component
    uno::Reference< xml::sax::XParser > xXMLParser(
        xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" )),
        uno::UNO_QUERY );

    if( ! xXMLParser.is() )
    {
        DBG_ERROR( "com.sun.star.xml.sax.Parser service missing" );
        return sal_False;
    }

    // create graphics resolver component
    SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create( mrStorage, GRAPHICHELPER_MODE_READ );
    uno::Reference< document::XGraphicObjectResolver > xGraphObjResolver = pGraphicHelper;

    // import meta information
    ImportStream(
        ::rtl::OUString::createFromAscii( sXML_metaStreamName ),
        ::rtl::OUString::createFromAscii( sXML_import_chart_meta_service ),
        xXMLParser, xServiceFactory, xGraphObjResolver );

    // import styles
    ImportStream(
        ::rtl::OUString::createFromAscii( sXML_styleStreamName ),
        ::rtl::OUString::createFromAscii( sXML_import_chart_styles_service ),
        xXMLParser, xServiceFactory, xGraphObjResolver );

    // import content
    nWarning = ImportStream(
        ::rtl::OUString::createFromAscii( sXML_contentStreamName ),
        ::rtl::OUString::createFromAscii( sXML_import_chart_content_service ),
        xXMLParser, xServiceFactory, xGraphObjResolver );

    // import of "content.xml" didn't work - try old "Content.xml" stream
    if( nWarning != 0 )
    {
        nWarning = ImportStream(
            ::rtl::OUString::createFromAscii( sXML_oldContentStreamName ),
            ::rtl::OUString::createFromAscii( sXML_import_chart_old_content_service ),
            xXMLParser, xServiceFactory, xGraphObjResolver );
    }

    // graphics resolver has to be destroyed this way!
    SvXMLGraphicHelper::Destroy( pGraphicHelper );

    return nWarning;
}

// -----------------------------------------------------------------------------

sal_Bool SchXMLWrapper::ExportStream(
    const ::rtl::OUString& rsStreamName,
    const ::rtl::OUString& rsServiceName,
    uno::Reference< io::XActiveDataSource >& xDataSource,
    uno::Reference< lang::XMultiServiceFactory>& xServiceFactory,
    uno::Sequence< uno::Any >& aArgs )
{
    sal_Bool bRet = sal_False;

    try
    {
        // create output stream
        SvStorageStreamRef rOutputStream( mrStorage.OpenStream(
            String( rsStreamName ), STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC ));

        rtl::OUString sMIMEType( RTL_CONSTASCII_USTRINGPARAM( "text/xml" ) );
        uno::Any aAny;
        aAny <<= sMIMEType;
        rOutputStream->SetProperty( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" )), aAny );

        // "Encrypted" means "Encryptable". Has to be set for all but the meta stream (which doesn't exist in charts)
        aAny <<= (sal_Bool)(sal_True);
        rOutputStream->SetProperty( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" )), aAny );

        rOutputStream->SetBufferSize( 0x4000 );	   // 16*1024

        if( xDataSource.is())
            xDataSource->setOutputStream( new ::utl::OOutputStreamWrapper( *rOutputStream ));

        uno::Reference< document::XFilter > xFilter(
            xServiceFactory->createInstanceWithArguments( rsServiceName, aArgs ),
            uno::UNO_QUERY );

        if( xFilter.is())
        {
            uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
            if( xExporter.is())
            {
                uno::Reference< lang::XComponent > xModelComponent( mxModel, uno::UNO_QUERY );
                xExporter->setSourceDocument( xModelComponent );
            }

            // empty Descriptior (formerly FileName was given)
            uno::Sequence< beans::PropertyValue > aEmptyDescriptor( 0 );
            bRet = xFilter->filter( aEmptyDescriptor );

            if( bRet && rOutputStream.Is())
                rOutputStream->Commit();
        }
    }
    catch( uno::Exception )
    {
    }

    return bRet;
}

sal_Bool SchXMLWrapper::Export()
{
    sal_Bool bRet = sal_False;

    try
    {
        if( !mxModel.is() )
        {
            DBG_ERROR("Got NO Model in XMLExport");
            return sal_False;
        }

        uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY );

        if( ! xServiceInfo.is() || !xServiceInfo->supportsService(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.ChartDocument" ) ) ) )
        {
            DBG_ERROR( "Model is no ChartDocument in XMLExport" );
            return sal_False;
        }

        uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );

        if( !xServiceFactory.is() )
        {
            DBG_ERROR( "got no service manager" );
            return sal_False;
        }

        uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );

        if( !xWriter.is() )
        {
            DBG_ERROR( "com.sun.star.xml.sax.Writer service missing" );
            return sal_False;
        }
        uno::Reference<xml::sax::XDocumentHandler > xHandler( xWriter, uno::UNO_QUERY );
        uno::Sequence< beans::PropertyValue > aEmptyDescriptor( 0 );


        /** property map for export info set */
        PropertyMapEntry aExportInfoMap[] =
        {
            { MAP_LEN( "UsePrettyPrinting"),0, &::getBooleanCppuType(),				::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { NULL, 0, 0, NULL, 0, 0 }
        };

        uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aExportInfoMap ) ) );

        SvtSaveOptions aSaveOpt;
        OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting"));
        sal_Bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
        xInfoSet->setPropertyValue( sUsePrettyPrinting, uno::makeAny( bUsePrettyPrinting ) );


        SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create( mrStorage, GRAPHICHELPER_MODE_WRITE, sal_False );
        uno::Reference< document::XGraphicObjectResolver > xGraphObjResolver( pGraphicHelper );
        uno::Reference< io::XActiveDataSource > xDataSource( xWriter, uno::UNO_QUERY );

        sal_Int32 nArgs = 2;
        if( mxStatusIndicator.is())
            nArgs++;
        if( xGraphObjResolver.is())
            nArgs++;

        uno::Sequence< uno::Any > aArgs( nArgs );

        nArgs = 0;
        aArgs[ nArgs++ ] <<= xHandler;
        aArgs[ nArgs++ ] <<= xInfoSet;

        if( mxStatusIndicator.is())
            aArgs[ nArgs++ ] <<= mxStatusIndicator;
        if( xGraphObjResolver.is())
            aArgs[ nArgs++ ] <<= xGraphObjResolver;

        //export to one stream
        bRet = ExportStream(
            ::rtl::OUString::createFromAscii( sXML_contentStreamName ),
            ::rtl::OUString::createFromAscii( sXML_export_chart_allinone_service ),
            xDataSource, xServiceFactory, aArgs );

        // graphics resolver has to be destroyed this way!
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    }
    catch( uno::Exception)
    {
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
