/*************************************************************************
 *
 *  $RCSfile: viewopt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-09-28 15:21:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _VIEWOPT_HXX
#define _VIEWOPT_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SV_COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif
#include "authratr.hxx"

class SwRect;
class Window;
class OutputDevice;
class ViewShell;
class SwDocShell;

#define VIEWOPT_1_IDLE          0x00000001L
#define VIEWOPT_1_TAB           0x00000002L
#define VIEWOPT_1_BLANK         0x00000004L
#define VIEWOPT_1_HARDBLANK     0x00000008L
#define VIEWOPT_1_PARAGRAPH     0x00000010L
#define VIEWOPT_1_LINEBREAK     0x00000020L
#define VIEWOPT_1_PAGEBREAK     0x00000040L
#define VIEWOPT_1_COLUMNBREAK   0x00000080L
#define VIEWOPT_1_SOFTHYPH      0x00000100L
#define VIEWOPT_1_TOX           0x00000200L
#define VIEWOPT_1_REF           0x00000400L
#define VIEWOPT_1_FLDNAME       0x00000800L
#define VIEWOPT_1_FIELD         0x00001000L
#define VIEWOPT_1_FOOTNOTE      0x00002000L
#define VIEWOPT_1_POSTITS       0x00004000L
#define VIEWOPT_1_HIDDEN        0x00008000L
#define VIEWOPT_1_SUBSLINES     0x00010000L
#define VIEWOPT_1_GRAPHIC       0x00020000L
#define VIEWOPT_1_TABLE         0x00040000L
#define VIEWOPT_1_DRAW          0x00080000L
#define VIEWOPT_1_CONTROL       0x00100000L
#define VIEWOPT_1_SUBSTABLE     0x00200000L
#define VIEWOPT_1_CROSSHAIR     0x00400000L
#define VIEWOPT_1_SNAP          0x00800000L
#define VIEWOPT_1_SYNCHRONIZE   0x01000000L
#define VIEWOPT_1_GRIDVISIBLE   0x02000000L
#define VIEWOPT_1_ONLINESPELL   0x04000000L
#define VIEWOPT_1_HIDESPELL     0x08000000L
#define VIEWOPT_1_RESERVED1     0x10000000L
#define VIEWOPT_1_VIEWMETACHARS 0x20000000L
#define VIEWOPT_1_PAGEBACK      0x40000000L
#define VIEWOPT_1_SOLIDMARKHDL  0x80000000L

#define VIEWOPT_CORE2_BLACKFONT         0x00000001L
#define VIEWOPT_CORE2_HIDDENPARA        0x00000002L
#define VIEWOPT_CORE2_SMOOTHSCROLL      0x00000004L
#define VIEWOPT_CORE2_CRSR_IN_PROT      0x00000008L
#define VIEWOPT_CORE2_SECTION_BOUNDS    0x00000010L
#define VIEWOPT_CORE2_INDEX_BACKGROUND  0x00000020L
#define VIEWOPT_CORE2_BIGMARKHDL        0x00000040L

//#define VIEWOPT_2_TOOLBOX         0x00000001L
//#define VIEWOPT_2_TOOLBOXTEXT     0x00000002L
//#define VIEWOPT_2_TOOLBOXBITMAP       0x00000004L
//#define VIEWOPT_2_TOOLBOXBOTH     0x00000008L
//#define VIEWOPT_2_TOOLBOXLEFT     0x00000010L
//#define VIEWOPT_2_TOOLBOXRIGHT      0x00000020L
//#define VIEWOPT_2_TOOLBOXTOP      0x00000040L
//#define VIEWOPT_2_TOOLBOXBOTTOM       0x00000080L
#define VIEWOPT_2_UNUSED1           0x00000100L
//#define VIEWOPT_2_RIBBONBAR           0x00000200L
#define VIEWOPT_2_TABWIN            0x00000400L
#define VIEWOPT_2_VSCROLLBAR        0x00000800L
#define VIEWOPT_2_HSCROLLBAR        0x00001000L
#define VIEWOPT_2_STATUSLINE        0x00002000L
#define VIEWOPT_2_VLIN              0x00004000L
#define VIEWOPT_2_SVLOOK            0x00008000L
#define VIEWOPT_2_MODIFIED          0x00010000L
#define VIEWOPT_2_KEEPASPECTRATIO   0x00020000L
#define VIEWOPT_2_GRFKEEPZOOM       0x00040000L
#define VIEWOPT_2_EXECHYPERLINKS    0x00080000L
#define VIEWOPT_2_RESERVED2         0x00100000L
#define VIEWOPT_2_RESERVED3         0x00200000L
#define VIEWOPT_2_RESERVED4         0x00400000L
#define VIEWOPT_2_PRTFORMAT         0x00800000L
#define VIEWOPT_2_SHADOWCRSR        0x01000000L

//Tabellenhintergrund
#define TBL_DEST_CELL   0
#define TBL_DEST_ROW    1
#define TBL_DEST_TBL    2

// Implementierung in core/text/txtpaint.cxx
extern void SyncVout( const OutputDevice *pOut );

class SwViewOption
{
protected:
    static USHORT   nPixelTwips;// 1 Pixel == ? Twips

    String          sSymbolFont;        // Symbolzeichensatz
    UINT32          nCoreOptions;       // Bits fuer die ViewShell
    UINT32          nCore2Options;      // Bits fuer die ViewShell
    UINT32          nUIOptions;         // UI-Bits
    Color           aRetoucheColor;     // DefaultBackground fuer BrowseView
    Color           aShdwCrsrCol;       // Farbe fuer den ShadowCrsr
    Color           aIdxBackgrndCol;    // color of index background
    Size            aSnapSize;          // Beschreibt hori. wie vert. Snap
    short           nDivisionX;         // Rasterunterteilung
    short           nDivisionY;
    BYTE            nPagePrevRow;       // Page Preview Row/Columns
    BYTE            nPagePrevCol;       // Page Preview Row/Columns
    BYTE            nShdwCrsrFillMode;  // FillMode fuer den ShadowCrsr
    BOOL            bReadonly : 1;      // Readonly-Doc
    BOOL            bStarOneSetting : 1;// prevent from UI automatics (no scrollbars in readonly documents)

    // Maszstab
    USHORT          nZoom;              // Angaben in Prozent
    BYTE            eZoom;              // 'enum' fuer Zoom
    BYTE            nTblDest;           // Ziel fuer Tabellenhintergrund

#ifndef PRODUCT
    // korrespondieren zu den Angaben in ui/config/cfgvw.src
    BOOL  bTest1        :1;     // Test-Flag  "Layout not loading"
    BOOL  bTest2        :1;     // Test-Flag  "WYSIWYG++"
    BOOL  bTest3        :1;     // Test-Flag  ""
    BOOL  bTest4        :1;     // Test-Flag  "WYSIWYG debug"
    BOOL  bTest5        :1;     // Test-Flag  "No idle format"
    BOOL  bTest6        :1;     // Test-Flag  "No screen adj"
    BOOL  bTest7        :1;     // Test-Flag  "win format"
    BOOL  bTest8        :1;     // Test-Flag  ""
    static BOOL  bTest9;    // Test-Flag  "DrawingLayerNotLoading"
    BOOL  bTest10       :1;     // Test-Flag  "Format by Input"
#endif

public:
            SwViewOption();                     // CTOR
            SwViewOption(const SwViewOption&);  //
            ~SwViewOption();                    //

    void   Init( Window *pWin );        // Initialisierung der statischen Daten

    USHORT GetPixelTwips() const { return nPixelTwips; }

    inline UINT32   GetCoreOptions() const {return nCoreOptions;}
    inline UINT32   GetUIOptions()   const {return nUIOptions;}
    inline void     SetUIOptions( const SwViewOption& );

/*---------------------------------------------------------------------------
    Optionen aus nCoreOptions
----------------------------------------------------------------------------*/

    inline BOOL IsIdle() const
        { return nCoreOptions & VIEWOPT_1_IDLE ? TRUE : FALSE; }
    inline void SetIdle( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_IDLE ) : ( nCoreOptions &= ~VIEWOPT_1_IDLE); }

    inline BOOL IsTab(BOOL bHard = FALSE) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_TAB) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? TRUE : FALSE; }
    inline void SetTab( BOOL b )        {
        (b != 0) ? (nCoreOptions |= VIEWOPT_1_TAB ) : ( nCoreOptions &= ~VIEWOPT_1_TAB); }
           void PaintTab( OutputDevice *pOut, const SwRect &rRect ) const;


    inline BOOL IsBlank(BOOL bHard = FALSE) const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_BLANK) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? TRUE : FALSE; }
    inline void SetBlank( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_BLANK ) : ( nCoreOptions &= ~VIEWOPT_1_BLANK); }

    inline BOOL IsHardBlank() const
                    { return !bReadonly && nCoreOptions & VIEWOPT_1_HARDBLANK ? TRUE : FALSE; }
    inline void SetHardBlank( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_HARDBLANK ) : ( nCoreOptions &= ~VIEWOPT_1_HARDBLANK); }

    inline BOOL IsParagraph(BOOL bHard = FALSE) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_PARAGRAPH) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? TRUE : FALSE; }
    inline void SetParagraph( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_PARAGRAPH ) : ( nCoreOptions &= ~VIEWOPT_1_PARAGRAPH); }

    inline BOOL IsLineBreak(BOOL bHard = FALSE) const
                    {   return !bReadonly && (nCoreOptions & VIEWOPT_1_LINEBREAK) &&
                            ((nCoreOptions & VIEWOPT_1_VIEWMETACHARS)||bHard)
                                    ? TRUE : FALSE; }
    inline void SetLineBreak( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_LINEBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_LINEBREAK); }
        void PaintLineBreak( OutputDevice *pOut, const SwRect &rRect ) const;
    USHORT GetLineBreakWidth( const OutputDevice *pOut = 0 ) const;

    inline BOOL IsPageBreak() const     { return !bReadonly && (nCoreOptions & VIEWOPT_1_PAGEBREAK) ? TRUE : FALSE; }
    inline void SetPageBreak( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_PAGEBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_PAGEBREAK); }

    inline BOOL IsColumnBreak() const   { return !bReadonly && (nCoreOptions & VIEWOPT_1_COLUMNBREAK) ? TRUE : FALSE; }
    inline void SetColumnBreak( BOOL b)
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_COLUMNBREAK ) : ( nCoreOptions &= ~VIEWOPT_1_COLUMNBREAK); }

    inline BOOL IsSoftHyph() const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_SOFTHYPH) ? TRUE : FALSE; }
    inline void SetSoftHyph( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SOFTHYPH ) : ( nCoreOptions &= ~VIEWOPT_1_SOFTHYPH); }

    inline BOOL IsTox() const
                        { return !bReadonly && (nCoreOptions & VIEWOPT_1_TOX) ? TRUE : FALSE; }
    inline void SetTox( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_TOX ) : ( nCoreOptions &= ~VIEWOPT_1_TOX); }

    inline BOOL IsRef() const           { return !bReadonly && (nCoreOptions & VIEWOPT_1_REF) ? TRUE : FALSE; }
    inline void SetRef( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_REF ) : ( nCoreOptions &= ~VIEWOPT_1_REF); }

    inline BOOL IsFldName() const       { return !bReadonly && (nCoreOptions & VIEWOPT_1_FLDNAME) ? TRUE : FALSE; }
    inline void SetFldName( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_FLDNAME ) : ( nCoreOptions &= ~VIEWOPT_1_FLDNAME); }

    inline BOOL IsField() const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_FIELD) ? TRUE : FALSE; }
    inline void SetField( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_FIELD ) : ( nCoreOptions &= ~VIEWOPT_1_FIELD); }

    inline BOOL IsFootNote() const
                    { return !bReadonly && (nCoreOptions & VIEWOPT_1_FOOTNOTE) ? TRUE : FALSE; }
    inline void SetFootNote( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_FOOTNOTE) : ( nCoreOptions &= ~VIEWOPT_1_FOOTNOTE); }

    inline BOOL IsPostIts() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_POSTITS) ? TRUE : FALSE; }
    inline void SetPostIts( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_POSTITS ) : ( nCoreOptions &= ~VIEWOPT_1_POSTITS); }
           void PaintPostIts( OutputDevice *pOut, const SwRect &rRect,
                              long nCol ) const;
           USHORT GetPostItsWidth( const OutputDevice *pOut = 0 ) const;

    inline BOOL IsHidden() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_HIDDEN) ? TRUE : FALSE; }
    inline void SetHidden( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_HIDDEN ) : ( nCoreOptions &= ~VIEWOPT_1_HIDDEN); }

    inline BOOL IsSubsLines() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_SUBSLINES) ? TRUE : FALSE; }
    inline void SetSubsLines( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SUBSLINES ) : ( nCoreOptions &= ~VIEWOPT_1_SUBSLINES); }

    inline BOOL IsGraphic() const
        { return nCoreOptions & VIEWOPT_1_GRAPHIC ? TRUE : FALSE; }
    inline void SetGraphic( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_GRAPHIC ) : ( nCoreOptions &= ~VIEWOPT_1_GRAPHIC); }

    inline BOOL IsPageBack() const
        { return nCoreOptions & VIEWOPT_1_PAGEBACK ? TRUE : FALSE; }
    inline void SetPageBack( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_PAGEBACK) : ( nCoreOptions &= ~VIEWOPT_1_PAGEBACK); }

    inline BOOL IsSolidMarkHdl() const
        { return nCoreOptions & VIEWOPT_1_SOLIDMARKHDL ? TRUE : FALSE; }
    inline void SetSolidMarkHdl( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SOLIDMARKHDL) : ( nCoreOptions &= ~VIEWOPT_1_SOLIDMARKHDL); }

    inline BOOL IsBigMarkHdl() const
        { return nCore2Options & VIEWOPT_CORE2_BIGMARKHDL ? TRUE : FALSE;}
    inline void SetBigMarkHdl(BOOL b)
        { (b != 0) ? (nCore2Options |=  VIEWOPT_CORE2_BIGMARKHDL ) : (nCore2Options &=  ~VIEWOPT_CORE2_BIGMARKHDL);}

    inline BOOL IsTable() const
        { return nCoreOptions & VIEWOPT_1_TABLE ? TRUE : FALSE; }
    inline void SetTable( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_TABLE ) : ( nCoreOptions &= ~VIEWOPT_1_TABLE); }

    inline BOOL IsDraw() const
        { return nCoreOptions & VIEWOPT_1_DRAW ? TRUE : FALSE; }
    inline void SetDraw( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_DRAW ) : ( nCoreOptions &= ~VIEWOPT_1_DRAW); }

    inline BOOL IsControl() const
        { return nCoreOptions & VIEWOPT_1_CONTROL ? TRUE : FALSE; }
    inline void SetControl( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_CONTROL ) : ( nCoreOptions &= ~VIEWOPT_1_CONTROL); }

    inline BOOL IsSubsTable() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_SUBSTABLE) ? TRUE : FALSE; }
    inline void SetSubsTable( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SUBSTABLE ) : ( nCoreOptions &= ~VIEWOPT_1_SUBSTABLE); }

    inline BOOL IsSnap() const
        { return nCoreOptions & VIEWOPT_1_SNAP ? TRUE : FALSE; }
    inline void SetSnap( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SNAP ) : ( nCoreOptions &= ~VIEWOPT_1_SNAP); }

    inline void SetSnapSize( Size &rSz ){ aSnapSize = rSz; }
    inline const Size &GetSnapSize() const { return aSnapSize; }

    inline BOOL IsGridVisible() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_GRIDVISIBLE) ? TRUE : FALSE; }
    inline void SetGridVisible( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_GRIDVISIBLE ) : ( nCoreOptions &= ~VIEWOPT_1_GRIDVISIBLE); }

    inline BOOL IsOnlineSpell() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_ONLINESPELL) ? TRUE : FALSE; }
    inline void SetOnlineSpell( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_ONLINESPELL ) : ( nCoreOptions &= ~VIEWOPT_1_ONLINESPELL); }

    inline BOOL IsHideSpell() const
        { return nCoreOptions & VIEWOPT_1_HIDESPELL ? TRUE : FALSE; }
    inline void SetHideSpell( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_HIDESPELL ) : ( nCoreOptions &= ~VIEWOPT_1_HIDESPELL); }

    inline BOOL IsViewMetaChars() const
        { return !bReadonly && (nCoreOptions & VIEWOPT_1_VIEWMETACHARS) ? TRUE : FALSE; }
    inline void SetViewMetaChars( BOOL b)
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_VIEWMETACHARS ) : ( nCoreOptions &= ~VIEWOPT_1_VIEWMETACHARS); }

    inline BOOL IsSynchronize() const
        {  return nCoreOptions & VIEWOPT_1_SYNCHRONIZE ? TRUE : FALSE;}
    inline void SetSynchronize( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_SYNCHRONIZE ) : ( nCoreOptions &= ~VIEWOPT_1_SYNCHRONIZE); }

    inline BOOL IsCrossHair() const
        { return nCoreOptions & VIEWOPT_1_CROSSHAIR ? TRUE : FALSE; }
    inline void SetCrossHair( BOOL b )
        { (b != 0) ? (nCoreOptions |= VIEWOPT_1_CROSSHAIR ) : ( nCoreOptions &= ~VIEWOPT_1_CROSSHAIR); }

