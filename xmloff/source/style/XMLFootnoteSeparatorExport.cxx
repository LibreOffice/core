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
#include "XMLFootnoteSeparatorExport.hxx"
#include <tools/debug.hxx>
#include <xmloff/xmlexp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlprmap.hxx>

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include <xmloff/PageMasterStyleMap.hxx>
#endif
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <rtl/ustrbuf.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;
using ::rtl::OUStringBuffer;
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
    sal_uInt32
    #ifdef DBG_UTIL
    nIdx
    #endif
    ,
    const UniReference<XMLPropertySetMapper> & rMapper)
{
    DBG_ASSERT(NULL != pProperties, "Need property states");

    // intialize values
    sal_Int16 eLineAdjust = text::HorizontalAdjust_LEFT;
    sal_Int32 nLineColor = 0;
    sal_Int32 nLineDistance = 0;
    sal_Int8 nLineRelWidth = 0;
    sal_Int32 nLineTextDistance = 0;
    sal_Int16 nLineWeight = 0;

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
            rState.maValue >>= eLineAdjust;
            break;
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
        case CTF_PM_FTN_LINE_WEIGTH:
            DBG_ASSERT( i == nIdx,
                        "received wrong property state index" );
            rState.maValue >>= nLineWeight;
            break;
        }
    }

    OUStringBuffer sBuf;

    // weight/width
    if (nLineWeight > 0)
    {
        rExport.GetMM100UnitConverter().convertMeasure(sBuf, nLineWeight);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_WIDTH,
                             sBuf.makeStringAndClear());
    }

    // line text distance
    if (nLineTextDistance > 0)
    {
        rExport.GetMM100UnitConverter().convertMeasure(sBuf,nLineTextDistance);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_DISTANCE_BEFORE_SEP,
                             sBuf.makeStringAndClear());
    }

    // line distance
    if (nLineDistance > 0)
    {
        rExport.GetMM100UnitConverter().convertMeasure(sBuf, nLineDistance);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_DISTANCE_AFTER_SEP,
                             sBuf.makeStringAndClear());
    }

    // adjustment
    static const SvXMLEnumMapEntry aXML_HorizontalAdjust_Enum[] =
    {
        { XML_LEFT,     text::HorizontalAdjust_LEFT },
        { XML_CENTER,   text::HorizontalAdjust_CENTER },
        { XML_RIGHT,    text::HorizontalAdjust_RIGHT },
        { XML_TOKEN_INVALID, 0 }
    };

    if (rExport.GetMM100UnitConverter().convertEnum(
        sBuf, eLineAdjust, aXML_HorizontalAdjust_Enum))
    {
        rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_ADJUSTMENT,
                             sBuf.makeStringAndClear());
    }

    // relative line width
    SvXMLUnitConverter::convertPercent(sBuf, nLineRelWidth);
    rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                         sBuf.makeStringAndClear());

    // color
    rExport.GetMM100UnitConverter().convertColor(sBuf, nLineColor);
    rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_COLOR,
                         sBuf.makeStringAndClear());

    SvXMLElementExport aElem(rExport, XML_NAMESPACE_STYLE,
                             XML_FOOTNOTE_SEP, sal_True, sal_True);
}
