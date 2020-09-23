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
#include <libxml/xmlwriter.h>
#include <typeinfo>
#include <boost/property_tree/json_parser.hpp>

//////////////////////////////////////////////////////////////////////////////
// list of classes derived from SfxPoolItem
// will not be kept up-to-date, but give a good overview for right now
//////////////////////////////////////////////////////////////////////////////
//
// class SbxItem : public SfxPoolItem
// class SvxChartColorTableItem : public SfxPoolItem
// class DriverPoolingSettingsItem final : public SfxPoolItem
// class DatabaseMapItem final : public SfxPoolItem
// class DbuTypeCollectionItem : public SfxPoolItem
// class OptionalBoolItem : public SfxPoolItem
// class OStringListItem : public SfxPoolItem
// class MediaItem : public SfxPoolItem
// class SvxBoxItem : public SfxPoolItem
// class SvxBoxInfoItem : public SfxPoolItem
// class SvxBrushItem : public SfxPoolItem
// class SvxBulletItem : public SfxPoolItem
// class SvxColorItem : public SfxPoolItem
//    class SvxBackgroundColorItem : public SvxColorItem
// class SvxFontHeightItem : public SfxPoolItem
// class SvxFieldItem : public SfxPoolItem
// class SvxFontListItem : public SfxPoolItem
// class SvxFontItem : public SfxPoolItem
// class SvxHyphenZoneItem : public SfxPoolItem
// class SvxLineItem : public SfxPoolItem
// class SvxLRSpaceItem : public SfxPoolItem
// class SvxNumBulletItem : public SfxPoolItem
// class SfxHyphenRegionItem: public SfxPoolItem
// class SvxProtectItem : public SfxPoolItem
// class SvxSizeItem : public SfxPoolItem
//    class SwFormatFrameSize: public SvxSizeItem
// class SvxTabStopItem : public SfxPoolItem
// class SvxTwoLinesItem : public SfxPoolItem
// class SvxULSpaceItem : public SfxPoolItem
// class SvXMLAttrContainerItem: public SfxPoolItem
// class SfxLinkItem : public SfxPoolItem
// class SfxEventNamesItem : public SfxPoolItem
// class SfxFrameItem: public SfxPoolItem
// class SfxUnoAnyItem : public SfxPoolItem
// class SfxUnoFrameItem : public SfxPoolItem
// class SfxMacroInfoItem: public SfxPoolItem
// class SfxObjectItem: public SfxPoolItem
// class SfxObjectShellItem: public SfxPoolItem
// class SfxViewFrameItem: public SfxPoolItem
// class SfxWatermarkItem: public SfxPoolItem
// class SfxAllEnumItem: public SfxPoolItem
// class SfxEnumItemInterface: public SfxPoolItem
//    class SvxAdjustItem : public SfxEnumItemInterface
//    class SvxEscapementItem : public SfxEnumItemInterface
//    class SvxLineSpacingItem : public SfxEnumItemInterface
//    class SvxShadowItem : public SfxEnumItemInterface
//    class SfxEnumItem : public SfxEnumItemInterface
//        class SvxCharReliefItem : public SfxEnumItem<FontRelief>
//        class SvxCaseMapItem : public SfxEnumItem<SvxCaseMap>
//        class SvxCrossedOutItem : public SfxEnumItem<FontStrikeout>
//        class SvxFormatBreakItem : public SfxEnumItem<SvxBreak>
//        class SvxFrameDirectionItem : public SfxEnumItem<SvxFrameDirection>
//        class SvxHorJustifyItem: public SfxEnumItem<SvxCellHorJustify>
//        class SvxVerJustifyItem: public SfxEnumItem<SvxCellVerJustify>
//        class SvxJustifyMethodItem: public SfxEnumItem<SvxCellJustifyMethod>
//        class SvxLanguageItem_Base: public SfxEnumItem<LanguageType>
//            class SvxLanguageItem : public SvxLanguageItem_Base
//        class SvxPostureItem : public SfxEnumItem<FontItalic>
//        class SvxTextLineItem : public SfxEnumItem<FontLineStyle>
//            class SvxUnderlineItem : public SvxTextLineItem
//            class SvxOverlineItem : public SvxTextLineItem
//        class SvxWeightItem : public SfxEnumItem<FontWeight>
//        class SvxOrientationItem: public SfxEnumItem<SvxCellOrientation>
//        class SvxChartRegressItem : public SfxEnumItem<SvxChartRegress>
//        class SvxChartTextOrderItem : public SfxEnumItem<SvxChartTextOrder>
//        class SvxChartKindErrorItem : public SfxEnumItem<SvxChartKindError>
//        class SvxChartIndicateItem : public SfxEnumItem<SvxChartIndicate>
//        class SvxRotateModeItem: public SfxEnumItem<SvxRotateMode>
//        class SdrGrafModeItem_Base: public SfxEnumItem<GraphicDrawMode>
//            class SdrGrafModeItem : public SdrGrafModeItem_Base
//        class SdrTextAniDirectionItem: public SfxEnumItem<SdrTextAniDirection>
//        class SdrTextVertAdjustItem: public SfxEnumItem<SdrTextVertAdjust>
//        class SdrTextHorzAdjustItem: public SfxEnumItem<SdrTextHorzAdjust>
//        class SdrTextAniKindItem: public SfxEnumItem<SdrTextAniKind>
//        class SdrTextFitToSizeTypeItem    : public SfxEnumItem<css::drawing::TextFitToSizeType>
//        class SdrCaptionEscDirItem: public SfxEnumItem<SdrCaptionEscDir>
//        class SdrCaptionTypeItem: public SfxEnumItem<SdrCaptionType>
//        class SdrEdgeKindItem: public SfxEnumItem<SdrEdgeKind>
//        class SdrMeasureTextHPosItem: public SfxEnumItem<css::drawing::MeasureTextHorzPos>
//        class SdrMeasureTextVPosItem: public SfxEnumItem<css::drawing::MeasureTextVertPos>
//        class SdrMeasureUnitItem: public SfxEnumItem<FieldUnit>
//        class XFillStyleItem : public SfxEnumItem<css::drawing::FillStyle>
//        class XFillBmpPosItem : public SfxEnumItem<RectPoint>
//        class XFormTextAdjustItem : public SfxEnumItem<XFormTextAdjust>
//        class XFormTextShadowItem : public SfxEnumItem<XFormTextShadow>
//        class XLineStyleItem : public SfxEnumItem<css::drawing::LineStyle>
//        class XLineJointItem : public SfxEnumItem<css::drawing::LineJoint>
//        class XLineCapItem : public SfxEnumItem<css::drawing::LineCap>
//        class XFormTextStyleItem : public SfxEnumItem<XFormTextStyle>
//        class ScViewObjectModeItem: public SfxEnumItem<ScVObjMode>
//        class SdrCircKindItem: public SfxEnumItem<SdrCircKind>
//        class SdrMeasureKindItem: public SfxEnumItem<SdrMeasureKind>
//        class SwFormatFillOrder: public SfxEnumItem<SwFillOrder>
//        class SwFormatFootnoteEndAtTextEnd : public SfxEnumItem<SwFootnoteEndPosEnum>
//            class SwFormatFootnoteAtTextEnd : public SwFormatFootnoteEndAtTextEnd
//            class SwFormatEndAtTextEnd : public SwFormatFootnoteEndAtTextEnd
//        class SwFormatSurround: public SfxEnumItem<css::text::WrapTextMode>
//        class SwMirrorGrf : public SfxEnumItem<MirrorGraph>
//        class SwDrawModeGrf_Base: public SfxEnumItem<GraphicDrawMode>
//            class SwDrawModeGrf : public SwDrawModeGrf_Base
//class CntByteItem: public SfxPoolItem
//    class SfxByteItem: public CntByteItem
//        class SvxOrphansItem: public SfxByteItem
//        class SvxPaperBinItem : public SfxByteItem
//        class SvxWidowsItem: public SfxByteItem
//        class SwTransparencyGrf : public SfxByteItem
//class CntUInt16Item: public SfxPoolItem
//    class SfxUInt16Item: public CntUInt16Item
//        class SvxTextRotateItem : public SfxUInt16Item
//            class SvxCharRotateItem : public SvxTextRotateItem
//        class SvxCharScaleWidthItem : public SfxUInt16Item
//        class SvxEmphasisMarkItem : public SfxUInt16Item
//        class SvxParaVertAlignItem : public SfxUInt16Item
//        class SvxWritingModeItem : public SfxUInt16Item
//        class SvxZoomItem: public SfxUInt16Item
//        class SdrPercentItem : public SfxUInt16Item
//            class SdrGrafTransparenceItem : public SdrPercentItem
//        class SdrTextAniCountItem: public SfxUInt16Item
//        class SdrTextAniDelayItem: public SfxUInt16Item
//        class Svx3DNormalsKindItem : public SfxUInt16Item
//        class Svx3DTextureProjectionXItem : public SfxUInt16Item
//        class Svx3DTextureProjectionYItem : public SfxUInt16Item
//        class Svx3DTextureKindItem : public SfxUInt16Item
//        class Svx3DTextureModeItem : public SfxUInt16Item
//        class Svx3DPerspectiveItem : public SfxUInt16Item
//        class Svx3DShadeModeItem : public SfxUInt16Item
//        class SdrEdgeLineDeltaCountItem: public SfxUInt16Item
//        class SvxViewLayoutItem: public SfxUInt16Item
//        class XFillBmpPosOffsetXItem : public SfxUInt16Item
//        class XFillBmpPosOffsetYItem : public SfxUInt16Item
//        class XFillBmpTileOffsetXItem : public SfxUInt16Item
//        class XFillBmpTileOffsetYItem : public SfxUInt16Item
//        class XFillTransparenceItem: public SfxUInt16Item
//        class XFormTextShadowTranspItem: public SfxUInt16Item
//        class XGradientStepCountItem: public SfxUInt16Item
//        class XLineTransparenceItem: public SfxUInt16Item
//        class SvxZoomSliderItem: public SfxUInt16Item
//        class SdrLayerIdItem: public SfxUInt16Item
//        class SwRotationGrf : public SfxUInt16Item
//class CntInt32Item: public SfxPoolItem
//    class SfxInt32Item: public CntInt32Item
//        class SfxMetricItem: public SfxInt32Item
//            class XFillBmpSizeXItem : public SfxMetricItem
//            class XFillBmpSizeYItem : public SfxMetricItem
//            class XFormTextDistanceItem : public SfxMetricItem
//            class XFormTextShadowXValItem : public SfxMetricItem
//            class XFormTextShadowYValItem : public SfxMetricItem
//            class XFormTextStartItem : public SfxMetricItem
//            class XLineEndWidthItem : public SfxMetricItem
//            class XLineStartWidthItem : public SfxMetricItem
//            class XLineWidthItem : public SfxMetricItem
//        class SdrAngleItem: public SfxInt32Item
//
//            class SdrCaptionAngleItem: public SdrAngleItem
//            class SdrMeasureTextFixedAngleItem: public SdrAngleItem
//            class SdrMeasureTextAutoAngleViewItem: public SdrAngleItem
//            class SdrRotateAllItem: public SdrAngleItem
//            class SdrRotateOneItem: public SdrAngleItem
//            class SdrShearAngleItem: public SdrAngleItem
//            class SdrHorzShearAllItem: public SdrAngleItem
//            class SdrVertShearAllItem: public SdrAngleItem
//            class SdrHorzShearOneItem: public SdrAngleItem
//            class SdrVertShearOneItem: public SdrAngleItem
//        class SdrMetricItem: public SfxInt32Item
//            class SdrCaptionEscAbsItem: public SdrMetricItem
//            class SdrCaptionGapItem: public SdrMetricItem
//            class SdrCaptionLineLenItem: public SdrMetricItem
//            class SdrEdgeNode1HorzDistItem: public SdrMetricItem
//            class SdrEdgeNode1VertDistItem: public SdrMetricItem
//            class SdrEdgeNode2HorzDistItem: public SdrMetricItem
//            class SdrEdgeNode2VertDistItem: public SdrMetricItem
//            class SdrEdgeNode1GlueDistItem: public SdrMetricItem
//            class SdrEdgeNode2GlueDistItem: public SdrMetricItem
//            class SdrAllPositionXItem: public SdrMetricItem
//            class SdrAllPositionYItem: public SdrMetricItem
//            class SdrAllSizeWidthItem: public SdrMetricItem
//            class SdrAllSizeHeightItem: public SdrMetricItem
//            class SdrLogicSizeWidthItem: public SdrMetricItem
//            class SdrLogicSizeHeightItem: public SdrMetricItem
//            class SdrMeasureOverhangItem: public SdrMetricItem
//            class SdrMoveXItem: public SdrMetricItem
//            class SdrMoveYItem: public SdrMetricItem
//            class SdrOnePositionXItem: public SdrMetricItem
//            class SdrOnePositionYItem: public SdrMetricItem
//            class SdrOneSizeWidthItem: public SdrMetricItem
//            class SdrOneSizeHeightItem: public SdrMetricItem
//            class SdrTransformRef1XItem: public SdrMetricItem
//            class SdrTransformRef1YItem: public SdrMetricItem
//            class SdrTransformRef2XItem: public SdrMetricItem
//            class SdrTransformRef2YItem: public SdrMetricItem
//        class SdrCaptionEscRelItem: public SfxInt32Item
//class CntUInt32Item: public SfxPoolItem
//    class SfxUInt32Item: public CntUInt32Item
//        class SvxRsidItem : public SfxUInt32Item
//        class SdrGrafGamma100Item : public SfxUInt32Item
//        class SwTableBoxNumFormat : public SfxUInt32Item
//class CntUnencodedStringItem: public SfxPoolItem
//    class SfxStringItem: public CntUnencodedStringItem
//        class SvxPageModelItem : public SfxStringItem
//        class SfxDocumentInfoItem : public SfxStringItem
//        class SvxPostItAuthorItem: public SfxStringItem
//        class SvxPostItDateItem: public SfxStringItem
//        class SvxPostItTextItem: public SfxStringItem
//        class SvxPostItIdItem: public SfxStringItem
//        class SdrMeasureFormatStringItem: public SfxStringItem
//        class NameOrIndex : public SfxStringItem
//            class XFillBitmapItem : public NameOrIndex
//            class XColorItem : public NameOrIndex
//                class XFillColorItem : public XColorItem
//                class XFormTextShadowColorItem : public XColorItem
//                class XLineColorItem : public XColorItem
//                class XSecondaryFillColorItem : public XColorItem
//            class XFillGradientItem : public NameOrIndex
//                class XFillFloatTransparenceItem : public XFillGradientItem
//            class XFillHatchItem : public NameOrIndex
//            class XLineDashItem : public NameOrIndex
//            class XLineEndItem : public NameOrIndex
//            class XLineStartItem : public NameOrIndex
//        class SfxScriptOrganizerItem : public SfxStringItem
//        class SdrLayerNameItem: public SfxStringItem
//        class SwNumRuleItem : public SfxStringItem
//class SfxBoolItem    : public SfxPoolItem
//    class SvxAutoKernItem : public SfxBoolItem
//    class SvxBlinkItem : public SfxBoolItem
//    class SvxCharHiddenItem : public SfxBoolItem
//    class SvxContourItem : public SfxBoolItem
//    class SvxForbiddenRuleItem : public SfxBoolItem
//    class SvxHangingPunctuationItem : public SfxBoolItem
//    class SvxFormatKeepItem : public SfxBoolItem
//    class SvxNoHyphenItem : public SfxBoolItem
//    class SvxOpaqueItem : public SfxBoolItem
//    class SvxParaGridItem : public SfxBoolItem
//    class SvxPrintItem : public SfxBoolItem
//    class SvxScriptSpaceItem : public SfxBoolItem
//    class SvxShadowedItem : public SfxBoolItem
//    class SvxFormatSplitItem : public SfxBoolItem
//    class SvxWordLineModeItem : public SfxBoolItem
//    class SdrOnOffItem: public SfxBoolItem
//        class SdrGrafInvertItem : public SdrOnOffItem
//    class SdrTextFixedCellHeightItem : public SfxBoolItem
//    class SdrYesNoItem: public SfxBoolItem
//        class SdrTextAniStartInsideItem: public SdrYesNoItem
//        class SdrTextAniStopInsideItem: public SdrYesNoItem
//        class SdrCaptionEscIsRelItem: public SdrYesNoItem
//        class SdrCaptionFitLineLenItem: public SdrYesNoItem
//        class SdrMeasureBelowRefEdgeItem: public SdrYesNoItem
//        class SdrMeasureTextIsFixedAngleItem: public SdrYesNoItem
//        class SdrMeasureTextRota90Item: public SdrYesNoItem
//        class SdrMeasureTextUpsideDownItem: public SdrYesNoItem
//        class SdrMeasureTextAutoAngleItem: public SdrYesNoItem
//        class SdrObjPrintableItem: public SdrYesNoItem
//        class SdrObjVisibleItem: public SdrYesNoItem
//    class Svx3DReducedLineGeometryItem : public SfxBoolItem
//    class Svx3DSmoothNormalsItem : public SfxBoolItem
//    class Svx3DSmoothLidsItem : public SfxBoolItem
//    class Svx3DCharacterModeItem : public SfxBoolItem
//    class Svx3DCloseFrontItem : public SfxBoolItem
//    class Svx3DCloseBackItem : public SfxBoolItem
//    class XFillBackgroundItem : public SfxBoolItem
//    class XFillBmpSizeLogItem : public SfxBoolItem
//    class XFillBmpTileItem : public SfxBoolItem
//    class XFillBmpStretchItem : public SfxBoolItem
//    class XFormTextMirrorItem : public SfxBoolItem
//    class XFormTextOutlineItem : public SfxBoolItem
//    class XLineEndCenterItem : public SfxBoolItem
//    class XLineStartCenterItem : public SfxBoolItem
//    class XFormTextHideFormItem : public SfxBoolItem
//    class SwFormatNoBalancedColumns : public SfxBoolItem
//    class SwFormatEditInReadonly : public SfxBoolItem
//    class SwFormatFollowTextFlow : public SfxBoolItem
//    class SwFormatLayoutSplit : public SfxBoolItem
//    class SwFormatRowSplit : public SfxBoolItem
//    class SwInvertGrf: public SfxBoolItem
//    class SwHeaderAndFooterEatSpacingItem : public SfxBoolItem
//    class SwRegisterItem : public SfxBoolItem
//    class SwParaConnectBorderItem : public SfxBoolItem
// class SfxFlagItem: public SfxPoolItem
//     class SfxTemplateItem: public SfxFlagItem
// class SfxGlobalNameItem: public SfxPoolItem
// class SfxGrabBagItem : public SfxPoolItem
// class SfxIntegerListItem : public SfxPoolItem
// class SfxInt64Item : public SfxPoolItem
// class SfxInt16Item: public SfxPoolItem
//     class SvxKerningItem : public SfxInt16Item
//     class SfxImageItem : public SfxInt16Item
//     class SdrSignedPercentItem : public SfxInt16Item
//         class SdrGrafRedItem : public SdrSignedPercentItem
//         class SdrGrafGreenItem : public SdrSignedPercentItem
//         class SdrGrafBlueItem : public SdrSignedPercentItem
//         class SdrGrafLuminanceItem : public SdrSignedPercentItem
//         class SdrGrafContrastItem : public SdrSignedPercentItem
//     class SdrTextAniAmountItem: public SfxInt16Item
//     class SdrMeasureDecimalPlacesItem: public SfxInt16Item
//     class ScMergeFlagAttr: public SfxInt16Item
//     class SwLuminanceGrf : public SfxInt16Item
//     class SwContrastGrf : public SfxInt16Item
//     class SwChannelGrf : public SfxInt16Item
// class SfxLockBytesItem : public SfxPoolItem
// class SvxMacroItem: public SfxPoolItem
// class SfxVoidItem final: public SfxPoolItem
// class SfxSetItem: public SfxPoolItem
//     class SvxScriptSetItem : public SfxSetItem
//     class SfxTabDialogItem: public SfxSetItem
//     class SvxSetItem: public SfxSetItem
//     class XFillAttrSetItem : public SfxSetItem
//     class XLineAttrSetItem : public SfxSetItem
//     class ScPatternAttr: public SfxSetItem
// class SfxPointItem: public SfxPoolItem
// class SfxRectangleItem: public SfxPoolItem
// class SfxRangeItem : public SfxPoolItem
// class SfxStringListItem : public SfxPoolItem
// class SvxSearchItem :        public SfxPoolItem
// class SfxVisibilityItem: public SfxPoolItem
// class AffineMatrixItem : public SfxPoolItem
// class SvxMarginItem: public SfxPoolItem
// class SvxDoubleItem : public SfxPoolItem
// class SvxClipboardFormatItem : public SfxPoolItem
// class SvxColorListItem: public SfxPoolItem
// class SvxGradientListItem : public SfxPoolItem
// class SvxHatchListItem : public SfxPoolItem
// class SvxBitmapListItem : public SfxPoolItem
// class SvxPatternListItem : public SfxPoolItem
// class SvxDashListItem : public SfxPoolItem
// class SvxLineEndListItem : public SfxPoolItem
// class SvxB3DVectorItem : public SfxPoolItem
// class SvxGalleryItem : public SfxPoolItem
// class SvxGrfCrop : public SfxPoolItem
//     class SdrGrafCropItem : public SvxGrfCrop
//     class SwCropGrf : public SvxGrfCrop
// class SvxHyperlinkItem : public SfxPoolItem
// class SvxNumberInfoItem : public SfxPoolItem
// class OfaPtrItem : public SfxPoolItem
// class OfaRefItem : public SfxPoolItem
// class SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
//     class SdOptionsGridItem : public SvxGridItem
// class SvxPageItem: public SfxPoolItem
// class SvxLongLRSpaceItem : public SfxPoolItem
// class SvxLongULSpaceItem : public SfxPoolItem
// class SvxPagePosSizeItem : public SfxPoolItem
// class SvxColumnItem : public SfxPoolItem
// class SvxObjectItem : public SfxPoolItem
// class SdrCustomShapeGeometryItem : public SfxPoolItem
// class SvxSmartTagItem : public SfxPoolItem
// class SvxGraphicItem: public SfxPoolItem
// class SdrFractionItem: public SfxPoolItem
//     class SdrScaleItem: public SdrFractionItem
//         class SdrMeasureScaleItem: public SdrScaleItem
//     class SdrResizeXAllItem: public SdrFractionItem
//     class SdrResizeYAllItem: public SdrFractionItem
//     class SdrResizeXOneItem: public SdrFractionItem
//     class SdrResizeYOneItem: public SdrFractionItem
// class ScMergeAttr: public SfxPoolItem
// class ScProtectionAttr: public SfxPoolItem
// class ScPageHFItem : public SfxPoolItem
// class ScPageScaleToItem : public SfxPoolItem
// class ScCondFormatItem : public SfxPoolItem
// class ScTpDefaultsItem : public SfxPoolItem
// class ScTpCalcItem : public SfxPoolItem
// class ScTpFormulaItem : public SfxPoolItem
// class ScTpPrintItem : public SfxPoolItem
// class ScTpViewItem : public SfxPoolItem
// class ScCondFormatDlgItem : public SfxPoolItem
// class ScInputStatusItem : public SfxPoolItem
// class ScSortItem : public SfxPoolItem
// class ScQueryItem : public SfxPoolItem
// class ScSubTotalItem : public SfxPoolItem
// class cUserListItem : public SfxPoolItem
// class ScConsolidateItem : public SfxPoolItem
// class ScPivotItem : public SfxPoolItem
// class ScSolveItem : public SfxPoolItem
// class ScTabOpItem : public SfxPoolItem
// class SdOptionsLayoutItem : public SfxPoolItem
// class SdOptionsMiscItem : public SfxPoolItem
// class SdOptionsSnapItem : public SfxPoolItem
// class SdOptionsPrintItem : public SfxPoolItem
// class SwCondCollItem : public SfxPoolItem
// class SwTableBoxFormula : public SfxPoolItem, public SwTableFormula
// class SwTableBoxValue : public SfxPoolItem
// class SwFormatCharFormat: public SfxPoolItem, public SwClient
// class SwFormatAnchor: public SfxPoolItem
// class SwFormatAutoFormat: public SfxPoolItem
// class SwFormatCol : public SfxPoolItem
// class SwFormatChain: public SfxPoolItem
// class SwFormatContent: public SfxPoolItem
// class SwFormatFlyCnt : public SfxPoolItem
// class SwFormatField    : public SfxPoolItem
// class SwFormatFootnote    : public SfxPoolItem
// class SwFormatHeader: public SfxPoolItem, public SwClient
// class SwFormatFooter: public SfxPoolItem, public SwClient
// class SwFormatINetFormat    : public SfxPoolItem
// class SwFormatLineNumber: public SfxPoolItem
// class SwFormatMeta    : public SfxPoolItem
// class SwFormatVertOrient: public SfxPoolItem
// class SwFormatHoriOrient: public SfxPoolItem
// class SwFormatPageDesc : public SfxPoolItem, public SwClient
// class SwFormatRefMark    : public SfxPoolItem
// class SwFormatRuby : public SfxPoolItem
// class SwFormatURL: public SfxPoolItem
// class SwFormatWrapInfluenceOnObjPos: public SfxPoolItem
// class SwGammaGrf : public SfxPoolItem
// class SwMsgPoolItem : public SfxPoolItem
//     class SwPtrMsgPoolItem : public SwMsgPoolItem
//     class SwFormatChg: public SwMsgPoolItem
//     class SwInsText: public SwMsgPoolItem
//     class SwDelChr: public SwMsgPoolItem
//     class SwDelText: public SwMsgPoolItem
//     class SwUpdateAttr : public SwMsgPoolItem
//     class SwRefMarkFieldUpdate : public SwMsgPoolItem
//     class SwDocPosUpdate : public SwMsgPoolItem
//     class SwTableFormulaUpdate : public SwMsgPoolItem
//     class SwAutoFormatGetDocNode: public SwMsgPoolItem
//     class SwAttrSetChg: public SwMsgPoolItem
//     class SwCondCollCondChg: public SwMsgPoolItem
//     class SwVirtPageNumInfo: public SwMsgPoolItem
//     class SwFindNearestNode : public SwMsgPoolItem
//     class SwStringMsgPoolItem : public SwMsgPoolItem
// class SwFormatDrop: public SfxPoolItem, public SwClient
// class SwTextGridItem : public SfxPoolItem
// class SwTOXMark    : public SfxPoolItem
// class SwFltAnchor : public SfxPoolItem
// class SwFltRedline : public SfxPoolItem
// class SwFltBookmark : public SfxPoolItem
// class SwFltRDFMark : public SfxPoolItem
// class SwFltTOX : public SfxPoolItem
// class SwDocDisplayItem : public SfxPoolItem
// class SwElemItem : public SfxPoolItem
// class SwAddPrinterItem : public SfxPoolItem, public SwPrintData
// class SwShadowCursorItem : public SfxPoolItem
// class SwTestItem : public SfxPoolItem
// class SwEnvItem : public SfxPoolItem
// class SwLabItem : public SfxPoolItem
// class SwWrtShellItem: public SfxPoolItem
// class SwPageFootnoteInfoItem : public SfxPoolItem
// class SwPtrItem : public SfxPoolItem
// class SwUINumRuleItem : public SfxPoolItem
// class SwPaMItem : public SfxPoolItem
//////////////////////////////////////////////////////////////////////////////

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
    assert(typeid(rCmp) == typeid(*this) && "comparing different pool item subclasses");
    (void)rCmp;
    return true;
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
        xmlTextWriterWriteAttribute(
            pWriter, BAD_CAST("presentation"), BAD_CAST(rText.toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

boost::property_tree::ptree SfxPoolItem::dumpAsJSON() const
{
    boost::property_tree::ptree aTree;
    return aTree;
}

std::unique_ptr<SfxPoolItem> SfxPoolItem::CloneSetWhich( sal_uInt16 nNewWhich ) const
{
    std::unique_ptr<SfxPoolItem> pItem(Clone());
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

SfxVoidItem* SfxVoidItem::Clone(SfxItemPool *) const
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
