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
#ifndef INCLUDED_SW_INC_SWATRSET_HXX
#define INCLUDED_SW_INC_SWATRSET_HXX

#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include "swdllapi.h"
#include "calbck.hxx"

class SwDoc;
class OutputDevice;
class IDocumentSettingAccess;
class SvxPostureItem;
class SvxWeightItem;
class SvxShadowedItem;
class SvxAutoKernItem;
class SvxWordLineModeItem;
class SvxContourItem;
class SvxKerningItem;
class SvxUnderlineItem;
class SvxOverlineItem;
class SvxCrossedOutItem;
class SvxFontHeightItem;
class SvxFontItem;
class SvxColorItem;
class SvxLanguageItem;
class SvxEscapementItem;
class SvxCaseMapItem;
class SvxNoHyphenItem;
class SvxBlinkItem;
class SvxEmphasisMarkItem;
class SvxTwoLinesItem;
class SvxCharScaleWidthItem;
class SvxCharRotateItem;
class SvxCharReliefItem;
class SvxCharHiddenItem;

// Frame attributes
class SwFormatFillOrder;
class SwFormatFrameSize;
class SvxPaperBinItem;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFormatContent;
class SwFormatHeader;
class SwFormatFooter;
class SvxPrintItem;
class SvxOpaqueItem;
class SvxProtectItem;
class SwFormatSurround;
class SwFormatVertOrient;
class SwFormatHoriOrient;
class SwFormatAnchor;
class SvxBoxItem;
class SvxBrushItem;
class SvxShadowItem;
class SwFormatPageDesc;
class SvxFormatBreakItem;
class SwFormatCol;
class SvxMacroItem;
class SvxFormatKeepItem;
class SwFormatURL;
class SwFormatLineNumber;
class SwFormatEditInReadonly;
class SwFormatLayoutSplit;
class SwFormatRowSplit;
class SwFormatChain;
class SwFormatFootnoteAtTextEnd;
class SwFormatEndAtTextEnd;
class SwFormatNoBalancedColumns;
class SvxFrameDirectionItem;
class SwHeaderAndFooterEatSpacingItem;
class SwFormatFollowTextFlow;
class SwFormatWrapInfluenceOnObjPos;
class SdrTextVertAdjustItem;

// Graphic attributes
class SwMirrorGrf;
class SwCropGrf;
class SwRotationGrf;
class SwLuminanceGrf;
class SwContrastGrf;
class SwChannelRGrf;
class SwChannelGGrf;
class SwChannelBGrf;
class SwGammaGrf;
class SwInvertGrf;
class SwTransparencyGrf;
class SwDrawModeGrf;

// Paragraph attributes
class SvxLineSpacingItem;
class SvxAdjustItem;
class SvxFormatSplitItem;
class SwRegisterItem;
class SwNumRuleItem;
class SvxWidowsItem;
class SvxOrphansItem;
class SvxTabStopItem;
class SvxHyphenZoneItem;
class SwFormatDrop;
class SvxScriptSpaceItem;
class SvxHangingPunctuationItem;
class SvxForbiddenRuleItem;
class SvxParaVertAlignItem;
class SvxParaGridItem;
class SwParaConnectBorderItem;

// TableBox attributes
class SwTableBoxNumFormat;
class SwTableBoxFormula;
class SwTableBoxValue;

class SwFormatLayoutInCell;

namespace vcl {
    typedef OutputDevice RenderContext;
};

class SAL_DLLPUBLIC_RTTI SwAttrPool final : public SfxItemPool
{
private:
    friend void InitCore();            // For creating/deleting of version maps.
    friend void FinitCore();

    SwDoc* m_pDoc;

public:
    SwAttrPool( SwDoc* pDoc );
private:
    virtual ~SwAttrPool() override;
public:

          SwDoc* GetDoc()           { return m_pDoc; }
    const SwDoc* GetDoc() const     { return m_pDoc; }

};

class SW_DLLPUBLIC SwAttrSet final : public SfxItemSet
{
    // Pointer for Modify-System
    SwAttrSet *m_pOldSet, *m_pNewSet;

    // Notification-Callback
    virtual void Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew ) override;

    void PutChgd( const SfxPoolItem& rI ) { SfxItemSet::PutDirect( rI ); }
