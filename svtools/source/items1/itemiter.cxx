/*************************************************************************
 *
 *  $RCSfile: itemiter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:16:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef GCC
#pragma hdrstop
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



