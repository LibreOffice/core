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

#include <hintids.hxx>
#include <tools/cachestr.hxx>
#include <svtools/rtftoken.h>
#include <svl/itemiter.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <fmtsrnd.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <swparrtf.hxx>
#include <grfatr.hxx>
#include <paratr.hxx>
#include <rtf.hxx>
#include <ndgrf.hxx>
#include <pagedesc.hxx>
#include <swtable.hxx>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <fltini.hxx>
#include <unoframe.hxx>
#include <deque>
#include <map>
#include <utility>
// #i27767#
#include <fmtwrapinfluenceonobjpos.hxx>
#include <editeng/brshitem.hxx>
#include <fmtfollowtextflow.hxx>
#include "dcontact.hxx"

using namespace ::com::sun::star;

#define ANCHOR(p)   ((SwFmtAnchor*)p)

// steht in shellio.hxx
extern SwCntntNode* GoNextNds( SwNodeIndex * pIdx, sal_Bool bChk );

inline const SwFmtFrmSize GetFrmSize(const SfxItemSet& rSet, sal_Bool bInP=sal_True)
{
    return (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE,bInP);
}

SwFlySave::SwFlySave(const SwPaM& rPam, SfxItemSet& rSet)
    : aFlySet(rSet), nSttNd(rPam.GetPoint()->nNode), nEndNd(nSttNd), nEndCnt(0),
     nPageWidth(ATT_MIN_SIZE), nDropLines(0), nDropAnchor(0)
{
}

int SwFlySave::IsEqualFly( const SwPaM& rPos, SfxItemSet& rSet )
{
    if( rSet.Count() != aFlySet.Count() || nDropAnchor )
        return sal_False;

    // nur TextNodes zusammenfassen
    if( nSttNd == nEndNd && nEndNd.GetNode().IsNoTxtNode() )
        return sal_False;

    // teste auf gleiche / naechste Position
    if( rPos.GetPoint()->nNode.GetIndex() == nEndNd.GetIndex() )
    {
        if( 1 < (rPos.GetPoint()->nContent.GetIndex() - nEndCnt) )
            return sal_False;
    }
    else if( rPos.GetPoint()->nContent.GetIndex() )
        return sal_False;
    else
    {
        SwNodeIndex aIdx( nEndNd );
        SwCntntNode *const pCNd = aIdx.GetNode().GetCntntNode();
        if( !GoNextNds( &aIdx, sal_True ) ||
            aIdx.GetIndex() != rPos.GetPoint()->nNode.GetIndex() ||
            ( pCNd && pCNd->Len() != nEndCnt ))
        {
            return sal_False;
        }
    }

    if( rSet.Count() )
    {
        SfxItemIter aIter( rSet );
        const SfxPoolItem *pItem, *pCurr = aIter.GetCurItem();
        while( sal_True )
        {
            if( SFX_ITEM_SET != aFlySet.GetItemState( pCurr->Which(),
                sal_False, &pItem ) ||
                // Ankerattribute gesondert behandeln
                ( RES_ANCHOR == pCurr->Which()
                    ? (ANCHOR(pCurr)->GetAnchorId() != ANCHOR(pItem)->GetAnchorId() ||
                       ANCHOR(pCurr)->GetPageNum() != ANCHOR(pItem)->GetPageNum())
                    : *pItem != *pCurr ))
                        return sal_False;

            if( aIter.IsAtEnd() )
                break;
            pCurr = aIter.NextItem();
        }
    }
    return sal_True;
}

void SwFlySave::SetFlySize( const SwTableNode& rTblNd )
{
    // sollte der Fly kleiner als diese Tabelle sein, dann
    // korrigiere diesen (nur bei abs. Angaben!)
    SwTwips nWidth = rTblNd.GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
    const SwFmtFrmSize& rSz = GetFrmSize( aFlySet );
    if( nWidth > rSz.GetWidth() )
        aFlySet.Put( SwFmtFrmSize( rSz.GetHeightSizeType(), nWidth, rSz.GetHeight() ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
