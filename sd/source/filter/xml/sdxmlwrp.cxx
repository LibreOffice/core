/*************************************************************************
 *
 *  $RCSfile: sdxmlwrp.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-27 22:02:21 $
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

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <svx/xmlgrhlp.hxx>
#endif

#ifndef MAC
#ifndef SVX_LIGHT
#include "../../ui/inc/docshell.hxx"
#endif //!SVX_LIGHT
#else  //MAC
#ifndef SVX_LIGHT
#include "docshell.hxx"
#endif //!SVX_LIGHT
#endif //!MAC
#include "sdxmlwrp.hxx"
#include "strmname.h"

#ifndef _XMLEOHLP_HXX
#include <svx/xmleohlp.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XExporter_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XGRAPHICOBJECTRESOLVER_HXX_
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>

#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

using namespace com::sun::star;
using namespace rtl;
using namespace comphelper;

//////////////////////////////////////////////////////////////////////////////

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

#define MAP_LEN(x) x, sizeof(x) - 1

#define XML_STRING(i, x) sal_Char __READONLY_DATA i[sizeof(x)] = x

XML_STRING( sXML_metaStreamName, "meta.xml");
XML_STRING( sXML_styleStreamName, "styles.xml" );
XML_STRING( sXML_contentStreamName, "content.xml" );
XML_STRING( sXML_oldContentStreamName, "Content.xml" );
XML_STRING( sXML_settingsStreamName, "settings.xml" );

XML_STRING( sXML_export_impress_meta_service, "com.sun.star.comp.Impress.XMLMetaExporter" );
XML_STRING( sXML_export_impress_styles_service, "com.sun.star.comp.Impress.XMLStylesExporter" );
XML_STRING( sXML_export_impress_content_service, "com.sun.star.comp.Impress.XMLContentExporter" );
XML_STRING( sXML_export_impress_settings_service, "com.sun.star.comp.Impress.XMLSettingsExporter" );

XML_STRING( sXML_export_draw_meta_service, "com.sun.star.comp.Draw.XMLMetaExporter" );
XML_STRING( sXML_export_draw_styles_service, "com.sun.star.comp.Draw.XMLStylesExporter" );
XML_STRING( sXML_export_draw_content_service, "com.sun.star.comp.Draw.XMLContentExporter" );
XML_STRING( sXML_export_draw_settings_service, "com.sun.star.comp.Draw.XMLSettingsExporter" );

XML_STRING( sXML_import_impress_service, "com.sun.star.comp.Impress.XMLImporter" );
XML_STRING( sXML_import_impress_meta_service, "com.sun.star.comp.Impress.XMLMetaImporter" );
XML_STRING( sXML_import_impress_styles_service, "com.sun.star.comp.Impress.XMLStylesImporter" );
XML_STRING( sXML_import_impress_content_service, "com.sun.star.comp.Impress.XMLContentImporter" );
XML_STRING( sXML_import_impress_settings_service, "com.sun.star.comp.Impress.XMLSettingsImporter" );

XML_STRING( sXML_import_draw_service, "com.sun.star.comp.Draw.XMLImporter" );
XML_STRING( sXML_import_draw_meta_service, "com.sun.star.comp.Draw.XMLMetaImporter" );
XML_STRING( sXML_import_draw_styles_service, "com.sun.star.comp.Draw.XMLStylesImporter" );
XML_STRING( sXML_import_draw_content_service, "com.sun.star.comp.Draw.XMLContentImporter" );
XML_STRING( sXML_import_draw_settings_service, "com.sun.star.comp.Draw.XMLSettingsImporter" );

struct XML_SERVICEMAP
{
    const sal_Char* mpService;
    const sal_Char* mpStream;
};


// ----------------
// - SdXMLWrapper -
// ----------------

SdXMLFilter::SdXMLFilter( SfxMedium& rMedium, SdDrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress )
{
}

// -----------------------------------------------------------------------------

sal_Bool SdXMLFilter::Import()
{
    if( !mxModel.is() )
    {
        DBG_ERROR("Got NO Model in XMLImport");
        return FALSE;
    }

    uno::Reference<lang::XServiceInfo> xServiceInfo(mxModel, uno::UNO_QUERY);

    if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) ) )
    {
        DBG_ERROR( "Model is no DrawingDocument in XMLImport" );
        return FALSE;
    }

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );

    if( !xServiceFactory.is() )
    {
        DBG_ERROR( "XMLReader::Read: got no service manager" );
        return FALSE;
    }

    uno::Reference< uno::XInterface> xXMLParser( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" ) ) ) );

    if( !xXMLParser.is() )
    {
        DBG_ERROR( "com.sun.star.xml.sax.Parser service missing" );
        return FALSE;
    }

    mxModel->lockControllers();

    /** property map for export info set */
    PropertyMapEntry aImportInfoMap[] =
    {
        { MAP_LEN( "PageLayouts" ), 0, SEQTYPE(::getCppuType((const uno::Reference< container::XNameAccess >*)0)),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
        { NULL, 0, 0, NULL, 0, 0 }
    };

    uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );

    sal_Bool    bRet = sal_False;

    do
    {
        UINT16      nStyleFamilyMask = 0;
        sal_Bool    bLoadDoc = TRUE;

        try
        {
            SvStorage* pStorage = mrMedium.GetStorage();

            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

            uno::Reference< document::XGraphicObjectResolver >  xGrfResolver;
            SvXMLGraphicHelper* pGraphicHelper = 0;

            if( pStorage )
            {
                SvPersist *pPersist = mrDocShell.GetDoc()->GetPersist();
                if( pPersist )
                {
                    pObjectHelper = SvXMLEmbeddedObjectHelper::Create(*pStorage, *pPersist, EMBEDDEDOBJECTHELPER_MODE_READ, sal_False );
                    xObjectResolver = pObjectHelper;
                }

                pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_READ );
                xGrfResolver = pGraphicHelper;
            }

            if( mbShowProgress )
            {
                CreateStatusIndicator();

                if( mxStatusIndicator.is() )
                    mxStatusIndicator->start( ::rtl::OUString::createFromAscii( "XML Import" ), 100 );
            }

            uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY );

            XML_SERVICEMAP aServices[5];
            {
                int i = 0;
                // old format?
                if( pStorage->IsStream( String( RTL_CONSTASCII_STRINGPARAM( sXML_oldContentStreamName ) ) ) )
                {
                    aServices[i  ].mpService = IsDraw() ? sXML_import_draw_service : sXML_import_impress_service;
                    aServices[i++].mpStream  = sXML_oldContentStreamName;
                }
                else
                {
                    aServices[i  ].mpService = IsDraw() ? sXML_import_draw_meta_service : sXML_import_impress_meta_service;
                    aServices[i++].mpStream  = sXML_metaStreamName;

                    aServices[i  ].mpService = IsDraw() ? sXML_import_draw_styles_service : sXML_import_impress_styles_service;
                    aServices[i++].mpStream  = sXML_styleStreamName;

                    aServices[i  ].mpService = IsDraw() ? sXML_import_draw_content_service : sXML_import_impress_content_service;
                    aServices[i++].mpStream  = sXML_contentStreamName;

                    aServices[i  ].mpService = IsDraw() ? sXML_import_draw_settings_service : sXML_import_impress_settings_service;
                    aServices[i++].mpStream  = sXML_settingsStreamName;
                }

                aServices[i].mpService = NULL;
                aServices[i].mpStream  = NULL;
            }

            XML_SERVICEMAP* pServices;
            for( pServices = aServices; pServices->mpService; pServices++ )
            {
                xml::sax::InputSource                   aParserInput;
                SvStorageStreamRef                      xIStm;
                uno::Reference< io::XActiveDataSource > xSource;

                aParserInput.sSystemId = mrMedium.GetName();

                if( pStorage )
                {
                    String aStreamName( OUString::createFromAscii( pServices->mpStream ) );

                    if( !pStorage->IsStream( aStreamName ) )
                        continue;

                    xIStm = pStorage->OpenStream( aStreamName, STREAM_READ | STREAM_NOCREATE );

                    // try for old content stream name with capital 'C'
                    if( !xIStm.Is() && pServices->mpStream == sXML_contentStreamName )
                        xIStm = pStorage->OpenStream( OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_oldContentStreamName ) ), STREAM_READ | STREAM_NOCREATE );

                    if( xIStm.Is() )
                    {
                        xIStm->SetVersion( pStorage->GetVersion() );
                        xIStm->SetKey( pStorage->GetKey() );
                        xIStm->SetBufferSize( 16 * 1024 );
                        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xIStm );
                    }
                    else
                    {
                        DBG_ERROR( "could not open Content stream" );
                        break;
                    }
                }
                else
                {
                    uno::Reference< uno::XInterface > xPipe;

                    mrMedium.GetInStream()->Seek( 0 );

                    xSource = mrMedium.GetDataSource();
                    DBG_ASSERT( xSource.is(), "got no data source from medium" );

                    if( !xSource.is() )
                        break;

                    xPipe = xServiceFactory->createInstance( OUString::createFromAscii( "com.sun.star.io.Pipe" ) );
                    DBG_ASSERT( xPipe.is(), "com.sun.star.io.Pipe service missing" );

                    if( !xPipe.is() )
                        break;

                    xSource->setOutputStream( uno::Reference< io::XOutputStream >( xPipe, uno::UNO_QUERY ) );
                    aParserInput.aInputStream = uno::Reference< io::XInputStream >( xPipe, uno::UNO_QUERY );
                }

                if( aParserInput.aInputStream.is() )
                {
                    uno::Reference< xml::sax::XParser > xParser( xXMLParser, uno::UNO_QUERY );

                    uno::Sequence< uno::Any > aArgs( 1 + ( mxStatusIndicator.is() ? 1 : 0 ) + ( xGrfResolver.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                    uno::Any* pArgs = aArgs.getArray();
                    if( xGrfResolver.is() )
                        *pArgs++ <<= xGrfResolver;

                    if( xObjectResolver.is() )
                        *pArgs++ <<= xObjectResolver;

                    if( mxStatusIndicator.is() )
                        *pArgs++ <<= mxStatusIndicator;

                    *pArgs <<= xInfoSet;

                    uno::Reference< xml::sax::XDocumentHandler> xDocHandler( xServiceFactory->createInstanceWithArguments( OUString::createFromAscii( pServices->mpService ), aArgs), uno::UNO_QUERY );

                    if( xDocHandler.is() )
                    {
                        uno::Reference< document::XImporter > xImporter( xDocHandler, uno::UNO_QUERY );
                        if( xImporter.is() )
                        {
                            xImporter->setTargetDocument( xComponent );
                            xParser->setDocumentHandler( xDocHandler );

                            if( !pStorage )
                                uno::Reference< io::XActiveDataControl >( xSource, uno::UNO_QUERY )->start();

                            xParser->parseStream( aParserInput );
                            bRet = sal_True;
                        }
                    }
                }
            }

            if( pGraphicHelper )
                SvXMLGraphicHelper::Destroy( pGraphicHelper );

            if( pObjectHelper )
                SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
        }
        catch( uno::Exception e )
        {
    #ifdef DEBUG
            ByteString aError( "uno Exception caught while importing:\n" );
            aError += ByteString( String( e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aError.GetBuffer() );
    #endif
        }
    } while( 0 );

    mxModel->unlockControllers();

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdXMLFilter::Export()
{
    sal_Bool bDocRet = FALSE;

    try
    {
        if( !mxModel.is() )
        {
            DBG_ERROR("Got NO Model in XMLExport");
            return FALSE;
        }

        uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY );

        if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) ) )
        {
            DBG_ERROR( "Model is no DrawingDocument in XMLExport" );
            return FALSE;
        }

        uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );

        if( !xServiceFactory.is() )
        {
            DBG_ERROR( "got no service manager" );
            return FALSE;
        }

        uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );

        if( !xWriter.is() )
        {
            DBG_ERROR( "com.sun.star.xml.sax.Writer service missing" );
            return FALSE;
        }

        uno::Reference<xml::sax::XDocumentHandler>  xHandler( xWriter, uno::UNO_QUERY );

        /** property map for export info set */
        PropertyMapEntry aExportInfoMap[] =
        {
            { MAP_LEN( "PageLayoutNames" ), 0, SEQTYPE(::getCppuType((const OUString*)0)),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
            { NULL, 0, 0, NULL, 0, 0 }
        };

        uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aExportInfoMap ) ) );

        SvStorage* pStorage = mrMedium.GetOutputStorage( sal_True );

        // initialize descriptor
        uno::Sequence< beans::PropertyValue > aDescriptor( 1 );
        beans::PropertyValue* pProps = aDescriptor.getArray();

        pProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
        pProps[0].Value <<= OUString( mrMedium.GetName() );

        {
            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

            uno::Reference< document::XGraphicObjectResolver > xGrfResolver;
            SvXMLGraphicHelper* pGraphicHelper = 0;

            // create helper for graphic and ole export if we have a storage
            if( pStorage )
            {
                SvPersist *pPersist = mrDocShell.GetDoc()->GetPersist();
                if( pPersist )
                {
                    pObjectHelper = SvXMLEmbeddedObjectHelper::Create( *pStorage, *pPersist, EMBEDDEDOBJECTHELPER_MODE_WRITE, sal_False );
                    xObjectResolver = pObjectHelper;
                }

                pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_WRITE, FALSE );
                xGrfResolver = pGraphicHelper;

            }

            if( mbShowProgress )
            {
                CreateStatusIndicator();

                if( mxStatusIndicator.is() )
                    mxStatusIndicator->start( ::rtl::OUString::createFromAscii( "XML Export" ), 100 );
            }

            uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY );

            XML_SERVICEMAP aServices[5];
            aServices[0].mpService = IsDraw() ? sXML_export_draw_meta_service : sXML_export_impress_meta_service;
            aServices[0].mpStream  = sXML_metaStreamName;

            aServices[1].mpService = IsDraw() ? sXML_export_draw_styles_service : sXML_export_impress_styles_service;
            aServices[1].mpStream  = sXML_styleStreamName;

            aServices[2].mpService = IsDraw() ? sXML_export_draw_content_service : sXML_export_impress_content_service;
            aServices[2].mpStream  = sXML_contentStreamName;

            aServices[3].mpService = IsDraw() ? sXML_export_draw_settings_service : sXML_export_impress_settings_service;
            aServices[3].mpStream  = sXML_settingsStreamName;

            aServices[4].mpService = NULL;
            aServices[4].mpStream  = NULL;

            XML_SERVICEMAP* pServices = aServices;

            // doc export
            do
            {
                uno::Reference<io::XOutputStream> xDocOut;
                SvStorageStreamRef xDocStream;

                if( pStorage )
                {
                    const OUString sDocName( OUString::createFromAscii( pServices->mpStream ) );
                    xDocStream = pStorage->OpenStream( sDocName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );
                    xDocStream->SetVersion( pStorage->GetVersion() );
                    xDocStream->SetKey( pStorage->GetKey() );
                    xDocStream->SetBufferSize( 16*1024 );
                    xDocOut = new utl::OOutputStreamWrapper( *xDocStream );

                    uno::Any aAny; aAny <<= OUString( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
                    xDocStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), aAny);

                }
                else
                {
                    xDocOut = mrMedium.GetDataSink();
                }

                uno::Reference< io::XActiveDataSource > xDocSrc( xWriter, uno::UNO_QUERY );
                xDocSrc->setOutputStream( xDocOut );

                uno::Sequence< uno::Any > aArgs( 2 + ( mxStatusIndicator.is() ? 1 : 0 ) + ( xGrfResolver.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                uno::Any* pArgs = aArgs.getArray();
                if( xGrfResolver.is() )         *pArgs++ <<= xGrfResolver;
                if( xObjectResolver.is() )      *pArgs++ <<= xObjectResolver;
                if( mxStatusIndicator.is() )    *pArgs++ <<= mxStatusIndicator;

                *pArgs++ <<= xInfoSet;
                *pArgs   <<= xHandler;

                uno::Reference< document::XFilter > xFilter( xServiceFactory->createInstanceWithArguments( OUString::createFromAscii( pServices->mpService ), aArgs ), uno::UNO_QUERY );
                if( xFilter.is() )
                {
                    uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
                    if( xExporter.is() )
                    {
                        xExporter->setSourceDocument( xComponent );

                        bDocRet = xFilter->filter( aDescriptor );

                        if(bDocRet && xDocStream.Is())
                            xDocStream->Commit();
                    }
                }

                pServices++;
            }
            while( bDocRet && pServices->mpService );

            if( pGraphicHelper )
                SvXMLGraphicHelper::Destroy( pGraphicHelper );

            if( pObjectHelper )
                SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
        }
    }
    catch(uno::Exception e)
    {
#ifdef DEBUG
        ByteString aError( "uno Exception caught while exporting:\n" );
        aError += ByteString( String( e.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        bDocRet = sal_False;
    }

    return bDocRet;
}
