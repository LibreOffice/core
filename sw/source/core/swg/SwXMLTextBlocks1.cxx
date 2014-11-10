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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <svl/macitem.hxx>
#include <svtools/unoevent.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <doc.hxx>
#include <docsh.hxx>
#include <shellio.hxx>
#include <SwXMLTextBlocks.hxx>
#include <SwXMLBlockImport.hxx>
#include <SwXMLBlockExport.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <swevent.hxx>
#include <swerror.h>

const char XMLN_BLOCKLIST[] = "BlockList.xml";

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace css::xml::sax;
using namespace xmloff::token;

using ::xmloff::token::XML_BLOCK_LIST;
using ::xmloff::token::XML_UNFORMATTED_TEXT;
using ::xmloff::token::GetXMLToken;

sal_uLong SwXMLTextBlocks::GetDoc( sal_uInt16 nIdx )
{
    OUString aFolderName ( GetPackageName ( nIdx ) );

    if (!IsOnlyTextBlock ( nIdx ) )
    {
        try
        {
            xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::READ );
            xMedium = new SfxMedium( xRoot, GetBaseURL(), OUString( "writer8" ) );
            SwReader aReader( *xMedium, aFolderName, pDoc );
            ReadXML->SetBlockMode( true );
            aReader.Read( *ReadXML );
            ReadXML->SetBlockMode( false );
            // Ole objects fail to display when inserted into the document, as
            // the ObjectReplacement folder and contents are missing
            OUString sObjReplacements( "ObjectReplacements" );
            if ( xRoot->hasByName( sObjReplacements ) )
            {
                uno::Reference< document::XStorageBasedDocument > xDocStor( pDoc->GetDocShell()->GetModel(), uno::UNO_QUERY_THROW );
                uno::Reference< embed::XStorage > xStr( xDocStor->getDocumentStorage() );
                if ( xStr.is() )
                {
                    xRoot->copyElementTo( sObjReplacements, xStr, sObjReplacements );
                    uno::Reference< embed::XTransactedObject > xTrans( xStr, uno::UNO_QUERY );
                    if ( xTrans.is() )
                        xTrans->commit();
                }
            }
        }
        catch( uno::Exception& )
        {
        }

        xRoot = 0;
    }
    else
    {
        OUString aStreamName = aFolderName + ".xml";
        try
        {
            xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::READ );
            uno::Reference < io::XStream > xStream = xRoot->openStreamElement( aStreamName, embed::ElementModes::READ );

            uno::Reference< uno::XComponentContext > xContext =
                comphelper::getProcessComponentContext();

            xml::sax::InputSource aParserInput;
            aParserInput.sSystemId = aNames[nIdx]->aPackageName;

            aParserInput.aInputStream = xStream->getInputStream();

            // get filter
            // uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this, aCur, sal_True );
            uno::Reference< xml::sax::XFastDocumentHandler > xFilter = new SwXMLTextBlockImport( xContext, *this, aCur, true );
            uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler = new SwXMLTextBlockTokenHandler();

            // connect parser and filter
            uno::Reference< xml::sax::XFastParser > xParser = xml::sax::FastParser::create(xContext);
            xParser->setFastDocumentHandler( xFilter );
            xParser->setTokenHandler( xTokenHandler );

            // all namespaces are mentioned in content.xml but only a few of them are handled
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:office:1.0", FastToken::NAMESPACE | XML_NAMESPACE_OFFICE );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:style:1.0", FastToken::NAMESPACE | XML_NAMESPACE_STYLE );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:text:1.0", FastToken::NAMESPACE | XML_NAMESPACE_TEXT );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:table:1.0", FastToken::NAMESPACE | XML_NAMESPACE_TABLE );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", FastToken::NAMESPACE | XML_NAMESPACE_DRAW );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0", FastToken::NAMESPACE | XML_NAMESPACE_FO );
            xParser->registerNamespace( "http://www.w3.org/1999/xlink", FastToken::NAMESPACE | XML_NAMESPACE_XLINK );
            xParser->registerNamespace( "http://purl.org/dc/elements/1.1/", FastToken::NAMESPACE | XML_NAMESPACE_DC );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:meta:1.0", FastToken::NAMESPACE | XML_NAMESPACE_META );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0", FastToken::NAMESPACE | XML_NAMESPACE_NUMBER );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", FastToken::NAMESPACE | XML_NAMESPACE_SVG );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:chart:1.0", FastToken::NAMESPACE | XML_NAMESPACE_CHART );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0", FastToken::NAMESPACE | XML_NAMESPACE_DR3D );
            xParser->registerNamespace( "http://www.w3.org/1998/Math/MathML", FastToken::NAMESPACE | XML_NAMESPACE_MATH );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:form:1.0", FastToken::NAMESPACE | XML_NAMESPACE_FORM );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:script:1.0", FastToken::NAMESPACE | XML_NAMESPACE_SCRIPT );
            xParser->registerNamespace( "http://openoffice.org/2004/office", FastToken::NAMESPACE | XML_NAMESPACE_OOO );
            xParser->registerNamespace( "http://openoffice.org/2004/writer", FastToken::NAMESPACE | XML_NAMESPACE_OOOW );
            xParser->registerNamespace( "http://openoffice.org/2004/calc", FastToken::NAMESPACE | XML_NAMESPACE_OOOC );
            xParser->registerNamespace( "http://www.w3.org/2001/xml-events", FastToken::NAMESPACE | XML_NAMESPACE_DOM );
            xParser->registerNamespace( "http://openoffice.org/2005/report", FastToken::NAMESPACE | XML_NAMESPACE_REPORT );
            xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:of:1.2", FastToken::NAMESPACE | XML_NAMESPACE_OF );
            xParser->registerNamespace( "http://www.w3.org/1999/xhtml", FastToken::NAMESPACE | XML_NAMESPACE_XHTML );
            xParser->registerNamespace( "http://www.w3.org/2003/g/data-view#", FastToken::NAMESPACE | XML_NAMESPACE_GRDDL );
            xParser->registerNamespace( "http://openoffice.org/2009/office", FastToken::NAMESPACE | XML_NAMESPACE_OFFICE_EXT );
            xParser->registerNamespace( "http://openoffice.org/2009/table", FastToken::NAMESPACE | XML_NAMESPACE_TABLE_EXT );
            xParser->registerNamespace( "http://openoffice.org/2009/draw", FastToken::NAMESPACE | XML_NAMESPACE_DRAW_EXT );
            xParser->registerNamespace( "urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0", FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT );
            xParser->registerNamespace( "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0", FastToken::NAMESPACE | XML_NAMESPACE_LO_EXT );

            // parse
            try
            {
                xParser->parseStream( aParserInput );
            }
            catch( xml::sax::SAXParseException&  )
            {
                // re throw ?
            }
            catch( xml::sax::SAXException&  )
            {
                // re throw ?
            }
            catch( io::IOException& )
            {
                // re throw ?
            }

            bInfoChanged = false;
            MakeBlockText(aCur);
        }
        catch( uno::Exception& )
        {
        }

        xRoot = 0;
    }
    return 0;
}

