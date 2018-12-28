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

#include "XMLDetectiveContext.hxx"

#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "xmlimprt.hxx"
#include "XMLConverter.hxx"
#include <rangeutl.hxx>

using namespace ::com::sun::star;
using namespace xmloff::token;

ScMyImpDetectiveObj::ScMyImpDetectiveObj() :
    aSourceRange(),
    eObjType( SC_DETOBJ_NONE ),
    bHasError( false )
{
}

bool ScMyImpDetectiveOp::operator<(const ScMyImpDetectiveOp& rDetOp) const
{
    return (nIndex < rDetOp.nIndex);
}

void ScMyImpDetectiveOpArray::Sort()
{
    aDetectiveOpList.sort();
}

bool ScMyImpDetectiveOpArray::GetFirstOp( ScMyImpDetectiveOp& rDetOp )
{
    if( aDetectiveOpList.empty() )
        return false;
    ScMyImpDetectiveOpList::iterator aItr = aDetectiveOpList.begin();
    rDetOp = *aItr;
    aDetectiveOpList.erase( aItr );
    return true;
}

ScXMLDetectiveContext::ScXMLDetectiveContext(
        ScXMLImport& rImport,
        ScMyImpDetectiveObjVec* pNewDetectiveObjVec ) :
    ScXMLImportContext( rImport ),
    pDetectiveObjVec( pNewDetectiveObjVec )
{
}

ScXMLDetectiveContext::~ScXMLDetectiveContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDetectiveContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext*     pContext    = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_HIGHLIGHTED_RANGE ):
            pContext = new ScXMLDetectiveHighlightedContext( GetScImport(), pAttribList, pDetectiveObjVec );
        break;
        case XML_ELEMENT( TABLE, XML_OPERATION ):
            pContext = new ScXMLDetectiveOperationContext( GetScImport(), pAttribList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLDetectiveHighlightedContext::ScXMLDetectiveHighlightedContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
        ScMyImpDetectiveObjVec* pNewDetectiveObjVec ):
    ScXMLImportContext( rImport ),
    pDetectiveObjVec( pNewDetectiveObjVec ),
    aDetectiveObj(),
    bValid( false )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_CELL_RANGE_ADDRESS ):
                {
                    sal_Int32 nOffset(0);
                    ScXMLImport::MutexGuard aGuard(GetScImport());
                    bValid = ScRangeStringConverter::GetRangeFromString( aDetectiveObj.aSourceRange, aIter.toString(), GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset );
                }
                break;
                case XML_ELEMENT( TABLE, XML_DIRECTION ):
                    aDetectiveObj.eObjType = ScXMLConverter::GetDetObjTypeFromString( aIter.toString() );
                break;
                case XML_ELEMENT( TABLE, XML_CONTAINS_ERROR ):
                    aDetectiveObj.bHasError = IsXMLToken(aIter, XML_TRUE);
                break;
                case XML_ELEMENT( TABLE, XML_MARKED_INVALID ):
                {
                    if (IsXMLToken(aIter, XML_TRUE))
                        aDetectiveObj.eObjType = SC_DETOBJ_CIRCLE;
                }
                break;
            }
        }
    }
}

ScXMLDetectiveHighlightedContext::~ScXMLDetectiveHighlightedContext()
{
}

void SAL_CALL ScXMLDetectiveHighlightedContext::endFastElement( sal_Int32 /*nElement*/ )
{
    switch( aDetectiveObj.eObjType )
    {
        case SC_DETOBJ_ARROW:
        case SC_DETOBJ_TOOTHERTAB:
        break;
        case SC_DETOBJ_FROMOTHERTAB:
        case SC_DETOBJ_CIRCLE:
            bValid = true;
        break;
        default:
            bValid = false;
    }
    if( bValid )
        pDetectiveObjVec->push_back( aDetectiveObj );
}

ScXMLDetectiveOperationContext::ScXMLDetectiveOperationContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    aDetectiveOp(),
    bHasType( false )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_NAME ):
                    bHasType = ScXMLConverter::GetDetOpTypeFromString( aDetectiveOp.eOpType, aIter.toString() );
                break;
                case XML_ELEMENT( TABLE, XML_INDEX ):
                {
                    sal_Int32 nValue;
                    if (::sax::Converter::convertNumber( nValue, aIter.toString(), 0 ))
                        aDetectiveOp.nIndex = nValue;
                }
                break;
            }
        }
    }
    aDetectiveOp.aPosition =  rImport.GetTables().GetCurrentCellPos();
}

ScXMLDetectiveOperationContext::~ScXMLDetectiveOperationContext()
{
}

void SAL_CALL ScXMLDetectiveOperationContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if( bHasType && (aDetectiveOp.nIndex >= 0) )
        GetScImport().GetDetectiveOpArray()->AddDetectiveOp( aDetectiveOp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
