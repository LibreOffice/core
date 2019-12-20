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
#include "XMLSectionImportContext.hxx"
#include <com/sun/star/text/SectionFileLink.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::xml::sax::XFastAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;


XMLSectionSourceImportContext::XMLSectionSourceImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rSectPropSet) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rSectionPropertySet(rSectPropSet)
{
}

XMLSectionSourceImportContext::~XMLSectionSourceImportContext()
{
}


void XMLSectionSourceImportContext::startFastElement(sal_Int32 /*nElement*/,
    const Reference<XFastAttributeList> & xAttrList)
{
    OUString sURL;
    OUString sFilterName;
    OUString sSectionName;

    sax_fastparser::FastAttributeList *pAttribList =
            sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    for (auto &aIter : *pAttribList)
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(XLINK, XML_HREF):
                sURL = aIter.toString();
                break;

            case XML_ELEMENT(TEXT, XML_FILTER_NAME):
                sFilterName = aIter.toString();
                break;

            case XML_ELEMENT(TEXT, XML_SECTION_NAME):
                sSectionName = aIter.toString();
                break;

            default:
                ; // ignore
                break;
        }
    }

    // we only need them once
    const OUString sFileLink("FileLink");
    const OUString sLinkRegion("LinkRegion");

    if (!sURL.isEmpty() || !sFilterName.isEmpty())
    {
        SectionFileLink aFileLink;
        aFileLink.FileURL = GetImport().GetAbsoluteReference( sURL );
        aFileLink.FilterName = sFilterName;

        rSectionPropertySet->setPropertyValue(sFileLink, Any(aFileLink));
    }

    if (!sSectionName.isEmpty())
    {
        rSectionPropertySet->setPropertyValue(sLinkRegion, Any(sSectionName));
    }
}

void XMLSectionSourceImportContext::endFastElement(sal_Int32 /*nElement*/)
{
    // this space intentionally left blank.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
