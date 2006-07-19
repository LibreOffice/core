/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLLineNumberingExport.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-19 16:37:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::text::XLineNumberingProperties;


XMLLineNumberingExport::XMLLineNumberingExport(SvXMLExport& rExp)
:   sCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName"))
,   sCountEmptyLines(RTL_CONSTASCII_USTRINGPARAM("CountEmptyLines"))
,   sCountLinesInFrames(RTL_CONSTASCII_USTRINGPARAM("CountLinesInFrames"))
,   sDistance(RTL_CONSTASCII_USTRINGPARAM("Distance"))
,   sInterval(RTL_CONSTASCII_USTRINGPARAM("Interval"))
,   sSeparatorText(RTL_CONSTASCII_USTRINGPARAM("SeparatorText"))
,   sNumberPosition(RTL_CONSTASCII_USTRINGPARAM("NumberPosition"))
,   sNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType"))
,   sIsOn(RTL_CONSTASCII_USTRINGPARAM("IsOn"))
,   sRestartAtEachPage(RTL_CONSTASCII_USTRINGPARAM("RestartAtEachPage"))
,   sSeparatorInterval(RTL_CONSTASCII_USTRINGPARAM("SeparatorInterval"))
,   rExport(rExp)
{
}

SvXMLEnumMapEntry __READONLY_DATA aLineNumberPositionMap[] =
{
    { XML_LEFT,     style::LineNumberPosition::LEFT },
    { XML_RIGHT,    style::LineNumberPosition::RIGHT },
    { XML_INSIDE,   style::LineNumberPosition::INSIDE },
    { XML_OUTSIDE,  style::LineNumberPosition::OUTSIDE },
    { XML_TOKEN_INVALID, 0 }
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
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                     rExport.EncodeStyleName( sTmp ));
            }

            // enable
            aAny = xLineNumbering->getPropertyValue(sIsOn);
            if (! *(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_NUMBER_LINES, XML_FALSE);
            }

            // count empty lines
            aAny = xLineNumbering->getPropertyValue(sCountEmptyLines);
            if (! *(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_COUNT_EMPTY_LINES, XML_FALSE);
            }

            // count in frames
            aAny = xLineNumbering->getPropertyValue(sCountLinesInFrames);
            if (*(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_COUNT_IN_TEXT_BOXES, XML_TRUE);
            }

            // restart numbering
            aAny = xLineNumbering->getPropertyValue(sRestartAtEachPage);
            if (*(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_RESTART_ON_PAGE, XML_TRUE);
            }

            // Distance
            aAny = xLineNumbering->getPropertyValue(sDistance);
            sal_Int32 nLength;
            aAny >>= nLength;
            if (nLength != 0)
            {
                OUStringBuffer sBuf;
                rExport.GetMM100UnitConverter().convertMeasure(sBuf, nLength);
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_OFFSET,
                                     sBuf.makeStringAndClear());
            }

            // NumeringType
            OUStringBuffer sNumPosBuf;
            aAny = xLineNumbering->getPropertyValue(sNumberingType);
            sal_Int16 nFormat = 0;
            aAny >>= nFormat;
            rExport.GetMM100UnitConverter().convertNumFormat( sNumPosBuf, nFormat );
            rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                 sNumPosBuf.makeStringAndClear());
            rExport.GetMM100UnitConverter().convertNumLetterSync( sNumPosBuf, nFormat );
            if( sNumPosBuf.getLength() )
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_NUM_LETTER_SYNC,
                                     sNumPosBuf.makeStringAndClear() );
            }

            // number position
            aAny = xLineNumbering->getPropertyValue(sNumberPosition);
            sal_Int16 nPosition = 0;
            aAny >>= nPosition;
            if (SvXMLUnitConverter::convertEnum(sNumPosBuf, nPosition,
                                                aLineNumberPositionMap))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_NUMBER_POSITION,
                                     sNumPosBuf.makeStringAndClear());
            }

            // sInterval
            aAny = xLineNumbering->getPropertyValue(sInterval);
            sal_Int16 nLineInterval = 0;
            aAny >>= nLineInterval;
            OUStringBuffer sBuf;
            SvXMLUnitConverter::convertNumber(sBuf,
                                              (sal_Int32)nLineInterval);
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_INCREMENT,
                                 sBuf.makeStringAndClear());

            SvXMLElementExport aConfigElem(rExport, XML_NAMESPACE_TEXT,
                                           XML_LINENUMBERING_CONFIGURATION,
                                           sal_True, sal_True);

            // line separator
            aAny = xLineNumbering->getPropertyValue(sSeparatorText);
            OUString sSeparator;
            aAny >>= sSeparator;
            if (sSeparator.getLength() > 0)
            {

                // SeparatorInterval
                aAny = xLineNumbering->getPropertyValue(sSeparatorInterval);
                sal_Int16 nLineDistance = 0;
                aAny >>= nLineDistance;
                SvXMLUnitConverter::convertNumber(sBuf,
                                                  (sal_Int32)nLineDistance);
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_INCREMENT,
                                     sBuf.makeStringAndClear());

                SvXMLElementExport aSeparatorElem(rExport, XML_NAMESPACE_TEXT,
                                                  XML_LINENUMBERING_SEPARATOR,
                                                  sal_True, sal_False);
                rExport.Characters(sSeparator);
            }
        }
        // else: no configuration: don't save -> default
    }
    // can't even get supplier: don't save -> default
}
