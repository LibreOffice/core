/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: emitcontext.cxx,v $
 *
 * $Revision: 1.2 $
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
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

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
#if 0
        OSL_ENSURE( false,
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
    }
}

SaxEmitter::~SaxEmitter()
{
    try
    {
        m_xDocHdl->endDocument();
    }
    catch( xml::sax::SAXException& )
    {
#if 0
        OSL_ENSURE( false,
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
    }
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
#if 0
        OSL_ENSURE( false,
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
    }
}

void SaxEmitter::write( const rtl::OUString& rText )
{
    try
    {
        m_xDocHdl->characters( rText );
    }
    catch( xml::sax::SAXException& )
    {
#if 0
        OSL_ENSURE( false,
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
    }
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
#if 0
        OSL_ENSURE( false,
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
    }
}

XmlEmitterSharedPtr createSaxEmitter( const uno::Reference< xml::sax::XDocumentHandler >& xDocHdl )
{
    return XmlEmitterSharedPtr(new SaxEmitter(xDocHdl));
}

}
