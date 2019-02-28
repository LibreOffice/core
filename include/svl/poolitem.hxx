/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVL_POOLITEM_HXX
#define INCLUDED_SVL_POOLITEM_HXX

#include <sal/config.h>

#include <memory>

#include <com/sun/star/uno/Any.hxx>
#include <svl/hint.hxx>
#include <svl/svldllapi.h>
#include <svl/typedwhich.hxx>
#include <tools/mapunit.hxx>

class IntlWrapper;
class SvStream;

enum class SfxItemKind : sal_Int8
{
   NONE,
   DeleteOnIdle,
   StaticDefault,
   PoolDefault
};

#define SFX_ITEMS_OLD_MAXREF                0xffef
#define SFX_ITEMS_MAXREF                    0xfffffffe
#define SFX_ITEMS_SPECIAL                   0xffffffff

#define CONVERT_TWIPS                       0x80    // Uno conversion for measurement (for MemberId)

// warning, if there is no boolean inside the any this will always return the value false
inline bool Any2Bool( const css::uno::Any&rValue )
{
    bool bValue = false;
    if( !(rValue >>= bValue) )
    {
        sal_Int32 nNum = 0;
        if( rValue >>= nNum )
            bValue = nNum != 0;
    }

    return bValue;
}

/*
 * The values of this enum describe the degree of textual
 * representation of an item after calling the virtual
 * method <SfxPoolItem::GetPresentation()const>.
 */
enum class SfxItemPresentation
{
    Nameless,
    Complete
};

/**
 * These values have to match the values in the
 * css::frame::status::ItemState IDL
 * to be found at offapi/com/sun/star/frame/status/ItemState.idl
*/
enum class SfxItemState {

    /** Specifies an unknown state. */
    UNKNOWN  = 0,

    /** Specifies that the property is currently disabled. */
    DISABLED = 0x0001,

    /** Specifies that the property is currently read-only. */
    READONLY = 0x0002,

    /** Specifies that the property is currently in a don't care state.
     * <br/>
     * This is normally used if a selection provides more than one state
     * for a property at the same time.
     */
    DONTCARE = 0x0010,

    /** Specifies that the property is currently in a default state. */
    DEFAULT  = 0x0020,

    /** The property has been explicitly set to a given value hence we know
     * we are not taking the default value.
     * <br/>
     * For example, you may want to get the font color and it might either
     * be the default one or one that has been explicitly set.
    */
    SET      = 0x0040
};

#define INVALID_POOL_ITEM reinterpret_cast<SfxPoolItem*>(-1)

class SfxItemPool;
class SfxItemSet;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

class SVL_DLLPUBLIC SfxPoolItem
{
friend class SfxItemPool;
friend class SfxItemDisruptor_Impl;
friend class SfxItemPoolCache;
friend class SfxItemSet;
friend class SfxVoidItem;

    mutable sal_uInt32 m_nRefCount;
    sal_uInt16  m_nWhich;
    SfxItemKind  m_nKind;

private:
    inline void              SetRefCount(sal_uInt32 n);
    inline void              SetKind( SfxItemKind n );
public:
    inline void              AddRef(sal_uInt32 n = 1) const;
private:
    inline sal_uInt32        ReleaseRef(sal_uInt32 n = 1) const;

protected:
                             explicit SfxPoolItem( sal_uInt16 nWhich = 0 );
                             SfxPoolItem( const SfxPoolItem& rCopy)
                                 : SfxPoolItem(rCopy.m_nWhich) {}

public:
    virtual                  ~SfxPoolItem();

    void                     SetWhich( sal_uInt16 nId )
                             {
                                 // can only change the Which before we are in a set
                                 assert(m_nRefCount==0);
                                 m_nWhich = nId;
                             }
    sal_uInt16               Which() const { return m_nWhich; }
    virtual bool             operator==( const SfxPoolItem& ) const = 0;
    bool                     operator!=( const SfxPoolItem& rItem ) const
                             { return !(*this == rItem); }

    /**  @return true if it has a valid string representation */
    virtual bool             GetPresentation( SfxItemPresentation ePresentation,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresentationMetric,
                                    OUString &rText,
                                    const IntlWrapper& rIntlWrapper ) const;

