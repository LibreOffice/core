/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sitem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:07:01 $
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
// INCLUDE ---------------------------------------------------------------

#ifndef GCC
#endif

#include <tools/string.hxx>
#include <tools/stream.hxx>

#include "poolitem.hxx"
#include "itemset.hxx"


// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxSetItem)

// --------------------------------------------------------------------------

SfxSetItem::SfxSetItem( USHORT which, const SfxItemSet &rSet) :
    SfxPoolItem(which),
    pSet(rSet.Clone(TRUE))
{
    DBG_CTOR(SfxSetItem, 0);
}

// --------------------------------------------------------------------------

SfxSetItem::SfxSetItem( USHORT which, SfxItemSet *pS) :
    SfxPoolItem(which),
    pSet(pS)
{
    DBG_CTOR(SfxSetItem, 0);
    DBG_ASSERT(pS, "SfxSetItem without set constructed" );
}

// --------------------------------------------------------------------------

SfxSetItem::SfxSetItem( const SfxSetItem& rCopy, SfxItemPool *pPool ) :
    SfxPoolItem(rCopy.Which()),
    pSet(rCopy.pSet->Clone(TRUE, pPool))
{
    DBG_CTOR(SfxSetItem, 0);
}

// --------------------------------------------------------------------------

SfxSetItem::~SfxSetItem()
{
    DBG_DTOR(SfxSetItem, 0);
    delete pSet; pSet = 0;
}

// --------------------------------------------------------------------------

int SfxSetItem::operator==( const SfxPoolItem& rCmp) const
{
    DBG_CHKTHIS(SfxSetItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rCmp ), "unequal type" );
    return *pSet == *(((const SfxSetItem &)rCmp).pSet);
}

// --------------------------------------------------------------------------

SfxItemPresentation SfxSetItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              /*rText*/,
    const IntlWrapper *
)   const
{
    DBG_CHKTHIS(SfxSetItem, 0);
    return SFX_ITEM_PRESENTATION_NONE;
}

// --------------------------------------------------------------------------

SvStream& SfxSetItem::Store(SvStream& rStream, USHORT) const
{
    GetItemSet().Store(rStream);
    return rStream;
}

