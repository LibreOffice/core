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

#ifndef INCLUDED_SW_INC_VIEWOPT_HXX
#define INCLUDED_SW_INC_VIEWOPT_HXX

#include <config_feature_desktop.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>

#include <svx/swframetypes.hxx>
#include <sfx2/zoomitem.hxx>
#include "swdllapi.h"

class SwRect;
class OutputDevice;
class SwDocShell;
namespace svtools{ class ColorConfig;}
enum class SwFillMode;

struct ViewOptFlags1
{
    bool bUseHeaderFooterMenu : 1;
    bool bTab : 1;
    bool bBlank : 1;
    bool bHardBlank : 1;
    bool bParagraph : 1;
    bool bLinebreak : 1;
    bool bPagebreak : 1;
    bool bColumnbreak : 1;
    bool bSoftHyph : 1;
    bool bBookmarks : 1;
    bool bRef : 1;
    bool bFieldName : 1;
    bool bPostits : 1;
    bool bFieldHidden : 1;
    bool bCharHidden : 1;
    bool bGraphic : 1;
    bool bTable : 1;
    bool bDraw : 1;
    bool bControl : 1;
    bool bCrosshair : 1;
    bool bSnap : 1;
    bool bSynchronize : 1;
    bool bGridVisible : 1;
    bool bOnlineSpell : 1;
    bool bTreatSubOutlineLevelsAsContent : 1;
    bool bShowInlineTooltips : 1; //tooltips on tracked changes
    bool bViewMetachars : 1;
    bool bPageback : 1;
    bool bShowOutlineContentVisibilityButton : 1;
    bool bShowChangesInMargin : 1; //tracked deletions in margin
    bool bShowChangesInMargin2 : 1; //tracked insertions in margin

    ViewOptFlags1()
        : bUseHeaderFooterMenu(false)
        , bTab(false)
        , bBlank(false)
        , bHardBlank(true)
        , bParagraph(false)
        , bLinebreak(false)
        , bPagebreak(false)
        , bColumnbreak(false)
        , bSoftHyph(true)
        , bBookmarks(false)
        , bRef(true)
        , bFieldName(false)
        , bPostits(true)
        , bFieldHidden(false)
        , bCharHidden(false)
        , bGraphic(true)
        , bTable(true)
        , bDraw(true)
        , bControl(true)
        , bCrosshair(false)
        , bSnap(false)
        , bSynchronize(false)
        , bGridVisible(false)
        , bOnlineSpell(false)
        , bTreatSubOutlineLevelsAsContent(false)
        , bShowInlineTooltips(false)
        , bViewMetachars(false)
        , bPageback(true)
        , bShowOutlineContentVisibilityButton(false)
        , bShowChangesInMargin(false)
        , bShowChangesInMargin2(false)
    {}