// event description for autotext events; this constant should really be
// taken from unocore/unoevents.cxx or ui/unotxt.cxx
const struct SvEventDescription aAutotextEvents[] =
{
    { SW_EVENT_START_INS_GLOSSARY,  "OnInsertStart" },
    { SW_EVENT_END_INS_GLOSSARY,    "OnInsertDone" },
    { 0, NULL }
};

sal_uLong SwXMLTextBlocks::GetMacroTable( sal_uInt16 nIdx,
                                      SvxMacroTableDtor& rMacroTbl,
                                      bool bFileAlreadyOpen )
{
    // set current auto text
    aShort = aNames[nIdx]->aShort;
    aLong = aNames[nIdx]->aLong;
    aPackageName = aNames[nIdx]->aPackageName;

    sal_uLong nRet = 0;

    // open stream in proper sub-storage
    if( !bFileAlreadyOpen )
    {
        CloseFile();
        nRet = OpenFile ( true );
    }
    if ( 0 == nRet )
    {
        try
        {
            xRoot = xBlkRoot->openStorageElement( aPackageName, embed::ElementModes::READ );
            long nTmp = SOT_FORMATSTR_ID_STARWRITER_60;
            bool bOasis = ( SotStorage::GetVersion( xRoot ) > nTmp );

            OUString sStreamName("atevent.xml");
            uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement(
                sStreamName, embed::ElementModes::READ );
            OSL_ENSURE(xDocStream.is(), "Can't create stream");
            if ( xDocStream.is() )
            {
                uno::Reference<io::XInputStream> xInputStream = xDocStream->getInputStream();

                // prepare ParserInputSrouce
                xml::sax::InputSource aParserInput;
                aParserInput.sSystemId = aName;
                aParserInput.aInputStream = xInputStream;

                // get service factory
                uno::Reference< uno::XComponentContext > xContext =
                    comphelper::getProcessComponentContext();

                // get parser
                uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

                // create descriptor and reference to it. Either
                // both or neither must be kept because of the
                // reference counting!
                SvMacroTableEventDescriptor* pDescriptor =
                    new SvMacroTableEventDescriptor(aAutotextEvents);
                uno::Reference<XNameReplace> xReplace = pDescriptor;
                Sequence<Any> aFilterArguments( 1 );
                aFilterArguments[0] <<= xReplace;

                // get filter
                OUString sFilterComponent = bOasis
                    ? OUString("com.sun.star.comp.Writer.XMLOasisAutotextEventsImporter")
                    : OUString("com.sun.star.comp.Writer.XMLAutotextEventsImporter");
                uno::Reference< xml::sax::XDocumentHandler > xFilter(
                    xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        sFilterComponent, aFilterArguments, xContext),
                    UNO_QUERY );
                OSL_ENSURE( xFilter.is(),
                            "can't instantiate atevents filter");
                if ( xFilter.is() )
                {
                    // connect parser and filter
                    xParser->setDocumentHandler( xFilter );

                    // connect model and filter
                    uno::Reference<document::XImporter> xImporter( xFilter, UNO_QUERY );

                    // we don't need a model
                    // xImporter->setTargetDocument( xModelComponent );

                    // parse the stream
                    try
                    {
                        xParser->parseStream( aParserInput );
                    }
                    catch( xml::sax::SAXParseException& )
                    {
                        // workaround for #83452#: SetSize doesn't work
                        // nRet = ERR_SWG_READ_ERROR;
                    }
                    catch( xml::sax::SAXException& )
                    {
                        nRet = ERR_SWG_READ_ERROR;
                    }
                    catch( io::IOException& )
                    {
                        nRet = ERR_SWG_READ_ERROR;
                    }

                    // and finally, copy macro into table
                    if (0 == nRet)
                        pDescriptor->copyMacrosIntoTable(rMacroTbl);
                }
                else
                    nRet = ERR_SWG_READ_ERROR;
            }
            else
                nRet = ERR_SWG_READ_ERROR;
        }
        catch( uno::Exception& )
        {
            nRet = ERR_SWG_READ_ERROR;
        }
    }
    else
        nRet = ERR_SWG_READ_ERROR;

    // success!
    return nRet;
}

