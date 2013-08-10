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

#include "ndindex.hxx"

SwNodeRange::SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE )
    : aStart( rS ), aEnd( rE )
{}

SwNodeRange::SwNodeRange( const SwNodeRange &rRange )
    : aStart( rRange.aStart ), aEnd( rRange.aEnd )
{}

SwNodeRange::SwNodeRange( SwNodes& rNds, sal_uLong nSttIdx, sal_uLong nEndIdx )
    : aStart( rNds, nSttIdx ), aEnd( rNds, nEndIdx )
{}

SwNodeRange::SwNodeRange( const SwNodeIndex& rS, long nSttDiff,
                          const SwNodeIndex& rE, long nEndDiff )
    : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff )
{}

SwNodeRange::SwNodeRange( const SwNode& rS, long nSttDiff,
                          const SwNode& rE, long nEndDiff )
    : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff )
{}

SwNodeIndex::SwNodeIndex( SwNodes& rNds, sal_uLong nIdx )
    : pNd( rNds[ nIdx ] ), pNext( 0 ), pPrev( 0 )
{
    rNds.RegisterIndex( *this );
}

SwNodeIndex::SwNodeIndex( const SwNodeIndex& rIdx, long nDiff )
    : pNext( 0 ), pPrev( 0 )
{
    if( nDiff )
        pNd = rIdx.GetNodes()[ rIdx.GetIndex() + nDiff ];
    else
        pNd = rIdx.pNd;

    pNd->GetNodes().RegisterIndex( *this );
}

SwNodeIndex::SwNodeIndex( const SwNode& rNd, long nDiff )
    : pNext( 0 ), pPrev( 0 )
{
    if( nDiff )
        pNd = rNd.GetNodes()[ rNd.GetIndex() + nDiff ];
    else
        pNd = (SwNode*)&rNd;

    pNd->GetNodes().RegisterIndex( *this );
}

void SwNodeIndex::Remove()
{
    pNd->GetNodes().DeRegisterIndex( *this );
}

SwNodeIndex& SwNodeIndex::operator=( const SwNodeIndex& rIdx )
{
    if( &pNd->GetNodes() != &rIdx.pNd->GetNodes() )
    {
        pNd->GetNodes().DeRegisterIndex( *this );
        pNd = rIdx.pNd;
        pNd->GetNodes().RegisterIndex( *this );
    }
    else
        pNd = rIdx.pNd;
    return *this;
}

SwNodeIndex& SwNodeIndex::operator=( const SwNode& rNd )
{
    if( &pNd->GetNodes() != &rNd.GetNodes() )
    {
        pNd->GetNodes().DeRegisterIndex( *this );
        pNd = (SwNode*)&rNd;
        pNd->GetNodes().RegisterIndex( *this );
    }
    else
        pNd = (SwNode*)&rNd;
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( SwNodes& rNds, sal_uLong nIdx )
{
    if( &pNd->GetNodes() != &rNds )
    {
        pNd->GetNodes().DeRegisterIndex( *this );
        pNd = rNds[ nIdx ];
        pNd->GetNodes().RegisterIndex( *this );
    }
    else
        pNd = rNds[ nIdx ];
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( const SwNode& rNd, long nOffset )
{
    if( &pNd->GetNodes() != &rNd.GetNodes() )
    {
        pNd->GetNodes().DeRegisterIndex( *this );
        pNd = (SwNode*)&rNd;
        pNd->GetNodes().RegisterIndex( *this );
    }
    else
        pNd = (SwNode*)&rNd;

    if( nOffset )
        pNd = pNd->GetNodes()[ pNd->GetIndex() + nOffset ];

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
