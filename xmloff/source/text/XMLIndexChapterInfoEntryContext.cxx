/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        // --> OD 2008-06-26 #i89791#
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
