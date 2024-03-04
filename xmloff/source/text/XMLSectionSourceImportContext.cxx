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

#include "XMLSectionSourceImportContext.hxx"
#include <com/sun/star/text/SectionFileLink.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;


XMLSectionSourceImportContext::XMLSectionSourceImportContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rSectPropSet) :
        SvXMLImportContext(rImport),
        rSectionPropertySet(rSectPropSet)
{
}

XMLSectionSourceImportContext::~XMLSectionSourceImportContext()
{
}

void XMLSectionSourceImportContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    OUString sURL;
    OUString sFilterName;
    OUString sSectionName;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_ELEMENT(XLINK, XML_HREF):
                sURL = sValue;
                break;

            case XML_ELEMENT(TEXT, XML_FILTER_NAME):
                sFilterName = sValue;
                break;

            case XML_ELEMENT(TEXT, XML_SECTION_NAME):
                sSectionName = sValue;
                break;

            default:
                ; // ignore
                break;
        }
    }

    if (!sURL.isEmpty() || !sFilterName.isEmpty())
    {
        SectionFileLink aFileLink;
        aFileLink.FileURL = GetImport().GetAbsoluteReference( sURL );
        aFileLink.FilterName = sFilterName;

        rSectionPropertySet->setPropertyValue("FileLink", Any(aFileLink));
    }

    if (!sSectionName.isEmpty())
    {
        rSectionPropertySet->setPropertyValue("LinkRegion", Any(sSectionName));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
