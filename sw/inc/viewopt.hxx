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

#include <config_features.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>

#include <sfx2/zoomitem.hxx>
#include "swdllapi.h"

class SwRect;
namespace vcl { class Window; }
class OutputDevice;
class SwDocShell;
namespace svtools{ class ColorConfig;}

enum class ViewOptFlags1 {
    UseHeaderFooterMenu = 0x00000001,
    Tab           = 0x00000002,
    Blank         = 0x00000004,
    HardBlank     = 0x00000008,
    Paragraph     = 0x00000010,
    Linebreak     = 0x00000020,
    Pagebreak     = 0x00000040,
    Columnbreak   = 0x00000080,
    SoftHyph      = 0x00000100,
    Ref           = 0x00000400,
    FieldName     = 0x00000800,
    Postits       = 0x00004000,
    FieldHidden   = 0x00008000,
    CharHidden    = 0x00010000,
    Graphic       = 0x00020000,
    Table         = 0x00040000,
    Draw          = 0x00080000,
    Control       = 0x00100000,
    Crosshair     = 0x00400000,
    Snap          = 0x00800000,
    Synchronize   = 0x01000000,
    GridVisible   = 0x02000000,
    OnlineSpell   = 0x04000000,
    ShowInlineTooltips = 0x10000000, //tooltips on tracked changes
    ViewMetachars = 0x20000000,
    Pageback      = 0x40000000
};
namespace o3tl {
    template<> struct typed_flags<ViewOptFlags1> : is_typed_flags<ViewOptFlags1, 0x77dfcdff> {};
}

enum class ViewOptCoreFlags2 {
    BlackFont         = 0x0001,
    HiddenPara        = 0x0002,
    SmoothScroll      = 0x0004,
    CursorInProt      = 0x0008,
    PdfExport         = 0x0010,
    Printing          = 0x0020,
};
namespace o3tl {
    template<> struct typed_flags<ViewOptCoreFlags2> : is_typed_flags<ViewOptCoreFlags2, 0x007f> {};
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
    VRulerRight     = 0x02000000
};
namespace o3tl {
    template<> struct typed_flags<ViewOptFlags2> : is_typed_flags<ViewOptFlags2, 0x03d7dc00> {};
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

class SW_DLLPUBLIC SwViewOption
{
    static Color    s_aDocColor;  // color of document boundaries
    static Color    s_aDocBoundColor;  // color of document boundaries
    static Color    s_aObjectBoundColor; // color of object boundaries
    static Color    s_aAppBackgroundColor; // application background
    static Color    s_aTableBoundColor; // color of table boundaries
    static Color    s_aFontColor;
    static Color    s_aIndexShadingsColor; // background color of indexes
    static Color    s_aLinksColor;
    static Color    s_aVisitedLinksColor;
    static Color    s_aDirectCursorColor;
    static Color    s_aTextGridColor;
    static Color    s_aSpellColor;     // mark color of online spell checking
    static Color    s_aSmarttagColor;
    static Color    s_aFieldShadingsColor;
    static Color    s_aSectionBoundColor;
    static Color    s_aPageBreakColor;
    static Color    s_aScriptIndicatorColor;
    static Color    s_aShadowColor;
    static Color    s_aHeaderFooterMarkColor;

    static ViewOptFlags s_nAppearanceFlags;
    static sal_uInt16   s_nPixelTwips;// 1 Pixel == ? Twips

    OUString        m_sSymbolFont;        // Symbolfont.
    ViewOptFlags1   m_nCoreOptions;       // Bits for SwViewShell.
    ViewOptCoreFlags2 m_nCore2Options;    // Bits for SwViewShell.
    ViewOptFlags2   m_nUIOptions;         // UI-Bits
    Color           m_aRetouchColor;      // DefaultBackground for BrowseView
    Size            m_aSnapSize;          // Describes horizontal and vertical snap.
    sal_uInt16      mnViewLayoutColumns;  // # columns for edit view
    short           m_nDivisionX;         // Grid division.
    short           m_nDivisionY;
    sal_uInt8       m_nPagePreviewRow;       // Page Preview Row/Columns.
    sal_uInt8       m_nPagePreviewCol;       // Page Preview Row/Columns.
    sal_uInt8       m_nShadowCursorFillMode;  // FillMode for ShadowCursor.
    bool            m_bReadonly : 1;      // Readonly-Doc.
    bool            m_bStarOneSetting : 1;// Prevent from UI automatics (no scrollbars in readonly documents).
    bool            m_bIsPagePreview : 1; // The preview mustn't print field/footnote/... shadings.
    bool            m_bSelectionInReadonly : 1; // Determines whether selection is switched on in readonly documents.
    bool            mbFormView : 1;
    bool            mbBrowseMode : 1;
    bool            mbBookView : 1;      // View mode for page preview.
    bool            mbViewLayoutBookMode : 1; // Book view mode for edit view.
    bool            mbHideWhitespaceMode : 1; // Hide header, footer, and pagebreak.
    bool            m_bShowPlaceHolderFields : 1; // Only used in printing!
    mutable bool    m_bIdle;

