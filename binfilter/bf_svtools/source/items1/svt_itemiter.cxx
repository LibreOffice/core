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


#ifndef GCC
#endif

#include <bf_svtools/itemiter.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_svtools/itemset.hxx>

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemIter)

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
            for( _nEnd = _rSet.TotalCount(); !*( ppFnd + --_nEnd);	)
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

}
