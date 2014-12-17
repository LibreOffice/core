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

#include <svl/svldllapi.h>
#include <com/sun/star/uno/Any.hxx>

#include <sal/config.h>
#include <tools/rtti.hxx>
#include <limits.h>
#include <tools/solar.h>
#include <tools/debug.hxx>
#include <svl/hint.hxx>

class SbxVariable;
class SbxObject;
class SvStream;
class Color;
class IntlWrapper;

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }

enum SfxItemKind {
   SFX_ITEMS_NONE,
   SFX_ITEMS_DELETEONIDLE,
   SFX_ITEMS_STATICDEFAULT,
   SFX_ITEMS_POOLDEFAULT
};

#define SFX_ITEMS_OLD_MAXREF                0xffef
#define SFX_ITEMS_MAXREF                    0xfffffffe
#define SFX_ITEMS_SPECIAL                   0xffffffff

#define CONVERT_TWIPS                       0x80    // Uno conversion for measurement (for MemberId)



// UNO3 shortcuts

// warning, if there is no boolean inside the any this will always return the value false
inline bool Any2Bool( const ::com::sun::star::uno::Any&rValue )
{
    bool nValue = false;
    if( rValue.hasValue() )
    {
        if( rValue.getValueType() == ::getCppuBooleanType() )
        {
            nValue = *(sal_Bool*)rValue.getValue();
        }
        else
        {
            sal_Int32 nNum = 0;
            if( rValue >>= nNum )
                nValue = nNum != 0;
        }
    }

    return nValue;
}

inline ::com::sun::star::uno::Any Bool2Any( bool bValue )
{
    return ::com::sun::star::uno::Any( &bValue, ::getCppuBooleanType() );
}


enum SfxFieldUnit
{
    SFX_FUNIT_NONE, SFX_FUNIT_TWIP
};

enum SfxMapUnit
{
    SFX_MAPUNIT_100TH_MM,
    SFX_MAPUNIT_10TH_MM,
    SFX_MAPUNIT_MM,
    SFX_MAPUNIT_CM,
    SFX_MAPUNIT_1000TH_INCH,
    SFX_MAPUNIT_100TH_INCH,
    SFX_MAPUNIT_10TH_INCH,
    SFX_MAPUNIT_INCH,
    SFX_MAPUNIT_POINT,
    SFX_MAPUNIT_TWIP,
    SFX_MAPUNIT_PIXEL,
    SFX_MAPUNIT_SYSFONT,
    SFX_MAPUNIT_APPFONT,
    SFX_MAPUNIT_RELATIVE,
    SFX_MAPUNIT_ABSOLUTE
};



enum SfxItemPresentation

/*
 * The values of this enum describe the degree of textual
 * representation of an item after calling the virtual
 * method <SfxPoolItem::GetPresentation()const>.
 */
{
    SFX_ITEM_PRESENTATION_NAMELESS,
    SFX_ITEM_PRESENTATION_COMPLETE
};



enum class SfxItemState {
    // These values have to match the values in the com::sun::star::frame::status::ItemState IDL
    UNKNOWN  = 0,
    DISABLED = 0x0001,
    READONLY = 0x0002,
    DONTCARE = 0x0010,
    DEFAULT  = 0x0020,
    SET      = 0x0030
};

class SvXMLUnitConverter;
class SfxItemPool;
class SfxItemSet;




class SVL_DLLPUBLIC SfxPoolItem
{
friend class SfxItemPool;
friend class SfxItemDisruptor_Impl;
friend class SfxItemPoolCache;
friend class SfxItemSet;
friend class SfxVoidItem;

    mutable sal_uLong   m_nRefCount;
    sal_uInt16  m_nWhich;
    SfxItemKind  m_nKind;

private:
    inline void              SetRefCount( sal_uLong n );
    inline void              SetKind( SfxItemKind n );
public:
    inline void              AddRef( sal_uLong n = 1 ) const;
private:
    inline sal_uLong         ReleaseRef( sal_uLong n = 1 ) const;

protected:
                             explicit SfxPoolItem( sal_uInt16 nWhich = 0 );
                             SfxPoolItem( const SfxPoolItem& );

public:
                             TYPEINFO();
    virtual                  ~SfxPoolItem();

    void                     SetWhich( sal_uInt16 nId ) { m_nWhich = nId; }
    sal_uInt16               Which() const { return m_nWhich; }
    virtual bool             operator==( const SfxPoolItem& ) const = 0;
    bool                     operator!=( const SfxPoolItem& rItem ) const
                             { return !(*this == rItem); }
    virtual int              Compare( const SfxPoolItem &rWith ) const;
    virtual int              Compare( const SfxPoolItem &rWith, const IntlWrapper& rIntlWrapper ) const;

