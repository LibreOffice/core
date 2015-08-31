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

// Implementation in core/text/txtpaint.cxx.
extern void SyncVout( const OutputDevice *pOut );

class SW_DLLPUBLIC SwViewOption
{
    static Color    aDocColor;  // color of document boundaries
    static Color    aDocBoundColor;  // color of document boundaries
    static Color    aObjectBoundColor; // color of object boundaries
    static Color    aAppBackgroundColor; // application background
    static Color    aTableBoundColor; // color of table boundaries
    static Color    aFontColor;
    static Color    aIndexShadingsColor; // background color of indexes
    static Color    aLinksColor;
    static Color    aVisitedLinksColor;
    static Color    aDirectCursorColor;
    static Color    aTextGridColor;
    static Color    aSpellColor;     // mark color of online spell checking
    static Color    aSmarttagColor;
    static Color    aFieldShadingsColor;
    static Color    aSectionBoundColor;
    static Color    aPageBreakColor;
    static Color    aScriptIndicatorColor;
    static Color    aShadowColor;
    static Color    aHeaderFooterMarkColor;

    static sal_Int32 nAppearanceFlags;
protected:
    static sal_uInt16   nPixelTwips;// 1 Pixel == ? Twips

    OUString        sSymbolFont;        // Symbolfont.
    sal_uInt32      nCoreOptions;       // Bits for SwViewShell.
    sal_uInt32      nCore2Options;      // Bits for SwViewShell.
    sal_uInt32      nUIOptions;         // UI-Bits
    Color           aRetoucheColor;     // DefaultBackground for BrowseView
    Size            aSnapSize;          // Describes horizontal and vertical snap.
    sal_uInt16      mnViewLayoutColumns;// # columns for edit view
    short           nDivisionX;         // Grid division.
    short           nDivisionY;
    sal_uInt8       nPagePrevRow;       // Page Preview Row/Columns.
    sal_uInt8       nPagePrevCol;       // Page Preview Row/Columns.
    sal_uInt8       nShdwCrsrFillMode;  // FillMode for ShadowCrsr.
    bool            bReadonly : 1;      // Readonly-Doc.
    bool            bStarOneSetting : 1;// Prevent from UI automatics (no scrollbars in readonly documents).
    bool            bIsPagePreview : 1; // The preview mustn't print field/footnote/... shadings.
    bool            bSelectionInReadonly : 1; // Determines whether selection is switched on in readonly documents.
    bool            mbFormView : 1;
    bool            mbBrowseMode : 1;
    bool            mbBookView : 1;      // View mode for page preview.
    bool            mbViewLayoutBookMode : 1; // Book view mode for edit view.
    bool            mbHideWhitespaceMode : 1; // Hide header, footer, and pagebreak.
    bool            bShowPlaceHolderFields : 1; // Only used in printing!
    mutable bool    bIdle;

    // Scale
    sal_uInt16          nZoom;          // In percent.
    SvxZoomType     eZoom;              // 'enum' for zoom.

    sal_uInt8            nTableDest;      // Destination for table background.

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
    static bool  s_bTest9;    // Test-Flag  "DrawingLayerNotLoading"
    bool  m_bTest10       :1;     // Test-Flag  "Format by Input"
#endif

public:
            SwViewOption();                     // CTOR
            SwViewOption(const SwViewOption&);
            ~SwViewOption();

    static void Init( vcl::Window *pWin );        // Initializing of static data.

    static sal_uInt16 GetPixelTwips() { return nPixelTwips; }

    inline sal_uInt32   GetCoreOptions() const {return nCoreOptions;}
    inline void     SetUIOptions( const SwViewOption& );

    // Options from nCoreOptions
    inline bool IsIdle() const
        { return bIdle; }

    // Logically this is a const function since it does not modify the viewoptions
    // but only effects idle formatting. Of course that member is already implement
    // in the wrong place here... Also currently there are many const modifying casts in the code
    // just to call this function on otherwise const objects. Thus declaring it as const now.
    inline void SetIdle( bool b ) const
        { bIdle = b; }

