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

#include "SchemaContext.hxx"

#include "SchemaSimpleTypeContext.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>

#include <tools/debug.hxx>


using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xforms::XDataTypeRepository;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XSD, SIMPLETYPE ),
    XML_TOKEN_MAP_END
};


SchemaContext::SchemaContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XDataTypeRepository>& rRepository ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
        mxRepository( rRepository )
{
}

SchemaContext::~SchemaContext()
{
}

void SchemaContext::HandleAttribute(
    sal_uInt16,
    const OUString& )
{
}

SvXMLImportContext* SchemaContext::HandleChild(
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    return ( nToken == XML_SIMPLETYPE )
        ? new SchemaSimpleTypeContext( GetImport(), nPrefix, rLocalName,
                                       mxRepository )
        : new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}