sal_uLong SwXMLTextBlocks::GetBlockText( const OUString& rShort, OUString& rText )
{
    sal_uLong n = 0;
    bool bTextOnly = true;
    OUString aFolderName = GeneratePackageName ( rShort );
    OUString aStreamName = aFolderName + ".xml";
    rText = "";

    try
    {
        xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::READ );
        uno::Reference < container::XNameAccess > xAccess( xRoot, uno::UNO_QUERY );
        if ( !xAccess->hasByName( aStreamName ) || !xRoot->isStreamElement( aStreamName ) )
        {
            bTextOnly = false;
            aStreamName = "content.xml";
        }

        uno::Reference < io::XStream > xContents = xRoot->openStreamElement( aStreamName, embed::ElementModes::READ );
        uno::Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aName;
        aParserInput.aInputStream = xContents->getInputStream();

        // get filter
        // #110680#
        // uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this, rText, bTextOnly );
        uno::Reference< xml::sax::XFastDocumentHandler > xFilter = new SwXMLTextBlockImport( xContext, *this, rText, bTextOnly );
        uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler = new SwXMLTextBlockTokenHandler();

        // connect parser and filter
        uno::Reference< xml::sax::XFastParser > xParser = xml::sax::FastParser::create(xContext);
        xParser->setFastDocumentHandler( xFilter );
        xParser->setTokenHandler( xTokenHandler );
        // all namespaces are mentioned in content.xml but only a few of them are handled
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:office:1.0", FastToken::NAMESPACE | XML_NAMESPACE_OFFICE );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:style:1.0", FastToken::NAMESPACE | XML_NAMESPACE_STYLE );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:text:1.0", FastToken::NAMESPACE | XML_NAMESPACE_TEXT );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:table:1.0", FastToken::NAMESPACE | XML_NAMESPACE_TABLE );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", FastToken::NAMESPACE | XML_NAMESPACE_DRAW );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0", FastToken::NAMESPACE | XML_NAMESPACE_FO );
        xParser->registerNamespace( "http://www.w3.org/1999/xlink", FastToken::NAMESPACE | XML_NAMESPACE_XLINK );
        xParser->registerNamespace( "http://purl.org/dc/elements/1.1/", FastToken::NAMESPACE | XML_NAMESPACE_DC );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:meta:1.0", FastToken::NAMESPACE | XML_NAMESPACE_META );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0", FastToken::NAMESPACE | XML_NAMESPACE_NUMBER );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", FastToken::NAMESPACE | XML_NAMESPACE_SVG );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:chart:1.0", FastToken::NAMESPACE | XML_NAMESPACE_CHART );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0", FastToken::NAMESPACE | XML_NAMESPACE_DR3D );
        xParser->registerNamespace( "http://www.w3.org/1998/Math/MathML", FastToken::NAMESPACE | XML_NAMESPACE_MATH );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:form:1.0", FastToken::NAMESPACE | XML_NAMESPACE_FORM );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:script:1.0", FastToken::NAMESPACE | XML_NAMESPACE_SCRIPT );
        xParser->registerNamespace( "http://openoffice.org/2004/office", FastToken::NAMESPACE | XML_NAMESPACE_OOO );
        xParser->registerNamespace( "http://openoffice.org/2004/writer", FastToken::NAMESPACE | XML_NAMESPACE_OOOW );
        xParser->registerNamespace( "http://openoffice.org/2004/calc", FastToken::NAMESPACE | XML_NAMESPACE_OOOC );
        xParser->registerNamespace( "http://www.w3.org/2001/xml-events", FastToken::NAMESPACE | XML_NAMESPACE_DOM );
        xParser->registerNamespace( "http://openoffice.org/2005/report", FastToken::NAMESPACE | XML_NAMESPACE_REPORT );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:of:1.2", FastToken::NAMESPACE | XML_NAMESPACE_OF );
        xParser->registerNamespace( "http://www.w3.org/1999/xhtml", FastToken::NAMESPACE | XML_NAMESPACE_XHTML );
        xParser->registerNamespace( "http://www.w3.org/2003/g/data-view#", FastToken::NAMESPACE | XML_NAMESPACE_GRDDL );
        xParser->registerNamespace( "http://openoffice.org/2009/office", FastToken::NAMESPACE | XML_NAMESPACE_OFFICE_EXT );
        xParser->registerNamespace( "http://openoffice.org/2009/table", FastToken::NAMESPACE | XML_NAMESPACE_TABLE_EXT );
        xParser->registerNamespace( "http://openoffice.org/2009/draw", FastToken::NAMESPACE | XML_NAMESPACE_DRAW_EXT );
        xParser->registerNamespace( "urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0", FastToken::NAMESPACE | XML_NAMESPACE_CALC_EXT );
        xParser->registerNamespace( "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0", FastToken::NAMESPACE | XML_NAMESPACE_LO_EXT );
        xParser->registerNamespace( "http://www.w3.org/TR/css3-text/", FastToken::NAMESPACE | XML_NAMESPACE_CSS3TEXT );

        // parse
        try
        {
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException&  )
        {
            // re throw ?
        }
        catch( xml::sax::SAXException&  )
        {
            // re throw ?
        }
        catch( io::IOException& )
        {
            // re throw ?
        }

        xRoot = 0;
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "Tried to open non-existent folder or stream!");
    }

    return n;
}

