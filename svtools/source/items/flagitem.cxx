/*************************************************************************
 *
 *  $RCSfile: flagitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#include "args.hxx"
#include "poolitem.hxx"
#include "flagitem.hxx"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxFlagItem);

USHORT nSfxFlagVal[16] =
{
    0x0001, 0x0002, 0x0004, 0x0008,
    0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800,
    0x1000, 0x2000, 0x4000, 0x8000
};


// -----------------------------------------------------------------------

TYPEINIT1(SfxFlagItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxFlagItem::SfxFlagItem( USHORT nW, USHORT nV ) :
    SfxPoolItem( nW ),
    nVal(nV)
{
    DBG_CTOR(SfxFlagItem, 0);
}

// -----------------------------------------------------------------------

SfxFlagItem::SfxFlagItem( USHORT nW, SvStream &rStream) :
    SfxPoolItem( nW )
{
    DBG_CTOR(SfxFlagItem, 0);
    rStream >> nVal;
}

// -----------------------------------------------------------------------

SfxFlagItem::SfxFlagItem( const SfxFlagItem& rItem ) :
    SfxPoolItem( rItem ),
    nVal( rItem.nVal )
{
    DBG_CTOR(SfxFlagItem, 0);
}

// -----------------------------------------------------------------------

SvStream& SfxFlagItem::Store(SvStream &rStream, USHORT nItemVersion) const
{
    DBG_CHKTHIS(SfxFlagItem, 0);
    rStream << nVal;
    return rStream;
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxFlagItem::GetPresentation
(
    SfxItemPresentation     ePresentation,
    SfxMapUnit              eCoreMetric,
    SfxMapUnit              ePresentationMetric,
    XubString&              rText,
    const International *
)   const
{
    DBG_CHKTHIS(SfxFlagItem, 0);
    rText.Erase();
    for ( int nFlag = 0; nFlag < GetFlagCount(); ++nFlag )
#ifndef ENABLEUNICODE
        rText += String( GetFlag(nFlag) );
#else
        rText += UniString::CreateFromInt32( GetFlag(nFlag) );
#endif
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

XubString SfxFlagItem::GetFlagText( BYTE nV ) const
{
    DBG_CHKTHIS(SfxFlagItem, 0);
    DBG_WARNING( "calling GetValueText(USHORT) on SfxFlagItem -- overload!" );
    return XubString();
}

// -----------------------------------------------------------------------


BYTE SfxFlagItem::GetFlagCount() const
{
    DBG_CHKTHIS(SfxFlagItem, 0);
    DBG_WARNING( "calling GetValueText(USHORT) on SfxFlagItem -- overload!" );
    return 0;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxFlagItem::Create(SvStream &rStream, USHORT nVersion) const
{
    DBG_CHKTHIS(SfxFlagItem, 0);
    DBG_WARNING( "calling Create() on SfxFlagItem -- overload!" );
    return 0;
}

// -----------------------------------------------------------------------

int SfxFlagItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxFlagItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return (((SfxFlagItem&)rItem).nVal == nVal);
}

// -----------------------------------------------------------------------

void SfxFlagItem::SetFlag( BYTE nFlag, int bVal )
{
    if ( bVal )
        nVal |= nSfxFlagVal[nFlag];
    else
        nVal &= ~nSfxFlagVal[nFlag];
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxFlagItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxFlagItem, 0);
    return new SfxFlagItem( *this );
}





