/*************************************************************************
 *
 *  $RCSfile: XMLLineNumberingExport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-19 09:41:43 $
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

#ifndef _XMLOFF_XMLLINENUMBERINGEXPORT_HXX_
#include "XMLLineNumberingExport.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include "com/sun/star/beans/XPropertySet.hpp"
#endif

#ifndef _COM_SUN_STAR_TEXT_XLINENUMBERINGPROPERTIES_HPP_
#include "com/sun/star/text/XLineNumberingProperties.hpp"
#endif

#ifndef _COM_SUN_STAR_STYLE_LINENUMBERPOSITION_HPP_
#include <com/sun/star/style/LineNumberPosition.hpp>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::text::XLineNumberingProperties;

XMLLineNumberingExport::XMLLineNumberingExport(SvXMLExport& rExp) :
    rExport(rExp),
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

SvXMLEnumMapEntry __READONLY_DATA aLineNumberPositionMap[] =
{
    { sXML_left,    style::LineNumberPosition::LEFT },
    { sXML_right,   style::LineNumberPosition::RIGHT },
    { sXML_inside,  style::LineNumberPosition::INSIDE },
    { sXML_outside, style::LineNumberPosition::OUTSIDE },
    { NULL, 0 }
};



void XMLLineNumberingExport::Export()
{
    // export element if we have line numbering info
    Reference<XLineNumberingProperties> xSupplier(rExport.GetModel(),
                                                  UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XPropertySet> xLineNumbering =
            xSupplier->getLineNumberingProperties();

        if (xLineNumbering.is())
        {
            Any aAny;

            // char style
            aAny = xLineNumbering->getPropertyValue(sCharStyleName);
            OUString sTmp;
            aAny >>= sTmp;
            if (sTmp.getLength() > 0)
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_style_name,sTmp);
            }

            // enable
            aAny = xLineNumbering->getPropertyValue(sIsOn);
            if (! *(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_number_lines, sXML_false);
            }

            // count empty lines
            aAny = xLineNumbering->getPropertyValue(sCountEmptyLines);
            if (! *(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_count_empty_lines, sXML_false);
            }

            // count in frames
            aAny = xLineNumbering->getPropertyValue(sCountLinesInFrames);
            if (*(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_count_in_floating_frames,
                                          sXML_true);
            }

            // restart numbering
            aAny = xLineNumbering->getPropertyValue(sRestartAtEachPage);
            if (*(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_restart_on_page, sXML_true);
            }

            // Distance
            aAny = xLineNumbering->getPropertyValue(sDistance);
            sal_Int32 nLength;
            aAny >>= nLength;
            if (nLength != 0)
            {
                OUStringBuffer sBuf;
                rExport.GetMM100UnitConverter().convertMeasure(sBuf, nLength);
                rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_offset,
                                     sBuf.makeStringAndClear());
            }

            // NumeringType
            OUStringBuffer sNumPosBuf;
            aAny = xLineNumbering->getPropertyValue(sNumberingType);
            sal_Int16 nFormat;
            aAny >>= nFormat;
            rExport.GetMM100UnitConverter().convertNumFormat( sNumPosBuf, nFormat );
            rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_num_format,
                                 sNumPosBuf.makeStringAndClear());
            rExport.GetMM100UnitConverter().convertNumLetterSync( sNumPosBuf, nFormat );
            if( sNumPosBuf.getLength() )
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE,
                                     sXML_num_letter_sync,
                                     sNumPosBuf.makeStringAndClear() );
            }

            // number position
            aAny = xLineNumbering->getPropertyValue(sNumberPosition);
            sal_Int16 nPosition;
            aAny >>= nPosition;
            if (SvXMLUnitConverter::convertEnum(sNumPosBuf, nPosition,
                                                aLineNumberPositionMap))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_number_position,
                                     sNumPosBuf.makeStringAndClear());
            }

            // sInterval
            aAny = xLineNumbering->getPropertyValue(sInterval);
            sal_Int16 nLineInterval;
            aAny >>= nLineInterval;
            OUStringBuffer sBuf;
            SvXMLUnitConverter::convertNumber(sBuf,
                                              (sal_Int32)nLineInterval);
            rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_increment,
                                 sBuf.makeStringAndClear());

            SvXMLElementExport aConfigElem(rExport, XML_NAMESPACE_TEXT,
                                           sXML_linenumbering_configuration,
                                           sal_True, sal_True);

            // line separator
            aAny = xLineNumbering->getPropertyValue(sSeparatorText);
            OUString sSeparator;
            aAny >>= sSeparator;
            if (sSeparator.getLength() > 0)
            {

                // SeparatorInterval
                aAny = xLineNumbering->getPropertyValue(sSeparatorInterval);
                sal_Int16 nLineDistance;
                aAny >>= nLineDistance;
                OUStringBuffer sBuf;
                SvXMLUnitConverter::convertNumber(sBuf,
                                                  (sal_Int32)nLineDistance);
                rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_increment,
                                     sBuf.makeStringAndClear());

                SvXMLElementExport aSeparatorElem(rExport, XML_NAMESPACE_TEXT,
                                                  sXML_linenumbering_separator,
                                                  sal_True, sal_False);
                rExport.GetDocHandler()->characters(sSeparator);
            }
        }
        // else: no configuration: don't save -> default
    }
    // can't even get supplier: don't save -> default
}
