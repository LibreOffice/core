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
#ifndef _SWATRSET_HXX
#define _SWATRSET_HXX
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
    virtual void Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew );

    void PutChgd( const SfxPoolItem& rI ) { SfxItemSet::PutDirect( rI ); }
public:
    SwAttrSet( SwAttrPool&, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
    SwAttrSet( SwAttrPool&, const sal_uInt16* nWhichPairTable );
    SwAttrSet( const SwAttrSet& );

    virtual SfxItemSet* Clone(sal_Bool bItems = sal_True, SfxItemPool *pToPool = 0) const;

    int Put_BC( const SfxPoolItem& rAttr, SwAttrSet* pOld, SwAttrSet* pNew );
    int Put_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

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
    inline const SvxPostureItem           &GetPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem          &GetWeight( sal_Bool = sal_True ) const;
    inline const SvxShadowedItem        &GetShadowed( sal_Bool = sal_True ) const;
    inline const SvxAutoKernItem        &GetAutoKern( sal_Bool = sal_True ) const;
    inline const SvxWordLineModeItem    &GetWordLineMode( sal_Bool = sal_True ) const;
    inline const SvxContourItem         &GetContour( sal_Bool = sal_True ) const;
    inline const SvxKerningItem         &GetKerning( sal_Bool = sal_True ) const;
    inline const SvxUnderlineItem       &GetUnderline( sal_Bool = sal_True ) const;
    inline const SvxOverlineItem        &GetOverline( sal_Bool = sal_True ) const;
    inline const SvxCrossedOutItem      &GetCrossedOut( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem        &GetSize( sal_Bool = sal_True ) const;
    inline const SvxPropSizeItem        &GetPropSize( sal_Bool = sal_True ) const;
    inline const SvxFontItem            &GetFont( sal_Bool = sal_True ) const;
    inline const SvxColorItem           &GetColor( sal_Bool = sal_True ) const;
    inline const SvxCharSetColorItem    &GetCharSetColor( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem        &GetLanguage( sal_Bool = sal_True ) const;
    inline const SvxEscapementItem      &GetEscapement( sal_Bool = sal_True ) const;
    inline const SvxCaseMapItem         &GetCaseMap( sal_Bool = sal_True ) const;
    inline const SvxNoHyphenItem      &GetNoHyphenHere( sal_Bool = sal_True ) const;
    inline const SvxBlinkItem         &GetBlink( sal_Bool = sal_True ) const;
    inline const SvxBrushItem         &GetChrBackground( sal_Bool = sal_True ) const;
    inline const SvxFontItem          &GetCJKFont( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem    &GetCJKSize( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem      &GetCJKLanguage( sal_Bool = sal_True ) const;
    inline const SvxPostureItem       &GetCJKPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem        &GetCJKWeight( sal_Bool = sal_True ) const;
    inline const SvxFontItem          &GetCTLFont( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem    &GetCTLSize( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem      &GetCTLLanguage( sal_Bool = sal_True ) const;
    inline const SvxPostureItem       &GetCTLPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem        &GetCTLWeight( sal_Bool = sal_True ) const;
    inline const SfxBoolItem              &GetWritingDirection( sal_Bool = sal_True ) const;
    inline const SvxEmphasisMarkItem  &GetEmphasisMark( sal_Bool = sal_True ) const;
    inline const SvxTwoLinesItem      &Get2Lines( sal_Bool = sal_True ) const;
    inline const SvxCharScaleWidthItem    &GetCharScaleW( sal_Bool = sal_True ) const;
    inline const SvxCharRotateItem        &GetCharRotate( sal_Bool = sal_True ) const;
    inline const SvxCharReliefItem        &GetCharRelief( sal_Bool = sal_True ) const;
    inline const SvxCharHiddenItem      &GetCharHidden( sal_Bool = sal_True ) const;
    inline const SvxBoxItem      &GetCharBorder( sal_Bool = sal_True ) const;

    // Frame attributes. Implementation in frmatr.hxx.
    inline const SwFmtFillOrder       &GetFillOrder( sal_Bool = sal_True ) const;
    inline const SwFmtFrmSize             &GetFrmSize( sal_Bool = sal_True ) const;
    inline const SvxPaperBinItem      &GetPaperBin( sal_Bool = sal_True ) const;
    inline const SvxLRSpaceItem           &GetLRSpace( sal_Bool = sal_True ) const;
    inline const SvxULSpaceItem           &GetULSpace( sal_Bool = sal_True ) const;
    inline const SwFmtCntnt           &GetCntnt( sal_Bool = sal_True ) const;
    inline const SwFmtHeader          &GetHeader( sal_Bool = sal_True ) const;
    inline const SwFmtFooter          &GetFooter( sal_Bool = sal_True ) const;
    inline const SvxPrintItem             &GetPrint( sal_Bool = sal_True ) const;
    inline const SvxOpaqueItem            &GetOpaque( sal_Bool = sal_True ) const;
    inline const SvxProtectItem           &GetProtect( sal_Bool = sal_True ) const;
    inline const SwFmtSurround            &GetSurround( sal_Bool = sal_True ) const;
    inline const SwFmtVertOrient      &GetVertOrient( sal_Bool = sal_True ) const;
    inline const SwFmtHoriOrient      &GetHoriOrient( sal_Bool = sal_True ) const;
    inline const SwFmtAnchor          &GetAnchor( sal_Bool = sal_True ) const;
    inline const SvxBoxItem               &GetBox( sal_Bool = sal_True ) const;
    inline const SvxFmtKeepItem         &GetKeep( sal_Bool = sal_True ) const;
    inline const SvxBrushItem           &GetBackground( sal_Bool = sal_True ) const;
    inline const XFillStyleItem           &GetFillStyle( sal_Bool = sal_True ) const;
    inline const XFillGradientItem        &GetFillGradient( sal_Bool = sal_True ) const;
    inline const SvxShadowItem            &GetShadow( sal_Bool = sal_True ) const;
    inline const SwFmtPageDesc            &GetPageDesc( sal_Bool = sal_True ) const;
    inline const SvxFmtBreakItem      &GetBreak( sal_Bool = sal_True ) const;
    inline const SwFmtCol                 &GetCol( sal_Bool = sal_True ) const;
    inline const SvxMacroItem             &GetMacro( sal_Bool = sal_True ) const;
    inline const SwFmtURL             &GetURL( sal_Bool = sal_True ) const;
    inline const SwFmtEditInReadonly  &GetEditInReadonly( sal_Bool = sal_True ) const;
    inline const SwFmtLayoutSplit     &GetLayoutSplit( sal_Bool = sal_True ) const;
    inline const SwFmtRowSplit          &GetRowSplit( sal_Bool = sal_True ) const;
    inline const SwFmtChain             &GetChain( sal_Bool = sal_True ) const;
    inline const SwFmtLineNumber      &GetLineNumber( sal_Bool = sal_True ) const;
    inline const SwFmtFtnAtTxtEnd     &GetFtnAtTxtEnd( sal_Bool = sal_True ) const;
    inline const SwFmtEndAtTxtEnd     &GetEndAtTxtEnd( sal_Bool = sal_True ) const;
    inline const SwFmtNoBalancedColumns &GetBalancedColumns( sal_Bool = sal_True ) const;
    inline const SvxFrameDirectionItem    &GetFrmDir( sal_Bool = sal_True ) const;
    inline const SwTextGridItem         &GetTextGrid( sal_Bool = sal_True ) const;
    inline const SwHeaderAndFooterEatSpacingItem &GetHeaderAndFooterEatSpacing( sal_Bool = sal_True ) const;
    inline const SwFmtFollowTextFlow    &GetFollowTextFlow(sal_Bool = sal_True) const;
    inline const SwFmtWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(sal_Bool = sal_True) const;

    // Graphic attributes   - implementation in grfatr.hxx
    inline const SwMirrorGrf            &GetMirrorGrf( sal_Bool = sal_True ) const;
    inline const SwCropGrf            &GetCropGrf( sal_Bool = sal_True ) const;
    inline const SwRotationGrf            &GetRotationGrf(sal_Bool = sal_True ) const;
    inline const SwLuminanceGrf       &GetLuminanceGrf(sal_Bool = sal_True ) const;
    inline const SwContrastGrf            &GetContrastGrf(sal_Bool = sal_True ) const;
    inline const SwChannelRGrf            &GetChannelRGrf(sal_Bool = sal_True ) const;
    inline const SwChannelGGrf            &GetChannelGGrf(sal_Bool = sal_True ) const;
    inline const SwChannelBGrf            &GetChannelBGrf(sal_Bool = sal_True ) const;
    inline const SwGammaGrf           &GetGammaGrf(sal_Bool = sal_True ) const;
    inline const SwInvertGrf          &GetInvertGrf(sal_Bool = sal_True ) const;
    inline const SwTransparencyGrf        &GetTransparencyGrf(sal_Bool = sal_True ) const;
    inline const SwDrawModeGrf            &GetDrawModeGrf(sal_Bool = sal_True ) const;

    // Paragraph attributes - implementation in paratr.hxx
    inline const SvxLineSpacingItem       &GetLineSpacing( sal_Bool = sal_True ) const;
    inline const SvxAdjustItem            &GetAdjust( sal_Bool = sal_True ) const;
    inline const SvxFmtSplitItem      &GetSplit( sal_Bool = sal_True ) const;
    inline const SwRegisterItem           &GetRegister( sal_Bool = sal_True ) const;
    inline const SwNumRuleItem            &GetNumRule( sal_Bool = sal_True ) const;
    inline const SvxWidowsItem            &GetWidows( sal_Bool = sal_True ) const;
    inline const SvxOrphansItem           &GetOrphans( sal_Bool = sal_True ) const;
    inline const SvxTabStopItem           &GetTabStops( sal_Bool = sal_True ) const;
    inline const SvxHyphenZoneItem        &GetHyphenZone( sal_Bool = sal_True ) const;
    inline const SwFmtDrop                &GetDrop( sal_Bool = sal_True ) const;
    inline const SvxScriptSpaceItem       &GetScriptSpace(sal_Bool = sal_True) const;
    inline const SvxHangingPunctuationItem &GetHangingPunctuation(sal_Bool = sal_True) const;
    inline const SvxForbiddenRuleItem     &GetForbiddenRule(sal_Bool = sal_True) const;
    inline const SvxParaVertAlignItem &GetParaVertAlign(sal_Bool = sal_True) const;
    inline const SvxParaGridItem        &GetParaGrid(sal_Bool = sal_True) const;
    inline const SwParaConnectBorderItem &GetParaConnectBorder(sal_Bool = sal_True ) const;

    // Tablebox attributes  - implementation in cellatr.hxx
    inline  const SwTblBoxNumFormat       &GetTblBoxNumFmt( sal_Bool = sal_True ) const;
    inline  const SwTblBoxFormula     &GetTblBoxFormula( sal_Bool = sal_True ) const;
    inline  const SwTblBoxValue           &GetTblBoxValue( sal_Bool = sal_True ) const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwAttrSet)
};

//Helper for filters to find true lineheight of a font
SW_DLLPUBLIC long AttrSetToLineHeight( const IDocumentSettingAccess& rIDocumentSettingAccess,
                          const SwAttrSet &rSet,
                          const OutputDevice &rOut, sal_Int16 nScript);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