    inline bool IsTab(bool bHard = false) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_TAB) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetTab( bool b )        {
        b ? (nCoreOptions |= VIEWOPT_1_TAB ) : ( nCoreOptions &= ~VIEWOPT_1_TAB); }

    inline bool IsBlank(bool bHard = false) const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_BLANK) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetBlank( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_BLANK ) : ( nCoreOptions &= ~VIEWOPT_1_BLANK); }

    inline bool IsHardBlank() const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_HARDBLANK) != 0; }
    inline void SetHardBlank( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_HARDBLANK ) : ( nCoreOptions &= ~VIEWOPT_1_HARDBLANK); }

    inline bool IsParagraph(bool bHard = false) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_PARAGRAPH) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetParagraph( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_PARAGRAPH ) : ( nCoreOptions &= ~VIEWOPT_1_PARAGRAPH); }

    inline bool IsLineBreak(bool bHard = false) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_LINEBREAK) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }
    inline void SetLineBreak( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_LINEBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_LINEBREAK); }

    inline void SetPageBreak( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_PAGEBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_PAGEBREAK); }

    inline void SetColumnBreak( bool b)
        { b ? (nCoreOptions |= VIEWOPT_1_COLUMNBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_COLUMNBREAK); }

    inline bool IsSoftHyph() const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_SOFTHYPH) != 0; }
    inline void SetSoftHyph( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_SOFTHYPH ) : ( nCoreOptions &= ~VIEWOPT_1_SOFTHYPH); }

    inline bool IsFieldName() const       { return !bReadonly && (nCoreOptions & VIEWOPT_1_FLDNAME) != 0; }
    inline void SetFieldName( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_FLDNAME ) : ( nCoreOptions &= ~VIEWOPT_1_FLDNAME); }

    inline bool IsPostIts() const
        { return (nCoreOptions & VIEWOPT_1_POSTITS) != 0; }
    inline void SetPostIts( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_POSTITS ) : ( nCoreOptions &= ~VIEWOPT_1_POSTITS); }
    static void PaintPostIts( OutputDevice *pOut, const SwRect &rRect,
                              bool bIsScript );
    static sal_uInt16 GetPostItsWidth( const OutputDevice *pOut = 0 );

    inline bool IsShowHiddenChar(bool bHard = false) const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_CHAR_HIDDEN) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard); }

    inline void SetShowHiddenChar( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_CHAR_HIDDEN ) : ( nCoreOptions &= ~VIEWOPT_1_CHAR_HIDDEN); }

    inline bool IsShowHiddenField() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_FLD_HIDDEN) != 0; }
    inline void SetShowHiddenField( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_FLD_HIDDEN ) : ( nCoreOptions &= ~VIEWOPT_1_FLD_HIDDEN); }

    inline bool IsGraphic() const
        { return (nCoreOptions & VIEWOPT_1_GRAPHIC) != 0; }
    inline void SetGraphic( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_GRAPHIC ) : ( nCoreOptions &= ~VIEWOPT_1_GRAPHIC); }

    inline bool IsPageBack() const
        { return (nCoreOptions & VIEWOPT_1_PAGEBACK) != 0; }
    inline void SetPageBack( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_PAGEBACK) : ( nCoreOptions &= ~VIEWOPT_1_PAGEBACK); }

    inline bool IsTable() const
        { return (nCoreOptions & VIEWOPT_1_TABLE) != 0; }
    inline void SetTable( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_TABLE ) : ( nCoreOptions &= ~VIEWOPT_1_TABLE); }

    inline bool IsDraw() const
        { return (nCoreOptions & VIEWOPT_1_DRAW) != 0; }
    inline void SetDraw( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_DRAW ) : ( nCoreOptions &= ~VIEWOPT_1_DRAW); }

    inline bool IsControl() const
        { return (nCoreOptions & VIEWOPT_1_CONTROL) != 0; }
    inline void SetControl( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_CONTROL ) : ( nCoreOptions &= ~VIEWOPT_1_CONTROL); }

    inline bool IsSnap() const
        { return (nCoreOptions & VIEWOPT_1_SNAP) != 0; }
    inline void SetSnap( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_SNAP ) : ( nCoreOptions &= ~VIEWOPT_1_SNAP); }

    inline void SetSnapSize( Size &rSz ){ aSnapSize = rSz; }
    inline const Size &GetSnapSize() const { return aSnapSize; }

    inline bool IsGridVisible() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_GRIDVISIBLE) != 0; }
    inline void SetGridVisible( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_GRIDVISIBLE ) : ( nCoreOptions &= ~VIEWOPT_1_GRIDVISIBLE); }

    inline bool IsOnlineSpell() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_ONLINESPELL) != 0; }
    inline void SetOnlineSpell( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_ONLINESPELL ) : ( nCoreOptions &= ~VIEWOPT_1_ONLINESPELL); }

    inline bool IsViewMetaChars() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_VIEWMETACHARS) != 0; }
    inline void SetViewMetaChars( bool b)
        { b ? (nCoreOptions |= VIEWOPT_1_VIEWMETACHARS ) : ( nCoreOptions &= ~VIEWOPT_1_VIEWMETACHARS); }

    inline bool IsSynchronize() const
        {  return (nCoreOptions & VIEWOPT_1_SYNCHRONIZE) != 0; }
    inline void SetSynchronize( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_SYNCHRONIZE ) : ( nCoreOptions &= ~VIEWOPT_1_SYNCHRONIZE); }

    inline bool IsCrossHair() const
        { return (nCoreOptions & VIEWOPT_1_CROSSHAIR) != 0; }
    inline void SetCrossHair( bool b )
        { b ? (nCoreOptions |= VIEWOPT_1_CROSSHAIR ) : ( nCoreOptions &= ~VIEWOPT_1_CROSSHAIR); }

    // Options from nCore2Options
    inline bool IsBlackFont() const
        {return (nCore2Options & VIEWOPT_CORE2_BLACKFONT) != 0; }

    inline void SetBlackFont(bool b)
        { b ? (nCore2Options |= VIEWOPT_CORE2_BLACKFONT) : (nCore2Options &= ~VIEWOPT_CORE2_BLACKFONT);}

    inline bool IsShowHiddenPara() const
        {return (nCore2Options & VIEWOPT_CORE2_HIDDENPARA) != 0; }

    inline void SetShowHiddenPara(bool b)
        { b ? (nCore2Options |= VIEWOPT_CORE2_HIDDENPARA) : (nCore2Options &= ~VIEWOPT_CORE2_HIDDENPARA);}

    inline bool IsSmoothScroll() const
        {return (nCore2Options & VIEWOPT_CORE2_SMOOTHSCROLL) != 0; }

    inline void SetSmoothScroll(bool b)
        { b ? (nCore2Options |= VIEWOPT_CORE2_SMOOTHSCROLL) : (nCore2Options &= ~VIEWOPT_CORE2_SMOOTHSCROLL);}

    inline bool IsCursorInProtectedArea() const
        {return (nCore2Options & VIEWOPT_CORE2_CRSR_IN_PROT) != 0; }

    inline void SetCursorInProtectedArea(bool b)
        { b ? (nCore2Options |= VIEWOPT_CORE2_CRSR_IN_PROT) : (nCore2Options &= ~VIEWOPT_CORE2_CRSR_IN_PROT);}

    bool IsIgnoreProtectedArea() const
    {
        return (nCore2Options & VIEWOPT_CORE2_IGNORE_PROT) != 0;
    }

    void SetIgnoreProtectedArea(bool bSet)
    {
        bSet ? (nCore2Options |= VIEWOPT_CORE2_IGNORE_PROT) : (nCore2Options &= ~VIEWOPT_CORE2_IGNORE_PROT);
    }

    inline bool IsPDFExport() const
        {return (nCore2Options & VIEWOPT_CORE2_PDF_EXPORT) != 0; }

    inline void SetPDFExport(bool b)
        { b ? (nCore2Options |= VIEWOPT_CORE2_PDF_EXPORT) : (nCore2Options &= ~VIEWOPT_CORE2_PDF_EXPORT);}

    inline bool IsPrinting() const
        {return (nCore2Options & VIEWOPT_CORE2_PRINTING) != 0; }

    inline void SetPrinting(bool b)
        { b ? (nCore2Options |= VIEWOPT_CORE2_PRINTING) : (nCore2Options &= ~VIEWOPT_CORE2_PRINTING);}

    inline short GetDivisionX() const   { return nDivisionX; }
    inline void  SetDivisionX( short n ){ nDivisionX = n; }
    inline short GetDivisionY() const   { return nDivisionY; }
    inline void  SetDivisionY( short n ){ nDivisionY = n; }

    // Default margin left and above document: 284 twips == 5.0 mm.
    static SAL_CONSTEXPR sal_uInt16 GetDefDocumentBorder() { return 284; }
    // Default gap between pages: 284 twips == 5.0 mm.
    static SAL_CONSTEXPR sal_uInt16 GetDefGapBetweenPages() { return 284; }
    // Minimum edge-to-text distance: 114 twips == 2.0 mm.
    static SAL_CONSTEXPR sal_uInt16 GetMinGapBetweenPages() { return 114; }

    inline sal_uInt16 GetDocumentBorder() const { return IsWhitespaceHidden() ? GetMinGapBetweenPages() : GetDefDocumentBorder(); }
    inline sal_uInt16 GetGapBetweenPages() const { return IsWhitespaceHidden() ? GetMinGapBetweenPages() : GetDefGapBetweenPages(); }

    inline sal_uInt8  GetPagePrevRow() const      { return nPagePrevRow; }
    inline void  SetPagePrevRow( sal_uInt8 n ) { nPagePrevRow = n; }
    inline sal_uInt8  GetPagePrevCol() const      { return nPagePrevCol; }
    inline void  SetPagePrevCol( sal_uInt8 n ) { nPagePrevCol = n; }
    bool         IsReadonly() const { return bReadonly; }
    void         SetReadonly(bool bSet) { bReadonly = bSet; }

    bool         IsSelectionInReadonly() const {return bSelectionInReadonly;}
    void         SetSelectionInReadonly(bool bSet) {bSelectionInReadonly = bSet;}

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

    inline sal_uInt16 GetZoom() const    { return nZoom; }
    inline void   SetZoom( sal_uInt16 n ){ nZoom = n; }

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
            return (nUIOptions & VIEWOPT_2_VSCROLLBAR) != 0;
