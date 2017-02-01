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

#include "XMLLineNumberingExport.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <o3tl/any.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnume.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::text::XLineNumberingProperties;


XMLLineNumberingExport::XMLLineNumberingExport(SvXMLExport& rExp)
: rExport(rExp)
{
}

SvXMLEnumMapEntry const aLineNumberPositionMap[] =
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
            aAny = xLineNumbering->getPropertyValue("CharStyleName");
            OUString sTmp;
            aAny >>= sTmp;
            if (!sTmp.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                     rExport.EncodeStyleName( sTmp ));
            }

            // enable
            aAny = xLineNumbering->getPropertyValue("IsOn");
            if (! *o3tl::doAccess<bool>(aAny))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_NUMBER_LINES, XML_FALSE);
            }

            // count empty lines
            aAny = xLineNumbering->getPropertyValue("CountEmptyLines");
            if (! *o3tl::doAccess<bool>(aAny))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_COUNT_EMPTY_LINES, XML_FALSE);
            }

            // count in frames
            aAny = xLineNumbering->getPropertyValue("CountLinesInFrames");
            if (*o3tl::doAccess<bool>(aAny))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_COUNT_IN_TEXT_BOXES, XML_TRUE);
            }

            // restart numbering
            aAny = xLineNumbering->getPropertyValue("RestartAtEachPage");
            if (*o3tl::doAccess<bool>(aAny))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_RESTART_ON_PAGE, XML_TRUE);
            }

            // Distance
            aAny = xLineNumbering->getPropertyValue("Distance");
            sal_Int32 nLength = 0;
            aAny >>= nLength;
            if (nLength != 0)
            {
                OUStringBuffer sBuf;
                rExport.GetMM100UnitConverter().convertMeasureToXML(
                        sBuf, nLength);
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_OFFSET,
                                     sBuf.makeStringAndClear());
            }

            // NumeringType
            OUStringBuffer sNumPosBuf;
            aAny = xLineNumbering->getPropertyValue("NumberingType");
            sal_Int16 nFormat = 0;
            aAny >>= nFormat;
            rExport.GetMM100UnitConverter().convertNumFormat( sNumPosBuf, nFormat );
            rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                 sNumPosBuf.makeStringAndClear());
            SvXMLUnitConverter::convertNumLetterSync( sNumPosBuf, nFormat );
            if( !sNumPosBuf.isEmpty() )
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_NUM_LETTER_SYNC,
                                     sNumPosBuf.makeStringAndClear() );
            }

            // number position
            aAny = xLineNumbering->getPropertyValue("NumberPosition");
            sal_Int16 nPosition = 0;
            aAny >>= nPosition;
            if (SvXMLUnitConverter::convertEnum(sNumPosBuf, nPosition,
                                                aLineNumberPositionMap))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_NUMBER_POSITION,
                                     sNumPosBuf.makeStringAndClear());
            }

            // sInterval
            aAny = xLineNumbering->getPropertyValue("Interval");
            sal_Int16 nLineInterval = 0;
            aAny >>= nLineInterval;
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_INCREMENT,
                                 OUString::number(nLineInterval));

            SvXMLElementExport aConfigElem(rExport, XML_NAMESPACE_TEXT,
                                           XML_LINENUMBERING_CONFIGURATION,
                                           true, true);

            // line separator
            aAny = xLineNumbering->getPropertyValue("SeparatorText");
            OUString sSeparator;
            aAny >>= sSeparator;
            if (!sSeparator.isEmpty())
            {

                // SeparatorInterval
                aAny = xLineNumbering->getPropertyValue("SeparatorInterval");
                sal_Int16 nLineDistance = 0;
                aAny >>= nLineDistance;
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_INCREMENT,
                                     OUString::number(nLineDistance));

                SvXMLElementExport aSeparatorElem(rExport, XML_NAMESPACE_TEXT,
                                                  XML_LINENUMBERING_SEPARATOR,
                                                  true, false);
                rExport.Characters(sSeparator);
            }
        }
        // else: no configuration: don't save -> default
    }
    // can't even get supplier: don't save -> default
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
