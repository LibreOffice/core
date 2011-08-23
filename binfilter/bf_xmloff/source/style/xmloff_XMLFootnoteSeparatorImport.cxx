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

#include "XMLFootnoteSeparatorImport.hxx"

#include "rtl/ustring.hxx"

#include <com/sun/star/uno/Reference.h>


#include <com/sun/star/text/HorizontalAdjust.hpp>

#include "xmlimp.hxx"


#include "xmluconv.hxx"

#include "xmlprmap.hxx"

#include "xmlnmspe.hxx"

#include "nmspmap.hxx"


#include "PageMasterStyleMap.hxx"

#include <tools/debug.hxx>


#include <vector>
namespace binfilter {


using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::std::vector;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


TYPEINIT1(XMLFootnoteSeparatorImport, SvXMLImportContext);


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


static const SvXMLEnumMapEntry aXML_HorizontalAdjust_Enum[] =
{
    { XML_LEFT,	    text::HorizontalAdjust_LEFT },
    { XML_CENTER,	text::HorizontalAdjust_CENTER },
    { XML_RIGHT,	text::HorizontalAdjust_RIGHT },
    { XML_TOKEN_INVALID, 0 }
};

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
        OUString sAttrValue = xAttrList->getValueByIndex(nAttr);

        sal_Int32 nTmp;

        if (XML_NAMESPACE_STYLE == nPrefix)
        {
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
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
