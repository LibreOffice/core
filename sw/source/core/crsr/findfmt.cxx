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
#include <pamtyp.hxx>
#include <pam.hxx>
#include <memory>

namespace sw {

bool FindFormatImpl(SwPaM & rSearchPam,
        const SwFormat& rFormat, SwMoveFnCollection const & fnMove,
        const SwPaM &rRegion, bool bInReadOnly)
{
    bool bFound = false;
    const bool bSrchForward = &fnMove == &fnMoveForward;
    std::unique_ptr<SwPaM> pPam(MakeRegion( fnMove, rRegion ));

    // if at beginning/end then move it out of the node
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetContentNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove.fnNds)( &pPam->GetPoint()->nNode, false ))
        {
            return false;
        }
        SwContentNode *pNd = pPam->GetPoint()->nNode.GetNode().GetContentNode();
        pPam->GetPoint()->nContent.Assign( pNd, bSrchForward ? 0 : pNd->Len() );
    }

    bool bFirst = true;
    SwContentNode* pNode;
    while( nullptr != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly )))
    {
        if ( pNode->GetFormatColl() == &rFormat )
        {
            // if a FormatCollection is found then it is definitely a SwContentNode

            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            // always: incl. start and incl. end
            *rSearchPam.GetPoint() = *pPam->GetPoint();
            rSearchPam.SetMark();
            pNode->MakeEndIndex( &rSearchPam.GetPoint()->nContent );
            rSearchPam.GetMark()->nContent = 0;

            // if backward search, switch point and mark
            if( !bSrchForward )
                rSearchPam.Exchange();

            bFound = true;
            break;
        }
    }
    return bFound;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
