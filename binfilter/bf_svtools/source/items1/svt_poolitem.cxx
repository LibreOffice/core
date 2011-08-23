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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <bf_svtools/poolitem.hxx>

#include <tools/stream.hxx>

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxPoolItem)
DBG_NAME(SfxVoidItem)

BYTE nSfxFlag8Val[8] =
{
    1, 2, 4, 8, 16, 32, 64, 128
};

USHORT nSfxFlag16Val[16] =
{
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
    1024, 2048, 4096, 8192, 16384, 32768
};

ULONG nSfxFlag32Val[32] =
{
    0x1L, 0x2L, 0x4L, 0x8L,
    0x10L, 0x20L, 0x40L, 0x80L,
    0x100L, 0x200L, 0x400L, 0x800L,
    0x1000L, 0x2000L, 0x40000L, 0x8000L,
    0x10000L, 0x20000L, 0x40000L, 0x80000L,
    0x100000L, 0x200000L, 0x400000L, 0x800000L,
    0x1000000L, 0x2000000L, 0x4000000L, 0x8000000L,
    0x10000000L, 0x20000000L, 0x40000000L, 0x80000000L
};

// RTTI ------------------------------------------------------------------

TYPEINIT0(SfxPoolItem);
TYPEINIT1(SfxVoidItem, SfxPoolItem);
// @@@ TYPEINIT1(SfxInvalidItem, SfxPoolItem);
TYPEINIT1(SfxSetItem, SfxPoolItem);
// @@@ TYPEINIT1(SfxItemChangedHint, SfxHint);

// ------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 1
static ULONG nItemCount = 0;

const char* pw1 = "Wow! 10.000 items!";
const char* pw2 = "Wow! 100.000 items!";
const char* pw3 = "Wow! 1.000.000 items!";
const char* pw4 = "Wow! 50.000.000 items!";
const char* pw5 = "Wow! 10.000.000 items!";
#endif

IMPL_PTRHINT(SfxPoolItemHint,SfxPoolItem)

// SfxPoolItem -----------------------------------------------------------
SfxPoolItem::SfxPoolItem( USHORT nW )
    : nRefCount( 0 ),
      nWhich( nW )
      , nKind( 0 )
{
    DBG_CTOR(SfxPoolItem, 0);
    DBG_ASSERT(nW <= SHRT_MAX, "Which Bereich ueberschritten");
#if OSL_DEBUG_LEVEL > 1
    ++nItemCount;
    if ( pw1 && nItemCount>=10000 )
    {
        DBG_WARNING( pw1 );
        pw1 = NULL;
    }
    if ( pw2 && nItemCount>=100000 )
    {
        DBG_WARNING( pw2 );
        pw2 = NULL;
    }
    if ( pw3 && nItemCount>=1000000 )
    {
        DBG_WARNING( pw3 );
        pw3 = NULL;
    }
    if ( pw4 && nItemCount>=5000000 )
    {
        DBG_WARNING( pw4 );
        pw4 = NULL;
    }
    if ( pw5 && nItemCount>=10000000 )
    {
        DBG_WARNING( pw5 );
        pw5 = NULL;
    }
#endif
}

// -----------------------------------------------------------------------
SfxPoolItem::SfxPoolItem( const SfxPoolItem& rCpy )
    : nRefCount( 0 ),				// wird ja ein neues Object!
      nWhich( rCpy.Which() )	// Funktion rufen wg. ChkThis()
      , nKind( 0 )
{
    DBG_CTOR(SfxPoolItem, 0);
#if OSL_DEBUG_LEVEL > 1
    ++nItemCount;
    if ( pw1 && nItemCount>=10000 )
    {
        DBG_WARNING( pw1 );
        pw1 = NULL;
    }
    if ( pw2 && nItemCount>=100000 )
    {
        DBG_WARNING( pw2 );
        pw2 = NULL;
    }
    if ( pw3 && nItemCount>=1000000 )
    {
        DBG_WARNING( pw3 );
        pw3 = NULL;
    }
    if ( pw4 && nItemCount>=5000000 )
    {
        DBG_WARNING( pw4 );
        pw4 = NULL;
    }
    if ( pw5 && nItemCount>=10000000 )
    {
        DBG_WARNING( pw5 );
        pw5 = NULL;
    }
#endif
}

