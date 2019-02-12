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


#include <saxemitter.hxx>
#include "emitcontext.hxx"
#include "saxattrlist.hxx"

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlimp.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <osl/file.hxx>
static osl::File* pStream = nullptr;
static int nIndent = 0;
#endif

using namespace com::sun::star;

namespace pdfi
{

SaxEmitter::SaxEmitter( const uno::Reference< xml::sax::XDocumentHandler >& xDocHdl ) :
    m_xDocHdl( xDocHdl )
{
    OSL_PRECOND(m_xDocHdl.is(), "SaxEmitter(): invalid doc handler");
    if (SvXMLImport *pFastHandler = dynamic_cast<SvXMLImport*>(m_xDocHdl.get()))
        m_xDocHdl.set( new SvXMLLegacyToFastDocHandler( pFastHandler ) );
    try
    {
        m_xDocHdl->startDocument();
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 0
    static const char* pDir = getenv( "DBG_PDFIMPORT_DIR" );
    if( pDir )
    {
        OUString aStr( OStringToOUString( pDir, RTL_TEXTENCODING_UTF8 ) );
        OUString aFileURL;
        osl_getFileURLFromSystemPath( aStr.pData, &aFileURL.pData );
        OUStringBuffer aBuf( 256 );
        aBuf.append( aFileURL );
        aBuf.append( "/pdfimport.xml" );
        pStream = new osl::File( aBuf.makeStringAndClear() );
        if( pStream->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create ) )
        {
            pStream->open( osl_File_OpenFlag_Write );
            pStream->setSize( 0 );
        }
    }
    else
        pStream = nullptr;
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
#if OSL_DEBUG_LEVEL > 0
    if( pStream )
    {
        pStream->close();
        delete pStream;
        pStream = nullptr;
    }
#endif
}

void SaxEmitter::beginTag( const char* pTag, const PropertyMap& rProperties )
{
    OUString aTag = OUString::createFromAscii( pTag );
    uno::Reference< xml::sax::XAttributeList > xAttr(
        new SaxAttrList( rProperties ) );
    try
    {
        m_xDocHdl->startElement( aTag, xAttr );
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 0
    if( !pStream )
        return;

    sal_uInt64 nWritten = 0;
    for( int i = 0; i < nIndent; i++ )
        pStream->write( "    ", 4, nWritten );

    OStringBuffer aBuf( 1024 );
    aBuf.append( '<' );
    aBuf.append( pTag );
    for( const auto& rProperty : rProperties )
    {
        aBuf.append( ' ' );
        aBuf.append( OUStringToOString( rProperty.first, RTL_TEXTENCODING_UTF8 ) );
        aBuf.append( "=\"" );
        aBuf.append( OUStringToOString( rProperty.second, RTL_TEXTENCODING_UTF8 ) );
        aBuf.append( "\"" );
    }
    aBuf.append( ">\n" );
    pStream->write( aBuf.getStr(), aBuf.getLength(), nWritten );
    nIndent++;
#endif
}

void SaxEmitter::write( const OUString& rText )
{
    try
    {
        m_xDocHdl->characters( rText );
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 0
    if( pStream )
    {
        OString aStr( OUStringToOString( rText, RTL_TEXTENCODING_UTF8 ) );
        sal_uInt64 nWritten = 0;
        pStream->write( aStr.getStr(), aStr.getLength(), nWritten );
    }
#endif
}

void SaxEmitter::endTag( const char* pTag )
{
    OUString aTag = OUString::createFromAscii( pTag );
    try
    {
        m_xDocHdl->endElement( aTag );
    }
    catch( xml::sax::SAXException& )
    {
    }
#if OSL_DEBUG_LEVEL > 0
    if( !pStream )
        return;

    sal_uInt64 nWritten = 0;
    for( int i = 0; i < nIndent; i++ )
        pStream->write( "    ", 4, nWritten );

    OStringBuffer aBuf( 1024 );
    aBuf.append( "</" );
    aBuf.append( pTag );
    aBuf.append( ">\n" );
    pStream->write( aBuf.getStr(), aBuf.getLength(), nWritten );
    nIndent--;
#endif
}

XmlEmitterSharedPtr createSaxEmitter( const uno::Reference< xml::sax::XDocumentHandler >& xDocHdl )
{
    return XmlEmitterSharedPtr(new SaxEmitter(xDocHdl));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
