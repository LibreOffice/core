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

SwNodeIndex& SwNodeIndex::operator=( const SwNodeIndex& rIdx )
{
    if( &pNd->GetNodes() != &rIdx.pNd->GetNodes() )
    {
        DeRegisterIndex( pNd->GetNodes() );
        pNd = rIdx.pNd;
        RegisterIndex( pNd->GetNodes() );
    }
    else
        pNd = rIdx.pNd;
    return *this;
}

SwNodeIndex& SwNodeIndex::operator=( const SwNode& rNd )
{
    if( &pNd->GetNodes() != &rNd.GetNodes() )
    {
        DeRegisterIndex( pNd->GetNodes() );
        pNd = (SwNode*)&rNd;
        RegisterIndex( pNd->GetNodes() );
    }
    else
        pNd = (SwNode*)&rNd;
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( SwNodes& rNds, sal_uLong nIdx )
{
    if( &pNd->GetNodes() != &rNds )
    {
        DeRegisterIndex( pNd->GetNodes() );
        pNd = rNds[ nIdx ];
        RegisterIndex( pNd->GetNodes() );
    }
    else
        pNd = rNds[ nIdx ];
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( const SwNode& rNd, long nOffset )
{
    if( &pNd->GetNodes() != &rNd.GetNodes() )
    {
        DeRegisterIndex( pNd->GetNodes() );
        pNd = (SwNode*)&rNd;
        RegisterIndex( pNd->GetNodes() );
    }
    else
        pNd = (SwNode*)&rNd;

    if( nOffset )
        pNd = pNd->GetNodes()[ pNd->GetIndex() + nOffset ];

    return *this;
}

std::ostream &operator <<(std::ostream& s, const SwNodeIndex& index)
{
    return s << "SwNodeIndex (node " << index.GetIndex() << ")";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
