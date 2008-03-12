/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlictxt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:29:49 $
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
#include "precompiled_xmloff.hxx"

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XLOCATOR_HPP_
#include <com/sun/star/xml/sax/XLocator.hpp>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

TYPEINIT0( SvXMLImportContext );

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp, USHORT nPrfx,
                              const OUString& rLName ) :
    mrImport( rImp ),
    mnPrefix( nPrfx ),
    maLocalName( rLName ),
    mpRewindMap( 0 )
{
}

SvXMLImportContext::~SvXMLImportContext()
{
}

SvXMLImportContext *SvXMLImportContext::CreateChildContext( USHORT nPrefix,
                                            const OUString& rLocalName,
                                            const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    return mrImport.CreateContext( nPrefix, rLocalName, xAttrList );
}

void SvXMLImportContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& )
{
}

void SvXMLImportContext::EndElement()
{
}

void SvXMLImportContext::Characters( const OUString& )
{
}


