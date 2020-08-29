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

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "ximplink.hxx"
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;


SdXMLShapeLinkContext::SdXMLShapeLinkContext(
    SvXMLImport& rImport,
    sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
: SvXMLShapeContext( rImport, false )
, mxParent( rShapes )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if( aIter.getToken() == XML_ELEMENT(XLINK, XML_HREF) )
        {
            assert(msHyperlink.pData);
            msHyperlink = aIter.toString();
        }
        else
            SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aIter.toString());
    }
}

SdXMLShapeLinkContext::~SdXMLShapeLinkContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLShapeLinkContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLShapeContext* pContext = XMLShapeImportHelper::CreateGroupChildContext( GetImport(), nElement, xAttrList, mxParent);

    if( pContext )
    {
        pContext->setHyperlink( msHyperlink );
        return pContext;
    }

    return nullptr;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
