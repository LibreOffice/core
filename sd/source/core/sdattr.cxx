/*************************************************************************
 *
 *  $RCSfile: sdattr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:24:14 $
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

#pragma hdrstop

#include "sdattr.hxx"

using namespace ::com::sun::star;

/*************************************************************************
|*
|*  DiaEffectItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaEffectItem, SfxEnumItem );


DiaEffectItem::DiaEffectItem( presentation::FadeEffect eFE ) :
    SfxEnumItem( ATTR_DIA_EFFECT, eFE )
{
}


DiaEffectItem::DiaEffectItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_EFFECT, rIn )
{
}


SfxPoolItem* DiaEffectItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaEffectItem( *this );
}


SfxPoolItem* DiaEffectItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new DiaEffectItem( rIn );
}

/*************************************************************************
|*
|*  DiaSpeedItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaSpeedItem, SfxEnumItem );


DiaSpeedItem::DiaSpeedItem( FadeSpeed eFS ) :
    SfxEnumItem( ATTR_DIA_SPEED, eFS )
{
}


DiaSpeedItem::DiaSpeedItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_SPEED, rIn )
{
}


SfxPoolItem* DiaSpeedItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaSpeedItem( *this );
}


SfxPoolItem* DiaSpeedItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new DiaSpeedItem( rIn );
}

/*************************************************************************
|*
|*  DiaAutoItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaAutoItem, SfxEnumItem );

DiaAutoItem::DiaAutoItem( PresChange eChange ) :
    SfxEnumItem( ATTR_DIA_AUTO, eChange )
{
}


DiaAutoItem::DiaAutoItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_AUTO, rIn )
{
}


SfxPoolItem* DiaAutoItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaAutoItem( *this );
}


SfxPoolItem* DiaAutoItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new DiaAutoItem( rIn );
}

/*************************************************************************
|*
|*  DiaTimeItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaTimeItem, SfxUInt32Item );


DiaTimeItem::DiaTimeItem( UINT32 nValue ) :
        SfxUInt32Item( ATTR_DIA_TIME, nValue )
{
}


SfxPoolItem* DiaTimeItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaTimeItem( *this );
}


int DiaTimeItem::operator==( const SfxPoolItem& rItem ) const
{
    return( ( (DiaTimeItem&) rItem ).GetValue() == GetValue() );
}