    virtual sal_uInt16       GetVersion( sal_uInt16 nFileFormatVersion ) const;
    virtual void             ScaleMetrics( long lMult, long lDiv );
    virtual bool             HasMetrics() const;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId );

    virtual SfxPoolItem*     Create( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SvStream&        Store( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const = 0;
    // clone and call SetWhich
    std::unique_ptr<SfxPoolItem> CloneSetWhich( sal_uInt16 nNewWhich ) const;
    template<class T> std::unique_ptr<T> CloneSetWhich( TypedWhichId<T> nId ) const
    {
        return std::unique_ptr<T>(static_cast<T*>(CloneSetWhich(sal_uInt16(nId)).release()));
    }

    sal_uInt32               GetRefCount() const { return m_nRefCount; }
    SfxItemKind       GetKind() const { return m_nKind; }
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;

    /** Only SfxVoidItem shall and must return true for this.

        This avoids costly calls to dynamic_cast<const SfxVoidItem*>()
        specifically in SfxItemSet::GetItemState()
     */
    virtual bool             IsVoidItem() const;

private:
    SfxPoolItem&             operator=( const SfxPoolItem& ) = delete;
};

inline void SfxPoolItem::SetRefCount(sal_uInt32 n)
{
    m_nRefCount = n;
    m_nKind = SfxItemKind::NONE;
}

inline void SfxPoolItem::SetKind( SfxItemKind n )
{
    m_nRefCount = SFX_ITEMS_SPECIAL;
    m_nKind = n;
}

inline void SfxPoolItem::AddRef(sal_uInt32 n) const
{
    assert(m_nRefCount <= SFX_ITEMS_MAXREF && "AddRef with non-Pool-Item");
    assert(n <= SFX_ITEMS_MAXREF - m_nRefCount && "AddRef: refcount overflow");
    m_nRefCount += n;
}

inline sal_uInt32 SfxPoolItem::ReleaseRef(sal_uInt32 n) const
{
    assert(m_nRefCount <= SFX_ITEMS_MAXREF && "ReleaseRef with non-Pool-Item");
    assert(n <= m_nRefCount);
    m_nRefCount -= n;
    return m_nRefCount;
}

inline bool IsPoolDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->GetKind() == SfxItemKind::PoolDefault;
}

inline bool IsStaticDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->GetKind() == SfxItemKind::StaticDefault;
}

inline bool IsDefaultItem( const SfxPoolItem *pItem )
{
    return pItem && (pItem->GetKind() == SfxItemKind::StaticDefault || pItem->GetKind() == SfxItemKind::PoolDefault);
}

inline bool IsPooledItem( const SfxPoolItem *pItem )
{
    return pItem && pItem->GetRefCount() > 0 && pItem->GetRefCount() <= SFX_ITEMS_MAXREF;
}

inline bool IsInvalidItem(const SfxPoolItem *pItem)
{
    return pItem == INVALID_POOL_ITEM;
}

class SVL_DLLPUBLIC SfxVoidItem final: public SfxPoolItem
{
public:
                            static SfxPoolItem* CreateDefault();
                            explicit SfxVoidItem( sal_uInt16 nWhich );
                            virtual ~SfxVoidItem() override;

    SfxVoidItem(SfxVoidItem const &) = default;
    SfxVoidItem(SfxVoidItem &&) = default;
    SfxVoidItem & operator =(SfxVoidItem const &) = delete; // due to SfxPoolItem
    SfxVoidItem & operator =(SfxVoidItem &&) = delete; // due to SfxPoolItem

    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper& ) const override;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    // create a copy of itself
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    /** Always returns true as this is an SfxVoidItem. */
    virtual bool            IsVoidItem() const override;
};

class SVL_DLLPUBLIC SfxSetItem: public SfxPoolItem
{
    std::unique_ptr<SfxItemSet>  pSet;

    SfxSetItem & operator=( const SfxSetItem& ) = delete;

public:
                            SfxSetItem( sal_uInt16 nWhich, std::unique_ptr<SfxItemSet> &&pSet );
                            SfxSetItem( sal_uInt16 nWhich, const SfxItemSet &rSet );
                            SfxSetItem( const SfxSetItem&, SfxItemPool *pPool = nullptr );
                            virtual ~SfxSetItem() override;

    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper& ) const override;

    // create a copy of itself
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override = 0;

    const SfxItemSet&       GetItemSet() const
                            { return *pSet; }
    SfxItemSet&             GetItemSet()
                            { return *pSet; }
};

class SVL_DLLPUBLIC SfxPoolItemHint: public SfxHint
{
    SfxPoolItem* pObj;
public:
    explicit SfxPoolItemHint( SfxPoolItem* Object ) : pObj(Object) {}
    SfxPoolItem* GetObject() const { return pObj; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
