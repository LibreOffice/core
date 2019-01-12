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


#include "XMLIndexChapterInfoEntryContext.hxx"

#include <com/sun/star/text/ChapterFormat.hpp>

#include <sax/tools/converter.hxx>

#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>


using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;


XMLIndexChapterInfoEntryContext::XMLIndexChapterInfoEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    bool bT ) :
        XMLIndexSimpleEntryContext(rImport,
                                   (bT ? OUString("TokenEntryNumber")
                                       : OUString("TokenChapterInfo")),
                                   rTemplate, nPrfx, rLocalName),
        nChapterInfo(ChapterFormat::NAME_NUMBER),
        bChapterInfoOK(false),
        bTOC( bT ),
        nOutlineLevel( 0 ),
        bOutlineLevelOK(false)
{
}

XMLIndexChapterInfoEntryContext::~XMLIndexChapterInfoEntryContext()
{
}

static const SvXMLEnumMapEntry<sal_uInt16> aChapterDisplayMap[] =
{
    { XML_NAME,                     ChapterFormat::NAME },
    { XML_NUMBER,                   ChapterFormat::NUMBER },
    { XML_NUMBER_AND_NAME,          ChapterFormat::NAME_NUMBER },
    //---> i89791
    // enabled for ODF 1.2, full index support in 3.0
    { XML_PLAIN_NUMBER_AND_NAME,    ChapterFormat::NO_PREFIX_SUFFIX },
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
                bCharStyleNameOK = true;
            }
            else if ( IsXMLToken( sLocalName, XML_DISPLAY ) )//i53420, always true, in TOC as well
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(
                    nTmp, xAttrList->getValueByIndex(nAttr),
                    aChapterDisplayMap))
                {
                    nChapterInfo = nTmp;
                    bChapterInfoOK = true;
                }
            }
            else if ( IsXMLToken( sLocalName, XML_OUTLINE_LEVEL ) )
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertNumber(nTmp,
                        xAttrList->getValueByIndex(nAttr)))
                {
//control on range is carried out in the UNO level
                    nOutlineLevel = static_cast<sal_uInt16>(nTmp);
                    bOutlineLevelOK = true;
                }
            }
        }
    }

    // if we have a style name, set it!
    if (bCharStyleNameOK)
    {
        nValues++;
    }

    // if we have chapter info, set it!
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
    }
    if (bOutlineLevelOK)
        nValues++;
}

void XMLIndexChapterInfoEntryContext::FillPropertyValues(
    css::uno::Sequence<css::beans::PropertyValue> & rValues)
{
    // entry name and (optionally) style name in parent class
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    sal_Int32 nIndex = bCharStyleNameOK ? 2 : 1;

    if( bChapterInfoOK )
    {
        // chapter info field
        rValues[nIndex].Name = "ChapterFormat";
        rValues[nIndex].Value <<= nChapterInfo;
        nIndex++;
    }
    if( bOutlineLevelOK )
    {
        rValues[nIndex].Name = "ChapterLevel";
        rValues[nIndex].Value <<= nOutlineLevel;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
