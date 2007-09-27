/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swwrtshitem.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:38:42 $
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
#include "precompiled_sw.hxx"
#include "swwrtshitem.hxx"
TYPEINIT1(SwWrtShellItem,SfxPoolItem);
SwWrtShellItem::SwWrtShellItem( USHORT _nWhich, SwWrtShell* pSh )
    : SfxPoolItem( _nWhich ), pWrtSh( pSh )
{

}
SwWrtShellItem::SwWrtShellItem( const SwWrtShellItem& rItem) :
    SfxPoolItem( rItem.Which() ),
    pWrtSh( rItem.pWrtSh )
{
}

int SwWrtShellItem::operator==( const SfxPoolItem& rItem) const
{
    return ((SwWrtShellItem&)rItem).pWrtSh == pWrtSh;
}

SfxPoolItem* SwWrtShellItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwWrtShellItem( *this );
}
