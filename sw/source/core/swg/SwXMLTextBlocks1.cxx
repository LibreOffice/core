/*************************************************************************
 *
 *  $RCSfile: SwXMLTextBlocks1.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 19:01:23 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#ifndef _SVTOOLS_UNOEVENT_HXX_
#include <svtools/unoevent.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SW_XMLTEXTBLOCKS_HXX
#include <SwXMLTextBlocks.hxx>
#endif
#ifndef _SW_XMLBLOCKIMPORT_HXX
#include <SwXMLBlockImport.hxx>
#endif
#ifndef _SW_XMLBLOCKEXPORT_HXX
#include <SwXMLBlockExport.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _ERRHDL_HXX //autogen wg. ASSERT
#include <errhdl.hxx>
#endif


#define STREAM_STGREAD  ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE )
#define STREAM_STGWRITE ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE )

sal_Char __FAR_DATA XMLN_BLOCKLIST[] = "BlockList.xml";

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::rtl;
using ::xmloff::token::XML_BLOCK_LIST;
using ::xmloff::token::XML_UNFORMATTED_TEXT;
using ::xmloff::token::GetXMLToken;



ULONG SwXMLTextBlocks::GetDoc( USHORT nIdx )
{
    String aFolderName ( GetPackageName ( nIdx ) );

    if (!IsOnlyTextBlock ( nIdx ) )
    {
        xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGREAD );
        SwReader aReader(*xRoot, aFolderName, pDoc );
        ReadXML->SetBlockMode( sal_True );
        aReader.Read( *ReadXML );
        ReadXML->SetBlockMode( sal_False );
        xRoot.Clear();
    }
    else
    {
#if defined(_MSC_VER) && (_MSC_VER >= 1310 )
        String aStreamName = aFolderName + (OUString) String::CreateFromAscii(".xml");
#else
        String aStreamName = aFolderName + String::CreateFromAscii(".xml");
#endif
        xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGREAD );
        SvStorageStreamRef xContents = xRoot->OpenStream( aStreamName, STREAM_STGREAD );

        Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aNames [ nIdx ] ->aPackageName;

        xContents->Seek( 0L );
        xContents->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xContents );

        // get parser
        Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
                "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this, aCur, sal_True );

        // connect parser and filter
        Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
        xParser->setDocumentHandler( xFilter );

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
        bInfoChanged = FALSE;
        MakeBlockText(aCur);
        xRoot.Clear();
        xContents.Clear();
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

ULONG SwXMLTextBlocks::GetMacroTable( USHORT nIdx,
                                      SvxMacroTableDtor& rMacroTbl,
                                      sal_Bool bFileAlreadyOpen )
{
    // set current auto text

    aShort = aNames[ nIdx ]->aShort;
    aLong = aNames[ nIdx ]->aLong;
    aPackageName = aNames[ nIdx ]->aPackageName;

    ULONG nRet = 0;

    // open stream in proper sub-storage
    if( !bFileAlreadyOpen )
    {
        CloseFile();
        nRet = OpenFile ( TRUE );
    }
    if ( 0 == nRet )
    {
        xRoot = xBlkRoot->OpenUCBStorage( aPackageName, STREAM_STGREAD );

        OUString sStreamName = OUString::createFromAscii("atevent.xml");
        SvStorageStreamRef xDocStream = xRoot->OpenStream(
            sStreamName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );
        DBG_ASSERT(xDocStream.Is(), "Can't create output stream");
        if ( xDocStream.Is() )
        {
            xDocStream->SetBufferSize( 16*1024 );
            Reference<io::XInputStream> xInputStream =
                new utl::OInputStreamWrapper( *xDocStream );

            // prepare ParserInputSrouce
            xml::sax::InputSource aParserInput;
            aParserInput.sSystemId = aName;
            aParserInput.aInputStream = xInputStream;

            // get service factory
            Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
            if ( xServiceFactory.is() )
            {

                // get parser
                OUString sParserService( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.xml.sax.Parser" ) );
                Reference< xml::sax::XParser > xParser(
                    xServiceFactory->createInstance(sParserService),
                    UNO_QUERY );
                DBG_ASSERT( xParser.is(), "Can't create parser" );
                if( xParser.is() )
                {
                    // create descriptor and reference to it. Either
                    // both or neither must be kept because of the
                    // reference counting!
                    SvMacroTableEventDescriptor* pDescriptor =
                        new SvMacroTableEventDescriptor(aAutotextEvents);
                    Reference<XNameReplace> xReplace = pDescriptor;
                    Sequence<Any> aFilterArguments( 1 );
                    aFilterArguments[0] <<= xReplace;

                    // get filter
                    OUString sFilterComponent( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.comp.Writer.XMLAutotextEventsImporter"));
                    Reference< xml::sax::XDocumentHandler > xFilter(
                        xServiceFactory->createInstanceWithArguments(
                            sFilterComponent, aFilterArguments),
                        UNO_QUERY );
                    DBG_ASSERT( xFilter.is(),
                                "can't instantiate atevents filter");
                    if ( xFilter.is() )
                    {
                        // connect parser and filter
                        xParser->setDocumentHandler( xFilter );

                        // connect model and filter
                        Reference<document::XImporter> xImporter( xFilter,
                                                                  UNO_QUERY );

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
            else
                nRet = ERR_SWG_READ_ERROR;
        }
        else
            nRet = ERR_SWG_READ_ERROR;
    }
    else
        nRet = ERR_SWG_READ_ERROR;

    // success!
    return nRet;
}


ULONG SwXMLTextBlocks::GetBlockText( const String& rShort, String& rText )
{
    ULONG n = 0;
    sal_Bool bTextOnly = sal_True;
    String aFolderName;
    GeneratePackageName ( rShort, aFolderName );
#if defined(_MSC_VER) && (_MSC_VER >= 1310 )
    String aStreamName = aFolderName + (OUString) String::CreateFromAscii(".xml");
#else
    String aStreamName = aFolderName + String::CreateFromAscii(".xml");
#endif
    rText.Erase();

    xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGREAD );
    if (xRoot.Is())
    {
        if ( !xRoot->IsContained ( aStreamName ) )
        {
            bTextOnly = sal_False;
            aStreamName = String::CreateFromAscii("content.xml");
        }

        SvStorageStreamRef xContents = xRoot->OpenStream( aStreamName, STREAM_STGREAD );
        xContents->Seek( 0L );
        xContents->SetBufferSize( 1024 * 2 );
        Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aName;
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xContents );

        // get parser
        Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
                "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this, rText, bTextOnly );

        // connect parser and filter
        Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
        xParser->setDocumentHandler( xFilter );

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
        xRoot.Clear();
        xContents.Clear();
    }
    else
        ASSERT( sal_False, "Tried to open non-existent folder!");
    return n;
}

ULONG SwXMLTextBlocks::PutBlockText( const String& rShort, const String& rName,
                                     const String& rText,  const String& rPackageName )
{
    USHORT nIndex = GetIndex ( rShort );
    /*
    if (xBlkRoot->IsContained ( rPackageName ) )
    {
        xBlkRoot->Remove ( rPackageName );
        xBlkRoot->Commit ( );
    }
    */
    String aFolderName( rPackageName );
