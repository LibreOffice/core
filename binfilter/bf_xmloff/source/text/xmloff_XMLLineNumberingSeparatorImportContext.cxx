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

#include "XMLLineNumberingSeparatorImportContext.hxx"

#include "XMLLineNumberingImportContext.hxx"

#include "xmlimp.hxx"

#include "nmspmap.hxx"

#include "xmlnmspe.hxx"


#include "xmluconv.hxx"
namespace binfilter {


using namespace ::com::sun::star::uno;

using ::com::sun::star::xml::sax::XAttributeList;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_INCREMENT;

TYPEINIT1( XMLLineNumberingSeparatorImportContext, SvXMLImportContext );

XMLLineNumberingSeparatorImportContext::XMLLineNumberingSeparatorImportContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    XMLLineNumberingImportContext& rLineNumbering) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rLineNumberingContext(rLineNumbering)
{
}

XMLLineNumberingSeparatorImportContext::~XMLLineNumberingSeparatorImportContext()
{
}

void XMLLineNumberingSeparatorImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++) 
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        if ( (nPrefix == XML_NAMESPACE_TEXT) &&
             IsXMLToken(sLocalName, XML_INCREMENT) )
        {
            sal_Int32 nTmp;
            if (SvXMLUnitConverter::convertNumber(
                nTmp, xAttrList->getValueByIndex(i), 0))
            {
                rLineNumberingContext.SetSeparatorIncrement((sal_Int16)nTmp);
            }
            // else: invalid number -> ignore
        }
        // else: unknown attribute -> ignore
    }
}

void XMLLineNumberingSeparatorImportContext::Characters(
    const OUString& rChars )
{
    sSeparatorBuf.append(rChars);
}

void XMLLineNumberingSeparatorImportContext::EndElement()
{
    rLineNumberingContext.SetSeparatorText(sSeparatorBuf.makeStringAndClear());
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
