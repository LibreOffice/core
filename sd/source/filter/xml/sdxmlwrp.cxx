/*************************************************************************
 *
 *  $RCSfile: sdxmlwrp.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:43:58 $
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

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HDL_
#include <com/sun/star/xml/sax/SAXParseException.hdl>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
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
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "../../ui/inc/DrawDocShell.hxx"
#endif
#endif //!SVX_LIGHT
#else  //MAC
#ifndef SVX_LIGHT
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
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
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPIOEXCEPTION_HPP_
#include <com/sun/star/packages/zip/ZipIOException.hpp>
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

#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

// #80365# include necessary for XML progress bar at load time
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#include "sderror.hxx"
#include "sdresid.hxx"
#include "glob.hrc"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::document;
using namespace rtl;
using namespace comphelper;

#define SD_XML_READERROR 1234

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

XML_STRING( sXML_export_impress_oasis_service, "com.sun.star.comp.Impress.XMLOasisExporter" );
XML_STRING( sXML_export_impress_meta_oasis_service, "com.sun.star.comp.Impress.XMLOasisMetaExporter" );
XML_STRING( sXML_export_impress_styles_oasis_service, "com.sun.star.comp.Impress.XMLOasisStylesExporter" );
XML_STRING( sXML_export_impress_content_oasis_service, "com.sun.star.comp.Impress.XMLOasisContentExporter" );
XML_STRING( sXML_export_impress_settings_oasis_service, "com.sun.star.comp.Impress.XMLOasisSettingsExporter" );

XML_STRING( sXML_export_draw_oasis_service, "com.sun.star.comp.Draw.XMLOasisExporter" );
XML_STRING( sXML_export_draw_meta_oasis_service, "com.sun.star.comp.Draw.XMLOasisMetaExporter" );
XML_STRING( sXML_export_draw_styles_oasis_service, "com.sun.star.comp.Draw.XMLOasisStylesExporter" );
XML_STRING( sXML_export_draw_content_oasis_service, "com.sun.star.comp.Draw.XMLOasisContentExporter" );
XML_STRING( sXML_export_draw_settings_oasis_service, "com.sun.star.comp.Draw.XMLOasisSettingsExporter" );

XML_STRING( sXML_import_impress_oasis_service, "com.sun.star.comp.Impress.XMLOasisImporter" );
XML_STRING( sXML_import_impress_meta_oasis_service, "com.sun.star.comp.Impress.XMLOasisMetaImporter" );
XML_STRING( sXML_import_impress_styles_oasis_service, "com.sun.star.comp.Impress.XMLOasisStylesImporter" );
XML_STRING( sXML_import_impress_content_oasis_service, "com.sun.star.comp.Impress.XMLOasisContentImporter" );
XML_STRING( sXML_import_impress_settings_oasis_service, "com.sun.star.comp.Impress.XMLOasisSettingsImporter" );

XML_STRING( sXML_import_draw_oasis_service, "com.sun.star.comp.Draw.XMLOasisImporter" );
XML_STRING( sXML_import_draw_meta_oasis_service, "com.sun.star.comp.Draw.XMLOasisMetaImporter" );
XML_STRING( sXML_import_draw_styles_oasis_service, "com.sun.star.comp.Draw.XMLOasisStylesImporter" );
XML_STRING( sXML_import_draw_content_oasis_service, "com.sun.star.comp.Draw.XMLOasisContentImporter" );
XML_STRING( sXML_import_draw_settings_oasis_service, "com.sun.star.comp.Draw.XMLOasisSettingsImporter" );

// OOo
XML_STRING( sXML_export_impress_ooo_service, "com.sun.star.comp.Impress.XMLExporter" );
XML_STRING( sXML_export_impress_meta_ooo_service, "com.sun.star.comp.Impress.XMLMetaExporter" );
XML_STRING( sXML_export_impress_styles_ooo_service, "com.sun.star.comp.Impress.XMLStylesExporter" );
XML_STRING( sXML_export_impress_content_ooo_service, "com.sun.star.comp.Impress.XMLContentExporter" );
XML_STRING( sXML_export_impress_settings_ooo_service, "com.sun.star.comp.Impress.XMLSettingsExporter" );

XML_STRING( sXML_export_draw_ooo_service, "com.sun.star.comp.Draw.XMLExporter" );
XML_STRING( sXML_export_draw_meta_ooo_service, "com.sun.star.comp.Draw.XMLMetaExporter" );
XML_STRING( sXML_export_draw_styles_ooo_service, "com.sun.star.comp.Draw.XMLStylesExporter" );
XML_STRING( sXML_export_draw_content_ooo_service, "com.sun.star.comp.Draw.XMLContentExporter" );
XML_STRING( sXML_export_draw_settings_ooo_service, "com.sun.star.comp.Draw.XMLSettingsExporter" );

XML_STRING( sXML_import_impress_ooo_service, "com.sun.star.comp.Impress.XMLImporter" );
XML_STRING( sXML_import_impress_meta_ooo_service, "com.sun.star.comp.Impress.XMLMetaImporter" );
XML_STRING( sXML_import_impress_styles_ooo_service, "com.sun.star.comp.Impress.XMLStylesImporter" );
XML_STRING( sXML_import_impress_content_ooo_service, "com.sun.star.comp.Impress.XMLContentImporter" );
XML_STRING( sXML_import_impress_settings_ooo_service, "com.sun.star.comp.Impress.XMLSettingsImporter" );

XML_STRING( sXML_import_draw_ooo_service, "com.sun.star.comp.Draw.XMLImporter" );
XML_STRING( sXML_import_draw_meta_ooo_service, "com.sun.star.comp.Draw.XMLMetaImporter" );
XML_STRING( sXML_import_draw_styles_ooo_service, "com.sun.star.comp.Draw.XMLStylesImporter" );
XML_STRING( sXML_import_draw_content_ooo_service, "com.sun.star.comp.Draw.XMLContentImporter" );
XML_STRING( sXML_import_draw_settings_ooo_service, "com.sun.star.comp.Draw.XMLSettingsImporter" );

struct XML_SERVICEMAP
{
    const sal_Char* mpService;
    const sal_Char* mpStream;
    sal_Bool mbPlain;
};

struct XML_SERVICES
{
    const sal_Char* mpAll;
    const sal_Char* mpMeta;
    const sal_Char* mpStyles;
    const sal_Char* mpContent;
    const sal_Char* mpSettings;
};

XML_SERVICES* getServices( bool bImport, bool bDraw, ULONG nStoreVer )
{
    static XML_SERVICES gServices[] =
    {
        { sXML_export_impress_oasis_service, sXML_export_impress_meta_oasis_service, sXML_export_impress_styles_oasis_service, sXML_export_impress_content_oasis_service, sXML_export_impress_settings_oasis_service },
        { sXML_export_draw_oasis_service, sXML_export_draw_meta_oasis_service, sXML_export_draw_styles_oasis_service, sXML_export_draw_content_oasis_service, sXML_export_draw_settings_oasis_service },
        { sXML_import_impress_oasis_service, sXML_import_impress_meta_oasis_service, sXML_import_impress_styles_oasis_service, sXML_import_impress_content_oasis_service, sXML_import_impress_settings_oasis_service },
        { sXML_import_draw_oasis_service, sXML_import_draw_meta_oasis_service, sXML_import_draw_styles_oasis_service, sXML_import_draw_content_oasis_service, sXML_import_draw_settings_oasis_service },

        { sXML_export_impress_ooo_service, sXML_export_impress_meta_ooo_service, sXML_export_impress_styles_ooo_service, sXML_export_impress_content_ooo_service, sXML_export_impress_settings_ooo_service },
        { sXML_export_draw_ooo_service, sXML_export_draw_meta_ooo_service, sXML_export_draw_styles_ooo_service, sXML_export_draw_content_ooo_service, sXML_export_draw_settings_ooo_service },
        { sXML_import_impress_ooo_service, sXML_import_impress_meta_ooo_service, sXML_import_impress_styles_ooo_service, sXML_import_impress_content_ooo_service, sXML_import_impress_settings_ooo_service },
        { sXML_import_draw_ooo_service, sXML_import_draw_meta_ooo_service, sXML_import_draw_styles_ooo_service, sXML_import_draw_content_ooo_service, sXML_import_draw_settings_ooo_service },
    };

    return &gServices[ (bImport ? 2 : 0) + ((nStoreVer == SOFFICE_FILEFORMAT_60) ? 4 : 0) + (bDraw ? 1 : 0 ) ];
}


// ----------------
// - SdXMLWrapper -
// ----------------

SdXMLFilter::SdXMLFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress, SdXMLFilterMode eFilterMode, ULONG nStoreVer ) :
    SdFilter( rMedium, rDocShell, bShowProgress ), meFilterMode( eFilterMode ), mnStoreVer( nStoreVer )
{
}

SdXMLFilter::~SdXMLFilter(void)
{
}

sal_Int32 ReadThroughComponent(
    Reference<io::XInputStream> xInputStream,
    Reference<XComponent> xModelComponent,
    const String& rStreamName,
    Reference<lang::XMultiServiceFactory> & rFactory,
    const sal_Char* pFilterName,
    Sequence<Any> rFilterArguments,
    const OUString& rName,
    sal_Bool bMustBeSuccessfull,
    sal_Bool bEncrypted )
{
    DBG_ASSERT(xInputStream.is(), "input stream missing");
    DBG_ASSERT(xModelComponent.is(), "document missing");
    DBG_ASSERT(rFactory.is(), "factory missing");
    DBG_ASSERT(NULL != pFilterName,"I need a service name for the component!");

    RTL_LOGFILE_CONTEXT( aLog, "ReadThroughComponent" );

    // prepare ParserInputSrouce
    xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = rName;
    aParserInput.aInputStream = xInputStream;

    // get parser
    Reference< xml::sax::XParser > xParser(
        rFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
        UNO_QUERY );
    DBG_ASSERT( xParser.is(), "Can't create parser" );
    if( !xParser.is() )
        return SD_XML_READERROR;
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "parser created" );

    // get filter
    Reference< xml::sax::XDocumentHandler > xFilter(
        rFactory->createInstanceWithArguments(
            OUString::createFromAscii(pFilterName), rFilterArguments),
        UNO_QUERY );
    DBG_ASSERT( xFilter.is(), "Can't instantiate filter component." );
    if( !xFilter.is() )
        return SD_XML_READERROR;
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "%s created", pFilterName );

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    // connect model and filter
    Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );
    // finally, parser the stream
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "parsing stream" );
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& r )
    {
        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "SAX parse exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif

        String sErr( String::CreateFromInt32( r.LineNumber ));
        sErr += ',';
        sErr += String::CreateFromInt32( r.ColumnNumber );

        if( rStreamName.Len() )
        {
            return *new TwoStringErrorInfo(
                            (bMustBeSuccessfull ? ERR_FORMAT_FILE_ROWCOL
                                                    : WARN_FORMAT_FILE_ROWCOL),
                            rStreamName, sErr,
                            ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        }
        else
        {
            DBG_ASSERT( bMustBeSuccessfull, "Warnings are not supported" );
            return *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                             ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        }
    }
    catch( xml::sax::SAXException& r )
    {
        (void)r;
        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "SAX exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        return SD_XML_READERROR;
    }
    catch( packages::zip::ZipIOException& r )
    {
        (void)r;
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "Zip exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch( io::IOException& r )
    {
        (void)r;
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "IO exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        return SD_XML_READERROR;
    }
    catch( uno::Exception& r )
    {
        (void)r;
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "uno exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        return SD_XML_READERROR;
    }

    // success!
    return 0;
}

sal_Int32 ReadThroughComponent(
    SvStorage* pStorage,
    Reference<XComponent> xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    Reference<lang::XMultiServiceFactory> & rFactory,
    const sal_Char* pFilterName,
    Sequence<Any> rFilterArguments,
    const OUString& rName,
    sal_Bool bMustBeSuccessfull )
{
    DBG_ASSERT(NULL != pStorage, "Need storage!");
    DBG_ASSERT(NULL != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    if (! pStorage->IsStream(sStreamName))
    {
        // stream name not found! Then try the compatibility name.
        // if no stream can be opened, return immediatly with OK signal

        // do we even have an alternative name?
        if ( NULL == pCompatibilityStreamName )
            return 0;

        // if so, does the stream exist?
        sStreamName = OUString::createFromAscii(pCompatibilityStreamName);
        if (! pStorage->IsStream(sStreamName) )
            return 0;
    }

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( rFilterArguments.getLength() > 0 )
        rFilterArguments.getConstArray()[0] >>= xInfoSet;
    DBG_ASSERT( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamName") );
        xInfoSet->setPropertyValue( sPropName, makeAny( sStreamName ) );
    }

    // get input stream
    SvStorageStreamRef xEventsStream;
    xEventsStream = pStorage->OpenStream( sStreamName,
                                          STREAM_READ | STREAM_NOCREATE );
    Any aAny;
    sal_Bool bEncrypted =
        xEventsStream->GetProperty(
                OUString( RTL_CONSTASCII_USTRINGPARAM("Encrypted") ), aAny ) &&
        aAny.getValueType() == ::getBooleanCppuType() &&
        *(sal_Bool *)aAny.getValue();

    Reference<io::XInputStream> xInputStream = xEventsStream->GetXInputStream();

    // read from the stream
    return ReadThroughComponent(
        xInputStream, xModelComponent, sStreamName, rFactory,
        pFilterName, rFilterArguments,
        rName, bMustBeSuccessfull, bEncrypted );
}

// -----------------------------------------------------------------------------

sal_Bool SdXMLFilter::Import()
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sd", "cl93746", "SdXMLFilter::Import" );
#ifdef TIMELOG
    ByteString aFile( mrMedium.GetName(), RTL_TEXTENCODING_ASCII_US );
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "importing %s", aFile.GetBuffer() );
#endif

    sal_uInt32  nRet = 0;

    // Get service factory
    Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
        return sal_False;

    // -------------------------------------

    SdDrawDocument* pDoc = mrDocShell.GetDoc();
    pDoc->NewOrLoadCompleted( NEW_DOC );
    pDoc->CreateFirstPages();
    pDoc->StopWorkStartupDelay();

    // -------------------------------------

    mxModel->lockControllers();

    // -------------------------------------

    /** property map for export info set */
    PropertyMapEntry aImportInfoMap[] =
    {
        // #80365# necessary properties for XML progress bar at load time
        { MAP_LEN( "ProgressRange" ),   0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "ProgressMax" ),     0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "Preview" ),         0, &::getCppuType((const sal_Bool*)0),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "PageLayouts" ), 0, SEQTYPE(::getCppuType((const uno::Reference< container::XNameAccess >*)0)),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
        { MAP_LEN( "PrivateData" ), 0,
              &::getCppuType( (Reference<XInterface> *)0 ),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "BaseURI" ), 0,
              &::getCppuType( (OUString *)0 ),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamRelPath" ), 0,
              &::getCppuType( (OUString *)0 ),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamName" ), 0,
              &::getCppuType( (OUString *)0 ),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { NULL, 0, 0, NULL, 0, 0 }
    };

    uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
    xInfoSet->setPropertyValue( OUString::createFromAscii( "Preview" ), uno::makeAny( mrDocShell.GetDoc()->IsStarDrawPreviewMode() ) );

    // -------------------------------------

    Reference< io::XActiveDataSource > xSource;
    Reference< XInterface > xPipe;
    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;
    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    Reference< lang::XComponent > xModelComp( mxModel, uno::UNO_QUERY );

    // -------------------------------------

    // #80365# try to get an XStatusIndicator from the Medium
    if( mbShowProgress )
    {
        SfxItemSet* pSet = mrMedium.GetItemSet();
        if(pSet)
        {
            const SfxUnoAnyItem* pItem = static_cast<const SfxUnoAnyItem*>(
                pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );
            if (pItem)
            {
                pItem->GetValue() >>= mxStatusIndicator;
            }
        }

        if(mxStatusIndicator.is())
        {
            sal_Int32 nProgressRange(1000000);
            sal_Int32 nProgressCurrent(0);
            OUString aMsg = String( SdResId( STR_LOAD_DOC ) );
            mxStatusIndicator->start(aMsg, nProgressRange);

            // set ProgressRange
            uno::Any aProgRange;
            aProgRange <<= nProgressRange;
            xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);

            // set ProgressCurrent
            uno::Any aProgCurrent;
            aProgCurrent <<= nProgressCurrent;
            xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressCurrent")), aProgCurrent);
        }
    }

    // -------------------------------------
    // get the input stream (storage or stream)
    // -------------------------------------

    SvStorageStreamRef xDocStream;
    Reference<io::XInputStream> xInputStream;
    SvStorage *pStorage = 0;

    pStorage = mrMedium.GetStorage();

    if( !pStorage )
        nRet = SD_XML_READERROR;

    if( 0 == nRet )
    {
        pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage,
                                                     GRAPHICHELPER_MODE_READ,
                                                     sal_False );

        xGraphicResolver = pGraphicHelper;
        SvPersist *pPersist = pDoc->GetPersist();;
        if( pPersist )
        {
                pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                            *pStorage, *pPersist,
                                            EMBEDDEDOBJECTHELPER_MODE_READ,
                                            sal_False );
            xObjectResolver = pObjectHelper;
        }
    }

    // Set base URI
    OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("BaseURI") );
    xInfoSet->setPropertyValue( sPropName,
                            makeAny( OUString(INetURLObject::GetBaseURL()) ) );
    if( SFX_CREATE_MODE_EMBEDDED == mrDocShell.GetCreateMode() )
    {
        OUString aName( pStorage->GetName() );
        if( aName.getLength() )
        {
            sPropName = OUString(RTL_CONSTASCII_USTRINGPARAM("StreamRelPath"));
            xInfoSet->setPropertyValue( sPropName, makeAny( aName ) );
        }
    }


    // -------------------------------------

    if( 0 == nRet )
    {

        // prepare filter arguments
        Sequence<Any> aFilterArgs( 4 );
        Any *pArgs = aFilterArgs.getArray();
        *pArgs++ <<= xInfoSet;
        *pArgs++ <<= xGraphicResolver;
        *pArgs++ <<= xObjectResolver;
        *pArgs++ <<= mxStatusIndicator;

        Sequence<Any> aEmptyArgs( 2 );
        pArgs = aEmptyArgs.getArray();
        *pArgs++ <<= xInfoSet;
        *pArgs++ <<= mxStatusIndicator;

        const OUString aName( mrMedium.GetName() );

        XML_SERVICES* pServices = getServices( true, IsDraw(), mnStoreVer );

        sal_uInt32 nWarn = 0;
        sal_uInt32 nWarn2 = 0;
        // read storage streams
        if( meFilterMode != SDXMLMODE_Organizer )
        {
            nWarn = ReadThroughComponent(
                pStorage, xModelComp, "meta.xml", "Meta.xml", xServiceFactory,
                pServices->mpMeta,
                aEmptyArgs, aName, sal_False );

            nWarn2 = ReadThroughComponent(
                pStorage, xModelComp, "settings.xml", NULL, xServiceFactory,
                pServices->mpSettings,
                aFilterArgs, aName, sal_False );
        }

        nRet = ReadThroughComponent(
            pStorage, xModelComp, "styles.xml", NULL, xServiceFactory,
            pServices->mpStyles,
            aFilterArgs, aName, sal_True );

        if( !nRet && (meFilterMode != SDXMLMODE_Organizer) )
            nRet = ReadThroughComponent(
               pStorage, xModelComp, "content.xml", "Content.xml", xServiceFactory,
               pServices->mpContent,
               aFilterArgs, aName, sal_True );

        if( !nRet )
        {
            if( nWarn )
                nRet = nWarn;
            else if( nWarn2 )
                nRet = nWarn2;
        }
    }

    // -------------------------------------
    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;
    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;

    if( mxStatusIndicator.is() )
        mxStatusIndicator->end();

    if( mxModel.is() )
        mxModel->unlockControllers();

    if( nRet == 0 )
        pDoc->UpdateAllLinks();

    switch( nRet )
    {
    case 0: break;
//  case ERRCODE_SFX_WRONGPASSWORD: break;
    case SD_XML_READERROR: break;
    case ERRCODE_IO_BROKENPACKAGE:
        if( pStorage )
        {
            pStorage->SetError( ERRCODE_IO_BROKENPACKAGE );
            break;
        }
        // fall through intented
    default:
        {
            ErrorHandler::HandleError( nRet );
            if( IsWarning( nRet ) )
                nRet = 0;
        }
    }

    return nRet == 0;
}

