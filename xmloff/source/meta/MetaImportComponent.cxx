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
#include "precompiled_xmloff.hxx"

#include "MetaImportComponent.hxx"
#include "xmloff/xmlnmspe.hxx"

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

