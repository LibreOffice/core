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

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include <sal/macros.h>
#include <tools/stream.hxx>
#include <vector>
#include <ucbhelper/content.hxx>
#include <xmloff/attrlist.hxx>

#include "LotusWordProImportFilter.hxx"
#include "lwpfilter.hxx"

using namespace com::sun::star;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::XComponent;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::io::XInputStream;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;
using com::sun::star::ucb::XCommandEnvironment;

using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

//                                 W     o     r     d     P     r     o
static const sal_Int8 header[] = { 0x57, 0x6f, 0x72, 0x64, 0x50, 0x72, 0x6f };

const sal_Int32 MAXCHARS = 65534;

 // Simple xml importer, currently the importer is very very simple
 // it only extracts pure text from the wordpro file. Absolutely no formatting
 // information is currently imported.
 // To reflect the current state of this importer the sax events sent
 // to the document handler are also the simplest possible. In addition to
 // the basic attributes set up for the 'office:document' element
 // all the imported text is inserted into 'text:p' elements.
 // The parser extracts the pure text and creates simple a simple 'text:p'
 // element to contain that text. In the event of the text exceeding
 // MAXCHARS new 'text:p' elements are created as needed
class SimpleXMLImporter
{
private:

    uno::Reference< XDocumentHandler > m_xDocHandler;
    std::vector< OUString > m_vStringChunks;
    SvStream& m_InputStream;

    bool CheckValidData( sal_Int8 nChar ) const
    {
        if( ( nChar >= 0x20 && nChar <= 0x7E ) && ( nChar != 0X40 ) )
            return  true;
        return false;
    }

    void addAttribute( SvXMLAttributeList* pAttrList, const char* key, const char* val )
    {
        pAttrList->AddAttribute( OUString::createFromAscii( key ), OUString::createFromAscii( val ) );
    }

    void writeTextChunk( const OUString& sChunk )
    {
        SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
        uno::Reference < XAttributeList > xAttrList(pAttrList);

        pAttrList->AddAttribute( "text:style-name",  "Standard" );

        m_xDocHandler->startElement( "text:p", xAttrList );
        m_xDocHandler->characters( sChunk );
        m_xDocHandler->endElement( "text:p" );
    }

