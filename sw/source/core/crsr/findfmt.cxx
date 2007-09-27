/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: findfmt.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:29:44 $
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



#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif


BOOL SwPaM::Find( const SwFmt& rFmt, SwMoveFn fnMove,
                        const SwPaM *pRegion, BOOL bInReadOnly  )
{
    BOOL bFound = FALSE;
    BOOL bSrchForward = fnMove == fnMoveForward;
    SwPaM* pPam = MakeRegion( fnMove, pRegion );

    // Wenn am Anfang/Ende, aus dem Node moven
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetCntntNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, FALSE ))
        {
            delete pPam;
            return FALSE;
        }
        SwCntntNode *pNd = pPam->GetPoint()->nNode.GetNode().GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        pPam->GetPoint()->nContent.Assign( pNd, nTmpPos );
    }

    BOOL bFirst = TRUE;
    SwCntntNode* pNode;
    while( !bFound &&
            0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly )))
    {
        if( 0 != ( bFound = pNode->GetFmtColl() == &rFmt ))
        {
            // wurde die FormatCollection gefunden, dann handelt es sich auf
            // jedenfall um einen SwCntntNode !!

            // FORWARD:  SPoint an das Ende, GetMark zum Anfanf vom Node
            // BACKWARD: SPoint zum Anfang, GetMark an das Ende vom Node
            // und immer nach der Logik: inkl. Start, exkl. End !!!
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            GetMark()->nContent = 0;
            if( !bSrchForward )         // rueckwaerts Suche?
                Exchange();             // SPoint und GetMark tauschen
            break;
        }
    }
    delete pPam;
    return bFound;
}