    /**  @return true if it has a valid string representation */
    virtual bool             GetPresentation( SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresentationMetric,
                                    OUString &rText,
                                    const IntlWrapper * pIntlWrapper = 0 ) const;

    virtual sal_uInt16       GetVersion( sal_uInt16 nFileFormatVersion ) const;
    virtual bool             ScaleMetrics( long lMult, long lDiv );
    virtual bool             HasMetrics() const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem*     Create( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SvStream&        Store( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const = 0;

    sal_uLong                GetRefCount() const { return m_nRefCount; }
    inline SfxItemKind       GetKind() const { return m_nKind; }

private:
    SfxPoolItem&             operator=( const SfxPoolItem& );    // not implemented!!
};



inline void SfxPoolItem::SetRefCount( sal_uLong n )
{
    m_nRefCount = n;
    m_nKind = SFX_ITEMS_NONE;
}

inline void SfxPoolItem::SetKind( SfxItemKind n )
{
    m_nRefCount = SFX_ITEMS_SPECIAL;
    m_nKind = n;
}

inline void SfxPoolItem::AddRef( sal_uLong n ) const
{
    DBG_ASSERT(m_nRefCount <= SFX_ITEMS_MAXREF, "AddRef with non-Pool-Item");
    DBG_ASSERT(ULONG_MAX - m_nRefCount > n, "AddRef: refcount overflow");
    m_nRefCount += n;
}

inline sal_uLong SfxPoolItem::ReleaseRef( sal_uLong n ) const
{
    DBG_ASSERT(m_nRefCount <= SFX_ITEMS_MAXREF, "AddRef with non-Pool-Item");
    DBG_ASSERT(m_nRefCount >= n, "AddRef: refcount underflow");
    m_nRefCount -= n;
    return m_nRefCount;
}



inline bool IsPoolDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->GetKind() == SFX_ITEMS_POOLDEFAULT;
}

inline bool IsStaticDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->GetKind() == SFX_ITEMS_STATICDEFAULT;
}

inline bool IsDefaultItem( const SfxPoolItem *pItem )
{
    return pItem && (pItem->GetKind() == SFX_ITEMS_STATICDEFAULT || pItem->GetKind() == SFX_ITEMS_POOLDEFAULT);
}

inline bool IsPooledItem( const SfxPoolItem *pItem )
{
    return pItem && pItem->GetRefCount() > 0 && pItem->GetRefCount() <= SFX_ITEMS_MAXREF;
}

inline bool IsInvalidItem(const SfxPoolItem *pItem)
{
    return pItem == reinterpret_cast<SfxPoolItem *>(-1);
}



class SVL_DLLPUBLIC SfxVoidItem: public SfxPoolItem
{
    SfxVoidItem & operator=( const SfxVoidItem& ); // not implemented.
public:
                            TYPEINFO_OVERRIDE();
                            explicit SfxVoidItem( sal_uInt16 nWhich );
                            SfxVoidItem( sal_uInt16 nWhich, SvStream & );
                            SfxVoidItem( const SfxVoidItem& );
                            virtual ~SfxVoidItem();

    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = 0 ) const SAL_OVERRIDE;

    // create a copy of itself
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
            void            SetWhich(sal_uInt16 nWh) { m_nWhich = nWh; }
};



class SVL_DLLPUBLIC SfxSetItem: public SfxPoolItem
{
    SfxItemSet              *pSet;

    SfxSetItem & operator=( const SfxSetItem& ); // not implemented.

public:
                            TYPEINFO_OVERRIDE();
                            SfxSetItem( sal_uInt16 nWhich, SfxItemSet *pSet );
                            SfxSetItem( sal_uInt16 nWhich, const SfxItemSet &rSet );
                            SfxSetItem( const SfxSetItem&, SfxItemPool *pPool = 0 );
                            virtual ~SfxSetItem();

    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = 0 ) const SAL_OVERRIDE;

    // create a copy of itself
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE = 0;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nVersion) const SAL_OVERRIDE = 0;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nVer) const SAL_OVERRIDE;

    const SfxItemSet&       GetItemSet() const
                            { return *pSet; }
    SfxItemSet&             GetItemSet()
                            { return *pSet; }
};


// Handle class for PoolItems

class SVL_DLLPUBLIC SfxItemHandle
{
    sal_uInt16      *pRef;
    SfxPoolItem     *pItem;
public:
    explicit SfxItemHandle( SfxPoolItem& );
    SfxItemHandle( const SfxItemHandle& );
    ~SfxItemHandle();

    const SfxItemHandle &operator=(const SfxItemHandle &);
    const SfxPoolItem &GetItem() const { return *pItem; }
};



DECL_PTRHINT(SVL_DLLPUBLIC, SfxPoolItemHint, SfxPoolItem);



#endif // INCLUDED_SVL_POOLITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
