/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemiter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:24:51 $
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
#ifndef _SFXITEMITER_HXX
#define _SFXITEMITER_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

class SfxPoolItem;
class SfxItemSet;
class SfxItemPool;

class SVL_DLLPUBLIC SfxItemIter
{
    // Item-Feld - Start & Ende
    const SfxItemSet&   _rSet;
    USHORT              _nStt, _nEnd, _nAkt;

public:
    SfxItemIter( const SfxItemSet& rSet );
    ~SfxItemIter();

    // falls es diese gibt, returne sie, sonst 0
    const SfxPoolItem* FirstItem()
                       { _nAkt = _nStt;
                         return _rSet._nCount ? *(_rSet._aItems+_nAkt) : 0; }
    const SfxPoolItem* LastItem()
                       { _nAkt = _nEnd;
                         return _rSet._nCount ? *(_rSet._aItems+_nAkt) : 0; }
    const SfxPoolItem* GetCurItem()
                       { return _rSet._nCount ? *(_rSet._aItems+_nAkt) : 0; }
    const SfxPoolItem* NextItem();
    const SfxPoolItem* PrevItem();

    BOOL               IsAtStart() const { return _nAkt == _nStt; }
    BOOL               IsAtEnd() const   { return _nAkt == _nEnd; }

    USHORT             GetCurPos() const { return _nAkt; }
    USHORT             GetFirstPos() const { return _nStt; }
    USHORT             GetLastPos() const { return _nEnd; }
};

#endif
