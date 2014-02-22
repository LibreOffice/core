/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ControlOOoTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/xmlnmspe.hxx>
#include "ActionMapTypesOOo.hxx"
#include "ElemTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLControlOOoTransformerContext, XMLTransformerContext );

XMLControlOOoTransformerContext::XMLControlOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLControlOOoTransformerContext::~XMLControlOOoTransformerContext()
{
}

void XMLControlOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
}

XMLTransformerContext *XMLControlOOoTransformerContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( m_aElemQName.isEmpty() )
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_False, sal_False );
        m_aElemQName = rQName;
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( rAttrList );
        GetTransformer().ProcessAttrList( m_xAttrList,
                                          OOO_FORM_CONTROL_ACTIONS,
                                          sal_False );
        GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                        m_xAttrList );
    }
    else
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_True, sal_True );
    }
    return pContext;
}

void XMLControlOOoTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLControlOOoTransformerContext::Characters( const OUString& rChars )
{
    
    if( !m_aElemQName.isEmpty() )
        XMLTransformerContext::Characters( rChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