/*---------------------------------------------------------------------------
    Optionen aus nCore2Options
----------------------------------------------------------------------------*/
    inline BOOL IsBlackFont() const
        {return nCore2Options & VIEWOPT_CORE2_BLACKFONT ? TRUE : FALSE;}

    inline void SetBlackFont(BOOL b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_BLACKFONT) : (nCore2Options &= ~VIEWOPT_CORE2_BLACKFONT);}

    inline BOOL IsShowHiddenPara() const
        {return nCore2Options & VIEWOPT_CORE2_HIDDENPARA ? TRUE : FALSE;}

    inline void SetShowHiddenPara(BOOL b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_HIDDENPARA) : (nCore2Options &= ~VIEWOPT_CORE2_HIDDENPARA);}

    inline BOOL IsSmoothScroll() const
        {return nCore2Options & VIEWOPT_CORE2_SMOOTHSCROLL ? TRUE : FALSE;}

    inline void SetSmoothScroll(BOOL b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_SMOOTHSCROLL) : (nCore2Options &= ~VIEWOPT_CORE2_SMOOTHSCROLL);}

    inline BOOL IsCursorInProtectedArea() const
        {return nCore2Options & VIEWOPT_CORE2_CRSR_IN_PROT ? TRUE : FALSE;}

    inline void SetCursorInProtectedArea(BOOL b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_CRSR_IN_PROT) : (nCore2Options &= ~VIEWOPT_CORE2_CRSR_IN_PROT);}

    inline BOOL IsSectionBounds() const
        {return !bReadonly && (nCore2Options & VIEWOPT_CORE2_SECTION_BOUNDS) ? TRUE : FALSE;}

    inline void SetSectionBounds(BOOL b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_SECTION_BOUNDS) : (nCore2Options &= ~VIEWOPT_CORE2_SECTION_BOUNDS);}

    inline BOOL IsIndexBackground() const
        {return nCore2Options & VIEWOPT_CORE2_INDEX_BACKGROUND ? TRUE : FALSE;}

    inline void SetIndexBackground(BOOL b)
        { (b != 0) ? (nCore2Options |= VIEWOPT_CORE2_INDEX_BACKGROUND) : (nCore2Options &= ~VIEWOPT_CORE2_INDEX_BACKGROUND);}



