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
#include <tools/solar.h>
#include <tools/mempool.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <swdllapi.h>

class SwModify;
class SwDoc;
class OutputDevice;
class IDocumentSettingAccess;
class SfxBoolItem;
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
class SvxPropSizeItem;
class SvxFontItem;
class SvxColorItem;
class SvxCharSetColorItem;
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
class SwFmtFillOrder;
class SwFmtFrmSize;
class SvxPaperBinItem;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFmtCntnt;
class SwFmtHeader;
class SwFmtFooter;
class SvxPrintItem;
class SvxOpaqueItem;
class SvxProtectItem;
class SwFmtSurround;
class SwFmtVertOrient;
class SwFmtHoriOrient;
class SwFmtAnchor;
class SvxBoxItem;
class SvxBrushItem;
class XFillStyleItem;
class XFillGradientItem;
class SvxShadowItem;
class SwFmtPageDesc;
class SvxFmtBreakItem;
class SwFmtCol;
class SvxMacroItem;
class SvxFmtKeepItem;
class SwFmtURL;
class SwFmtLineNumber;
class SwFmtEditInReadonly;
class SwFmtLayoutSplit;
class SwFmtRowSplit;
class SwFmtChain;
class SwFmtFtnAtTxtEnd;
class SwFmtEndAtTxtEnd;
class SwFmtNoBalancedColumns;
class SvxFrameDirectionItem;
class SwTextGridItem;
class SwHeaderAndFooterEatSpacingItem;
class SwFmtFollowTextFlow;
class SwFmtWrapInfluenceOnObjPos;
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
class SvxFmtSplitItem;
class SwRegisterItem;
class SwNumRuleItem;
class SvxWidowsItem;
class SvxOrphansItem;
class SvxTabStopItem;
class SvxHyphenZoneItem;
class SwFmtDrop;
class SvxScriptSpaceItem;
class SvxHangingPunctuationItem;
class SvxForbiddenRuleItem;
class SvxParaVertAlignItem;
class SvxParaGridItem;
class SwParaConnectBorderItem;

// TableBox attributes
class SwTblBoxNumFormat;
class SwTblBoxFormula;
class SwTblBoxValue;

class SwAttrPool : public SfxItemPool
{
    friend void _InitCore();            // For creating/deleting of version maps.
    friend void _FinitCore();
    static sal_uInt16* pVersionMap1;
    static sal_uInt16* pVersionMap2;
    static sal_uInt16* pVersionMap3;
    static sal_uInt16* pVersionMap4;
    // due to extension of attribute set a new version
    // map for binary filter is necessary (version map 5).
    static sal_uInt16* pVersionMap5;
    static sal_uInt16* pVersionMap6;
    static sal_uInt16* pVersionMap7;

    SwDoc* pDoc;

public:
    SwAttrPool( SwDoc* pDoc );
protected:
    virtual ~SwAttrPool();
public:

          SwDoc* GetDoc()           { return pDoc; }
    const SwDoc* GetDoc() const     { return pDoc; }

};

class SW_DLLPUBLIC SwAttrSet : public SfxItemSet
{
    // Pointer for Modify-System
    SwAttrSet *pOldSet, *pNewSet;

    // Notification-Callback
    virtual void Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew ) SAL_OVERRIDE;

    void PutChgd( const SfxPoolItem& rI ) { SfxItemSet::PutDirect( rI ); }
