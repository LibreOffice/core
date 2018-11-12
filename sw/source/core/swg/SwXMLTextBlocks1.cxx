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
#include <osl/diagnose.h>
#include <svl/macitem.hxx>
#include <svtools/unoevent.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <doc.hxx>
#include <docsh.hxx>
#include <shellio.hxx>
#include <SwXMLTextBlocks.hxx>
#include <SwXMLBlockImport.hxx>
#include <SwXMLBlockExport.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <sfx2/event.hxx>
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

ErrCode SwXMLTextBlocks::GetDoc( sal_uInt16 nIdx )
{
    OUString aFolderName ( GetPackageName ( nIdx ) );

    if (!IsOnlyTextBlock ( nIdx ) )
    {
        try
        {
            xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::READ );
            xMedium = new SfxMedium( xRoot, GetBaseURL(), OUString( "writer8" ) );
            SwReader aReader( *xMedium, aFolderName, m_xDoc.get() );
            ReadXML->SetBlockMode( true );
            aReader.Read( *ReadXML );
            ReadXML->SetBlockMode( false );
            // Ole objects fail to display when inserted into the document, as
            // the ObjectReplacement folder and contents are missing
            OUString sObjReplacements( "ObjectReplacements" );
            if ( xRoot->hasByName( sObjReplacements ) )
            {
                uno::Reference< document::XStorageBasedDocument > xDocStor( m_xDoc->GetDocShell()->GetModel(), uno::UNO_QUERY_THROW );
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

        xRoot = nullptr;
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
            aParserInput.sSystemId = m_aNames[nIdx]->aPackageName;

            aParserInput.aInputStream = xStream->getInputStream();

            // get filter
            uno::Reference< xml::sax::XFastDocumentHandler > xFilter = new SwXMLTextBlockImport( xContext, m_aCurrentText, true );
            uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler = new SwXMLTextBlockTokenHandler();

            // connect parser and filter
            uno::Reference< xml::sax::XFastParser > xParser = xml::sax::FastParser::create(xContext);
            xParser->setFastDocumentHandler( xFilter );
            xParser->setTokenHandler( xTokenHandler );

            xParser->registerNamespace( "http://openoffice.org/2000/text", FastToken::NAMESPACE | XML_NAMESPACE_TEXT );
            xParser->registerNamespace( "http://openoffice.org/2000/office", FastToken::NAMESPACE | XML_NAMESPACE_OFFICE );

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

            m_bInfoChanged = false;
            MakeBlockText(m_aCurrentText);
        }
        catch( uno::Exception& )
        {
        }

        xRoot = nullptr;
    }
    return ERRCODE_NONE;
}

// event description for autotext events; this constant should really be
// taken from unocore/unoevents.cxx or ui/unotxt.cxx
const struct SvEventDescription aAutotextEvents[] =
{
    { SvMacroItemId::SwStartInsGlossary,  "OnInsertStart" },
    { SvMacroItemId::SwEndInsGlossary,    "OnInsertDone" },
    { SvMacroItemId::NONE, nullptr }
};

