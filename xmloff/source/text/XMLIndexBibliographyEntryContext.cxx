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

#include "XMLIndexBibliographyEntryContext.hxx"
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <com/sun/star/text/BibliographyDataField.hpp>
#include <sal/log.hxx>


using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;


XMLIndexBibliographyEntryContext::XMLIndexBibliographyEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate ) :
        XMLIndexSimpleEntryContext(rImport,
                                   "TokenBibliographyDataField",
                                   rTemplate),
        nBibliographyInfo(BibliographyDataField::IDENTIFIER),
        bBibliographyInfoOK(false)
{
}

XMLIndexBibliographyEntryContext::~XMLIndexBibliographyEntryContext()
{
}

const SvXMLEnumMapEntry<sal_uInt16> aBibliographyDataFieldMap[] =
{
    { XML_ADDRESS,              BibliographyDataField::ADDRESS },
    { XML_ANNOTE,               BibliographyDataField::ANNOTE },
    { XML_AUTHOR,               BibliographyDataField::AUTHOR },
    { XML_BIBLIOGRAPHY_TYPE,    BibliographyDataField::BIBILIOGRAPHIC_TYPE },
    // #96658#: also read old documents (bib*i*liographic...)
    { XML_BIBILIOGRAPHIC_TYPE,  BibliographyDataField::BIBILIOGRAPHIC_TYPE },
    { XML_BOOKTITLE,            BibliographyDataField::BOOKTITLE },
    { XML_CHAPTER,              BibliographyDataField::CHAPTER },
    { XML_CUSTOM1,              BibliographyDataField::CUSTOM1 },
    { XML_CUSTOM2,              BibliographyDataField::CUSTOM2 },
    { XML_CUSTOM3,              BibliographyDataField::CUSTOM3 },
    { XML_CUSTOM4,              BibliographyDataField::CUSTOM4 },
    { XML_CUSTOM5,              BibliographyDataField::CUSTOM5 },
    { XML_EDITION,              BibliographyDataField::EDITION },
    { XML_EDITOR,               BibliographyDataField::EDITOR },
    { XML_HOWPUBLISHED,         BibliographyDataField::HOWPUBLISHED },
    { XML_IDENTIFIER,           BibliographyDataField::IDENTIFIER },
    { XML_INSTITUTION,          BibliographyDataField::INSTITUTION },
    { XML_ISBN,                 BibliographyDataField::ISBN },
    { XML_JOURNAL,              BibliographyDataField::JOURNAL },
    { XML_MONTH,                BibliographyDataField::MONTH },
    { XML_NOTE,                 BibliographyDataField::NOTE },
    { XML_NUMBER,               BibliographyDataField::NUMBER },
    { XML_ORGANIZATIONS,        BibliographyDataField::ORGANIZATIONS },
    { XML_PAGES,                BibliographyDataField::PAGES },
    { XML_PUBLISHER,            BibliographyDataField::PUBLISHER },
    { XML_REPORT_TYPE,          BibliographyDataField::REPORT_TYPE },
    { XML_SCHOOL,               BibliographyDataField::SCHOOL },
    { XML_SERIES,               BibliographyDataField::SERIES },
    { XML_TITLE,                BibliographyDataField::TITLE },
    { XML_URL,                  BibliographyDataField::URL },
    { XML_VOLUME,               BibliographyDataField::VOLUME },
    { XML_YEAR,                 BibliographyDataField::YEAR },
    { XML_TOKEN_INVALID, 0 }
};

void XMLIndexBibliographyEntryContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // handle both, style name and bibliography info
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_STYLE_NAME):
            {
                m_sCharStyleName = aIter.toString();
                m_bCharStyleNameOK = true;
                break;
            }
            case XML_ELEMENT(TEXT, XML_BIBLIOGRAPHY_DATA_FIELD):
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(nTmp, aIter.toView(), aBibliographyDataFieldMap))
                {
                    nBibliographyInfo = nTmp;
                    bBibliographyInfoOK = true;
                }
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    // if we have a style name, set it!
    if (m_bCharStyleNameOK)
    {
        m_nValues++;
    }

    // always bibliography; else element is not valid
    m_nValues++;
}

void XMLIndexBibliographyEntryContext::endFastElement(sal_Int32 nElement)
{
    // only valid, if we have bibliography info
    if (bBibliographyInfoOK)
    {
        XMLIndexSimpleEntryContext::endFastElement(nElement);
    }
}

void XMLIndexBibliographyEntryContext::FillPropertyValues(
    css::uno::Sequence<css::beans::PropertyValue> & rValues)
{
    // entry name and (optionally) style name in parent class
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // bibliography data field
    sal_Int32 nIndex = m_bCharStyleNameOK ? 2 : 1;
    auto pValues = rValues.getArray();
    pValues[nIndex].Name = "BibliographyDataField";
    pValues[nIndex].Value <<= nBibliographyInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