#else
            return false;
#endif
        }
    bool    IsViewHScrollBar() const
        {
#if HAVE_FEATURE_DESKTOP
            return (nUIOptions & VIEWOPT_2_HSCROLLBAR) != 0;
#else
            return false;
#endif
        }
    bool    IsKeepRatio()      const
        { return (nUIOptions & VIEWOPT_2_KEEPASPECTRATIO) != 0; }
    bool    IsGrfKeepZoom()    const
        { return (nUIOptions & VIEWOPT_2_GRFKEEPZOOM) != 0; }
    bool    IsShowContentTips() const
        { return (nUIOptions & VIEWOPT_2_CONTENT_TIPS) != 0; }
    bool    IsPrtFormat() const
        { return (nUIOptions & VIEWOPT_2_PRTFORMAT) != 0; }
    bool    IsShowScrollBarTips() const
        { return (nUIOptions & VIEWOPT_2_SCROLLBAR_TIPS) != 0; }

    SvxZoomType    GetZoomType()      const { return eZoom; }

    sal_uInt8   GetTableDest() const    { return nTableDest; }

    void   SetViewVScrollBar(bool b)
        { b ? (nUIOptions |= VIEWOPT_2_VSCROLLBAR ) : ( nUIOptions &= ~VIEWOPT_2_VSCROLLBAR); }
    void   SetViewHScrollBar(bool b)
        { b ? (nUIOptions |= VIEWOPT_2_HSCROLLBAR ) : ( nUIOptions &= ~VIEWOPT_2_HSCROLLBAR); }
    void   SetKeepRatio     (bool b)
        { b ? (nUIOptions |= VIEWOPT_2_KEEPASPECTRATIO ) : ( nUIOptions &= ~VIEWOPT_2_KEEPASPECTRATIO); }
    void   SetGrfKeepZoom   (bool b)
        { b ? (nUIOptions |= VIEWOPT_2_GRFKEEPZOOM ) : ( nUIOptions &= ~VIEWOPT_2_GRFKEEPZOOM); }
    void SetShowContentTips( bool b)
        { b ? (nUIOptions |= VIEWOPT_2_CONTENT_TIPS) : (nUIOptions &= ~VIEWOPT_2_CONTENT_TIPS); }
    void SetPrtFormat( bool b)
        { b ? (nUIOptions |= VIEWOPT_2_PRTFORMAT) : (nUIOptions &= ~VIEWOPT_2_PRTFORMAT); }
    void SetShowScrollBarTips( bool b)
        { b ? (nUIOptions |= VIEWOPT_2_SCROLLBAR_TIPS) : (nUIOptions &= ~VIEWOPT_2_SCROLLBAR_TIPS); }

    void            SetZoomType     (SvxZoomType eZoom_){ eZoom = eZoom_;  }
    void            SetTableDest( sal_uInt8 nNew )    { nTableDest = nNew;  }

    const OUString& GetSymbolFont() const {return sSymbolFont;}
    void            SetSymbolFont(const OUString& sSet) {sSymbolFont = sSet;}

    const Color&    GetRetoucheColor() const        { return aRetoucheColor;}
    void            SetRetoucheColor(const Color&r) { aRetoucheColor = r;   }

    bool        IsViewAnyRuler() const
        {
#if HAVE_FEATURE_DESKTOP
            return 0 != (nUIOptions & VIEWOPT_2_ANY_RULER);
#else
            return false;
#endif
        }
    void            SetViewAnyRuler(bool bSet)
                        { bSet ? (nUIOptions |= VIEWOPT_2_ANY_RULER) : (nUIOptions &= ~VIEWOPT_2_ANY_RULER);}

    bool        IsViewHRuler(bool bDirect = false)     const
                        {
#if HAVE_FEATURE_DESKTOP
                            return bDirect
                                   ? 0 != (nUIOptions & VIEWOPT_2_H_RULER)
                                   : !bReadonly && (nUIOptions & (VIEWOPT_2_ANY_RULER|VIEWOPT_2_H_RULER)) == (VIEWOPT_2_ANY_RULER|VIEWOPT_2_H_RULER);
#else
                            (void) bDirect;
                            return false;
#endif
                        }
    void            SetViewHRuler   (bool b)
                        {    b ? (nUIOptions |= VIEWOPT_2_H_RULER ) : ( nUIOptions &= ~VIEWOPT_2_H_RULER);}

    bool            IsViewVRuler(bool bDirect = false) const
                        {
#if HAVE_FEATURE_DESKTOP
                            return bDirect
                                   ? 0 != (nUIOptions & VIEWOPT_2_V_RULER)
                                   : !bReadonly && (nUIOptions & (VIEWOPT_2_ANY_RULER|VIEWOPT_2_V_RULER)) == (VIEWOPT_2_ANY_RULER|VIEWOPT_2_V_RULER);
#else
                            (void) bDirect;
                            return false;
#endif
                        }
    void            SetViewVRuler     (bool b)
                        { b ? (nUIOptions |= VIEWOPT_2_V_RULER ) : ( nUIOptions &= ~VIEWOPT_2_V_RULER);}

    // ShadowCursor, switch on/off, get/set color/mode.
    bool    IsShadowCursor()    const
        { return (nUIOptions & VIEWOPT_2_SHADOWCRSR) != 0; }
    void   SetShadowCursor(bool b)
        { b ? (nUIOptions |= VIEWOPT_2_SHADOWCRSR ) : ( nUIOptions &= ~VIEWOPT_2_SHADOWCRSR); }

    //move vertical ruler to the right
    bool    IsVRulerRight()    const
        { return (nUIOptions & VIEWOPT_2_V_RULER_RIGHT) != 0; }
    void   SetVRulerRight(bool b)
        { b ? (nUIOptions |= VIEWOPT_2_V_RULER_RIGHT ) : ( nUIOptions &= ~VIEWOPT_2_V_RULER_RIGHT); }

    bool            IsStarOneSetting() const {return bStarOneSetting; }
    void            SetStarOneSetting(bool bSet) {bStarOneSetting = bSet; }

    bool            IsPagePreview() const {return bIsPagePreview; }
    void            SetPagePreview(bool bSet) { bIsPagePreview= bSet; }

    sal_uInt8           GetShdwCrsrFillMode() const { return nShdwCrsrFillMode; }
    void            SetShdwCrsrFillMode( sal_uInt8 nMode ) { nShdwCrsrFillMode = nMode; };

    bool        IsShowPlaceHolderFields() const { return bShowPlaceHolderFields; }
    void            SetShowPlaceHolderFields(bool bSet) { bShowPlaceHolderFields = bSet; }

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
    return IsEqualFlags( rOpt ) && nZoom == rOpt.GetZoom();
}

inline void SwViewOption::SetUIOptions( const SwViewOption& rVOpt )
{
    nUIOptions = rVOpt.nUIOptions;
    nTableDest = rVOpt.nTableDest;
    nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
}

// Helper function for checking HTML-capabilities.
SW_DLLPUBLIC sal_uInt16     GetHtmlMode(const SwDocShell*);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
