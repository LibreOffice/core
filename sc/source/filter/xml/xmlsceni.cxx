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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "document.hxx"
#include "xmlimprt.hxx"
#include "xmlsceni.hxx"
#include "docuno.hxx"
#include "attrib.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;
using ::rtl::OUString;

//------------------------------------------------------------------

ScXMLTableScenarioContext::ScXMLTableScenarioContext(
        ScXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aBorderColor( COL_BLACK ),
    bDisplayBorder( sal_True ),
    bCopyBack( sal_True ),
    bCopyStyles( sal_True ),
    bCopyFormulas( sal_True ),
    bIsActive( sal_False ),
    bProtected( sal_False )
{
    rImport.LockSolarMutex();
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetTableScenarioAttrTokenMap());
    for( sal_Int16 i = 0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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
    SCTAB nCurrTable( sal::static_int_cast<SCTAB>( GetScImport().GetTables().GetCurrentSheet() ) );
    ScDocument* pDoc(GetScImport().GetDocument());
    if (pDoc)
    {
        pDoc->SetScenario( nCurrTable, sal_True );
        sal_uInt16 nFlags( 0 );
        if( bDisplayBorder )
            nFlags |= SC_SCENARIO_SHOWFRAME;
        if( bCopyBack )
            nFlags |= SC_SCENARIO_TWOWAY;
        if( bCopyStyles )
            nFlags |= SC_SCENARIO_ATTRIB;
        if( !bCopyFormulas )
            nFlags |= SC_SCENARIO_VALUE;
        if( bProtected )
            nFlags |= SC_SCENARIO_PROTECT;
        pDoc->SetScenarioData( nCurrTable, String( sComment ), aBorderColor, nFlags );
        for( sal_Int32 i = 0; i < static_cast<sal_Int32>(aScenarioRanges.Count()); ++i )
        {
            ScRange* pRange(aScenarioRanges.GetObject( i ));
            if( pRange )
                pDoc->ApplyFlagsTab( pRange->aStart.Col(), pRange->aStart.Row(),
                    pRange->aEnd.Col(), pRange->aEnd.Row(), nCurrTable, SC_MF_SCENARIO );
        }
        pDoc->SetActiveScenario( nCurrTable, bIsActive );
    }
}

