/*************************************************************************
 *
 *  $RCSfile: XMLFootnoteSeparatorImport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-01 14:09:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLFOOTNOTESEPARATORIMPORT_HXX
#include "XMLFootnoteSeparatorImport.hxx"
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#include <vector>


using namespace ::com::sun::star;

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
    sal_Int32 nIndex) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rProperties(rProps),
        nPropIndex(nIndex)
{
}

XMLFootnoteSeparatorImport::~XMLFootnoteSeparatorImport()
{
}


static const SvXMLEnumMapEntry aXML_HorizontalAdjust_Enum[] =
{
    { sXML_left,    text::HorizontalAdjust_LEFT },
    { sXML_center,  text::HorizontalAdjust_CENTER },
    { sXML_right,   text::HorizontalAdjust_RIGHT },
    { 0, 0 }
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
            if (sLocalName.equalsAsciiL(sXML_width, sizeof(sXML_width)-1))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasure(
                    nTmp, sAttrValue))
                {
                    nLineWeight = (sal_Int16)nTmp;
                }
            }
            else if (sLocalName.equalsAsciiL(sXML_distance_before_sep,
                                           sizeof(sXML_distance_before_sep)-1))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasure(
                                                        nTmp, sAttrValue))
                    nLineTextDistance = nTmp;
            }
            else if (sLocalName.equalsAsciiL(sXML_distance_after_sep,
                                           sizeof(sXML_distance_after_sep)-1))
            {
                if (GetImport().GetMM100UnitConverter().convertMeasure(
                                                        nTmp, sAttrValue))
                    nLineDistance = nTmp;
            }
            else if (sLocalName.equalsAsciiL(sXML_adjustment,
                                             sizeof(sXML_adjustment)-1))
            {
                sal_uInt16 nTmpU;
                if (SvXMLUnitConverter::convertEnum(
                            nTmpU, sAttrValue, aXML_HorizontalAdjust_Enum))
                    eLineAdjust = (sal_Int16)nTmpU;
            }
            else if (sLocalName.equalsAsciiL(sXML_rel_width,
                                             sizeof(sXML_rel_width)-1))
            {
                if (SvXMLUnitConverter::convertPercent(nTmp, sAttrValue))
                    nLineRelWidth = (sal_uInt8)nTmp;
            }
            else if (sLocalName.equalsAsciiL(sXML_color, sizeof(sXML_color)-1))
            {
                Color aColor;
                if (SvXMLUnitConverter::convertColor(aColor, sAttrValue))
                    nLineColor = (sal_Int32)aColor.GetColor();
            }
        }
    }

    // OK, now we have all values and can fill the XMLPropertyState vector
    Any aAny;

    aAny <<= eLineAdjust;
    XMLPropertyState aLineAdjust( nPropIndex-5, aAny);
    rProperties.push_back(aLineAdjust);

    aAny <<= nLineColor;
    XMLPropertyState aLineColor( nPropIndex-4, aAny );
    rProperties.push_back(aLineColor);

    aAny <<= nLineDistance;
    XMLPropertyState aLineDistance( nPropIndex-3, aAny );
    rProperties.push_back(aLineDistance);

    aAny <<= nLineRelWidth;
    XMLPropertyState aLineRelWidth( nPropIndex-2, aAny);
    rProperties.push_back(aLineRelWidth);

    aAny <<= nLineTextDistance;
    XMLPropertyState aLineTextDistance( nPropIndex-1, aAny);
    rProperties.push_back(aLineTextDistance);

    aAny <<= nLineWeight;
    XMLPropertyState aLineWeight( nPropIndex, aAny );
    rProperties.push_back(aLineWeight);
}
