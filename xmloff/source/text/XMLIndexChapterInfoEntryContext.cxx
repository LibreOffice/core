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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"


#include "XMLIndexChapterInfoEntryContext.hxx"
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/text/ChapterFormat.hpp>


using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

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
    const OUString& rLocalName,
    sal_Bool bT ) :
        XMLIndexSimpleEntryContext(rImport,
                                   (bT ? rTemplate.sTokenEntryNumber
                                          : rTemplate.sTokenChapterInfo),
                                   rTemplate, nPrfx, rLocalName),
        nChapterInfo(ChapterFormat::NAME_NUMBER),
        bChapterInfoOK(sal_False),
        bTOC( bT ),
        nOutlineLevel( 0 ),
        bOutlineLevelOK(sal_False)
{
}

XMLIndexChapterInfoEntryContext::~XMLIndexChapterInfoEntryContext()
{
}

static const SvXMLEnumMapEntry aChapterDisplayMap[] =
{
    { XML_NAME,                     ChapterFormat::NAME },
    { XML_NUMBER,                   ChapterFormat::NUMBER },
    { XML_NUMBER_AND_NAME,          ChapterFormat::NAME_NUMBER },
    //---> i89791
    // enabled for ODF 1.2, full index support in 3.0
    { XML_PLAIN_NUMBER_AND_NAME,    ChapterFormat::NO_PREFIX_SUFFIX },
    //<---
    { XML_PLAIN_NUMBER,             ChapterFormat::DIGIT },
    { XML_TOKEN_INVALID,            0 }
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
            else if ( IsXMLToken( sLocalName, XML_DISPLAY ) )//i53420, always true, in TOC as well
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
            else if ( IsXMLToken( sLocalName, XML_OUTLINE_LEVEL ) )
            {
                sal_Int32 nTmp;

                if (SvXMLUnitConverter::convertNumber(nTmp, xAttrList->getValueByIndex(nAttr)))
                {
//control on range is carried out in the UNO level
                    nOutlineLevel = static_cast<sal_uInt16>(nTmp);
                    bOutlineLevelOK = sal_True;
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
        /* Some of the index chapter information attributes written to ODF 1.1
           and 1.2 don't reflect the displaying (#i89791#)
        */
        if ( !bTOC )
        {
            bool bConvert( false );
            {
                sal_Int32 nUPD( 0 );
                sal_Int32 nBuild( 0 );
                const bool bBuildIdFound = GetImport().getBuildIds( nUPD, nBuild );
                if ( GetImport().IsTextDocInOOoFileFormat() ||
                     ( bBuildIdFound &&
                       ( nUPD== 680 || nUPD == 645 || nUPD == 641 ) ) )
                {
                    bConvert = true;
                }
            }
            if ( bConvert )
            {
                if ( nChapterInfo == ChapterFormat::NUMBER )
                {
                    nChapterInfo = ChapterFormat::DIGIT;
                }
                else if ( nChapterInfo == ChapterFormat::NAME_NUMBER )
                {
                    nChapterInfo = ChapterFormat::NO_PREFIX_SUFFIX;
                }
            }
        }
        // <--
    }
    if (bOutlineLevelOK)
        nValues++;
}

void XMLIndexChapterInfoEntryContext::FillPropertyValues(
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rValues)
{
    // entry name and (optionally) style name in parent class
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    sal_Int32 nIndex = bCharStyleNameOK ? 2 : 1;

    if( bChapterInfoOK )
    {
        // chapter info field
        rValues[nIndex].Name = rTemplateContext.sChapterFormat;
        Any aAny;
        aAny <<= nChapterInfo;
        rValues[nIndex].Value = aAny;
        nIndex++;
    }
    if( bOutlineLevelOK )
    {
        rValues[nIndex].Name = rTemplateContext.sChapterLevel;
        Any aAny;
        aAny <<= nOutlineLevel;
        rValues[nIndex].Value = aAny;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
