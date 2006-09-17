/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemiter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:04:59 $
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

#ifndef GCC
#endif

#include "itemiter.hxx"
#include "itempool.hxx"
#include "itemset.hxx"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemIter);


// --------------------------------------------------------------------------


SfxItemIter::SfxItemIter( const SfxItemSet& rItemSet )
    : _rSet( rItemSet )
{
    DBG_CTOR(SfxItemIter, 0);
    DBG_ASSERTWARNING( _rSet.Count(), "es gibt gar keine Attribute" );

    if ( !_rSet._nCount )
    {
        _nStt = 1;
        _nEnd = 0;
    }
    else
    {
        SfxItemArray ppFnd = _rSet._aItems;

        // suche das 1. gesetzte Item
        for ( _nStt = 0; !*(ppFnd + _nStt ); ++_nStt )
            ; // empty loop
        if ( 1 < _rSet.Count() )
            for( _nEnd = _rSet.TotalCount(); !*( ppFnd + --_nEnd);  )
                ; // empty loop
        else
            _nEnd = _nStt;
    }

    _nAkt = _nStt;
}

// --------------------------------------------------------------------------


SfxItemIter::~SfxItemIter()
{
    DBG_DTOR(SfxItemIter, 0);
}

// --------------------------------------------------------------------------


const SfxPoolItem* SfxItemIter::NextItem()
{
    DBG_CHKTHIS(SfxItemIter, 0);
    SfxItemArray ppFnd = _rSet._aItems;

    if( _nAkt < _nEnd )
    {
        do {
            _nAkt++;
        } while( _nAkt < _nEnd && !*(ppFnd + _nAkt ) );
        return *(ppFnd+_nAkt);
    }
    return 0;
}

// --------------------------------------------------------------------------


const SfxPoolItem* SfxItemIter::PrevItem()
{
    DBG_CHKTHIS(SfxItemIter, 0);
    SfxItemArray ppFnd = _rSet._aItems;

    if ( _nAkt > _nStt )
    {
        do {
            --_nAkt;
        } while( _nAkt && !*(ppFnd + _nAkt ));
        return *(ppFnd+_nAkt);
    }
    return 0;
}



