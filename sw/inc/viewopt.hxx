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

#ifndef _VIEWOPT_HXX
#define _VIEWOPT_HXX

#include <config_features.h>

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <tools/color.hxx>

#include <sfx2/zoomitem.hxx>
#include <svx/svxids.hrc>
#include "swdllapi.h"
#include "authratr.hxx"

class SwRect;
class Window;
class OutputDevice;
class ViewShell;
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
#define VIEWOPT_1_RESERVED1     0x10000000L
#define VIEWOPT_1_VIEWMETACHARS 0x20000000L
#define VIEWOPT_1_PAGEBACK      0x40000000L

#define VIEWOPT_CORE2_BLACKFONT         0x00000001L
#define VIEWOPT_CORE2_HIDDENPARA        0x00000002L
#define VIEWOPT_CORE2_SMOOTHSCROLL      0x00000004L
#define VIEWOPT_CORE2_CRSR_IN_PROT      0x00000008L
#define VIEWOPT_CORE2_PDF_EXPORT        0x00000010L
#define VIEWOPT_CORE2_PRINTING          0x00000020L

#define VIEWOPT_2_UNUSED1           0x00000100L
#define VIEWOPT_2_UNUSED2           0x00000200L
#define VIEWOPT_2_H_RULER           0x00000400L
#define VIEWOPT_2_VSCROLLBAR        0x00000800L
#define VIEWOPT_2_HSCROLLBAR        0x00001000L
#define VIEWOPT_2_STATUSLINE        0x00002000L
#define VIEWOPT_2_V_RULER           0x00004000L
#define VIEWOPT_2_ANY_RULER         0x00008000L
#define VIEWOPT_2_MODIFIED          0x00010000L
#define VIEWOPT_2_KEEPASPECTRATIO   0x00020000L
#define VIEWOPT_2_GRFKEEPZOOM       0x00040000L
#define VIEWOPT_2_PREVENT_TIPS      0x00100000L
#define VIEWOPT_2_RESERVED3         0x00200000L
#define VIEWOPT_2_RESERVED4         0x00400000L
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

class SwViewOption
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

    OUString   sSymbolFont;        // Symbolfont.
    sal_uInt32      nCoreOptions;       // Bits for ViewShell.
    sal_uInt32      nCore2Options;      // Bits for ViewShell.
    sal_uInt32      nUIOptions;         // UI-Bits
    Color           aRetoucheColor;     // DefaultBackground for BrowseView
    Size            aSnapSize;          // Describes horizontal and vertical snap.
    sal_uInt16          mnViewLayoutColumns;// # columns for edit view
    short           nDivisionX;         // Grid division.
    short           nDivisionY;
    sal_uInt8           nPagePrevRow;       // Page Preview Row/Columns.
    sal_uInt8           nPagePrevCol;       // Page Preview Row/Columns.
    sal_uInt8           nShdwCrsrFillMode;  // FillMode for ShadowCrsr.
    sal_Bool            bReadonly : 1;      // Readonly-Doc.
    sal_Bool            bStarOneSetting : 1;// Prevent from UI automatics (no scrollbars in readonly documents).
    sal_Bool            bIsPagePreview : 1; // The preview mustn't print field/footnote/... shadings.
    sal_Bool            bSelectionInReadonly : 1; // Determines whether selection is switched on in readonly documents.
    sal_Bool            mbFormView : 1;
    sal_Bool            mbBrowseMode : 1;    // swmod 080130.
    sal_Bool            mbBookView : 1;      // View mode for page preview.
    sal_Bool            mbViewLayoutBookMode : 1; // Book view mode for edit view.
    sal_Bool        bShowPlaceHolderFields : 1; // Only used in printing!
    mutable bool    bIdle;

    // Scale
    sal_uInt16          nZoom;          // In percent.
    SvxZoomType     eZoom;              // 'enum' for zoom.

    sal_uInt8            nTblDest;      // Destination for table background.

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

    void   Init( Window *pWin );        // Initializing of static data.

    sal_uInt16 GetPixelTwips() const { return nPixelTwips; }

    inline sal_uInt32   GetCoreOptions() const {return nCoreOptions;}
    inline void     SetUIOptions( const SwViewOption& );

