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
#include <svx/svxids.hrc>
#include "swdllapi.h"
#include "authratr.hxx"

class SwRect;
namespace vcl { class Window; }
class OutputDevice;
class SwViewShell;
class SwDocShell;
namespace svtools{ class ColorConfig;}

#define VIEWOPT_1_TAB           0x00000002L
#define VIEWOPT_1_BLANK         0x00000004L
#define VIEWOPT_1_HARDBLANK     0x00000008L
#define VIEWOPT_1_PARAGRAPH     0x00000010L
#define VIEWOPT_1_LINEBREAK     0x00000020L
#define VIEWOPT_1_PAGEBREAK     0x00000040L
#define VIEWOPT_1_COLUMNBREAK   0x00000080L
#define VIEWOPT_1_SOFTHYPH      0x00000100L

#define VIEWOPT_1_REF           0x00000400L
#define VIEWOPT_1_FLDNAME       0x00000800L
#define VIEWOPT_1_POSTITS       0x00004000L
#define VIEWOPT_1_FLD_HIDDEN    0x00008000L
#define VIEWOPT_1_CHAR_HIDDEN   0x00010000L
#define VIEWOPT_1_GRAPHIC       0x00020000L
#define VIEWOPT_1_TABLE         0x00040000L
#define VIEWOPT_1_DRAW          0x00080000L
#define VIEWOPT_1_CONTROL       0x00100000L
#define VIEWOPT_1_CROSSHAIR     0x00400000L
#define VIEWOPT_1_SNAP          0x00800000L
#define VIEWOPT_1_SYNCHRONIZE   0x01000000L
#define VIEWOPT_1_GRIDVISIBLE   0x02000000L
#define VIEWOPT_1_ONLINESPELL   0x04000000L
#define VIEWOPT_1_VIEWMETACHARS 0x20000000L
#define VIEWOPT_1_PAGEBACK      0x40000000L

#define VIEWOPT_CORE2_BLACKFONT         0x00000001L
#define VIEWOPT_CORE2_HIDDENPARA        0x00000002L
#define VIEWOPT_CORE2_SMOOTHSCROLL      0x00000004L
#define VIEWOPT_CORE2_CRSR_IN_PROT      0x00000008L
#define VIEWOPT_CORE2_PDF_EXPORT        0x00000010L
#define VIEWOPT_CORE2_PRINTING          0x00000020L
#define VIEWOPT_CORE2_IGNORE_PROT       0x00000040L

#define VIEWOPT_2_H_RULER           0x00000400L
#define VIEWOPT_2_VSCROLLBAR        0x00000800L
#define VIEWOPT_2_HSCROLLBAR        0x00001000L
#define VIEWOPT_2_V_RULER           0x00004000L
#define VIEWOPT_2_ANY_RULER         0x00008000L
#define VIEWOPT_2_MODIFIED          0x00010000L
#define VIEWOPT_2_KEEPASPECTRATIO   0x00020000L
#define VIEWOPT_2_GRFKEEPZOOM       0x00040000L
#define VIEWOPT_2_CONTENT_TIPS      0x00100000L
#define VIEWOPT_2_SCROLLBAR_TIPS    0x00400000L
#define VIEWOPT_2_PRTFORMAT         0x00800000L
#define VIEWOPT_2_SHADOWCRSR        0x01000000L
#define VIEWOPT_2_V_RULER_RIGHT     0x02000000L

// Table background.
#define TBL_DEST_CELL   0
#define TBL_DEST_ROW    1
#define TBL_DEST_TBL    2

// Appearance flags.
#define VIEWOPT_DOC_BOUNDARIES      0x0001
#define VIEWOPT_OBJECT_BOUNDARIES   0x0002
#define VIEWOPT_TABLE_BOUNDARIES    0x0004
#define VIEWOPT_INDEX_SHADINGS      0x0008
#define VIEWOPT_LINKS               0x0010
#define VIEWOPT_VISITED_LINKS       0x0020
#define VIEWOPT_FIELD_SHADINGS      0x0040
#define VIEWOPT_SECTION_BOUNDARIES  0x0080
#define VIEWOPT_SHADOW              0x0100