public:
    SwAttrSet( SwAttrPool&, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
    SwAttrSet( SwAttrPool&, const sal_uInt16* nWhichPairTable );
    SwAttrSet( const SwAttrSet& );

    virtual SfxItemSet* Clone(bool bItems = true, SfxItemPool *pToPool = 0) const SAL_OVERRIDE;

    bool Put_BC( const SfxPoolItem& rAttr, SwAttrSet* pOld, SwAttrSet* pNew );
    bool Put_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

    // Delete an item or a range.
    sal_uInt16 ClearItem_BC( sal_uInt16 nWhich, SwAttrSet* pOld, SwAttrSet* pNew );
    sal_uInt16 ClearItem_BC( sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                        SwAttrSet* pOld = 0, SwAttrSet* pNew = 0 );

    int Intersect_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

    void GetPresentation( SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString &rText ) const;

    SwAttrPool* GetPool() const { return (SwAttrPool*)SfxItemSet::GetPool(); }

    // Copy attributes, if necessary across documents.
    void CopyToModify( SwModify& rMod ) const;

    // Special treatment for some attributes.
    // Set Modify-pointer (the old pDefinedIn) at the following attributes:
    //  - SwFmtDropCaps
    //  - SwFmtPageDesc
    // (Is called at insert in formats/nodes.)
    // Second version is for the SwAttrSet handles of SwCntntNode.
    bool SetModifyAtAttr( const SwModify* pModify );

    // Document is set at SwAttrPool. Therefore it is always accessible.
    const SwDoc *GetDoc() const { return GetPool()->GetDoc(); }
          SwDoc *GetDoc()       { return GetPool()->GetDoc(); }

    // Get methods: bool indicates whether to search only in Set (sal_False)
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
    inline const SvxPropSizeItem        &GetPropSize( bool = true ) const;
    inline const SvxFontItem            &GetFont( bool = true ) const;
    inline const SvxColorItem           &GetColor( bool = true ) const;
    inline const SvxCharSetColorItem    &GetCharSetColor( bool = true ) const;
    inline const SvxLanguageItem        &GetLanguage( bool = true ) const;
    inline const SvxEscapementItem      &GetEscapement( bool = true ) const;
    inline const SvxCaseMapItem         &GetCaseMap( bool = true ) const;
    inline const SvxNoHyphenItem      &GetNoHyphenHere( bool = true ) const;
    inline const SvxBlinkItem         &GetBlink( bool = true ) const;
    inline const SvxBrushItem         &GetChrBackground( bool = true ) const;
    inline const SvxBrushItem         &GetChrHighlight( bool = true ) const;
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
    inline const SfxBoolItem              &GetWritingDirection( bool = true ) const;
    inline const SvxEmphasisMarkItem  &GetEmphasisMark( bool = true ) const;
    inline const SvxTwoLinesItem      &Get2Lines( bool = true ) const;
    inline const SvxCharScaleWidthItem    &GetCharScaleW( bool = true ) const;
    inline const SvxCharRotateItem        &GetCharRotate( bool = true ) const;
    inline const SvxCharReliefItem        &GetCharRelief( bool = true ) const;
    inline const SvxCharHiddenItem      &GetCharHidden( bool = true ) const;
    inline const SvxBoxItem      &GetCharBorder( bool = true ) const;
    inline const SvxShadowItem      &GetCharShadow( bool = true ) const;

    // Frame attributes. Implementation in frmatr.hxx.
    inline const SwFmtFillOrder       &GetFillOrder( bool = true ) const;
    inline const SwFmtFrmSize             &GetFrmSize( bool = true ) const;
    inline const SvxPaperBinItem      &GetPaperBin( bool = true ) const;
    inline const SvxLRSpaceItem           &GetLRSpace( bool = true ) const;
    inline const SvxULSpaceItem           &GetULSpace( bool = true ) const;
    inline const SwFmtCntnt           &GetCntnt( bool = true ) const;
    inline const SwFmtHeader          &GetHeader( bool = true ) const;
    inline const SwFmtFooter          &GetFooter( bool = true ) const;
    inline const SvxPrintItem             &GetPrint( bool = true ) const;
    inline const SvxOpaqueItem            &GetOpaque( bool = true ) const;
    inline const SvxProtectItem           &GetProtect( bool = true ) const;
    inline const SwFmtSurround            &GetSurround( bool = true ) const;
    inline const SwFmtVertOrient      &GetVertOrient( bool = true ) const;
    inline const SwFmtHoriOrient      &GetHoriOrient( bool = true ) const;
    inline const SwFmtAnchor          &GetAnchor( bool = true ) const;
    inline const SvxBoxItem               &GetBox( bool = true ) const;
    inline const SvxFmtKeepItem         &GetKeep( bool = true ) const;
    inline const SvxBrushItem           &GetBackground( bool = true ) const;
    inline const XFillStyleItem           &GetFillStyle( bool = true ) const;
    inline const XFillGradientItem        &GetFillGradient( bool = true ) const;
    inline const SvxShadowItem            &GetShadow( bool = true ) const;
    inline const SwFmtPageDesc            &GetPageDesc( bool = true ) const;
    inline const SvxFmtBreakItem      &GetBreak( bool = true ) const;
    inline const SwFmtCol                 &GetCol( bool = true ) const;
    inline const SvxMacroItem             &GetMacro( bool = true ) const;
    inline const SwFmtURL             &GetURL( bool = true ) const;
    inline const SwFmtEditInReadonly  &GetEditInReadonly( bool = true ) const;
    inline const SwFmtLayoutSplit     &GetLayoutSplit( bool = true ) const;
    inline const SwFmtRowSplit          &GetRowSplit( bool = true ) const;
    inline const SwFmtChain             &GetChain( bool = true ) const;
    inline const SwFmtLineNumber      &GetLineNumber( bool = true ) const;
    inline const SwFmtFtnAtTxtEnd     &GetFtnAtTxtEnd( bool = true ) const;
    inline const SwFmtEndAtTxtEnd     &GetEndAtTxtEnd( bool = true ) const;
    inline const SwFmtNoBalancedColumns &GetBalancedColumns( bool = true ) const;
    inline const SvxFrameDirectionItem    &GetFrmDir( bool = true ) const;
    inline const SwTextGridItem         &GetTextGrid( bool = true ) const;
    inline const SwHeaderAndFooterEatSpacingItem &GetHeaderAndFooterEatSpacing( bool = true ) const;
    inline const SwFmtFollowTextFlow    &GetFollowTextFlow(bool = true) const;
    inline const SwFmtWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(bool = true) const;
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
    inline const SvxFmtSplitItem      &GetSplit( bool = true ) const;
    inline const SwRegisterItem           &GetRegister( bool = true ) const;
    inline const SwNumRuleItem            &GetNumRule( bool = true ) const;
    inline const SvxWidowsItem            &GetWidows( bool = true ) const;
    inline const SvxOrphansItem           &GetOrphans( bool = true ) const;
    inline const SvxTabStopItem           &GetTabStops( bool = true ) const;
    inline const SvxHyphenZoneItem        &GetHyphenZone( bool = true ) const;
    inline const SwFmtDrop                &GetDrop( bool = true ) const;
    inline const SvxScriptSpaceItem       &GetScriptSpace(bool = true) const;
    inline const SvxHangingPunctuationItem &GetHangingPunctuation(bool = true) const;
    inline const SvxForbiddenRuleItem     &GetForbiddenRule(bool = true) const;
    inline const SvxParaVertAlignItem &GetParaVertAlign(bool = true) const;
    inline const SvxParaGridItem        &GetParaGrid(bool = true) const;
    inline const SwParaConnectBorderItem &GetParaConnectBorder(bool = true ) const;

    // Tablebox attributes  - implementation in cellatr.hxx
    inline  const SwTblBoxNumFormat       &GetTblBoxNumFmt( bool = true ) const;
    inline  const SwTblBoxFormula     &GetTblBoxFormula( bool = true ) const;
    inline  const SwTblBoxValue           &GetTblBoxValue( bool = true ) const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwAttrSet)
};

//Helper for filters to find true lineheight of a font
SW_DLLPUBLIC long AttrSetToLineHeight( const IDocumentSettingAccess& rIDocumentSettingAccess,
                          const SwAttrSet &rSet,
                          const OutputDevice &rOut, sal_Int16 nScript);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