ErrCode SwXMLTextBlocks::GetMacroTable( sal_uInt16 nIdx,
                                      SvxMacroTableDtor& rMacroTable )
{
    // set current auto text
    m_aShort = m_aNames[nIdx]->aShort;
    m_aLong = m_aNames[nIdx]->aLong;
    aPackageName = m_aNames[nIdx]->aPackageName;

    ErrCode nRet = ERRCODE_NONE;

    // open stream in proper sub-storage
    CloseFile();
    nRet = OpenFile();
    if ( ERRCODE_NONE == nRet )
    {
        try
        {
            xRoot = xBlkRoot->openStorageElement( aPackageName, embed::ElementModes::READ );
            bool bOasis = SotStorage::GetVersion( xRoot ) > SOFFICE_FILEFORMAT_60;

            uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement(
                "atevent.xml", embed::ElementModes::READ );
            OSL_ENSURE(xDocStream.is(), "Can't create stream");
            if ( xDocStream.is() )
            {
                uno::Reference<io::XInputStream> xInputStream = xDocStream->getInputStream();

                // prepare ParserInputSrouce
                xml::sax::InputSource aParserInput;
                aParserInput.sSystemId = m_aName;
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
                            "can't instantiate atevent filter");
                if ( xFilter.is() )
                {
                    // connect parser and filter
                    xParser->setDocumentHandler( xFilter );

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
                    if (ERRCODE_NONE == nRet)
                        pDescriptor->copyMacrosIntoTable(rMacroTable);
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

ErrCode SwXMLTextBlocks::GetBlockText( const OUString& rShort, OUString& rText )
{
    OUString aFolderName = GeneratePackageName ( rShort );
    OUString aStreamName = aFolderName + ".xml";
    rText.clear();

    try
    {
        bool bTextOnly = true;

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
        aParserInput.sSystemId = m_aName;
        aParserInput.aInputStream = xContents->getInputStream();

        // get filter
        uno::Reference< xml::sax::XFastDocumentHandler > xFilter = new SwXMLTextBlockImport( xContext, rText, bTextOnly );
        uno::Reference< xml::sax::XFastTokenHandler > xTokenHandler = new SwXMLTextBlockTokenHandler();

        // connect parser and filter
        uno::Reference< xml::sax::XFastParser > xParser = xml::sax::FastParser::create(xContext);
        xParser->setFastDocumentHandler( xFilter );
        xParser->setTokenHandler( xTokenHandler );

        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:office:1.0", FastToken::NAMESPACE | XML_NAMESPACE_OFFICE );
        xParser->registerNamespace( "urn:oasis:names:tc:opendocument:xmlns:text:1.0", FastToken::NAMESPACE | XML_NAMESPACE_TEXT );

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

        xRoot = nullptr;
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "Tried to open non-existent folder or stream!");
    }

    return ERRCODE_NONE;
}

ErrCode SwXMLTextBlocks::PutBlockText( const OUString& rShort,
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
    OUString aStreamName = rPackageName + ".xml";

    uno::Reference< uno::XComponentContext > xContext =
        comphelper::getProcessComponentContext();

    uno::Reference < xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);
    ErrCode nRes = ERRCODE_NONE;

    try
    {
    xRoot = xBlkRoot->openStorageElement( rPackageName, embed::ElementModes::WRITE );
    uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement( aStreamName,
                embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

    uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
    xSet->setPropertyValue("MediaType", Any(OUString( "text/xml" )) );
    uno::Reference < io::XOutputStream > xOut = xDocStream->getOutputStream();
    uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
    xSrc->setOutputStream(xOut);

    uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter,
        uno::UNO_QUERY);

    rtl::Reference<SwXMLTextBlockExport> xExp( new SwXMLTextBlockExport( xContext, *this, GetXMLToken ( XML_UNFORMATTED_TEXT ), xHandler) );

    xExp->exportDoc( rText );

    uno::Reference < embed::XTransactedObject > xTrans( xRoot, uno::UNO_QUERY );
    if ( xTrans.is() )
        xTrans->commit();

    if (! (nFlags & SwXmlFlags::NoRootCommit) )
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

    xRoot = nullptr;

    //TODO/LATER: error handling
    /*
    sal_uLong nErr = xBlkRoot->GetError();
    sal_uLong nRes = 0;
    if( nErr == SVSTREAM_DISK_FULL )
        nRes = ERR_W4W_WRITE_FULL;
    else if( nErr != ERRCODE_NONE )
        nRes = ERR_SWG_WRITE_ERROR;
    */
    if( !nRes ) // So that we can access the Doc via GetText & nCur
        MakeBlockText( rText );

    return nRes;
}

void SwXMLTextBlocks::ReadInfo()
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
void SwXMLTextBlocks::WriteInfo()
{
    if ( xBlkRoot.is() || ERRCODE_NONE == OpenFile ( false ) )
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
        xSet->setPropertyValue("MediaType", Any(OUString( "text/xml" )) );
        uno::Reference < io::XOutputStream > xOut = xDocStream->getOutputStream();
        uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
        xSrc->setOutputStream(xOut);

        uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

        rtl::Reference<SwXMLBlockListExport> xExp(new SwXMLBlockListExport( xContext, *this, XMLN_BLOCKLIST, xHandler) );

        xExp->exportDoc( XML_BLOCK_LIST );

        uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
        if ( xTrans.is() )
            xTrans->commit();
        }
        catch ( uno::Exception& )
        {
        }

        m_bInfoChanged = false;
        return;
    }
}

ErrCode SwXMLTextBlocks::SetMacroTable(
    sal_uInt16 nIdx,
    const SvxMacroTableDtor& rMacroTable )
{
    // set current autotext
    m_aShort = m_aNames[nIdx]->aShort;
    m_aLong = m_aNames[nIdx]->aLong;
    aPackageName = m_aNames[nIdx]->aPackageName;

    // start XML autotext event export
    ErrCode nRes = ERRCODE_NONE;

    uno::Reference< uno::XComponentContext > xContext =
        comphelper::getProcessComponentContext();

    // Get model
    uno::Reference< lang::XComponent > xModelComp(
        m_xDoc->GetDocShell()->GetModel(), UNO_QUERY );
    OSL_ENSURE( xModelComp.is(), "XMLWriter::Write: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_WRITE_ERROR;

    // open stream in proper sub-storage
    CloseFile(); // close (it may be open in read-only-mode)
    nRes = OpenFile ( false );

    if ( ERRCODE_NONE == nRes )
    {
        try
        {
            xRoot = xBlkRoot->openStorageElement( aPackageName, embed::ElementModes::WRITE );
            bool bOasis = SotStorage::GetVersion( xRoot ) > SOFFICE_FILEFORMAT_60;

            uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement( "atevent.xml",
                        embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

            uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
            xSet->setPropertyValue("MediaType", Any(OUString( "text/xml" )) );
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
                new SvMacroTableEventDescriptor(rMacroTable,aAutotextEvents);

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

            uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
            if ( xTrans.is() )
                xTrans->commit();

            xRoot = nullptr;
        }
        catch ( uno::Exception& )
        {
            nRes = ERR_SWG_WRITE_ERROR;
        }

        CloseFile();
    }
    else
        nRes = ERR_SWG_WRITE_ERROR;

    return nRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