class SW_DLLPUBLIC SwViewOption
{
    static Color    m_aDocColor;  // color of document boundaries
    static Color    m_aDocBoundColor;  // color of document boundaries
    static Color    m_aObjectBoundColor; // color of object boundaries
    static Color    m_aAppBackgroundColor; // application background
    static Color    m_aTableBoundColor; // color of table boundaries
    static Color    m_aFontColor;
    static Color    m_aIndexShadingsColor; // background color of indexes
    static Color    m_aLinksColor;
    static Color    m_aVisitedLinksColor;
    static Color    m_aDirectCursorColor;
    static Color    m_aTextGridColor;
    static Color    m_aSpellColor;     // mark color of online spell checking
    static Color    m_aSmarttagColor;
    static Color    m_aFieldShadingsColor;
    static Color    m_aSectionBoundColor;
    static Color    m_aPageBreakColor;
    static Color    m_aScriptIndicatorColor;
    static Color    m_aShadowColor;
    static Color    m_aHeaderFooterMarkColor;

    static sal_Int32 m_nAppearanceFlags;
protected:
    static sal_uInt16   m_nPixelTwips;// 1 Pixel == ? Twips

    OUString        m_sSymbolFont;        // Symbolfont.
    sal_uInt32      m_nCoreOptions;       // Bits for SwViewShell.
    sal_uInt32      m_nCore2Options;      // Bits for SwViewShell.
    sal_uInt32      m_nUIOptions;         // UI-Bits
    Color           m_aRetouchColor;     // DefaultBackground for BrowseView
    Size            m_aSnapSize;          // Describes horizontal and vertical snap.
    sal_uInt16      mnViewLayoutColumns;// # columns for edit view
    short           m_nDivisionX;         // Grid division.
    short           m_nDivisionY;
    sal_uInt8       m_nPagePreviewRow;       // Page Preview Row/Columns.
    sal_uInt8       m_nPagePreviewCol;       // Page Preview Row/Columns.
    sal_uInt8       m_nShadowCrsrFillMode;  // FillMode for ShadowCrsr.
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
    static bool  m_bTest9;    // Test-Flag  "DrawingLayerNotLoading"
    bool  m_bTest10       :1;     // Test-Flag  "Format by Input"
#endif

public:
            SwViewOption();                     // CTOR
            SwViewOption(const SwViewOption&);
            ~SwViewOption();

    static void Init( vcl::Window *pWin );        // Initializing of static data.

    static sal_uInt16 GetPixelTwips() { return m_nPixelTwips; }

    inline sal_uInt32   GetCoreOptions() const {return m_nCoreOptions;}
    inline void     SetUIOptions( const SwViewOption& );

    // Options from nCoreOptions
    inline bool IsIdle() const
        { return m_bIdle; }

    // Logically this is a const function since it does not modify the viewoptions
    // but only effects idle formatting. Of course that member is already implement
    // in the wrong place here... Also currently there are many const modifying casts in the code
    // just to call this function on otherwise const objects. Thus declaring it as const now.
    inline void SetIdle( bool b ) const
        { m_bIdle = b; }

