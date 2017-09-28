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
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <osl/diagnose.h>
#include <libxml/xmlwriter.h>
#include <typeinfo>

SfxPoolItem::SfxPoolItem(sal_uInt16 const nWhich)
    : m_nRefCount(0)
    , m_nWhich(nWhich)
    , m_nKind(SfxItemKind::NONE)
{
    assert(nWhich <= SHRT_MAX);
}


SfxPoolItem::~SfxPoolItem()
{
    assert((m_nRefCount == 0 || m_nRefCount > SFX_ITEMS_MAXREF)
            && "destroying item in use");
}


bool SfxPoolItem::operator==( const SfxPoolItem& rCmp ) const
{
    return typeid(rCmp)  == typeid(*this);
}


SfxPoolItem* SfxPoolItem::Create(SvStream &, sal_uInt16) const
{
    assert(!"this item is not serialisable");
    return Clone();
}


sal_uInt16 SfxPoolItem::GetVersion( sal_uInt16 ) const
{
    return 0;
}


SvStream& SfxPoolItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    assert(!"this item is not serialisable");
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
 * @return SfxItemPresentation     SfxItemPresentation::Nameless
 *                                 A textual representation (if applicable
 *                                 with a unit of measure) could be created,
 *                                 but it doesn't contain any semantic meaning
 *
 *                                 SfxItemPresentation::Complete
 *                                 A complete textual representation could be
 *                                 created with semantic meaning (if applicable
 *                                 with unit of measure)
 *
 * Example:
 *
 *    pSvxFontItem->GetPresentation( SFX_PRESENTATION_NAMELESS, ... )
 *      "12pt" with return SfxItemPresentation::Nameless
 *
 *    pSvxColorItem->GetPresentation( SFX_PRESENTATION_COMPLETE, ... )
 *        "red" with return SfxItemPresentation::Nameless
 *        Because the SvxColorItem does not know which color it represents
 *        it cannot provide a name, which is communicated by the return value
 *
 *    pSvxBorderItem->GetPresentation( SFX_PRESENTATION_COMPLETE, ... )
 *        "1cm top border, 2cm left border, 0.2cm bottom border, ..."
 */
bool SfxPoolItem::GetPresentation
(
    SfxItemPresentation /*ePresentation*/,       // IN:  how we should format
    MapUnit             /*eCoreMetric*/,         // IN:  current metric of the SfxPoolItems
    MapUnit             /*ePresentationMetric*/, // IN:  target metric of the presentation
    OUString&           /*rText*/,               // OUT: textual representation
    const IntlWrapper&
)   const
{
    return false;
}

void SfxPoolItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SfxPoolItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("typeName"), BAD_CAST(typeid(*this).name()));
    OUString rText;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    if (GetPresentation( SfxItemPresentation::Complete, MapUnit::Map100thMM, MapUnit::Map100thMM, rText, aIntlWrapper))
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(rText.getStr()));
    xmlTextWriterEndElement(pWriter);
}

SfxPoolItem* SfxPoolItem::CloneSetWhich( sal_uInt16 nNewWhich ) const
{
    SfxPoolItem* pItem = Clone();
    pItem->SetWhich(nNewWhich);
    return pItem;
}

bool SfxPoolItem::IsVoidItem() const
{
    return false;
}

SfxPoolItem* SfxVoidItem::CreateDefault()
{
    return new SfxVoidItem(0);
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
    assert(SfxPoolItem::operator==(rCmp));
    (void) rCmp;
    return true;
}


bool SfxVoidItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    MapUnit                 /*eCoreMetric*/,
    MapUnit                 /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper&
)   const
{
    rText = "Void";
    return true;
}

void SfxVoidItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SfxVoidItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

SfxPoolItem* SfxVoidItem::Clone(SfxItemPool *) const
{
    return new SfxVoidItem(*this);
}

bool SfxVoidItem::IsVoidItem() const
{
    return true;
}

void SfxPoolItem::ScaleMetrics( long /*lMult*/, long /*lDiv*/ )
{
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
