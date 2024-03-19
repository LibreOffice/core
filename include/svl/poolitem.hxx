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
#include <vector>

#include <com/sun/star/uno/Any.hxx>
#include <svl/hint.hxx>
#include <svl/svldllapi.h>
#include <svl/typedwhich.hxx>
#include <tools/mapunit.hxx>
#include <tools/long.hxx>
#include <boost/property_tree/ptree_fwd.hpp>
#include <unordered_set>

class IntlWrapper;

#define SFX_ITEMS_MAXREF                    0xffffffff
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

// Offer simple assert if Item is RefCounted (RefCnt > 1) and thus CANNOT be changed.
// This should be used at *all* SfxPoolItem set* methods. Remember that SfxPoolItems
// are by design intended to be create-one, read-only, shared data packages
#define ASSERT_CHANGE_REFCOUNTED_ITEM \
    assert(!GetRefCount() && "ERROR: RefCounted SfxPoolItem CANNOT be changed (!)")

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

    /** Specifies that the property is currently in a don't care state
     * and thus invalid
     * <br/>
     * This is normally used if a selection provides more than one state
     * for a property at the same time, so the Item is over-defined and
     * has no valid state -> invalid
     */
    INVALID = 0x0010,

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

#ifdef DBG_UTIL
SVL_DLLPUBLIC size_t getAllocatedSfxPoolItemCount();
SVL_DLLPUBLIC size_t getUsedSfxPoolItemCount();
SVL_DLLPUBLIC void listAllocatedSfxPoolItems();
#endif

class SfxItemPool;
class SfxItemSet;
typedef struct _xmlTextWriter* xmlTextWriterPtr;
class ItemInstanceManager;

class SVL_DLLPUBLIC SfxPoolItem
{
    friend class SfxItemPool;
    friend class SfxItemSet;
    friend class InstanceManagerHelper;
    friend class ItemInfoStatic;
    friend class ItemInfoDynamic;

    // allow ItemSetTooling to access
    friend SfxPoolItem const* implCreateItemEntry(SfxItemPool&, SfxPoolItem const*, bool);
    friend void implCleanupItemEntry(SfxPoolItem const*);

    mutable sal_uInt32 m_nRefCount;
    sal_uInt16  m_nWhich;

#ifdef DBG_UTIL
    // for debugging add a serial number, will be set in the constructor
    // and count up from zero. If you have a deterministic error case and
    // see the Item involved in the debugger you can  use that number in
    // the next run to see where that Item gets constructed and how it is
    // involved/ processed
    sal_uInt32  m_nSerialNumber;
#endif

    // bitfield for Item attributes that are Item-Dependent

    // Item is registered at some Pool as default.
    //   m_bStaticDefault: direct Pool Item (CAUTION:
    //     these are not really 'static', but should be)
    //   m_bDynamicDefault: dynamic pool item, e.g.
    //     SfxSetItems which are Pool dependent
    bool        m_bStaticDefault : 1;
    bool        m_bDynamicDefault : 1;

    // Item is derived from SfxSetItem -> is Pool-dependent
    bool        m_bIsSetItem : 1;

    // Defines if the Item can be shared/RefCounted else it will be cloned.
    // Default is true - as it should be for all Items. It is needed by some
    // SW items, so protected to let them set it in constructor. If this could
    // be fixed at that Items we may remove this again.
    bool        m_bShareable : 1;

protected:
#ifdef DBG_UTIL
    // this flag will make debugging item stuff much simpler
    bool        m_bDeleted : 1;
#endif

private:
    inline void SetRefCount(sal_uInt32 n)
    {
        m_nRefCount = n;
        m_bStaticDefault = m_bDynamicDefault = false;
    }

protected:
    void setStaticDefault() { m_bStaticDefault = true; }
    void setDynamicDefault() { m_bDynamicDefault = true; }
    void setIsSetItem() { m_bIsSetItem = true; }
    void setNonShareable() { m_bShareable = false; }

    // access ItemInstanceManager for this Item, default
    // is nullptr. If you overload this it is expected that
    // you return a ptr to a static, Item-local managed
    // instance that exists the whole office lifetime. This
    // usually means to have a static instance directly in the
    // implementation of the overloaded function (just grep
    // for examples)
    virtual ItemInstanceManager* getItemInstanceManager() const;

public:
    inline void AddRef(sal_uInt32 n = 1) const
    {
        assert(n <= SFX_ITEMS_MAXREF - m_nRefCount && "AddRef: refcount overflow");
        m_nRefCount += n;
    }

#ifdef DBG_UTIL
    sal_uInt32 getSerialNumber() const { return m_nSerialNumber; }
#endif

    bool isStaticDefault() const { return m_bStaticDefault; }
    bool isDynamicDefault() const { return m_bDynamicDefault; }
    bool isSetItem() const { return m_bIsSetItem; }
    bool isShareable() const { return m_bShareable; }

    // version that allows nullptrs
    static bool areSame(const SfxPoolItem* pItem1, const SfxPoolItem* pItem2);

