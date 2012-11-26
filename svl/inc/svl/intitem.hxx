/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SFXINTITEM_HXX
#define _SFXINTITEM_HXX

#include "svl/svldllapi.h"
#include <svl/cintitem.hxx>

//============================================================================
class SVL_DLLPUBLIC SfxByteItem: public CntByteItem
{
public:
    POOLITEM_FACTORY()
    SfxByteItem(sal_uInt16 which = 0, sal_uInt8 nValue = 0):
        CntByteItem(which, nValue) {}

    SfxByteItem(sal_uInt16 which, SvStream & rStream):
        CntByteItem(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxByteItem(*this); }
};

//============================================================================
DBG_NAMEEX_VISIBILITY(SfxInt16Item, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC SfxInt16Item: public SfxPoolItem
{
    sal_Int16 m_nValue;

public:
    POOLITEM_FACTORY()
    SfxInt16Item(sal_uInt16 which = 0, sal_Int16 nTheValue = 0):
        SfxPoolItem(which), m_nValue(nTheValue)
    { DBG_CTOR(SfxInt16Item, 0); }

    SfxInt16Item(sal_uInt16 nWhich, SvStream & rStream);

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

    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const;

    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal,
                           sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const;

    virtual SvStream & Store(SvStream & rStream, sal_uInt16) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual sal_Int16 GetMin() const;

    virtual sal_Int16 GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    sal_Int16 GetValue() const { return m_nValue; }

    inline void SetValue(sal_Int16 nTheValue);
};

inline void SfxInt16Item::SetValue(sal_Int16 nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "SfxInt16Item::SetValue(); Pooled item");
    m_nValue = nTheValue;
}

//============================================================================
class SVL_DLLPUBLIC SfxUInt16Item: public CntUInt16Item
{
public:
    POOLITEM_FACTORY()
    SfxUInt16Item(sal_uInt16 which = 0, sal_uInt16 nValue = 0):
        CntUInt16Item(which, nValue) {}

    SfxUInt16Item(sal_uInt16 which, SvStream & rStream):
        CntUInt16Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
    { return new SfxUInt16Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxUInt16Item(*this); }
};

//============================================================================
class SVL_DLLPUBLIC SfxInt32Item: public CntInt32Item
{
public:
    POOLITEM_FACTORY()
    SfxInt32Item(sal_uInt16 which = 0, sal_Int32 nValue = 0):
        CntInt32Item(which, nValue) {}

    SfxInt32Item(sal_uInt16 which, SvStream & rStream):
        CntInt32Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
    { return new SfxInt32Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxInt32Item(*this); }

};

//============================================================================
class SVL_DLLPUBLIC SfxUInt32Item: public CntUInt32Item
{
public:
    POOLITEM_FACTORY()
    SfxUInt32Item(sal_uInt16 which = 0, sal_uInt32 nValue = 0):
        CntUInt32Item(which, nValue) {}

    SfxUInt32Item(sal_uInt16 which, SvStream & rStream):
        CntUInt32Item(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, sal_uInt16) const
    { return new SfxUInt32Item(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxUInt32Item(*this); }
};

#endif // _SFXINTITEM_HXX

