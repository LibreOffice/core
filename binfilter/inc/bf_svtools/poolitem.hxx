/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXPOOLITEM_HXX
#define _SFXPOOLITEM_HXX

#include <com/sun/star/uno/Any.hxx>

#define TF_POOLABLE

#include <sal/config.h>
#include <tools/rtti.hxx>
#include <limits.h>
#include <tools/solar.h>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <bf_svtools/svarray.hxx>
#include <bf_svtools/hint.hxx>

class SbxObject;

class SvStream;
class Color;
class IntlWrapper;

namespace binfilter
{
typedef long SfxArgumentError;

#define SFX_ITEMS_DIRECT                    0xffff
#define SFX_ITEMS_NULL                      0xfff0  // anstelle StoreSurrogate

#define SFX_ITEMS_POOLDEFAULT               0xffff
#define SFX_ITEMS_STATICDEFAULT             0xfffe
#define SFX_ITEMS_DELETEONIDLE              0xfffd

#define SFX_ITEMS_OLD_MAXREF                0xffef
#define SFX_ITEMS_MAXREF                    0xfffffffe
#define SFX_ITEMS_SPECIAL					0xffffffff

#define CONVERT_TWIPS 						0x80	//Uno-Konvertierung fuer Massangaben (fuer MemberId)

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
    SFX_FUNIT_INCH, SFX_FUNIT_FOOT, SFX_FUNIT_MILE, SFX_FUNIT_CUSTOM
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

typedef USHORT SfxItemState;

#define	SFX_ITEM_UNKNOWN	0x0000

#define SFX_ITEM_DISABLED	0x0001
#define SFX_ITEM_READONLY	0x0002

#define SFX_ITEM_DONTCARE   0x0010
#define SFX_ITEM_DEFAULT	0x0020
#define SFX_ITEM_SET		0x0030

// old stuff - dont use!!!
#define SFX_ITEM_AVAILABLE	SFX_ITEM_DEFAULT
#define SFX_ITEM_OFF      	SFX_ITEM_DEFAULT
#define SFX_ITEM_ON			SFX_ITEM_SET

DBG_NAMEEX_VISIBILITY(SfxPoolItem, )
DBG_NAMEEX(SfxVoidItem)

class SvXMLUnitConverter;
class SfxItemPool;
class SfxItemSet;

// -----------------------------------------------------------------------

class  SfxPoolItem
{
friend class SfxItemPool;
friend class SfxItemPoolCache;
friend class SfxItemSet;
friend class SfxVoidItem;

    ULONG                    nRefCount;                    // Referenzzaehler
    USHORT                   nWhich;
    USHORT					 nKind;

private:
    inline void              SetRefCount( ULONG n );
    inline void				 SetKind( USHORT n );
public:
    inline ULONG             AddRef( ULONG n = 1 ) const;
private:
    inline ULONG             ReleaseRef( ULONG n = 1 ) const;
     long      Delete_Impl(void*);

protected:
                             SfxPoolItem( USHORT nWhich = 0 );
                             SfxPoolItem( const SfxPoolItem& );

public:
                             TYPEINFO();
    virtual                  ~SfxPoolItem();

    void                     SetWhich( USHORT nId ) {
                                DBG_CHKTHIS(SfxPoolItem, 0);
                                nWhich = nId; }
    USHORT                   Which() const {
                                 DBG_CHKTHIS(SfxPoolItem, 0);
                                 return nWhich; }
    virtual int              operator==( const SfxPoolItem& ) const = 0;
    int                      operator!=( const SfxPoolItem& rItem ) const
                             { return !(*this == rItem); }
    virtual int				 Compare( const SfxPoolItem &rWith ) const;
    virtual int              Compare( const SfxPoolItem &rWith, const ::IntlWrapper& rIntlWrapper ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePresentation,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresentationMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * pIntlWrapper = 0 ) const;

