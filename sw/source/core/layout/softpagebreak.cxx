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

#include "ndtxt.hxx"
#include "txtfrm.hxx"
#include "pagefrm.hxx"
#include "swtable.hxx"
#include "frmfmt.hxx"
#include "rowfrm.hxx"
#include "tabfrm.hxx"
#include "switerator.hxx"

void SwTxtNode::fillSoftPageBreakList( SwSoftPageBreakList& rBreak ) const
{
    SwIterator<SwTxtFrm,SwTxtNode> aIter( *this );
    for( const SwTxtFrm *pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
    {
        // No soft page break in header or footer
        if( pFrm->FindFooterOrHeader() || pFrm->IsInFly() )
            return;
        // No soft page break if I'm not the first frame in my layout frame
        if( pFrm->GetIndPrev() )
            continue;
        const SwPageFrm* pPage = pFrm->FindPageFrm();
        // No soft page break at the first page
        if( pPage && pPage->GetPrev() )
        {
            const SwCntntFrm* pFirst2 = pPage->FindFirstBodyCntnt();
            // Special handling for content frame in table frames
            if( pFrm->IsInTab() )
            {
                // No soft page break if I'm in a table but the first content frame
                // at my page is not in a table
                if( !pFirst2 || !pFirst2->IsInTab() )
                    continue;
                const SwLayoutFrm *pRow = pFrm->GetUpper();
                // Looking for the "most upper" row frame,
                // skipping sub tables and/or table in table
                while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ||
                    pRow->GetUpper()->GetUpper()->IsInTab() )
                    pRow = pRow->GetUpper();
                const SwTabFrm *pTab = pRow->FindTabFrm();
                // For master tables the soft page break will exported at the table row,
                // not at the content frame.
                // If the first content is outside my table frame, no soft page break.
                if( !pTab->IsFollow() || !pTab->IsAnLower( pFirst2 ) )
                    continue;
                // Only content of non-heading-rows can get a soft page break
                const SwFrm* pFirstRow = pTab->GetFirstNonHeadlineRow();
                // If there's no follow flow line, the soft page break will be
                // exported at the row, not at the content.
                if( pRow == pFirstRow &&
                    pTab->FindMaster( false )->HasFollowFlowLine() )
                {
                    // Now we have the row which causes a new page,
                    // this row is a follow flow line and therefor cannot get
                    // the soft page break itself.
                    // Every first content frame of every cell frane in this row
                    // will get the soft page break
                    const SwFrm* pCell = pRow->Lower();
                    while( pCell )
                    {
                        pFirst2 = static_cast<const SwLayoutFrm*>(pCell)->ContainsCntnt();
                        if( pFirst2 == pFrm )
                        {   // Here we are: a first content inside a cell
                            // inside the splitted row => soft page break
                            rBreak.insert( pFrm->GetOfst() );
                            break;
                        }
                        pCell = pCell->GetNext();
                    }
                }
            }
            else // No soft page break if there's a "hard" page break attribute
            if( pFirst2 == pFrm && !pFrm->IsPageBreak( sal_True ) )
                rBreak.insert( pFrm->GetOfst() );
        }
    }
}

bool SwTableLine::hasSoftPageBreak() const
{
    // No soft page break for sub tables
    if( GetUpper() || !GetFrmFmt() )
        return false;
    SwIterator<SwRowFrm,SwFmt> aIter( *GetFrmFmt() );
    for( SwRowFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        if( pLast->GetTabLine() == this )
        {
            const SwTabFrm* pTab = pLast->FindTabFrm();
            // No soft page break for
            //   tables with prevs, i.e. if the frame is not the first in its layout frame
            //   tables in footer or header
            //   tables in flies
            //   inner tables of nested tables
            //   master table frames with "hard" page break attribute
            if( pTab->GetIndPrev() || pTab->FindFooterOrHeader()
                || pTab->IsInFly() || pTab->GetUpper()->IsInTab() ||
                ( !pTab->IsFollow() && pTab->IsPageBreak( sal_True ) ) )
                return false;
            const SwPageFrm* pPage = pTab->FindPageFrm();
            // No soft page break at the first page of the document
            if( pPage && !pPage->GetPrev() )
                return false;
            const SwCntntFrm* pFirst = pPage->FindFirstBodyCntnt();
            // No soft page break for
            //   tables which does not contain the first body content of the page
            if( !pFirst || !pTab->IsAnLower( pFirst->FindTabFrm() ) )
                return false;
            // The row which could get a soft page break must be either the first
            // row of a master table frame or the first "non-headline-row" of a
            // follow table frame...
            const SwFrm* pRow = pTab->IsFollow() ?
                pTab->GetFirstNonHeadlineRow() : pTab->Lower();
            if( pRow == pLast )
            {
                // The last check: no soft page break for "follow" table lines
                if( pTab->IsFollow() && pTab->FindMaster( false )->HasFollowFlowLine() )
                    return false;
                return true;
            }
            return false;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
