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

#include"xmloff/xmlnmspe.hxx"
#include "ximplink.hxx"
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

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

SdXMLShapeLinkContext::~SdXMLShapeLinkContext()
{
}

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

void SdXMLShapeLinkContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttr )
{
    SvXMLImportContext::StartElement( xAttr );
}

void SdXMLShapeLinkContext::EndElement()
{
    SvXMLImportContext::EndElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
