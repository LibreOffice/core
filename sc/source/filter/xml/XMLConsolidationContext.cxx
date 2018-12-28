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

#include "XMLConsolidationContext.hxx"
#include <document.hxx>
#include <rangeutl.hxx>
#include "xmlimprt.hxx"
#include "XMLConverter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

using namespace ::com::sun::star;
using namespace xmloff::token;

ScXMLConsolidationContext::ScXMLConsolidationContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    eFunction( SUBTOTAL_FUNC_NONE ),
    bLinkToSource( false ),
    bTargetAddr(false)
{
    rImport.LockSolarMutex();
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_FUNCTION ):
                    eFunction = ScXMLConverter::GetSubTotalFuncFromString( aIter.toString() );
                break;
                case XML_ELEMENT( TABLE, XML_SOURCE_CELL_RANGE_ADDRESSES ):
                    sSourceList = aIter.toString();
                break;
                case XML_ELEMENT( TABLE, XML_TARGET_CELL_ADDRESS ):
                    {
                        sal_Int32 nOffset(0);
                        bTargetAddr = ScRangeStringConverter::GetAddressFromString(
                            aTargetAddr, aIter.toString(), GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset );
                    }
                    break;
                case XML_ELEMENT( TABLE, XML_USE_LABEL ):
                    sUseLabel = aIter.toString();
                break;
                case XML_ELEMENT( TABLE, XML_LINK_TO_SOURCE_DATA ):
                    bLinkToSource = IsXMLToken( aIter, XML_TRUE );
                break;
            }
        }
    }
}

ScXMLConsolidationContext::~ScXMLConsolidationContext()
{
    GetScImport().UnlockSolarMutex();
}

void SAL_CALL ScXMLConsolidationContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (bTargetAddr)
    {
        std::unique_ptr<ScConsolidateParam> pConsParam(new ScConsolidateParam);
        pConsParam->nCol = aTargetAddr.Col();
        pConsParam->nRow = aTargetAddr.Row();
        pConsParam->nTab = aTargetAddr.Tab();
        pConsParam->eFunction = eFunction;

        sal_uInt16 nCount = static_cast<sal_uInt16>(std::min( ScRangeStringConverter::GetTokenCount( sSourceList ), sal_Int32(0xFFFF) ));
        if( nCount )
        {
            std::unique_ptr<ScArea[]> ppAreas(new ScArea[ nCount ]);
            sal_Int32 nOffset = 0;
            sal_uInt16 nIndex;
            for( nIndex = 0; nIndex < nCount; ++nIndex )
            {
                if ( !ScRangeStringConverter::GetAreaFromString(
                    ppAreas[ nIndex ], sSourceList, GetScImport().GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset ) )
                {
                    //! handle error
                }
            }

            pConsParam->SetAreas( std::move(ppAreas), nCount );
        }

        pConsParam->bByCol = pConsParam->bByRow = false;
        if( IsXMLToken(sUseLabel, XML_COLUMN ) )
            pConsParam->bByCol = true;
        else if( IsXMLToken( sUseLabel, XML_ROW ) )
            pConsParam->bByRow = true;
        else if( IsXMLToken( sUseLabel, XML_BOTH ) )
            pConsParam->bByCol = pConsParam->bByRow = true;

        pConsParam->bReferenceData = bLinkToSource;

        ScDocument* pDoc = GetScImport().GetDocument();
        if( pDoc )
            pDoc->SetConsolidateDlgData( std::move(pConsParam) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
