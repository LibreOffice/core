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

#include "swdllapi.h"

#include "BorderCacheOwner.hxx"
#include "calbck.hxx"
#include "hintids.hxx"
#include "swatrset.hxx"
#include <memory>

class IDocumentSettingAccess;
class IDocumentDrawModelAccess;
class IDocumentLayoutAccess;
class IDocumentTimerAccess;
class IDocumentFieldsAccess;
class IDocumentChartDataProviderAccess;
class SwDoc;
class SfxGrabBagItem;
class SwTextGridItem;

namespace drawinglayer::attribute {
    class SdrAllFillAttributesHelper;
    typedef std::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
}

/// Base class for various Writer styles.
class SW_DLLPUBLIC SwFormat : public sw::BorderCacheOwner, public sw::BroadcastingModify
{
    friend class SwFrameFormat;

    OUString m_aFormatName;
    SwAttrSet m_aSet;

    sal_uInt16 m_nWhichId;
    sal_uInt16 m_nPoolFormatId;        /**< Id for "automatically" created formats.
                                       (is not hard attribution!!!) */
    sal_uInt16 m_nPoolHelpId;       ///< HelpId for this Pool-style.
    sal_uInt8 m_nPoolHlpFileId;     ///< FilePos to Doc to these style helps.
    bool   m_bAutoFormat : 1;      /**< FALSE: it is a template.
                                       default is true! */
    bool   m_bFormatInDTOR : 1;    /**< TRUE: Format becomes deleted. In order to be able
                                       to recognize this in FormatChg-message!! */
    bool   m_bAutoUpdateFormat : 1;/**< TRUE: Set attributes of a whole paragraph
                                       at format (UI-side!). */
    bool m_bHidden : 1;
    std::shared_ptr<SfxGrabBagItem> m_pGrabBagItem; ///< Style InteropGrabBag.
    virtual void InvalidateInSwFntCache(sal_uInt16) {};

protected:
    SwFormat( SwAttrPool& rPool, const char* pFormatNm,
            const WhichRangesContainer& pWhichRanges, SwFormat *pDrvdFrame, sal_uInt16 nFormatWhich );
    SwFormat( SwAttrPool& rPool, const OUString &rFormatNm, const WhichRangesContainer& pWhichRanges,
            SwFormat *pDrvdFrame, sal_uInt16 nFormatWhich );
    SwFormat( const SwFormat& rFormat );
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:

    virtual ~SwFormat() override;
    SwFormat &operator=(const SwFormat&);

    /// for Querying of Writer-functions.
    sal_uInt16 Which() const { return m_nWhichId; };

    /// Copy attributes even among documents.
    void CopyAttrs( const SwFormat& );

    /// Delete all attributes that are not in rFormat.
    void DelDiffs( const SfxItemSet& rSet );
    void DelDiffs( const SwFormat& rFormat ) { DelDiffs( rFormat.GetAttrSet() ); }

    /// 0 is Default.
    bool SetDerivedFrom(SwFormat *pDerivedFrom = nullptr);

