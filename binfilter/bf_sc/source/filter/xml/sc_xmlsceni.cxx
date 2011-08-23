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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "document.hxx"
#include "xmlimprt.hxx"
#include "xmlsceni.hxx"
#include "attrib.hxx"

#include "XMLConverter.hxx"

#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmluconv.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

using rtl::OUString;

//------------------------------------------------------------------

ScXMLTableScenarioContext::ScXMLTableScenarioContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aBorderColor( COL_BLACK ),
    bDisplayBorder( sal_True ),
    bCopyBack( sal_True ),
    bCopyStyles( sal_True ),
    bCopyFormulas( sal_True ),
    bIsActive( sal_False )
{
    rImport.LockSolarMutex();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableScenarioAttrTokenMap();
    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_SCENARIO_ATTR_DISPLAY_BORDER:
            {
                bDisplayBorder = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_BORDER_COLOR:
            {
                SvXMLUnitConverter::convertColor(aBorderColor, sValue);
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
                ScXMLConverter::GetRangeListFromString(
                    aScenarioRanges, sValue, GetScImport().GetDocument() );
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COMMENT:
            {
                sComment = sValue;
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
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLTableScenarioContext::EndElement()
{
    sal_Int16	nCurrTable( GetScImport().GetTables().GetCurrentSheet() );
    ScDocument*	pDoc = GetScImport().GetDocument();
    if (pDoc)
    {
        pDoc->SetScenario( nCurrTable, TRUE );
        USHORT nFlags( 0 );
        if( bDisplayBorder )
            nFlags |= SC_SCENARIO_SHOWFRAME;
        if( bCopyBack )
            nFlags |= SC_SCENARIO_TWOWAY;
        if( bCopyStyles )
            nFlags |= SC_SCENARIO_ATTRIB;
        if( !bCopyFormulas )
            nFlags |= SC_SCENARIO_VALUE;
        pDoc->SetScenarioData( nCurrTable, String( sComment ), aBorderColor, nFlags );
        for( sal_Int32 i = 0; i < static_cast<sal_Int32>(aScenarioRanges.Count()); i++ )
        {
            ScRange* pRange = aScenarioRanges.GetObject( i );
            if( pRange )
                pDoc->ApplyFlagsTab( pRange->aStart.Col(), pRange->aStart.Row(),
                    pRange->aEnd.Col(), pRange->aEnd.Row(), nCurrTable, SC_MF_SCENARIO );
        }
        pDoc->SetActiveScenario( nCurrTable, bIsActive );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