// ------------------------------------------------------------------------
SfxPoolItem::~SfxPoolItem()
{
    DBG_DTOR(SfxPoolItem, 0);
    DBG_ASSERT(nRefCount == 0 || nRefCount > SFX_ITEMS_MAXREF, "destroying item in use" );
#if OSL_DEBUG_LEVEL > 1
    --nItemCount;
#endif
}

// ------------------------------------------------------------------------
int SfxPoolItem::Compare( const SfxPoolItem& ) const
{
    return 0;
}

// ------------------------------------------------------------------------
int SfxPoolItem::Compare( const SfxPoolItem& rWith, const ::IntlWrapper& ) const
{
    return Compare( rWith );
}

// ------------------------------------------------------------------------
int SfxPoolItem::operator==( const SfxPoolItem& rCmp ) const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return rCmp.Type() == Type();
}

// -----------------------------------------------------------------------
#ifndef TF_POOLABLE

int SfxPoolItem::IsPoolable() const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return TRUE;
}
#endif

// -----------------------------------------------------------------------
SfxPoolItem* SfxPoolItem::Create(SvStream &, USHORT) const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return Clone(0);
}

// -----------------------------------------------------------------------
USHORT SfxPoolItem::GetVersion( USHORT ) const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return 0;
}

// -----------------------------------------------------------------------
SvStream& SfxPoolItem::Store(SvStream &rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return rStream;
}

//============================================================================
// static
bool SfxPoolItem::readByteString(SvStream & rStream, UniString & rString)
{
    rStream.ReadByteString(rString);
    return rStream.GetError() == ERRCODE_NONE;
}

//============================================================================
// static
void SfxPoolItem::writeByteString(SvStream & rStream,
                                  UniString const & rString)
{
    rStream.WriteByteString(rString);
}

//============================================================================
// static
bool SfxPoolItem::readUnicodeString(SvStream & rStream, UniString & rString,
                                    bool bUnicode)
{
    rStream.ReadByteString(rString,
                           bUnicode ? RTL_TEXTENCODING_UCS2 :
                                      rStream.GetStreamCharSet());
    return rStream.GetError() == ERRCODE_NONE;
}

//============================================================================
// static
void SfxPoolItem::writeUnicodeString(SvStream & rStream,
                                     UniString const & rString)
{
    rStream.WriteByteString(rString, RTL_TEXTENCODING_UCS2);
}

// ------------------------------------------------------------------------
SfxItemPresentation SfxPoolItem::GetPresentation
(
    SfxItemPresentation	/*ePresentation*/,       // IN:  wie formatiert werden soll
    SfxMapUnit			/*eCoreMetric*/,		 // IN:  Ma\seinheit des SfxPoolItems
    SfxMapUnit			/*ePresentationMetric*/, // IN:  Wunsch-Ma\einheit der Darstellung
    XubString&			/*rText*/,				 // OUT: textuelle Darstellung
    const ::IntlWrapper *
) 	const

