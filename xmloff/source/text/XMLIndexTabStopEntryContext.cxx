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


#include "XMLIndexTabStopEntryContext.hxx"

#include <sax/tools/converter.hxx>

#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

using namespace ::xmloff::token;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::PropertyValue;


XMLIndexTabStopEntryContext::XMLIndexTabStopEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate ) :
        XMLIndexSimpleEntryContext(rImport, "TokenTabStop",
                                   rTemplate),
        nTabPosition(0),
        bTabPositionOK(false),
        bTabRightAligned(false),
        bLeaderCharOK(false),
        bWithTab(true) // #i21237#
{
}

XMLIndexTabStopEntryContext::~XMLIndexTabStopEntryContext()
{
}

void XMLIndexTabStopEntryContext::startFastElement(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // process three attributes: type, position, leader char
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(STYLE, XML_TYPE):
            {
                // if it's neither left nor right, value is
                // ignored. Since left is default, we only need to
                // check for right
                bTabRightAligned = IsXMLToken( aIter, XML_RIGHT );
                break;
            }
            case XML_ELEMENT(STYLE, XML_POSITION):
            {
                sal_Int32 nTmp;
                if (GetImport().GetMM100UnitConverter().
                                        convertMeasureToCore(nTmp, aIter.toView()))
                {
                    nTabPosition = nTmp;
                    bTabPositionOK = true;
                }
                break;
            }
            case XML_ELEMENT(STYLE, XML_LEADER_CHAR):
            {
                sLeaderChar = aIter.toString();
                // valid only, if we have a char!
                bLeaderCharOK = !sLeaderChar.isEmpty();
                break;
            }
            case XML_ELEMENT(STYLE, XML_WITH_TAB):
            {
                // #i21237#
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, aIter.toView()))
                    bWithTab = bTmp;
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
            // else: unknown style: attribute -> ignore
        }
    }

    // how many entries? #i21237#
    m_nValues += 2 + (bTabPositionOK ? 1 : 0) + (bLeaderCharOK ? 1 : 0);

    // now try parent class (for character style)
    XMLIndexSimpleEntryContext::startFastElement( nElement, xAttrList );
}

void XMLIndexTabStopEntryContext::FillPropertyValues(
    Sequence<PropertyValue> & rValues)
{
    // fill values from parent class (type + style name)
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // get values array and next entry to be written;
    sal_Int32 nNextEntry = m_bCharStyleNameOK ? 2 : 1;
    PropertyValue* pValues = rValues.getArray();

    // right aligned?
    pValues[nNextEntry].Name = "TabStopRightAligned";
    pValues[nNextEntry].Value <<= bTabRightAligned;
    nNextEntry++;

    // position
    if (bTabPositionOK)
    {
        pValues[nNextEntry].Name = "TabStopPosition";
        pValues[nNextEntry].Value <<= nTabPosition;
        nNextEntry++;
    }

    // leader char
    if (bLeaderCharOK)
    {
        pValues[nNextEntry].Name = "TabStopFillCharacter";
        pValues[nNextEntry].Value <<= sLeaderChar;
        nNextEntry++;
    }

    // tab character #i21237#
    pValues[nNextEntry].Name = "WithTab";
    pValues[nNextEntry].Value <<= bWithTab;
    nNextEntry++;

    // check whether we really filled all elements of the sequence
    SAL_WARN_IF( nNextEntry != rValues.getLength(), "xmloff",
                "length incorrectly precomputed!" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