/*---------------------------------------------------------------------------

----------------------------------------------------------------------------*/


    inline short GetDivisionX() const   { return nDivisionX; }
    inline void  SetDivisionX( short n ){ nDivisionX = n; }
    inline short GetDivisionY() const   { return nDivisionY; }
    inline void  SetDivisionY( short n ){ nDivisionY = n; }

    inline BYTE  GetPagePrevRow() const   { return nPagePrevRow; }
    inline void  SetPagePrevRow( BYTE n ) { nPagePrevRow = n; }
    inline BYTE  GetPagePrevCol() const   { return nPagePrevCol; }
    inline void  SetPagePrevCol( BYTE n ) { nPagePrevCol = n; }
    BOOL         IsReadonly() const { return bReadonly; }
    void         SetReadonly(BOOL bSet) { bReadonly = bSet; }

    BOOL IsAutoCompleteWords() const;


#ifndef PRODUCT
    // korrespondieren zu den Angaben in ui/config/cfgvw.src
    inline BOOL IsTest1() const     { return bTest1; }
    inline void SetTest1( BOOL b )  { bTest1 = b; }
    inline BOOL IsTest2() const     { return bTest2; }
    inline void SetTest2( BOOL b )  { bTest2 = b; }
    inline BOOL IsTest3() const     { return bTest3; }
    inline void SetTest3( BOOL b )  { bTest3 = b; }
    inline BOOL IsTest4() const     { return bTest4; }
    inline void SetTest4( BOOL b )  { bTest4 = b; }
    inline BOOL IsTest5() const     { return bTest5; }
    inline void SetTest5( BOOL b )  { bTest5 = b; }
    inline BOOL IsTest6() const     { return bTest6; }
    inline void SetTest6( BOOL b )  { bTest6 = b; }
    inline BOOL IsTest7() const     { return bTest7; }
    inline void SetTest7( BOOL b )  { bTest7 = b; }
    inline BOOL IsTest8() const     { return bTest8; }
    inline void SetTest8( BOOL b )  { bTest8 = b; }
    static inline BOOL IsTest9()            { return bTest9; }
    static inline void SetTest9( BOOL b )   { bTest9 = b; }
    inline BOOL IsTest10() const    { return bTest10; }
    inline void SetTest10( BOOL b ) { bTest10 = b; }
