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

#include "document.hxx"
#include "xmlimprt.hxx"
#include "xmlsceni.hxx"
#include "docuno.hxx"
#include "attrib.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>

#include <sax/tools/converter.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableScenarioContext::ScXMLTableScenarioContext(
        ScXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aBorderColor( COL_BLACK ),
    bDisplayBorder( true ),
    bCopyBack( true ),
    bCopyStyles( true ),
    bCopyFormulas( true ),
    bIsActive( false ),
    bProtected( false )
{
    rImport.LockSolarMutex();
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetTableScenarioAttrTokenMap());
    for( sal_Int16 i = 0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_SCENARIO_ATTR_DISPLAY_BORDER:
            {
                bDisplayBorder = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_BORDER_COLOR:
            {
                sal_Int32 nColor(0);
                ::sax::Converter::convertColor(nColor, sValue);
                aBorderColor.SetColor(nColor);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COPY_BACK:
            {
                bCopyBack = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COPY_STYLES:
            {
                bCopyStyles = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COPY_FORMULAS:
            {
                bCopyFormulas = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_IS_ACTIVE:
            {
                bIsActive = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_SCENARIO_RANGES:
            {
                ScRangeStringConverter::GetRangeListFromString(
                    aScenarioRanges, sValue, GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO );
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COMMENT:
            {
                sComment = sValue;
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_PROTECTED:
            {
                bProtected = IsXMLToken(sValue, XML_TRUE);
            }
            break;
        }
    }
}

ScXMLTableScenarioContext::~ScXMLTableScenarioContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLTableScenarioContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLTableScenarioContext::EndElement()
{
    SCTAB nCurrTable( GetScImport().GetTables().GetCurrentSheet() );
    ScDocument* pDoc(GetScImport().GetDocument());
    if (pDoc)
    {
        pDoc->SetScenario( nCurrTable, true );
        ScScenarioFlags nFlags( ScScenarioFlags::NONE );
        if( bDisplayBorder )
            nFlags |= ScScenarioFlags::ShowFrame;
        if( bCopyBack )
            nFlags |= ScScenarioFlags::TwoWay;
        if( bCopyStyles )
            nFlags |= ScScenarioFlags::Attrib;
        if( !bCopyFormulas )
            nFlags |= ScScenarioFlags::Value;
        if( bProtected )
            nFlags |= ScScenarioFlags::Protected;
        pDoc->SetScenarioData( nCurrTable, OUString( sComment ), aBorderColor, nFlags );
        for( size_t i = 0; i < aScenarioRanges.size(); ++i )
        {
            ScRange* pRange(aScenarioRanges[ i ]);
            if( pRange )
                pDoc->ApplyFlagsTab( pRange->aStart.Col(), pRange->aStart.Row(),
                    pRange->aEnd.Col(), pRange->aEnd.Row(), nCurrTable, ScMF::Scenario );
        }
        pDoc->SetActiveScenario( nCurrTable, bIsActive );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
