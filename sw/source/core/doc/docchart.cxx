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

#include <doc.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <ndindex.hxx>
#include <swtable.hxx>
#include <viewsh.hxx>
#include <ndole.hxx>
#include <swtblfmt.hxx>
#include <tblsel.hxx>
#include <frameformats.hxx>
#include <unochart.hxx>
#include <osl/diagnose.h>

void SwTable::UpdateCharts() const
{
    GetFrameFormat()->GetDoc().UpdateCharts( GetFrameFormat()->GetName() );
}

bool SwTable::IsTableComplexForChart( std::u16string_view aSelection ) const
{
    const SwTableBox* pSttBox, *pEndBox;
    if( 2 < aSelection.size() )
    {
        const size_t nSeparator = aSelection.find( u':' );
        OSL_ENSURE( std::u16string_view::npos != nSeparator, "no valid selection" );

        // Remove brackets at the beginning and from the end
        const sal_Int32 nOffset = '<' == aSelection[0] ? 1 : 0;
        const sal_Int32 nLength = '>' == aSelection[ aSelection.size()-1 ]
            ? aSelection.size()-1 : aSelection.size();

        pSttBox = GetTableBox(OUString(aSelection.substr( nOffset, nSeparator - nOffset )));
        pEndBox = GetTableBox(OUString(aSelection.substr( nSeparator+1, nLength - (nSeparator+1) )));
    }
    else
    {
        const SwTableLines* pLns = &GetTabLines();
        pSttBox = (*pLns)[ 0 ]->GetTabBoxes().front();
        while( !pSttBox->GetSttNd() )
            // Until the Content Box!
            pSttBox = pSttBox->GetTabLines().front()->GetTabBoxes().front();

        const SwTableBoxes* pBoxes = &pLns->back()->GetTabBoxes();
        pEndBox = pBoxes->back();
        while( !pEndBox->GetSttNd() )
        {
            // Until the Content Box!
            pLns = &pEndBox->GetTabLines();
            pBoxes = &pLns->back()->GetTabBoxes();
            pEndBox = pBoxes->back();
        }
    }

    return !pSttBox || !pEndBox || !::ChkChartSel( *pSttBox->GetSttNd(),
                                        *pEndBox->GetSttNd() );
}

void SwDoc::DoUpdateAllCharts()
{
    SwViewShell* pVSh = getIDocumentLayoutAccess().GetCurrentViewShell();
    if( !pVSh )
        return;

    for(const SwTableFormat* pFormat: *GetTableFrameFormats())
    {
        if( SwTable* pTmpTable = SwTable::FindTable( pFormat ) )
            if( const SwTableNode* pTableNd = pTmpTable->GetTableNode() )
                if( pTableNd->GetNodes().IsDocNodes() )
                {
                    UpdateCharts_( *pTmpTable, *pVSh );
                }
    }
}

void SwDoc::UpdateCharts_( const SwTable& rTable, SwViewShell const & rVSh ) const
{
    UIName aName( rTable.GetFrameFormat()->GetName() );
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwOLENode *pONd = aIdx.GetNode().GetOLENode();
        if( pONd &&
            aName == pONd->GetChartTableName() &&
            pONd->getLayoutFrame( rVSh.GetLayout() ) )
        {
            // tdf#122995 for OLE/Charts in SW we do not (yet) have a refresh
            // mechanism or embedding of the primitive representation, so this
            // needs to be done locally here (simplest solution).
            bool bImmediateMode(false);

            if(pONd->IsChart())
            {
                // refresh to trigger repaint
                const SwRect aChartRect(pONd->FindLayoutRect());
                if(!aChartRect.IsEmpty())
                    const_cast<SwViewShell &>(rVSh).InvalidateWindows(aChartRect);

                // forced refresh of the chart's primitive representation
                pONd->GetOLEObj().resetBufferedData();

                // InvalidateTable using the Immediate-Mode, else the chart will
                // not yet know that it is invalidated at the next repaint and create
                // the same graphical representation again
                bImmediateMode = true;
            }

            SwChartDataProvider *pPCD = getIDocumentChartDataProviderAccess().GetChartDataProvider();
            if (pPCD)
                pPCD->InvalidateTable( &rTable, bImmediateMode );
            // following this the framework will now take care of repainting
            // the chart or it's replacement image...
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
}

void SwDoc::UpdateCharts( const UIName& rName ) const
{
    SwTable* pTmpTable = SwTable::FindTable( FindTableFormatByName( rName ) );
    if( pTmpTable )
    {
        SwViewShell const * pVSh = getIDocumentLayoutAccess().GetCurrentViewShell();

        if( pVSh )
            UpdateCharts_( *pTmpTable, *pVSh );
    }
}

void SwDoc::SetTableName( SwFrameFormat& rTableFormat, const UIName &rNewName )
{
    const UIName aOldName( rTableFormat.GetName() );

    bool bNameFound = rNewName.isEmpty();
    if( !bNameFound )
    {
        for(const SwTableFormat* pFormat: *GetTableFrameFormats())
        {
            if( !pFormat->IsDefault() &&
                pFormat->GetName() == rNewName && IsUsed( *pFormat ) )
            {
                bNameFound = true;
                break;
            }
        }
    }

    if( !bNameFound )
        rTableFormat.SetFormatName( rNewName, true );
    else
        rTableFormat.SetFormatName( GetUniqueTableName(), true );

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