#if defined(_MSC_VER) && (_MSC_VER >= 1310 )
    String aStreamName = aFolderName + (OUString) String::CreateFromAscii(".xml");
#else
    String aStreamName = aFolderName + String::CreateFromAscii(".xml");
#endif

    Reference< lang::XMultiServiceFactory > xServiceFactory =
        comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
    {
        // Throw an exception ?
    }

       Reference < XInterface > xWriter (xServiceFactory->createInstance(
           OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
       DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");

    xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGWRITE );
    SvStorageStreamRef xDocStream = xRoot->OpenStream( aStreamName, STREAM_WRITE | STREAM_TRUNC );

    xDocStream->SetSize ( 0L );
    xDocStream->SetBufferSize( 2*1024 );
    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
    OUString aMime ( RTL_CONSTASCII_USTRINGPARAM ( "text/xml") );
    Any aAny;
    aAny <<= aMime;
    xDocStream->SetProperty( aPropName, aAny );
    Reference < io::XOutputStream > xOut = new  utl::OOutputStreamWrapper(*xDocStream);
       uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
       xSrc->setOutputStream(xOut);

       uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter,
        uno::UNO_QUERY);

       SwXMLTextBlockExport aExp(*this, GetXMLToken ( XML_UNFORMATTED_TEXT ), xHandler);
    aExp.exportDoc( rText );

    xDocStream->Commit();
    xDocStream.Clear();
    xRoot->Commit();
    xRoot.Clear();

    if (! (nFlags & SWXML_NOROOTCOMMIT) )
        xBlkRoot->Commit();


    ULONG nErr = xBlkRoot->GetError();
    ULONG nRes = 0;
    if( nErr == SVSTREAM_DISK_FULL )
        nRes = ERR_W4W_WRITE_FULL;
    else if( nErr != SVSTREAM_OK )
        nRes = ERR_SWG_WRITE_ERROR;

    if( !nRes )         // damit ueber GetText & nCur aufs Doc zugegriffen
        MakeBlockText( rText );

    return nRes;
}

