/*************************************************************************
 *
 *  $RCSfile: XMLFootnoteSeparatorExport.cxx,v $
 *
 *  $Revision: 1.3 $
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

#ifndef _XMLOFF_XMLFOOTNOTESEPARATOREXPORT_HXX
#include "XMLFootnoteSeparatorExport.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif


using namespace ::com::sun::star;
using ::rtl::OUStringBuffer;
using ::std::vector;

XMLFootnoteSeparatorExport::XMLFootnoteSeparatorExport(SvXMLExport& rExp) :
    rExport(rExp)
{
}

XMLFootnoteSeparatorExport::~XMLFootnoteSeparatorExport()
{
}


static const SvXMLEnumMapEntry aXML_HorizontalAdjust_Enum[] =
{
    { sXML_left,    text::HorizontalAdjust_LEFT },
    { sXML_center,  text::HorizontalAdjust_CENTER },
    { sXML_right,   text::HorizontalAdjust_RIGHT },
    { 0, 0 }
};

void XMLFootnoteSeparatorExport::exportXML(
    const vector<XMLPropertyState> * pProperties,
    sal_uInt32 nIdx,
    const UniReference<XMLPropertySetMapper> & rMapper)
{
    DBG_ASSERT(NULL != pProperties, "Need property states");
    DBG_ASSERT(nIdx >= 6, "index too low");

    if (nIdx >= 6)
    {
        // get the values from the properties
        // enum text::HorizontalAdjust
        sal_Int16 eLineAdjust = text::HorizontalAdjust_LEFT;
        sal_Int32 nLineColor = 0;
        sal_Int32 nLineDistance = 0;
        sal_Int8 nLineRelWidth = 0;
        sal_Int32 nLineTextDistance = 0;
        sal_Int16 nLineWeight = 0;

        (*pProperties)[nIdx-5].maValue >>= eLineAdjust;
        (*pProperties)[nIdx-4].maValue >>= nLineColor;
        (*pProperties)[nIdx-3].maValue >>= nLineDistance;
        (*pProperties)[nIdx-2].maValue >>= nLineRelWidth;
        (*pProperties)[nIdx-1].maValue >>= nLineTextDistance;
        (*pProperties)[nIdx  ].maValue >>= nLineWeight;

        OUStringBuffer sBuf;

        // weight/width
        if (nLineWeight > 0)
        {
            rExport.GetMM100UnitConverter().convertMeasure(sBuf, nLineWeight);
            rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_width,
                                 sBuf.makeStringAndClear());
        }

        // line text distance
        if (nLineTextDistance > 0)
        {
            rExport.GetMM100UnitConverter().convertMeasure(sBuf,
                                                           nLineTextDistance);
            rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_distance_before_sep,
                                 sBuf.makeStringAndClear());
        }

        // line distance
        if (nLineDistance > 0)
        {
            rExport.GetMM100UnitConverter().convertMeasure(sBuf,
                                                           nLineDistance);
            rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_distance_after_sep,
                                 sBuf.makeStringAndClear());
        }

        // adjustment
        if (rExport.GetMM100UnitConverter().convertEnum(
            sBuf, eLineAdjust, aXML_HorizontalAdjust_Enum))
        {
            rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_adjustment,
                                 sBuf.makeStringAndClear());
        }


        SvXMLUnitConverter::convertPercent(sBuf, nLineRelWidth);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_rel_width,
                             sBuf.makeStringAndClear());

        // color
        rExport.GetMM100UnitConverter().convertColor(sBuf, nLineColor);
        rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_color,
                             sBuf.makeStringAndClear());

        SvXMLElementExport aElem(rExport, XML_NAMESPACE_STYLE,
                                 sXML_footnote_sep, sal_True, sal_True);
    }
    // else: ignore (illegal map)
}
