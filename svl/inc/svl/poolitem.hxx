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
#ifndef _SFXPOOLITEM_HXX
#define _SFXPOOLITEM_HXX

#include "svl/svldllapi.h"
#include <com/sun/star/uno/Any.hxx>

#include <sal/config.h>
#include <tools/rtti.hxx>
#include <limits.h>
#include <tools/solar.h>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <svl/hint.hxx>

typedef long SfxArgumentError;

class SbxVariable;
class SbxObject;
class SvStream;
class Color;
class IntlWrapper;

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }

static const sal_uInt32 SFX_ITEMS_DIRECT=   0xffffffff;
static const sal_uInt32 SFX_ITEMS_NULL=     0xfffffff0;  // instead StoreSurrogate
static const sal_uInt32 SFX_ITEMS_DEFAULT=  0xfffffffe;

#define SFX_ITEMS_POOLDEFAULT               0xffff
#define SFX_ITEMS_STATICDEFAULT             0xfffe
#define SFX_ITEMS_DELETEONIDLE              0xfffd

#define SFX_ITEMS_OLD_MAXREF                0xffef
#define SFX_ITEMS_MAXREF                    0xfffffffe
#define SFX_ITEMS_SPECIAL                   0xffffffff

#define CONVERT_TWIPS                       0x80    //Uno-Konvertierung fuer Massangaben (fuer MemberId)

// -----------------------------------------------------------------------

// UNO3 shortcuts

