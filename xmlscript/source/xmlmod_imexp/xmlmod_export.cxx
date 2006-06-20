/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlmod_export.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:13:55 $
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

#include <xmlscript/xmlmod_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace rtl;

namespace xmlscript
{

//==================================================================================================

void
SAL_CALL exportScriptModule(
    Reference< xml::sax::XExtendedDocumentHandler > const & xOut,
    const ModuleDescriptor& rMod )
        SAL_THROW( (Exception) )
{
    xOut->startDocument();

    OUString aDocTypeStr( RTL_CONSTASCII_USTRINGPARAM(
        "<!DOCTYPE script:module PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"module.dtd\">" ) );
    xOut->unknown( aDocTypeStr );
    xOut->ignorableWhitespace( OUString() );

    OUString aModuleName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":module") );
    XMLElement* pModElement = new XMLElement( aModuleName );
    Reference< xml::sax::XAttributeList > xAttributes( pModElement );

    pModElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("xmlns:" XMLNS_SCRIPT_PREFIX) ),
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_URI) ) );

    pModElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":name") ),
                                rMod.aName );
    pModElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":language") ),
                                rMod.aLanguage );

    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aModuleName, xAttributes );
    xOut->characters( rMod.aCode );
    xOut->endElement( aModuleName );
    xOut->endDocument();
}

}

