/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <doc.hxx>
#include <pamtyp.hxx>


sal_Bool SwPaM::Find( const SwFmt& rFmt, SwMoveFn fnMove,
                        const SwPaM *pRegion, sal_Bool bInReadOnly  )
{
    sal_Bool bFound = sal_False;
    sal_Bool bSrchForward = fnMove == fnMoveForward;
    SwPaM* pPam = MakeRegion( fnMove, pRegion );

    // if at beginning/end then move it out of the node
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetCntntNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, sal_False ))
        {
            delete pPam;
            return sal_False;
        }
        SwCntntNode *pNd = pPam->GetPoint()->nNode.GetNode().GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        pPam->GetPoint()->nContent.Assign( pNd, nTmpPos );
    }

    sal_Bool bFirst = sal_True;
    SwCntntNode* pNode;
    while( !bFound &&
            0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly )))
    {
        if( 0 != ( bFound = (pNode->GetFmtColl() == &rFmt) ))
        {
            // if a FormatCollection is found then it is definitely a SwCntntNode

            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            // always: incl. start and incl. end
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            GetMark()->nContent = 0;

            // if backward search, switch point and mark
            if( !bSrchForward )
                Exchange();
            break;
        }
    }
    delete pPam;
    return bFound;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
