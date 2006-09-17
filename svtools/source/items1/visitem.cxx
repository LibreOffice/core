/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: visitem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:07:54 $
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

#ifndef _SFXVISIBILITYITEM_HXX
#include <visitem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

//============================================================================
//
//  class SfxVisibilityItem
//
//============================================================================

DBG_NAME(SfxVisibilityItem)

//============================================================================
TYPEINIT1_AUTOFACTORY(SfxVisibilityItem, SfxPoolItem);

//============================================================================
SfxVisibilityItem::SfxVisibilityItem(USHORT which, SvStream & rStream):
    SfxPoolItem(which)
{
    DBG_CTOR(SfxVisibilityItem, 0);
    sal_Bool bValue = 0;
    rStream >> bValue;
    m_nValue.bVisible = bValue;
}

//============================================================================
// virtual
int SfxVisibilityItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    DBG_ASSERT(SfxPoolItem::operator ==(rItem), "unequal type");
    return m_nValue.bVisible == SAL_STATIC_CAST(const SfxVisibilityItem *, &rItem)->
                        m_nValue.bVisible;
}

//============================================================================
// virtual
int SfxVisibilityItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_ASSERT(rWith.ISA(SfxVisibilityItem), "SfxVisibilityItem::Compare(): Bad type");
    return m_nValue.bVisible == static_cast< SfxVisibilityItem const * >(&rWith)->m_nValue.bVisible ?
               0 : m_nValue.bVisible ? -1 : 1;
}

//============================================================================
// virtual
SfxItemPresentation SfxVisibilityItem::GetPresentation(SfxItemPresentation,
                                                  SfxMapUnit, SfxMapUnit,
                                                  XubString & rText,
                                                  const IntlWrapper *) const
{
    rText = GetValueTextByVal(m_nValue.bVisible);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}


//============================================================================
// virtual
BOOL SfxVisibilityItem::QueryValue(com::sun::star::uno::Any& rVal,BYTE) const
{
    rVal <<= m_nValue;
    return TRUE;
}

//============================================================================
// virtual
BOOL SfxVisibilityItem::PutValue(const com::sun::star::uno::Any& rVal,BYTE)
{
    if (rVal >>= m_nValue)
        return TRUE;

    DBG_ERROR( "SfxInt16Item::PutValue - Wrong type!" );
    return FALSE;
}

//============================================================================
// virtual
SfxPoolItem * SfxVisibilityItem::Create(SvStream & rStream, USHORT) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    return new SfxVisibilityItem(Which(), rStream);
}

//============================================================================
// virtual
SvStream & SfxVisibilityItem::Store(SvStream & rStream, USHORT) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    rStream << m_nValue.bVisible;
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * SfxVisibilityItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxVisibilityItem, 0);
    return new SfxVisibilityItem(*this);
}

//============================================================================
// virtual
USHORT SfxVisibilityItem::GetValueCount() const
{
    return 2;
}

//============================================================================
// virtual
UniString SfxVisibilityItem::GetValueTextByVal(BOOL bTheValue) const
{
    return
        bTheValue ?
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TRUE")) :
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("FALSE"));
}
