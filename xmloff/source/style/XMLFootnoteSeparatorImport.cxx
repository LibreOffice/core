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

#include <sal/log.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/text/HorizontalAdjust.hpp>


#include <sax/tools/converter.hxx>

#include <utility>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlement.hxx>

#include <PageMasterStyleMap.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::std::vector;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;


XMLFootnoteSeparatorImport::XMLFootnoteSeparatorImport(
    SvXMLImport& rImport,
    sal_Int32 /*nElement*/,
    vector<XMLPropertyState> & rProps,
    rtl::Reference<XMLPropertySetMapper> xMapperRef,
    sal_Int32 nIndex) :
        SvXMLImportContext(rImport),
        rProperties(rProps),
        rMapper(std::move(xMapperRef)),
        nPropIndex(nIndex)
{
}

XMLFootnoteSeparatorImport::~XMLFootnoteSeparatorImport()
{
}

void XMLFootnoteSeparatorImport::startFastElement(
    sal_Int32 /*nElement*/,
    const Reference<css::xml::sax::XFastAttributeList> & xAttrList)
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
    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        sal_Int32 nTmp;
        switch (aIter.getToken())
        {
            case XML_ELEMENT(STYLE, XML_WIDTH):
            {
                if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nTmp, aIter.toView()))
                {
                    nLineWeight = static_cast<sal_Int16>(nTmp);
                }
                break;
            }
            case XML_ELEMENT(STYLE, XML_DISTANCE_BEFORE_SEP):
            {
                if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                                                        nTmp, aIter.toView()))
                    nLineTextDistance = nTmp;
                break;
            }
            case XML_ELEMENT(STYLE, XML_DISTANCE_AFTER_SEP):
            {
                if (GetImport().GetMM100UnitConverter().convertMeasureToCore(
                                                        nTmp, aIter.toView()))
                    nLineDistance = nTmp;
                break;
            }
            case XML_ELEMENT(STYLE, XML_ADJUSTMENT ):
            {
                static const SvXMLEnumMapEntry<text::HorizontalAdjust> aXML_HorizontalAdjust_Enum[] =
                {
                    { XML_LEFT,     text::HorizontalAdjust_LEFT },
                    { XML_CENTER,   text::HorizontalAdjust_CENTER },
                    { XML_RIGHT,    text::HorizontalAdjust_RIGHT },
                    { XML_TOKEN_INVALID, text::HorizontalAdjust(0) }
                };

                SvXMLUnitConverter::convertEnum(
                            eLineAdjust, aIter.toView(), aXML_HorizontalAdjust_Enum);
                break;
            }
            case XML_ELEMENT(STYLE, XML_REL_WIDTH ):
            {
                if (::sax::Converter::convertPercent(nTmp, aIter.toView()))
                    nLineRelWidth = static_cast<sal_uInt8>(nTmp);
                break;
            }
            case XML_ELEMENT(STYLE, XML_COLOR):
            {
                if (::sax::Converter::convertColor(nTmp, aIter.toView()))
                {
                    nLineColor = nTmp;
                }
                break;
            }
            case XML_ELEMENT(STYLE, XML_LINE_STYLE ):
            {
                static const SvXMLEnumMapEntry<sal_Int8> aXML_LineStyle_Enum[] =
                {
                    { XML_NONE,     0 },
                    { XML_SOLID,    1 },
                    { XML_DOTTED,   2 },
                    { XML_DASH,     3 },
                    { XML_TOKEN_INVALID, 0 }
                };

                SvXMLUnitConverter::convertEnum(nLineStyle, aIter.toView(), aXML_LineStyle_Enum);
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
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