    bool operator==(const ViewOptFlags1& rOther) const
    {
        return bUseHeaderFooterMenu == rOther.bUseHeaderFooterMenu
            && bTab == rOther.bTab
            && bBlank == rOther.bBlank
            && bHardBlank == rOther.bHardBlank
            && bParagraph == rOther.bParagraph
            && bLinebreak == rOther.bLinebreak
            && bPagebreak == rOther.bPagebreak
            && bColumnbreak == rOther.bColumnbreak
            && bSoftHyph == rOther.bSoftHyph
            && bBookmarks == rOther.bBookmarks
            && bRef == rOther.bRef
            && bFieldName == rOther.bFieldName
            && bPostits == rOther.bPostits
            && bFieldHidden == rOther.bFieldHidden
            && bCharHidden == rOther.bCharHidden
            && bGraphic == rOther.bGraphic
            && bTable == rOther.bTable
            && bDraw == rOther.bDraw
            && bControl == rOther.bControl
            && bCrosshair == rOther.bCrosshair
            && bSnap == rOther.bSnap
            && bSynchronize == rOther.bSynchronize
            && bGridVisible == rOther.bGridVisible
            && bOnlineSpell == rOther.bOnlineSpell
            && bTreatSubOutlineLevelsAsContent == rOther.bTreatSubOutlineLevelsAsContent
            && bShowInlineTooltips == rOther.bShowInlineTooltips
            && bViewMetachars == rOther.bViewMetachars
            && bPageback == rOther.bPageback
            && bShowOutlineContentVisibilityButton == rOther.bShowOutlineContentVisibilityButton
            && bShowChangesInMargin == rOther.bShowChangesInMargin
            && bShowChangesInMargin2 == rOther.bShowChangesInMargin2;
    }

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

enum class ViewOptCoreFlags2 {
    BlackFont         = 0x0001,
    HiddenPara        = 0x0002,
    SmoothScroll      = 0x0004,
    CursorInProt      = 0x0008,
    PdfExport         = 0x0010,
    Printing          = 0x0020,
};
namespace o3tl {
    template<> struct typed_flags<ViewOptCoreFlags2> : is_typed_flags<ViewOptCoreFlags2, 0x003f> {};
};

enum class ViewOptFlags2 {
    HRuler          = 0x00000400,
    VScrollbar      = 0x00000800,
    HScrollbar      = 0x00001000,
    VRuler          = 0x00004000,
    AnyRuler        = 0x00008000,
    Modified        = 0x00010000,
    KeepAspectRatio = 0x00020000,
    GrfKeepZoom     = 0x00040000,
    ContentTips     = 0x00100000,
    ScrollbarTips   = 0x00400000,
    PrintFormat     = 0x00800000,
    ShadowCursor    = 0x01000000,
    VRulerRight     = 0x02000000,
    ResolvedPostits = 0x04000000,
};
namespace o3tl {
    template<> struct typed_flags<ViewOptFlags2> : is_typed_flags<ViewOptFlags2, 0x07d7dc00> {};
};

// Table background.
#define TBL_DEST_CELL   0
#define TBL_DEST_ROW    1
#define TBL_DEST_TBL    2

// Appearance flags.
enum class ViewOptFlags {
    NONE               = 0x0000,
    DocBoundaries      = 0x0001,
    ObjectBoundaries   = 0x0002,
    TableBoundaries    = 0x0004,
    IndexShadings      = 0x0008,
    Links              = 0x0010,
    VisitedLinks       = 0x0020,
    FieldShadings      = 0x0040,
    SectionBoundaries  = 0x0080,
    Shadow             = 0x0100,
};
namespace o3tl {
    template<> struct typed_flags<ViewOptFlags> : is_typed_flags<ViewOptFlags, 0x01ff> {};
}

struct SwViewColors
{
    SwViewColors();
    SwViewColors(const svtools::ColorConfig& rConfig);
    bool operator==(const SwViewColors& rOther) const
    {
        return m_aDocColor == rOther.m_aDocColor
            && m_aDocBoundColor == rOther.m_aDocBoundColor
            && m_aObjectBoundColor == rOther.m_aObjectBoundColor
            && m_aAppBackgroundColor == rOther.m_aAppBackgroundColor
            && m_aTableBoundColor == rOther.m_aTableBoundColor
            && m_aFontColor == rOther.m_aFontColor
            && m_aIndexShadingsColor == rOther.m_aIndexShadingsColor
            && m_aLinksColor == rOther.m_aLinksColor
            && m_aVisitedLinksColor == rOther.m_aVisitedLinksColor
            && m_aTextGridColor == rOther.m_aTextGridColor
            && m_aSpellColor == rOther.m_aSpellColor
            && m_aGrammarColor == rOther.m_aGrammarColor
            && m_aSmarttagColor == rOther.m_aSmarttagColor
            && m_aFieldShadingsColor == rOther.m_aFieldShadingsColor
            && m_aSectionBoundColor == rOther.m_aSectionBoundColor
            && m_aPageBreakColor == rOther.m_aPageBreakColor
            && m_aScriptIndicatorColor == rOther.m_aScriptIndicatorColor
            && m_aShadowColor == rOther.m_aShadowColor
            && m_aHeaderFooterMarkColor == rOther.m_aHeaderFooterMarkColor
            && m_nAppearanceFlags == rOther.m_nAppearanceFlags;
    }
    Color m_aDocColor;  // color of document boundaries
    Color m_aDocBoundColor;  // color of document boundaries
    Color m_aObjectBoundColor; // color of object boundaries
    Color m_aAppBackgroundColor; // application background
    Color m_aTableBoundColor; // color of table boundaries
    Color m_aFontColor;
    Color m_aIndexShadingsColor; // background color of indexes
    Color m_aLinksColor;
    Color m_aVisitedLinksColor;
    Color m_aTextGridColor;
    Color m_aSpellColor;     // mark color of online spell checking
    Color m_aGrammarColor;
    Color m_aSmarttagColor;
    Color m_aFieldShadingsColor;
    Color m_aSectionBoundColor;
    Color m_aPageBreakColor;
    Color m_aScriptIndicatorColor;
    Color m_aShadowColor;
    Color m_aHeaderFooterMarkColor;
    ViewOptFlags m_nAppearanceFlags;
};

class SwViewOption
{
    SwViewColors m_aColorConfig;
    static SwViewColors s_aInitialColorConfig;
    OUString m_sThemeName;

    static sal_uInt16   s_nPixelTwips;// 1 Pixel == ? Twips

