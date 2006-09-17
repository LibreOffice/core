/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stritem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:01:23 $
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

#ifndef _SFXSTRITEM_HXX
#include <stritem.hxx>
#endif

//============================================================================
//
//  class SfxStringItem
//
//============================================================================

TYPEINIT1_AUTOFACTORY(SfxStringItem, CntUnencodedStringItem)

//============================================================================
// virtual
SfxStringItem::SfxStringItem(USHORT which, SvStream & rStream):
    CntUnencodedStringItem(which)
{
    UniString aValue;
    readByteString(rStream, aValue);
    SetValue(aValue);
}


//============================================================================
// virtual
SfxPoolItem * SfxStringItem::Create(SvStream & rStream, USHORT) const
{
    return new SfxStringItem(Which(), rStream);
}

//============================================================================
// virtual
SvStream & SfxStringItem::Store(SvStream & rStream, USHORT) const
{
    writeByteString(rStream, GetValue());
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * SfxStringItem::Clone(SfxItemPool *) const
{
    return new SfxStringItem(*this);
}

