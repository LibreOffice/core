/*************************************************************************
 *
 *  $RCSfile: MetaImportComponent.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hjs $ $Date: 2001-09-12 10:31:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_METAIMPORTCOMPONENT_HXX
#include "MetaImportComponent.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLMETAI_HXX
#include "xmlmetai.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::xmloff::token;

class SvXMLMetaDocumentContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentInfo>  xDocInfo;

public:
    SvXMLMetaDocumentContext(SvXMLImport& rImport, USHORT nPrfx,
                            const rtl::OUString& rLName,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::document::XDocumentInfo>& rDocInfo);
    virtual ~SvXMLMetaDocumentContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

SvXMLMetaDocumentContext::SvXMLMetaDocumentContext(SvXMLImport& rImport,
                        USHORT nPrfx, const rtl::OUString& rLName,
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
                                     const rtl::OUString& rLocalName,
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

XMLMetaImportComponent::XMLMetaImportComponent() throw() :
    SvXMLImport()
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

uno::Sequence< rtl::OUString > SAL_CALL
    XMLMetaImportComponent_getSupportedServiceNames()
        throw()
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.document.XMLMetaImporter" ) );
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
    return (cppu::OWeakObject*)new XMLMetaImportComponent;
}


