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

#include "XMLIndexBibliographySourceContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <sal/log.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;


XMLIndexBibliographySourceContext::XMLIndexBibliographySourceContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet)
    : XMLIndexSourceBaseContext(rImport, rPropSet, UseStyles::None)
{
}

XMLIndexBibliographySourceContext::~XMLIndexBibliographySourceContext()
{
}

void XMLIndexBibliographySourceContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    // We have no attributes. Who wants attributes, anyway?
    XMLOFF_WARN_UNKNOWN("xmloff", aIter);
}

void XMLIndexBibliographySourceContext::endFastElement(sal_Int32 )
{
    // No attributes, no properties.
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexBibliographySourceContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_BIBLIOGRAPHY_ENTRY_TEMPLATE) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           aLevelNameBibliographyMap,
                                           XML_BIBLIOGRAPHY_TYPE,
                                           aLevelStylePropNameBibliographyMap,
                                           aAllowedTokenTypesBibliography);
    }
    else
    {
        return XMLIndexSourceBaseContext::createFastChildContext(nElement,
                                                             xAttrList);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
