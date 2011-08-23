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

#ifndef _XMLOFF_XMLLINENUMBERINGIMPORTCONTEXT_HXX_
#include "XMLLineNumberingImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLLINENUMBERINGSEPARATORIMPORTCONTEXT_HXX_
#include "XMLLineNumberingSeparatorImportContext.hxx"
#endif


#ifndef _COM_SUN_STAR_TEXT_XLINENUMBERINGPROPERTIES_HPP_
#include "com/sun/star/text/XLineNumberingProperties.hpp"
#endif

#ifndef _COM_SUN_STAR_STYLE_LINENUMBERPOSITION_HPP_
#include <com/sun/star/style/LineNumberPosition.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif


#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif
namespace binfilter {


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::binfilter::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::text::XLineNumberingProperties;
using ::rtl::OUString;

TYPEINIT1( XMLLineNumberingImportContext, SvXMLStyleContext );


XMLLineNumberingImportContext::XMLLineNumberingImportContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList) :
        SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList, XML_STYLE_FAMILY_TEXT_LINENUMBERINGCONFIG),
        sStyleName(),
        sNumFormat(GetXMLToken(XML_1)),
        sNumLetterSync(GetXMLToken(XML_FALSE)),
        sSeparator(),
        nOffset(-1),
        nNumberPosition(style::LineNumberPosition::LEFT),
        nIncrement(-1),
        nSeparatorIncrement(-1),
        bNumberLines(sal_True),
        bCountEmptyLines(sal_True),
        bCountInFloatingFrames(sal_False),
        bRestartNumbering(sal_False),
        sCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")),
        sCountEmptyLines(RTL_CONSTASCII_USTRINGPARAM("CountEmptyLines")),
        sCountLinesInFrames(RTL_CONSTASCII_USTRINGPARAM("CountLinesInFrames")),
        sDistance(RTL_CONSTASCII_USTRINGPARAM("Distance")),
        sInterval(RTL_CONSTASCII_USTRINGPARAM("Interval")),
        sSeparatorText(RTL_CONSTASCII_USTRINGPARAM("SeparatorText")),
        sNumberPosition(RTL_CONSTASCII_USTRINGPARAM("NumberPosition")),
        sNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType")),
        sIsOn(RTL_CONSTASCII_USTRINGPARAM("IsOn")),
        sRestartAtEachPage(RTL_CONSTASCII_USTRINGPARAM("RestartAtEachPage")),
        sSeparatorInterval(RTL_CONSTASCII_USTRINGPARAM("SeparatorInterval"))
{
}

XMLLineNumberingImportContext::~XMLLineNumberingImportContext()
{
}



static const SvXMLEnumMapEntry aLineNumberPositionMap[] =
{
    { XML_LEFT,	    style::LineNumberPosition::LEFT },
    { XML_RIGHT,	style::LineNumberPosition::RIGHT },
    { XML_INSIDE,	style::LineNumberPosition::INSIDE },
    { XML_OUTSIDE,  style::LineNumberPosition::OUTSIDE },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLTokenMapEntry aLineNumberingTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_LINENUMBERING_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_NUMBER_LINES, 
          XML_TOK_LINENUMBERING_NUMBER_LINES },
    { XML_NAMESPACE_TEXT, XML_COUNT_EMPTY_LINES, 
          XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES },
    { XML_NAMESPACE_TEXT, XML_COUNT_IN_FLOATING_FRAMES, 
          XML_TOK_LINENUMBERING_COUNT_IN_FLOATING_FRAMES },
    { XML_NAMESPACE_TEXT, XML_RESTART_ON_PAGE, 
          XML_TOK_LINENUMBERING_RESTART_NUMBERING },
    { XML_NAMESPACE_TEXT, XML_OFFSET, XML_TOK_LINENUMBERING_OFFSET },
    { XML_NAMESPACE_STYLE, XML_NUM_FORMAT, XML_TOK_LINENUMBERING_NUM_FORMAT },
    { XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC, 
          XML_TOK_LINENUMBERING_NUM_LETTER_SYNC },
    { XML_NAMESPACE_TEXT, XML_NUMBER_POSITION, 
          XML_TOK_LINENUMBERING_NUMBER_POSITION },
    { XML_NAMESPACE_TEXT, XML_INCREMENT, XML_TOK_LINENUMBERING_INCREMENT },
//	{ XML_NAMESPACE_TEXT, XML_LINENUMBERING_CONFIGURATION, 
//		  XML_TOK_LINENUMBERING_LINENUMBERING_CONFIGURATION },
//	{ XML_NAMESPACE_TEXT, XML_INCREMENT, XML_TOK_LINENUMBERING_INCREMENT },
//	{ XML_NAMESPACE_TEXT, XML_LINENUMBERING_SEPARATOR, 
//		  XML_TOK_LINENUMBERING_LINENUMBERING_SEPARATOR },

    XML_TOKEN_MAP_END
};

void XMLLineNumberingImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLTokenMap aTokenMap(aLineNumberingTokenMap);

    // process attributes
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++) 
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        ProcessAttribute(
            (enum LineNumberingToken)aTokenMap.Get(nPrefix, sLocalName),
            xAttrList->getValueByIndex(i));
    }
}

void XMLLineNumberingImportContext::ProcessAttribute(
    enum LineNumberingToken eToken,
    OUString sValue)
{
    sal_Bool bTmp;
    sal_Int32 nTmp;

    switch (eToken)
    {
        case XML_TOK_LINENUMBERING_STYLE_NAME:
            sStyleName = sValue;
            break;

        case XML_TOK_LINENUMBERING_NUMBER_LINES:
            if (SvXMLUnitConverter::convertBool(bTmp, sValue))
            {
                bNumberLines = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES:
            if (SvXMLUnitConverter::convertBool(bTmp, sValue))
            {
                bCountEmptyLines = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_COUNT_IN_FLOATING_FRAMES:
            if (SvXMLUnitConverter::convertBool(bTmp, sValue))
            {
                bCountInFloatingFrames = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_RESTART_NUMBERING:
            if (SvXMLUnitConverter::convertBool(bTmp, sValue))
            {
                bRestartNumbering = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_OFFSET:
            if (GetImport().GetMM100UnitConverter().
                convertMeasure(nTmp, sValue))
            {
                nOffset = nTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_NUM_FORMAT:
            sNumFormat = sValue;
            break;

        case XML_TOK_LINENUMBERING_NUM_LETTER_SYNC:
            sNumLetterSync = sValue;
            break;

        case XML_TOK_LINENUMBERING_NUMBER_POSITION:
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sValue, 
                                                aLineNumberPositionMap))
            {
                nNumberPosition = nTmp;
            }
            break;
        }

        case XML_TOK_LINENUMBERING_INCREMENT:
            if (SvXMLUnitConverter::convertNumber(nTmp, sValue, 0))
            {
                nIncrement = (sal_Int16)nTmp;
            }
            break;
    }
}

void XMLLineNumberingImportContext::CreateAndInsert(
    sal_Bool bOverwrite)
{
    // insert and block mode is handled in insertStyleFamily

    // we'll try to get the LineNumberingProperties
    Reference<XLineNumberingProperties> xSupplier(GetImport().GetModel(), 
                                                  UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XPropertySet> xLineNumbering = 
            xSupplier->getLineNumberingProperties();

        if (xLineNumbering.is())
        {
            Any aAny;

            // set style name (if it exists)
            if ( GetImport().GetStyles()->FindStyleChildContext( 
                            XML_STYLE_FAMILY_TEXT_TEXT, sStyleName ) != NULL )
            {
                aAny <<= sStyleName;
                xLineNumbering->setPropertyValue(sCharStyleName, aAny);
            }

            aAny <<= sSeparator;
            xLineNumbering->setPropertyValue(sSeparatorText, aAny);

            aAny <<= nOffset;
            xLineNumbering->setPropertyValue(sDistance, aAny);

            aAny <<= nNumberPosition;
            xLineNumbering->setPropertyValue(sNumberPosition, aAny);

            if (nIncrement >= 0)
            {
                aAny <<= nIncrement;
                xLineNumbering->setPropertyValue(sInterval, aAny);
            }

            if (nSeparatorIncrement >= 0)
            {
                aAny <<= nSeparatorIncrement;
                xLineNumbering->setPropertyValue(sSeparatorInterval, aAny);
            }

            aAny.setValue(&bNumberLines, ::getBooleanCppuType());
            xLineNumbering->setPropertyValue(sIsOn, aAny);

            aAny.setValue(&bCountEmptyLines, ::getBooleanCppuType());
            xLineNumbering->setPropertyValue(sCountEmptyLines, aAny);

            aAny.setValue(&bCountInFloatingFrames, ::getBooleanCppuType());
            xLineNumbering->setPropertyValue(sCountLinesInFrames, aAny);

            aAny.setValue(&bRestartNumbering, ::getBooleanCppuType());
            xLineNumbering->setPropertyValue(sRestartAtEachPage, aAny);

            sal_Int16 nNumType = NumberingType::ARABIC;
            GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumFormat,
                                                    sNumLetterSync );
            aAny <<= nNumType;
            xLineNumbering->setPropertyValue(sNumberingType, aAny);
        }
    }
}

SvXMLImportContext* XMLLineNumberingImportContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (nPrefix == XML_NAMESPACE_TEXT) &&
         IsXMLToken(rLocalName, XML_LINENUMBERING_SEPARATOR) )
    {
        return new XMLLineNumberingSeparatorImportContext(GetImport(), 
                                                          nPrefix, rLocalName,
                                                          *this);
    }
    else
    {
        // unknown element: default context
        return SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, 
                                                      xAttrList);
    }
}

void XMLLineNumberingImportContext::SetSeparatorText(
    const OUString& sText)
{
    sSeparator = sText;
}

void XMLLineNumberingImportContext::SetSeparatorIncrement(
    sal_Int16 nIncr)
{
    nSeparatorIncrement = nIncr;
}

}//end of namespace binfilter
