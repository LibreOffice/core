/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_parceldesc.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:12:11 $
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
#include "precompiled_desktop.hxx"
#include "dp_parceldesc.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

namespace css = ::com::sun::star;
namespace dp_registry
{
namespace backend
{
namespace sfwk
{


// XDocumentHandler
void SAL_CALL
ParcelDescDocHandler::startDocument()
throw ( xml::sax::SAXException, RuntimeException )
{
    m_bIsParsed = false;
}

void SAL_CALL
ParcelDescDocHandler::endDocument()
throw ( xml::sax::SAXException, RuntimeException )
{
    m_bIsParsed = true;
}

void SAL_CALL
ParcelDescDocHandler::characters( const OUString & )
    throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::ignorableWhitespace( const OUString & )
    throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::processingInstruction(
    const OUString &, const OUString & )
        throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::setDocumentLocator(
    const Reference< xml::sax::XLocator >& )
        throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::startElement( const OUString& aName,
    const Reference< xml::sax::XAttributeList > & xAttribs )
        throw ( xml::sax::SAXException,
            RuntimeException )
{
    OSL_TRACE("ParcelDescDocHandler::startElement() for %s",
        rtl::OUStringToOString( aName, RTL_TEXTENCODING_ASCII_US ).getStr() );
    if ( !skipIndex )
    {
        if ( aName.equals( OUString::createFromAscii( "parcel" ) ) )
        {
            m_sLang = xAttribs->getValueByName( OUString::createFromAscii( "language" ) );
        }
        ++skipIndex;
    }
    else
    {
        OSL_TRACE("ParcelDescDocHandler::startElement() skipping for %s",
            rtl::OUStringToOString( aName, RTL_TEXTENCODING_ASCII_US ).getStr() );
    }

}

void SAL_CALL ParcelDescDocHandler::endElement( const OUString & aName )
   throw ( xml::sax::SAXException, RuntimeException )
{
    if ( skipIndex )
    {
        --skipIndex;
        OSL_TRACE("ParcelDescDocHandler::endElement() skipping for %s",
            rtl::OUStringToOString( aName, RTL_TEXTENCODING_ASCII_US ).getStr() );
    }
}


}
}
}

