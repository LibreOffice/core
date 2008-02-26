/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MetaImportComponent.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:37:44 $
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

#include "MetaImportComponent.hxx"
#include "xmlnmspe.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/nmspmap.hxx>
#include <tools/string.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;


//===========================================================================

// #110680#
XMLMetaImportComponent::XMLMetaImportComponent(
    const uno::Reference< lang::XMultiServiceFactory >& xServiceFactory) throw()
    :   SvXMLImport(xServiceFactory), mxDocProps()
{
}

XMLMetaImportComponent::~XMLMetaImportComponent() throw()
{
}


SvXMLImportContext* XMLMetaImportComponent::CreateContext(
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList > & xAttrList )
{
    if (  (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_DOCUMENT_META) )
    {
        if (!mxDocProps.is()) {
            throw uno::RuntimeException(::rtl::OUString::createFromAscii(
                "XMLMetaImportComponent::CreateContext: setTargetDocument "
                "has not been called"), *this);
        }
        uno::Reference<xml::sax::XDocumentHandler> xDocBuilder(
            mxServiceFactory->createInstance(::rtl::OUString::createFromAscii(
                    "com.sun.star.xml.dom.SAXDocumentBuilder")),
                 uno::UNO_QUERY_THROW);
        return new SvXMLMetaDocumentContext(
                        *this, nPrefix, rLocalName, mxDocProps, xDocBuilder);
    }
    else
    {
        return SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
}

void SAL_CALL XMLMetaImportComponent::setTargetDocument(
    const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    mxDocProps = uno::Reference< document::XDocumentProperties >::query( xDoc );
    if( !mxDocProps.is() )
        throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii(
            "XMLMetaImportComponent::setTargetDocument: argument is no "
            "XDocumentProperties"), uno::Reference<uno::XInterface>(*this), 0);
}

uno::Sequence< rtl::OUString > SAL_CALL
    XMLMetaImportComponent_getSupportedServiceNames()
        throw()
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.document.XMLOasisMetaImporter" ) );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

rtl::OUString SAL_CALL XMLMetaImportComponent_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLMetaImportComponent" ) );
}

uno::Reference< uno::XInterface > SAL_CALL XMLMetaImportComponent_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new XMLMetaImportComponent;
    return (cppu::OWeakObject*)new XMLMetaImportComponent(rSMgr);
}