    // Scale
    sal_uInt16          m_nZoom;          // In percent.
    SvxZoomType     m_eZoom;              // 'enum' for zoom.

    sal_uInt8            m_nTableDestination;      // Destination for table background.

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
            SwViewOption();                     // CTOR
            SwViewOption(const SwViewOption&);
            ~SwViewOption();

    static void Init( vcl::Window const *pWin );        // Initializing of static data.

    ViewOptFlags1   GetCoreOptions() const {return m_nCoreOptions;}
    inline void     SetUIOptions( const SwViewOption& );

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
                    {   return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::Tab) &&
                            ((m_nCoreOptions & ViewOptFlags1::ViewMetachars)||bHard); }
    void SetTab( bool b )
        { SetCoreOption(b, ViewOptFlags1::Tab); }

    bool IsBlank(bool bHard = false) const
                    { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::Blank) &&
                            ((m_nCoreOptions & ViewOptFlags1::ViewMetachars)||bHard); }
    void SetBlank( bool b )
        { SetCoreOption(b, ViewOptFlags1::Blank); }

    bool IsHardBlank() const
                    { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::HardBlank); }
    void SetHardBlank( bool b )
        { SetCoreOption(b, ViewOptFlags1::HardBlank); }

    bool IsParagraph(bool bHard = false) const
                    {   return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::Paragraph) &&
                            ((m_nCoreOptions & ViewOptFlags1::ViewMetachars)||bHard); }
    void SetParagraph( bool b )
        { SetCoreOption(b, ViewOptFlags1::Paragraph); }

    bool IsLineBreak(bool bHard = false) const
                    {   return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::Linebreak) &&
                            ((m_nCoreOptions & ViewOptFlags1::ViewMetachars)||bHard); }
    void SetLineBreak( bool b )
        { SetCoreOption(b, ViewOptFlags1::Linebreak); }

    void SetPageBreak( bool b )
        { SetCoreOption(b, ViewOptFlags1::Pagebreak); }

    void SetColumnBreak( bool b)
        { SetCoreOption(b, ViewOptFlags1::Columnbreak); }

    bool IsSoftHyph() const
                    { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::SoftHyph); }
    void SetSoftHyph( bool b )
        { SetCoreOption(b, ViewOptFlags1::SoftHyph); }

    bool IsFieldName() const       { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::FieldName); }
    void SetFieldName( bool b )
        { SetCoreOption(b, ViewOptFlags1::FieldName); }

    bool IsPostIts() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Postits); }
    void SetPostIts( bool b )
        { SetCoreOption(b, ViewOptFlags1::Postits); }
    static void PaintPostIts( OutputDevice *pOut, const SwRect &rRect,
                              bool bIsScript );
    static sal_uInt16 GetPostItsWidth( const OutputDevice *pOut );

    //show/hide tooltips on tracked changes
    bool IsShowInlineTooltips() const
        { return bool(m_nCoreOptions & ViewOptFlags1::ShowInlineTooltips); }
    void SetShowInlineTooltips( bool b )
        { SetCoreOption(b, ViewOptFlags1::ShowInlineTooltips); }

    //show/hide interactive header/footer on top/bottom of pages
    bool IsUseHeaderFooterMenu() const
        { return bool(m_nCoreOptions & ViewOptFlags1::UseHeaderFooterMenu ); }
    void SetUseHeaderFooterMenu( bool b )
        { SetCoreOption(b, ViewOptFlags1::UseHeaderFooterMenu); }

    bool IsShowHiddenChar(bool bHard = false) const
        { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::CharHidden) &&
                            ((m_nCoreOptions & ViewOptFlags1::ViewMetachars)||bHard); }

    void SetShowHiddenChar( bool b )
        { SetCoreOption(b, ViewOptFlags1::CharHidden); }

    bool IsShowHiddenField() const
        { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::FieldHidden); }
    void SetShowHiddenField( bool b )
        { SetCoreOption(b, ViewOptFlags1::FieldHidden); }

    bool IsGraphic() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Graphic); }
    void SetGraphic( bool b )
        { SetCoreOption(b, ViewOptFlags1::Graphic); }

    bool IsPageBack() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Pageback); }
    void SetPageBack( bool b )
        { SetCoreOption(b, ViewOptFlags1::Pageback); }

    bool IsTable() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Table); }
    void SetTable( bool b )
        { SetCoreOption(b, ViewOptFlags1::Table); }

    bool IsDraw() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Draw); }
    void SetDraw( bool b )
        { SetCoreOption(b, ViewOptFlags1::Draw); }

    bool IsControl() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Control); }
    void SetControl( bool b )
        { SetCoreOption(b, ViewOptFlags1::Control); }

    bool IsSnap() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Snap); }
    void SetSnap( bool b )
        { SetCoreOption(b, ViewOptFlags1::Snap); }

    void SetSnapSize( Size const &rSz ){ m_aSnapSize = rSz; }
    const Size &GetSnapSize() const { return m_aSnapSize; }

    bool IsGridVisible() const
        { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::GridVisible); }
    void SetGridVisible( bool b )
        { SetCoreOption(b, ViewOptFlags1::GridVisible); }

    bool IsOnlineSpell() const
        { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::OnlineSpell); }
    void SetOnlineSpell( bool b );

    bool IsViewMetaChars() const
        { return !m_bReadonly && (m_nCoreOptions & ViewOptFlags1::ViewMetachars); }
    void SetViewMetaChars( bool b)
        { SetCoreOption(b, ViewOptFlags1::ViewMetachars); }

    bool IsSynchronize() const
        {  return bool(m_nCoreOptions & ViewOptFlags1::Synchronize); }
    void SetSynchronize( bool b )
        { SetCoreOption(b, ViewOptFlags1::Synchronize); }

    bool IsCrossHair() const
        { return bool(m_nCoreOptions & ViewOptFlags1::Crosshair); }
    void SetCrossHair( bool b )
        { SetCoreOption(b, ViewOptFlags1::Crosshair); }

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
    void SetCoreOption(bool b, ViewOptFlags1 f)
    {
        if (b)
            m_nCoreOptions |= f;
        else
            m_nCoreOptions &= ~f;
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

    //move vertical ruler to the right
    bool    IsVRulerRight()    const
        { return bool(m_nUIOptions & ViewOptFlags2::VRulerRight); }
    void   SetVRulerRight(bool b)
        { SetUIOption(b, ViewOptFlags2::VRulerRight); }

    bool            IsStarOneSetting() const {return m_bStarOneSetting; }
    void            SetStarOneSetting(bool bSet) {m_bStarOneSetting = bSet; }

    bool            IsPagePreview() const {return m_bIsPagePreview; }
    void            SetPagePreview(bool bSet) { m_bIsPagePreview= bSet; }

    sal_uInt8           GetShdwCursorFillMode() const { return m_nShadowCursorFillMode; }
    void            SetShdwCursorFillMode( sal_uInt8 nMode ) { m_nShadowCursorFillMode = nMode; };

    bool        IsShowPlaceHolderFields() const { return m_bShowPlaceHolderFields; }
    void            SetShowPlaceHolderFields(bool bSet) { m_bShowPlaceHolderFields = bSet; }

    static Color&   GetDocColor();
    static Color&   GetDocBoundariesColor();
    static Color&   GetAppBackgroundColor();
    static Color&   GetObjectBoundariesColor();
    static Color&   GetTableBoundariesColor();
    static Color&   GetIndexShadingsColor();
    static Color&   GetLinksColor();
    static Color&   GetVisitedLinksColor();
    static Color&   GetDirectCursorColor();
    static Color&   GetTextGridColor();
    static Color&   GetSpellColor();
    static Color&   GetSmarttagColor();
    static Color&   GetShadowColor();
    static Color&   GetFontColor();
    static Color&   GetFieldShadingsColor();
    static Color&   GetSectionBoundColor();
    static Color&   GetPageBreakColor();
    static Color&   GetHeaderFooterMarkColor();

    static bool     IsAppearanceFlag(ViewOptFlags nFlag);

    static bool     IsDocBoundaries()     {return IsAppearanceFlag(ViewOptFlags::DocBoundaries);}
    static bool     IsObjectBoundaries()  {return IsAppearanceFlag(ViewOptFlags::ObjectBoundaries);}
    static bool     IsTableBoundaries()   {return IsAppearanceFlag(ViewOptFlags::TableBoundaries );}
    static bool     IsIndexShadings()     {return IsAppearanceFlag(ViewOptFlags::IndexShadings   );}
    static bool     IsLinks()             {return IsAppearanceFlag(ViewOptFlags::Links            );}
    static bool     IsVisitedLinks()      {return IsAppearanceFlag(ViewOptFlags::VisitedLinks    );}
    static bool     IsFieldShadings()     {return IsAppearanceFlag(ViewOptFlags::FieldShadings);}
    static bool     IsSectionBoundaries() {return IsAppearanceFlag(ViewOptFlags::SectionBoundaries);}
    static bool     IsShadow()            {return IsAppearanceFlag(ViewOptFlags::Shadow           );}

    static void     SetAppearanceFlag(ViewOptFlags nFlag, bool bSet, bool bSaveInConfig = false);

    static void     SetDocBoundaries(bool bSet)   {SetAppearanceFlag(ViewOptFlags::DocBoundaries, bSet);}

    static void     ApplyColorConfigValues(const svtools::ColorConfig& rConfig);
};

inline bool SwViewOption::operator==( const SwViewOption &rOpt ) const
{
    return IsEqualFlags( rOpt ) && m_nZoom == rOpt.GetZoom();
}

inline void SwViewOption::SetUIOptions( const SwViewOption& rVOpt )
{
    m_nUIOptions = rVOpt.m_nUIOptions;
    m_nTableDestination = rVOpt.m_nTableDestination;
    m_nShadowCursorFillMode = rVOpt.m_nShadowCursorFillMode;
}

// Helper function for checking HTML-capabilities.
SW_DLLPUBLIC sal_uInt16     GetHtmlMode(const SwDocShell*);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