void SwXMLTextBlocks::ReadInfo( void )
{
    const OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( XMLN_BLOCKLIST ) );
    if ( xBlkRoot->IsContained ( sDocName ) )
    {
        Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = sDocName;

        SvStorageStreamRef xDocStream = xBlkRoot->OpenStream( sDocName, STREAM_STGREAD );
        xDocStream->Seek( 0L );
        xDocStream->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xDocStream );

        // get parser
        Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
            "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLBlockListImport( *this );

        // connect parser and filter
        Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
        xParser->setDocumentHandler( xFilter );

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
void SwXMLTextBlocks::WriteInfo( void )
{
    if ( xBlkRoot.Is() || 0 == OpenFile ( FALSE ) )
    {
        Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
        DBG_ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        Reference < XInterface > xWriter (xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
        DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");
        OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( XMLN_BLOCKLIST ) );

        /*
        if ( xBlkRoot->IsContained( sDocName) )
        {
            xBlkRoot->Remove ( sDocName );
            xBlkRoot->Commit();
        }
        */

        SvStorageStreamRef xDocStream = xBlkRoot->OpenStream( sDocName, STREAM_WRITE | STREAM_TRUNC );
        xDocStream->SetSize ( 0L );
        xDocStream->SetBufferSize( 16*1024 );
        String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
        OUString aMime ( RTL_CONSTASCII_USTRINGPARAM ( "text/xml") );
        Any aAny;
        aAny <<= aMime;
        xDocStream->SetProperty( aPropName, aAny );

        Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper(*xDocStream);
        uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
        xSrc->setOutputStream(xOut);

        uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

        SwXMLBlockListExport aExp(*this, OUString::createFromAscii(XMLN_BLOCKLIST), xHandler);
        aExp.exportDoc( XML_BLOCK_LIST );
        xDocStream->Commit();
        xDocStream.Clear();
        xBlkRoot->Commit();
        bInfoChanged = FALSE;
        return;
    }
}