    virtual USHORT           GetVersion( USHORT nFileFormatVersion ) const;
    virtual int              ScaleMetrics( long lMult, long lDiv );
    virtual int              HasMetrics() const;

    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxPoolItem*     Create( SvStream &, USHORT nItemVersion ) const;
    virtual SvStream&        Store( SvStream &, USHORT nItemVersion ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const = 0;

    ULONG                    GetRefCount() const { return nRefCount; }
    inline USHORT			 GetKind() const { return nKind; }

    /** Read in a Unicode string from a streamed byte string representation.

        @param rStream  Some (input) stream.  Its Stream/TargetCharSets must
        be set to correct values!

        @param rString  On success, returns the reconstructed Unicode string.

        @return  True if the string was successfuly read and reconstructed.
     */
    static bool readByteString(SvStream & rStream, UniString & rString);

    /** Write a byte string representation of a Unicode string into a stream.

        @param rStream  Some (output) stream.  Its Stream/TargetCharSets must
        be set to correct values!

        @param rString  Some Unicode string.
     */
    static void writeByteString(SvStream & rStream,
                                UniString const & rString);

    /** Read in a Unicode string from either a streamed Unicode or byte string
        representation.

        @param rStream  Some (input) stream.  If bUnicode is false, its
        Stream/TargetCharSets must be set to correct values!

        @param rString  On success, returns the reconstructed Unicode string.

        @param bUnicode  Whether to read in a stream Unicode (true) or byte
        string (false) representation.

        @return  True if the string was successfuly read and reconstructed.
     */
    static bool readUnicodeString(SvStream & rStream, UniString & rString,
                                  bool bUnicode);

    /** Write a Unicode string representation of a Unicode string into a
        stream.

        @param rStream  Some (output) stream.

        @param rString  Some Unicode string.
     */
    static void writeUnicodeString(SvStream & rStream,
                                   UniString const & rString);

private:
    SfxPoolItem&             operator=( const SfxPoolItem& );    // n.i.!!
};

// -----------------------------------------------------------------------

inline void SfxPoolItem::SetRefCount( ULONG n )
{
    DBG_CHKTHIS( SfxPoolItem, 0 );
    nRefCount = n;
    nKind = 0;
}

inline void SfxPoolItem::SetKind( USHORT n )
{
    DBG_CHKTHIS( SfxPoolItem, 0 );
    nRefCount = SFX_ITEMS_SPECIAL;
    nKind = n;
}

inline ULONG SfxPoolItem::AddRef( ULONG n ) const
{
    DBG_CHKTHIS( SfxPoolItem, 0 );
    DBG_ASSERT( nRefCount <= SFX_ITEMS_MAXREF, "AddRef mit nicht-Pool-Item" );
    DBG_ASSERT( ULONG_MAX - nRefCount > n, "AddRef: Referenzzaehler ueberschlaegt sich" );
    return ( ((SfxPoolItem *)this)->nRefCount += n );
}

inline ULONG SfxPoolItem::ReleaseRef( ULONG n ) const
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

class  SfxVoidItem: public SfxPoolItem
{
    SfxVoidItem & operator=( const SfxVoidItem& ); // not implemented.
public:
                            TYPEINFO();
                            SfxVoidItem( USHORT nWhich );
                            SfxVoidItem( USHORT nWhich, SvStream & );
                            SfxVoidItem( const SfxVoidItem& );
                            ~SfxVoidItem();

    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * = 0 ) const;

    // von sich selbst eine Kopie erzeugen
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
            void            SetWhich(USHORT nWh) { nWhich = nWh; }
};

// -----------------------------------------------------------------------

class  SfxSetItem: public SfxPoolItem
{
    SfxItemSet              *pSet;

    SfxSetItem & operator=( const SfxSetItem& ); // not implemented.

public:
                            TYPEINFO();
                            SfxSetItem( USHORT nWhich, SfxItemSet *pSet );
                            SfxSetItem( USHORT nWhich, const SfxItemSet &rSet );
                            SfxSetItem( const SfxSetItem&, SfxItemPool *pPool = 0 );
                            ~SfxSetItem();

    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * = 0 ) const;

    // von sich selbst eine Kopie erzeugen
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const = 0;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVersion) const = 0;
    virtual SvStream&       Store(SvStream &, USHORT nVer) const;

    const SfxItemSet&       GetItemSet() const
                            { return *pSet; }
    SfxItemSet&             GetItemSet()
                            { return *pSet; }
};

// -----------------------------------------------------------------------

DECL_PTRHINT(, SfxPoolItemHint, SfxPoolItem);

}

#endif // #ifndef _SFXPOOLITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
