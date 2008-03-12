/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXMLTextBlocks1.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:23:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

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
using ::rtl::OUString;

using ::xmloff::token::XML_BLOCK_LIST;
using ::xmloff::token::XML_UNFORMATTED_TEXT;
using ::xmloff::token::GetXMLToken;

ULONG SwXMLTextBlocks::GetDoc( USHORT nIdx )
{
    String aFolderName ( GetPackageName ( nIdx ) );

    if (!IsOnlyTextBlock ( nIdx ) )
    {
        try
        {
            xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::READ );
            xMedium = new SfxMedium(xRoot, GetBaseURL());
            SwReader aReader(*xMedium,aFolderName, pDoc );
            ReadXML->SetBlockMode( sal_True );
            aReader.Read( *ReadXML );
            ReadXML->SetBlockMode( sal_False );
        }
        catch( uno::Exception& )
        {
        }

        xRoot = 0;
    }
    else
    {
        String aStreamName = aFolderName + (OUString) String::CreateFromAscii(".xml");
        try
        {
            xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::READ );
            uno::Reference < io::XStream > xStream = xRoot->openStreamElement( aStreamName, embed::ElementModes::READ );

            uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
            ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
            if( !xServiceFactory.is() )
            {
                // Throw an exception ?
            }

            xml::sax::InputSource aParserInput;
            aParserInput.sSystemId = aNames [ nIdx ] ->aPackageName;

            aParserInput.aInputStream = xStream->getInputStream();

            // get parser
            uno::Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
                    OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
            ASSERT( xXMLParser.is(),
                    "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
            if( !xXMLParser.is() )
            {
                // Maybe throw an exception?
            }

            // get filter
            // #110680#
            // uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this, aCur, sal_True );
            uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( xServiceFactory, *this, aCur, sal_True );

            // connect parser and filter
            uno::Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
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
        try
        {
            xRoot = xBlkRoot->openStorageElement( aPackageName, embed::ElementModes::READ );
            long nTmp = SOT_FORMATSTR_ID_STARWRITER_60;
            sal_Bool bOasis = ( SotStorage::GetVersion( xRoot ) > nTmp );

            OUString sStreamName = OUString::createFromAscii("atevent.xml");
            uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement(
                sStreamName, embed::ElementModes::READ );
            DBG_ASSERT(xDocStream.is(), "Can't create stream");
            if ( xDocStream.is() )
            {
                uno::Reference<io::XInputStream> xInputStream = xDocStream->getInputStream();

                // prepare ParserInputSrouce
                xml::sax::InputSource aParserInput;
                aParserInput.sSystemId = aName;
                aParserInput.aInputStream = xInputStream;

                // get service factory
                uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
                    comphelper::getProcessServiceFactory();
                if ( xServiceFactory.is() )
                {

                    // get parser
                    OUString sParserService( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.xml.sax.Parser" ) );
                    uno::Reference< xml::sax::XParser > xParser(
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
                        uno::Reference<XNameReplace> xReplace = pDescriptor;
                        Sequence<Any> aFilterArguments( 1 );
                        aFilterArguments[0] <<= xReplace;

                        // get filter
                        OUString sFilterComponent( OUString::createFromAscii(
                            bOasis
                            ? "com.sun.star.comp.Writer.XMLOasisAutotextEventsImporter"
                            : "com.sun.star.comp.Writer.XMLAutotextEventsImporter"));
                        uno::Reference< xml::sax::XDocumentHandler > xFilter(
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
                            uno::Reference<document::XImporter> xImporter( xFilter,
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


ULONG SwXMLTextBlocks::GetBlockText( const String& rShort, String& rText )
{
    ULONG n = 0;
    sal_Bool bTextOnly = sal_True;
    String aFolderName;
    GeneratePackageName ( rShort, aFolderName );
    String aStreamName = aFolderName + (OUString) String::CreateFromAscii(".xml");
    rText.Erase();

    try
    {
        xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::READ );
        uno::Reference < container::XNameAccess > xAccess( xRoot, uno::UNO_QUERY );
        if ( !xAccess->hasByName( aStreamName ) || !xRoot->isStreamElement( aStreamName ) )
        {
            bTextOnly = sal_False;
            aStreamName = String::CreateFromAscii("content.xml");
        }

        uno::Reference < io::XStream > xContents = xRoot->openStreamElement( aStreamName, embed::ElementModes::READ );
        uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aName;
        aParserInput.aInputStream = xContents->getInputStream();

        // get parser
        uno::Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
                "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        // #110680#
        // uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this, rText, bTextOnly );
        uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( xServiceFactory, *this, rText, bTextOnly );

        // connect parser and filter
        uno::Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
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

        xRoot = 0;
    }
    catch ( uno::Exception& )
    {
        ASSERT( sal_False, "Tried to open non-existent folder or stream!");
    }

    return n;
}

ULONG SwXMLTextBlocks::PutBlockText( const String& rShort, const String& ,
                                     const String& rText,  const String& rPackageName )
{
    GetIndex ( rShort );
    /*
    if (xBlkRoot->IsContained ( rPackageName ) )
    {
        xBlkRoot->Remove ( rPackageName );
        xBlkRoot->Commit ( );
    }
    */
    String aFolderName( rPackageName );
    String aStreamName = aFolderName + (OUString) String::CreateFromAscii(".xml");

    uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
        comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
    {
        // Throw an exception ?
    }

       uno::Reference < XInterface > xWriter (xServiceFactory->createInstance(
           OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
       DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");
    ULONG nRes = 0;

    try
    {
    xRoot = xBlkRoot->openStorageElement( aFolderName, embed::ElementModes::WRITE );
    uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement( aStreamName,
                embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

    uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
    OUString aMime ( RTL_CONSTASCII_USTRINGPARAM ( "text/xml") );
    Any aAny;
    aAny <<= aMime;
    xSet->setPropertyValue( aPropName, aAny );
    uno::Reference < io::XOutputStream > xOut = xDocStream->getOutputStream();
       uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
       xSrc->setOutputStream(xOut);

       uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter,
        uno::UNO_QUERY);

    // #110680#
       // SwXMLTextBlockExport aExp(*this, GetXMLToken ( XML_UNFORMATTED_TEXT ), xHandler);
       SwXMLTextBlockExport aExp( xServiceFactory, *this, GetXMLToken ( XML_UNFORMATTED_TEXT ), xHandler);

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
    ULONG nErr = xBlkRoot->GetError();
    ULONG nRes = 0;
    if( nErr == SVSTREAM_DISK_FULL )
        nRes = ERR_W4W_WRITE_FULL;
    else if( nErr != SVSTREAM_OK )
        nRes = ERR_SWG_WRITE_ERROR;
    */
    if( !nRes )         // damit ueber GetText & nCur aufs Doc zugegriffen
        MakeBlockText( rText );

    return nRes;
}

void SwXMLTextBlocks::ReadInfo( void )
{
    try
    {
    const OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( XMLN_BLOCKLIST ) );
    uno::Reference < container::XNameAccess > xAccess( xBlkRoot, uno::UNO_QUERY );
    if ( xAccess.is() && xAccess->hasByName( sDocName ) && xBlkRoot->isStreamElement( sDocName ) )
    {
        uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = sDocName;

        uno::Reference < io::XStream > xDocStream = xBlkRoot->openStreamElement( sDocName, embed::ElementModes::READ );
        aParserInput.aInputStream = xDocStream->getInputStream();

        // get parser
        uno::Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
            "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        // #110680#
        // uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLBlockListImport( *this );
        uno::Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLBlockListImport( xServiceFactory, *this );

        // connect parser and filter
        uno::Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
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
    catch ( uno::Exception& )
    {
    }
}
void SwXMLTextBlocks::WriteInfo( void )
{
    if ( xBlkRoot.is() || 0 == OpenFile ( FALSE ) )
    {
        uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
        DBG_ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        uno::Reference < XInterface > xWriter (xServiceFactory->createInstance(
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

        try
        {
        uno::Reference < io::XStream > xDocStream = xBlkRoot->openStreamElement( sDocName,
                    embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

        uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
        String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
        OUString aMime ( RTL_CONSTASCII_USTRINGPARAM ( "text/xml") );
        Any aAny;
        aAny <<= aMime;
        xSet->setPropertyValue( aPropName, aAny );
        uno::Reference < io::XOutputStream > xOut = xDocStream->getOutputStream();
        uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
        xSrc->setOutputStream(xOut);

        uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

        // #110680#
        // SwXMLBlockListExport aExp(*this, OUString::createFromAscii(XMLN_BLOCKLIST), xHandler);
        SwXMLBlockListExport aExp( xServiceFactory, *this, OUString::createFromAscii(XMLN_BLOCKLIST), xHandler);

        aExp.exportDoc( XML_BLOCK_LIST );

        uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
        if ( xTrans.is() )
            xTrans->commit();
        }
        catch ( uno::Exception& )
        {
        }

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

    uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
        comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XML autotext event write:: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_WRITE_ERROR;

    // Get model
    uno::Reference< lang::XComponent > xModelComp(
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
        try
        {
            xRoot = xBlkRoot->openStorageElement( aPackageName, embed::ElementModes::WRITE );
            OUString sStreamName( RTL_CONSTASCII_USTRINGPARAM("atevent.xml") );
            long nTmp = SOT_FORMATSTR_ID_STARWRITER_60;
            sal_Bool bOasis = ( SotStorage::GetVersion( xRoot ) > nTmp );

            uno::Reference < io::XStream > xDocStream = xRoot->openStreamElement( sStreamName,
                        embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

            uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );
            String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
            OUString aMime ( RTL_CONSTASCII_USTRINGPARAM ( "text/xml") );
            Any aAny;
            aAny <<= aMime;
            xSet->setPropertyValue( aPropName, aAny );
            uno::Reference < io::XOutputStream > xOutputStream = xDocStream->getOutputStream();

            // get XML writer
            uno::Reference< io::XActiveDataSource > xSaxWriter(
                xServiceFactory->createInstance(
                    OUString::createFromAscii("com.sun.star.xml.sax.Writer") ),
                UNO_QUERY );
            ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );
            if( xSaxWriter.is() )
            {

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
                uno::Reference< document::XExporter > xExporter(
                    xServiceFactory->createInstanceWithArguments(
                        OUString::createFromAscii(
                         bOasis
                             ? "com.sun.star.comp.Writer.XMLOasisAutotextEventsExporter"
                            : "com.sun.star.comp.Writer.XMLAutotextEventsExporter"),
                        aParams), UNO_QUERY);
                ASSERT( xExporter.is(),
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

