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

#include <com/sun/star/util/XCloneable.hpp>
#include "IgnoreTContext.hxx"
#include "TransformerBase.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/namespacemap.hxx>
#include "PersAttrListTContext.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;

void XMLPersAttrListTContext::AddAttribute(
        sal_Int32 nAttributeToken,
           ::xmloff::token::XMLTokenEnum eVToken )
{
    const OUString& aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) );
    AddAttribute( nAttributeToken, aAttrValue );
}

void XMLPersAttrListTContext::AddAttribute(
    sal_Int32 nAttributeToken,
    const OUString & rValue )
{
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

    pMutableAttrList->AddAttribute( nAttributeToken, rValue );
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nActionMap( INVALID_ACTIONS )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nActionMap( nActionMap )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName,
        sal_Int32 rQName2 ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName2 ),
    m_nActionMap( INVALID_ACTIONS )
{
}

XMLPersAttrListTContext::XMLPersAttrListTContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName,
        sal_Int32 rQName2,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName2 ),
    m_nActionMap( nActionMap )
{
}

rtl::Reference<XMLTransformerContext> XMLPersAttrListTContext::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& )
{
    // ignore all child elements
    return  new XMLIgnoreTransformerContext( GetTransformer(),
                                             nElement, true, true );
}

void XMLPersAttrListTContext::startFastElement(sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    XMLMutableAttributeList *pMutableAttrList = nullptr;

    Reference< XFastAttributeList > xAttrList( rAttrList );
    if( m_nActionMap != INVALID_ACTIONS )
    {
        pMutableAttrList =
            GetTransformer().ProcessAttrList( xAttrList, m_nActionMap,
                                                 true );
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
        m_xAttrList = new XMLMutableAttributeList( rAttrList, true );
    }
}

void XMLPersAttrListTContext::endFastElement(sal_Int32 )
{
    // ignore for now
}

void XMLPersAttrListTContext::Characters( const OUString& )
{
}

bool XMLPersAttrListTContext::IsPersistent() const
{
    return true;
}

void XMLPersAttrListTContext::Export()
{
    GetTransformer().GetDocHandler()->startFastElement( m_aElemQName, m_xAttrList );
    ExportContent();
    GetTransformer().GetDocHandler()->endFastElement( m_aElemQName );
}

void XMLPersAttrListTContext::ExportContent()
{
    // nothing to export
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
