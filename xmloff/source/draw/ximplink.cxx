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
#include "precompiled_xmloff.hxx"
#include"xmloff/xmlnmspe.hxx"
#include "ximplink.hxx"
#include <xmloff/xmltoken.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLShapeLinkContext, SvXMLImportContext );

SdXMLShapeLinkContext::SdXMLShapeLinkContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< xml::sax::XAttributeList>& xAttrList, uno::Reference< drawing::XShapes >& rShapes)
: SvXMLShapeContext( rImport, nPrfx, rLocalName, false )
, mxParent( rShapes )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if( (nPrefix == XML_NAMESPACE_XLINK) && IsXMLToken( aLocalName, XML_HREF ) )
        {
            msHyperlink = xAttrList->getValueByIndex( i );
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLShapeLinkContext::~SdXMLShapeLinkContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLShapeLinkContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLShapeContext* pContext = GetImport().GetShapeImport()->CreateGroupChildContext( GetImport(), nPrefix, rLocalName, xAttrList, mxParent);

    if( pContext )
    {
        pContext->setHyperlink( msHyperlink );
        return pContext;
    }

    // call parent when no own context was created
    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList);

}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeLinkContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttr )
{
    SvXMLImportContext::StartElement( xAttr );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeLinkContext::EndElement()
{
    SvXMLImportContext::EndElement();
}


