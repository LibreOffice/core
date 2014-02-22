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
#include <tools/debug.hxx>

using namespace ::xmloff::token;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::xml::sax::XAttributeList;


TYPEINIT1( XMLIndexTabStopEntryContext, XMLIndexSimpleEntryContext );

XMLIndexTabStopEntryContext::XMLIndexTabStopEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        XMLIndexSimpleEntryContext(rImport, rTemplate.sTokenTabStop,
                                   rTemplate, nPrfx, rLocalName),
        sLeaderChar(),
        nTabPosition(0),
        bTabPositionOK(sal_False),
        bTabRightAligned(sal_False),
        bLeaderCharOK(sal_False),
        bWithTab(sal_True) 
{
}

XMLIndexTabStopEntryContext::~XMLIndexTabStopEntryContext()
{
}

void XMLIndexTabStopEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    
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
                
                
                
                bTabRightAligned = IsXMLToken( sAttr, XML_RIGHT );
            }
            else if ( IsXMLToken( sLocalName, XML_POSITION ) )
            {
                sal_Int32 nTmp;
                if (GetImport().GetMM100UnitConverter().
                                        convertMeasureToCore(nTmp, sAttr))
                {
                    nTabPosition = nTmp;
                    bTabPositionOK = sal_True;
                }
            }
            else if ( IsXMLToken( sLocalName, XML_LEADER_CHAR ) )
            {
                sLeaderChar = sAttr;
                
                bLeaderCharOK = !sAttr.isEmpty();
            }
            
            else if ( IsXMLToken( sLocalName, XML_WITH_TAB ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, sAttr))
                    bWithTab = bTmp;
            }
            
        }
        
    }

    
    nValues += 2 + (bTabPositionOK ? 1 : 0) + (bLeaderCharOK ? 1 : 0);

    
    XMLIndexSimpleEntryContext::StartElement( xAttrList );
}

void XMLIndexTabStopEntryContext::FillPropertyValues(
    Sequence<PropertyValue> & rValues)
{
    
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    
    sal_Int32 nNextEntry = bCharStyleNameOK ? 2 : 1;
    PropertyValue* pValues = rValues.getArray();

    
    pValues[nNextEntry].Name = rTemplateContext.sTabStopRightAligned;
    pValues[nNextEntry].Value.setValue( &bTabRightAligned,
                                        ::getBooleanCppuType());
    nNextEntry++;

    
    if (bTabPositionOK)
    {
        pValues[nNextEntry].Name = rTemplateContext.sTabStopPosition;
        pValues[nNextEntry].Value <<= nTabPosition;
        nNextEntry++;
    }

    
    if (bLeaderCharOK)
    {
        pValues[nNextEntry].Name = rTemplateContext.sTabStopFillCharacter;
        pValues[nNextEntry].Value <<= sLeaderChar;
        nNextEntry++;
    }

    
     pValues[nNextEntry].Name = "WithTab";
    pValues[nNextEntry].Value.setValue( &bWithTab,
                                        ::getBooleanCppuType());
    nNextEntry++;

    
    DBG_ASSERT( nNextEntry == rValues.getLength(),
                "length incorrectly precumputed!" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
