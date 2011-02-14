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

#include "SchemaSimpleTypeContext.hxx"

#include "SchemaRestrictionContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>

#include <tools/debug.hxx>


using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
using com::sun::star::xforms::XDataTypeRepository;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, NAME ),
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XSD, RESTRICTION ),
    XML_TOKEN_MAP_END
};


SchemaSimpleTypeContext::SchemaSimpleTypeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XDataTypeRepository>& rRepository ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
        mxRepository( rRepository )
{
}

SchemaSimpleTypeContext::~SchemaSimpleTypeContext()
{
}

void SchemaSimpleTypeContext::HandleAttribute(
    sal_uInt16 nToken,
    const OUString& rValue )
{
    if( nToken == XML_NAME )
    {
        msTypeName = rValue;
    }
}

SvXMLImportContext* SchemaSimpleTypeContext::HandleChild(
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    SvXMLImportContext* pContext = NULL;
    switch( nToken )
    {
    case XML_RESTRICTION:
        pContext = new SchemaRestrictionContext( GetImport(),
                                                 nPrefix, rLocalName,
                                                 mxRepository, msTypeName );
        break;
    default:
        DBG_ERROR( "Booo!" );
    }

    return ( pContext != NULL )
        ? pContext
        : new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}
