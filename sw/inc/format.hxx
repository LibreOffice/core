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
#ifndef _FORMAT_HXX
#define _FORMAT_HXX

#include <tools/solar.h>
#include "swdllapi.h"
#include <swatrset.hxx>     ///< For SfxItemPool/-Set, Attr forward decl.
#include <calbck.hxx>       ///< For SwModify.
#include <hintids.hxx>

class IDocumentSettingAccess;
class IDocumentDrawModelAccess;
class IDocumentLayoutAccess;
class IDocumentTimerAccess;
class IDocumentFieldsAccess;
class IDocumentChartDataProviderAccess;
class SwDoc;

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
    sal_Bool   bWritten : 1;      ///< TRUE: already written.
    bool   bAutoFmt : 1;      /**< FALSE: it is a template.
                                       default is true! */
    sal_Bool   bFmtInDTOR : 1;    /**< TRUE: Format becomes deleted. In order to be able
                                       to recognize this in FmtChg-message!! */
    sal_Bool   bAutoUpdateFmt : 1;/**< TRUE: Set attributes of a whole paragraph
                                       at format (UI-side!). */
    bool bHidden : 1;

protected:
    SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
            const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich );
    SwFmt( SwAttrPool& rPool, const OUString &rFmtNm, const sal_uInt16* pWhichRanges,
            SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich );
    SwFmt( const SwFmt& rFmt );
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue );

