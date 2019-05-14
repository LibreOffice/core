/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/base/ItemBase.hxx>
#include <item/base/ItemControlBlock.hxx>
#include <cassert>
#include <osl/diagnose.h>
#include <libxml/xmlwriter.h>
#include <unotools/syslocale.hxx>
#include <unotools/intlwrapper.hxx>

///////////////////////////////////////////////////////////////////////////////
// derived items from : public SfxPoolItem -> 123
// derived from these not yet evaluated. do on demand...
/*

class SbxItem : public SfxPoolItem
-> basctl::Item::Sbx
-> SID_BASICIDE_ARG_SBX
-> Done!
-> Now moved to use ::ItemSet and new ExecuteList2, no more SlotSet

class SfxInt16Item
-> Item::CntInt16
-> SID_ATTR_TRANSFORM_ANCHOR -> need own type to replace in ItemSet -> Item::TransformAnchor
-> replace using TransformAnchor and ItemSet -> done!

defs from sfx2\sdi\sfxitems.sdi may be a good hint which items to convert first (?)
these are:
    item void       SfxVoidItem;
    item BOOL       SfxBoolItem;
    item INT32      SfxUInt16Item;
    item INT16      SfxInt16Item;
    item INT32      SfxUInt32Item;
    item INT32      SfxInt32Item;
    item String     SfxStringItem;
    item BYTE       SfxByteItem;
    item INT16      SfxEnumItem;

    item String     SfxObjectItem;          //! Dummy
    item String     SfxTemplateItem;        //! Dummy
    item String     SfxMacroInfoItem;       //! Dummy
    item String     SfxImageItem;           //! Dummy
    item String     SfxObjectShellItem      //! Dummy
    item String     SfxUnoAnyItem           //! Dummy
    item String     SfxUnoFrameItem         //! Dummy
    item String     SfxWatermarkItem        //! Dummy

    item Point SfxPointItem;
    item Rectangle SfxRectangleItem;
    item DocInfo SfxDocumentInfoItem;
    item SvxSearch SvxSearchItem;
    item SvxSize SvxSizeItem;
    item SfxScriptOrganizer SfxScriptOrganizerItem;
    item String     SvxClipboardFormatItem;    //! Dummy
    item SvxZoom SvxZoomItem;

class SvxChartColorTableItem : public SfxPoolItem
-> only uses SID_SCH_EDITOPTIONS, currently has non-static members, but no need for them
-> change this in master first
SID_SCH_EDITOPTIONS uses SfxItemPool from SfxShell::GetPool(), so maybe use
SfxShell::SetPool and debug/break to check who/how and what kind of pool is to be set/used
in that cases
Uses
    const SfxPoolItem* SfxItemSet::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
which sets the item, adds it to the SfxItemPool using
                    const SfxPoolItem& rNew = m_pPool->Put( rItem, nWhich );
and does trigger ::Changed broadcasts based on SfxItemPool::IsWhich (nId <= SFX_WHICH_MAX, SFX_WHICH_MAX = 4999)
Uses
    const SfxPoolItem& SfxItemPool::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
that checks
    bool bSID = IsSlot(nWhich);
and if yes - so for all SlotItems - (nId > SFX_WHICH_MAX, , SFX_WHICH_MAX = 4999) just
- clones the item using SfxPoolItem *pPoolItem = rItem.Clone(pImpl->mpMaster);
- adds the item using AddRef( *pPoolItem );
These Items are NOT added to the ItemLists in The SfxItemPool(!), only to the SfxItemSet
using the SfxItemPool::Put call, only their RefCount keeps them alive.
Nonetheless these SlotItems STILL depend on the SfxItem-RANGES defined in the SfxItemSet
-> SLOT ITEMS do NOT get POOLED (IsItemPoolable/IsPooledItem/...)
-> SLOT ITEMS can be put in *any* ItemPool - due to not using the pooling mechanism
*/
///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    bool ItemBase::putAnyValues(const AnyIDArgs& rArgs)
    {
        // default implementation does spread given arguments to putAnyValue-call
        for(const auto& arg : rArgs)
        {
            if(!putAnyValue(arg.first, arg.second))
            {
                return false;
            }
        }

        return true;
    }

    bool ItemBase::isInvalidateItem() const
    {
        return false;
    }

    bool ItemBase::isDisableItem() const
    {
        return false;
    }

    bool ItemBase::putAnyValue(const css::uno::Any& /*rVal*/, sal_uInt8 /*nMemberId*/)
    {
        // default has nothing to to
        OSL_FAIL("There is no implementation for putAnyValue for this item!");
        return false;
    }

    ItemBase::ItemBase(ItemControlBlock& rItemControlBlock)
    :   m_rItemControlBlock(rItemControlBlock)
    {
    }

    ItemBase::~ItemBase()
    {
    }

    ItemBase::ItemBase(const ItemBase& rRef)
    :   m_rItemControlBlock(rRef.m_rItemControlBlock)
    {
    }

    ItemBase& ItemBase::operator=(const ItemBase& rRef)
    {
        // nothing to copy - type has to be identical
        assert(typeid(const_cast<ItemBase&>(rRef)) == typeid(*this) && "Unequal types in ItemBase::operator= not allowed (!)");
        return *this;
    }

    bool ItemBase::operator==(const ItemBase& rRef) const
    {
        // ptr-compare
        assert(typeid(rRef) == typeid(*this) && "Unequal types in ItemBase::operator== not allowed (!)");
        return (this == &rRef);
    }

    bool ItemBase::operator!=(const ItemBase& rRef) const
    {
        assert(typeid(rRef) == typeid(*this) && "Unequal types in ItemBase::operator!= not allowed (!)");
        return !(*this == rRef);
    }

    std::unique_ptr<ItemBase> ItemBase::clone() const
    {
        // callback to ItemControlBlock
        return m_rItemControlBlock.clone(*this);
    }

    const ItemBase& ItemBase::getDefault() const
    {
        // callback to ItemControlBlock
        return m_rItemControlBlock.getDefault();
    }

    bool ItemBase::isDefault() const
    {
        // callback to ItemControlBlock
        return m_rItemControlBlock.isDefault(*this);
    }

    bool isDefault(const ItemBase& rCandidate)
    {
        // callback to ItemControlBlock
        return rCandidate.m_rItemControlBlock.isDefault(rCandidate);
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
    bool ItemBase::getPresentation
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

    void ItemBase::scaleMetrics( long /*lMult*/, long /*lDiv*/ )
    {
    }

    bool ItemBase::queryValue( css::uno::Any&, sal_uInt8 ) const
    {
        OSL_FAIL("There is no implementation for queryValue for this item!");
        return false;
    }

    void ItemBase::dumpAsXml(xmlTextWriterPtr pWriter) const
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("ItemBase"));
        // no whichId anymore, type-safe
        // xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("typeName"), BAD_CAST(typeid(*this).name()));
        OUString rText;
        IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
        if(getPresentation(SfxItemPresentation::Complete, MapUnit::Map100thMM, MapUnit::Map100thMM, rText, aIntlWrapper))
        {
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(rText.getStr()));
        }
        xmlTextWriterEndElement(pWriter);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