    /// If bInParents is FALSE, search only in this format for attribute.
    const SfxPoolItem& GetFormatAttr( sal_uInt16 nWhich,
                                   bool bInParents = true ) const;
    template<class T> const T& GetFormatAttr( TypedWhichId<T> nWhich, bool bInParents = true ) const
    {
        return static_cast<const T&>(GetFormatAttr(sal_uInt16(nWhich), bInParents));
    }
    SfxItemState GetItemState( sal_uInt16 nWhich, bool bSrchInParent = true,
                                    const SfxPoolItem **ppItem = nullptr ) const;
    SfxItemState GetBackgroundState(std::unique_ptr<SvxBrushItem>& rItem) const;
    virtual bool SetFormatAttr( const SfxPoolItem& rAttr );
    virtual bool SetFormatAttr( const SfxItemSet& rSet );
    virtual bool ResetFormatAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );

    /** Takes all hints from Delta-Array,
        @return count of deleted hints. */
    virtual sal_uInt16 ResetAllFormatAttr();

    SwFormat* DerivedFrom() const { return const_cast<SwFormat*>(static_cast<const SwFormat*>(GetRegisteredIn())); }
    bool IsDefault() const { return DerivedFrom() == nullptr; }

    const OUString& GetName() const                  { return m_aFormatName; }
    bool HasName(std::u16string_view rName) const { return m_aFormatName == rName; }
    virtual void SetName( const OUString& rNewName, bool bBroadcast=false );

    /// For querying the attribute array.
    const SwAttrSet& GetAttrSet() const { return m_aSet; }

    /** The document is set in SwAttrPool now, therefore you always can access it. */
    const SwDoc *GetDoc() const         { return m_aSet.GetDoc(); }
          SwDoc *GetDoc()               { return m_aSet.GetDoc(); }

    /// Provides access to the document settings interface.
    const IDocumentSettingAccess& getIDocumentSettingAccess() const;

    /// Provides access to the document draw model interface.
    const IDocumentDrawModelAccess& getIDocumentDrawModelAccess() const;
          IDocumentDrawModelAccess& getIDocumentDrawModelAccess();

    /// Provides access to the document layout interface.
    const IDocumentLayoutAccess& getIDocumentLayoutAccess() const;
          IDocumentLayoutAccess& getIDocumentLayoutAccess();

     /// Provides access to the document idle timer interface.
     IDocumentTimerAccess& getIDocumentTimerAccess();

     /// Provides access to the document idle timer interface.
    IDocumentFieldsAccess& getIDocumentFieldsAccess();

     /// Gives access to the chart data-provider.
    IDocumentChartDataProviderAccess& getIDocumentChartDataProviderAccess();

    /// Get and set Pool style IDs.
    sal_uInt16 GetPoolFormatId() const { return m_nPoolFormatId; }
    void SetPoolFormatId( sal_uInt16 nId ) { m_nPoolFormatId = nId; }

    /// Get and set Help-IDs for document templates.
    sal_uInt16 GetPoolHelpId() const { return m_nPoolHelpId; }
    void SetPoolHelpId( sal_uInt16 nId ) { m_nPoolHelpId = nId; }
    sal_uInt8 GetPoolHlpFileId() const { return m_nPoolHlpFileId; }
    void SetPoolHlpFileId( sal_uInt8 nId ) { m_nPoolHlpFileId = nId; }

    /// Get attribute-description. Returns passed string.
    void GetPresentation( SfxItemPresentation ePres,
        MapUnit eCoreMetric, MapUnit ePresMetric, OUString &rText ) const
        { m_aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, rText ); }

    /// Query / set AutoFormat-flag.
    bool IsAuto() const                 { return m_bAutoFormat; }
    void SetAuto( bool bNew )           { m_bAutoFormat = bNew; }

    bool IsHidden() const               { return m_bHidden; }
    void SetHidden( bool bValue )       { m_bHidden = bValue; }

    void GetGrabBagItem(css::uno::Any& rVal) const;
    void SetGrabBagItem(const css::uno::Any& rVal);

    /// Query / set bAutoUpdateFormat-flag.
    bool IsAutoUpdateFormat() const                { return m_bAutoUpdateFormat; }
    void SetAutoUpdateFormat( bool bNew = true )   { m_bAutoUpdateFormat = bNew; }

    bool IsFormatInDTOR() const { return m_bFormatInDTOR; }

    /** GetMethods: Bool indicates whether to search only in Set (FALSE)
     or also in Parents.
     If nothing is found the defaulted attribute is returned. */

    /// Character-attributes - implemented in charatr.hxx
    inline const SvxUnderlineItem    &GetUnderline( bool = true ) const;
    inline const SvxFontHeightItem   &GetSize( bool = true ) const;
    inline const SvxFontItem         &GetFont( bool = true ) const;
    inline const SvxColorItem        &GetColor( bool = true ) const;
    inline const SvxFontItem         &GetCJKFont( bool = true ) const;
    inline const SvxFontItem         &GetCTLFont( bool = true ) const;

    /// Frame-attributes - implemented in frmatr.hxx.
    inline const SwFormatFillOrder           &GetFillOrder( bool = true ) const;
    inline const SwFormatFrameSize             &GetFrameSize( bool = true ) const;
    inline const SwFormatHeader          &GetHeader( bool = true ) const;
    inline const SwFormatFooter          &GetFooter( bool = true ) const;
    inline const SwFormatSurround            &GetSurround( bool = true ) const;
    inline const SwFormatHoriOrient      &GetHoriOrient( bool = true ) const;
    inline const SwFormatAnchor          &GetAnchor( bool = true ) const;
    inline const SwFormatCol                 &GetCol( bool = true ) const;
    inline const SvxPaperBinItem      &GetPaperBin( bool = true ) const;
    inline const SvxLRSpaceItem           &GetLRSpace( bool = true ) const;
    inline const SvxULSpaceItem           &GetULSpace( bool = true ) const;
    inline const SwFormatContent           &GetContent( bool = true ) const;
    inline const SvxPrintItem             &GetPrint( bool = true ) const;
    inline const SvxOpaqueItem            &GetOpaque( bool = true ) const;
    inline const SvxProtectItem           &GetProtect( bool = true ) const;
    inline const SwFormatVertOrient      &GetVertOrient( bool = true ) const;
    inline const SvxBoxItem               &GetBox( bool = true ) const;
    inline const SvxFormatKeepItem         &GetKeep( bool = true ) const;

    // Create SvxBrushItem for Background fill (partially for backwards compatibility)
    std::unique_ptr<SvxBrushItem> makeBackgroundBrushItem( bool = true ) const;

    inline const SvxShadowItem            &GetShadow( bool = true ) const;
    inline const SwFormatPageDesc            &GetPageDesc( bool = true ) const;
    inline const SvxFormatBreakItem      &GetBreak( bool = true ) const;
    inline const SvxMacroItem             &GetMacro( bool = true ) const;
    inline const SwFormatURL             &GetURL( bool = true ) const;
    inline const SwFormatEditInReadonly  &GetEditInReadonly( bool = true ) const;
    inline const SwFormatLayoutSplit     &GetLayoutSplit( bool = true ) const;
    inline const SwFormatRowSplit          &GetRowSplit( bool = true ) const;
    inline const SwFormatChain               &GetChain( bool = true ) const;
    inline const SwFormatFootnoteAtTextEnd     &GetFootnoteAtTextEnd( bool = true ) const;
    inline const SwFormatEndAtTextEnd     &GetEndAtTextEnd( bool = true ) const;
    inline const SwFormatNoBalancedColumns &GetBalancedColumns( bool = true ) const;
    inline const SvxFrameDirectionItem    &GetFrameDir( bool = true ) const;
    inline const SwTextGridItem         &GetTextGrid( bool = true ) const;
    inline const SwHeaderAndFooterEatSpacingItem &GetHeaderAndFooterEatSpacing( bool = true ) const;
    // #i18732#
    inline const SwFormatFollowTextFlow    &GetFollowTextFlow(bool = true) const;
    // #i28701#
    inline const SwFormatWrapInfluenceOnObjPos& GetWrapInfluenceOnObjPos(bool = true) const;
    inline const SdrTextVertAdjustItem& GetTextVertAdjust(bool = true) const;

    /// Paragraph-attributes - implemented in paratr.hxx.
    inline const SvxLineSpacingItem       &GetLineSpacing( bool = true ) const;
    inline const SwNumRuleItem            &GetNumRule( bool = true ) const;
    inline const SvxTabStopItem           &GetTabStops( bool = true ) const;
    inline const SwFormatDrop                &GetDrop( bool = true ) const;

    /// TableBox attributes - implemented in cellatr.hxx.
    inline  const SwTableBoxNumFormat     &GetTableBoxNumFormat( bool = true ) const;
    inline  const SwTableBoxFormula       &GetTableBoxFormula( bool = true ) const;
    inline  const SwTableBoxValue         &GetTableBoxValue( bool = true ) const;

    inline  const SwFormatLayoutInCell         &GetLayoutInCell( bool = true ) const;

    void SetPageFormatToDefault();

    /** SwFormat::IsBackgroundTransparent

        Virtual method to determine, if background of format is transparent.
        Default implementation returns false. Thus, subclasses have to override
        method, if the specific subclass can have a transparent background.

        @return false, default implementation
    */
    virtual bool IsBackgroundTransparent() const;

    // Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const;
    virtual bool supportsFullDrawingLayerFillAttributeSet() const;
    void RemoveAllUnos();
};

#endif // INCLUDED_SW_INC_FORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