    inline bool IsTab(bool bHard = false) const
                    {   return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_TAB) &&
                            ((m_nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetTab( bool b )        {
        b ? (m_nCoreOptions |= VIEWOPT_1_TAB ) : ( m_nCoreOptions &= ~VIEWOPT_1_TAB); }

    inline bool IsBlank(bool bHard = false) const
                    { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_BLANK) &&
                            ((m_nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetBlank( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_BLANK ) : ( m_nCoreOptions &= ~VIEWOPT_1_BLANK); }

    inline bool IsHardBlank() const
                    { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_HARDBLANK) != 0; }
    inline void SetHardBlank( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_HARDBLANK ) : ( m_nCoreOptions &= ~VIEWOPT_1_HARDBLANK); }

    inline bool IsParagraph(bool bHard = false) const
                    {   return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_PARAGRAPH) &&
                            ((m_nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetParagraph( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_PARAGRAPH ) : ( m_nCoreOptions &= ~VIEWOPT_1_PARAGRAPH); }

    inline bool IsLineBreak(bool bHard = false) const
                    {   return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_LINEBREAK) &&
                            ((m_nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetLineBreak( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_LINEBREAK ) : ( m_nCoreOptions &= ~VIEWOPT_1_LINEBREAK); }

    inline void SetPageBreak( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_PAGEBREAK ) : ( m_nCoreOptions &= ~VIEWOPT_1_PAGEBREAK); }

    inline void SetColumnBreak( bool b)
        { b ? (m_nCoreOptions |= VIEWOPT_1_COLUMNBREAK ) : ( m_nCoreOptions &= ~VIEWOPT_1_COLUMNBREAK); }

    inline bool IsSoftHyph() const
                    { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_SOFTHYPH) != 0; }
    inline void SetSoftHyph( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_SOFTHYPH ) : ( m_nCoreOptions &= ~VIEWOPT_1_SOFTHYPH); }