    OUString          m_sSymbolFont;                // Symbolfont.
    ViewOptFlags1     m_nCoreOptions;               // Bits for SwViewShell.
    ViewOptCoreFlags2 m_nCore2Options;              // Bits for SwViewShell.
    ViewOptFlags2     m_nUIOptions;                 // UI-Bits
    Color             m_aRetouchColor;              // DefaultBackground for BrowseView
    Size              m_aSnapSize;                  // Describes horizontal and vertical snap.
    sal_uInt16        mnViewLayoutColumns;          // # columns for edit view
    short             m_nDivisionX;                 // Grid division.
    short             m_nDivisionY;
    sal_uInt8         m_nPagePreviewRow;            // Page Preview Row/Columns.
    sal_uInt8         m_nPagePreviewCol;            // Page Preview Row/Columns.
    SwFillMode        m_nShadowCursorFillMode;      // FillMode for ShadowCursor.
    bool              m_bReadonly : 1;              // Readonly-Doc.
    bool              m_bStarOneSetting : 1;        // Prevent from UI automatics (no scrollbars in readonly documents).
    bool              m_bIsPagePreview : 1;         // The preview mustn't print field/footnote/... shadings.
    bool              m_bSelectionInReadonly : 1;   // Determines whether selection is switched on in readonly documents.
    bool              mbFormView : 1;
    bool              mbBrowseMode : 1;
    bool              mbBookView : 1;               // View mode for page preview.
    bool              mbViewLayoutBookMode : 1;     // Book view mode for edit view.
    bool              mbHideWhitespaceMode : 1;     // Hide header, footer, and pagebreak.
    bool              m_bShowPlaceHolderFields : 1; // Only used in printing!
    bool              m_bEncloseWithCharactersOn : 1;
    mutable bool      m_bIdle;
    sal_Int32         m_nDefaultAnchor;             // GetDefaultAnchorType() to convert int to RndStdIds
    // tdf#135266 - tox dialog: remember last used entry level depending on the index type
    sal_uInt8         m_nTocEntryLvl;
    sal_uInt8         m_nIdxEntryLvl;

    // Scale
    sal_uInt16        m_nZoom;                     // In percent.
    SvxZoomType       m_eZoom;                     // 'enum' for zoom.

    sal_uInt8         m_nTableDestination;         // Destination for table background.

#ifdef DBG_UTIL
    // Corresponds to statements in ui/config/cfgvw.src.
    bool  m_bTest1        :1;     // Test-flag  "Layout not loading"
    bool  m_bTest2        :1;     // Test-flag  "WYSIWYG++"
    bool  m_bTest3        :1;     // Test-flag  ""
    bool  m_bTest4        :1;     // Test-flag  "WYSIWYG debug"
    bool  m_bTest5        :1;     // Test-flag  "No idle format"
    bool  m_bTest6        :1;     // Test-flag  "No screen adj"
    bool  m_bTest7        :1;     // Test-flag  "win format"
    bool  m_bTest8        :1;     // Test-flag  ""
    bool  m_bTest10       :1;     // Test-Flag  "Format by Input"
#endif

public:
    SW_DLLPUBLIC SwViewOption();                     // CTOR
    SW_DLLPUBLIC SwViewOption(const SwViewOption&);
    SW_DLLPUBLIC ~SwViewOption();

    static void Init(const OutputDevice* pWin);        // Initializing of static data.

    inline void     SetUIOptions( const SwViewOption& );

    void SetColorConfig(const SwViewColors& rColorConfig)
    {
        m_aColorConfig = rColorConfig;
    }

    const SwViewColors& GetColorConfig() const
    {
        return m_aColorConfig;
    }

    void SetThemeName(const OUString& rThemeName)
    {
        m_sThemeName = rThemeName;
    }

    OUString GetThemeName() const
    {
        return m_sThemeName;
    }

    static void SetInitialColorConfig(const SwViewColors& rColorConfig)
    {
        s_aInitialColorConfig = rColorConfig;
    }

    // Options from nCoreOptions
    bool IsIdle() const
        { return m_bIdle; }

    // Logically this is a const function since it does not modify the viewoptions
    // but only effects idle formatting. Of course that member is already implement
    // in the wrong place here... Also currently there are many const modifying casts in the code
    // just to call this function on otherwise const objects. Thus declaring it as const now.
    void SetIdle( bool b ) const
    { m_bIdle = b; }

    bool IsTab(bool bHard = false) const
    {
        return !m_bReadonly && m_nCoreOptions.bTab && (m_nCoreOptions.bViewMetachars || bHard);
    }
    void SetTab( bool b )
    {
        m_nCoreOptions.bTab = b;
    }

    bool IsBlank(bool bHard = false) const
    {
        return !m_bReadonly && m_nCoreOptions.bBlank && (m_nCoreOptions.bViewMetachars || bHard);
    }
    void SetBlank(bool b)
    {
        m_nCoreOptions.bBlank = b;
    }

