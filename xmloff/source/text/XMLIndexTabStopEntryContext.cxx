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
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

using namespace ::xmloff::token;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::xml::sax::XAttributeList;


XMLIndexTabStopEntryContext::XMLIndexTabStopEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        XMLIndexSimpleEntryContext(rImport, "TokenTabStop",
                                   rTemplate, nPrfx, rLocalName),
        sLeaderChar(),
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

void XMLIndexTabStopEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process three attributes: type, position, leader char
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sAttr = xAttrList->getValueByIndex(nAttr);
        if (XML_NAMESPACE_STYLE == nPrefix)
        {
            if ( IsXMLToken( sLocalName, XML_TYPE ) )
            {
                // if it's neither left nor right, value is
                // ignored. Since left is default, we only need to
                // check for right
                bTabRightAligned = IsXMLToken( sAttr, XML_RIGHT );
            }
            else if ( IsXMLToken( sLocalName, XML_POSITION ) )
            {
                sal_Int32 nTmp;
                if (GetImport().GetMM100UnitConverter().
                                        convertMeasureToCore(nTmp, sAttr))
                {
                    nTabPosition = nTmp;
                    bTabPositionOK = true;
                }
            }
            else if ( IsXMLToken( sLocalName, XML_LEADER_CHAR ) )
            {
                sLeaderChar = sAttr;
                // valid only, if we have a char!
                bLeaderCharOK = !sAttr.isEmpty();
            }
            // #i21237#
            else if ( IsXMLToken( sLocalName, XML_WITH_TAB ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, sAttr))
                    bWithTab = bTmp;
            }
            // else: unknown style: attribute -> ignore
        }
        // else: no style attribute -> ignore
    }

    // how many entries? #i21237#
    nValues += 2 + (bTabPositionOK ? 1 : 0) + (bLeaderCharOK ? 1 : 0);

    // now try parent class (for character style)
    XMLIndexSimpleEntryContext::StartElement( xAttrList );
}

void XMLIndexTabStopEntryContext::FillPropertyValues(
    Sequence<PropertyValue> & rValues)
{
    // fill values from parent class (type + style name)
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // get values array and next entry to be written;
    sal_Int32 nNextEntry = bCharStyleNameOK ? 2 : 1;
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
