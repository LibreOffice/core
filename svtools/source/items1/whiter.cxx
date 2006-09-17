/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: whiter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:08:08 $
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
#include "precompiled_svtools.hxx"
// INCLUDE ---------------------------------------------------------------
#ifndef GCC
#endif

#include "whiter.hxx"
#include "itemset.hxx"

DBG_NAME(SfxWhichIter)

// -----------------------------------------------------------------------

SfxWhichIter::SfxWhichIter( const SfxItemSet& rSet, USHORT nFromWh, USHORT nToWh ):
    pRanges(rSet.GetRanges()),
    pStart(rSet.GetRanges()),
    nOfst(0), nFrom(nFromWh), nTo(nToWh)
{
    DBG_CTOR(SfxWhichIter, 0);
    if ( nFrom > 0 )
        FirstWhich();
}

// -----------------------------------------------------------------------

SfxWhichIter::~SfxWhichIter()
{
    DBG_DTOR(SfxWhichIter, 0);
}

// -----------------------------------------------------------------------

USHORT SfxWhichIter::NextWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    while ( 0 != *pRanges )
    {
        const USHORT nLastWhich = *pRanges + nOfst;
        ++nOfst;
        if (*(pRanges+1) == nLastWhich)
        {
            pRanges += 2;
            nOfst = 0;
        }
        USHORT nWhich = *pRanges + nOfst;
        if ( 0 == nWhich || ( nWhich >= nFrom && nWhich <= nTo ) )
            return nWhich;
    }
    return 0;
}

// -----------------------------------------------------------------------

USHORT  SfxWhichIter::PrevWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    while ( pRanges != pStart || 0 != nOfst )
    {
        if(nOfst)
            --nOfst;
        else {
            pRanges -= 2;
            nOfst = *(pRanges+1) - (*pRanges);
        }
        USHORT nWhich = *pRanges + nOfst;
        if ( nWhich >= nFrom && nWhich <= nTo )
            return nWhich;
    }
    return 0;
}

// -----------------------------------------------------------------------

USHORT SfxWhichIter::FirstWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    pRanges = pStart;
    nOfst = 0;
    if ( *pRanges >= nFrom && *pRanges <= nTo )
        return *pRanges;
    return NextWhich();
}

// -----------------------------------------------------------------------

USHORT SfxWhichIter::LastWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    while(*pRanges)
        ++pRanges;
    nOfst = 0;
    USHORT nWhich = *(pRanges-1);
    if ( nWhich >= nFrom && nWhich <= nTo )
        return nWhich;
    return PrevWhich();
}

