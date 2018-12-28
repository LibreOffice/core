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

#include <document.hxx>
#include "xmlimprt.hxx"
#include "xmlsceni.hxx"
#include <attrib.hxx>
#include <rangeutl.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

#include <sax/tools/converter.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableScenarioContext::ScXMLTableScenarioContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ):
    ScXMLImportContext( rImport ),
    aBorderColor( COL_BLACK ),
    bDisplayBorder( true ),
    bCopyBack( true ),
    bCopyStyles( true ),
    bCopyFormulas( true ),
    bIsActive( false ),
    bProtected( false )
{
    rImport.LockSolarMutex();

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_DISPLAY_BORDER ):
                bDisplayBorder = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_ELEMENT( TABLE, XML_BORDER_COLOR ):
                ::sax::Converter::convertColor(aBorderColor, aIter.toString());
                break;
            case XML_ELEMENT( TABLE, XML_COPY_BACK ):
                bCopyBack = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_ELEMENT( TABLE, XML_COPY_STYLES ):
                bCopyStyles = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_ELEMENT( TABLE, XML_COPY_FORMULAS ):
                bCopyFormulas = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_ELEMENT( TABLE, XML_IS_ACTIVE ):
                bIsActive = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_ELEMENT( TABLE, XML_SCENARIO_RANGES ):
                ScRangeStringConverter::GetRangeListFromString(
                    aScenarioRanges, aIter.toString(), GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO );
                break;
            case XML_ELEMENT( TABLE, XML_COMMENT ):
                sComment = aIter.toString();
                break;
            case XML_ELEMENT( TABLE, XML_PROTECTED ):
                bProtected = IsXMLToken(aIter, XML_TRUE);
                break;
            }
        }
    }
}

ScXMLTableScenarioContext::~ScXMLTableScenarioContext()
{
    GetScImport().UnlockSolarMutex();
}

void SAL_CALL ScXMLTableScenarioContext::endFastElement( sal_Int32 /*nElement*/ )
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
        pDoc->SetScenarioData( nCurrTable, sComment, aBorderColor, nFlags );
        for( size_t i = 0; i < aScenarioRanges.size(); ++i )
        {
            ScRange const & rRange = aScenarioRanges[ i ];
            pDoc->ApplyFlagsTab( rRange.aStart.Col(), rRange.aStart.Row(),
                rRange.aEnd.Col(), rRange.aEnd.Row(), nCurrTable, ScMF::Scenario );
        }
        pDoc->SetActiveScenario( nCurrTable, bIsActive );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
