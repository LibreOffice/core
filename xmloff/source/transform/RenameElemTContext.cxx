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

#include "RenameElemTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include <xmloff/namespacemap.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLRenameElemTransformerContext::XMLRenameElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) )
{
}

XMLRenameElemTransformerContext::XMLRenameElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
        sal_uInt16 nAPrefix,
        ::xmloff::token::XMLTokenEnum eAToken,
        ::xmloff::token::XMLTokenEnum eVToken ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) ),
    m_aAttrQName( rImp.GetNamespaceMap().GetQNameByKey( nAPrefix,
                                    ::xmloff::token::GetXMLToken( eAToken ) ) ),
    m_aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) )
{
}

XMLRenameElemTransformerContext::~XMLRenameElemTransformerContext()
{
}

void XMLRenameElemTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );
    if( !m_aAttrQName.isEmpty() )
    {
        rtl::Reference<XMLMutableAttributeList> pMutableAttrList =
            new XMLMutableAttributeList( xAttrList );
        xAttrList = pMutableAttrList;
        pMutableAttrList->AddAttribute( m_aAttrQName, m_aAttrValue );
    }
    GetTransformer().GetDocHandler()->startElement( m_aElemQName, xAttrList );
}

void XMLRenameElemTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
