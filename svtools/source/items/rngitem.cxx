/*************************************************************************
 *
 *  $RCSfile: rngitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:01 $
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
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#pragma hdrstop

#ifndef NUMTYPE

#include "rngitem.hxx"
#include "sbx.hxx"
#include "args.hxx"

#if SUPD<355
DBG_NAME(SfxRangeItem);
#endif


#define NUMTYPE USHORT
#define SfxXRangeItem SfxRangeItem
#define SfxXRangesItem SfxUShortRangesItem
#include "rngitem.cxx"

#define NUMTYPE ULONG
#define SfxXRangeItem SfxULongRangeItem
#define SfxXRangesItem SfxULongRangesItem
#include "rngitem.cxx"

#else

static inline NUMTYPE Count_Impl(const NUMTYPE * pRanges)
{
    NUMTYPE nCount = 0;
    for (; *pRanges; pRanges += 2) nCount += 2;
    return nCount;
}

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxXRangeItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SfxXRangesItem, SfxPoolItem);

NUMTYPE Count_Impl( const NUMTYPE *pRanges );

// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem()
{
    nFrom = 0;
    nTo = 0;
}

// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem( USHORT nWhich, NUMTYPE from, NUMTYPE to ):
    SfxPoolItem( nWhich ),
    nFrom( from ),
    nTo( to )
{
}


// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem( USHORT nW, SvStream &rStream ) :
    SfxPoolItem( nW )
{
    rStream >> nFrom;
    rStream >> nTo;
}

// -----------------------------------------------------------------------

SfxXRangeItem::SfxXRangeItem( const SfxXRangeItem& rItem ) :
    SfxPoolItem( rItem )
{
    nFrom = rItem.nFrom;
    nTo = rItem.nTo;
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxXRangeItem::GetPresentation
(
    SfxItemPresentation     ePresentation,
    SfxMapUnit              eCoreMetric,
    SfxMapUnit              ePresentationMetric,
    XubString&              rText,
    const International *
)   const
{
    rText = UniString::CreateFromInt64(nFrom);
    rText += ':';
    rText += UniString::CreateFromInt64(nTo);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxXRangeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    SfxXRangeItem* pT = (SfxXRangeItem*)&rItem;
    if( nFrom==pT->nFrom && nTo==pT->nTo )
        return 1;
    return 0;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxXRangeItem::Clone(SfxItemPool *) const
{
    return new SfxXRangeItem( Which(), nFrom, nTo );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxXRangeItem::Create(SvStream &rStream, USHORT nVersion ) const
{
    NUMTYPE     nVon, nBis;
    rStream >> nVon;
    rStream >> nBis;
    return new SfxXRangeItem( Which(), nVon, nBis );
}

// -----------------------------------------------------------------------

SvStream& SfxXRangeItem::Store(SvStream &rStream, USHORT nItemVersion) const
{
    rStream << nFrom;
    rStream << nTo;
    return rStream;
}

//=========================================================================

SfxXRangesItem::SfxXRangesItem()
:   _pRanges(0)
{
}

//-------------------------------------------------------------------------

SfxXRangesItem::SfxXRangesItem( USHORT nWID, const NUMTYPE *pRanges )
:   SfxPoolItem( nWID )
{
    NUMTYPE nCount = Count_Impl(pRanges) + 1;
    _pRanges = new NUMTYPE[nCount];
    memcpy( _pRanges, pRanges, sizeof(NUMTYPE) * nCount );
}

//-------------------------------------------------------------------------

SfxXRangesItem::SfxXRangesItem( USHORT nWID, SvStream &rStream )
:   SfxPoolItem( nWID )
{
    NUMTYPE nCount;
    rStream >> nCount;
    _pRanges = new NUMTYPE[nCount + 1];
    for ( NUMTYPE n = 0; n < nCount; ++n )
        rStream >> _pRanges[n];
    _pRanges[nCount] = 0;
}

//-------------------------------------------------------------------------

SfxXRangesItem::SfxXRangesItem( const SfxXRangesItem& rItem )
:   SfxPoolItem( rItem )
{
    NUMTYPE nCount = Count_Impl(rItem._pRanges) + 1;
    _pRanges = new NUMTYPE[nCount];
    memcpy( _pRanges, rItem._pRanges, sizeof(NUMTYPE) * nCount );
}

//-------------------------------------------------------------------------

SfxXRangesItem::~SfxXRangesItem()
{
    delete _pRanges;
}

//-------------------------------------------------------------------------

int SfxXRangesItem::operator==( const SfxPoolItem &rItem ) const
{
    const SfxXRangesItem &rOther = (const SfxXRangesItem&) rItem;
    if ( !_pRanges && !rOther._pRanges )
        return TRUE;
    if ( _pRanges || rOther._pRanges )
        return FALSE;

    NUMTYPE n;
    for ( n = 0; _pRanges[n] && rOther._pRanges[n]; ++n )
        if ( *_pRanges != rOther._pRanges[n] )
            return 0;

    return !_pRanges[n] && !rOther._pRanges[n];
}

//-------------------------------------------------------------------------

SfxItemPresentation SfxXRangesItem::GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const International * ) const
{
    HACK(n. i.)
    return SFX_ITEM_PRESENTATION_NONE;
}

//-------------------------------------------------------------------------

SfxPoolItem* SfxXRangesItem::Clone( SfxItemPool *pPool ) const
{
    return new SfxXRangesItem( *this );
}

//-------------------------------------------------------------------------

SfxPoolItem* SfxXRangesItem::Create( SvStream &rStream, USHORT nVersion ) const
{
    return new SfxXRangesItem( Which(), rStream );
}

//-------------------------------------------------------------------------

SvStream& SfxXRangesItem::Store( SvStream &rStream, USHORT nItemVersion ) const
{
    NUMTYPE nCount = Count_Impl( _pRanges );
    rStream >> nCount;
    for ( NUMTYPE n = 0; _pRanges[n]; ++n )
        rStream >> _pRanges[n];
    return rStream;
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.24  2000/09/18 14:13:35  willem.vandorp
    OpenOffice header added.

    Revision 1.23  2000/08/31 13:41:28  willem.vandorp
    Header and footer replaced

    Revision 1.22  2000/05/26 16:38:31  os
    Set/FillVariable removed

    Revision 1.21  2000/04/12 08:01:39  sb
    Adapted to Unicode.

    Revision 1.20  2000/02/09 16:23:50  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.19  1999/09/27 12:16:04  hr
    for-scope

    Revision 1.18  1999/09/08 15:35:16  sb
    #66082# Copied Count_Impl from nranges.cxx.

    Revision 1.17  1999/09/01 06:52:36  sb
    #66082# Added International param to SfxPoolItem::GetPresentation().

    Revision 1.16  1998/01/09 11:17:06  MH
    chg: header


      Rev 1.15   09 Jan 1998 12:17:06   MH
   chg: header

      Rev 1.14   05 Mar 1997 13:44:56   MI
   falsche Var verwendet daher op== falsch

      Rev 1.13   03 Feb 1997 10:00:14   MI
   Typeinfo fuer Ranges

      Rev 1.12   03 Feb 1997 09:13:22   MI
   NUMTYPE durch USHORT ersetzt

      Rev 1.11   02 Feb 1997 16:46:32   MI
   compilebar

      Rev 1.10   02 Feb 1997 13:29:24   MI
   jetzt auch Sfx..RangesItem (mehrere Ranges)

      Rev 1.9   02 Feb 1997 12:23:56   MI
   jetzt auch LongRange

      Rev 1.8   14 Nov 1996 18:49:12   MI
   FileFirmat 4.0

      Rev 1.7   14 Mar 1996 17:40:46   MH
   del: S_VERSION

      Rev 1.6   12 Mar 1996 12:49:42   OV
   ein fehlendes DBG_CTOR eingefuegt

      Rev 1.5   21 Nov 1995 15:50:52   MI
   FillVariable fuer strukturierte Items

      Rev 1.4   20 Nov 1995 15:44:20   AB
   Unicode-Anpassung

      Rev 1.3   15 Nov 1995 20:16:36   MI
   GetVariable/SetVariable

      Rev 1.2   05 Jul 1995 18:11:38   MI
   GetValueText raus #pragmas rein

      Rev 1.1   29 Jun 1995 17:16:30   MI
   GetPresentation impl.

      Rev 1.0   22 Feb 1995 16:26:28   OV
   Initial revision.

------------------------------------------------------------------------*/
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem


#endif

