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
#include <osl/diagnose.h>
#include <libxml/xmlwriter.h>

TYPEINIT0(SfxPoolItem);
TYPEINIT1(SfxVoidItem, SfxPoolItem);
// @@@ TYPEINIT1(SfxInvalidItem, SfxPoolItem);
TYPEINIT1(SfxSetItem, SfxPoolItem);
// @@@ TYPEINIT1(SfxItemChangedHint, SfxHint);


#if OSL_DEBUG_LEVEL > 1
static sal_uLong nItemCount = 0;

const char* pw1 = "Wow! 10.000 items!";
const char* pw2 = "Wow! 100.000 items!";
const char* pw3 = "Wow! 1.000.000 items!";
const char* pw4 = "Wow! 50.000.000 items!";
const char* pw5 = "Wow! 10.000.000 items!";
#endif

SfxPoolItem::SfxPoolItem(sal_uInt16 const nWhich)
    : m_nRefCount(0)
    , m_nWhich(nWhich)
    , m_nKind(SFX_ITEMS_NONE)
{
    DBG_ASSERT(nWhich <= SHRT_MAX, "invalid WhichId");
#if OSL_DEBUG_LEVEL > 1
    ++nItemCount;
    if ( pw1 && nItemCount>=10000 )
    {
        SAL_INFO( "svl", pw1 );
        pw1 = NULL;
    }
    if ( pw2 && nItemCount>=100000 )
    {
        SAL_INFO( "svl", pw2 );
        pw2 = NULL;
    }
    if ( pw3 && nItemCount>=1000000 )
    {
        SAL_INFO( "svl", pw3 );
        pw3 = NULL;
    }
    if ( pw4 && nItemCount>=5000000 )
    {
        SAL_INFO( "svl", pw4 );
        pw4 = NULL;
    }
    if ( pw5 && nItemCount>=10000000 )
    {
        SAL_INFO( "svl", pw5 );
        pw5 = NULL;
    }
#endif
}


SfxPoolItem::SfxPoolItem( const SfxPoolItem& rCpy )
    : m_nRefCount(0) // don't copy that
    , m_nWhich(rCpy.Which()) // call function because of ChkThis() (WTF does that mean?)
    , m_nKind( SFX_ITEMS_NONE )
{
#if OSL_DEBUG_LEVEL > 1
    ++nItemCount;
    if ( pw1 && nItemCount>=10000 )
    {
        SAL_INFO( "svl", pw1 );
        pw1 = NULL;
    }
    if ( pw2 && nItemCount>=100000 )
    {
        SAL_INFO( "svl", pw2 );
        pw2 = NULL;
    }
    if ( pw3 && nItemCount>=1000000 )
    {
        SAL_INFO( "svl", pw3 );
        pw3 = NULL;
    }
    if ( pw4 && nItemCount>=5000000 )
    {
        SAL_INFO( "svl", pw4 );
        pw4 = NULL;
    }
    if ( pw5 && nItemCount>=10000000 )
    {
        SAL_INFO( "svl", pw5 );
        pw5 = NULL;
    }
#endif
}


SfxPoolItem::~SfxPoolItem()
{
    DBG_ASSERT(m_nRefCount == 0 || m_nRefCount > SFX_ITEMS_MAXREF,
            "destroying item in use");
#if OSL_DEBUG_LEVEL > 1
    --nItemCount;
#endif
}


bool SfxPoolItem::operator==( const SfxPoolItem& rCmp ) const
{
    return rCmp.Type() == Type();
}


SfxPoolItem* SfxPoolItem::Create(SvStream &, sal_uInt16) const
{
    return Clone();
}


sal_uInt16 SfxPoolItem::GetVersion( sal_uInt16 ) const
{
    return 0;
}


SvStream& SfxPoolItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    return rStream;
}

/**
 * This virtual method allows to get a textual representation of the value
 * for the SfxPoolItem subclasses. It should be overridden by all UI-relevant
 * SfxPoolItem subclasses.
 *
 * Because the unit of measure of the value in the SfxItemPool is only
 * queryable via @see SfxItemPool::GetMetric(sal_uInt16) const (and not
 * via the SfxPoolItem instance or subclass, the own unit of measure is
 * passed to 'eCoreMetric'.
 *
 * The corresponding unit of measure is passed as 'ePresentationMetric'.
 *
 *
 * @return SfxItemPresentation     SFX_ITEM_PRESENTATION_NAMELESS
 *                                 A textual representation (if applicable
 *                                 with a unit of measure) could be created,
 *                                 but it doesn't contain any semantic meaning
 *
 *                                 SFX_ITEM_PRESENTATION_COMPLETE
 *                                 A complete textual representation could be
 *                                 created with semantic meaning (if applicable
 *                                 with unit of measure)
 *
 * Example:
 *
 *    pSvxFontItem->GetPresentation( SFX_PRESENTATION_NAMELESS, ... )
 *      "12pt" with return SFX_ITEM_PRESENTATION_NAMELESS
 *
 *    pSvxColorItem->GetPresentation( SFX_PRESENTATION_COMPLETE, ... )
 *        "red" with return SFX_ITEM_PRESENTATION_NAMELESS
 *        Because the SvxColorItem does not know which color it represents
 *        it cannot provide a name, which is communicated by the return value
 *
 *    pSvxBorderItem->GetPresentation( SFX_PRESENTATION_COMPLETE, ... )
 *        "1cm top border, 2cm left border, 0.2cm bottom border, ..."
 */
bool SfxPoolItem::GetPresentation
(
    SfxItemPresentation /*ePresentation*/,       // IN:  how we should format
    SfxMapUnit          /*eCoreMetric*/,         // IN:  current metric of the SfxPoolItems
    SfxMapUnit          /*ePresentationMetric*/, // IN:  target metric of the presentation
    OUString&           /*rText*/,               // OUT: textual representation
    const IntlWrapper *
)   const
{
    return false;
}

void SfxPoolItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sfxPoolItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

SfxVoidItem::SfxVoidItem( sal_uInt16 which ):
    SfxPoolItem(which)
{
}

SfxVoidItem::SfxVoidItem( const SfxVoidItem& rCopy):
    SfxPoolItem(rCopy)
{
}


bool SfxVoidItem::operator==( const SfxPoolItem& rCmp ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rCmp ), "unequal type" );
    return true;
}


bool SfxVoidItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *
)   const
{
    rText = "Void";
    return true;
}


SfxPoolItem* SfxVoidItem::Clone(SfxItemPool *) const
{
    return new SfxVoidItem(*this);
}

bool SfxPoolItem::ScaleMetrics( long /*lMult*/, long /*lDiv*/ )
{
    return false;
}


bool SfxPoolItem::HasMetrics() const
{
    return false;
}



bool SfxPoolItem::QueryValue( css::uno::Any&, sal_uInt8 ) const
{
    OSL_FAIL("There is no implementation for QueryValue for this item!");
    return false;
}



bool SfxPoolItem::PutValue( const css::uno::Any&, sal_uInt8 )
{
    OSL_FAIL("There is no implementation for PutValue for this item!");
    return false;
}

SfxVoidItem::~SfxVoidItem()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
