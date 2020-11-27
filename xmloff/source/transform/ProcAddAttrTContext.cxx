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

#include "ProcAddAttrTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include <xmloff/namespacemap.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

XMLProcAddAttrTransformerContext::XMLProcAddAttrTransformerContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName,
        sal_Int32 rQName2,
        sal_uInt16 nActionMap,
        sal_Int32 rQName3,
           ::xmloff::token::XMLTokenEnum eVToken ) :
    XMLProcAttrTransformerContext( rImp, rQName, rQName2, nActionMap ),
    m_aAttrQName( rQName3 ),
    m_aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) )
{
}

XMLProcAddAttrTransformerContext::~XMLProcAddAttrTransformerContext()
{
}

void XMLProcAddAttrTransformerContext::startFastElement(sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    Reference< XFastAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList =
        GetTransformer().ProcessAttrList( xAttrList, GetActionMap(),
                                          false );
    if( !pMutableAttrList )
    {
        pMutableAttrList = new XMLMutableAttributeList;
        xAttrList = pMutableAttrList;
    }
    pMutableAttrList->AddAttribute( m_aAttrQName, m_aAttrValue );
    GetTransformer().GetDocHandler()->startFastElement( GetElemQName(), xAttrList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