sal_uLong SwXMLTextBlocks::PutBlockText( const OUString& rShort, const OUString& ,
                                         const OUString& rText,  const OUString& rPackageName )
{
    GetIndex ( rShort );
    /*
    if (xBlkRoot->IsContained ( rPackageName ) )
    {
        xBlkRoot->Remove ( rPackageName );
        xBlkRoot->Commit ( );
    }
    */
    OUString aFolderName( rPackageName );
    OUString aStreamName = aFolderName + ".xml";

    uno::Reference< uno::XComponentContext > xContext =
        comphelper::getProcessComponentContext();

    uno::Reference < xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);
    sal_uLong nRes = 0;

    try
    {
    xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::WRITE );
    uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement( aStreamName,
                embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

    uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
    OUString aMime ( "text/xml" );
    Any aAny;
    aAny <<= aMime;
    xSet->setPropertyValue("MediaType", aAny );
    uno::Reference < io::XOutputStream > xOut = xDocStream->getOutputStream();
       uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
       xSrc->setOutputStream(xOut);

       uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter,
        uno::UNO_QUERY);

   SwXMLTextBlockExport aExp( xContext, *this, GetXMLToken ( XML_UNFORMATTED_TEXT ), xHandler);

    aExp.exportDoc( rText );

    uno::Reference < embed::XTransactedObject > xTrans( xRoot, uno::UNO_QUERY );
    if ( xTrans.is() )
        xTrans->commit();

    if (! (nFlags & SWXML_NOROOTCOMMIT) )
    {
        uno::Reference < embed::XTransactedObject > xTmpTrans( xBlkRoot, uno::UNO_QUERY );
        if ( xTmpTrans.is() )
            xTmpTrans->commit();
    }
    }
    catch ( uno::Exception& )
    {
        nRes = ERR_SWG_WRITE_ERROR;
    }

    xRoot = 0;

    //TODO/LATER: error handling
    /*
    sal_uLong nErr = xBlkRoot->GetError();
    sal_uLong nRes = 0;
    if( nErr == SVSTREAM_DISK_FULL )
        nRes = ERR_W4W_WRITE_FULL;
    else if( nErr != SVSTREAM_OK )
        nRes = ERR_SWG_WRITE_ERROR;
    */
    if( !nRes ) // So that we can access the Doc via GetText & nCur
        MakeBlockText( rText );

    return nRes;
}

