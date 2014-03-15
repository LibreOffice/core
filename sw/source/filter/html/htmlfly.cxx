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

#include "htmlfly.hxx"

#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmturl.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <fmtsrnd.hxx>
#include <fmtinfmt.hxx>
#include <flypos.hxx>
#include <docary.hxx>

#include "doc.hxx"
#include "frmfmt.hxx"

SwHTMLPosFlyFrm::SwHTMLPosFlyFrm( const SwPosFlyFrm& rPosFly,
                                  const SdrObject *pSdrObj,
                                  sal_uInt8 nOutMode ) :
    pFrmFmt( &rPosFly.GetFmt() ),
    pSdrObject( pSdrObj ),
    pNdIdx( new SwNodeIndex( rPosFly.GetNdIndex() ) ),
    nOrdNum( rPosFly.GetOrdNum() ),
    nCntntIdx( 0 ),
    nOutputMode( nOutMode )
{
    const SwFmtAnchor& rAnchor = rPosFly.GetFmt().GetAnchor();
    if ((FLY_AT_CHAR == rAnchor.GetAnchorId()) &&
        HTML_POS_INSIDE == GetOutPos() )
    {
        // Auto-gebundene Rahmen werden ein Zeichen weiter hinten
        // ausgegeben, weil dann die Positionierung mit Netscape
        // uebereinstimmt.
        OSL_ENSURE( rAnchor.GetCntntAnchor(), "Keine Anker-Position?" );
        if( rAnchor.GetCntntAnchor() )
        {
            nCntntIdx = rAnchor.GetCntntAnchor()->nContent.GetIndex();
            sal_Int16 eHoriRel = rPosFly.GetFmt().GetHoriOrient().
                                                GetRelationOrient();
            if( text::RelOrientation::FRAME == eHoriRel || text::RelOrientation::PRINT_AREA == eHoriRel )
            {
                const SwCntntNode *pCNd = pNdIdx->GetNode().GetCntntNode();
                OSL_ENSURE( pCNd, "Kein Content-Node an PaM-Position" );
                if( pCNd && nCntntIdx < pCNd->Len() )
                    nCntntIdx++;
            }
        }
    }
}

bool SwHTMLPosFlyFrm::operator<( const SwHTMLPosFlyFrm& rFrm ) const
{
    if( pNdIdx->GetIndex() == rFrm.pNdIdx->GetIndex() )
    {
        if( nCntntIdx == rFrm.nCntntIdx )
        {
            if( GetOutPos() == rFrm.GetOutPos() )
                return nOrdNum < rFrm.nOrdNum;
            else
                return GetOutPos() < rFrm.GetOutPos();
        }
        else
            return nCntntIdx < rFrm.nCntntIdx;
    }
    else
        return pNdIdx->GetIndex() < rFrm.pNdIdx->GetIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