    bool IsHardBlank() const
    {
        return !m_bReadonly && (m_nCoreOptions.bHardBlank);
    }
    void SetHardBlank( bool b )
    {
        m_nCoreOptions.bHardBlank = b;
    }

    bool IsParagraph(bool bHard = false) const
    {
        return !m_bReadonly && m_nCoreOptions.bParagraph &&
                            (m_nCoreOptions.bViewMetachars || bHard);
    }
    void SetParagraph( bool b )
    {
        m_nCoreOptions.bParagraph = b;
    }

    void SetShowBookmarks(bool const b)
    {
        m_nCoreOptions.bBookmarks = b;
    }
    bool IsShowBookmarks(bool const bHard = false) const
    {
        return !m_bReadonly && (m_nCoreOptions.bBookmarks)
                && (bHard || IsFieldShadings());
    }

    bool IsLineBreak(bool bHard = false) const
    {
        return !m_bReadonly && (m_nCoreOptions.bLinebreak) &&
                            ((m_nCoreOptions.bViewMetachars) || bHard);
    }
    void SetLineBreak( bool b )
    {
        m_nCoreOptions.bLinebreak = b;
    }

    void SetPageBreak( bool b )
    {
        m_nCoreOptions.bPagebreak = b;
    }

    void SetColumnBreak( bool b)
    {
        m_nCoreOptions.bColumnbreak = b;
    }

    bool IsSoftHyph() const
    { return !m_bReadonly && (m_nCoreOptions.bSoftHyph); }

    void SetSoftHyph( bool b )
    {
        m_nCoreOptions.bSoftHyph = b;
    }

    bool IsFieldName() const
    { return !m_bReadonly && (m_nCoreOptions.bFieldName); }

    void SetFieldName( bool b )
    {
        m_nCoreOptions.bFieldName = b;
    }

    bool IsPostIts() const
    {
        return m_nCoreOptions.bPostits;
    }

    void SetPostIts( bool b )
    {
        m_nCoreOptions.bPostits = b;
    }

    bool IsResolvedPostIts() const
    { return bool(m_nUIOptions & ViewOptFlags2::ResolvedPostits); }

    void SetResolvedPostIts( bool b )
    { SetUIOption(b, ViewOptFlags2::ResolvedPostits); }

    void PaintPostIts( OutputDevice *pOut, const SwRect &rRect,
                              bool bIsScript ) const;
    static sal_uInt16 GetPostItsWidth( const OutputDevice *pOut );

    //show/hide tooltips on tracked changes
    bool IsShowInlineTooltips() const
    {
        return m_nCoreOptions.bShowInlineTooltips;
    }

    void SetShowInlineTooltips( bool b )
    { m_nCoreOptions.bShowInlineTooltips = b; }

    //show/hide tracked deletions in text
    bool IsShowChangesInMargin() const
    {
        return m_nCoreOptions.bShowChangesInMargin;
    }

    void SetShowChangesInMargin( bool b )
    {
        m_nCoreOptions.bShowChangesInMargin = b;
    }

    //show/hide tracked insertions in text
    bool IsShowChangesInMargin2() const
    { return m_nCoreOptions.bShowChangesInMargin2; }

    void SetShowChangesInMargin2( bool b )
    { m_nCoreOptions.bShowChangesInMargin2 = b; }

    //show/hide interactive header/footer on top/bottom of pages
    bool IsUseHeaderFooterMenu() const
    { return m_nCoreOptions.bUseHeaderFooterMenu; }
    void SetUseHeaderFooterMenu( bool b )
    { m_nCoreOptions.bUseHeaderFooterMenu = b; }

    //show/hide outline content visibility button
    SW_DLLPUBLIC bool IsShowOutlineContentVisibilityButton() const;
    void SetShowOutlineContentVisibilityButton(bool b)
    { m_nCoreOptions.bShowOutlineContentVisibilityButton = b; }

    bool IsTreatSubOutlineLevelsAsContent() const;
    void SetTreatSubOutlineLevelsAsContent(bool b)
    { m_nCoreOptions.bTreatSubOutlineLevelsAsContent = b; }

    bool IsShowHiddenChar(bool bHard = false) const
    { return !m_bReadonly && m_nCoreOptions.bCharHidden &&
                            (m_nCoreOptions.bViewMetachars || bHard); }

    void SetShowHiddenChar( bool b )
    { m_nCoreOptions.bCharHidden = b; }

    bool IsShowHiddenField() const
    { return !m_bReadonly && m_nCoreOptions.bFieldHidden; }
    void SetShowHiddenField( bool b )
    { m_nCoreOptions.bFieldHidden = b; }

