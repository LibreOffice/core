/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/base/ItemBase.hxx>
// #include <item/base/ItemAdministrator.hxx>
#include <item/base/ItemControlBlock.hxx>
#include <cassert>

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
    void ItemBase::putValues(const AnyIDArgs& rArgs)
    {
        if(!rArgs.empty())
        {
            for(const auto& arg : rArgs)
            {
                putValue(arg.first, arg.second);
            }
        }
    }

    void ItemBase::putValue(const css::uno::Any& /*rVal*/, sal_uInt8 /*nMemberId*/)
    {
    }

    ItemBase::ItemBase(ItemControlBlock& rItemControlBlock)
    :   m_rItemControlBlock(rItemControlBlock)
    {
    }

    ItemBase::ItemBase(const ItemBase& rRef)
    :   m_rItemControlBlock(rRef.m_rItemControlBlock)
    {
    }

    ItemBase& ItemBase::operator=(const ItemBase&)
    {
        return *this;
    }

    bool ItemBase::operator==(const ItemBase& rRef) const
    {
        // ptr-compare
        return (this == &rRef);
    }

    bool ItemBase::operator!=(const ItemBase& rRef) const
    {
        return !(*this == rRef);
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
        return rCandidate.m_rItemControlBlock.isDefault(rCandidate);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
