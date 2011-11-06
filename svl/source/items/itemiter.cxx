/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#ifndef GCC
#endif

#include <svl/itemiter.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>

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