public:
    TYPEINFO();     ///< Already in base class Client.

    virtual ~SwFmt();
    SwFmt &operator=(const SwFmt&);

    /// for Querying of Writer-functions.
    sal_uInt16 Which() const { return nWhichId; }


    /// Query format information.
    virtual bool GetInfo( SfxPoolItem& ) const;

    /// Copy attributes even among documents.
    void CopyAttrs( const SwFmt&, sal_Bool bReplace=sal_True );

    /// Delete all attributes that are not in rFmt.
    void DelDiffs( const SfxItemSet& rSet );
    void DelDiffs( const SwFmt& rFmt ) { DelDiffs( rFmt.GetAttrSet() ); }

    /// 0 is Default.
    sal_Bool SetDerivedFrom(SwFmt *pDerivedFrom = 0);

    /// If bInParents is FALSE, search only in this format for attribute.
    inline const SfxPoolItem& GetFmtAttr( sal_uInt16 nWhich,
                                          sal_Bool bInParents = sal_True ) const;
    inline SfxItemState GetItemState( sal_uInt16 nWhich, sal_Bool bSrchInParent = sal_True,
                                    const SfxPoolItem **ppItem = 0 ) const;
    virtual bool SetFmtAttr( const SfxPoolItem& rAttr );
    virtual bool SetFmtAttr( const SfxItemSet& rSet );
    virtual bool ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );

    /** Takes all hints from Delta-Array,
        @return count of deleted hints. */
    virtual sal_uInt16 ResetAllFmtAttr();

    inline SwFmt* DerivedFrom() const { return (SwFmt*)GetRegisteredIn(); }
    inline sal_Bool IsDefault() const { return DerivedFrom() == 0; }

    inline OUString GetName() const   { return aFmtName; }
    void SetName( const OUString& rNewName, sal_Bool bBroadcast=sal_False );
    inline void SetName( const sal_Char* pNewName,
                         sal_Bool bBroadcast=sal_False);

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
    void   ResetWritten()    { bWritten = sal_False; }

    /// Query / set AutoFmt-flag.
    bool IsAuto() const                 { return bAutoFmt; }
    void SetAuto( bool bNew = false )   { bAutoFmt = bNew; }

    bool IsHidden() const                 { return bHidden; }
    void SetHidden( bool bValue = false ) { bHidden = bValue; }

    /// Query / set bAutoUpdateFmt-flag.
    sal_Bool IsAutoUpdateFmt() const                { return bAutoUpdateFmt; }
    void SetAutoUpdateFmt( sal_Bool bNew = sal_True )   { bAutoUpdateFmt = bNew; }

    sal_Bool IsFmtInDTOR() const { return bFmtInDTOR; }

    /** GetMethods: Bool indicates whether to search only in Set (FALSE)
     or also in Parents.
     If nothing is found the defaulted attribute is returned. */

    /// Character-attributes - implemented in charatr.hxx
    inline const SvxPostureItem      &GetPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem       &GetWeight( sal_Bool = sal_True ) const;
    inline const SvxShadowedItem     &GetShadowed( sal_Bool = sal_True ) const;
    inline const SvxAutoKernItem     &GetAutoKern( sal_Bool = sal_True ) const;
    inline const SvxWordLineModeItem &GetWordLineMode( sal_Bool = sal_True ) const;
    inline const SvxContourItem      &GetContour( sal_Bool = sal_True ) const;
    inline const SvxKerningItem      &GetKerning( sal_Bool = sal_True ) const;
    inline const SvxUnderlineItem    &GetUnderline( sal_Bool = sal_True ) const;
    inline const SvxOverlineItem     &GetOverline( sal_Bool = sal_True ) const;
    inline const SvxCrossedOutItem   &GetCrossedOut( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem   &GetSize( sal_Bool = sal_True ) const;
    inline const SvxPropSizeItem     &GetPropSize( sal_Bool = sal_True ) const;
    inline const SvxFontItem         &GetFont( sal_Bool = sal_True ) const;
    inline const SvxColorItem        &GetColor( sal_Bool = sal_True ) const;
    inline const SvxCharSetColorItem &GetCharSetColor( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem     &GetLanguage( sal_Bool = sal_True ) const;
    inline const SvxEscapementItem   &GetEscapement( sal_Bool = sal_True ) const;
    inline const SvxCaseMapItem      &GetCaseMap( sal_Bool = sal_True ) const;
    inline const SvxNoHyphenItem     &GetNoHyphenHere( sal_Bool = sal_True ) const;
    inline const SvxBlinkItem        &GetBlink( sal_Bool = sal_True ) const;
    inline const SvxBrushItem        &GetChrBackground( sal_Bool = sal_True ) const;

    inline const SvxFontItem         &GetCJKFont( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem   &GetCJKSize( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem     &GetCJKLanguage( sal_Bool = sal_True ) const;
    inline const SvxPostureItem      &GetCJKPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem       &GetCJKWeight( sal_Bool = sal_True ) const;
    inline const SvxFontItem         &GetCTLFont( sal_Bool = sal_True ) const;
    inline const SvxFontHeightItem   &GetCTLSize( sal_Bool = sal_True ) const;
    inline const SvxLanguageItem     &GetCTLLanguage( sal_Bool = sal_True ) const;
    inline const SvxPostureItem      &GetCTLPosture( sal_Bool = sal_True ) const;
    inline const SvxWeightItem       &GetCTLWeight( sal_Bool = sal_True ) const;
    inline const SfxBoolItem           &GetWritingDirection( sal_Bool = sal_True ) const;
    inline const SvxEmphasisMarkItem &GetEmphasisMark( sal_Bool = sal_True ) const;
    inline const SvxTwoLinesItem   &Get2Lines( sal_Bool = sal_True ) const;
    inline const SvxCharScaleWidthItem &GetCharScaleW( sal_Bool = sal_True ) const;
    inline const SvxCharRotateItem     &GetCharRotate( sal_Bool = sal_True ) const;
    inline const SvxCharReliefItem     &GetCharRelief( sal_Bool = sal_True ) const;
    inline const SvxCharHiddenItem   &GetCharHidden( sal_Bool = sal_True ) const;
    inline const SvxBoxItem   &GetCharBorder( sal_Bool = sal_True ) const;
    inline const SvxShadowItem   &GetCharShadow( sal_Bool = sal_True ) const;

    /// Frame-attributes - implemented in frmatr.hxx.
    inline const SwFmtFillOrder           &GetFillOrder( sal_Bool = sal_True ) const;
    inline const SwFmtFrmSize             &GetFrmSize( sal_Bool = sal_True ) const;
    inline const SwFmtHeader          &GetHeader( sal_Bool = sal_True ) const;
    inline const SwFmtFooter          &GetFooter( sal_Bool = sal_True ) const;
    inline const SwFmtSurround            &GetSurround( sal_Bool = sal_True ) const;
    inline const SwFmtHoriOrient      &GetHoriOrient( sal_Bool = sal_True ) const;
    inline const SwFmtAnchor          &GetAnchor( sal_Bool = sal_True ) const;
    inline const SwFmtCol                 &GetCol( sal_Bool = sal_True ) const;
    inline const SvxPaperBinItem      &GetPaperBin( sal_Bool = sal_True ) const;
    inline const SvxLRSpaceItem           &GetLRSpace( sal_Bool = sal_True ) const;
    inline const SvxULSpaceItem           &GetULSpace( sal_Bool = sal_True ) const;
    inline const SwFmtCntnt           &GetCntnt( sal_Bool = sal_True ) const;
    inline const SvxPrintItem             &GetPrint( sal_Bool = sal_True ) const;
    inline const SvxOpaqueItem            &GetOpaque( sal_Bool = sal_True ) const;
    inline const SvxProtectItem           &GetProtect( sal_Bool = sal_True ) const;
    inline const SwFmtVertOrient      &GetVertOrient( sal_Bool = sal_True ) const;
    inline const SvxBoxItem               &GetBox( sal_Bool = sal_True ) const;
    inline const SvxFmtKeepItem         &GetKeep( sal_Bool = sal_True ) const;
    inline const SvxBrushItem           &GetBackground( sal_Bool = sal_True ) const;
    inline const SvxShadowItem            &GetShadow( sal_Bool = sal_True ) const;
    inline const SwFmtPageDesc            &GetPageDesc( sal_Bool = sal_True ) const;
    inline const SvxFmtBreakItem      &GetBreak( sal_Bool = sal_True ) const;
    inline const SvxMacroItem             &GetMacro( sal_Bool = sal_True ) const;
    inline const SwFmtURL             &GetURL( sal_Bool = sal_True ) const;
    inline const SwFmtEditInReadonly  &GetEditInReadonly( sal_Bool = sal_True ) const;
    inline const SwFmtLayoutSplit     &GetLayoutSplit( sal_Bool = sal_True ) const;
    inline const SwFmtRowSplit          &GetRowSplit( sal_Bool = sal_True ) const;
    inline const SwFmtChain               &GetChain( sal_Bool = sal_True ) const;
    inline const SwFmtLineNumber      &GetLineNumber( sal_Bool = sal_True ) const;
    inline const SwFmtFtnAtTxtEnd     &GetFtnAtTxtEnd( sal_Bool = sal_True ) const;
    inline const SwFmtEndAtTxtEnd     &GetEndAtTxtEnd( sal_Bool = sal_True ) const;
    inline const SwFmtNoBalancedColumns &GetBalancedColumns( sal_Bool = sal_True ) const;
    inline const SvxFrameDirectionItem    &GetFrmDir( sal_Bool = sal_True ) const;
    inline const SwTextGridItem         &GetTextGrid( sal_Bool = sal_True ) const;
    inline const SwHeaderAndFooterEatSpacingItem &GetHeaderAndFooterEatSpacing( sal_Bool = sal_True ) const;
    // #i18732#
    inline const SwFmtFollowTextFlow    &GetFollowTextFlow(sal_Bool = sal_True) const;
    // #i28701#
    inline const SwFmtWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(sal_Bool = sal_True) const;

    /// Graphics-attributes - implemented in grfatr.hxx
    inline const SwMirrorGrf          &GetMirrorGrf( sal_Bool = sal_True ) const;
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

    /// Paragraph-attributes - implemented in paratr.hxx.
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

    /// TableBox attributes - implemented in cellatr.hxx.
    inline  const SwTblBoxNumFormat     &GetTblBoxNumFmt( sal_Bool = sal_True ) const;
    inline  const SwTblBoxFormula       &GetTblBoxFormula( sal_Bool = sal_True ) const;
    inline  const SwTblBoxValue         &GetTblBoxValue( sal_Bool = sal_True ) const;

    /** SwFmt::IsBackgroundTransparent

        Virtual method to determine, if background of format is transparent.
        Default implementation returns false. Thus, subclasses have to overload
        method, if the specific subclass can have a transparent background.

        @author OD

        @return false, default implementation
    */
    virtual sal_Bool IsBackgroundTransparent() const;

    /** SwFmt::IsShadowTransparent

        Virtual method to determine, if shadow of format is transparent.
        Default implementation returns false. Thus, subclasses have to overload
        method, if the specific subclass can have a transparent shadow.

        @author OD

        @return false, default implementation
    */
    virtual sal_Bool IsShadowTransparent() const;
};

// --------------- inline Implementations ------------------------

inline const SfxPoolItem& SwFmt::GetFmtAttr( sal_uInt16 nWhich,
                                             sal_Bool bInParents ) const
{
    return aSet.Get( nWhich, bInParents );
}

inline void SwFmt::SetName( const sal_Char* pNewName,
                             sal_Bool bBroadcast )
{
    SetName(OUString::createFromAscii(pNewName), bBroadcast);
}

inline SfxItemState SwFmt::GetItemState( sal_uInt16 nWhich, sal_Bool bSrchInParent,
                                        const SfxPoolItem **ppItem ) const
{
    return aSet.GetItemState( nWhich, bSrchInParent, ppItem );
}

#undef inline

#endif // _FORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
