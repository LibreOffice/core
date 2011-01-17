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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "errhdl.hxx"           // fuers ASSERT
#include "error.h"              // fuers ASSERT
#include "ndindex.hxx"

#ifdef DBG_UTIL
int SwNodeIndex::nSerial = 0;
#endif


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

#ifdef DBG_UTIL
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
}


SwNodeIndex::SwNodeIndex( const SwNodeIndex& rIdx, long nDiff )
    : pNext( 0 ), pPrev( 0 )
{
    if( nDiff )
        pNd = rIdx.GetNodes()[ rIdx.GetIndex() + nDiff ];
    else
        pNd = rIdx.pNd;

    pNd->GetNodes().RegisterIndex( *this );
#ifdef DBG_UTIL
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
}


SwNodeIndex::SwNodeIndex( const SwNode& rNd, long nDiff )
    : pNext( 0 ), pPrev( 0 )
{
    if( nDiff )
        pNd = rNd.GetNodes()[ rNd.GetIndex() + nDiff ];
    else
        pNd = (SwNode*)&rNd;

    pNd->GetNodes().RegisterIndex( *this );
#ifdef DBG_UTIL
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
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