void SwXMLTextBlocks::ReadInfo( void )
{
    try
    {
    const OUString sDocName( XMLN_BLOCKLIST );
    uno::Reference < container::XNameAccess > xAccess( xBlkRoot, uno::UNO_QUERY );
    if ( xAccess.is() && xAccess->hasByName( sDocName ) && xBlkRoot->isStreamElement( sDocName ) )
    {
        uno::Reference< uno::XComponentContext > xContext =
                comphelper::getProcessComponentContext();

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = sDocName;

        uno::Reference < io::XStream > xDocStream = xBlkRoot->openStreamElement( sDocName, embed::ElementModes::READ );
        aParserInput.aInputStream = xDocStream->getInputStream();

        // get filter
        uno::Reference< xml::sax::XFastDocumentHandler > xFilter = new SwXMLBlockListImport( xContext, *this );
        uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler = new SwXMLBlockListTokenHandler();

        // connect parser and filter
        uno::Reference< xml::sax::XFastParser > xParser = xml::sax::FastParser::create(xContext);
        xParser->setFastDocumentHandler( xFilter );
        xParser->registerNamespace( "http://openoffice.org/2001/block-list", FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST );
        xParser->setTokenHandler( xTokenHandler );

        // parse
        try
        {
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException&  )
        {
            // re throw ?
        }
        catch( xml::sax::SAXException&  )
        {
            // re throw ?
        }
        catch( io::IOException& )
        {
            // re throw ?
        }
    }
    }
    catch ( uno::Exception& )
    {
    }
}
void SwXMLTextBlocks::WriteInfo( void )
{
    if ( xBlkRoot.is() || 0 == OpenFile ( false ) )
    {
        uno::Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();

        uno::Reference < xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);
        OUString sDocName( XMLN_BLOCKLIST );

        /*
        if ( xBlkRoot->IsContained( sDocName) )
        {
            xBlkRoot->Remove ( sDocName );
            xBlkRoot->Commit();
        }
        */

        try
        {
        uno::Reference < io::XStream > xDocStream = xBlkRoot->openStreamElement( sDocName,
                    embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

        uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
        OUString aMime ( "text/xml" );
        Any aAny;
        aAny <<= aMime;
        xSet->setPropertyValue("MediaType", aAny );
        uno::Reference < io::XOutputStream > xOut = xDocStream->getOutputStream();
        uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
        xSrc->setOutputStream(xOut);

        uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

        SwXMLBlockListExport aExp( xContext, *this, OUString(XMLN_BLOCKLIST), xHandler);

        aExp.exportDoc( XML_BLOCK_LIST );

        uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
        if ( xTrans.is() )
            xTrans->commit();
        }
        catch ( uno::Exception& )
        {
        }

        bInfoChanged = false;
        return;
    }
}

