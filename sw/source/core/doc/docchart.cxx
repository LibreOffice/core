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

#include <float.h>
#include <hintids.hxx>
#include <vcl/window.hxx>
#include <doc.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <ndindex.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <cellfml.hxx>
#include <viewsh.hxx>
#include <ndole.hxx>
#include <calbck.hxx>
#include <cntfrm.hxx>
#include <swtblfmt.hxx>
#include <tblsel.hxx>
#include <cellatr.hxx>

#include <unochart.hxx>

void SwTable::UpdateCharts() const
{
    GetFrameFormat()->GetDoc()->UpdateCharts( GetFrameFormat()->GetName() );
}

bool SwTable::IsTableComplexForChart( const OUString& rSelection ) const
{
    const SwTableBox* pSttBox, *pEndBox;
    if( 2 < rSelection.getLength() )
    {
        const sal_Int32 nSeparator {rSelection.indexOf( ':' )};
        OSL_ENSURE( -1 != nSeparator, "no valid selection" );

        // Remove brackets at the beginning and from the end
        const sal_Int32 nOffset {'<' == rSelection[0] ? 1 : 0};
        const sal_Int32 nLength {'>' == rSelection[ rSelection.getLength()-1 ]
            ? rSelection.getLength()-1 : rSelection.getLength()};

        pSttBox = GetTableBox(rSelection.copy( nOffset, nSeparator - nOffset ));
        pEndBox = GetTableBox(rSelection.copy( nSeparator+1, nLength - (nSeparator+1) ));
    }
    else
    {
        const SwTableLines* pLns = &GetTabLines();
        pSttBox = (*pLns)[ 0 ]->GetTabBoxes().front().get();
        while( !pSttBox->GetSttNd() )
            // Until the Content Box!
            pSttBox = pSttBox->GetTabLines().front()->GetTabBoxes().front().get();

        const SwTableBoxes* pBoxes = &pLns->back()->GetTabBoxes();
        pEndBox = pBoxes->back().get();
        while( !pEndBox->GetSttNd() )
        {
            // Until the Content Box!
            pLns = &pEndBox->GetTabLines();
            pBoxes = &pLns->back()->GetTabBoxes();
            pEndBox = pBoxes->back().get();
        }
    }

    return !pSttBox || !pEndBox || !::ChkChartSel( *pSttBox->GetSttNd(),
                                        *pEndBox->GetSttNd() );
}

void SwDoc::DoUpdateAllCharts()
{
    SwViewShell* pVSh = getIDocumentLayoutAccess().GetCurrentViewShell();
    if( pVSh )
    {
        const SwFrameFormats& rTableFormats = *GetTableFrameFormats();
        for( size_t n = 0; n < rTableFormats.size(); ++n )
        {
            SwTable* pTmpTable;
            const SwTableNode* pTableNd;
            const SwFrameFormat* pFormat = rTableFormats[ n ];

            if( nullptr != ( pTmpTable = SwTable::FindTable( pFormat ) ) &&
                nullptr != ( pTableNd = pTmpTable->GetTableNode() ) &&
                pTableNd->GetNodes().IsDocNodes() )
            {
                UpdateCharts_( *pTmpTable, *pVSh );
            }
        }
    }
}

void SwDoc::UpdateCharts_( const SwTable& rTable, SwViewShell const & rVSh ) const
{
    OUString aName( rTable.GetFrameFormat()->GetName() );
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwOLENode *pONd;
        if( nullptr != ( pONd = aIdx.GetNode().GetOLENode() ) &&
            aName == pONd->GetChartTableName() &&
            pONd->getLayoutFrame( rVSh.GetLayout() ) )
        {
            SwChartDataProvider *pPCD = getIDocumentChartDataProviderAccess().GetChartDataProvider();
            if (pPCD)
                pPCD->InvalidateTable( &rTable );
            // following this the framework will now take care of repainting
            // the chart or it's replacement image...
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
}

void SwDoc::UpdateCharts( const OUString &rName ) const
{
    SwTable* pTmpTable = SwTable::FindTable( FindTableFormatByName( rName ) );
    if( pTmpTable )
    {
        SwViewShell const * pVSh = getIDocumentLayoutAccess().GetCurrentViewShell();

        if( pVSh )
            UpdateCharts_( *pTmpTable, *pVSh );
    }
}

void SwDoc::SetTableName( SwFrameFormat& rTableFormat, const OUString &rNewName )
{
    const OUString aOldName( rTableFormat.GetName() );

    bool bNameFound = rNewName.isEmpty();
    if( !bNameFound )
    {
        const SwFrameFormats& rTable = *GetTableFrameFormats();
        for( size_t i = rTable.size(); i; )
        {
            const SwFrameFormat* pFormat;
            if( !( pFormat = rTable[ --i ] )->IsDefault() &&
                pFormat->GetName() == rNewName && IsUsed( *pFormat ) )
            {
                bNameFound = true;
                break;
            }
        }
    }

    if( !bNameFound )
        rTableFormat.SetName( rNewName, true );
    else
        rTableFormat.SetName( GetUniqueTableName(), true );

    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while ( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwOLENode *pNd = aIdx.GetNode().GetOLENode();
        if( pNd && aOldName == pNd->GetChartTableName() )
        {
            pNd->SetChartTableName( rNewName );

            SwTable* pTable = SwTable::FindTable( &rTableFormat );
            SwChartDataProvider *pPCD = getIDocumentChartDataProviderAccess().GetChartDataProvider();
            if (pPCD)
                pPCD->InvalidateTable( pTable );
            // following this the framework will now take care of repainting
            // the chart or it's replacement image...
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
    getIDocumentState().SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
