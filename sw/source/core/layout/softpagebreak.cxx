/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
        
        if( pFrm->FindFooterOrHeader() || pFrm->IsInFly() )
            return;
        
        if( pFrm->GetIndPrev() )
            continue;
        const SwPageFrm* pPage = pFrm->FindPageFrm();
        
        if( pPage && pPage->GetPrev() )
        {
            const SwCntntFrm* pFirst2 = pPage->FindFirstBodyCntnt();
            
            if( pFrm->IsInTab() )
            {
                
                
                if( !pFirst2->IsInTab() )
                    continue;
                const SwLayoutFrm *pRow = pFrm->GetUpper();
                
                
                while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ||
                    pRow->GetUpper()->GetUpper()->IsInTab() )
                    pRow = pRow->GetUpper();
                const SwTabFrm *pTab = pRow->FindTabFrm();
                
                
                
                if( !pTab->IsFollow() || !pTab->IsAnLower( pFirst2 ) )
                    continue;
                
                const SwFrm* pFirstRow = pTab->GetFirstNonHeadlineRow();
                
                
                if( pRow == pFirstRow &&
                    pTab->FindMaster( false )->HasFollowFlowLine() )
                {
                    
                    
                    
                    
                    
                    const SwFrm* pCell = pRow->Lower();
                    while( pCell )
                    {
                        pFirst2 = static_cast<const SwLayoutFrm*>(pCell)->ContainsCntnt();
                        if( pFirst2 == pFrm )
                        {   
                            
                            rBreak.insert( pFrm->GetOfst() );
                            break;
                        }
                        pCell = pCell->GetNext();
                    }
                }
            }
            else 
            if( pFirst2 == pFrm && !pFrm->IsPageBreak( sal_True ) )
                rBreak.insert( pFrm->GetOfst() );
        }
    }
}

bool SwTableLine::hasSoftPageBreak() const
{
    
    if( GetUpper() || !GetFrmFmt() )
        return false;
    SwIterator<SwRowFrm,SwFmt> aIter( *GetFrmFmt() );
    for( SwRowFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        if( pLast->GetTabLine() == this )
        {
            const SwTabFrm* pTab = pLast->FindTabFrm();
            
            
            
            
            
            
            if( pTab->GetIndPrev() || pTab->FindFooterOrHeader()
                || pTab->IsInFly() || pTab->GetUpper()->IsInTab() ||
                ( !pTab->IsFollow() && pTab->IsPageBreak( sal_True ) ) )
                return false;
            const SwPageFrm* pPage = pTab->FindPageFrm();
            
            if( pPage && !pPage->GetPrev() )
                return false;
            const SwCntntFrm* pFirst = pPage->FindFirstBodyCntnt();
            
            
            if( !pFirst || !pTab->IsAnLower( pFirst->FindTabFrm() ) )
                return false;
            
            
            
            const SwFrm* pRow = pTab->IsFollow() ?
                pTab->GetFirstNonHeadlineRow() : pTab->Lower();
            if( pRow == pLast )
            {
                
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