sal_uLong SwXMLTextBlocks::SetMacroTable(
    sal_uInt16 nIdx,
    const SvxMacroTableDtor& rMacroTbl,
    bool bFileAlreadyOpen )
{
    // set current autotext
    aShort = aNames[nIdx]->aShort;
    aLong = aNames[nIdx]->aLong;
    aPackageName = aNames[nIdx]->aPackageName;

    // start XML autotext event export
    sal_uLong nRes = 0;

    uno::Reference< uno::XComponentContext > xContext =
        comphelper::getProcessComponentContext();

    // Get model
    uno::Reference< lang::XComponent > xModelComp(
        pDoc->GetDocShell()->GetModel(), UNO_QUERY );
    OSL_ENSURE( xModelComp.is(), "XMLWriter::Write: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_WRITE_ERROR;

    // open stream in proper sub-storage
    if( !bFileAlreadyOpen )
    {
        CloseFile(); // close (it may be open in read-only-mode)
        nRes = OpenFile ( false );
    }

    if ( 0 == nRes )
    {
        try
        {
            xRoot = xBlkRoot->openStorageElement( aPackageName, embed::ElementModes::WRITE );
            OUString sStreamName("atevent.xml" );
            long nTmp = SOT_FORMATSTR_ID_STARWRITER_60;
            bool bOasis = ( SotStorage::GetVersion( xRoot ) > nTmp );

            uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement( sStreamName,
                        embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

            uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
            OUString aMime( "text/xml" );
            Any aAny;
            aAny <<= aMime;
            xSet->setPropertyValue("MediaType", aAny );
            uno::Reference < io::XOutputStream > xOutputStream = xDocStream->getOutputStream();

            // get XML writer
            uno::Reference< xml::sax::XWriter > xSaxWriter =
                xml::sax::Writer::create( xContext );

            // connect XML writer to output stream
            xSaxWriter->setOutputStream( xOutputStream );
            uno::Reference<xml::sax::XDocumentHandler> xDocHandler(
                xSaxWriter, UNO_QUERY);

            // construct events object
            uno::Reference<XNameAccess> xEvents =
                new SvMacroTableEventDescriptor(rMacroTbl,aAutotextEvents);

            // prepare arguments (prepend doc handler to given arguments)
            Sequence<Any> aParams(2);
            aParams[0] <<= xDocHandler;
            aParams[1] <<= xEvents;

            // get filter component
            OUString sFilterComponent = bOasis
                ? OUString("com.sun.star.comp.Writer.XMLOasisAutotextEventsExporter")
                : OUString("com.sun.star.comp.Writer.XMLAutotextEventsExporter");
            uno::Reference< document::XExporter > xExporter(
                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    sFilterComponent, aParams, xContext), UNO_QUERY);
            OSL_ENSURE( xExporter.is(),
                    "can't instantiate export filter component" );
            if( xExporter.is() )
            {
                // connect model and filter
                xExporter->setSourceDocument( xModelComp );

                // filter!
                Sequence<beans::PropertyValue> aFilterProps( 0 );
                uno::Reference < document::XFilter > xFilter( xExporter,
                                                         UNO_QUERY );
                xFilter->filter( aFilterProps );
            }
            else
                nRes = ERR_SWG_WRITE_ERROR;

            // finally, commit stream, sub-storage and storage
            uno::Reference < embed::XTransactedObject > xTmpTrans( xRoot, uno::UNO_QUERY );
            if ( xTmpTrans.is() )
                xTmpTrans->commit();

            if ( !bFileAlreadyOpen )
            {
                uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
                if ( xTrans.is() )
                    xTrans->commit();
            }

            xRoot = 0;
        }
        catch ( uno::Exception& )
        {
            nRes = ERR_SWG_WRITE_ERROR;
        }

        if( !bFileAlreadyOpen )
            CloseFile();
    }
    else
        nRes = ERR_SWG_WRITE_ERROR;

    return nRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
