/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/base/IBase.hxx>
#include <item/base/IAdministrator.hxx>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////
// export CXXFLAGS=-DENABLE_ITEMS
/*
class SbxItem : public SfxPoolItem
class SvxChartColorTableItem : public SfxPoolItem
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
    IBase::IBase()
    :   std::enable_shared_from_this<IBase>(),
        m_bAdministrated(false)
    {
    }

    bool IBase::CheckSameType(const IBase& rCmp) const
    {
        return typeid(rCmp) == typeid(*this);
    }

    IAdministrator* IBase::GetIAdministrator() const
    {
        // not intended to be used, error
        assert(false && "IBase::GetIAdministrator call not allowed (!)");
        return nullptr;
    }

    IBase::~IBase()
    {
    }

    bool IBase::operator==(const IBase& rCmp) const
    {
        // basic implementation compares type, no data available
        return CheckSameType(rCmp);
    }

    bool IBase::operator<(const IBase& rCmp) const
    {
        // basic implementation uses addresses of instances to
        // deliver a consistent result, but should *not* be used in
        // this form - it will not compare any data
        assert(CheckSameType(rCmp));
        return this < &rCmp;
    }

    size_t IBase::GetUniqueKey() const
    {
        // basic implementation uses adress of instance as hash, to
        // deliver a consistent result, but should *not* be used in
        // this form - it will not compare any data
        return static_cast<size_t>(sal_uInt64(this));
    }

    bool IBase::IsDefault() const
    {
        // callback to static administrator
        return GetIAdministrator()->IsDefault(this);
    }

    const IBase::SharedPtr& IBase::GetDefault() const
    {
        // callback to static administrator
        return GetIAdministrator()->GetDefault();
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