#endif

    inline USHORT GetZoom() const    { return nZoom; }
    inline void   SetZoom( USHORT n ){ nZoom = n; }

    void DrawRect( OutputDevice* pOut, const SwRect &rRect, long nCol ) const;
    void DrawRectPrinter( OutputDevice* pOut, const SwRect& rRect ) const;

    SwViewOption& operator=( const SwViewOption &rOpt );
    // Vergleichsmethoden
    BOOL IsEqualFlags ( const SwViewOption &rOpt ) const;
    inline BOOL operator==( const SwViewOption &rOpt ) const;


/*---------------------------------------------------------------------------
    Optionen aus nUIOptions
----------------------------------------------------------------------------*/

//  BOOL    IsViewRibbonBar()  const
//      { return nUIOptions & VIEWOPT_2_RIBBONBAR ? TRUE : FALSE; }
    BOOL    IsViewVScrollBar() const
        { return nUIOptions & VIEWOPT_2_VSCROLLBAR ? TRUE : FALSE;    }
    BOOL    IsViewHScrollBar() const
        { return nUIOptions & VIEWOPT_2_HSCROLLBAR ? TRUE : FALSE;    }
    BOOL    IsViewSVLook()     const
        { return nUIOptions & VIEWOPT_2_SVLOOK ? TRUE : FALSE;    }
    BOOL    IsKeepRatio()      const
        { return nUIOptions & VIEWOPT_2_KEEPASPECTRATIO ? TRUE : FALSE;   }
    BOOL    IsGrfKeepZoom()    const
        { return nUIOptions & VIEWOPT_2_GRFKEEPZOOM ? TRUE : FALSE;   }
    BOOL    IsExecHyperlinks() const
        { return nUIOptions & VIEWOPT_2_EXECHYPERLINKS ? TRUE : FALSE; }
    BOOL    IsPrtFormat() const
        { return nUIOptions & VIEWOPT_2_PRTFORMAT ? TRUE : FALSE; }

    BYTE    GetZoomType()      const { return eZoom;              }

    BYTE    GetTblDest() const    { return nTblDest; }

