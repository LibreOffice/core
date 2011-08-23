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

#include "MetaImportComponent.hxx"

#include "xmlnmspe.hxx"


#include "xmlmetai.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

class SvXMLMetaDocumentContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentInfo>	xDocInfo;

public:
    SvXMLMetaDocumentContext(SvXMLImport& rImport, USHORT nPrfx,
                            const ::rtl::OUString& rLName,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::document::XDocumentInfo>&	rDocInfo);
    virtual ~SvXMLMetaDocumentContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

SvXMLMetaDocumentContext::SvXMLMetaDocumentContext(SvXMLImport& rImport,
                        USHORT nPrfx, const ::rtl::OUString& rLName,
                        const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                        const uno::Reference<document::XDocumentInfo>& rDocInfo) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xDocInfo(rDocInfo)
{
    // here are no attributes
}

SvXMLMetaDocumentContext::~SvXMLMetaDocumentContext()
{
}

SvXMLImportContext *SvXMLMetaDocumentContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if (  (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_META) )
    {
        return new SfxXMLMetaContext(GetImport(), nPrefix, rLocalName, xDocInfo);
    }
    else
    {
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
}

void SvXMLMetaDocumentContext::EndElement()
{
}

//===========================================================================

// #110680#
XMLMetaImportComponent::XMLMetaImportComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory) throw() 
:	SvXMLImport(xServiceFactory)
{
}

XMLMetaImportComponent::~XMLMetaImportComponent() throw()
{
}


SvXMLImportContext* XMLMetaImportComponent::CreateContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList > & xAttrList )
{
    if (  (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_DOCUMENT_META) )
    {
        return new SvXMLMetaDocumentContext(*this, nPrefix, rLocalName, xAttrList, xDocInfo);
    }
    else
    {
        return SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
}

void SAL_CALL XMLMetaImportComponent::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    xDocInfo = uno::Reference< document::XDocumentInfo >::query( xDoc );
    if( !xDocInfo.is() )
        throw lang::IllegalArgumentException();
}

uno::Sequence< ::rtl::OUString > SAL_CALL
    XMLMetaImportComponent_getSupportedServiceNames()
        throw()
{
    const ::rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.document.XMLMetaImporter" ) );
    const uno::Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

rtl::OUString SAL_CALL XMLMetaImportComponent_getImplementationName() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLMetaImportComponent" ) );
}

uno::Reference< uno::XInterface > SAL_CALL XMLMetaImportComponent_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new XMLMetaImportComponent;
    return (cppu::OWeakObject*)new XMLMetaImportComponent(rSMgr);
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