/*	[Beschreibung]

    "Uber diese virtuelle Methode kann von den SfxPoolItem-Subklassen
    eine textuelle Datstellung des Wertes erhalten werden. Sie sollte
    von allen UI-relevanten SfxPoolItem-Subklassen "uberladen werden.

    Da die Ma\seinheit des Wertes im SfxItemPool nur "uber
    <SfxItemPool::GetMetric(USHORT)const> erfragbar ist, und nicht etwa
    in der SfxPoolItem-Instanz oder -Subklasse  verf"ugbar ist, wird die
    eigene Ma\seinheit als 'eCoreMetric' "ubergeben.

    Die darzustellende Ma\seinheit wird als 'ePresentationMetric'
    "ubergeben.


    [R"uckgabewert]

    SfxItemPresentation		SFX_ITEM_PRESENTATION_NONE
                            es konnte keine Text-Darstellung erzeugt werden

                            SFX_ITEM_PRESENTATION_NAMELESS
                            es konnte eine Text-Darstellung (ggf. mit
                            Ma\seinheit) erzeugt werden, die jedoch keine
                            semantische Bedeutung enth"alt

                            SFX_ITEM_PRESENTATION_COMPLETE
                            es konnte eine komplette Text-Darstellung mit
                            semantischer Bedeutung (und ggf. Ma\seinheit)
                            erzeugt werden


    [Beispiele]

    pSvxFontItem->GetPresentation( SFX_PRESENTATION_NAMELESS, ... )
        "12pt" mit return SFX_ITEM_PRESENTATION_NAMELESS

    pSvxColorItem->GetPresentation( SFX_PRESENTATION_COMPLETE, ... )
        "rot" mit return SFX_ITEM_PRESENTATION_NAMELESS
        (das das SvxColorItem nicht wei\s, was f"ur eine Farbe es darstellt,
        kann es keinen Namen angeben, was durch den Returnwert mitgeteilt wird.

    pSvxBorderItem->GetPresentation( SFX_PRESENTATION_COMPLETE, ... )
        "1cm oberer Rand, 2cm linker Rand, 0,2cm unterer Rand, ..."
*/

{
    return SFX_ITEM_PRESENTATION_NONE;
}

// SfxVoidItem ------------------------------------------------------------
SfxVoidItem::SfxVoidItem( USHORT which ):
    SfxPoolItem(which)
{
    DBG_CTOR(SfxVoidItem, 0);
}

// SfxVoidItem ------------------------------------------------------------
SfxVoidItem::SfxVoidItem( const SfxVoidItem& rCopy):
    SfxPoolItem(rCopy)
{
    DBG_CTOR(SfxVoidItem, 0);
}

// ------------------------------------------------------------------------
int SfxVoidItem::operator==( const SfxPoolItem&
#ifdef DBG_UTIL
rCmp
#endif
) const
{
    DBG_CHKTHIS(SfxVoidItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rCmp ), "unequal type" );
    return TRUE;
}

// ------------------------------------------------------------------------
SfxItemPresentation SfxVoidItem::GetPresentation
(
    SfxItemPresentation 	/*ePresentation*/,
    SfxMapUnit				/*eCoreMetric*/,
    SfxMapUnit				/*ePresentationMetric*/,
    XubString& 				rText,
    const ::IntlWrapper *
)	const
{
    DBG_CHKTHIS(SfxVoidItem, 0);
    rText.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Void"));
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// ------------------------------------------------------------------------
SfxPoolItem* SfxVoidItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxVoidItem, 0);
    return new SfxVoidItem(*this);
}

// ------------------------------------------------------------------------
int SfxPoolItem::ScaleMetrics( long /*lMult*/, long /*lDiv*/ )
{
    return 0;
}

// ------------------------------------------------------------------------
int SfxPoolItem::HasMetrics() const
{
    return 0;
}

// -----------------------------------------------------------------------
#if 0  /* @@@ NOT USED @@@ */
void SfxPoolItem::GetVersion() const
{
    DBG_ERROR( "dummy called" );
}

// -----------------------------------------------------------------------
void SfxPoolItem::Store(SvStream &rStream) const
{
    DBG_ERROR( "dummy called" );
}
#endif /* @@@ NOT USED @@@ */

// -----------------------------------------------------------------------

bool SfxPoolItem::QueryValue( com::sun::star::uno::Any&, BYTE ) const
{
    DBG_ERROR("There is no implementation for QueryValue for this item!");
    return false;
}

// -----------------------------------------------------------------------

bool SfxPoolItem::PutValue( const com::sun::star::uno::Any&, BYTE )
{
    DBG_ERROR("There is no implementation for PutValue for this item!");
    return false;
}

SfxVoidItem::~SfxVoidItem()
{
    DBG_DTOR(SfxVoidItem, 0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
