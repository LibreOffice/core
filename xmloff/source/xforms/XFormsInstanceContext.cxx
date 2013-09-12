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

#include "DomBuilderContext.hxx"
#include "xformsapi.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xforms/XModel2.hpp>
#include <tools/debug.hxx>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/nmspmap.hxx>


using com::sun::star::uno::Reference;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Sequence;
using com::sun::star::xforms::XModel2;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::PropertyValue;
using com::sun::star::xml::sax::XAttributeList;

using xmloff::token::IsXMLToken;
using xmloff::token::XML_INSTANCE;
using xmloff::token::XML_SRC;
using xmloff::token::XML_ID;

static const SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, SRC ),
    TOKEN_MAP_ENTRY( NONE, ID ),
    XML_TOKEN_MAP_END
};

XFormsInstanceContext::XFormsInstanceContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XModel2> & xModel ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributes, aEmptyMap ),
        mxModel( xModel )
{
    DBG_ASSERT( mxModel.is(), "need model" );
}

XFormsInstanceContext::~XFormsInstanceContext()
{
}

SvXMLImportContext* XFormsInstanceContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    SvXMLImportContext* pContext = NULL;

    // only the first element child of an xforms:instance element
    // is used as an instance. The other children remainder must be
    // ignored.
    if( mxInstance.is() )
    {
        GetImport().SetError( XMLERROR_XFORMS_ONLY_ONE_INSTANCE_ELEMENT, rLocalName );
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
    else
    {
        // create new DomBuilderContext. Save reference to tree in Model.
        DomBuilderContext* pInstance =
            new DomBuilderContext( GetImport(), nPrefix, rLocalName );
        mxInstance = pInstance->getTree();
        pContext = pInstance;
    }

    DBG_ASSERT( pContext != NULL, "no context!" );
    return pContext;

}

void XFormsInstanceContext::EndElement()
{
    Sequence<PropertyValue> aSequence( 3 );
    PropertyValue* pSequence = aSequence.getArray();
    pSequence[0].Name = OUString( "Instance" );
    pSequence[0].Value <<= mxInstance;
    pSequence[1].Name = OUString( "ID" );
    pSequence[1].Value <<= msId;
    pSequence[2].Name = OUString( "URL" );
    pSequence[2].Value <<= msURL;

    mxModel->getInstances()->insert( makeAny( aSequence ) );
}


void XFormsInstanceContext::HandleAttribute(
    sal_uInt16 nToken,
    const OUString& rValue )
{
    switch( nToken )
    {
    case XML_SRC:
        msURL = rValue;
        break;
    case XML_ID:
        msId = rValue;
        break;
    default:
        OSL_FAIL( "should not happen" );
        break;
    }
}

SvXMLImportContext* XFormsInstanceContext::HandleChild(
    sal_uInt16,
    sal_uInt16,
    const OUString&,
    const Reference<XAttributeList>& )
{
    OSL_FAIL( "to be handled by CreateChildContext" );
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