/*---------------------------------------------------------------------------
    Options from nCoreOptions
----------------------------------------------------------------------------*/

    inline sal_Bool IsIdle() const
        { return bIdle; }

    // Logically this is a const function since it does not modify the viewoptions
    // but only effects idle formatting. Of course that member is already implement
    // in the wrong place here... Also currently there are many const modifying casts in the code
    // just to call this function on otherwise const objects. Thus declaring it as const now.
    inline void SetIdle( sal_Bool b ) const
        { bIdle = b; }

    inline sal_Bool IsTab(sal_Bool bHard = sal_False) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_TAB) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? sal_True : sal_False; }
    inline void SetTab( sal_Bool b )        {
        (b != 0) ? (nCoreOptions |= VIEWOPT_1_TAB ) : ( nCoreOptions &= ~VIEWOPT_1_TAB); }

    inline sal_Bool IsBlank(sal_Bool bHard = sal_False) const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_BLANK) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? sal_True : sal_False; }
    inline void SetBlank( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_BLANK ) : ( nCoreOptions &= ~VIEWOPT_1_BLANK); }

    inline sal_Bool IsHardBlank() const
                    { return !bReadonly && nCoreOptions & VIEWOPT_1_HARDBLANK ? sal_True : sal_False; }
    inline void SetHardBlank( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_HARDBLANK ) : ( nCoreOptions &= ~VIEWOPT_1_HARDBLANK); }

    inline sal_Bool IsParagraph(sal_Bool bHard = sal_False) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_PARAGRAPH) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? sal_True : sal_False; }
    inline void SetParagraph( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_PARAGRAPH ) : ( nCoreOptions &= ~VIEWOPT_1_PARAGRAPH); }

    inline sal_Bool IsLineBreak(sal_Bool bHard = sal_False) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_LINEBREAK) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? sal_True : sal_False; }
    inline void SetLineBreak( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_LINEBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_LINEBREAK); }

    inline void SetPageBreak( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_PAGEBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_PAGEBREAK); }

    inline void SetColumnBreak( sal_Bool b)
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_COLUMNBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_COLUMNBREAK); }

    inline sal_Bool IsSoftHyph() const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_SOFTHYPH) ? sal_True : sal_False; }
    inline void SetSoftHyph( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SOFTHYPH ) : ( nCoreOptions &= ~VIEWOPT_1_SOFTHYPH); }

    inline sal_Bool IsFldName() const       { return !bReadonly && (nCoreOptions & VIEWOPT_1_FLDNAME) ? sal_True : sal_False; }
    inline void SetFldName( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_FLDNAME ) : ( nCoreOptions &= ~VIEWOPT_1_FLDNAME); }

    inline sal_Bool IsPostIts() const
        { return (nCoreOptions & VIEWOPT_1_POSTITS) ? sal_True : sal_False; }
    inline void SetPostIts( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_POSTITS ) : ( nCoreOptions &= ~VIEWOPT_1_POSTITS); }
           void PaintPostIts( OutputDevice *pOut, const SwRect &rRect,
                              sal_Bool bIsScript ) const;
           sal_uInt16 GetPostItsWidth( const OutputDevice *pOut = 0 ) const;

    inline sal_Bool IsShowHiddenChar(sal_Bool bHard = sal_False) const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_CHAR_HIDDEN) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? sal_True : sal_False; }

    inline void SetShowHiddenChar( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_CHAR_HIDDEN ) : ( nCoreOptions &= ~VIEWOPT_1_CHAR_HIDDEN); }


    inline sal_Bool IsShowHiddenField() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_FLD_HIDDEN) ? sal_True : sal_False; }
    inline void SetShowHiddenField( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_FLD_HIDDEN ) : ( nCoreOptions &= ~VIEWOPT_1_FLD_HIDDEN); }

    inline sal_Bool IsGraphic() const
        { return nCoreOptions & VIEWOPT_1_GRAPHIC ? sal_True : sal_False; }
    inline void SetGraphic( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_GRAPHIC ) : ( nCoreOptions &= ~VIEWOPT_1_GRAPHIC); }

    inline sal_Bool IsPageBack() const
        { return nCoreOptions & VIEWOPT_1_PAGEBACK ? sal_True : sal_False; }
    inline void SetPageBack( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_PAGEBACK) : ( nCoreOptions &= ~VIEWOPT_1_PAGEBACK); }

    inline sal_Bool IsTable() const
        { return nCoreOptions & VIEWOPT_1_TABLE ? sal_True : sal_False; }
    inline void SetTable( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_TABLE ) : ( nCoreOptions &= ~VIEWOPT_1_TABLE); }

    inline sal_Bool IsDraw() const
        { return nCoreOptions & VIEWOPT_1_DRAW ? sal_True : sal_False; }
    inline void SetDraw( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_DRAW ) : ( nCoreOptions &= ~VIEWOPT_1_DRAW); }

    inline sal_Bool IsControl() const
        { return nCoreOptions & VIEWOPT_1_CONTROL ? sal_True : sal_False; }
    inline void SetControl( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_CONTROL ) : ( nCoreOptions &= ~VIEWOPT_1_CONTROL); }

    inline sal_Bool IsSnap() const
        { return nCoreOptions & VIEWOPT_1_SNAP ? sal_True : sal_False; }
    inline void SetSnap( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SNAP ) : ( nCoreOptions &= ~VIEWOPT_1_SNAP); }

    inline void SetSnapSize( Size &rSz ){ aSnapSize = rSz; }
    inline const Size &GetSnapSize() const { return aSnapSize; }

    inline sal_Bool IsGridVisible() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_GRIDVISIBLE) ? sal_True : sal_False; }
    inline void SetGridVisible( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_GRIDVISIBLE ) : ( nCoreOptions &= ~VIEWOPT_1_GRIDVISIBLE); }

    inline sal_Bool IsOnlineSpell() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_ONLINESPELL) ? sal_True : sal_False; }
    inline void SetOnlineSpell( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_ONLINESPELL ) : ( nCoreOptions &= ~VIEWOPT_1_ONLINESPELL); }

    inline sal_Bool IsViewMetaChars() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_VIEWMETACHARS) ? sal_True : sal_False; }
    inline void SetViewMetaChars( sal_Bool b)
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_VIEWMETACHARS ) : ( nCoreOptions &= ~VIEWOPT_1_VIEWMETACHARS); }

    inline sal_Bool IsSynchronize() const
        {  return nCoreOptions & VIEWOPT_1_SYNCHRONIZE ? sal_True : sal_False;}
    inline void SetSynchronize( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SYNCHRONIZE ) : ( nCoreOptions &= ~VIEWOPT_1_SYNCHRONIZE); }

    inline sal_Bool IsCrossHair() const
        { return nCoreOptions & VIEWOPT_1_CROSSHAIR ? sal_True : sal_False; }
    inline void SetCrossHair( sal_Bool b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_CROSSHAIR ) : ( nCoreOptions &= ~VIEWOPT_1_CROSSHAIR); }