// warning, if there is no boolean inside the any this will always return the value false
inline sal_Bool Any2Bool( const ::com::sun::star::uno::Any&rValue )
{
    sal_Bool nValue = sal_False;
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

inline ::com::sun::star::uno::Any Bool2Any( sal_Bool bValue )
{
    return ::com::sun::star::uno::Any( &bValue, ::getCppuBooleanType() );
}

// -----------------------------------------------------------------------

//! Notloesung!!!
enum SfxFieldUnit
{
    SFX_FUNIT_NONE, SFX_FUNIT_MM, SFX_FUNIT_CM, SFX_FUNIT_M, SFX_FUNIT_KM,
    SFX_FUNIT_TWIP, SFX_FUNIT_POINT, SFX_FUNIT_PICA,
    SFX_FUNIT_INCH, SFX_FUNIT_FOOT, SFX_FUNIT_MILE, SFX_FUNIT_CHAR, SFX_FUNIT_LINE, SFX_FUNIT_CUSTOM
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

// -----------------------------------------------------------------------

enum SfxItemPresentation

/*  [Beschreibung]

    Die Werte dieses Enums bezeichnen den Grad der textuellen
    Presentation eines Items nach Aufruf der virtuellen Methode
    <SfxPoolItem::GetPresentation()const>.
*/

{
    SFX_ITEM_PRESENTATION_NONE,
    SFX_ITEM_PRESENTATION_NAMEONLY,
    SFX_ITEM_PRESENTATION_NAMELESS,
    SFX_ITEM_PRESENTATION_COMPLETE
};

// -----------------------------------------------------------------------

typedef sal_uInt16 SfxItemState;

#define SFX_ITEM_UNKNOWN    0x0000

#define SFX_ITEM_DISABLED   0x0001
#define SFX_ITEM_READONLY   0x0002

#define SFX_ITEM_DONTCARE   0x0010
#define SFX_ITEM_DEFAULT    0x0020
#define SFX_ITEM_SET        0x0030

// old stuff - dont use!!!
#define SFX_ITEM_AVAILABLE  SFX_ITEM_DEFAULT
#define SFX_ITEM_OFF        SFX_ITEM_DEFAULT
#define SFX_ITEM_ON         SFX_ITEM_SET

DBG_NAMEEX_VISIBILITY(SfxPoolItem, SVL_DLLPUBLIC)
DBG_NAMEEX(SfxVoidItem)
DBG_NAMEEX(SfxItemHandle)

class SvXMLUnitConverter;
class SfxItemPool;
class SfxItemSet;

class String;
namespace rtl
{
    class OUString;
}

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxPoolItem
{
friend class SfxItemPool;
friend class SfxItemDesruptor_Impl;
friend class SfxItemPoolCache;
friend class SfxItemSet;
friend class SfxVoidItem;

    sal_uLong                    nRefCount;                    // Referenzzaehler
    sal_uInt16                   nWhich;
    sal_uInt16                   nKind;

private:
    inline void              SetRefCount( sal_uLong n );
    inline void              SetKind( sal_uInt16 n );
public:
    inline sal_uLong             AddRef( sal_uLong n = 1 ) const;
private:
    inline sal_uLong             ReleaseRef( sal_uLong n = 1 ) const;
    SVL_DLLPRIVATE long      Delete_Impl(void*);

protected:
                             SfxPoolItem( sal_uInt16 nWhich = 0 );
                             SfxPoolItem( const SfxPoolItem& );

public:
                             TYPEINFO();
    virtual                  ~SfxPoolItem();

    void                     SetWhich( sal_uInt16 nId ) {
                                DBG_CHKTHIS(SfxPoolItem, 0);
                                nWhich = nId; }
    sal_uInt16                   Which() const {
                                 DBG_CHKTHIS(SfxPoolItem, 0);
                                 return nWhich; }
    virtual int              operator==( const SfxPoolItem& ) const = 0;
    int                      operator!=( const SfxPoolItem& rItem ) const
                             { return !(*this == rItem); }
    virtual int              Compare( const SfxPoolItem &rWith ) const;
    virtual int              Compare( const SfxPoolItem &rWith, const IntlWrapper& rIntlWrapper ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresentationMetric,
                                    XubString &rText,
                                    const IntlWrapper * pIntlWrapper = 0 ) const;

    virtual sal_uInt16           GetVersion( sal_uInt16 nFileFormatVersion ) const;
    virtual bool             ScaleMetrics( long lMult, long lDiv );
    virtual bool             HasMetrics() const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem*     Create( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SvStream&        Store( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const = 0;

    sal_uLong                    GetRefCount() const { return nRefCount; }
    inline sal_uInt16            GetKind() const { return nKind; }

    /** Read in a Unicode string from a streamed byte string representation.

        @param rStream  Some (input) stream.  Its Stream/TargetCharSets must
        be set to correct values!

        @param rString  On success, returns the reconstructed Unicode string.

        @return  True if the string was successfuly read and reconstructed.
     */
    static rtl::OUString readByteString(SvStream & rStream);

    /** Write a byte string representation of a Unicode string into a stream.

        @param rStream  Some (output) stream.  Its Stream/TargetCharSets must
        be set to correct values!

        @param rString  Some Unicode string.
     */
    static void writeByteString(SvStream & rStream,
                                const rtl::OUString& rString);

    /** Read in a Unicode string from either a streamed Unicode or byte string
        representation.

        @param rStream  Some (input) stream.  If bUnicode is false, its
        Stream/TargetCharSets must be set to correct values!

        @param bUnicode  Whether to read in a stream Unicode (true) or byte
        string (false) representation.

        @return          On success, returns the reconstructed Unicode string.
     */
    static rtl::OUString readUnicodeString(SvStream & rStream, bool bUnicode);

    /** Write a Unicode string representation of a Unicode string into a
        stream.

        @param rStream  Some (output) stream.

        @param rString  Some Unicode string.
     */
    static void writeUnicodeString(SvStream & rStream,
                                   const rtl::OUString& rString);

private:
    SfxPoolItem&             operator=( const SfxPoolItem& );    // n.i.!!
};

// -----------------------------------------------------------------------

inline void SfxPoolItem::SetRefCount( sal_uLong n )
{
    DBG_CHKTHIS( SfxPoolItem, 0 );
    nRefCount = n;
    nKind = 0;
}

inline void SfxPoolItem::SetKind( sal_uInt16 n )
{
    DBG_CHKTHIS( SfxPoolItem, 0 );
    nRefCount = SFX_ITEMS_SPECIAL;
    nKind = n;
}

inline sal_uLong SfxPoolItem::AddRef( sal_uLong n ) const
{
    DBG_CHKTHIS( SfxPoolItem, 0 );
    DBG_ASSERT( nRefCount <= SFX_ITEMS_MAXREF, "AddRef mit nicht-Pool-Item" );
    DBG_ASSERT( ULONG_MAX - nRefCount > n, "AddRef: Referenzzaehler ueberschlaegt sich" );
    return ( ((SfxPoolItem *)this)->nRefCount += n );
}

inline sal_uLong SfxPoolItem::ReleaseRef( sal_uLong n ) const
{
    DBG_CHKTHIS( SfxPoolItem, 0 );
    DBG_ASSERT( nRefCount <= SFX_ITEMS_MAXREF, "AddRef mit nicht-Pool-Item" );
    DBG_ASSERT( nRefCount >= n, "ReleaseRef: Referenzzaehler ueberschlaegt sich" );
    ((SfxPoolItem *)this)->nRefCount -= n;
    return nRefCount;
}

// -----------------------------------------------------------------------

inline int IsPoolDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->GetKind() == SFX_ITEMS_POOLDEFAULT;
}

inline int IsStaticDefaultItem(const SfxPoolItem *pItem )
{
    return pItem && pItem->GetKind() == SFX_ITEMS_STATICDEFAULT;
}

inline int IsDefaultItem( const SfxPoolItem *pItem )
{
    return pItem && pItem->GetKind() >= SFX_ITEMS_STATICDEFAULT;
}

inline int IsPooledItem( const SfxPoolItem *pItem )
{
    return pItem && pItem->GetRefCount() > 0 && pItem->GetRefCount() <= SFX_ITEMS_MAXREF;
}

inline int IsInvalidItem(const SfxPoolItem *pItem)
{
    return pItem == (SfxPoolItem *)-1;
}

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxVoidItem: public SfxPoolItem
{
    SfxVoidItem & operator=( const SfxVoidItem& ); // not implemented.
public:
                            TYPEINFO();
                            SfxVoidItem( sal_uInt16 nWhich );
                            SfxVoidItem( sal_uInt16 nWhich, SvStream & );
                            SfxVoidItem( const SfxVoidItem& );
                            ~SfxVoidItem();

    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;

    // von sich selbst eine Kopie erzeugen
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
            void            SetWhich(sal_uInt16 nWh) { nWhich = nWh; }
};

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxSetItem: public SfxPoolItem
{
    SfxItemSet              *pSet;

    SfxSetItem & operator=( const SfxSetItem& ); // not implemented.

public:
                            TYPEINFO();
                            SfxSetItem( sal_uInt16 nWhich, SfxItemSet *pSet );
                            SfxSetItem( sal_uInt16 nWhich, const SfxItemSet &rSet );
                            SfxSetItem( const SfxSetItem&, SfxItemPool *pPool = 0 );
                            ~SfxSetItem();

    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;

    // von sich selbst eine Kopie erzeugen
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const = 0;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nVersion) const = 0;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nVer) const;

    const SfxItemSet&       GetItemSet() const
                            { return *pSet; }
    SfxItemSet&             GetItemSet()
                            { return *pSet; }
};

// -----------------------------------------------------------------------
// Handle Klasse fuer PoolItems

class SVL_DLLPUBLIC SfxItemHandle
{
    sal_uInt16      *pRef;
    SfxPoolItem *pItem;
public:
    SfxItemHandle( SfxPoolItem& );
    SfxItemHandle( const SfxItemHandle& );
    ~SfxItemHandle();

    const SfxItemHandle &operator=(const SfxItemHandle &);
    const SfxPoolItem &GetItem() const { return *pItem; }
};

// -----------------------------------------------------------------------

DECL_PTRHINT(SVL_DLLPUBLIC, SfxPoolItemHint, SfxPoolItem);

// -----------------------------------------------------------------------

#endif // #ifndef _SFXPOOLITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
