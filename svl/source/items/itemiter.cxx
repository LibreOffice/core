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


#include <svl/itemiter.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>

SfxItemIter::SfxItemIter( const SfxItemSet& rItemSet )
    : _rSet( rItemSet )
{
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




SfxItemIter::~SfxItemIter()
{
}




const SfxPoolItem* SfxItemIter::NextItem()
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
