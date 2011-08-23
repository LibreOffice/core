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

#include "XMLLineNumberingExport.hxx"


#include "com/sun/star/text/XLineNumberingProperties.hpp"

#include <com/sun/star/style/LineNumberPosition.hpp>

#include "xmlexp.hxx"

#include "xmluconv.hxx"

#include "xmlnmspe.hxx"


namespace binfilter {


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

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
    { XML_LEFT,	    style::LineNumberPosition::LEFT },
    { XML_RIGHT,	style::LineNumberPosition::RIGHT },
    { XML_INSIDE,	style::LineNumberPosition::INSIDE },
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
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME, sTmp);
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
                                     XML_COUNT_IN_FLOATING_FRAMES, XML_TRUE);
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
            sal_Int16 nFormat;
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
            sal_Int16 nPosition;
            aAny >>= nPosition;
            if (SvXMLUnitConverter::convertEnum(sNumPosBuf, nPosition, 
                                                aLineNumberPositionMap))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_NUMBER_POSITION,
                                     sNumPosBuf.makeStringAndClear());
            }

            // sInterval
            aAny = xLineNumbering->getPropertyValue(sInterval);
            sal_Int16 nLineInterval;
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
                sal_Int16 nLineDistance;
                aAny >>= nLineDistance;
                OUStringBuffer sBuf;
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
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
