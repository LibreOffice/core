/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

// Frame-Attribute
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
// OD 18.09.2003 #i18732#
class SwFmtFollowTextFlow;
// OD 2004-05-05 #i28701#
class SwFmtWrapInfluenceOnObjPos;

// Grafik-Attribute
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

// Paragraph-Attribute
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

// TabellenBox-Attribute
class SwTblBoxNumFormat;
class SwTblBoxFormula;
class SwTblBoxValue;

class SwAttrPool : public SfxItemPool
{
    friend void _InitCore();            // fuers anlegen/zerstoeren der
    friend void _FinitCore();           // Versionsmaps
    static sal_uInt16* pVersionMap1;
    static sal_uInt16* pVersionMap2;
    static sal_uInt16* pVersionMap3;
    static sal_uInt16* pVersionMap4;
    // OD 2004-01-21 #i18732# - due to extension of attribute set a new version
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

    static sal_uInt16* GetVersionMap1() { return pVersionMap1; }
    static sal_uInt16* GetVersionMap2() { return pVersionMap2; }
    static sal_uInt16* GetVersionMap3() { return pVersionMap3; }
    static sal_uInt16* GetVersionMap6() { return pVersionMap4; }
};


class SW_DLLPUBLIC SwAttrSet : public SfxItemSet
{
    // Pointer fuers Modify-System
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

    // ein Item oder einen Bereich loeschen
    sal_uInt16 ClearItem_BC( sal_uInt16 nWhich, SwAttrSet* pOld, SwAttrSet* pNew );
    sal_uInt16 ClearItem_BC( sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                        SwAttrSet* pOld = 0, SwAttrSet* pNew = 0 );

    int Intersect_BC( const SfxItemSet& rSet, SwAttrSet* pOld, SwAttrSet* pNew );

    void GetPresentation( SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String &rText ) const;

    SwAttrPool* GetPool() const { return (SwAttrPool*)SfxItemSet::GetPool(); }

    // kopiere die Attribute ggfs. ueber Dokumentgrenzen
    void CopyToModify( SwModify& rMod ) const;

    // ----------------------------------------------------------------
    // Sonderbehandlung fuer einige Attribute
    // Setze den Modify-Pointer (alten pDefinedIn) bei folgenden Attributen:
    //  - SwFmtDropCaps
    //  - SwFmtPageDesc
    // (Wird beim Einfuegen in Formate/Nodes gerufen)
    // Second version is for the SwAttrSet handles of SwCntntNode.
    bool SetModifyAtAttr( const SwModify* pModify );

    // Das Doc wird jetzt am SwAttrPool gesetzt. Dadurch hat man es immer
    // im Zugriff.
    const SwDoc *GetDoc() const { return GetPool()->GetDoc(); }
          SwDoc *GetDoc()       { return GetPool()->GetDoc(); }

    // GetMethoden: das Bool gibt an, ob nur im Set (sal_False) oder auch in
    //              den Parents gesucht werden soll. Wird nichts gefunden,
    //              wird das deflt. Attribut returnt.
    // Charakter-Attribute  - impl. steht im charatr.hxx
    // AMA 12.10.94: Umstellung von SwFmt... auf Svx...
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

    // Frame-Attribute  - impl. steht im frmatr.hxx
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
    // OD 18.09.2003 #i18732#
    inline const SwFmtFollowTextFlow    &GetFollowTextFlow(sal_Bool = sal_True) const;
    // OD 2004-05-05 #i28701#
    inline const SwFmtWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(sal_Bool = sal_True) const;

    // Grafik-Attribute - impl. steht im grfatr.hxx
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

    // Paragraph-Attribute  - impl. steht im paratr.hxx
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

    // TabellenBox-Attribute    - impl. steht im cellatr.hxx
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
