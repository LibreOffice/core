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

#include <com/sun/star/util/XCloneable.hpp>
#include "IgnoreTContext.hxx"
#include "TransformerBase.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/nmspmap.hxx>
#include "PersAttrListTContext.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLPersAttrListTContext, XMLTransformerContext );

void XMLPersAttrListTContext::AddAttribute(
        sal_uInt16 nAPrefix,
           ::xmloff::token::XMLTokenEnum eAToken,
           ::xmloff::token::XMLTokenEnum eVToken )
{
    OUString aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) );
    AddAttribute( nAPrefix, eAToken, aAttrValue );
}

void XMLPersAttrListTContext::AddAttribute(
    sal_uInt16 nAPrefix,
    ::xmloff::token::XMLTokenEnum eAToken,
    const OUString & rValue )
{
    OUString aAttrQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                nAPrefix, ::xmloff::token::GetXMLToken( eAToken ) ) );
    OUString aAttrValue( rValue );

    XMLMutableAttributeList *pMutableAttrList;
    if( m_xAttrList.is() )
    {
        pMutableAttrList =
            static_cast< XMLMutableAttributeList * >( m_xAttrList.get() );
    }
    else
    {
        pMutableAttrList = new XMLMutableAttributeList ;
        m_xAttrList = pMutableAttrList;
    }

    pMutableAttrList->AddAttribute( aAttrQName, aAttrValue );
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nActionMap( INVALID_ACTIONS )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nActionMap( nActionMap )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) ),
    m_nActionMap( INVALID_ACTIONS )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) ),
    m_nActionMap( nActionMap )
{
}

XMLPersAttrListTContext::~XMLPersAttrListTContext()
{
}

XMLTransformerContext *XMLPersAttrListTContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& )
{
    
    return  new XMLIgnoreTransformerContext( GetTransformer(),
                                             rQName, sal_True, sal_True );
}

void XMLPersAttrListTContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLMutableAttributeList *pMutableAttrList = 0;

    Reference< XAttributeList > xAttrList( rAttrList );
    if( m_nActionMap != INVALID_ACTIONS )
    {
        pMutableAttrList =
            GetTransformer().ProcessAttrList( xAttrList, m_nActionMap,
                                                 sal_True );
    }

    if( m_xAttrList.is() )
    {
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( xAttrList );
    }
    else if( pMutableAttrList )
    {
        m_xAttrList = xAttrList;
    }
    else
    {
        m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
    }
}

void XMLPersAttrListTContext::EndElement()
{
    
}

void XMLPersAttrListTContext::Characters( const OUString& )
{
}

sal_Bool XMLPersAttrListTContext::IsPersistent() const
{
    return sal_True;
}

void XMLPersAttrListTContext::Export()
{
    GetTransformer().GetDocHandler()->startElement( m_aElemQName, m_xAttrList );
    ExportContent();
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLPersAttrListTContext::ExportContent()
{
    
}

Reference< XAttributeList > XMLPersAttrListTContext::GetAttrList() const
{
    return m_xAttrList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
