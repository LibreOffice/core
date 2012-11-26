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
#include "XMLFootnoteSeparatorImport.hxx"

#ifndef _RTL_USTRING
#include <rtl/ustring.hxx>
#endif
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/maptype.hxx>

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include <xmloff/PageMasterStyleMap.hxx>
#endif
#include <tools/debug.hxx>
#include <tools/color.hxx>

#include <vector>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::std::vector;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


XMLFootnoteSeparatorImport::XMLFootnoteSeparatorImport(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    vector<XMLPropertyState> & rProps,
    const UniReference<XMLPropertySetMapper> & rMapperRef,
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
    sal_Int16 eLineAdjust = text::HorizontalAdjust_LEFT; // enum text::HorizontalAdjust
    sal_Int32 nLineTextDistance = 0;
    sal_Int32 nLineDistance = 0;

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
                if (GetImport().GetMM100UnitConverter().convertMeasure(
                    nTmp, sAttrValue))
                {
                    nLineWeight = (sal_Int16)nTmp;
                }
            }
            else if (IsXMLToken( sLocalName, XML_DISTANCE_BEFORE_SEP ))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasure(
                                                        nTmp, sAttrValue))
                    nLineTextDistance = nTmp;
            }
            else if (IsXMLToken( sLocalName, XML_DISTANCE_AFTER_SEP ))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasure(
                                                        nTmp, sAttrValue))
                    nLineDistance = nTmp;
            }
            else if (IsXMLToken( sLocalName, XML_ADJUSTMENT ))
            {
                sal_uInt16 nTmpU;
                static const SvXMLEnumMapEntry aXML_HorizontalAdjust_Enum[] =
                {
                    { XML_LEFT,     text::HorizontalAdjust_LEFT },
                    { XML_CENTER,   text::HorizontalAdjust_CENTER },
                    { XML_RIGHT,    text::HorizontalAdjust_RIGHT },
                    { XML_TOKEN_INVALID, 0 }
                };

                if (SvXMLUnitConverter::convertEnum(
                            nTmpU, sAttrValue, aXML_HorizontalAdjust_Enum))
                    eLineAdjust = (sal_Int16)nTmpU;
            }
            else if (IsXMLToken( sLocalName, XML_REL_WIDTH ))
            {
                if (SvXMLUnitConverter::convertPercent(nTmp, sAttrValue))
                    nLineRelWidth = (sal_uInt8)nTmp;
            }
            else if (IsXMLToken( sLocalName, XML_COLOR ))
            {
                Color aColor;
                if (SvXMLUnitConverter::convertColor(aColor, sAttrValue))
                    nLineColor = (sal_Int32)aColor.GetColor();
            }
        }
    }

    // OK, now we have all values and can fill the XMLPropertyState vector
    Any aAny;
    sal_Int32 nIndex;

    aAny <<= eLineAdjust;
    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_ADJUST);
    XMLPropertyState aLineAdjust( nIndex, aAny);
    rProperties.push_back(aLineAdjust);

    aAny <<= nLineColor;
    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_COLOR);
    XMLPropertyState aLineColor( nIndex, aAny );
    rProperties.push_back(aLineColor);

    aAny <<= nLineDistance;
    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_DISTANCE);
    XMLPropertyState aLineDistance( nIndex, aAny );
    rProperties.push_back(aLineDistance);

    aAny <<= nLineRelWidth;
    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_WIDTH);
    XMLPropertyState aLineRelWidth( nIndex, aAny);
    rProperties.push_back(aLineRelWidth);

    aAny <<= nLineTextDistance;
    nIndex = rMapper->FindEntryIndex(CTF_PM_FTN_LINE_DISTANCE);
    XMLPropertyState aLineTextDistance( nIndex, aAny);
    rProperties.push_back(aLineTextDistance);

    DBG_ASSERT( rMapper->FindEntryIndex(CTF_PM_FTN_LINE_WEIGTH) == nPropIndex,
                "Received wrong property map index!" );
    aAny <<= nLineWeight;
    XMLPropertyState aLineWeight( nPropIndex, aAny );
    rProperties.push_back(aLineWeight);
}