    bool IsGraphic() const
    { return m_nCoreOptions.bGraphic; }
    void SetGraphic( bool b )
    { m_nCoreOptions.bGraphic = b; }

    bool IsPageBack() const
    { return m_nCoreOptions.bPageback; }
    void SetPageBack( bool b )
    { m_nCoreOptions.bPageback = b; }

    bool IsTable() const
    { return m_nCoreOptions.bTable; }
    void SetTable( bool b )
    { m_nCoreOptions.bTable = b; }

    bool IsDraw() const
    { return m_nCoreOptions.bDraw; }
    void SetDraw( bool b )
    { m_nCoreOptions.bDraw = b; }

    bool IsControl() const
    { return m_nCoreOptions.bControl; }
    void SetControl( bool b )
    { m_nCoreOptions.bControl = b; }

    bool IsSnap() const
    { return m_nCoreOptions.bSnap; }
    void SetSnap( bool b )
    { m_nCoreOptions.bSnap = b; }

    void SetSnapSize( Size const &rSz ){ m_aSnapSize = rSz; }
    const Size &GetSnapSize() const { return m_aSnapSize; }

    bool IsGridVisible() const
    { return !m_bReadonly && m_nCoreOptions.bGridVisible; }
    void SetGridVisible( bool b )
    { m_nCoreOptions.bGridVisible = b; }

    bool IsOnlineSpell() const
    {
        return !m_bReadonly && m_nCoreOptions.bOnlineSpell;
    }
    void SetOnlineSpell( bool b )
    {
        m_nCoreOptions.bOnlineSpell = b;
    }

    bool IsViewMetaChars() const
    { return !m_bReadonly && m_nCoreOptions.bViewMetachars; }
    void SetViewMetaChars( bool b)
    { m_nCoreOptions.bViewMetachars = b; }

    bool IsSynchronize() const
    {  return m_nCoreOptions.bSynchronize; }
    void SetSynchronize( bool b )
    { m_nCoreOptions.bSynchronize = b; }

    bool IsCrossHair() const
    { return m_nCoreOptions.bCrosshair; }
    void SetCrossHair( bool b )
    { m_nCoreOptions.bCrosshair = b; }

    // Options from nCore2Options
    bool IsBlackFont() const
        {return bool(m_nCore2Options & ViewOptCoreFlags2::BlackFont); }

    void SetBlackFont(bool b)
        { SetCore2Option(b, ViewOptCoreFlags2::BlackFont); }

    bool IsShowHiddenPara() const
        {return bool(m_nCore2Options & ViewOptCoreFlags2::HiddenPara); }

    void SetShowHiddenPara(bool b)
        { SetCore2Option(b, ViewOptCoreFlags2::HiddenPara); }

    bool IsSmoothScroll() const
        {return bool(m_nCore2Options & ViewOptCoreFlags2::SmoothScroll); }

    void SetSmoothScroll(bool b)
        { SetCore2Option(b, ViewOptCoreFlags2::SmoothScroll); }

    bool IsCursorInProtectedArea() const
        {return bool(m_nCore2Options & ViewOptCoreFlags2::CursorInProt); }

    void SetCursorInProtectedArea(bool b)
        { SetCore2Option(b, ViewOptCoreFlags2::CursorInProt); }

    static bool IsIgnoreProtectedArea();

    bool IsPDFExport() const
        {return bool(m_nCore2Options & ViewOptCoreFlags2::PdfExport); }

    void SetPDFExport(bool b)
        { SetCore2Option(b, ViewOptCoreFlags2::PdfExport); }

    bool IsPrinting() const
        {return bool(m_nCore2Options & ViewOptCoreFlags2::Printing); }

    void SetPrinting(bool b)
        { SetCore2Option(b, ViewOptCoreFlags2::Printing); }

    void SetCore2Option(bool b, ViewOptCoreFlags2 f)
    {
        if (b)
            m_nCore2Options |= f;
        else
            m_nCore2Options &= ~f;
    }

    short GetDivisionX() const   { return m_nDivisionX; }
    void  SetDivisionX( short n ){ m_nDivisionX = n; }
    short GetDivisionY() const   { return m_nDivisionY; }
    void  SetDivisionY( short n ){ m_nDivisionY = n; }

    // Default margin left and above document: 284 twips == 5.0 mm.
    static constexpr sal_uInt16 defDocumentBorder = 284;
    // Default gap between pages: 284 twips == 5.0 mm.
    static constexpr sal_uInt16 defGapBetweenPages = 284;
    // Minimum edge-to-text distance: 22 twips == 0.4 mm.
    static constexpr sal_uInt16 minGapBetweenPages = 22;

