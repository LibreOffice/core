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
#ifndef INCLUDED_SW_INC_FORMAT_HXX
#define INCLUDED_SW_INC_FORMAT_HXX

#include <tools/solar.h>
#include "swdllapi.h"
#include <swatrset.hxx>
#include <calbck.hxx>
#include <hintids.hxx>
#include <boost/shared_ptr.hpp>

class IDocumentSettingAccess;
class IDocumentDrawModelAccess;
class IDocumentLayoutAccess;
class IDocumentTimerAccess;
class IDocumentFieldsAccess;
class IDocumentChartDataProviderAccess;
class SwDoc;
class SfxGrabBagItem;

namespace drawinglayer { namespace attribute {
    class SdrAllFillAttributesHelper;
    typedef boost::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
}}

/// Base class for various Writer styles.
class SW_DLLPUBLIC SwFmt : public SwModify
{
    OUString aFmtName;
    SwAttrSet aSet;

    sal_uInt16 nWhichId;
    sal_uInt16 nPoolFmtId;        /**< Id for "automatically" created formats.
                                       (is not hard attribution!!!) */
    sal_uInt16 nPoolHelpId;       ///< HelpId for this Pool-style.
    sal_uInt8 nPoolHlpFileId;     ///< FilePos to Doc to these style helps.
    bool   bWritten : 1;      ///< TRUE: already written.
    bool   bAutoFmt : 1;      /**< FALSE: it is a template.
                                       default is true! */
    bool   bFmtInDTOR : 1;    /**< TRUE: Format becomes deleted. In order to be able
                                       to recognize this in FmtChg-message!! */
    bool   bAutoUpdateFmt : 1;/**< TRUE: Set attributes of a whole paragraph
                                       at format (UI-side!). */
    bool bHidden : 1;
    boost::shared_ptr<SfxGrabBagItem> m_pGrabBagItem; ///< Style InteropGrabBag.

protected:
    SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
            const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich );
    SwFmt( SwAttrPool& rPool, const OUString &rFmtNm, const sal_uInt16* pWhichRanges,
            SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich );
    SwFmt( const SwFmt& rFmt );
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue ) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();     ///< Already in base class Client.

    virtual ~SwFmt();
    SwFmt &operator=(const SwFmt&);

    /// for Querying of Writer-functions.
    sal_uInt16 Which() const { return nWhichId; }

    /// Query format information.
    virtual bool GetInfo( SfxPoolItem& ) const SAL_OVERRIDE;

    /// Copy attributes even among documents.
    void CopyAttrs( const SwFmt&, bool bReplace=true );

    /// Delete all attributes that are not in rFmt.
    void DelDiffs( const SfxItemSet& rSet );
    void DelDiffs( const SwFmt& rFmt ) { DelDiffs( rFmt.GetAttrSet() ); }

    /// 0 is Default.
    bool SetDerivedFrom(SwFmt *pDerivedFrom = 0);

    /// If bInParents is FALSE, search only in this format for attribute.
    //UUUUinline
    const SfxPoolItem& GetFmtAttr( sal_uInt16 nWhich,
                                   bool bInParents = true ) const;
    //UUUUinline
    SfxItemState GetItemState( sal_uInt16 nWhich, bool bSrchInParent = true,
                                    const SfxPoolItem **ppItem = 0 ) const;
    SfxItemState GetBackgroundState(SvxBrushItem &rItem,
                                    bool bSrchInParent = true) const;
    virtual bool SetFmtAttr( const SfxPoolItem& rAttr );
    virtual bool SetFmtAttr( const SfxItemSet& rSet );
    virtual bool ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );

    /** Takes all hints from Delta-Array,
        @return count of deleted hints. */
    virtual sal_uInt16 ResetAllFmtAttr();

    inline SwFmt* DerivedFrom() const { return const_cast<SwFmt*>(static_cast<const SwFmt*>(GetRegisteredIn())); }
    inline bool IsDefault() const { return DerivedFrom() == 0; }

    inline OUString GetName() const   { return aFmtName; }
    void SetName( const OUString& rNewName, bool bBroadcast=false );
    inline void SetName( const sal_Char* pNewName,
                         bool bBroadcast=false);

    /// For querying the attribute array.
    inline const SwAttrSet& GetAttrSet() const { return aSet; }

    /** Das Doc wird jetzt am SwAttrPool gesetzt. Dadurch hat man es immer
       im Zugriff. */
    const SwDoc *GetDoc() const         { return aSet.GetDoc(); }
          SwDoc *GetDoc()               { return aSet.GetDoc(); }

    /// Provides access to the document settings interface.
    const IDocumentSettingAccess* getIDocumentSettingAccess() const;

    /// Provides access to the document draw model interface.
    const IDocumentDrawModelAccess* getIDocumentDrawModelAccess() const;
          IDocumentDrawModelAccess* getIDocumentDrawModelAccess();

    /// Provides access to the document layout interface.
    const IDocumentLayoutAccess* getIDocumentLayoutAccess() const;
          IDocumentLayoutAccess* getIDocumentLayoutAccess();

     /// Provides access to the document idle timer interface.
     IDocumentTimerAccess* getIDocumentTimerAccess();

     /// Provides access to the document idle timer interface.
    IDocumentFieldsAccess* getIDocumentFieldsAccess();

     /// Gives access to the chart data-provider.
    IDocumentChartDataProviderAccess* getIDocumentChartDataProviderAccess();

    /// Get and set Pool style IDs.
    sal_uInt16 GetPoolFmtId() const { return nPoolFmtId; }
    void SetPoolFmtId( sal_uInt16 nId ) { nPoolFmtId = nId; }

    /// Get and set Help-IDs for document templates.
    sal_uInt16 GetPoolHelpId() const { return nPoolHelpId; }
    void SetPoolHelpId( sal_uInt16 nId ) { nPoolHelpId = nId; }
    sal_uInt8 GetPoolHlpFileId() const { return nPoolHlpFileId; }
    void SetPoolHlpFileId( sal_uInt8 nId ) { nPoolHlpFileId = nId; }

    /// Get attribute-description. Returns passed string.
    void GetPresentation( SfxItemPresentation ePres,
        SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString &rText ) const
        { aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, rText ); }

    /// Format-ID for reading/writing:
    void   ResetWritten()    { bWritten = false; }

    /// Query / set AutoFmt-flag.
    bool IsAuto() const                 { return bAutoFmt; }
    void SetAuto( bool bNew = false )   { bAutoFmt = bNew; }

    bool IsHidden() const                 { return bHidden; }
    void SetHidden( bool bValue = false ) { bHidden = bValue; }

    void GetGrabBagItem(com::sun::star::uno::Any& rVal) const;
    void SetGrabBagItem(const com::sun::star::uno::Any& rVal);

    /// Query / set bAutoUpdateFmt-flag.
    bool IsAutoUpdateFmt() const                { return bAutoUpdateFmt; }
    void SetAutoUpdateFmt( bool bNew = true )   { bAutoUpdateFmt = bNew; }

    bool IsFmtInDTOR() const { return bFmtInDTOR; }

    /** GetMethods: Bool indicates whether to search only in Set (FALSE)
     or also in Parents.
     If nothing is found the defaulted attribute is returned. */

    /// Character-attributes - implemented in charatr.hxx
    inline const SvxPostureItem      &GetPosture( bool = true ) const;
    inline const SvxWeightItem       &GetWeight( bool = true ) const;
    inline const SvxShadowedItem     &GetShadowed( bool = true ) const;
    inline const SvxAutoKernItem     &GetAutoKern( bool = true ) const;
    inline const SvxWordLineModeItem &GetWordLineMode( bool = true ) const;
    inline const SvxContourItem      &GetContour( bool = true ) const;
    inline const SvxKerningItem      &GetKerning( bool = true ) const;
    inline const SvxUnderlineItem    &GetUnderline( bool = true ) const;
    inline const SvxOverlineItem     &GetOverline( bool = true ) const;
    inline const SvxCrossedOutItem   &GetCrossedOut( bool = true ) const;
    inline const SvxFontHeightItem   &GetSize( bool = true ) const;
    inline const SvxPropSizeItem     &GetPropSize( bool = true ) const;
    inline const SvxFontItem         &GetFont( bool = true ) const;
    inline const SvxColorItem        &GetColor( bool = true ) const;
    inline const SvxCharSetColorItem &GetCharSetColor( bool = true ) const;
    inline const SvxLanguageItem     &GetLanguage( bool = true ) const;
    inline const SvxEscapementItem   &GetEscapement( bool = true ) const;
    inline const SvxCaseMapItem      &GetCaseMap( bool = true ) const;
    inline const SvxNoHyphenItem     &GetNoHyphenHere( bool = true ) const;
    inline const SvxBlinkItem        &GetBlink( bool = true ) const;
    inline const SvxBrushItem        &GetChrBackground( bool = true ) const;
    inline const SvxBrushItem        &GetChrHighlight( bool = true ) const;

    inline const SvxFontItem         &GetCJKFont( bool = true ) const;
    inline const SvxFontHeightItem   &GetCJKSize( bool = true ) const;
    inline const SvxLanguageItem     &GetCJKLanguage( bool = true ) const;
    inline const SvxPostureItem      &GetCJKPosture( bool = true ) const;
    inline const SvxWeightItem       &GetCJKWeight( bool = true ) const;
    inline const SvxFontItem         &GetCTLFont( bool = true ) const;
    inline const SvxFontHeightItem   &GetCTLSize( bool = true ) const;
    inline const SvxLanguageItem     &GetCTLLanguage( bool = true ) const;
    inline const SvxPostureItem      &GetCTLPosture( bool = true ) const;
    inline const SvxWeightItem       &GetCTLWeight( bool = true ) const;
    inline const SfxBoolItem           &GetWritingDirection( bool = true ) const;
    inline const SvxEmphasisMarkItem &GetEmphasisMark( bool = true ) const;
    inline const SvxTwoLinesItem   &Get2Lines( bool = true ) const;
    inline const SvxCharScaleWidthItem &GetCharScaleW( bool = true ) const;
    inline const SvxCharRotateItem     &GetCharRotate( bool = true ) const;
    inline const SvxCharReliefItem     &GetCharRelief( bool = true ) const;
    inline const SvxCharHiddenItem   &GetCharHidden( bool = true ) const;
    inline const SvxBoxItem   &GetCharBorder( bool = true ) const;
    inline const SvxShadowItem   &GetCharShadow( bool = true ) const;

    /// Frame-attributes - implemented in frmatr.hxx.
    inline const SwFmtFillOrder           &GetFillOrder( bool = true ) const;
    inline const SwFmtFrmSize             &GetFrmSize( bool = true ) const;
    inline const SwFmtHeader          &GetHeader( bool = true ) const;
    inline const SwFmtFooter          &GetFooter( bool = true ) const;
    inline const SwFmtSurround            &GetSurround( bool = true ) const;
    inline const SwFmtHoriOrient      &GetHoriOrient( bool = true ) const;
    inline const SwFmtAnchor          &GetAnchor( bool = true ) const;
    inline const SwFmtCol                 &GetCol( bool = true ) const;
    inline const SvxPaperBinItem      &GetPaperBin( bool = true ) const;
    inline const SvxLRSpaceItem           &GetLRSpace( bool = true ) const;
    inline const SvxULSpaceItem           &GetULSpace( bool = true ) const;
    inline const SwFmtCntnt           &GetCntnt( bool = true ) const;
    inline const SvxPrintItem             &GetPrint( bool = true ) const;
    inline const SvxOpaqueItem            &GetOpaque( bool = true ) const;
    inline const SvxProtectItem           &GetProtect( bool = true ) const;
    inline const SwFmtVertOrient      &GetVertOrient( bool = true ) const;
    inline const SvxBoxItem               &GetBox( bool = true ) const;
    inline const SvxFmtKeepItem         &GetKeep( bool = true ) const;

    //UUUU Create SvxBrushItem for Background fill (partially for backwards compatibility)
    SvxBrushItem makeBackgroundBrushItem( bool = true ) const;

    inline const SvxShadowItem            &GetShadow( bool = true ) const;
    inline const SwFmtPageDesc            &GetPageDesc( bool = true ) const;
    inline const SvxFmtBreakItem      &GetBreak( bool = true ) const;
    inline const SvxMacroItem             &GetMacro( bool = true ) const;
    inline const SwFmtURL             &GetURL( bool = true ) const;
    inline const SwFmtEditInReadonly  &GetEditInReadonly( bool = true ) const;
    inline const SwFmtLayoutSplit     &GetLayoutSplit( bool = true ) const;
    inline const SwFmtRowSplit          &GetRowSplit( bool = true ) const;
    inline const SwFmtChain               &GetChain( bool = true ) const;
    inline const SwFmtLineNumber      &GetLineNumber( bool = true ) const;
    inline const SwFmtFtnAtTxtEnd     &GetFtnAtTxtEnd( bool = true ) const;
    inline const SwFmtEndAtTxtEnd     &GetEndAtTxtEnd( bool = true ) const;
    inline const SwFmtNoBalancedColumns &GetBalancedColumns( bool = true ) const;
    inline const SvxFrameDirectionItem    &GetFrmDir( bool = true ) const;
    inline const SwTextGridItem         &GetTextGrid( bool = true ) const;
    inline const SwHeaderAndFooterEatSpacingItem &GetHeaderAndFooterEatSpacing( bool = true ) const;
    // #i18732#
    inline const SwFmtFollowTextFlow    &GetFollowTextFlow(bool = true) const;
    // #i28701#
    inline const SwFmtWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(bool = true) const;
    inline const SdrTextVertAdjustItem& GetTextVertAdjust(bool = true) const;

    /// Graphics-attributes - implemented in grfatr.hxx
    inline const SwMirrorGrf          &GetMirrorGrf( bool = true ) const;
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

    /// Paragraph-attributes - implemented in paratr.hxx.
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

    /// TableBox attributes - implemented in cellatr.hxx.
    inline  const SwTblBoxNumFormat     &GetTblBoxNumFmt( bool = true ) const;
    inline  const SwTblBoxFormula       &GetTblBoxFormula( bool = true ) const;
    inline  const SwTblBoxValue         &GetTblBoxValue( bool = true ) const;

    /** SwFmt::IsBackgroundTransparent

        Virtual method to determine, if background of format is transparent.
        Default implementation returns false. Thus, subclasses have to override
        method, if the specific subclass can have a transparent background.

        @author OD

        @return false, default implementation
    */
    virtual bool IsBackgroundTransparent() const;

    /** SwFmt::IsShadowTransparent

        Virtual method to determine, if shadow of format is transparent.
        Default implementation returns false. Thus, subclasses have to override
        method, if the specific subclass can have a transparent shadow.

        @author OD

        @return false, default implementation
    */
    virtual bool IsShadowTransparent() const;

    //UUUU Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const;
    virtual bool supportsFullDrawingLayerFillAttributeSet() const;
};

inline void SwFmt::SetName( const sal_Char* pNewName,
                             bool bBroadcast )
{
    SetName(OUString::createFromAscii(pNewName), bBroadcast);
}

#endif // INCLUDED_SW_INC_FORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
