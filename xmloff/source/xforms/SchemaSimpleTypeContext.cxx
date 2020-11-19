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


#include "SchemaSimpleTypeContext.hxx"

#include "SchemaRestrictionContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlimp.hxx>

#include <osl/diagnose.h>

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XFastAttributeList;
using com::sun::star::xforms::XDataTypeRepository;
using namespace xmloff::token;


const SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, NAME ),
    XML_TOKEN_MAP_END
};

SchemaSimpleTypeContext::SchemaSimpleTypeContext(
    SvXMLImport& rImport,
    const Reference<XDataTypeRepository>& rRepository ) :
        TokenContext( rImport, aAttributes ),
        mxRepository( rRepository )
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
    sal_Int32 nElementToken,
    const Reference<XFastAttributeList>& )
{
    switch( nElementToken )
    {
    case XML_ELEMENT(XSD, XML_RESTRICTION):
        return new SchemaRestrictionContext( GetImport(),
                                                 mxRepository, msTypeName );
        break;
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
