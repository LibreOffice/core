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


#include "XMLIndexBibliographyEntryContext.hxx"

#include "XMLIndexTemplateContext.hxx"


#include "xmlimp.hxx"


#include "nmspmap.hxx"

#include "xmlnmspe.hxx"


#include "xmluconv.hxx"

#include <com/sun/star/text/BibliographyDataField.hpp>
namespace binfilter {


using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;


const sal_Char sAPI_TokenType[] = "TokenType";
const sal_Char sAPI_CharacterStyleName[] = "CharacterStyleName";

TYPEINIT1( XMLIndexBibliographyEntryContext, XMLIndexSimpleEntryContext);

XMLIndexBibliographyEntryContext::XMLIndexBibliographyEntryContext(
    SvXMLImport& rImport, 
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        XMLIndexSimpleEntryContext(rImport, 
                                   rTemplate.sTokenBibliographyDataField, 
                                   rTemplate, 
                                   nPrfx, rLocalName),
        nBibliographyInfo(BibliographyDataField::IDENTIFIER),
        bBibliographyInfoOK(sal_False)
{
}

XMLIndexBibliographyEntryContext::~XMLIndexBibliographyEntryContext()
{
}

const SvXMLEnumMapEntry aBibliographyDataFieldMap[] =
{
    { XML_ADDRESS,				BibliographyDataField::ADDRESS },
    { XML_ANNOTE,				BibliographyDataField::ANNOTE },
    { XML_AUTHOR,				BibliographyDataField::AUTHOR },
    { XML_BIBLIOGRAPHY_TYPE,    BibliographyDataField::BIBILIOGRAPHIC_TYPE },
    // #96658#: also read old documents (bib*i*liographic...)
    { XML_BIBILIOGRAPHIC_TYPE,  BibliographyDataField::BIBILIOGRAPHIC_TYPE },
    { XML_BOOKTITLE,			BibliographyDataField::BOOKTITLE },
    { XML_CHAPTER,				BibliographyDataField::CHAPTER },
    { XML_CUSTOM1,				BibliographyDataField::CUSTOM1 },
    { XML_CUSTOM2,				BibliographyDataField::CUSTOM2 },
    { XML_CUSTOM3,				BibliographyDataField::CUSTOM3 },
    { XML_CUSTOM4,				BibliographyDataField::CUSTOM4 },
    { XML_CUSTOM5,				BibliographyDataField::CUSTOM5 },
    { XML_EDITION,				BibliographyDataField::EDITION },
    { XML_EDITOR,				BibliographyDataField::EDITOR },
    { XML_HOWPUBLISHED,		    BibliographyDataField::HOWPUBLISHED },
    { XML_IDENTIFIER,			BibliographyDataField::IDENTIFIER },
    { XML_INSTITUTION,			BibliographyDataField::INSTITUTION },
    { XML_ISBN,				    BibliographyDataField::ISBN },
    { XML_JOURNAL,				BibliographyDataField::JOURNAL },
    { XML_MONTH,				BibliographyDataField::MONTH },
    { XML_NOTE,				    BibliographyDataField::NOTE },
    { XML_NUMBER,				BibliographyDataField::NUMBER },
    { XML_ORGANIZATIONS,		BibliographyDataField::ORGANIZATIONS },
    { XML_PAGES,				BibliographyDataField::PAGES },
    { XML_PUBLISHER,			BibliographyDataField::PUBLISHER },
    { XML_REPORT_TYPE,			BibliographyDataField::REPORT_TYPE },
    { XML_SCHOOL,				BibliographyDataField::SCHOOL },
    { XML_SERIES,				BibliographyDataField::SERIES },
    { XML_TITLE,				BibliographyDataField::TITLE },
    { XML_URL,					BibliographyDataField::URL },
    { XML_VOLUME,				BibliographyDataField::VOLUME },
    { XML_YEAR, 				BibliographyDataField::YEAR },
    { XML_TOKEN_INVALID, 0 }
};

void XMLIndexBibliographyEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // handle both, style name and bibliography info
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
                              &sLocalName );
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( sLocalName, XML_STYLE_NAME ) )
            {
                sCharStyleName = xAttrList->getValueByIndex(nAttr);
                bCharStyleNameOK = sal_True;
            }
            else if ( IsXMLToken( sLocalName, XML_BIBLIOGRAPHY_DATA_FIELD ) )
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(
                    nTmp, xAttrList->getValueByIndex(nAttr), 
                    aBibliographyDataFieldMap))
                {
                    nBibliographyInfo = nTmp;
                    bBibliographyInfoOK = sal_True;
                }
            }
        }
    }

    // if we have a style name, set it!
    if (bCharStyleNameOK)
    {
        nValues++;
    }

    // always bibliography; else element is not valid
    nValues++;
}

void XMLIndexBibliographyEntryContext::EndElement()
{
    // only valid, if we have bibliography info
    if (bBibliographyInfoOK)
    {
        XMLIndexSimpleEntryContext::EndElement();
    }
}

void XMLIndexBibliographyEntryContext::FillPropertyValues(
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rValues)
{
    // entry name and (optionally) style name in parent class
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // bibliography data field
    sal_Int32 nIndex = bCharStyleNameOK ? 2 : 1;
    rValues[nIndex].Name = rTemplateContext.sBibliographyDataField;
    Any aAny;	
    aAny <<= nBibliographyInfo;
    rValues[nIndex].Value = aAny;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
