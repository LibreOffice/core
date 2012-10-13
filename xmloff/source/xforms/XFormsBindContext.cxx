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


#include "XFormsBindContext.hxx"

#include "xformsapi.hxx"

#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlerror.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xforms/XModel.hpp>

#include <tools/debug.hxx>

using rtl::OUString;
using com::sun::star::beans::XPropertySet;
using com::sun::star::uno::Reference;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xforms::XModel;
using namespace xmloff::token;




static struct SvXMLTokenMapEntry aAttributeMap[] =
{
    TOKEN_MAP_ENTRY( NONE, NODESET ),
    TOKEN_MAP_ENTRY( NONE, ID ),
    TOKEN_MAP_ENTRY( NONE, READONLY ),
    TOKEN_MAP_ENTRY( NONE, RELEVANT ),
    TOKEN_MAP_ENTRY( NONE, REQUIRED ),
    TOKEN_MAP_ENTRY( NONE, CONSTRAINT ),
    TOKEN_MAP_ENTRY( NONE, CALCULATE ),
    TOKEN_MAP_ENTRY( NONE, TYPE ),
    XML_TOKEN_MAP_END
};

// helper function; see below
static void lcl_fillNamespaceContainer( const SvXMLNamespaceMap&,
                                 Reference<XNameContainer>& );

XFormsBindContext::XFormsBindContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XPropertySet>& xModel ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributeMap, aEmptyMap ),
        mxModel( xModel, UNO_QUERY_THROW ),
        mxBinding( NULL )
{
    // attach binding to model
    mxBinding = mxModel->createBinding();
    DBG_ASSERT( mxBinding.is(), "can't create binding" );
    mxModel->getBindings()->insert( makeAny( mxBinding ) );
}

XFormsBindContext::~XFormsBindContext()
{
}

void XFormsBindContext::HandleAttribute( sal_uInt16 nToken,
                                         const OUString& rValue )
{
    switch( nToken )
    {
    case XML_NODESET:
        xforms_setValue( mxBinding, OUSTRING("BindingExpression"), rValue );
        break;
    case XML_ID:
        xforms_setValue( mxBinding, OUSTRING("BindingID"), rValue );
        break;
    case XML_READONLY:
        xforms_setValue( mxBinding, OUSTRING("ReadonlyExpression"), rValue );
        break;
    case XML_RELEVANT:
        xforms_setValue( mxBinding, OUSTRING("RelevantExpression"), rValue );
        break;
    case XML_REQUIRED:
        xforms_setValue( mxBinding, OUSTRING("RequiredExpression"), rValue );
        break;
    case XML_CONSTRAINT:
        xforms_setValue( mxBinding, OUSTRING("ConstraintExpression"), rValue );
        break;
    case XML_CALCULATE:
        xforms_setValue( mxBinding, OUSTRING("CalculateExpression"), rValue );
        break;
    case XML_TYPE:
        xforms_setValue( mxBinding, OUSTRING("Type"),
                      makeAny( xforms_getTypeName( mxModel->getDataTypeRepository(),
                                       GetImport().GetNamespaceMap(),
                                       rValue ) ) );
        break;
    default:
        OSL_FAIL( "should not happen" );
        break;
    }
}

void XFormsBindContext::StartElement(
    const Reference<XAttributeList>& xAttributeList )
{
    // we need to register the namespaces
    Reference<XNameContainer> xContainer(
        mxBinding->getPropertyValue( OUSTRING("BindingNamespaces") ),
        UNO_QUERY );

    DBG_ASSERT( xContainer.is(), "binding should have a namespace container" );
    if( xContainer.is() )
        lcl_fillNamespaceContainer( GetImport().GetNamespaceMap(), xContainer);

    // call super-class for attribute handling
    TokenContext::StartElement( xAttributeList );
}

/** will be called for each child element */
SvXMLImportContext* XFormsBindContext::HandleChild(
    sal_uInt16,
    sal_uInt16,
    const OUString&,
    const Reference<XAttributeList>& )
{
    OSL_FAIL( "no children supported" );
    return NULL;
}


static void lcl_fillNamespaceContainer(
    const SvXMLNamespaceMap& aMap,
    Reference<XNameContainer>& xContainer )
{
    sal_uInt16 nKeyIter = aMap.GetFirstKey();
    do
    {
        // get prefix and namespace
        const OUString& sPrefix = aMap.GetPrefixByKey( nKeyIter );
        const OUString& sNamespace = aMap.GetNameByKey( nKeyIter );

        // as a hack, we will ignore our own 'default' namespaces
        DBG_ASSERT( !sPrefix.isEmpty(), "no prefix?" );
        if( sPrefix.getStr()[0] != sal_Unicode( '_' )  &&
            nKeyIter >= XML_OLD_NAMESPACE_META_IDX )
        {
            // insert prefix (use replace if already known)
            if( xContainer->hasByName( sPrefix ) )
                xContainer->replaceByName( sPrefix, makeAny( sNamespace ) );
            else
                xContainer->insertByName( sPrefix, makeAny( sNamespace ) );
        }

        // proceed to next
        nKeyIter = aMap.GetNextKey( nKeyIter );
    }
    while( nKeyIter != XML_NAMESPACE_UNKNOWN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
