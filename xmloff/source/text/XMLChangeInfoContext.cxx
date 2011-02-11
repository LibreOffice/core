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
#include "XMLChangeInfoContext.hxx"
#include "XMLChangedRegionImportContext.hxx"
#include "XMLStringBufferImportContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>



using namespace ::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


TYPEINIT1(XMLChangeInfoContext, SvXMLImportContext);

XMLChangeInfoContext::XMLChangeInfoContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    XMLChangedRegionImportContext& rPParent,
    const OUString& rChangeType)
:   SvXMLImportContext(rImport, nPrefix, rLocalName)
,   rType(rChangeType)
,   rChangedRegion(rPParent)
{
}

XMLChangeInfoContext::~XMLChangeInfoContext()
{
}

void XMLChangeInfoContext::StartElement(const Reference<XAttributeList> &)
{
    // no attributes
}

SvXMLImportContext* XMLChangeInfoContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CREATOR ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, sAuthorBuffer);
        else if( IsXMLToken( rLocalName, XML_DATE ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, sDateTimeBuffer);
    }
    else if ( ( XML_NAMESPACE_TEXT == nPrefix ) &&
         IsXMLToken( rLocalName, XML_P )       )
    {
        pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                                   rLocalName, sCommentBuffer);
    }

    if( !pContext )
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}

void XMLChangeInfoContext::EndElement()
{
    // set values at changed region context
    rChangedRegion.SetChangeInfo(rType, sAuthorBuffer.makeStringAndClear(),
                                 sCommentBuffer.makeStringAndClear(),
                                 sDateTimeBuffer.makeStringAndClear());
}
