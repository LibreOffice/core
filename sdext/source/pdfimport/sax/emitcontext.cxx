/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "saxemitter.hxx"
#include "emitcontext.hxx"
#include "saxattrlist.hxx"

#include <rtl/strbuf.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#if OSL_DEBUG_LEVEL > 1
#include <osl/file.hxx>
static osl::File* pStream = NULL;
static int nIndent = 0;
#endif

using namespace com::sun::star;

namespace pdfi
{

SaxEmitter::SaxEmitter( const uno::Reference< xml::sax::XDocumentHandler >& xDocHdl ) :
    m_xDocHdl( xDocHdl )
{
    OSL_PRECOND(m_xDocHdl.is(), "SaxEmitter(): invalid doc handler");
    try
    {
        m_xDocHdl->startDocument();
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 1
    static const char* pDir = getenv( "DBG_PDFIMPORT_DIR" );
    if( pDir )
    {
        rtl::OUString aStr( rtl::OStringToOUString( pDir, RTL_TEXTENCODING_UTF8 ) );
        rtl::OUString aFileURL;
        osl_getFileURLFromSystemPath( aStr.pData, &aFileURL.pData );
        rtl::OUStringBuffer aBuf( 256 );
        aBuf.append( aFileURL );
        aBuf.appendAscii( "/pdfimport.xml" );
        pStream = new osl::File( aBuf.makeStringAndClear() );
        if( pStream->open( OpenFlag_Write | OpenFlag_Create ) )
        {
            pStream->open( OpenFlag_Write );
            pStream->setSize( 0 );
        }
    }
    else
        pStream = 0;
#endif
}

SaxEmitter::~SaxEmitter()
{
    try
    {
        m_xDocHdl->endDocument();
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 1
    if( pStream )
    {
        pStream->close();
        delete pStream;
        pStream = 0;
    }
#endif
}

void SaxEmitter::beginTag( const char* pTag, const PropertyMap& rProperties )
{
    rtl::OUString aTag = rtl::OUString::createFromAscii( pTag );
    uno::Reference< xml::sax::XAttributeList > xAttr(
        new SaxAttrList( rProperties ) );
    try
    {
        m_xDocHdl->startElement( aTag, xAttr );
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 1
    if( pStream )
    {
        sal_uInt64 nWritten = 0;
        for( int i = 0; i < nIndent; i++ )
            pStream->write( "    ", 4, nWritten );

        rtl::OStringBuffer aBuf( 1024 );
        aBuf.append( '<' );
        aBuf.append( pTag );
        for( PropertyMap::const_iterator it = rProperties.begin(); it != rProperties.end(); ++it )
        {
            aBuf.append( ' ' );
            aBuf.append( rtl::OUStringToOString( it->first, RTL_TEXTENCODING_UTF8 ) );
            aBuf.append( "=\"" );
            aBuf.append( rtl::OUStringToOString( it->second, RTL_TEXTENCODING_UTF8 ) );
            aBuf.append( "\"" );
        }
        aBuf.append( ">\n" );
        pStream->write( aBuf.getStr(), aBuf.getLength(), nWritten );
        nIndent++;
    }
#endif
}

void SaxEmitter::write( const rtl::OUString& rText )
{
    try
    {
        m_xDocHdl->characters( rText );
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 1
    if( pStream )
    {
        rtl::OString aStr( rtl::OUStringToOString( rText, RTL_TEXTENCODING_UTF8 ) );
        sal_uInt64 nWritten = 0;
        pStream->write( aStr.getStr(), aStr.getLength(), nWritten );
    }
#endif
}

void SaxEmitter::endTag( const char* pTag )
{
    rtl::OUString aTag = rtl::OUString::createFromAscii( pTag );
    try
    {
        m_xDocHdl->endElement( aTag );
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 1
    if( pStream )
    {
        sal_uInt64 nWritten = 0;
        for( int i = 0; i < nIndent; i++ )
            pStream->write( "    ", 4, nWritten );

        rtl::OStringBuffer aBuf( 1024 );
        aBuf.append( "</" );
        aBuf.append( pTag );
        aBuf.append( ">\n" );
        pStream->write( aBuf.getStr(), aBuf.getLength(), nWritten );
        nIndent--;
    }
#endif
}

XmlEmitterSharedPtr createSaxEmitter( const uno::Reference< xml::sax::XDocumentHandler >& xDocHdl )
{
    return XmlEmitterSharedPtr(new SaxEmitter(xDocHdl));
}

}