/*---------------------------------------------------------------------------
    Options from nCore2Options
----------------------------------------------------------------------------*/
    inline sal_Bool IsBlackFont() const
        {return nCore2Options & VIEWOPT_CORE2_BLACKFONT ? sal_True : sal_False;}

    inline void SetBlackFont(sal_Bool b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_BLACKFONT) : (nCore2Options &= ~VIEWOPT_CORE2_BLACKFONT);}

    inline sal_Bool IsShowHiddenPara() const
        {return nCore2Options & VIEWOPT_CORE2_HIDDENPARA ? sal_True : sal_False;}

    inline void SetShowHiddenPara(sal_Bool b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_HIDDENPARA) : (nCore2Options &= ~VIEWOPT_CORE2_HIDDENPARA);}

    inline sal_Bool IsSmoothScroll() const
        {return nCore2Options & VIEWOPT_CORE2_SMOOTHSCROLL ? sal_True : sal_False;}

    inline void SetSmoothScroll(sal_Bool b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_SMOOTHSCROLL) : (nCore2Options &= ~VIEWOPT_CORE2_SMOOTHSCROLL);}

    inline sal_Bool IsCursorInProtectedArea() const
        {return nCore2Options & VIEWOPT_CORE2_CRSR_IN_PROT ? sal_True : sal_False;}

    inline void SetCursorInProtectedArea(sal_Bool b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_CRSR_IN_PROT) : (nCore2Options &= ~VIEWOPT_CORE2_CRSR_IN_PROT);}

    inline sal_Bool IsPDFExport() const
        {return nCore2Options & VIEWOPT_CORE2_PDF_EXPORT ? sal_True : sal_False;}

    inline void SetPDFExport(sal_Bool b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_PDF_EXPORT) : (nCore2Options &= ~VIEWOPT_CORE2_PDF_EXPORT);}

    inline sal_Bool IsPrinting() const
        {return nCore2Options & VIEWOPT_CORE2_PRINTING ? sal_True : sal_False;}

    inline void SetPrinting(sal_Bool b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_PRINTING) : (nCore2Options &= ~VIEWOPT_CORE2_PRINTING);}

    inline short GetDivisionX() const   { return nDivisionX; }
    inline void  SetDivisionX( short n ){ nDivisionX = n; }
    inline short GetDivisionY() const   { return nDivisionY; }
    inline void  SetDivisionY( short n ){ nDivisionY = n; }

    inline sal_uInt8  GetPagePrevRow() const      { return nPagePrevRow; }
    inline void  SetPagePrevRow( sal_uInt8 n ) { nPagePrevRow = n; }
    inline sal_uInt8  GetPagePrevCol() const      { return nPagePrevCol; }
    inline void  SetPagePrevCol( sal_uInt8 n ) { nPagePrevCol = n; }
    sal_Bool         IsReadonly() const { return bReadonly; }
    void         SetReadonly(sal_Bool bSet) { bReadonly = bSet; }

    sal_Bool         IsSelectionInReadonly() const {return bSelectionInReadonly;}
    void         SetSelectionInReadonly(sal_Bool bSet) {bSelectionInReadonly = bSet;}

    sal_Bool         IsFormView() const { return mbFormView; }
    void         SetFormView( sal_Bool bSet ) { mbFormView = bSet; }

    inline sal_Bool  getBrowseMode() const { return mbBrowseMode; }
    inline void  setBrowseMode(sal_Bool bSet) { mbBrowseMode = bSet; }
    inline sal_Bool  IsPagePrevBookview() const { return mbBookView; }
    inline void  SetPagePrevBookview(sal_Bool bSet) { mbBookView = bSet; }

    sal_Bool IsAutoCompleteWords() const;

    bool   IsViewLayoutBookMode() const { return mbViewLayoutBookMode; }
    void   SetViewLayoutBookMode( bool bNew ) { mbViewLayoutBookMode = bNew; }
    sal_uInt16 GetViewLayoutColumns() const { return mnViewLayoutColumns; }
    void   SetViewLayoutColumns( sal_uInt16 nNew ) { mnViewLayoutColumns = nNew; }