//  void   SetViewRibbonBar (BOOL b)
//      { b ? (nUIOptions |= VIEWOPT_2_RIBBONBAR ) : ( nUIOptions &= ~VIEWOPT_2_RIBBONBAR); }
    void   SetViewVScrollBar(BOOL b)
        { b ? (nUIOptions |= VIEWOPT_2_VSCROLLBAR ) : ( nUIOptions &= ~VIEWOPT_2_VSCROLLBAR); }
    void   SetViewHScrollBar(BOOL b)
        { b ? (nUIOptions |= VIEWOPT_2_HSCROLLBAR ) : ( nUIOptions &= ~VIEWOPT_2_HSCROLLBAR); }
//  void   SetViewSVLook    (BOOL b)
//      { b ? (nUIOptions |= VIEWOPT_2_SVLOOK ) : ( nUIOptions &= ~VIEWOPT_2_SVLOOK); }
    void   SetKeepRatio     (BOOL b)
        { b ? (nUIOptions |= VIEWOPT_2_KEEPASPECTRATIO ) : ( nUIOptions &= ~VIEWOPT_2_KEEPASPECTRATIO); }
    void   SetGrfKeepZoom   (BOOL b)
        { b ? (nUIOptions |= VIEWOPT_2_GRFKEEPZOOM ) : ( nUIOptions &= ~VIEWOPT_2_GRFKEEPZOOM); }
    void SetExecHyperlinks( BOOL b)
        { b ? (nUIOptions |= VIEWOPT_2_EXECHYPERLINKS) : (nUIOptions &= ~VIEWOPT_2_EXECHYPERLINKS); }
    void SetPrtFormat( BOOL b)
        { b ? (nUIOptions |= VIEWOPT_2_PRTFORMAT) : (nUIOptions &= ~VIEWOPT_2_PRTFORMAT); }

    void            SetZoomType     (BYTE nZoom){ eZoom = nZoom;    }
    void            SetTblDest( BYTE nNew )     { nTblDest = nNew;  }

    const String&   GetSymbolFont() const {return sSymbolFont;}
    void            SetSymbolFont(const String& sSet) {sSymbolFont = sSet;}

    const Color&    GetRetoucheColor() const        { return aRetoucheColor;}
    void            SetRetoucheColor(const Color&r) { aRetoucheColor = r;   }

    inline BOOL     IsViewTabwin()     const;
    inline BOOL     IsViewVLin()       const;
    inline void     SetViewTabwin   (BOOL b);
    inline void     SetViewVLin     (BOOL b);

    // ShadowCursor ein schalten/abfragen/Farbe setzen/Modus setzen
    BOOL    IsShadowCursor()    const
        { return nUIOptions & VIEWOPT_2_SHADOWCRSR ? TRUE : FALSE;   }
    void   SetShadowCursor(BOOL b)
        { b ? (nUIOptions |= VIEWOPT_2_SHADOWCRSR ) : ( nUIOptions &= ~VIEWOPT_2_SHADOWCRSR); }

    BOOL            IsStarOneSetting() const {return bStarOneSetting; }
    void            SetStarOneSetting(BOOL bSet) {bStarOneSetting = bSet; }

    const Color&    GetShdwCrsrColor() const { return aShdwCrsrCol; }
    void            SetShdwCrsrColor( const Color& rCol ) { aShdwCrsrCol = rCol; };

    BYTE            GetShdwCrsrFillMode() const { return nShdwCrsrFillMode; }
    void            SetShdwCrsrFillMode( BYTE nMode ) { nShdwCrsrFillMode = nMode; };

    const Color&    GetIndexBackgrndColor() const { return aIdxBackgrndCol;}
    void            SetIndexBackgrndColor(const Color& rCol) {aIdxBackgrndCol = rCol;}
};


