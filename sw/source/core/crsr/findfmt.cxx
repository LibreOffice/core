/*************************************************************************
 *
 *  $RCSfile: findfmt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif


FASTBOOL SwPaM::Find( const SwFmt& rFmt, SwMoveFn fnMove,
                        const SwPaM *pRegion, FASTBOOL bInReadOnly  )
{
    FASTBOOL bFound = FALSE;
    FASTBOOL bSrchForward = fnMove == fnMoveForward;
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

    FASTBOOL bFirst = TRUE;
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