    inline bool IsFieldName() const       { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_FLDNAME) != 0; }
    inline void SetFieldName( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_FLDNAME ) : ( m_nCoreOptions &= ~VIEWOPT_1_FLDNAME); }

    inline bool IsPostIts() const
        { return (m_nCoreOptions & VIEWOPT_1_POSTITS) != 0; }
    inline void SetPostIts( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_POSTITS ) : ( m_nCoreOptions &= ~VIEWOPT_1_POSTITS); }
    static void PaintPostIts( OutputDevice *pOut, const SwRect &rRect,
                              bool bIsScript );
    static sal_uInt16 GetPostItsWidth( const OutputDevice *pOut = nullptr );

    inline bool IsShowHiddenChar(bool bHard = false) const
        { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_CHAR_HIDDEN) &&
                            ((m_nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }

    inline void SetShowHiddenChar( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_CHAR_HIDDEN ) : ( m_nCoreOptions &= ~VIEWOPT_1_CHAR_HIDDEN); }

    inline bool IsShowHiddenField() const
        { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_FLD_HIDDEN) != 0; }
    inline void SetShowHiddenField( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_FLD_HIDDEN ) : ( m_nCoreOptions &= ~VIEWOPT_1_FLD_HIDDEN); }

    inline bool IsGraphic() const
        { return (m_nCoreOptions & VIEWOPT_1_GRAPHIC) != 0; }
    inline void SetGraphic( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_GRAPHIC ) : ( m_nCoreOptions &= ~VIEWOPT_1_GRAPHIC); }

    inline bool IsPageBack() const
        { return (m_nCoreOptions & VIEWOPT_1_PAGEBACK) != 0; }
    inline void SetPageBack( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_PAGEBACK) : ( m_nCoreOptions &= ~VIEWOPT_1_PAGEBACK); }

    inline bool IsTable() const
        { return (m_nCoreOptions & VIEWOPT_1_TABLE) != 0; }
    inline void SetTable( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_TABLE ) : ( m_nCoreOptions &= ~VIEWOPT_1_TABLE); }

    inline bool IsDraw() const
        { return (m_nCoreOptions & VIEWOPT_1_DRAW) != 0; }
    inline void SetDraw( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_DRAW ) : ( m_nCoreOptions &= ~VIEWOPT_1_DRAW); }

    inline bool IsControl() const
        { return (m_nCoreOptions & VIEWOPT_1_CONTROL) != 0; }
    inline void SetControl( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_CONTROL ) : ( m_nCoreOptions &= ~VIEWOPT_1_CONTROL); }

    inline bool IsSnap() const
        { return (m_nCoreOptions & VIEWOPT_1_SNAP) != 0; }
    inline void SetSnap( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_SNAP ) : ( m_nCoreOptions &= ~VIEWOPT_1_SNAP); }

    inline void SetSnapSize( Size &rSz ){ m_aSnapSize = rSz; }
    inline const Size &GetSnapSize() const { return m_aSnapSize; }

    inline bool IsGridVisible() const
        { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_GRIDVISIBLE) != 0; }
    inline void SetGridVisible( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_GRIDVISIBLE ) : ( m_nCoreOptions &= ~VIEWOPT_1_GRIDVISIBLE); }

    inline bool IsOnlineSpell() const
        { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_ONLINESPELL) != 0; }
    void SetOnlineSpell( bool b );

    inline bool IsViewMetaChars() const
        { return !m_bReadonly && (m_nCoreOptions & VIEWOPT_1_VIEWMETACHARS) != 0; }
    inline void SetViewMetaChars( bool b)
        { b ? (m_nCoreOptions |= VIEWOPT_1_VIEWMETACHARS ) : ( m_nCoreOptions &= ~VIEWOPT_1_VIEWMETACHARS); }

    inline bool IsSynchronize() const
        {  return (m_nCoreOptions & VIEWOPT_1_SYNCHRONIZE) != 0; }
    inline void SetSynchronize( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_SYNCHRONIZE ) : ( m_nCoreOptions &= ~VIEWOPT_1_SYNCHRONIZE); }

    inline bool IsCrossHair() const
        { return (m_nCoreOptions & VIEWOPT_1_CROSSHAIR) != 0; }
    inline void SetCrossHair( bool b )
        { b ? (m_nCoreOptions |= VIEWOPT_1_CROSSHAIR ) : ( m_nCoreOptions &= ~VIEWOPT_1_CROSSHAIR); }

    // Options from nCore2Options
    inline bool IsBlackFont() const
        {return (m_nCore2Options & VIEWOPT_CORE2_BLACKFONT) != 0; }

    inline void SetBlackFont(bool b)
        { b ? (m_nCore2Options |= VIEWOPT_CORE2_BLACKFONT) : (m_nCore2Options &= ~VIEWOPT_CORE2_BLACKFONT);}

    inline bool IsShowHiddenPara() const
        {return (m_nCore2Options & VIEWOPT_CORE2_HIDDENPARA) != 0; }

    inline void SetShowHiddenPara(bool b)
        { b ? (m_nCore2Options |= VIEWOPT_CORE2_HIDDENPARA) : (m_nCore2Options &= ~VIEWOPT_CORE2_HIDDENPARA);}

    inline bool IsSmoothScroll() const
        {return (m_nCore2Options & VIEWOPT_CORE2_SMOOTHSCROLL) != 0; }

    inline void SetSmoothScroll(bool b)
        { b ? (m_nCore2Options |= VIEWOPT_CORE2_SMOOTHSCROLL) : (m_nCore2Options &= ~VIEWOPT_CORE2_SMOOTHSCROLL);}

    inline bool IsCursorInProtectedArea() const
        {return (m_nCore2Options & VIEWOPT_CORE2_CRSR_IN_PROT) != 0; }

    inline void SetCursorInProtectedArea(bool b)
        { b ? (m_nCore2Options |= VIEWOPT_CORE2_CRSR_IN_PROT) : (m_nCore2Options &= ~VIEWOPT_CORE2_CRSR_IN_PROT);}

    bool IsIgnoreProtectedArea() const
    {
        return (m_nCore2Options & VIEWOPT_CORE2_IGNORE_PROT) != 0;
    }

    void SetIgnoreProtectedArea(bool bSet)
    {
        bSet ? (m_nCore2Options |= VIEWOPT_CORE2_IGNORE_PROT) : (m_nCore2Options &= ~VIEWOPT_CORE2_IGNORE_PROT);
    }

    inline bool IsPDFExport() const
        {return (m_nCore2Options & VIEWOPT_CORE2_PDF_EXPORT) != 0; }

    inline void SetPDFExport(bool b)
        { b ? (m_nCore2Options |= VIEWOPT_CORE2_PDF_EXPORT) : (m_nCore2Options &= ~VIEWOPT_CORE2_PDF_EXPORT);}

    inline bool IsPrinting() const
        {return (m_nCore2Options & VIEWOPT_CORE2_PRINTING) != 0; }

    inline void SetPrinting(bool b)
        { b ? (m_nCore2Options |= VIEWOPT_CORE2_PRINTING) : (m_nCore2Options &= ~VIEWOPT_CORE2_PRINTING);}

    inline short GetDivisionX() const   { return m_nDivisionX; }
    inline void  SetDivisionX( short n ){ m_nDivisionX = n; }
    inline short GetDivisionY() const   { return m_nDivisionY; }
    inline void  SetDivisionY( short n ){ m_nDivisionY = n; }

    // Default margin left and above document: 284 twips == 5.0 mm.
    static SAL_CONSTEXPR sal_uInt16 GetDefDocumentBorder() { return 284; }
    // Default gap between pages: 284 twips == 5.0 mm.
    static SAL_CONSTEXPR sal_uInt16 GetDefGapBetweenPages() { return 284; }
    // Minimum edge-to-text distance: 22 twips == 0.4 mm.
    static SAL_CONSTEXPR sal_uInt16 GetMinGapBetweenPages() { return 22; }

    inline sal_uInt16 GetDocumentBorder() const { return IsWhitespaceHidden() ? GetMinGapBetweenPages() : GetDefDocumentBorder(); }
    inline sal_uInt16 GetGapBetweenPages() const { return IsWhitespaceHidden() ? GetMinGapBetweenPages() : GetDefGapBetweenPages(); }

    inline sal_uInt8  GetPagePrevRow() const      { return m_nPagePreviewRow; }
    inline void  SetPagePrevRow( sal_uInt8 n ) { m_nPagePreviewRow = n; }
    inline sal_uInt8  GetPagePrevCol() const      { return m_nPagePreviewCol; }
    inline void  SetPagePrevCol( sal_uInt8 n ) { m_nPagePreviewCol = n; }
    bool         IsReadonly() const { return m_bReadonly; }
    void         SetReadonly(bool bSet) { m_bReadonly = bSet; }

    bool         IsSelectionInReadonly() const {return m_bSelectionInReadonly;}
    void         SetSelectionInReadonly(bool bSet) {m_bSelectionInReadonly = bSet;}

    bool         IsFormView() const { return mbFormView; }
    void         SetFormView( bool bSet ) { mbFormView = bSet; }

    inline bool  getBrowseMode() const { return mbBrowseMode; }
    inline void  setBrowseMode(bool bSet) { mbBrowseMode = bSet; }
    inline bool  IsPagePrevBookview() const { return mbBookView; }
    inline void  SetPagePrevBookview(bool bSet) { mbBookView = bSet; }

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
    inline bool IsTest1() const     { return m_bTest1; }
    inline bool IsTest2() const     { return m_bTest2; }
    inline bool IsTest3() const     { return m_bTest3; }
    inline bool IsTest4() const     { return m_bTest4; }
    inline bool IsTest5() const     { return m_bTest5; }
    inline bool IsTest6() const     { return m_bTest6; }
    inline bool IsTest7() const     { return m_bTest7; }
    inline bool IsTest8() const     { return m_bTest8; }
    inline bool IsTest10() const    { return m_bTest10; }