ULONG SwXMLTextBlocks::SetMacroTable(
    USHORT nIdx,
    const SvxMacroTableDtor& rMacroTbl,
    sal_Bool bFileAlreadyOpen )
{
    // set current autotext
    aShort = aNames[ nIdx ]->aShort;
    aLong = aNames[ nIdx ]->aLong;
    aPackageName = aNames[ nIdx ]->aPackageName;

    // start XML autotext event export
    ULONG nRes = 0;

    Reference< lang::XMultiServiceFactory > xServiceFactory =
        comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XML autotext event write:: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_WRITE_ERROR;

    // Get model
    Reference< lang::XComponent > xModelComp(
        pDoc->GetDocShell()->GetModel(), UNO_QUERY );
    ASSERT( xModelComp.is(), "XMLWriter::Write: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_WRITE_ERROR;

    // open stream in proper sub-storage
    if( !bFileAlreadyOpen )
    {
        CloseFile();    // close (it may be open in read-only-mode)
        nRes = OpenFile ( FALSE );
    }

    if ( 0 == nRes )
    {
        xRoot = xBlkRoot->OpenUCBStorage( aPackageName, STREAM_STGWRITE );
        OUString sStreamName( RTL_CONSTASCII_USTRINGPARAM("atevent.xml") );

        // workaround for bug: storages do not get overwritten
        // (This workaround is SLOOOOWWW! Remove this ASAP.)
        /*
        if( xRoot->IsContained( sStreamName ) )
        {
            xRoot->Remove( sStreamName );
            xRoot->Commit();
            xBlkRoot->Commit();
        }
        */


        SvStorageStreamRef xDocStream = xRoot->OpenStream(
            sStreamName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );
        DBG_ASSERT(xDocStream.Is(), "Can't create output stream");
        if ( xDocStream.Is() )
        {
            String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
            OUString aMime ( RTL_CONSTASCII_USTRINGPARAM ( "text/xml") );
            Any aAny;
            aAny <<= aMime;
            xDocStream->SetProperty( aPropName, aAny );
            xDocStream->SetSize( 0 );
            xDocStream->SetBufferSize( 16*1024 );
            Reference<io::XOutputStream> xOutputStream =
                new utl::OOutputStreamWrapper( *xDocStream );

            // get XML writer
            Reference< io::XActiveDataSource > xSaxWriter(
                xServiceFactory->createInstance(
                    OUString::createFromAscii("com.sun.star.xml.sax.Writer") ),
                UNO_QUERY );
            ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );
            if( xSaxWriter.is() )
            {

                // connect XML writer to output stream
                xSaxWriter->setOutputStream( xOutputStream );
                Reference<xml::sax::XDocumentHandler> xDocHandler(
                    xSaxWriter, UNO_QUERY);

                // construct events object
                Reference<XNameAccess> xEvents =
                    new SvMacroTableEventDescriptor(rMacroTbl,aAutotextEvents);

                // prepare arguments (prepend doc handler to given arguments)
                Sequence<Any> aParams(2);
                aParams[0] <<= xDocHandler;
                aParams[1] <<= xEvents;

                // get filter component
                Reference< document::XExporter > xExporter(
                    xServiceFactory->createInstanceWithArguments(
                        OUString::createFromAscii(
                         "com.sun.star.comp.Writer.XMLAutotextEventsExporter"),
                        aParams), UNO_QUERY);
                ASSERT( xExporter.is(),
                        "can't instantiate export filter component" );
                if( xExporter.is() )
                {
                    // connect model and filter
                    xExporter->setSourceDocument( xModelComp );

                    // filter!
                    Sequence<beans::PropertyValue> aFilterProps( 0 );
                    Reference < document::XFilter > xFilter( xExporter,
                                                             UNO_QUERY );
                    xFilter->filter( aFilterProps );
                }
                else
                    nRes = ERR_SWG_WRITE_ERROR;
            }
            else
                nRes = ERR_SWG_WRITE_ERROR;

            // finally, commit stream, sub-storage and storage
            xDocStream->Commit();
            xDocStream.Clear();
            xRoot->Commit();
            xRoot.Clear();
            if ( !bFileAlreadyOpen )
                xBlkRoot->Commit();
        }
        else
            nRes = ERR_SWG_WRITE_ERROR;

        // close file (because it's in write-mode)
        xRoot.Clear();

        if( !bFileAlreadyOpen )
            CloseFile();
    }
    else
        nRes = ERR_SWG_WRITE_ERROR;

    return nRes;
}

