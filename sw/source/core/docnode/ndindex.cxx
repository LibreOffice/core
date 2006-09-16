/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ndindex.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 20:59:54 $
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



#include "errhdl.hxx"           // fuers ASSERT
#include "error.h"              // fuers ASSERT
#include "ndindex.hxx"

#ifndef PRODUCT
int SwNodeIndex::nSerial = 0;
#endif


SwNodeRange::SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE )
    : aStart( rS ), aEnd( rE )
{}

SwNodeRange::SwNodeRange( const SwNodeRange &rRange )
    : aStart( rRange.aStart ), aEnd( rRange.aEnd )
{}

SwNodeRange::SwNodeRange( SwNodes& rNds, ULONG nSttIdx, ULONG nEndIdx )
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


SwNodeIndex::SwNodeIndex( SwNodes& rNds, ULONG nIdx )
    : pNd( rNds[ nIdx ] ), pNext( 0 ), pPrev( 0 )
{
    rNds.RegisterIndex( *this );

#ifndef PRODUCT
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
#ifndef PRODUCT
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
#ifndef PRODUCT
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

SwNodeIndex& SwNodeIndex::Assign( SwNodes& rNds, ULONG nIdx )
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