#endif

    inline sal_uInt16 GetZoom() const    { return m_nZoom; }
    inline void   SetZoom( sal_uInt16 n ){ m_nZoom = n; }

    static void DrawRect( OutputDevice* pOut, const SwRect &rRect, long nCol );
    static void DrawRectPrinter( OutputDevice* pOut, const SwRect& rRect );

    SwViewOption& operator=( const SwViewOption &rOpt );

    // Compare methods.
    bool IsEqualFlags ( const SwViewOption &rOpt ) const;
    inline bool operator == ( const SwViewOption &rOpt ) const;
    inline bool operator != ( const SwViewOption &rOpt ) const  { return !(*this == rOpt); }

    // Options from nUIOptions
    bool    IsViewVScrollBar() const
        {
#if HAVE_FEATURE_DESKTOP
            return (m_nUIOptions & VIEWOPT_2_VSCROLLBAR) != 0;
#else
            return false;
#endif
        }
    bool    IsViewHScrollBar() const
        {
#if HAVE_FEATURE_DESKTOP
            return (m_nUIOptions & VIEWOPT_2_HSCROLLBAR) != 0;
#else
            return false;
#endif
        }
    bool    IsKeepRatio()      const
        { return (m_nUIOptions & VIEWOPT_2_KEEPASPECTRATIO) != 0; }
    bool    IsGrfKeepZoom()    const
        { return (m_nUIOptions & VIEWOPT_2_GRFKEEPZOOM) != 0; }
    bool    IsShowContentTips() const
        { return (m_nUIOptions & VIEWOPT_2_CONTENT_TIPS) != 0; }
    bool    IsPrtFormat() const
        { return (m_nUIOptions & VIEWOPT_2_PRTFORMAT) != 0; }
    bool    IsShowScrollBarTips() const
        { return (m_nUIOptions & VIEWOPT_2_SCROLLBAR_TIPS) != 0; }

    SvxZoomType    GetZoomType()      const { return m_eZoom; }

    sal_uInt8   GetTableDest() const    { return m_nTableDestination; }

    void   SetViewVScrollBar(bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_VSCROLLBAR ) : ( m_nUIOptions &= ~VIEWOPT_2_VSCROLLBAR); }
    void   SetViewHScrollBar(bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_HSCROLLBAR ) : ( m_nUIOptions &= ~VIEWOPT_2_HSCROLLBAR); }
    void   SetKeepRatio     (bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_KEEPASPECTRATIO ) : ( m_nUIOptions &= ~VIEWOPT_2_KEEPASPECTRATIO); }
    void   SetGrfKeepZoom   (bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_GRFKEEPZOOM ) : ( m_nUIOptions &= ~VIEWOPT_2_GRFKEEPZOOM); }
    void SetShowContentTips( bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_CONTENT_TIPS) : (m_nUIOptions &= ~VIEWOPT_2_CONTENT_TIPS); }
    void SetPrtFormat( bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_PRTFORMAT) : (m_nUIOptions &= ~VIEWOPT_2_PRTFORMAT); }
    void SetShowScrollBarTips( bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_SCROLLBAR_TIPS) : (m_nUIOptions &= ~VIEWOPT_2_SCROLLBAR_TIPS); }

    void            SetZoomType     (SvxZoomType eZoom_){ m_eZoom = eZoom_;  }
    void            SetTableDest( sal_uInt8 nNew )    { m_nTableDestination = nNew;  }

    const OUString& GetSymbolFont() const {return m_sSymbolFont;}
    void            SetSymbolFont(const OUString& sSet) {m_sSymbolFont = sSet;}

    const Color&    GetRetoucheColor() const        { return m_aRetouchColor;}
    void            SetRetoucheColor(const Color&r) { m_aRetouchColor = r;   }

    bool        IsViewAnyRuler() const
        {
#if HAVE_FEATURE_DESKTOP
            return 0 != (m_nUIOptions & VIEWOPT_2_ANY_RULER);
#else
            return false;
#endif
        }
    void            SetViewAnyRuler(bool bSet)
                        { bSet ? (m_nUIOptions |= VIEWOPT_2_ANY_RULER) : (m_nUIOptions &= ~VIEWOPT_2_ANY_RULER);}

    bool        IsViewHRuler(bool bDirect = false)     const
                        {
#if HAVE_FEATURE_DESKTOP
                            return bDirect
                                   ? 0 != (m_nUIOptions & VIEWOPT_2_H_RULER)
                                   : !m_bReadonly && (m_nUIOptions & (VIEWOPT_2_ANY_RULER|VIEWOPT_2_H_RULER)) == (VIEWOPT_2_ANY_RULER|VIEWOPT_2_H_RULER);
#else
                            (void) bDirect;
                            return false;
#endif
                        }
    void            SetViewHRuler   (bool b)
                        {    b ? (m_nUIOptions |= VIEWOPT_2_H_RULER ) : ( m_nUIOptions &= ~VIEWOPT_2_H_RULER);}

    bool            IsViewVRuler(bool bDirect = false) const
                        {
#if HAVE_FEATURE_DESKTOP
                            return bDirect
                                   ? 0 != (m_nUIOptions & VIEWOPT_2_V_RULER)
                                   : !m_bReadonly && (m_nUIOptions & (VIEWOPT_2_ANY_RULER|VIEWOPT_2_V_RULER)) == (VIEWOPT_2_ANY_RULER|VIEWOPT_2_V_RULER);
#else
                            (void) bDirect;
                            return false;
#endif
                        }
    void            SetViewVRuler     (bool b)
                        { b ? (m_nUIOptions |= VIEWOPT_2_V_RULER ) : ( m_nUIOptions &= ~VIEWOPT_2_V_RULER);}

    // ShadowCursor, switch on/off, get/set color/mode.
    bool    IsShadowCursor()    const
        { return (m_nUIOptions & VIEWOPT_2_SHADOWCRSR) != 0; }
    void   SetShadowCursor(bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_SHADOWCRSR ) : ( m_nUIOptions &= ~VIEWOPT_2_SHADOWCRSR); }

    //move vertical ruler to the right
    bool    IsVRulerRight()    const
        { return (m_nUIOptions & VIEWOPT_2_V_RULER_RIGHT) != 0; }
    void   SetVRulerRight(bool b)
        { b ? (m_nUIOptions |= VIEWOPT_2_V_RULER_RIGHT ) : ( m_nUIOptions &= ~VIEWOPT_2_V_RULER_RIGHT); }

    bool            IsStarOneSetting() const {return m_bStarOneSetting; }
    void            SetStarOneSetting(bool bSet) {m_bStarOneSetting = bSet; }

    bool            IsPagePreview() const {return m_bIsPagePreview; }
    void            SetPagePreview(bool bSet) { m_bIsPagePreview= bSet; }

    sal_uInt8           GetShdwCrsrFillMode() const { return m_nShadowCrsrFillMode; }
    void            SetShdwCrsrFillMode( sal_uInt8 nMode ) { m_nShadowCrsrFillMode = nMode; };

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

    static bool     IsAppearanceFlag(sal_Int32 nFlag);

    static bool     IsDocBoundaries()   {return IsAppearanceFlag(VIEWOPT_DOC_BOUNDARIES);}
    static bool     IsObjectBoundaries(){return IsAppearanceFlag(VIEWOPT_OBJECT_BOUNDARIES);}
    static bool     IsTableBoundaries() {return IsAppearanceFlag(VIEWOPT_TABLE_BOUNDARIES );}
    static bool     IsIndexShadings()   {return IsAppearanceFlag(VIEWOPT_INDEX_SHADINGS   );}
    static bool     IsLinks()           {return IsAppearanceFlag(VIEWOPT_LINKS            );}
    static bool     IsVisitedLinks()    {return IsAppearanceFlag(VIEWOPT_VISITED_LINKS    );}
    static bool     IsFieldShadings()   {return IsAppearanceFlag(VIEWOPT_FIELD_SHADINGS);}
    static bool     IsSectionBoundaries() {return IsAppearanceFlag(VIEWOPT_SECTION_BOUNDARIES);}
    static bool     IsShadow()          {return IsAppearanceFlag(VIEWOPT_SHADOW           );}

    static void     SetAppearanceFlag(sal_Int32 nFlag, bool bSet, bool bSaveInConfig = false);

    static void     SetDocBoundaries(bool bSet)   {SetAppearanceFlag(VIEWOPT_DOC_BOUNDARIES, bSet);}

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
    m_nShadowCrsrFillMode = rVOpt.m_nShadowCrsrFillMode;
}

// Helper function for checking HTML-capabilities.
SW_DLLPUBLIC sal_uInt16     GetHtmlMode(const SwDocShell*);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
