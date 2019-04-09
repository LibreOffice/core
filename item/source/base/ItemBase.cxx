/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/base/ItemBase.hxx>
#include <item/base/ItemAdministrator.hxx>
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
-> SID_ATTR_TRANSFORM_ANCHOR -> need own type to replace in ItemSet ->

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

class DriverPoolingSettingsItem final : public SfxPoolItem
class DatabaseMapItem final : public SfxPoolItem
class DbuTypeCollectionItem : public SfxPoolItem
class OptionalBoolItem : public SfxPoolItem
class OStringListItem : public SfxPoolItem
class AVMEDIA_DLLPUBLIC MediaItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxBoxItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxBoxInfoItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxBrushItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxBulletItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxColorItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxFontHeightItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxFieldItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxFontListItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxFontItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxHyphenZoneItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxLineItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxLRSpaceItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxNumBulletItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SfxHyphenRegionItem: public SfxPoolItem
class EDITENG_DLLPUBLIC SvxProtectItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxSizeItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxTabStopItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxTwoLinesItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvxULSpaceItem : public SfxPoolItem
class EDITENG_DLLPUBLIC SvXMLAttrContainerItem: public SfxPoolItem
class SFX2_DLLPUBLIC SfxLinkItem : public SfxPoolItem
class SFX2_DLLPUBLIC SfxEventNamesItem : public SfxPoolItem
class SFX2_DLLPUBLIC SfxFrameItem: public SfxPoolItem
class SFX2_DLLPUBLIC SfxUnoAnyItem : public SfxPoolItem
class SFX2_DLLPUBLIC SfxUnoFrameItem : public SfxPoolItem
class SFX2_DLLPUBLIC SfxMacroInfoItem: public SfxPoolItem
class SFX2_DLLPUBLIC SfxObjectItem: public SfxPoolItem
class SFX2_DLLPUBLIC SfxObjectShellItem: public SfxPoolItem
class SFX2_DLLPUBLIC SfxViewFrameItem: public SfxPoolItem
class SFX2_DLLPUBLIC SfxWatermarkItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxEnumItemInterface: public SfxPoolItem
class SVL_DLLPUBLIC CntByteItem: public SfxPoolItem
class SVL_DLLPUBLIC CntUInt16Item: public SfxPoolItem
class SVL_DLLPUBLIC CntInt32Item: public SfxPoolItem
class SVL_DLLPUBLIC CntUInt32Item: public SfxPoolItem
class SVL_DLLPUBLIC CntUnencodedStringItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxFlagItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxGlobalNameItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxGrabBagItem : public SfxPoolItem
class SVL_DLLPUBLIC SfxIntegerListItem : public SfxPoolItem
class SVL_DLLPUBLIC SfxInt64Item : public SfxPoolItem
class SVL_DLLPUBLIC SfxInt16Item: public SfxPoolItem
class SVL_DLLPUBLIC SfxLockBytesItem : public SfxPoolItem
class SVL_DLLPUBLIC SvxMacroItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxVoidItem final: public SfxPoolItem
class SVL_DLLPUBLIC SfxSetItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxPointItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxRectangleItem: public SfxPoolItem
class SVL_DLLPUBLIC SfxRangeItem : public SfxPoolItem
class SVL_DLLPUBLIC SfxStringListItem : public SfxPoolItem
class SVL_DLLPUBLIC SfxVisibilityItem: public SfxPoolItem
class SVX_DLLPUBLIC AffineMatrixItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxMarginItem: public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxDoubleItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxClipboardFormatItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxColorListItem: public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxGradientListItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxHatchListItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxBitmapListItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxPatternListItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxDashListItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxLineEndListItem : public SfxPoolItem
class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxB3DVectorItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxGalleryItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxHyperlinkItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxNumberInfoItem : public SfxPoolItem
class SVX_DLLPUBLIC OfaPtrItem : public SfxPoolItem
class OfaRefItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
class SVX_DLLPUBLIC SvxPageItem: public SfxPoolItem
class SVX_DLLPUBLIC SvxLongLRSpaceItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxLongULSpaceItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxPagePosSizeItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxColumnItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxObjectItem : public SfxPoolItem
class SVX_DLLPUBLIC SdrCustomShapeGeometryItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxSmartTagItem : public SfxPoolItem
class SVX_DLLPUBLIC SvxGraphicItem: public SfxPoolItem
class SdrFractionItem: public SfxPoolItem {
class SC_DLLPUBLIC ScPageHFItem : public SfxPoolItem
class SC_DLLPUBLIC ScPageScaleToItem : public SfxPoolItem
class ScCondFormatItem : public SfxPoolItem
class SC_DLLPUBLIC ScTpDefaultsItem : public SfxPoolItem
class SC_DLLPUBLIC ScTpCalcItem : public SfxPoolItem
class SC_DLLPUBLIC ScTpFormulaItem : public SfxPoolItem
class SC_DLLPUBLIC ScTpPrintItem : public SfxPoolItem
class SC_DLLPUBLIC ScTpViewItem : public SfxPoolItem
class ScCondFormatDlgItem : public SfxPoolItem
class ScInputStatusItem : public SfxPoolItem
class SC_DLLPUBLIC ScSortItem : public SfxPoolItem
class SC_DLLPUBLIC ScQueryItem : public SfxPoolItem
class SC_DLLPUBLIC ScSubTotalItem : public SfxPoolItem
class SC_DLLPUBLIC ScUserListItem : public SfxPoolItem
class ScConsolidateItem : public SfxPoolItem
class ScPivotItem : public SfxPoolItem
class ScSolveItem : public SfxPoolItem
class ScTabOpItem : public SfxPoolItem
class SD_DLLPUBLIC SdOptionsLayoutItem : public SfxPoolItem
class SD_DLLPUBLIC SdOptionsMiscItem : public SfxPoolItem
class SD_DLLPUBLIC SdOptionsSnapItem : public SfxPoolItem
class SD_DLLPUBLIC SdOptionsPrintItem : public SfxPoolItem
class SW_DLLPUBLIC SwCondCollItem : public SfxPoolItem
class SwMsgPoolItem : public SfxPoolItem
class SW_DLLPUBLIC SwTextGridItem : public SfxPoolItem
class SW_DLLPUBLIC SwDocDisplayItem : public SfxPoolItem
class SW_DLLPUBLIC SwElemItem : public SfxPoolItem
class SW_DLLPUBLIC SwAddPrinterItem : public SfxPoolItem, public SwPrintData
class SW_DLLPUBLIC SwShadowCursorItem : public SfxPoolItem
class SW_DLLPUBLIC SwTestItem : public SfxPoolItem
class SW_DLLPUBLIC SwEnvItem : public SfxPoolItem
class SW_DLLPUBLIC SwLabItem : public SfxPoolItem
class SW_DLLPUBLIC SwWrtShellItem: public SfxPoolItem
class SW_DLLPUBLIC SwPageFootnoteInfoItem : public SfxPoolItem
class SW_DLLPUBLIC SwPtrItem : public SfxPoolItem
class SW_DLLPUBLIC SwUINumRuleItem : public SfxPoolItem
class SW_DLLPUBLIC SwPaMItem : public SfxPoolItem
*/
///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemControlBlock& ItemBase::GetStaticItemControlBlock()
    {
        assert(false && "ItemBase::GetItemControlBlock call not allowed (!)");
        static ItemControlBlock aItemControlBlock(
            std::shared_ptr<ItemAdministrator>(),
            std::shared_ptr<const ItemBase>(),
            [](){ return nullptr; });

        return aItemControlBlock;
    }

