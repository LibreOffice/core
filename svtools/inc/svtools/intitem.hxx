/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:24:30 $
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

#ifndef _SFXINTITEM_HXX
#define _SFXINTITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SVTOOLS_CINTITEM_HXX
#include <svtools/cintitem.hxx>
#endif

//============================================================================
class SVT_DLLPUBLIC SfxByteItem: public CntByteItem
{
public:
    TYPEINFO();

    SfxByteItem(USHORT which = 0, BYTE nValue = 0):
        CntByteItem(which, nValue) {}

    SfxByteItem(USHORT which, SvStream & rStream):
        CntByteItem(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxByteItem(*this); }
};

//============================================================================
DBG_NAMEEX_VISIBILITY(SfxInt16Item, SVT_DLLPUBLIC)

class SVT_DLLPUBLIC SfxInt16Item: public SfxPoolItem
{
    INT16 m_nValue;

public:
    TYPEINFO();

    SfxInt16Item(USHORT which = 0, INT16 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(SfxInt16Item, 0); }

    SfxInt16Item(USHORT nWhich, SvStream & rStream);

    SfxInt16Item(const SfxInt16Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(SfxInt16Item, 0); }

    virtual ~SfxInt16Item() { DBG_DTOR(SfxInt16Item, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual BOOL QueryValue( com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;

    virtual BOOL PutValue( const com::sun::star::uno::Any& rVal,
                           BYTE nMemberId = 0 );

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual INT16 GetMin() const;

    virtual INT16 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    INT16 GetValue() const { return m_nValue; }

    inline void SetValue(INT16 nTheValue);
};

inline void SfxInt16Item::SetValue(INT16 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "SfxInt16Item::SetValue(); Pooled item");
    m_nValue = nTheValue;
}

//============================================================================
class SVT_DLLPUBLIC SfxUInt16Item: public CntUInt16Item
{
public:
    TYPEINFO();

    SfxUInt16Item(USHORT which = 0, UINT16 nValue = 0):
        CntUInt16Item(which, nValue) {}

    SfxUInt16Item(USHORT which, SvStream & rStream):
        CntUInt16Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const
    { return new SfxUInt16Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxUInt16Item(*this); }
};

//============================================================================
class SVT_DLLPUBLIC SfxInt32Item: public CntInt32Item
{
public:
    TYPEINFO();

    SfxInt32Item(USHORT which = 0, INT32 nValue = 0):
        CntInt32Item(which, nValue) {}

    SfxInt32Item(USHORT which, SvStream & rStream):
        CntInt32Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const
    { return new SfxInt32Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxInt32Item(*this); }

};

//============================================================================
class SVT_DLLPUBLIC SfxUInt32Item: public CntUInt32Item
{
public:
    TYPEINFO();

    SfxUInt32Item(USHORT which = 0, UINT32 nValue = 0):
        CntUInt32Item(which, nValue) {}

    SfxUInt32Item(USHORT which, SvStream & rStream):
        CntUInt32Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const
    { return new SfxUInt32Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxUInt32Item(*this); }
};

#endif // _SFXINTITEM_HXX