#ifdef DBG_UTIL
    // Correspond to statements in ui/config/cfgvw.src.
    inline bool IsTest1() const     { return m_bTest1; }
    inline void SetTest1( bool b )  { m_bTest1 = b; }
    inline bool IsTest2() const     { return m_bTest2; }
    inline void SetTest2( bool b )  { m_bTest2 = b; }
    inline bool IsTest3() const     { return m_bTest3; }
    inline void SetTest3( bool b )  { m_bTest3 = b; }
    inline bool IsTest4() const     { return m_bTest4; }
    inline void SetTest4( bool b )  { m_bTest4 = b; }
    inline bool IsTest5() const     { return m_bTest5; }
    inline void SetTest5( bool b )  { m_bTest5 = b; }
    inline bool IsTest6() const     { return m_bTest6; }
    inline void SetTest6( bool b )  { m_bTest6 = b; }
    inline bool IsTest7() const     { return m_bTest7; }
    inline void SetTest7( bool b )  { m_bTest7 = b; }
    inline bool IsTest8() const     { return m_bTest8; }
    inline void SetTest8( bool b )  { m_bTest8 = b; }
    static inline bool IsTest9()            { return s_bTest9; }
    static inline void SetTest9( bool b )   { s_bTest9 = b; }
    inline bool IsTest10() const    { return m_bTest10; }
    inline void SetTest10( bool b ) { m_bTest10 = b; }
#endif

    inline sal_uInt16 GetZoom() const    { return nZoom; }
    inline void   SetZoom( sal_uInt16 n ){ nZoom = n; }

    void DrawRect( OutputDevice* pOut, const SwRect &rRect, long nCol ) const;
    void DrawRectPrinter( OutputDevice* pOut, const SwRect& rRect ) const;

    SwViewOption& operator=( const SwViewOption &rOpt );

    // Compare methods.
    bool IsEqualFlags ( const SwViewOption &rOpt ) const;
    inline bool operator == ( const SwViewOption &rOpt ) const;
    inline bool operator != ( const SwViewOption &rOpt ) const  { return !(*this == rOpt); }


