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
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <libxml/xmlwriter.h>
#include <typeinfo>
#include <boost/property_tree/ptree.hpp>

#ifdef DBG_UTIL
#include <unordered_set>
#endif

//////////////////////////////////////////////////////////////////////////////
// list of classes derived from SfxPoolItem
// removed, but use 'grep DECLARE_ITEM_TYPE_FUNCTION' to get a complete list
// of Items that may get incarnated during office runtime
//////////////////////////////////////////////////////////////////////////////

#ifdef DBG_UTIL
// static size_t nAllocatedSfxPoolItemCount(0);
static size_t nUsedSfxPoolItemCount(0);
size_t getUsedSfxPoolItemCount() { return nUsedSfxPoolItemCount; }
static std::unordered_set<const SfxPoolItem*>& incarnatedSfxPoolItems()
{
    // Deferred instantiation to avoid initialization-order-fiasco:
    static std::unordered_set<const SfxPoolItem*> items;
    return items;
}
size_t getAllocatedSfxPoolItemCount()
{
    size_t aRetval(0);

    // count globally allocated Items. Exclude Static/DynamicDefaults to
    // get the number without the PoolDefaultItems that will be freed with
    // the Pool. This also excludes the two InvalidOrDisabledItem's used
    // for INVALID_POOL_ITEM/DISABLED_POOL_ITEM
    for (const auto& rCandidate : incarnatedSfxPoolItems())
        if (!rCandidate->isStaticDefault() && !rCandidate->isDynamicDefault())
            aRetval++;
    return aRetval;
}
void listAllocatedSfxPoolItems()
{
    SAL_INFO("svl.items", "ITEM: List of still allocated SfxPoolItems:");
    for (const auto& rCandidate : incarnatedSfxPoolItems())
    {
        if (!rCandidate->isStaticDefault() && !rCandidate->isDynamicDefault())
            SAL_INFO("svl.items", "  ITEM: WhichID: " << rCandidate->Which() << "  SerialNumber: "
                                                      << rCandidate->getSerialNumber()
                                                      << "  Class: " << typeid(*rCandidate).name());
    }
}
#endif

const SfxPoolItem* DefaultItemInstanceManager::find(const SfxPoolItem& rItem) const
{
    auto it = maRegistered.find(rItem.Which());
    if (it == maRegistered.end())
        return nullptr;
    for (const auto& rCandidate : it->second)
        if (*rCandidate == rItem)
            return rCandidate;

    return nullptr;
}

ItemInstanceManager* SfxPoolItem::getItemInstanceManager() const { return nullptr; }

SfxPoolItem::SfxPoolItem(sal_uInt16 const nWhich)
    : m_nRefCount(0)
    , m_nWhich(nWhich)
// , m_eItemType(eType)
#ifdef DBG_UTIL
    , m_nSerialNumber(nUsedSfxPoolItemCount)
#endif
    , m_bStaticDefault(false)
    , m_bDynamicDefault(false)
    , m_bIsSetItem(false)
    , m_bShareable(true)
    , m_bNameOrIndex(false)
#ifdef DBG_UTIL
    , m_bDeleted(false)
#endif
{
#ifdef DBG_UTIL
    // nAllocatedSfxPoolItemCount++;
    nUsedSfxPoolItemCount++;
    incarnatedSfxPoolItems().insert(this);
#endif
    assert(nWhich <= SHRT_MAX);
}

SfxPoolItem::~SfxPoolItem()
{
#ifdef DBG_UTIL
    // nAllocatedSfxPoolItemCount--;
    incarnatedSfxPoolItems().erase(this);
    m_bDeleted = true;
#endif
    assert((m_nRefCount == 0) && "destroying item in use");
}

bool SfxPoolItem::operator==(const SfxPoolItem& rCmp) const
{
    SAL_WARN_IF(rCmp.ItemType() != ItemType(), "svl",
                "comparing different pool item subclasses " << typeid(rCmp).name() << " && "
                                                            << typeid(*this).name());
    assert(rCmp.ItemType() == ItemType() && "comparing different pool item subclasses");
    return true;
}

bool SfxPoolItem::supportsHashCode() const { return false; }