    sal_uInt16 GetDocumentBorder() const { return IsWhitespaceHidden() ? minGapBetweenPages : defDocumentBorder; }
    sal_uInt16 GetGapBetweenPages() const { return IsWhitespaceHidden() ? minGapBetweenPages : defGapBetweenPages; }

    sal_uInt8  GetPagePrevRow() const      { return m_nPagePreviewRow; }
    void  SetPagePrevRow( sal_uInt8 n ) { m_nPagePreviewRow = n; }
    sal_uInt8  GetPagePrevCol() const      { return m_nPagePreviewCol; }
    void  SetPagePrevCol( sal_uInt8 n ) { m_nPagePreviewCol = n; }
    bool         IsReadonly() const { return m_bReadonly; }
    void         SetReadonly(bool bSet) { m_bReadonly = bSet; }

    bool         IsSelectionInReadonly() const {return m_bSelectionInReadonly;}
    void         SetSelectionInReadonly(bool bSet) {m_bSelectionInReadonly = bSet;}

    bool         IsFormView() const { return mbFormView; }
    void         SetFormView( bool bSet ) { mbFormView = bSet; }

    bool  getBrowseMode() const { return mbBrowseMode; }
    void  setBrowseMode(bool bSet) { mbBrowseMode = bSet; }
    bool  IsPagePrevBookview() const { return mbBookView; }
    void  SetPagePrevBookview(bool bSet) { mbBookView = bSet; }

    static bool IsAutoCompleteWords();

    bool   IsViewLayoutBookMode() const { return mbViewLayoutBookMode; }
    void   SetViewLayoutBookMode( bool bNew ) { mbViewLayoutBookMode = bNew; }
    sal_uInt16 GetViewLayoutColumns() const { return mnViewLayoutColumns; }
    void   SetViewLayoutColumns( sal_uInt16 nNew ) { mnViewLayoutColumns = nNew; }
    bool   IsHideWhitespaceMode() const { return mbHideWhitespaceMode; }
    void   SetHideWhitespaceMode( bool bMode ) { mbHideWhitespaceMode = bMode; }

    /// The rules that allow for hiding whitespace.
    bool   CanHideWhitespace() const { return !IsMultipageView(); }
    bool   IsWhitespaceHidden() const { return IsHideWhitespaceMode() && !IsMultipageView(); }
    bool   IsMultipageView() const { return IsViewLayoutBookMode() ||
                                            GetViewLayoutColumns() == 0; }

#ifdef DBG_UTIL
    // Correspond to statements in ui/config/cfgvw.src.
    bool IsTest1() const     { return m_bTest1; }
    bool IsTest2() const     { return m_bTest2; }
    bool IsTest3() const     { return m_bTest3; }
    bool IsTest4() const     { return m_bTest4; }
    bool IsTest5() const     { return m_bTest5; }
    bool IsTest6() const     { return m_bTest6; }
    bool IsTest7() const     { return m_bTest7; }
    bool IsTest8() const     { return m_bTest8; }
    bool IsTest10() const    { return m_bTest10; }
#endif

    sal_uInt16 GetZoom() const    { return m_nZoom; }
    void   SetZoom( sal_uInt16 n ){ m_nZoom = n; }

    static void DrawRect( OutputDevice* pOut, const SwRect &rRect, ::Color nCol );
    static void DrawRectPrinter( OutputDevice* pOut, const SwRect& rRect );

    SwViewOption& operator=( const SwViewOption &rOpt );

    // Compare methods.
    bool IsEqualFlags ( const SwViewOption &rOpt ) const;
    inline bool operator == ( const SwViewOption &rOpt ) const;
    bool operator != ( const SwViewOption &rOpt ) const  { return !(*this == rOpt); }

    // Options from nUIOptions
    bool    IsViewVScrollBar() const
        {
#if HAVE_FEATURE_DESKTOP
            return bool(m_nUIOptions & ViewOptFlags2::VScrollbar);
#else
            return false;
#endif
        }
    bool    IsViewHScrollBar() const
        {
#if HAVE_FEATURE_DESKTOP
            return bool(m_nUIOptions & ViewOptFlags2::HScrollbar);
#else
            return false;
#endif
        }
    bool    IsKeepRatio()      const
        { return bool(m_nUIOptions & ViewOptFlags2::KeepAspectRatio); }
    bool    IsGrfKeepZoom()    const
        { return bool(m_nUIOptions & ViewOptFlags2::GrfKeepZoom); }
    bool    IsShowContentTips() const
        { return bool(m_nUIOptions & ViewOptFlags2::ContentTips); }
    bool    IsPrtFormat() const
        { return bool(m_nUIOptions & ViewOptFlags2::PrintFormat); }
    bool    IsShowScrollBarTips() const
        { return bool(m_nUIOptions & ViewOptFlags2::ScrollbarTips); }