/*---------------------------------------------------------------------------
    Options from nUIOptions
----------------------------------------------------------------------------*/

    sal_Bool    IsViewVScrollBar() const
        {
#if HAVE_FEATURE_DESKTOP
            return nUIOptions & VIEWOPT_2_VSCROLLBAR ? sal_True : sal_False;
#else
            return sal_False;
#endif
        }
    sal_Bool    IsViewHScrollBar() const
        {
#if HAVE_FEATURE_DESKTOP
            return nUIOptions & VIEWOPT_2_HSCROLLBAR ? sal_True : sal_False;
#else
            return sal_False;
#endif
        }
    sal_Bool    IsKeepRatio()      const
        { return nUIOptions & VIEWOPT_2_KEEPASPECTRATIO ? sal_True : sal_False;   }
    sal_Bool    IsGrfKeepZoom()    const
        { return nUIOptions & VIEWOPT_2_GRFKEEPZOOM ? sal_True : sal_False;   }
    sal_Bool    IsPreventTips() const
        { return nUIOptions & VIEWOPT_2_PREVENT_TIPS ? sal_True : sal_False; }
    sal_Bool    IsPrtFormat() const
        { return nUIOptions & VIEWOPT_2_PRTFORMAT ? sal_True : sal_False; }

    SvxZoomType    GetZoomType()      const { return eZoom; }

    sal_uInt8   GetTblDest() const    { return nTblDest; }

    void   SetViewVScrollBar(sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_VSCROLLBAR ) : ( nUIOptions &= ~VIEWOPT_2_VSCROLLBAR); }
    void   SetViewHScrollBar(sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_HSCROLLBAR ) : ( nUIOptions &= ~VIEWOPT_2_HSCROLLBAR); }
    void   SetKeepRatio     (sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_KEEPASPECTRATIO ) : ( nUIOptions &= ~VIEWOPT_2_KEEPASPECTRATIO); }
    void   SetGrfKeepZoom   (sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_GRFKEEPZOOM ) : ( nUIOptions &= ~VIEWOPT_2_GRFKEEPZOOM); }
    void SetPreventTips( sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_PREVENT_TIPS) : (nUIOptions &= ~VIEWOPT_2_PREVENT_TIPS); }
    void SetPrtFormat( sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_PRTFORMAT) : (nUIOptions &= ~VIEWOPT_2_PRTFORMAT); }

    void            SetZoomType     (SvxZoomType eZoom_){ eZoom = eZoom_;  }
    void            SetTblDest( sal_uInt8 nNew )    { nTblDest = nNew;  }

    const OUString& GetSymbolFont() const {return sSymbolFont;}
    void            SetSymbolFont(const OUString& sSet) {sSymbolFont = sSet;}

    const Color&    GetRetoucheColor() const        { return aRetoucheColor;}
    void            SetRetoucheColor(const Color&r) { aRetoucheColor = r;   }

    sal_Bool        IsViewAnyRuler() const
        {
#if HAVE_FEATURE_DESKTOP
            return 0 != (nUIOptions & VIEWOPT_2_ANY_RULER);
#else
            return sal_False;
#endif
        }
    void            SetViewAnyRuler(sal_Bool bSet)
                        { bSet ? (nUIOptions |= VIEWOPT_2_ANY_RULER) : (nUIOptions &= ~VIEWOPT_2_ANY_RULER);}

    sal_Bool        IsViewHRuler(sal_Bool bDirect = sal_False)     const
                        {
#if HAVE_FEATURE_DESKTOP
                            sal_Bool bRet = sal::static_int_cast< sal_Bool >( bDirect  ?
                                    0 != (nUIOptions & VIEWOPT_2_H_RULER) :
                                    !bReadonly ?
                                        (nUIOptions & (VIEWOPT_2_ANY_RULER|VIEWOPT_2_H_RULER)) == (VIEWOPT_2_ANY_RULER|VIEWOPT_2_H_RULER)
                                        : sal_False );
                            return bRet;
#else
                            (void) bDirect;
                            return sal_False;
#endif
                        }
    void            SetViewHRuler   (sal_Bool b)
                        {    b ? (nUIOptions |= VIEWOPT_2_H_RULER ) : ( nUIOptions &= ~VIEWOPT_2_H_RULER);}

    sal_Bool            IsViewVRuler(sal_Bool bDirect = sal_False) const
                        {
#if HAVE_FEATURE_DESKTOP
                            sal_Bool bRet = sal::static_int_cast< sal_Bool >( bDirect  ?
                                    0 !=(nUIOptions & VIEWOPT_2_V_RULER) :
                                    !bReadonly ?
                                        (nUIOptions &
                                            (VIEWOPT_2_ANY_RULER|VIEWOPT_2_V_RULER)) == (VIEWOPT_2_ANY_RULER|VIEWOPT_2_V_RULER)
                                        : sal_False );
                            return bRet;
#else
                            (void) bDirect;
                            return sal_False;
#endif
                        }
    void            SetViewVRuler     (sal_Bool b)
                        { b ? (nUIOptions |= VIEWOPT_2_V_RULER ) : ( nUIOptions &= ~VIEWOPT_2_V_RULER);}

    // ShadowCursor, switch on/off, get/set color/mode.
    sal_Bool    IsShadowCursor()    const
        { return nUIOptions & VIEWOPT_2_SHADOWCRSR ? sal_True : sal_False;   }
    void   SetShadowCursor(sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_SHADOWCRSR ) : ( nUIOptions &= ~VIEWOPT_2_SHADOWCRSR); }

    //move vertical ruler to the right
    sal_Bool    IsVRulerRight()    const
        { return nUIOptions & VIEWOPT_2_V_RULER_RIGHT ? sal_True : sal_False;   }
    void   SetVRulerRight(sal_Bool b)
        { b ? (nUIOptions |= VIEWOPT_2_V_RULER_RIGHT ) : ( nUIOptions &= ~VIEWOPT_2_V_RULER_RIGHT); }

    sal_Bool            IsStarOneSetting() const {return bStarOneSetting; }
    void            SetStarOneSetting(sal_Bool bSet) {bStarOneSetting = bSet; }

    sal_Bool            IsPagePreview() const {return bIsPagePreview; }
    void            SetPagePreview(sal_Bool bSet) { bIsPagePreview= bSet; }

    sal_uInt8           GetShdwCrsrFillMode() const { return nShdwCrsrFillMode; }
    void            SetShdwCrsrFillMode( sal_uInt8 nMode ) { nShdwCrsrFillMode = nMode; };

    sal_Bool        IsShowPlaceHolderFields() const { return bShowPlaceHolderFields; }
    void            SetShowPlaceHolderFields(sal_Bool bSet) { bShowPlaceHolderFields = bSet; }

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
    SW_DLLPUBLIC static Color&   GetFontColor();
    static Color&   GetFieldShadingsColor();
    static Color&   GetSectionBoundColor();
    static Color&   GetPageBreakColor();
    static Color&   GetHeaderFooterMarkColor();

    static sal_Bool     IsAppearanceFlag(sal_Int32 nFlag);

    static sal_Bool     IsDocBoundaries()   {return IsAppearanceFlag(VIEWOPT_DOC_BOUNDARIES);}
    static sal_Bool     IsObjectBoundaries(){return IsAppearanceFlag(VIEWOPT_OBJECT_BOUNDARIES);}
    static sal_Bool     IsTableBoundaries() {return IsAppearanceFlag(VIEWOPT_TABLE_BOUNDARIES );}
    static sal_Bool     IsIndexShadings()   {return IsAppearanceFlag(VIEWOPT_INDEX_SHADINGS   );}
    static sal_Bool     IsLinks()           {return IsAppearanceFlag(VIEWOPT_LINKS            );}
    static sal_Bool     IsVisitedLinks()    {return IsAppearanceFlag(VIEWOPT_VISITED_LINKS    );}
    static sal_Bool     IsFieldShadings()   {return IsAppearanceFlag(VIEWOPT_FIELD_SHADINGS);}
    static sal_Bool     IsSectionBoundaries() {return IsAppearanceFlag(VIEWOPT_SECTION_BOUNDARIES);}
    static sal_Bool     IsShadow()          {return IsAppearanceFlag(VIEWOPT_SHADOW           );}

    static void     SetAppearanceFlag(sal_Int32 nFlag, sal_Bool bSet, sal_Bool bSaveInConfig = sal_False);

    void    SetDocBoundaries(sal_Bool bSet)   {SetAppearanceFlag(VIEWOPT_DOC_BOUNDARIES, bSet);}

    static void     ApplyColorConfigValues(const svtools::ColorConfig& rConfig);
};


inline bool SwViewOption::operator==( const SwViewOption &rOpt ) const
{
    return IsEqualFlags( rOpt ) && nZoom == rOpt.GetZoom();
}

inline void SwViewOption::SetUIOptions( const SwViewOption& rVOpt )
{
    nUIOptions = rVOpt.nUIOptions;
    nTblDest = rVOpt.nTblDest;
    nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
}


// Helper function for checking HTML-capabilities.
SW_DLLPUBLIC sal_uInt16     GetHtmlMode(const SwDocShell*);


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