    void writeDocContentPreamble()
    {
        SvXMLAttributeList *pDocContentPropList = new SvXMLAttributeList();
    uno::Reference < XAttributeList > xDocContentList(pDocContentPropList);
        addAttribute( pDocContentPropList, "xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" );
        addAttribute( pDocContentPropList, "xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
        addAttribute( pDocContentPropList, "xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0" );
        addAttribute( pDocContentPropList, "xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0" );
        addAttribute( pDocContentPropList, "xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" );
        addAttribute( pDocContentPropList, "xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" );
        addAttribute( pDocContentPropList, "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        addAttribute( pDocContentPropList, "xmlns:dc", "http://purl.org/dc/elements/1.1/" );
        addAttribute( pDocContentPropList, "xmlns:meta", "urn:oasis:names:tc:opendocument:xmlns:meta:1.0" );
        addAttribute( pDocContentPropList, "xmlns:number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" );
        addAttribute( pDocContentPropList, "xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" );
        addAttribute( pDocContentPropList, "xmlns:chart", "urn:oasis:names:tc:opendocument:xmlns:chart:1.0" );
        addAttribute( pDocContentPropList, "xmlns:dr3d", "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" );
        addAttribute( pDocContentPropList, "xmlns:math", "http://www.w3.org/1998/Math/MathML" );
        addAttribute( pDocContentPropList, "xmlns:form", "urn:oasis:names:tc:opendocument:xmlns:form:1.0" );
        addAttribute( pDocContentPropList, "xmlns:script", "urn:oasis:names:tc:opendocument:xmlns:script:1.0" );
        addAttribute( pDocContentPropList, "xmlns:ooo", "http://openoffice.org/2004/office" );
        addAttribute( pDocContentPropList, "xmlns:ooow", "http://openoffice.org/2004/writer" );
        addAttribute( pDocContentPropList, "xmlns:oooc", "http://openoffice.org/2004/calc" );
        addAttribute( pDocContentPropList, "xmlns:dom", "http://www.w3.org/2001/xml-events" );
        addAttribute( pDocContentPropList, "xmlns:xforms", "http://www.w3.org/2002/xforms" );
        addAttribute( pDocContentPropList, "xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
        addAttribute( pDocContentPropList, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
        addAttribute( pDocContentPropList, "office:version", "1.0");
        m_xDocHandler->startElement("office:document-content" , xDocContentList );
    }

    void parseDoc()
    {
        sal_uInt8 nDelim, nDummy, nLen, nData;
        sal_uInt16 nOpcode;
        OUStringBuffer sBuf( MAXCHARS );
        sal_Int32 nChars = 0;

        while( !m_InputStream.IsEof())
        {
            m_InputStream.ReadUChar( nDelim );
            if( nDelim == 0x40 )
            {
                m_InputStream.ReadUChar( nDummy ).ReadUInt16( nOpcode );
                switch( nOpcode )
                {
                    case 0xC00B:  // Dictionary Word
                        m_InputStream.ReadUChar( nLen ).ReadUChar( nDummy );
                        while( nLen > 0 && !m_InputStream.IsEof() )
                        {
                            sal_uInt8 nChar;
                            m_InputStream.ReadUChar( nChar );
                            if( CheckValidData( nChar ) )
                            {
                                sBuf.appendAscii( (sal_Char*)(&nChar),1 );
                                if ( ++nChars >=  MAXCHARS )
                                {
                                    m_vStringChunks.push_back( sBuf.makeStringAndClear() );
                                    nChars = 0;
                                }
                            }
                            nLen--;
                        }
                        break;

                    case 0x0242:  // Non Dictionary word
                        m_InputStream.ReadUChar( nData );
                        if( nData == 0x02 )
                        {
                            m_InputStream.ReadUChar( nLen ).ReadUChar( nDummy );
                            while( nLen > 0 && !m_InputStream.IsEof() )
                            {
                                m_InputStream.ReadUChar( nData );
                                if( CheckValidData( nData ) )
                                {
                                    sBuf.appendAscii( (sal_Char*)(&nData),1 );
                                    if ( ++nChars >=  MAXCHARS )
                                    {
                                        m_vStringChunks.push_back( sBuf.makeStringAndClear() );
                                        nChars = 0;
                                    }
                                }
                                nLen--;
                            }
                        }
                        break;
                }
            }
        }
        if ( nChars )
            m_vStringChunks.push_back( sBuf.makeStringAndClear() );
    }

    void writeXML()
    {
        if ( !m_vStringChunks.empty() )
        {
            m_xDocHandler->startDocument();
            SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
            writeDocContentPreamble(); // writes "office:document-content" elem
            uno::Reference < XAttributeList > xAttrList(pAttrList);

            m_xDocHandler->startElement( "office:body", xAttrList  );

            // process strings imported
            std::vector< OUString >::const_iterator it = m_vStringChunks.begin();
            std::vector< OUString >::const_iterator it_end = m_vStringChunks.end();
            for ( ; it!=it_end; ++it )
                writeTextChunk( *it );

            m_xDocHandler->endElement( "office:body" );
            m_xDocHandler->endElement( "office:document-content" );
            m_xDocHandler->endDocument();
        }
    }
public:

    SimpleXMLImporter( const uno::Reference< XDocumentHandler >&  xDocHandler, SvStream& rStream ) : m_xDocHandler( xDocHandler ), m_InputStream( rStream ) {}

    void import()
    {
        parseDoc();
        writeXML();
    }
};

sal_Bool SAL_CALL LotusWordProImportFilter::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{

    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    OUString sURL;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        //Note, we should attempt to use InputStream here first!
        if ( pValue[i].Name == "URL" )
            pValue[i].Value >>= sURL;
    }

    SvFileStream inputStream( sURL, STREAM_READ );
    if ( inputStream.IsEof() || ( inputStream.GetError() != SVSTREAM_OK ) )
         return sal_False;

    // An XML import service: what we push sax messages to..
    OUString sXMLImportService ( "com.sun.star.comp.Writer.XMLImporter" );

    uno::Reference< XDocumentHandler > xInternalHandler( mxContext->getServiceManager()->createInstanceWithContext( sXMLImportService, mxContext ), UNO_QUERY );
    uno::Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    if (xImporter.is())
        xImporter->setTargetDocument(mxDoc);

    return ( ReadWordproFile( inputStream, xInternalHandler) == 0 );

}

sal_Bool SAL_CALL LotusWordProImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException, std::exception)
{
    return importImpl ( aDescriptor );
}
void SAL_CALL LotusWordProImportFilter::cancel(  )
    throw (RuntimeException, std::exception)
{
}

// XImporter
void SAL_CALL LotusWordProImportFilter::setTargetDocument( const uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL LotusWordProImportFilter::detect( com::sun::star::uno::Sequence< PropertyValue >& Descriptor )
    throw( com::sun::star::uno::RuntimeException, std::exception )
{

    OUString sTypeName( "writer_LotusWordPro_Document" );
    sal_Int32 nLength = Descriptor.getLength();
    OUString sURL;
    const PropertyValue * pValue = Descriptor.getConstArray();
    uno::Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "TypeName" )
            pValue[i].Value >>= sTypeName;
        else if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name == "URL" )
            pValue[i].Value >>= sURL;
    }

    uno::Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;
    if (!xInputStream.is())
    {
        try
        {
            ::ucbhelper::Content aContent(sURL, xEnv, mxContext);
            xInputStream = aContent.openStream();
        }
        catch ( Exception& )
        {
            return OUString();
        }

        if (!xInputStream.is())
            return OUString();
    }

    Sequence< ::sal_Int8 > aData;
    sal_Int32 nLen = SAL_N_ELEMENTS( header );
    if ( !( ( nLen == xInputStream->readBytes( aData, nLen ) )
                && ( memcmp( ( void* )header, (void*) aData.getConstArray(), nLen ) == 0 ) ) )
        sTypeName = OUString();

    return sTypeName;
}

// XInitialization
void SAL_CALL LotusWordProImportFilter::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException, std::exception)
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue * pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue[i].Name == "Type" )
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}

OUString LotusWordProImportFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( "com.sun.star.comp.Writer.LotusWordProImportFilter" );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"

Sequence< OUString > SAL_CALL LotusWordProImportFilter_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( SERVICE_NAME1 );
    pArray[1] =  OUString ( SERVICE_NAME2 );
    return aRet;
}

#undef SERVICE_NAME2
#undef SERVICE_NAME1

uno::Reference< XInterface > SAL_CALL LotusWordProImportFilter_createInstance( const uno::Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new LotusWordProImportFilter( comphelper::getComponentContext(rSMgr) );
}

// XServiceInfo
OUString SAL_CALL LotusWordProImportFilter::getImplementationName(  )
    throw (RuntimeException, std::exception)
{
    return LotusWordProImportFilter_getImplementationName();
}
sal_Bool SAL_CALL LotusWordProImportFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL LotusWordProImportFilter::getSupportedServiceNames(  )
    throw (RuntimeException, std::exception)
{
    return LotusWordProImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