public:
    SwAttrSet( SwAttrPool&, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
    SwAttrSet( SwAttrPool&, const WhichRangesContainer& nWhichPairTable );
    SwAttrSet( const SwAttrSet& );

    virtual std::unique_ptr<SfxItemSet> Clone(bool bItems = true, SfxItemPool *pToPool = nullptr) const override;

    bool Put_BC( const SfxPoolItem& rAttr, SwAttrSet* pOld, SwAttrSet* pNew );
    bool Put_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

    // Delete an item or a range.
    sal_uInt16 ClearItem_BC( sal_uInt16 nWhich, SwAttrSet* pOld, SwAttrSet* pNew );
    sal_uInt16 ClearItem_BC( sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                        SwAttrSet* pOld = nullptr, SwAttrSet* pNew = nullptr );

    int Intersect_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

    void GetPresentation( SfxItemPresentation ePres,
        MapUnit eCoreMetric, MapUnit ePresMetric, OUString &rText ) const;

    SwAttrPool* GetPool() const { return static_cast<SwAttrPool*>(SfxItemSet::GetPool()); }

    // Copy attributes, if necessary across documents.
    void CopyToModify( sw::BroadcastingModify& rMod ) const;

    // Special treatment for some attributes.
    // Set Modify-pointer (the old pDefinedIn) at the following attributes:
    //  - SwFormatDropCaps
    //  - SwFormatPageDesc
    // (Is called at insert in formats/nodes.)
    // Second version is for the SwAttrSet handles of SwContentNode.
    bool SetModifyAtAttr( const sw::BroadcastingModify* pModify );

    // Document is set at SwAttrPool. Therefore it is always accessible.
    const SwDoc *GetDoc() const { return GetPool()->GetDoc(); }
          SwDoc *GetDoc()       { return GetPool()->GetDoc(); }

    // Get methods: bool indicates whether to search only in Set (when false)
    // or also in parents. If nothing is found then default attribute is returned.
    // Character attributes. Implementation in charatr.hxx.
    inline const SvxPostureItem           &GetPosture( bool = true ) const;
    inline const SvxWeightItem          &GetWeight( bool = true ) const;
    inline const SvxShadowedItem        &GetShadowed( bool = true ) const;
    inline const SvxAutoKernItem        &GetAutoKern( bool = true ) const;
    inline const SvxWordLineModeItem    &GetWordLineMode( bool = true ) const;
    inline const SvxContourItem         &GetContour( bool = true ) const;
    inline const SvxKerningItem         &GetKerning( bool = true ) const;
    inline const SvxUnderlineItem       &GetUnderline( bool = true ) const;
    inline const SvxOverlineItem        &GetOverline( bool = true ) const;
    inline const SvxCrossedOutItem      &GetCrossedOut( bool = true ) const;
    inline const SvxFontHeightItem        &GetSize( bool = true ) const;
    inline const SvxFontItem            &GetFont( bool = true ) const;
    inline const SvxColorItem           &GetColor( bool = true ) const;
    inline const SvxLanguageItem        &GetLanguage( bool = true ) const;
    inline const SvxEscapementItem      &GetEscapement( bool = true ) const;
    inline const SvxCaseMapItem         &GetCaseMap( bool = true ) const;
    inline const SvxNoHyphenItem      &GetNoHyphenHere( bool = true ) const;
    inline const SvxFontItem          &GetCJKFont( bool = true ) const;
    inline const SvxFontHeightItem    &GetCJKSize( bool = true ) const;
    inline const SvxLanguageItem      &GetCJKLanguage( bool = true ) const;
    inline const SvxPostureItem       &GetCJKPosture( bool = true ) const;
    inline const SvxWeightItem        &GetCJKWeight( bool = true ) const;
    inline const SvxFontItem          &GetCTLFont( bool = true ) const;
    inline const SvxFontHeightItem    &GetCTLSize( bool = true ) const;
    inline const SvxLanguageItem      &GetCTLLanguage( bool = true ) const;
    inline const SvxPostureItem       &GetCTLPosture( bool = true ) const;
    inline const SvxWeightItem        &GetCTLWeight( bool = true ) const;
    inline const SvxEmphasisMarkItem  &GetEmphasisMark( bool = true ) const;
    inline const SvxTwoLinesItem      &Get2Lines( bool = true ) const;
    inline const SvxCharScaleWidthItem    &GetCharScaleW( bool = true ) const;
    inline const SvxCharRotateItem        &GetCharRotate( bool = true ) const;
    inline const SvxCharReliefItem        &GetCharRelief( bool = true ) const;
    inline const SvxCharHiddenItem      &GetCharHidden( bool = true ) const;

    // Frame attributes. Implementation in frmatr.hxx.
    inline const SwFormatFillOrder       &GetFillOrder( bool = true ) const;
    inline const SwFormatFrameSize             &GetFrameSize( bool = true ) const;
    inline const SvxPaperBinItem      &GetPaperBin( bool = true ) const;
    inline const SvxLRSpaceItem           &GetLRSpace( bool = true ) const;
    inline const SvxULSpaceItem           &GetULSpace( bool = true ) const;
    inline const SwFormatContent           &GetContent( bool = true ) const;
    inline const SwFormatHeader          &GetHeader( bool = true ) const;
    inline const SwFormatFooter          &GetFooter( bool = true ) const;
    inline const SvxPrintItem             &GetPrint( bool = true ) const;
    inline const SvxOpaqueItem            &GetOpaque( bool = true ) const;
    inline const SvxProtectItem           &GetProtect( bool = true ) const;
    inline const SwFormatSurround            &GetSurround( bool = true ) const;
    inline const SwFormatVertOrient      &GetVertOrient( bool = true ) const;
    inline const SwFormatHoriOrient      &GetHoriOrient( bool = true ) const;
    inline const SwFormatAnchor          &GetAnchor( bool = true ) const;
    inline const SvxBoxItem               &GetBox( bool = true ) const;
    inline const SvxFormatKeepItem         &GetKeep( bool = true ) const;
    inline const SvxBrushItem           &GetBackground( bool = true ) const;
    inline const SvxShadowItem            &GetShadow( bool = true ) const;
    inline const SwFormatPageDesc            &GetPageDesc( bool = true ) const;
    inline const SvxFormatBreakItem      &GetBreak( bool = true ) const;
    inline const SwFormatCol                 &GetCol( bool = true ) const;
    inline const SvxMacroItem             &GetMacro( bool = true ) const;
    inline const SwFormatURL             &GetURL( bool = true ) const;
    inline const SwFormatEditInReadonly  &GetEditInReadonly( bool = true ) const;
    inline const SwFormatLayoutSplit     &GetLayoutSplit( bool = true ) const;
    inline const SwFormatRowSplit          &GetRowSplit( bool = true ) const;
    inline const SwFormatChain             &GetChain( bool = true ) const;
    inline const SwFormatLineNumber      &GetLineNumber( bool = true ) const;
    inline const SwFormatFootnoteAtTextEnd     &GetFootnoteAtTextEnd( bool = true ) const;
    inline const SwFormatEndAtTextEnd     &GetEndAtTextEnd( bool = true ) const;
    inline const SwFormatNoBalancedColumns &GetBalancedColumns( bool = true ) const;
    inline const SvxFrameDirectionItem    &GetFrameDir( bool = true ) const;
    inline const SwHeaderAndFooterEatSpacingItem &GetHeaderAndFooterEatSpacing( bool = true ) const;
    inline const SwFormatFollowTextFlow    &GetFollowTextFlow(bool = true) const;
    inline const SwFormatWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(bool = true) const;
    inline const SdrTextVertAdjustItem& GetTextVertAdjust(bool = true) const;

    // Graphic attributes   - implementation in grfatr.hxx
    inline const SwMirrorGrf            &GetMirrorGrf( bool = true ) const;
    inline const SwCropGrf            &GetCropGrf( bool = true ) const;
    inline const SwRotationGrf            &GetRotationGrf(bool = true ) const;
    inline const SwLuminanceGrf       &GetLuminanceGrf(bool = true ) const;
    inline const SwContrastGrf            &GetContrastGrf(bool = true ) const;
    inline const SwChannelRGrf            &GetChannelRGrf(bool = true ) const;
    inline const SwChannelGGrf            &GetChannelGGrf(bool = true ) const;
    inline const SwChannelBGrf            &GetChannelBGrf(bool = true ) const;
    inline const SwGammaGrf           &GetGammaGrf(bool = true ) const;
    inline const SwInvertGrf          &GetInvertGrf(bool = true ) const;
    inline const SwTransparencyGrf        &GetTransparencyGrf(bool = true ) const;
    inline const SwDrawModeGrf            &GetDrawModeGrf(bool = true ) const;

    // Paragraph attributes - implementation in paratr.hxx
    inline const SvxLineSpacingItem       &GetLineSpacing( bool = true ) const;
    inline const SvxAdjustItem            &GetAdjust( bool = true ) const;
    inline const SvxFormatSplitItem      &GetSplit( bool = true ) const;
    inline const SwRegisterItem           &GetRegister( bool = true ) const;
    inline const SwNumRuleItem            &GetNumRule( bool = true ) const;
    inline const SvxWidowsItem            &GetWidows( bool = true ) const;
    inline const SvxOrphansItem           &GetOrphans( bool = true ) const;
    inline const SvxTabStopItem           &GetTabStops( bool = true ) const;
    inline const SvxHyphenZoneItem        &GetHyphenZone( bool = true ) const;
    inline const SwFormatDrop                &GetDrop( bool = true ) const;
    inline const SvxScriptSpaceItem       &GetScriptSpace(bool = true) const;
    inline const SvxHangingPunctuationItem &GetHangingPunctuation(bool = true) const;
    inline const SvxForbiddenRuleItem     &GetForbiddenRule(bool = true) const;
    inline const SvxParaVertAlignItem &GetParaVertAlign(bool = true) const;
    inline const SvxParaGridItem        &GetParaGrid(bool = true) const;
    inline const SwParaConnectBorderItem &GetParaConnectBorder(bool = true ) const;

    // Tablebox attributes  - implementation in cellatr.hxx
    inline  const SwTableBoxNumFormat       &GetTableBoxNumFormat( bool = true ) const;
    inline  const SwTableBoxFormula     &GetTableBoxFormula( bool = true ) const;
    inline  const SwTableBoxValue           &GetTableBoxValue( bool = true ) const;

    inline  const SwFormatLayoutInCell &GetLayoutInCell( bool = true ) const;
};

//Helper for filters to find true lineheight of a font
SW_DLLPUBLIC tools::Long AttrSetToLineHeight( const IDocumentSettingAccess& rIDocumentSettingAccess,
                          const SwAttrSet &rSet,
                          const vcl::RenderContext &rOut, sal_Int16 nScript);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