    ItemControlBlock& ItemBase::GetItemControlBlock() const
    {
        return ItemBase::GetStaticItemControlBlock();
    }

    ItemBase::ItemBase()
    :   std::enable_shared_from_this<ItemBase>(),
        m_bAdministrated(false)
    {
    }

    bool ItemBase::CheckSameType(const ItemBase& rCmp) const
    {
        return typeid(rCmp) == typeid(*this);
    }

    void ItemBase::PutValues(const AnyIDArgs& rArgs)
    {
        for(const auto& arg : rArgs)
        {
            PutValue(arg.first, arg.second);
        }
    }

    void ItemBase::PutValue(const css::uno::Any& /*rVal*/, sal_uInt8 /*nMemberId*/)
    {
        // not intended to be used, error
        assert(false && "Error: Some instance tries to set an Any at an ::Item instance where this is not supported - implement ItemBase::PutValue method there (!)");
    }

    void ItemBase::implInstanceCleanup()
    {
        if(IsAdministrated())
        {
            GetItemControlBlock().GetItemAdministrator()->HintExpired(this);
        }
    }

    ItemBase::~ItemBase()
    {
    }

    bool ItemBase::operator==(const ItemBase& rCmp) const
    {
        // basic implementation compares type, no data available
        return CheckSameType(rCmp);
    }

    bool ItemBase::operator<(const ItemBase& rCmp) const
    {
        // basic implementation uses addresses of instances to
        // deliver a consistent result, but should *not* be used in
        // this form - it will not compare any data
        assert(CheckSameType(rCmp));
        return this < &rCmp;
    }

    size_t ItemBase::GetUniqueKey() const
    {
        // basic implementation uses adress of instance as hash, to
        // deliver a consistent result, but should *not* be used in
        // this form - it will not compare any data
        return static_cast<size_t>(sal_uInt64(this));
    }

    bool ItemBase::IsDefault() const
    {
        // callback to ItemControlBlock
        return GetItemControlBlock().IsDefaultDDD(*this);
    }

    const std::shared_ptr<const ItemBase>& ItemBase::GetDefault() const
    {
        // callback to ItemControlBlock
        assert(GetItemControlBlock().GetDefaultItem() && "empty DefaultItem detected - not allowed (!)");
        return GetItemControlBlock().GetDefaultItem();
    }

    bool ItemBase::IsDefault(const std::shared_ptr<const ItemBase>& rCandidate)
    {
        return rCandidate && rCandidate->GetItemControlBlock().IsDefaultDDD(*rCandidate);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
