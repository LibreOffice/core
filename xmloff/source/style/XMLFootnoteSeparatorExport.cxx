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


#include "XMLFootnoteSeparatorExport.hxx"

#include <sax/tools/converter.hxx>

#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlement.hxx>

#include <xmloff/PageMasterStyleMap.hxx>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <rtl/ustrbuf.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;
using ::std::vector;

XMLFootnoteSeparatorExport::XMLFootnoteSeparatorExport(SvXMLExport& rExp) :
    rExport(rExp)
{
}

XMLFootnoteSeparatorExport::~XMLFootnoteSeparatorExport()
{
}


void XMLFootnoteSeparatorExport::exportXML(
    const vector<XMLPropertyState> * pProperties,
    sal_uInt32 const nIdx,
    const rtl::Reference<XMLPropertySetMapper> & rMapper)
{
    assert(pProperties);

    // initialize values
    text::HorizontalAdjust eLineAdjust = text::HorizontalAdjust_LEFT;
    sal_Int32 nLineColor = 0;
    sal_Int32 nLineDistance = 0;
    sal_Int8 nLineRelWidth = 0;
    sal_Int32 nLineTextDistance = 0;
    sal_Int16 nLineWeight = 0;
    sal_Int8 nLineStyle = 0;

    // find indices into property map and get values
    sal_uInt32 nCount = pProperties->size();
    for(sal_uInt32 i = 0; i < nCount; i++)
    {
        const XMLPropertyState& rState = (*pProperties)[i];

        if( rState.mnIndex == -1 )
            continue;

        switch (rMapper->GetEntryContextId(rState.mnIndex))
        {
        case CTF_PM_FTN_LINE_ADJUST:
        {
            sal_Int16 nTmp;
            if (rState.maValue >>= nTmp)
                eLineAdjust = static_cast<text::HorizontalAdjust>(nTmp);
            break;
        }
        case CTF_PM_FTN_LINE_COLOR:
            rState.maValue >>= nLineColor;
            break;
        case CTF_PM_FTN_DISTANCE:
            rState.maValue >>= nLineDistance;
            break;
        case CTF_PM_FTN_LINE_WIDTH:
            rState.maValue >>= nLineRelWidth;
            break;
        case CTF_PM_FTN_LINE_DISTANCE:
            rState.maValue >>= nLineTextDistance;
            break;
        case CTF_PM_FTN_LINE_WEIGHT:
            (void) nIdx;
            assert(i == nIdx && "received wrong property state index");
            rState.maValue >>= nLineWeight;
            break;
        case CTF_PM_FTN_LINE_STYLE:
            rState.maValue >>= nLineStyle;
            break;
        }
    }

    OUStringBuffer sBuf;

    // weight/width
    if (nLineWeight > 0)
    {
        rExport.GetMM100UnitConverter().convertMeasureToXML(sBuf, nLineWeight);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_WIDTH,
                             sBuf.makeStringAndClear());
    }

    // line text distance
    if (nLineTextDistance > 0)
    {
        rExport.GetMM100UnitConverter().convertMeasureToXML(sBuf,
                nLineTextDistance);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_DISTANCE_BEFORE_SEP,
                             sBuf.makeStringAndClear());
    }

    // line distance
    if (nLineDistance > 0)
    {
        rExport.GetMM100UnitConverter().convertMeasureToXML(sBuf,
                nLineDistance);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_DISTANCE_AFTER_SEP,
                             sBuf.makeStringAndClear());
    }

    // line style
    static const SvXMLEnumMapEntry<sal_Int8> aXML_LineStyle_Enum[] =
    {
        { XML_NONE,     0 },
        { XML_SOLID,    1 },
        { XML_DOTTED,   2 },
        { XML_DASH,     3 },
        { XML_TOKEN_INVALID, 0 }
    };
    if (SvXMLUnitConverter::convertEnum(
            sBuf, nLineStyle, aXML_LineStyle_Enum ) )
    {
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_LINE_STYLE,
                sBuf.makeStringAndClear());
    }

    // adjustment
    static const SvXMLEnumMapEntry<text::HorizontalAdjust> aXML_HorizontalAdjust_Enum[] =
    {
        { XML_LEFT,     text::HorizontalAdjust_LEFT },
        { XML_CENTER,   text::HorizontalAdjust_CENTER },
        { XML_RIGHT,    text::HorizontalAdjust_RIGHT },
        { XML_TOKEN_INVALID, text::HorizontalAdjust(0) }
    };

    if (SvXMLUnitConverter::convertEnum(
        sBuf, eLineAdjust, aXML_HorizontalAdjust_Enum))
    {
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_ADJUSTMENT,
                             sBuf.makeStringAndClear());
    }

    // relative line width
    ::sax::Converter::convertPercent(sBuf, nLineRelWidth);
    rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                         sBuf.makeStringAndClear());

    // color
    ::sax::Converter::convertColor(sBuf, nLineColor);
    rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_COLOR,
                         sBuf.makeStringAndClear());

    // line-style

    SvXMLElementExport aElem(rExport, XML_NAMESPACE_STYLE,
                             XML_FOOTNOTE_SEP, true, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
