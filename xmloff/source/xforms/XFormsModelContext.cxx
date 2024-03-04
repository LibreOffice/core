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


#include "XFormsModelContext.hxx"

#include "XFormsBindContext.hxx"
#include "XFormsSubmissionContext.hxx"
#include "XFormsInstanceContext.hxx"
#include "SchemaContext.hxx"
#include "xformsapi.hxx"

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>

#include <sal/log.hxx>

#include <com/sun/star/util/XUpdatable.hpp>


using com::sun::star::util::XUpdatable;
using namespace com::sun::star::uno;
using namespace xmloff::token;


XFormsModelContext::XFormsModelContext( SvXMLImport& rImport ) :
    TokenContext( rImport ),
    mxModel( xforms_createXFormsModel() )
{
}

void XFormsModelContext::HandleAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() & TOKEN_MASK)
    {
    case XML_ID:
        mxModel->setPropertyValue( "ID", Any( aIter.toString() ) );
        break;
    case XML_SCHEMA:
        GetImport().SetError( XMLERROR_XFORMS_NO_SCHEMA_SUPPORT );
        break;
    default:
        XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        assert( false && "this should not happen" );
        break;
    }
}

SvXMLImportContext* XFormsModelContext::HandleChild(
    sal_Int32 nElementToken,
    const Reference<css::xml::sax::XFastAttributeList>& )
{
    SvXMLImportContext* pContext = nullptr;

    switch( nElementToken )
    {
    case XML_ELEMENT(XFORMS, XML_INSTANCE):
        pContext = new XFormsInstanceContext( GetImport(), mxModel );
        break;
    case XML_ELEMENT(XFORMS, XML_BIND):
        pContext = new XFormsBindContext( GetImport(), mxModel );
        break;
    case XML_ELEMENT(XFORMS, XML_SUBMISSION):
        pContext = new XFormsSubmissionContext( GetImport(), mxModel );
        break;
    case XML_ELEMENT(XSD, XML_SCHEMA):
        pContext = new SchemaContext( GetImport(), mxModel->getDataTypeRepository() );
        break;
    default:
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElementToken);
        assert( false && "Boooo!" );
        break;
    }

    return pContext;
}

void XFormsModelContext::endFastElement(sal_Int32 )
{
    // update before putting model into document
    Reference<XUpdatable> xUpdate( mxModel, UNO_QUERY );
    if( xUpdate.is() )
        xUpdate->update();

    GetImport().initXForms();
    xforms_addXFormsModel( GetImport().GetModel(), mxModel );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