    // if you have the items (not nullptrs) use this version
    static bool areSame(const SfxPoolItem& rItem1, const SfxPoolItem& rItem2);

private:
    inline sal_uInt32 ReleaseRef(sal_uInt32 n = 1) const
    {
        assert(n <= m_nRefCount);
        m_nRefCount -= n;
        return m_nRefCount;
    }

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
    // StaticWhichCast asserts if the TypedWhichId is not matching its type, otherwise it returns a reference.
    // You can use StaticWhichCast when you are sure about the type at compile time -- like a static_cast.
    template<class T> T& StaticWhichCast(TypedWhichId<T> nId)
    {
        (void)nId;
        assert(nId == m_nWhich);
        assert(dynamic_cast<T*>(this));
        return *static_cast<T*>(this);
    }
    template<class T> const T& StaticWhichCast(TypedWhichId<T> nId) const
    {
        (void)nId;
        assert(nId == m_nWhich);
        assert(dynamic_cast<const T*>(this));
        return *static_cast<const T*>(this);
    }
    // DynamicWhichCast returns nullptr if the TypedWhichId is not matching its type, otherwise it returns a typed pointer.
    // it asserts if the TypedWhichId matches its Which, but not the RTTI type.
    // You can use DynamicWhichCast when you are not sure about the type at compile time -- like a dynamic_cast.
    template<class T> T* DynamicWhichCast(TypedWhichId<T> nId)
    {
        if(m_nWhich != nId)
            return nullptr;
        assert(dynamic_cast<T*>(this));
        return static_cast<T*>(this);
    }
    template<class T> const T* DynamicWhichCast(TypedWhichId<T> nId) const
    {
        if(m_nWhich != nId)
            return nullptr;
        assert(dynamic_cast<const T*>(this));
        return static_cast<const T*>(this);
    }
    virtual bool             operator==( const SfxPoolItem& ) const = 0;
    bool                     operator!=( const SfxPoolItem& rItem ) const
                             { return !(*this == rItem); }

    /**  @return true if it has a valid string representation */
    virtual bool             GetPresentation( SfxItemPresentation ePresentation,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresentationMetric,
                                    OUString &rText,
                                    const IntlWrapper& rIntlWrapper ) const;

    virtual void             ScaleMetrics( tools::Long lMult, tools::Long lDiv );
    virtual bool             HasMetrics() const;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const = 0;
    // clone and call SetWhich
    std::unique_ptr<SfxPoolItem> CloneSetWhich( sal_uInt16 nNewWhich ) const;
    template<class T> std::unique_ptr<T> CloneSetWhich( TypedWhichId<T> nId ) const
    {
        return std::unique_ptr<T>(static_cast<T*>(CloneSetWhich(sal_uInt16(nId)).release()));
    }

    sal_uInt32               GetRefCount() const { return m_nRefCount; }
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
    virtual boost::property_tree::ptree dumpAsJSON() const;

private:
    SfxPoolItem&             operator=( const SfxPoolItem& ) = delete;
};

// basic Interface definition
class SVL_DLLPUBLIC ItemInstanceManager
{
    // allow *only* ItemSetTooling to access
    friend SfxPoolItem const* implCreateItemEntry(SfxItemPool&, SfxPoolItem const*, bool);
    friend void implCleanupItemEntry(SfxPoolItem const*);

    // Define for which class to register (usually typeid().hash_code()).
    std::size_t     m_aClassHash;

public:
    ItemInstanceManager(const std::size_t aClassHash)
    : m_aClassHash(aClassHash)
    {
    }
    virtual ~ItemInstanceManager() = default;

    std::size_t getClassHash() const { return m_aClassHash; }

private:
    // standard interface, accessed exclusively
    // by implCreateItemEntry/implCleanupItemEntry
    virtual const SfxPoolItem* find(const SfxPoolItem&) const = 0;
    virtual void add(const SfxPoolItem&) = 0;
    virtual void remove(const SfxPoolItem&) = 0;
};

// offering a default implementation that can be use for
// each SfxPoolItem (except when !isShareable()). It just
// uses an unordered_set holding ptrs to SfxPoolItems added
// and SfxPoolItem::operator== to linearly search for one.
// Thus this is not the fastest, but as fast as old 'poooled'
// stuff - better use an intelligent, pro-Item implementation
// that does e.g. hashing or whatever might be feasible for
// that specific Item (see other derivations)
class SVL_DLLPUBLIC DefaultItemInstanceManager : public ItemInstanceManager
{
    std::unordered_set<const SfxPoolItem*>  maRegistered;

public:
    DefaultItemInstanceManager(const std::size_t aClassHash)
    : ItemInstanceManager(aClassHash)
    {
    }

private:
    virtual const SfxPoolItem* find(const SfxPoolItem&) const override;
    virtual void add(const SfxPoolItem&) override;
    virtual void remove(const SfxPoolItem&) override;
};

inline bool IsStaticDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->isStaticDefault();
}

inline bool IsDynamicDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->isDynamicDefault();
}

inline bool IsDefaultItem( const SfxPoolItem *pItem )
{
    return pItem && (pItem->isStaticDefault() || pItem->isDynamicDefault());
}

inline bool IsPooledItem( const SfxPoolItem *pItem )
{
    return pItem && pItem->GetRefCount() > 0;
}

SVL_DLLPUBLIC extern SfxPoolItem const * const INVALID_POOL_ITEM;
SVL_DLLPUBLIC extern SfxPoolItem const * const DISABLED_POOL_ITEM;

inline bool IsInvalidItem(const SfxPoolItem *pItem)
{
    return pItem == INVALID_POOL_ITEM;
}

inline bool IsDisabledItem(const SfxPoolItem *pItem)
{
    return pItem == DISABLED_POOL_ITEM;
}

SVL_DLLPUBLIC bool areSfxPoolItemPtrsEqual(const SfxPoolItem* pItem1, const SfxPoolItem* pItem2);

class SVL_DLLPUBLIC SfxPoolItemHint final : public SfxHint
{
    SfxPoolItem* pObj;
public:
    explicit SfxPoolItemHint( SfxPoolItem* Object ) : pObj(Object) {}
    SfxPoolItem* GetObject() const { return pObj; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
