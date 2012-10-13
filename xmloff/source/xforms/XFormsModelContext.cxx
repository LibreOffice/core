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


#include "XFormsModelContext.hxx"

#include "XFormsBindContext.hxx"
#include "XFormsSubmissionContext.hxx"
#include "XFormsInstanceContext.hxx"
#include "SchemaContext.hxx"
#include "xformsapi.hxx"

#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlerror.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/xforms/XModel.hpp>


using rtl::OUString;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
using com::sun::star::util::XUpdatable;
using namespace com::sun::star::uno;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, ID ),
    TOKEN_MAP_ENTRY( NONE, SCHEMA ),
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XFORMS, INSTANCE ),
    TOKEN_MAP_ENTRY( XFORMS, BIND ),
    TOKEN_MAP_ENTRY( XFORMS, SUBMISSION ),
    TOKEN_MAP_ENTRY( XSD,    SCHEMA ),
    XML_TOKEN_MAP_END
};


XFormsModelContext::XFormsModelContext( SvXMLImport& rImport,
                                        sal_uInt16 nPrefix,
                                        const OUString& rLocalName ) :
    TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
    mxModel( xforms_createXFormsModel() )
{
}

XFormsModelContext::~XFormsModelContext()
{
}


Reference<XPropertySet> XFormsModelContext::getModel()
{
    return mxModel;
}


void XFormsModelContext::HandleAttribute(
    sal_uInt16 nToken,
    const OUString& rValue )
{
    switch( nToken )
    {
    case XML_ID:
        mxModel->setPropertyValue( OUSTRING("ID"), makeAny( rValue ) );
        break;
    case XML_SCHEMA:
        GetImport().SetError( XMLERROR_XFORMS_NO_SCHEMA_SUPPORT );
        break;
    default:
        OSL_FAIL( "this should not happen" );
        break;
    }
}

SvXMLImportContext* XFormsModelContext::HandleChild(
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    SvXMLImportContext* pContext = NULL;

    switch( nToken )
    {
    case XML_INSTANCE:
        pContext = new XFormsInstanceContext( GetImport(), nPrefix, rLocalName,
                                              mxModel );
        break;
    case XML_BIND:
        pContext = new XFormsBindContext( GetImport(), nPrefix, rLocalName,
                                          mxModel );
        break;
    case XML_SUBMISSION:
        pContext = new XFormsSubmissionContext( GetImport(), nPrefix,
                                                rLocalName, mxModel );
        break;
    case XML_SCHEMA:
        pContext = new SchemaContext(
            GetImport(), nPrefix, rLocalName,
            Reference<com::sun::star::xforms::XModel>( mxModel,
                                                       UNO_QUERY_THROW )
                ->getDataTypeRepository() );
        break;
    default:
        OSL_FAIL( "Boooo!" );
        break;
    }

    return pContext;
}

void XFormsModelContext::EndElement()
{
    // update before putting model into document
    Reference<XUpdatable> xUpdate( mxModel, UNO_QUERY );
    if( xUpdate.is() )
        xUpdate->update();

    GetImport().initXForms();
    xforms_addXFormsModel( GetImport().GetModel(), getModel() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
