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

#include "IgnoreTContext.hxx"
#include "TransformerBase.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLIgnoreTransformerContext, XMLTransformerContext );

XMLIgnoreTransformerContext::XMLIgnoreTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_Bool bIgnoreChars,
        sal_Bool bIgnoreElems ) :
    XMLTransformerContext( rImp, rQName ),
    m_bIgnoreCharacters( bIgnoreChars ),
    m_bIgnoreElements( bIgnoreElems ),
    m_bAllowCharactersRecursive( false ),
    m_bRecursiveUse( sal_False )
{
}

XMLIgnoreTransformerContext::XMLIgnoreTransformerContext(
        XMLTransformerBase& rTransformer,
        const OUString& rQName,
        sal_Bool bAllowCharactersRecursive ) :
    XMLTransformerContext( rTransformer, rQName ),
    m_bIgnoreCharacters( sal_False ),
    m_bIgnoreElements( sal_False ),
    m_bAllowCharactersRecursive( bAllowCharactersRecursive ),
    m_bRecursiveUse( sal_True )
{
}

XMLIgnoreTransformerContext::~XMLIgnoreTransformerContext()
{
}

XMLTransformerContext *XMLIgnoreTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& xAttrList )
{
    XMLTransformerContext *pContext = 0;
    if( m_bIgnoreElements )
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName, sal_True,
                                                    sal_True );
    else if (m_bRecursiveUse)
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName, m_bAllowCharactersRecursive );
    else
        pContext = XMLTransformerContext::CreateChildContext(
                        nPrefix, rLocalName, rQName, xAttrList );

    return pContext;
}

void XMLIgnoreTransformerContext::StartElement( const Reference< XAttributeList >& )
{
    
}

void XMLIgnoreTransformerContext::EndElement()
{
    
}

void XMLIgnoreTransformerContext::Characters( const OUString& rChars )
{
    if( !m_bIgnoreCharacters )
        GetTransformer().GetDocHandler()->characters( rChars );
    else if ( m_bRecursiveUse && m_bAllowCharactersRecursive )
        GetTransformer().GetDocHandler()->characters( rChars );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
