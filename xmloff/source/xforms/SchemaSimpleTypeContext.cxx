/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
