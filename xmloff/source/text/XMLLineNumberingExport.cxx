/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "XMLLineNumberingExport.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
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
:   sCharStyleName("CharStyleName")
,   sCountEmptyLines("CountEmptyLines")
,   sCountLinesInFrames("CountLinesInFrames")
,   sDistance("Distance")
,   sInterval("Interval")
,   sSeparatorText("SeparatorText")
,   sNumberPosition("NumberPosition")
,   sNumberingType("NumberingType")
,   sIsOn("IsOn")
,   sRestartAtEachPage("RestartAtEachPage")
,   sSeparatorInterval("SeparatorInterval")
,   rExport(rExp)
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
    
    Reference<XLineNumberingProperties> xSupplier(rExport.GetModel(),
                                                  UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XPropertySet> xLineNumbering =
            xSupplier->getLineNumberingProperties();

        if (xLineNumbering.is())
        {
            Any aAny;

            
            aAny = xLineNumbering->getPropertyValue(sCharStyleName);
            OUString sTmp;
            aAny >>= sTmp;
            if (!sTmp.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                     rExport.EncodeStyleName( sTmp ));
            }

            
            aAny = xLineNumbering->getPropertyValue(sIsOn);
            if (! *(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_NUMBER_LINES, XML_FALSE);
            }

            
            aAny = xLineNumbering->getPropertyValue(sCountEmptyLines);
            if (! *(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_COUNT_EMPTY_LINES, XML_FALSE);
            }

            
            aAny = xLineNumbering->getPropertyValue(sCountLinesInFrames);
            if (*(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_COUNT_IN_TEXT_BOXES, XML_TRUE);
            }

            
            aAny = xLineNumbering->getPropertyValue(sRestartAtEachPage);
            if (*(sal_Bool*)aAny.getValue())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_RESTART_ON_PAGE, XML_TRUE);
            }

            
            aAny = xLineNumbering->getPropertyValue(sDistance);
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

            
            OUStringBuffer sNumPosBuf;
            aAny = xLineNumbering->getPropertyValue(sNumberingType);
            sal_Int16 nFormat = 0;
            aAny >>= nFormat;
            rExport.GetMM100UnitConverter().convertNumFormat( sNumPosBuf, nFormat );
            rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                 sNumPosBuf.makeStringAndClear());
            rExport.GetMM100UnitConverter().convertNumLetterSync( sNumPosBuf, nFormat );
            if( !sNumPosBuf.isEmpty() )
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_NUM_LETTER_SYNC,
                                     sNumPosBuf.makeStringAndClear() );
            }

            
            aAny = xLineNumbering->getPropertyValue(sNumberPosition);
            sal_Int16 nPosition = 0;
            aAny >>= nPosition;
            if (SvXMLUnitConverter::convertEnum(sNumPosBuf, nPosition,
                                                aLineNumberPositionMap))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_NUMBER_POSITION,
                                     sNumPosBuf.makeStringAndClear());
            }

            
            aAny = xLineNumbering->getPropertyValue(sInterval);
            sal_Int16 nLineInterval = 0;
            aAny >>= nLineInterval;
            OUStringBuffer sBuf;
            ::sax::Converter::convertNumber(sBuf,
                                              (sal_Int32)nLineInterval);
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_INCREMENT,
                                 sBuf.makeStringAndClear());

            SvXMLElementExport aConfigElem(rExport, XML_NAMESPACE_TEXT,
                                           XML_LINENUMBERING_CONFIGURATION,
                                           sal_True, sal_True);

            
            aAny = xLineNumbering->getPropertyValue(sSeparatorText);
            OUString sSeparator;
            aAny >>= sSeparator;
            if (!sSeparator.isEmpty())
            {

                
                aAny = xLineNumbering->getPropertyValue(sSeparatorInterval);
                sal_Int16 nLineDistance = 0;
                aAny >>= nLineDistance;
                ::sax::Converter::convertNumber(sBuf,
                                                  (sal_Int32)nLineDistance);
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_INCREMENT,
                                     sBuf.makeStringAndClear());

                SvXMLElementExport aSeparatorElem(rExport, XML_NAMESPACE_TEXT,
                                                  XML_LINENUMBERING_SEPARATOR,
                                                  sal_True, sal_False);
                rExport.Characters(sSeparator);
            }
        }
        
    }
    
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