    SvxZoomType    GetZoomType()      const { return m_eZoom; }

    sal_uInt8   GetTableDest() const    { return m_nTableDestination; }

    void   SetViewVScrollBar(bool b)
        { SetUIOption(b, ViewOptFlags2::VScrollbar); }
    void   SetViewHScrollBar(bool b)
        { SetUIOption(b, ViewOptFlags2::HScrollbar); }
    void   SetKeepRatio     (bool b)
        { SetUIOption(b, ViewOptFlags2::KeepAspectRatio); }
    void   SetGrfKeepZoom   (bool b)
        { SetUIOption(b, ViewOptFlags2::GrfKeepZoom); }
    void SetShowContentTips( bool b)
        { SetUIOption(b, ViewOptFlags2::ContentTips); }
    void SetPrtFormat( bool b)
        { SetUIOption(b, ViewOptFlags2::PrintFormat); }
    void SetShowScrollBarTips( bool b)
        { SetUIOption(b, ViewOptFlags2::ScrollbarTips); }
    void SetUIOption( bool b, ViewOptFlags2 f)
    {
        if (b)
            m_nUIOptions |= f;
        else
            m_nUIOptions &= ~f;
    }

    void            SetZoomType     (SvxZoomType eZoom_){ m_eZoom = eZoom_;  }
    void            SetTableDest( sal_uInt8 nNew )    { m_nTableDestination = nNew;  }

    const OUString& GetSymbolFont() const {return m_sSymbolFont;}
    void            SetSymbolFont(const OUString& sSet) {m_sSymbolFont = sSet;}

    const Color&    GetRetoucheColor() const        { return m_aRetouchColor;}
    void            SetRetoucheColor(const Color&r) { m_aRetouchColor = r;   }

    bool        IsViewAnyRuler() const
        {
#if HAVE_FEATURE_DESKTOP
            return bool(m_nUIOptions & ViewOptFlags2::AnyRuler);
#else
            return false;
#endif
        }
    void            SetViewAnyRuler(bool bSet)
                        { SetUIOption(bSet, ViewOptFlags2::AnyRuler);}

    bool        IsViewHRuler(bool bDirect = false)     const
                        {
#if HAVE_FEATURE_DESKTOP
                            return bDirect
                                   ? bool(m_nUIOptions & ViewOptFlags2::HRuler)
                                   : !m_bReadonly && (m_nUIOptions & (ViewOptFlags2::AnyRuler|ViewOptFlags2::HRuler)) == (ViewOptFlags2::AnyRuler|ViewOptFlags2::HRuler);
#else
                            (void) bDirect;
                            return false;
#endif
                        }
    void            SetViewHRuler   (bool b)
                        { SetUIOption(b, ViewOptFlags2::HRuler ); }

    bool            IsViewVRuler(bool bDirect = false) const
                        {
#if HAVE_FEATURE_DESKTOP
                            return bDirect
                                   ? bool(m_nUIOptions & ViewOptFlags2::VRuler)
                                   : !m_bReadonly && (m_nUIOptions & (ViewOptFlags2::AnyRuler|ViewOptFlags2::VRuler)) == (ViewOptFlags2::AnyRuler|ViewOptFlags2::VRuler);
#else
                            (void) bDirect;
                            return false;
#endif
                        }
    void            SetViewVRuler     (bool b)
                        { SetUIOption(b, ViewOptFlags2::VRuler); }

    // ShadowCursor, switch on/off, get/set color/mode.
    bool    IsShadowCursor()    const
        { return bool(m_nUIOptions & ViewOptFlags2::ShadowCursor); }
    void   SetShadowCursor(bool b)
        { SetUIOption(b, ViewOptFlags2::ShadowCursor); }

    // Enclose with characters autocomplete, switch on/off
    bool IsEncloseWithCharactersOn() const { return m_bEncloseWithCharactersOn; }
    void SetEncloseWithCharactersOn(bool b) { m_bEncloseWithCharactersOn = b; }

    static bool IsEncloseWithCharactersTrigger(sal_Unicode cChar)
    {
        switch (cChar)
        {
            case '(':  [[fallthrough]];
            case '{':  [[fallthrough]];
            case '[':  [[fallthrough]];
            case '\'': [[fallthrough]];
            case '\"':
                return true;
            default:
                return false;
        }
    }

    //move vertical ruler to the right
    bool    IsVRulerRight()    const
        { return bool(m_nUIOptions & ViewOptFlags2::VRulerRight); }
    void   SetVRulerRight(bool b)
        { SetUIOption(b, ViewOptFlags2::VRulerRight); }

    bool            IsStarOneSetting() const {return m_bStarOneSetting; }
    void            SetStarOneSetting(bool bSet) {m_bStarOneSetting = bSet; }