size_t SfxPoolItem::hashCode() const
{
    assert(false
           && "this should never be called, classes should implement both supportsHashCode() and "
              "hashCode(), or neither");
    return 0;
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
bool SfxPoolItem::GetPresentation(
    SfxItemPresentation /*ePresentation*/, // IN:  how we should format
    MapUnit /*eCoreMetric*/, // IN:  current metric of the SfxPoolItems
    MapUnit /*ePresentationMetric*/, // IN:  target metric of the presentation
    OUString& /*rText*/, // OUT: textual representation
    const IntlWrapper&) const
{
    return false;
}

void SfxPoolItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxPoolItem"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"),
                                      BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("typeName"),
                                      BAD_CAST(typeid(*this).name()));
    OUString rText;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    if (GetPresentation(SfxItemPresentation::Complete, MapUnit::Map100thMM, MapUnit::Map100thMM,
                        rText, aIntlWrapper))
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"),
                                          BAD_CAST(rText.toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

boost::property_tree::ptree SfxPoolItem::dumpAsJSON() const
{
    boost::property_tree::ptree aTree;
    return aTree;
}

std::unique_ptr<SfxPoolItem> SfxPoolItem::CloneSetWhich(sal_uInt16 nNewWhich) const
{
    std::unique_ptr<SfxPoolItem> pItem(Clone());
    pItem->SetWhich(nNewWhich);
    return pItem;
}

void SfxPoolItem::ScaleMetrics(tools::Long /*lMult*/, tools::Long /*lDiv*/) {}

bool SfxPoolItem::HasMetrics() const { return false; }

bool SfxPoolItem::QueryValue(css::uno::Any&, sal_uInt8) const
{
    OSL_FAIL("There is no implementation for QueryValue for this item!");
    return false;
}

bool SfxPoolItem::PutValue(const css::uno::Any&, sal_uInt8)
{
    OSL_FAIL("There is no implementation for PutValue for this item!");
    return false;
}

bool areSfxPoolItemPtrsEqual(const SfxPoolItem* pItem1, const SfxPoolItem* pItem2)
{
#ifdef DBG_UTIL
    if (nullptr != pItem1 && nullptr != pItem2 && pItem1->Which() == pItem2->Which()
        && static_cast<const void*>(pItem1) != static_cast<const void*>(pItem2)
        && typeid(*pItem1) == typeid(*pItem2) && *pItem1 == *pItem2)
    {
        SAL_INFO("svl.items", "ITEM: PtrCompare != ContentCompare (!)");
    }
#endif

    // cast to void* to not trigger [loplugin:itemcompare]
    return (static_cast<const void*>(pItem1) == static_cast<const void*>(pItem2));
}

bool SfxPoolItem::areSame(const SfxPoolItem* pItem1, const SfxPoolItem* pItem2)
{
    if (pItem1 == pItem2)
        // pointer compare, this handles already
        // nullptr, INVALID_POOL_ITEM, DISABLED_POOL_ITEM
        // and if any Item is indeed handed over twice
        return true;

    if (nullptr == pItem1 || nullptr == pItem2)
        // one ptr is nullptr, not both, that would
        // have triggered above
        return false;

    if (pItem1->Which() != pItem2->Which())
        // WhichIDs differ (fast)
        return false;

    if (pItem1->ItemType() != pItem2->ItemType())
        // types differ (fast)
        return false;

    // return content compare using operator== at last
    return *pItem1 == *pItem2;
}

bool SfxPoolItem::areSame(const SfxPoolItem& rItem1, const SfxPoolItem& rItem2)
{
    if (&rItem1 == &rItem2)
        // still use pointer compare, this handles already
        // nullptr, INVALID_POOL_ITEM, SfxVoidItem
        // and if any Item is indeed handed over twice
        return true;

    if (rItem1.Which() != rItem2.Which())
        // WhichIDs differ (fast)
        return false;

    if (rItem1.ItemType() != rItem2.ItemType())
        // types differ (fast)
        return false;

    // return content compare using operator== at last
    return rItem1 == rItem2;
}

namespace
{
class InvalidOrDisabledItem final : public SfxPoolItem
{
    virtual bool operator==(const SfxPoolItem&) const override { return true; }
    virtual SfxPoolItem* Clone(SfxItemPool*) const override { return nullptr; }

public:
    // make it StaticDefaultItem to process similar to these
    // which is plausible (never change and are not allowed to)
    DECLARE_ITEM_TYPE_FUNCTION(InvalidOrDisabledItem)
    InvalidOrDisabledItem()
        : SfxPoolItem(0)
    {
        setStaticDefault();
    }
};
InvalidOrDisabledItem aInvalidItem;
InvalidOrDisabledItem aDisabledItem;
}

SfxPoolItem const* const INVALID_POOL_ITEM = &aInvalidItem;
SfxPoolItem const* const DISABLED_POOL_ITEM = &aDisabledItem;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
