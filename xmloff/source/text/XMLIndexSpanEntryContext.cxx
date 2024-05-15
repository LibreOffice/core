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

#include "XMLIndexSpanEntryContext.hxx"
#include <rtl/ustring.hxx>
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>


using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::PropertyValue;


XMLIndexSpanEntryContext::XMLIndexSpanEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate ) :
        XMLIndexSimpleEntryContext(rImport, u"TokenText"_ustr,
                                   rTemplate)
{
    m_nValues++;  // one more for the text string
}

XMLIndexSpanEntryContext::~XMLIndexSpanEntryContext()
{
}

void XMLIndexSpanEntryContext::characters(const OUString& sString)
{
    sContent.append(sString);
}

void XMLIndexSpanEntryContext::FillPropertyValues(
    Sequence<PropertyValue> & rValues)
{
    // call superclass for token type, stylename,
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // content
    auto pValues = rValues.getArray();
    Any aAny;
    aAny <<= sContent.makeStringAndClear();
    pValues[m_nValues-1].Name = "Text";
    pValues[m_nValues-1].Value = aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
