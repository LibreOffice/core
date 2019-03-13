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


#include "XMLFootnoteSeparatorImport.hxx"

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>


#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlement.hxx>

#include <xmloff/PageMasterStyleMap.hxx>

#include <vector>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::std::vector;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


XMLFootnoteSeparatorImport::XMLFootnoteSeparatorImport(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    vector<XMLPropertyState> & rProps,
    const rtl::Reference<XMLPropertySetMapper> & rMapperRef,
    sal_Int32 nIndex) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rProperties(rProps),
        rMapper(rMapperRef),
        nPropIndex(nIndex)
{
}

XMLFootnoteSeparatorImport::~XMLFootnoteSeparatorImport()
{
}

void XMLFootnoteSeparatorImport::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // get the values from the properties
    sal_Int16 nLineWeight = 0;
    sal_Int32 nLineColor = 0;
    sal_Int8 nLineRelWidth = 0;
    text::HorizontalAdjust eLineAdjust = text::HorizontalAdjust_LEFT;
    sal_Int32 nLineTextDistance = 0;
    sal_Int32 nLineDistance = 0;

    // Default separator line style should be SOLID (used to be default before
    // the choice selector was available)
    sal_Int8 nLineStyle = 1;

    // iterate over xattribute list and fill values
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        if (XML_NAMESPACE_STYLE == nPrefix)
        {
            OUString sAttrValue = xAttrList->getValueByIndex(nAttr);
            sal_Int32 nTmp;
            if (IsXMLToken( sLocalName, XML_WIDTH ))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nTmp, sAttrValue))
                {
                    nLineWeight = static_cast<sal_Int16>(nTmp);
                }
            }
            else if (IsXMLToken( sLocalName, XML_DISTANCE_BEFORE_SEP ))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                                                        nTmp, sAttrValue))
                    nLineTextDistance = nTmp;
            }
            else if (IsXMLToken( sLocalName, XML_DISTANCE_AFTER_SEP ))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                                                        nTmp, sAttrValue))
                    nLineDistance = nTmp;
            }
            else if (IsXMLToken( sLocalName, XML_ADJUSTMENT ))
            {
                static const SvXMLEnumMapEntry<text::HorizontalAdjust> aXML_HorizontalAdjust_Enum[] =
                {
                    { XML_LEFT,     text::HorizontalAdjust_LEFT },
                    { XML_CENTER,   text::HorizontalAdjust_CENTER },
                    { XML_RIGHT,    text::HorizontalAdjust_RIGHT },
                    { XML_TOKEN_INVALID, text::HorizontalAdjust(0) }
                };

                SvXMLUnitConverter::convertEnum(
                            eLineAdjust, sAttrValue, aXML_HorizontalAdjust_Enum);
            }
            else if (IsXMLToken( sLocalName, XML_REL_WIDTH ))
            {
                if (::sax::Converter::convertPercent(nTmp, sAttrValue))
                    nLineRelWidth = static_cast<sal_uInt8>(nTmp);
            }
            else if (IsXMLToken( sLocalName, XML_COLOR ))
            {
                if (::sax::Converter::convertColor(nTmp, sAttrValue))
                {
                    nLineColor = nTmp;
                }
            }
            else if (IsXMLToken( sLocalName, XML_LINE_STYLE ))
            {
                static const SvXMLEnumMapEntry<sal_Int8> aXML_LineStyle_Enum[] =
                {
                    { XML_NONE,     0 },
                    { XML_SOLID,    1 },
                    { XML_DOTTED,   2 },
                    { XML_DASH,     3 },
                    { XML_TOKEN_INVALID, 0 }
                };

                SvXMLUnitConverter::convertEnum(nLineStyle, sAttrValue, aXML_LineStyle_Enum);
            }
        }
    }

    // OK, now we have all values and can fill the XMLPropertyState vector
    sal_Int32 nIndex;

    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_ADJUST);
    XMLPropertyState aLineAdjust( nIndex, uno::Any(sal_Int16(eLineAdjust)) );
    rProperties.push_back(aLineAdjust);

    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_COLOR);
    XMLPropertyState aLineColor( nIndex, uno::Any(nLineColor) );
    rProperties.push_back(aLineColor);

    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_STYLE);
    XMLPropertyState aLineStyle( nIndex, uno::Any(nLineStyle) );
    rProperties.push_back(aLineStyle);

    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_DISTANCE);
    XMLPropertyState aLineDistance( nIndex, uno::Any(nLineDistance) );
    rProperties.push_back(aLineDistance);

    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_WIDTH);
    XMLPropertyState aLineRelWidth( nIndex, uno::Any(nLineRelWidth));
    rProperties.push_back(aLineRelWidth);

    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_DISTANCE);
    XMLPropertyState aLineTextDistance( nIndex, uno::Any(nLineTextDistance));
    rProperties.push_back(aLineTextDistance);

    SAL_WARN_IF( rMapper->FindEntryIndex(CTF_PM_FTN_LINE_WEIGHT) != nPropIndex, "xmloff",
                "Received wrong property map index!" );
    XMLPropertyState aLineWeight( nPropIndex, uno::Any(nLineWeight) );
    rProperties.push_back(aLineWeight);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
