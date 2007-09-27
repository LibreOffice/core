/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: softpagebreak.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:06:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "ndtxt.hxx"
#include "txtfrm.hxx"
#include "pagefrm.hxx"
#include "swtable.hxx"
#include "frmfmt.hxx"
#include "rowfrm.hxx"
#include "tabfrm.hxx"

void SwTxtNode::fillSoftPageBreakList( SwSoftPageBreakList& rBreak ) const
{
    SwClientIter aIter( const_cast<SwTxtNode&>(*this) );
    for( const SwTxtFrm *pFrm = (const SwTxtFrm*)aIter.First( TYPE(SwTxtFrm) );
         pFrm; pFrm = (const SwTxtFrm*)aIter.Next() )
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
                if( !pFirst2->IsInTab() )
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
            if( pFirst2 == pFrm && !pFrm->IsPageBreak( TRUE ) )
                rBreak.insert( pFrm->GetOfst() );
        }
    }
}

bool SwTableLine::hasSoftPageBreak() const
{
    // No soft page break for sub tables
    if( GetUpper() || !GetFrmFmt() )
        return false;
    SwClientIter aIter( *GetFrmFmt() );
    for( SwClient* pLast = aIter.First( TYPE( SwFrm ) ); pLast;
         pLast = aIter.Next() )
    {
        if( ((SwRowFrm*)pLast)->GetTabLine() == this )
        {
            const SwTabFrm* pTab = static_cast<SwRowFrm*>(pLast)->FindTabFrm();
            // No soft page break for
            //   tables with prevs, i.e. if the frame is not the first in its layout frame
            //   tables in footer or header
            //   tables in flies
            //   inner tables of nested tables
            //   master table frames with "hard" page break attribute
            if( pTab->GetIndPrev() || pTab->FindFooterOrHeader()
                || pTab->IsInFly() || pTab->GetUpper()->IsInTab() ||
                ( !pTab->IsFollow() && pTab->IsPageBreak( TRUE ) ) )
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