    bool            IsPagePreview() const {return m_bIsPagePreview; }
    void            SetPagePreview(bool bSet) { m_bIsPagePreview= bSet; }

    SwFillMode      GetShdwCursorFillMode() const { return m_nShadowCursorFillMode; }
    void            SetShdwCursorFillMode( SwFillMode nMode ) { m_nShadowCursorFillMode = nMode; };

    bool        IsShowPlaceHolderFields() const { return m_bShowPlaceHolderFields; }
    void            SetShowPlaceHolderFields(bool bSet) { m_bShowPlaceHolderFields = bSet; }

    SW_DLLPUBLIC const Color& GetDocColor() const;
    SW_DLLPUBLIC const Color& GetDocBoundariesColor() const;
    const Color& GetAppBackgroundColor() const;
    const Color& GetObjectBoundariesColor() const;
    const Color& GetTableBoundariesColor() const;
    const Color& GetIndexShadingsColor() const;
    const Color& GetLinksColor() const;
    const Color& GetVisitedLinksColor() const;
    const Color& GetTextGridColor() const;
    const Color& GetSpellColor() const;
    const Color& GetGrammarColor() const;
    const Color& GetSmarttagColor() const;
    const Color& GetShadowColor() const;
    SW_DLLPUBLIC const Color& GetFontColor() const;
    const Color& GetFieldShadingsColor() const;
    const Color& GetSectionBoundColor() const;
    const Color& GetPageBreakColor() const;
    const Color& GetHeaderFooterMarkColor() const;

    bool IsAppearanceFlag(ViewOptFlags nFlag) const;

    bool IsDocBoundaries() const {return IsAppearanceFlag(ViewOptFlags::DocBoundaries);}
    bool IsObjectBoundaries() const {return IsAppearanceFlag(ViewOptFlags::ObjectBoundaries);}
    bool IsTableBoundaries() const {return IsAppearanceFlag(ViewOptFlags::TableBoundaries);}
    bool IsIndexShadings() const {return IsAppearanceFlag(ViewOptFlags::IndexShadings);}
    bool IsLinks() const {return IsAppearanceFlag(ViewOptFlags::Links);}
    bool IsVisitedLinks() const {return IsAppearanceFlag(ViewOptFlags::VisitedLinks);}
    bool IsFieldShadings() const {return IsAppearanceFlag(ViewOptFlags::FieldShadings);}
    bool IsSectionBoundaries() const {return IsAppearanceFlag(ViewOptFlags::SectionBoundaries);}
    bool IsShadow() const {return IsAppearanceFlag(ViewOptFlags::Shadow);}

    void     SetAppearanceFlag(ViewOptFlags nFlag, bool bSet, bool bSaveInConfig = false);

    void     SetDocBoundaries(bool bSet)   {SetAppearanceFlag(ViewOptFlags::DocBoundaries, bSet);}

    // get/set default anchor (0..2); use GetDefaultAnchorType() to convert into RndStdIds::FLY_*
    sal_Int32 GetDefaultAnchor() const
        {   return m_nDefaultAnchor; }
    void SetDefaultAnchor( const sal_Int32 aFlag )
        { m_nDefaultAnchor = aFlag; }

    RndStdIds GetDefaultAnchorType() const;

    // tdf#135266 - tox dialog: remember last used entry level depending on the index type
    sal_uInt8 GetTocEntryLvl() const { return m_nTocEntryLvl; }
    void SetTocEntryLvl(sal_uInt8 n) { m_nTocEntryLvl = n; }
    sal_uInt8 GetIdxEntryLvl() const { return m_nIdxEntryLvl; }
    void SetIdxEntryLvl(sal_uInt8 n) { m_nIdxEntryLvl = n; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    // Useful for when getting the current view SwViewOption is not possible otherwise
    SW_DLLPUBLIC static const SwViewOption& GetCurrentViewOptions();

    void SyncLayoutRelatedViewOptions(const SwViewOption& rOpt);
};

inline bool SwViewOption::operator==( const SwViewOption &rOpt ) const
{
    return IsEqualFlags( rOpt ) && m_nZoom == rOpt.GetZoom() && m_aColorConfig == rOpt.m_aColorConfig;
}

inline void SwViewOption::SetUIOptions( const SwViewOption& rVOpt )
{
    m_nUIOptions = rVOpt.m_nUIOptions;
    m_nTableDestination = rVOpt.m_nTableDestination;
    m_nShadowCursorFillMode = rVOpt.m_nShadowCursorFillMode;
    m_bEncloseWithCharactersOn = rVOpt.m_bEncloseWithCharactersOn;
}

// Helper function for checking HTML-capabilities.
SW_DLLPUBLIC sal_uInt16     GetHtmlMode(const SwDocShell*);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
