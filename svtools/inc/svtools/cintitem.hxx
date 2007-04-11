/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cintitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:13:12 $
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

#ifndef _SVTOOLS_CINTITEM_HXX
#define _SVTOOLS_CINTITEM_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

//============================================================================
DBG_NAMEEX_VISIBILITY(CntByteItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntByteItem: public SfxPoolItem
{
    BYTE m_nValue;

public:
    TYPEINFO();

    CntByteItem(USHORT which = 0, BYTE nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue) { DBG_CTOR(CntByteItem, 0); }

    CntByteItem(USHORT which, SvStream & rStream);

    CntByteItem(const CntByteItem & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntByteItem, 0); }

    virtual ~CntByteItem() { DBG_DTOR(CntByteItem, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual BOOL QueryValue(com::sun::star::uno::Any& rVal,
                            BYTE nMemberId = 0) const;

    virtual BOOL PutValue(const com::sun::star::uno::Any& rVal,
                          BYTE nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual BYTE GetMin() const;

    virtual BYTE GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    BYTE GetValue() const { return m_nValue; }

    inline void SetValue(BYTE nTheValue);
};

inline void CntByteItem::SetValue(BYTE nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntByteItem::SetValue(): Pooled item");
    m_nValue = nTheValue;
}

//============================================================================
DBG_NAMEEX_VISIBILITY(CntUInt16Item, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntUInt16Item: public SfxPoolItem
{
    UINT16 m_nValue;

public:
    TYPEINFO();

    CntUInt16Item(USHORT which = 0, UINT16 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(CntUInt16Item, 0); }

    CntUInt16Item(USHORT which, SvStream & rStream);

    CntUInt16Item(const CntUInt16Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntUInt16Item, 0); }

    virtual ~CntUInt16Item() { DBG_DTOR(CntUInt16Item, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual BOOL QueryValue(com::sun::star::uno::Any& rVal,
                            BYTE nMemberId = 0) const;

    virtual BOOL PutValue(const com::sun::star::uno::Any& rVal,
                          BYTE nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual UINT16 GetMin() const;

    virtual UINT16 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    INT16 GetValue() const { return m_nValue; }

    inline void SetValue(UINT16 nTheValue);
};

inline void CntUInt16Item::SetValue(UINT16 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntUInt16Item::SetValue(): Pooled item");
    m_nValue = nTheValue;
}

//============================================================================
DBG_NAMEEX_VISIBILITY(CntInt32Item, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntInt32Item: public SfxPoolItem
{
    INT32 m_nValue;

public:
    TYPEINFO();

    CntInt32Item(USHORT which = 0, INT32 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(CntInt32Item, 0); }

    CntInt32Item(USHORT which, SvStream & rStream);

    CntInt32Item(const CntInt32Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntInt32Item, 0); }

    virtual ~CntInt32Item() { DBG_DTOR(CntInt32Item, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual BOOL QueryValue(com::sun::star::uno::Any& rVal,
                            BYTE nMemberId = 0) const;

    virtual BOOL PutValue(const com::sun::star::uno::Any& rVal,
                          BYTE nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream &, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual INT32 GetMin() const;

    virtual INT32 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    INT32 GetValue() const { return m_nValue; }

    inline void SetValue(INT32 nTheValue);
};

inline void CntInt32Item::SetValue(INT32 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntInt32Item::SetValue(): Pooled item");
    m_nValue = nTheValue;
}

//============================================================================
DBG_NAMEEX_VISIBILITY(CntUInt32Item, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntUInt32Item: public SfxPoolItem
{
    UINT32 m_nValue;

public:
    TYPEINFO();

    CntUInt32Item(USHORT which = 0, UINT32 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(CntUInt32Item, 0); }

    CntUInt32Item(USHORT nWhich, SvStream & rStream);

    CntUInt32Item(const CntUInt32Item & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(CntUInt32Item, 0); }

    virtual ~CntUInt32Item() { DBG_DTOR(CntUInt32Item, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual BOOL QueryValue(com::sun::star::uno::Any& rVal,
                            BYTE nMemberId = 0) const;

    virtual BOOL PutValue(const com::sun::star::uno::Any& rVal,
                          BYTE nMemberId = 0);

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual UINT32 GetMin() const;

    virtual UINT32 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    UINT32 GetValue() const { return m_nValue; }

    inline void SetValue(UINT32 nTheValue);
};

inline void CntUInt32Item::SetValue(UINT32 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "CntUInt32Item::SetValue(): Pooled item");
    m_nValue = nTheValue;
}

#endif // _SVTOOLS_CINTITEM_HXX