inline BOOL SwViewOption::operator==( const SwViewOption &rOpt ) const
{
    return IsEqualFlags( rOpt ) && nZoom == rOpt.GetZoom();
}

inline void SwViewOption::SetUIOptions( const SwViewOption& rVOpt )
{
    nUIOptions = rVOpt.nUIOptions;
    nTblDest = rVOpt.nTblDest;
    nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
    aShdwCrsrCol = rVOpt.aShdwCrsrCol;
}

inline BOOL SwViewOption::IsViewTabwin() const
{
    return !bReadonly && (nUIOptions & VIEWOPT_2_TABWIN) ? TRUE : FALSE;
}

inline BOOL SwViewOption::IsViewVLin() const
{
    return !bReadonly && (nUIOptions & VIEWOPT_2_VLIN) ? TRUE : FALSE;
}

inline void SwViewOption::SetViewTabwin(BOOL b)
{
    b ? (nUIOptions |= VIEWOPT_2_TABWIN ) : ( nUIOptions &= ~VIEWOPT_2_TABWIN);
}

inline void SwViewOption::SetViewVLin(BOOL b)
{
    b ? (nUIOptions |= VIEWOPT_2_VLIN ) : ( nUIOptions &= ~VIEWOPT_2_VLIN);
}


// Hilfsfunktion zur Ermittlung der HTML-Faehigkeiten
USHORT      GetHtmlMode(const SwDocShell*);


#endif