// -----------------------------------------------------------------------------

sal_Bool SdXMLFilter::Export()
{
#ifdef TIMELOG
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sd", "cl93746", "SdXMLFilter::Export" );
    ByteString aFile( mrMedium.GetName(), RTL_TEXTENCODING_ASCII_US );
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "exporting %s", aFile.GetBuffer() );
#endif

    SvXMLEmbeddedObjectHelper*  pObjectHelper = NULL;
    SvXMLGraphicHelper*         pGraphicHelper = NULL;
    sal_Bool                    bDocRet = FALSE;

    try
    {
        if( !mxModel.is() )
        {
            DBG_ERROR("Got NO Model in XMLExport");
            return FALSE;
        }

        uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY );

        if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GenericDrawingDocument" ) ) ) )
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
            // #82003#
            { MAP_LEN( "ProgressRange" ),   0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressMax" ),     0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "UsePrettyPrinting"),0, &::getBooleanCppuType(),             ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},

            { MAP_LEN( "PageLayoutNames" ), 0, SEQTYPE(::getCppuType((const OUString*)0)),  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
            { MAP_LEN( "BaseURI" ), 0,
                  &::getCppuType( (OUString *)0 ),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamRelPath" ), 0,
                  &::getCppuType( (OUString *)0 ),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamName" ), 0,
                  &::getCppuType( (OUString *)0 ),
                  ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };

        uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aExportInfoMap ) ) );


        SvtSaveOptions aSaveOpt;
        OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting"));
        sal_Bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
        xInfoSet->setPropertyValue( sUsePrettyPrinting, makeAny( bUsePrettyPrinting ) );

        SvStorage* pStorage = mrMedium.GetOutputStorage( sal_True );

        // Set base URI
        OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("BaseURI") );
        xInfoSet->setPropertyValue( sPropName,
                                makeAny( OUString(INetURLObject::GetBaseURL()) ) );
        if( SFX_CREATE_MODE_EMBEDDED == mrDocShell.GetCreateMode() )
        {
            OUString aName( pStorage->GetName() );
            if( aName.getLength() )
            {
                sPropName = OUString(RTL_CONSTASCII_USTRINGPARAM("StreamRelPath"));
                xInfoSet->setPropertyValue( sPropName, makeAny( aName ) );
            }
        }

        // initialize descriptor
        uno::Sequence< beans::PropertyValue > aDescriptor( 1 );
        beans::PropertyValue* pProps = aDescriptor.getArray();

        pProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
        pProps[0].Value <<= OUString( mrMedium.GetName() );

        {
            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            uno::Reference< document::XGraphicObjectResolver >  xGrfResolver;

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

            // #82003#
            if(mbShowProgress)
            {
                CreateStatusIndicator();
                if(mxStatusIndicator.is())
                {
                    sal_Int32 nProgressRange(1000000);
                    sal_Int32 nProgressCurrent(0);
                    OUString aMsg = String( SdResId( STR_SAVE_DOC ) );
                    mxStatusIndicator->start(aMsg, nProgressRange);

                    // set ProgressRange
                    uno::Any aProgRange;
                    aProgRange <<= nProgressRange;
                    xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);

                    // set ProgressCurrent
                    uno::Any aProgCurrent;
                    aProgCurrent <<= nProgressCurrent;
                    xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressCurrent")), aProgCurrent);
                }
            }

            uno::Reference< lang::XComponent > xComponent( mxModel, uno::UNO_QUERY );

            XML_SERVICES* pServiceNames = getServices( false, IsDraw(), mnStoreVer );

            XML_SERVICEMAP aServices[5]; sal_uInt16 i = 0;
            aServices[i  ].mpService = pServiceNames->mpStyles;
            aServices[i  ].mpStream  = sXML_styleStreamName;
            aServices[i++].mbPlain = sal_False;

            aServices[i  ].mpService = pServiceNames->mpContent;
            aServices[i  ].mpStream  = sXML_contentStreamName;
            aServices[i++].mbPlain = sal_False;

            aServices[i  ].mpService = pServiceNames->mpSettings;
            aServices[i  ].mpStream  = sXML_settingsStreamName;
            aServices[i++].mbPlain = sal_False;

            if( mrDocShell.GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                aServices[i  ].mpService = pServiceNames->mpMeta;
                aServices[i  ].mpStream  = sXML_metaStreamName;
                aServices[i++].mbPlain = sal_True;
            };

            aServices[i].mpService = NULL;
            aServices[i].mpStream  = NULL;

            XML_SERVICEMAP* pServices = aServices;

            // doc export
            do
            {
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "exporting substream %s", pServices->mpStream );

                uno::Reference<io::XOutputStream> xDocOut;
                SvStorageStreamRef xDocStream;

                if( pStorage )
                {
                    const OUString sDocName( OUString::createFromAscii( pServices->mpStream ) );
                    xDocStream = pStorage->OpenStream( sDocName, STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC  );
                    DBG_ASSERT(xDocStream.Is(), "Can't create output stream in package!");
                    if( !xDocStream.Is() )
                        return sal_False;

                    xDocStream->SetVersion( pStorage->GetVersion() );
//                  xDocStream->SetKey( pStorage->GetKey() );
                    xDocStream->SetBufferSize( 16*1024 );
                    xDocOut = new utl::OOutputStreamWrapper( *xDocStream );

                    uno::Any aAny; aAny <<= OUString( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
                    xDocStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), aAny);

                    if( pServices->mbPlain )
                    {
                        xDocStream->SetProperty( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Compressed") ), uno::makeAny( (sal_Bool) sal_False ) );
                    }
                    else
                    {
                        xDocStream->SetProperty( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Encrypted") ), uno::makeAny( (sal_Bool)sal_True ) );
                    }

                    OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamName") );
                    xInfoSet->setPropertyValue( sPropName, makeAny( sDocName ) );
                }

                uno::Reference< io::XActiveDataSource > xDocSrc( xWriter, uno::UNO_QUERY );
                xDocSrc->setOutputStream( xDocOut );

                uno::Sequence< uno::Any > aArgs( 2 + ( mxStatusIndicator.is() ? 1 : 0 ) + ( xGrfResolver.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                uno::Any* pArgs = aArgs.getArray();
                *pArgs++ <<= xInfoSet;
                if( xGrfResolver.is() )         *pArgs++ <<= xGrfResolver;
                if( xObjectResolver.is() )      *pArgs++ <<= xObjectResolver;
                if( mxStatusIndicator.is() )    *pArgs++ <<= mxStatusIndicator;

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

            // #82003#
            if(mbShowProgress)
            {
                if(mxStatusIndicator.is())
                    mxStatusIndicator->end();
            }
        }
    }
    catch(uno::Exception e)
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "uno Exception caught while exporting:\n" );
        aError += ByteString( String( e.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        bDocRet = sal_False;
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );


    return bDocRet;
}
