/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
        if( pStream->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create ) )
        {
            pStream->open( osl_File_OpenFlag_Write );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
