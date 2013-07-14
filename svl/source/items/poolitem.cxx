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


#include <svl/poolitem.hxx>
#include <tools/stream.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxPoolItem)
DBG_NAME(SfxVoidItem)
// @@@ DBG_NAME(SfxInvalidItem);
DBG_NAME(SfxItemHandle)

// RTTI ------------------------------------------------------------------

TYPEINIT0(SfxPoolItem);
TYPEINIT1(SfxVoidItem, SfxPoolItem);
// @@@ TYPEINIT1(SfxInvalidItem, SfxPoolItem);
TYPEINIT1(SfxSetItem, SfxPoolItem);
// @@@ TYPEINIT1(SfxItemChangedHint, SfxHint);

// ------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 1
static sal_uLong nItemCount = 0;

const char* pw1 = "Wow! 10.000 items!";
const char* pw2 = "Wow! 100.000 items!";
const char* pw3 = "Wow! 1.000.000 items!";
const char* pw4 = "Wow! 50.000.000 items!";
const char* pw5 = "Wow! 10.000.000 items!";
#endif

IMPL_PTRHINT(SfxPoolItemHint,SfxPoolItem)

// SfxPoolItem -----------------------------------------------------------
SfxPoolItem::SfxPoolItem(sal_uInt16 const nWhich)
    : m_nRefCount(0)
    , m_nWhich(nWhich)
    , m_nKind(0)
{
    DBG_CTOR(SfxPoolItem, 0);
    DBG_ASSERT(nWhich <= SHRT_MAX, "invalid WhichId");
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
    : m_nRefCount(0) // don't copy that
    , m_nWhich(rCpy.Which()) // call function because of ChkThis() (WTF does that mean?)
    , m_nKind( 0 )
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
    DBG_ASSERT(m_nRefCount == 0 || m_nRefCount > SFX_ITEMS_MAXREF,
            "destroying item in use");
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
int SfxPoolItem::Compare( const SfxPoolItem& rWith, const IntlWrapper& ) const
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
SfxPoolItem* SfxPoolItem::Create(SvStream &, sal_uInt16) const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return Clone(0);
}

// -----------------------------------------------------------------------
sal_uInt16 SfxPoolItem::GetVersion( sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return 0;
}

// -----------------------------------------------------------------------
SvStream& SfxPoolItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxPoolItem, 0);
    return rStream;
}

// static
OUString SfxPoolItem::readByteString(SvStream& rStream)
{
    return rStream.ReadUniOrByteString(rStream.GetStreamCharSet());
}

// static
void SfxPoolItem::writeByteString(SvStream & rStream, const OUString& rString)
{
    rStream.WriteUniOrByteString(rString, rStream.GetStreamCharSet());
}

// static
OUString SfxPoolItem::readUnicodeString(SvStream & rStream, bool bUnicode)
{
    return rStream.ReadUniOrByteString(bUnicode ? RTL_TEXTENCODING_UCS2 :
                                      rStream.GetStreamCharSet());
}

// static
void SfxPoolItem::writeUnicodeString(SvStream & rStream, const OUString& rString)
{
    rStream.WriteUniOrByteString(rString, RTL_TEXTENCODING_UCS2);
}

// ------------------------------------------------------------------------
SfxItemPresentation SfxPoolItem::GetPresentation
(
    SfxItemPresentation /*ePresentation*/,       // IN:  wie formatiert werden soll
    SfxMapUnit          /*eCoreMetric*/,         // IN:  Ma\seinheit des SfxPoolItems
    SfxMapUnit          /*ePresentationMetric*/, // IN:  Wunsch-Ma\einheit der Darstellung
    OUString&           /*rText*/,               // OUT: textuelle Darstellung
    const IntlWrapper *
)   const

/*  [Beschreibung]

    "Uber diese virtuelle Methode kann von den SfxPoolItem-Subklassen
    eine textuelle Datstellung des Wertes erhalten werden. Sie sollte
    von allen UI-relevanten SfxPoolItem-Subklassen "uberladen werden.

    Da die Ma\seinheit des Wertes im SfxItemPool nur "uber
    <SfxItemPool::GetMetric(sal_uInt16)const> erfragbar ist, und nicht etwa
    in der SfxPoolItem-Instanz oder -Subklasse  verf"ugbar ist, wird die
    eigene Ma\seinheit als 'eCoreMetric' "ubergeben.

    Die darzustellende Ma\seinheit wird als 'ePresentationMetric'
    "ubergeben.


    [R"uckgabewert]

    SfxItemPresentation     SFX_ITEM_PRESENTATION_NONE
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
SfxVoidItem::SfxVoidItem( sal_uInt16 which ):
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
    return sal_True;
}

// ------------------------------------------------------------------------
SfxItemPresentation SfxVoidItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *
)   const
{
    DBG_CHKTHIS(SfxVoidItem, 0);
    rText = "Void";
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// ------------------------------------------------------------------------
SfxPoolItem* SfxVoidItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxVoidItem, 0);
    return new SfxVoidItem(*this);
}

// SfxInvalidItem ---------------------------------------------------------

// SfxItemHandle ----------------------------------------------------------
SfxItemHandle::SfxItemHandle(SfxPoolItem &rItem):
    pRef(new sal_uInt16(1)),
    pItem(rItem.Clone(0))
{
    DBG_CTOR(SfxItemHandle, 0);
}

// ------------------------------------------------------------------------
SfxItemHandle::SfxItemHandle(const SfxItemHandle &rCopy):
    pRef(rCopy.pRef),
    pItem(rCopy.pItem)
{
    DBG_CTOR(SfxItemHandle, 0);
    ++(*pRef);
}

// ------------------------------------------------------------------------
const SfxItemHandle &SfxItemHandle::operator=(const SfxItemHandle &rCopy)
{
    DBG_CHKTHIS(SfxItemHandle, 0);
    if(&rCopy == this || pItem == rCopy.pItem)
        return *this;
    --(*pRef);
    if(!(*pRef))
    {
        delete pItem;
        pItem = 0;
    }
    pRef = rCopy.pRef;
    ++(*pRef);
    pItem = rCopy.pItem;
    return *this;
}

// ------------------------------------------------------------------------
SfxItemHandle::~SfxItemHandle()
{
    DBG_DTOR(SfxItemHandle, 0);
    --(*pRef);
    if(!(*pRef)) {
        delete pRef; pRef = 0;
        delete pItem; pItem = 0;
    }
}

// ------------------------------------------------------------------------
bool SfxPoolItem::ScaleMetrics( long /*lMult*/, long /*lDiv*/ )
{
    return false;
}

// ------------------------------------------------------------------------
bool SfxPoolItem::HasMetrics() const
{
    return false;
}

// -----------------------------------------------------------------------

bool SfxPoolItem::QueryValue( com::sun::star::uno::Any&, sal_uInt8 ) const
{
    OSL_FAIL("There is no implementation for QueryValue for this item!");
    return false;
}

// -----------------------------------------------------------------------

bool SfxPoolItem::PutValue( const com::sun::star::uno::Any&, sal_uInt8 )
{
    OSL_FAIL("There is no implementation for PutValue for this item!");
    return false;
}

SfxVoidItem::~SfxVoidItem()
{
    DBG_DTOR(SfxVoidItem, 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
