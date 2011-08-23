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


#include "XMLIndexChapterInfoEntryContext.hxx"

#include "XMLIndexTemplateContext.hxx"


#include "xmlimp.hxx"


#include "nmspmap.hxx"

#include "xmlnmspe.hxx"


#include "xmluconv.hxx"

#include <com/sun/star/text/ChapterFormat.hpp>
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



TYPEINIT1( XMLIndexChapterInfoEntryContext, XMLIndexSimpleEntryContext);

XMLIndexChapterInfoEntryContext::XMLIndexChapterInfoEntryContext(
    SvXMLImport& rImport, 
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        XMLIndexSimpleEntryContext(rImport, rTemplate.sTokenChapterInfo,
                                   rTemplate, nPrfx, rLocalName),
        nChapterInfo(ChapterFormat::NAME_NUMBER),
        bChapterInfoOK(sal_False)
{
}

XMLIndexChapterInfoEntryContext::~XMLIndexChapterInfoEntryContext()
{
}

static const SvXMLEnumMapEntry aChapterDisplayMap[] =
{
    { XML_NAME,				    	ChapterFormat::NAME },
    { XML_NUMBER,					ChapterFormat::NUMBER },
    { XML_NUMBER_AND_NAME,			ChapterFormat::NAME_NUMBER },
// not supported by the template:
//	{ XML_PLAIN_NUMBER_AND_NAME,	ChapterFormat::NO_PREFIX_SUFFIX },
//	{ XML_PLAIN_NUMBER,	    		ChapterFormat::DIGIT },
    { XML_TOKEN_INVALID,			0 }
};

void XMLIndexChapterInfoEntryContext::StartElement(
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
            else if ( IsXMLToken( sLocalName, XML_DISPLAY ) )
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(
                    nTmp, xAttrList->getValueByIndex(nAttr), 
                    aChapterDisplayMap))
                {
                    nChapterInfo = nTmp;
                    bChapterInfoOK = sal_True;
                }
            }
        }
    }

    // if we have a style name, set it!
    if (bCharStyleNameOK)
    {
        nValues++;
    }

    // if we have chaper info, set it!
    if (bChapterInfoOK)
    {
        nValues++;
    }
}

void XMLIndexChapterInfoEntryContext::FillPropertyValues(
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rValues)
{
    // entry name and (optionally) style name in parent class
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // chapter info field
    sal_Int32 nIndex = bCharStyleNameOK ? 2 : 1;
    rValues[nIndex].Name = rTemplateContext.sChapterFormat;
    Any aAny;	
    aAny <<= nChapterInfo;
    rValues[nIndex].Value = aAny;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
