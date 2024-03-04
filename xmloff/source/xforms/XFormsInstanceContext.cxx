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


#include "XFormsInstanceContext.hxx"

#include <DomBuilderContext.hxx>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/xforms/XModel2.hpp>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlerror.hxx>


using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::Sequence;
using com::sun::star::xforms::XModel2;
using com::sun::star::beans::PropertyValue;

using xmloff::token::XML_SRC;
using xmloff::token::XML_ID;

XFormsInstanceContext::XFormsInstanceContext(
    SvXMLImport& rImport,
    const Reference<XModel2> & xModel ) :
        TokenContext( rImport ),
        mxModel( xModel )
{
    SAL_WARN_IF( !mxModel.is(), "xmloff", "need model" );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XFormsInstanceContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& )
{
    SvXMLImportContext* pContext = nullptr;

    // only the first element child of an xforms:instance element
    // is used as an instance. The other children remainder must be
    // ignored.
    if( mxInstance.is() )
    {
        const OUString& rLocalName = SvXMLImport::getNameFromToken( nElement );
        GetImport().SetError( XMLERROR_XFORMS_ONLY_ONE_INSTANCE_ELEMENT, rLocalName );
    }
    else
    {
        // create new DomBuilderContext. Save reference to tree in Model.
        DomBuilderContext* pInstance = new DomBuilderContext( GetImport(), nElement );
        mxInstance = pInstance->getTree();
        pContext = pInstance;
    }

    SAL_WARN_IF( pContext == nullptr, "xmloff", "no context!" );
    return pContext;

}

css::uno::Reference< css::xml::sax::XFastContextHandler > XFormsInstanceContext::createUnknownChildContext(
    const OUString & rNamespace, const OUString &rName, const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*Attribs*/)
{
    SvXMLImportContext* pContext = nullptr;

    // only the first element child of an xforms:instance element
    // is used as an instance. The other children remainder must be
    // ignored.
    if( mxInstance.is() )
    {
        GetImport().SetError( XMLERROR_XFORMS_ONLY_ONE_INSTANCE_ELEMENT, rName );
    }
    else
    {
        // create new DomBuilderContext. Save reference to tree in Model.
        DomBuilderContext* pInstance = new DomBuilderContext( GetImport(), rNamespace, rName );
        mxInstance = pInstance->getTree();
        pContext = pInstance;
    }

    SAL_WARN_IF( pContext == nullptr, "xmloff", "no context!" );
    return pContext;

}

void XFormsInstanceContext::endFastElement(sal_Int32 )
{
    Sequence<PropertyValue> aSequence( 3 );
    PropertyValue* pSequence = aSequence.getArray();
    pSequence[0].Name = "Instance";
    pSequence[0].Value <<= mxInstance;
    pSequence[1].Name = "ID";
    pSequence[1].Value <<= msId;
    pSequence[2].Name = "URL";
    pSequence[2].Value <<= msURL;

    mxModel->getInstances()->insert( Any( aSequence ) );
}

void XFormsInstanceContext::endUnknownElement(const OUString & /*Namespace*/, const OUString & /*Name*/)
{
    Sequence<PropertyValue> aSequence( 3 );
    PropertyValue* pSequence = aSequence.getArray();
    pSequence[0].Name = "Instance";
    pSequence[0].Value <<= mxInstance;
    pSequence[1].Name = "ID";
    pSequence[1].Value <<= msId;
    pSequence[2].Name = "URL";
    pSequence[2].Value <<= msURL;

    mxModel->getInstances()->insert( Any( aSequence ) );
}

void XFormsInstanceContext::HandleAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() & TOKEN_MASK )
    {
    case XML_SRC:
        msURL = aIter.toString();
        break;
    case XML_ID:
        msId = aIter.toString();
        break;
    default:
        XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        assert( false && "this should not happen" );
        break;
    }
}

SvXMLImportContext* XFormsInstanceContext::HandleChild(
    sal_Int32,
    const Reference<css::xml::sax::XFastAttributeList>& )
{
    assert( false && "to be handled by CreateChildContext" );
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
