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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <com/sun/star/util/XCloneable.hpp>
#include "IgnoreTContext.hxx"
#include "TransformerBase.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/nmspmap.hxx>
#include "PersAttrListTContext.hxx"

using ::rtl::OUString;

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
    const ::rtl::OUString & rValue )
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
    // ignore all child elements
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
    // ignore for now
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
    // nothing to export
}

Reference< XAttributeList > XMLPersAttrListTContext::GetAttrList() const
{
    return m_xAttrList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
