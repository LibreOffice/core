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

#include "XMLIndexSpanEntryContext.hxx"

#include "rtl/ustring.hxx"

#include "XMLIndexTemplateContext.hxx"

namespace binfilter {


using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::PropertyValue;


TYPEINIT1( XMLIndexSpanEntryContext, XMLIndexSimpleEntryContext);

XMLIndexSpanEntryContext::XMLIndexSpanEntryContext(
    SvXMLImport& rImport, 
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        XMLIndexSimpleEntryContext(rImport, rTemplate.sTokenText, 
                                   rTemplate, nPrfx, rLocalName)
{
    nValues++;	// one more for the text string
}

XMLIndexSpanEntryContext::~XMLIndexSpanEntryContext()
{
}

void XMLIndexSpanEntryContext::Characters(const OUString& sString)
{
    sContent.append(sString);
}

void XMLIndexSpanEntryContext::FillPropertyValues(
    Sequence<PropertyValue> & rValues)
{
    // call superclass for token type, stylename, 
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // content
    Any aAny;
    aAny <<= sContent.makeStringAndClear();
    rValues[nValues-1].Name = rTemplateContext.sText;
    rValues[nValues-1].Value = aAny;
}

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
