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


#include "XFormsBindContext.hxx"

#include "xformsapi.hxx"

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xforms/XModel2.hpp>

#include <sal/log.hxx>

using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::sax::XFastAttributeList;
using com::sun::star::xforms::XModel2;
using namespace xmloff::token;

// helper function; see below
static void lcl_fillNamespaceContainer( const SvXMLNamespaceMap&,
                                 Reference<XNameContainer> const & );

XFormsBindContext::XFormsBindContext(
    SvXMLImport& rImport,
    const Reference<XModel2>& xModel ) :
        TokenContext( rImport ),
        mxModel( xModel )
{
    // attach binding to model
    mxBinding = mxModel->createBinding();
    SAL_WARN_IF( !mxBinding.is(), "xmloff", "can't create binding" );
    mxModel->getBindings()->insert( Any( mxBinding ) );
}

void XFormsBindContext::HandleAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() & TOKEN_MASK )
    {
    case XML_NODESET:
        xforms_setValue( mxBinding, u"BindingExpression"_ustr, aIter.toString() );
        break;
    case XML_ID:
        xforms_setValue( mxBinding, u"BindingID"_ustr, aIter.toString() );
        break;
    case XML_READONLY:
        xforms_setValue( mxBinding, u"ReadonlyExpression"_ustr, aIter.toString() );
        break;
    case XML_RELEVANT:
        xforms_setValue( mxBinding, u"RelevantExpression"_ustr, aIter.toString() );
        break;
    case XML_REQUIRED:
        xforms_setValue( mxBinding, u"RequiredExpression"_ustr, aIter.toString() );
        break;
    case XML_CONSTRAINT:
        xforms_setValue( mxBinding, u"ConstraintExpression"_ustr, aIter.toString() );
        break;
    case XML_CALCULATE:
        xforms_setValue( mxBinding, u"CalculateExpression"_ustr, aIter.toString() );
        break;
    case XML_TYPE:
        xforms_setValue( mxBinding, u"Type"_ustr,
                         xforms_getTypeName( mxModel->getDataTypeRepository(),
                                       GetImport().GetNamespaceMap(),
                                       aIter.toString() ) );
        break;
    default:
        assert( false && "should not happen" );
        break;
    }
}

void XFormsBindContext::startFastElement(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // we need to register the namespaces
    Reference<XNameContainer> xContainer(
        mxBinding->getPropertyValue( u"BindingNamespaces"_ustr ),
        UNO_QUERY );

    SAL_WARN_IF( !xContainer.is(), "xmloff", "binding should have a namespace container" );
    if( xContainer.is() )
        lcl_fillNamespaceContainer( GetImport().GetNamespaceMap(), xContainer);

    // call super-class for attribute handling
    TokenContext::startFastElement( nElement, xAttrList );
}

/** will be called for each child element */
SvXMLImportContext* XFormsBindContext::HandleChild(
    sal_Int32,
    const Reference<XFastAttributeList>& )
{
    assert( false && "no children supported" );
    return nullptr;
}


static void lcl_fillNamespaceContainer(
    const SvXMLNamespaceMap& aMap,
    Reference<XNameContainer> const & xContainer )
{
    sal_uInt16 nKeyIter = aMap.GetFirstKey();
    do
    {
        // get prefix and namespace
        const OUString& sPrefix = aMap.GetPrefixByKey( nKeyIter );
        const OUString& sNamespace = aMap.GetNameByKey( nKeyIter );

        // as a hack, we will ignore our own 'default' namespaces
        SAL_WARN_IF( sPrefix.isEmpty(), "xmloff", "no prefix?" );
        if( !sPrefix.startsWith("_") &&
            nKeyIter >= XML_NAMESPACE_META_SO52)
        {
            // insert prefix (use replace if already known)
            if( xContainer->hasByName( sPrefix ) )
                xContainer->replaceByName( sPrefix, Any( sNamespace ) );
            else
                xContainer->insertByName( sPrefix, Any( sNamespace ) );
        }

        // proceed to next
        nKeyIter = aMap.GetNextKey( nKeyIter );
    }
    while( nKeyIter != XML_NAMESPACE_UNKNOWN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
