/*************************************************************************
 *
 *  $RCSfile: htmltab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
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

//#define TEST_RESIZE

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif


#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FRMATR_HXX
#include "frmatr.hxx"
#endif
#include "pam.hxx"
#include "doc.hxx"
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "poolfmt.hxx"
#include "swtable.hxx"
#include "cellatr.hxx"
#ifdef TEST_RESIZE
#include "viewsh.hxx"
#endif
#include "htmltbl.hxx"
#include "swtblfmt.hxx"
#include "htmlnum.hxx"
#include "swhtml.hxx"
#include "swcss1.hxx"

#define NETSCAPE_DFLT_BORDER 1
#define NETSCAPE_DFLT_CELLPADDING 1
#define NETSCAPE_DFLT_CELLSPACING 2

//#define FIX56334

static HTMLOptionEnum __FAR_DATA aHTMLTblVAlignTable[] =
{
    { sHTML_VA_top,         VERT_NONE       },
    { sHTML_VA_middle,      VERT_CENTER     },
    { sHTML_VA_bottom,      VERT_BOTTOM     },
    { 0,                    0               }
};


/*  */

// Die Optionen eines Table-Tags

struct HTMLTableOptions
{
    sal_uInt16 nCols;
    sal_uInt16 nWidth;
    sal_uInt16 nHeight;
    sal_uInt16 nCellPadding;
    sal_uInt16 nCellSpacing;
    sal_uInt16 nBorder;
    sal_uInt16 nHSpace;
    sal_uInt16 nVSpace;

    SvxAdjust eAdjust;
    SwVertOrient eVertOri;
    HTMLTableFrame eFrame;
    HTMLTableRules eRules;

    sal_Bool bPrcWidth : 1;
    sal_Bool bTableAdjust : 1;
    sal_Bool bBGColor : 1;

    Color aBorderColor;
    Color aBGColor;

    String aBGImage, aStyle, aId, aClass;

    HTMLTableOptions( const HTMLOptions *pOptions, SvxAdjust eParentAdjust );
};

/*  */

class _HTMLTableContext
{
    SwHTMLNumRuleInfo aNumRuleInfo; // Vor der Tabelle gueltige Numerierung

    SwTableNode *pTblNd;            // der Tabellen-Node
    SwFrmFmt *pFrmFmt;              // der Fly ::com::sun::star::frame::Frame, in dem die Tabelle steht
    SwPosition *pPos;               // die Position hinter der Tabelle

    sal_uInt16 nContextStAttrMin;
    sal_uInt16 nContextStMin;

    sal_Bool    bRestartPRE : 1;
    sal_Bool    bRestartXMP : 1;
    sal_Bool    bRestartListing : 1;

public:

    _HTMLAttrTable aAttrTab;        // und die Attribute

    _HTMLTableContext( SwPosition *pPs, sal_uInt16 nCntxtStMin,
                       sal_uInt16 nCntxtStAttrMin ) :
        pTblNd( 0 ), pFrmFmt( 0 ), pPos( pPs ),
        nContextStMin( nCntxtStMin ), nContextStAttrMin( nCntxtStAttrMin ),
        bRestartPRE( sal_False ), bRestartXMP( sal_False ), bRestartListing( sal_False )
    {
        memset( &aAttrTab, 0, sizeof( _HTMLAttrTable ));
    }

    ~_HTMLTableContext();

    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; };

    void SavePREListingXMP( SwHTMLParser& rParser );
    void RestorePREListingXMP( SwHTMLParser& rParser );

    SwPosition *GetPos() const { return pPos; }

    void SetTableNode( SwTableNode *pNd ) { pTblNd = pNd; }
    SwTableNode *GetTableNode() const { return pTblNd; }

    void SetFrmFmt( SwFrmFmt *pFmt ) { pFrmFmt = pFmt; }
    SwFrmFmt *GetFrmFmt() const { return pFrmFmt; }

    sal_uInt16 GetContextStMin() const { return nContextStMin; }
    sal_uInt16 GetContextStAttrMin() const { return nContextStAttrMin; }
};

/*  */

// der Inhalt einer Zelle ist eine verkettete Liste mit SwStartNodes und
// HTMLTables.

class HTMLTableCnts
{
    HTMLTableCnts *pNext;               // der naechste Inhalt

    // von den beiden naechsten Pointern darf nur einer gesetzt sein!
    const SwStartNode *pStartNode;      // ein Abastz
    HTMLTable *pTable;                  // eine Tabelle

    SwHTMLTableLayoutCnts* pLayoutInfo;

    sal_Bool bNoBreak;

    void InitCtor();

public:

    HTMLTableCnts( const SwStartNode* pStNd );
    HTMLTableCnts( HTMLTable* pTab );

    ~HTMLTableCnts();                   // nur in ~HTMLTableCell erlaubt

    // Ermitteln des SwStartNode bzw. der HTMLTable
    const SwStartNode *GetStartNode() const { return pStartNode; }
    const HTMLTable *GetTable() const { return pTable; }
    HTMLTable *GetTable() { return pTable; }

    // hinzufuegen eines neuen Knotens am Listenende
    void Add( HTMLTableCnts* pNewCnts );

    // Ermitteln des naechsten Knotens
    const HTMLTableCnts *Next() const { return pNext; }
    HTMLTableCnts *Next() { return pNext; }

    inline void SetTableBox( SwTableBox *pBox );

    void SetNoBreak() { bNoBreak = sal_True; }

    SwHTMLTableLayoutCnts *CreateLayoutInfo();
};

/*  */

// Eine Zelle der HTML-Tabelle

class HTMLTableCell
{
    // !!!ACHTUNG!!!!! Fuer jeden neuen Pointer muss die SetProtected-
    // Methode (und natuerlich der Destruktor) bearbeitet werden.
    HTMLTableCnts *pContents;       // der Inhalt der Zelle
    SvxBrushItem *pBGBrush;         // Hintergrund der Zelle
    // !!!ACHTUNG!!!!!

    sal_uInt32 nNumFmt;
    sal_uInt16 nRowSpan;                // ROWSPAN der Zelle
    sal_uInt16 nColSpan;                // COLSPAN der Zelle
    sal_uInt16 nWidth;                  // WIDTH der Zelle
    double nValue;
    SwVertOrient eVertOri;          // vertikale Ausrichtung der Zelle
    sal_Bool bProtected : 1;            // Zelle darf nicht belegt werden
    sal_Bool bRelWidth : 1;             // nWidth ist %-Angabe
    sal_Bool bHasNumFmt : 1;
    sal_Bool bHasValue : 1;
    sal_Bool bNoWrap : 1;

public:

    HTMLTableCell();                // neue Zellen sind immer leer

    ~HTMLTableCell();               // nur in ~HTMLTableRow erlaubt

    // Belegen einer nicht-leeren Zelle
    void Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
              SwVertOrient eVertOri, SvxBrushItem *pBGBrush,
              sal_Bool bHasNumFmt, sal_uInt32 nNumFmt,
              sal_Bool bHasValue, double nValue, sal_Bool bNoWrap );

    // Schuetzen einer leeren 1x1-Zelle
    void SetProtected();

    // Setzen/Ermitteln des Inhalts einer Zelle
    void SetContents( HTMLTableCnts *pCnts ) { pContents = pCnts; }
    const HTMLTableCnts *GetContents() const { return pContents; }
    HTMLTableCnts *GetContents() { return pContents; }

    // ROWSPAN/COLSPAN der Zelle Setzen/Ermitteln
    void SetRowSpan( sal_uInt16 nRSpan ) { nRowSpan = nRSpan; }
    sal_uInt16 GetRowSpan() const { return nRowSpan; }

    void SetColSpan( sal_uInt16 nCSpan ) { nColSpan = nCSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    inline void SetWidth( sal_uInt16 nWidth, sal_Bool bRelWidth );

    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }

    inline sal_Bool GetNumFmt( sal_uInt32& rNumFmt ) const;
    inline sal_Bool GetValue( double& rValue ) const;

    SwVertOrient GetVertOri() const { return eVertOri; }

    // Ist die Zelle belegt oder geschuetzt?
    sal_Bool IsUsed() const { return pContents!=0 || bProtected; }

    SwHTMLTableLayoutCell *CreateLayoutInfo();
};

/*  */

// Eine Zeile der HTML-Tabelle

typedef HTMLTableCell* HTMLTableCellPtr;
SV_DECL_PTRARR_DEL(HTMLTableCells,HTMLTableCellPtr,5,5)

class HTMLTableRow
{
    HTMLTableCells *pCells;             // die Zellen der Zeile

    sal_Bool bIsEndOfGroup : 1;
    sal_Bool bSplitable : 1;

    sal_uInt16 nHeight;                     // Optionen von <TR>/<TD>
    sal_uInt16 nEmptyRows;                  // wieviele Leere Zeilen folgen

    SvxAdjust eAdjust;
    SwVertOrient eVertOri;
    SvxBrushItem *pBGBrush;             // Hintergrund der Zelle aus STYLE

public:

    sal_Bool bBottomBorder;                 // kommt hinter der Zeile eine Linie?

    HTMLTableRow( sal_uInt16 nCells=0 );    // die Zellen der Zeile sind leer

    ~HTMLTableRow();

    inline void SetHeight( sal_uInt16 nHeight );
    sal_uInt16 GetHeight() const { return nHeight; }

    // Ermitteln einer Zelle
    inline HTMLTableCell *GetCell( sal_uInt16 nCell ) const;
    inline const HTMLTableCells *GetCells() const { return pCells; }


    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( SwVertOrient eV) { eVertOri = eV; }
    inline SwVertOrient GetVertOri() const { return eVertOri; }

    void SetBGBrush( SvxBrushItem *pBrush ) { pBGBrush = pBrush; }
    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }

    inline void SetEndOfGroup() { bIsEndOfGroup = sal_True; }
    inline sal_Bool IsEndOfGroup() const { return bIsEndOfGroup; }

    void IncEmptyRows() { nEmptyRows++; }
    sal_uInt16 GetEmptyRows() const { return nEmptyRows; }

    // Expandieren einer Zeile durch hinzufuegen leerer Zellen
    void Expand( sal_uInt16 nCells, sal_Bool bOneCell=sal_False );

    // Verkuerzen einer Zeile durch loesen von leeren Zellen
    void Shrink( sal_uInt16 nCells );

    void SetSplitable( sal_Bool bSet ) { bSplitable = bSet; }
    sal_Bool IsSplitable() const { return bSplitable; }
};

/*  */

// Eine Spalte der HTML-Tabelle

class HTMLTableColumn
{
    sal_Bool bIsEndOfGroup;

    sal_uInt16 nWidth;                      // Optionen von <COL>
    sal_Bool bRelWidth;

    SvxAdjust eAdjust;
    SwVertOrient eVertOri;

    SwFrmFmt *aFrmFmts[6];

    inline sal_uInt16 GetFrmFmtIdx( sal_Bool bBorderLine,
                                SwVertOrient eVertOri ) const;

public:

    sal_Bool bLeftBorder;                   // kommt vor der Spalte eine Linie

    HTMLTableColumn();

    inline void SetWidth( sal_uInt16 nWidth, sal_Bool bRelWidth);

    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( SwVertOrient eV) { eVertOri = eV; }
    inline SwVertOrient GetVertOri() const { return eVertOri; }

    inline void SetEndOfGroup() { bIsEndOfGroup = sal_True; }
    inline sal_Bool IsEndOfGroup() const { return bIsEndOfGroup; }

    inline void SetFrmFmt( SwFrmFmt *pFmt, sal_Bool bBorderLine,
                           SwVertOrient eVertOri );
    inline SwFrmFmt *GetFrmFmt( sal_Bool bBorderLine,
                                SwVertOrient eVertOri ) const;

    SwHTMLTableLayoutColumn *CreateLayoutInfo();
};

/*  */

// eine HTML-Tabelle

typedef HTMLTableRow* HTMLTableRowPtr;
SV_DECL_PTRARR_DEL(HTMLTableRows,HTMLTableRowPtr,5,5)

typedef HTMLTableColumn* HTMLTableColumnPtr;
SV_DECL_PTRARR_DEL(HTMLTableColumns,HTMLTableColumnPtr,5,5)

SV_DECL_PTRARR(SdrObjects,SdrObject *,1,1)

class HTMLTable
{
    String aId;
    String aStyle;
    String aClass;

    SdrObjects *pResizeDrawObjs;// SDR-Objekte
    SvUShorts *pDrawObjPrcWidths;   // Spalte des Zeichen-Objekts und dessen
                                    // relative Breite

    HTMLTableRows *pRows;           // die Zeilen der Tabelle
    HTMLTableColumns *pColumns;     // die Spalten der Tabelle

    sal_uInt16 nRows;                   // Anzahl Zeilen
    sal_uInt16 nCols;                   // Anzahl Spalten
    sal_uInt16 nFilledCols;             // Anzahl tatsaechlich gefuellter Spalten

    sal_uInt16 nCurRow;                 // aktuelle Zeile
    sal_uInt16 nCurCol;                 // aktuelle Spalte

    sal_uInt16 nLeftMargin;             // Abstand zum linken Rand (aus Absatz)
    sal_uInt16 nRightMargin;            // Abstand zum rechten Rand (aus Absatz)

    sal_uInt16 nCellPadding;            // Abstand Umrandung zum Text
    sal_uInt16 nCellSpacing;            // Abstand zwischen zwei Zellen
    sal_uInt16 nHSpace;
    sal_uInt16 nVSpace;

    sal_uInt16 nBoxes;                  // Wievele Boxen enthaelt die Tabelle

    const SwStartNode *pPrevStNd;   // der Table-Node oder der Start-Node
                                    // der vorhergehenden Section
    const SwTable *pSwTable;        // die SW-Tabelle (nur auf dem Top-Level)
    SwTableBox *pBox1;              // die TableBox, die beim Erstellen
                                    // der Top-Level-Tabelle angelegt wird

    SwTableBoxFmt *pBoxFmt;         // das ::com::sun::star::frame::Frame-Format einer SwTableBox
    SwTableLineFmt *pLineFmt;       // das ::com::sun::star::frame::Frame-Format einer SwTableLine
    SwTableLineFmt *pLineFrmFmtNoHeight;
    SvxBrushItem *pBGBrush;         // Hintergrund der Tabelle
    SvxBrushItem *pInhBGBrush;      // "geerbter" Hintergrund der Tabelle
    const SwStartNode *pCaptionStartNode;   // Start-Node der Tabellen-Ueberschrift

    SvxBorderLine aTopBorderLine;   // die Linie fuer die Umrandung
    SvxBorderLine aBottomBorderLine;// die Linie fuer die Umrandung
    SvxBorderLine aLeftBorderLine;  // die Linie fuer die Umrandung
    SvxBorderLine aRightBorderLine; // die Linie fuer die Umrandung
    SvxBorderLine aBorderLine;      // die Linie fuer die Umrandung
    SvxBorderLine aInhLeftBorderLine;   // die Linie fuer die Umrandung
    SvxBorderLine aInhRightBorderLine;  // die Linie fuer die Umrandung
    sal_Bool bTopBorder;                // besitzt die Tabelle oben eine Linie
    sal_Bool bRightBorder;              // besitzt die Tabelle rechts eine Linie
    sal_Bool bTopAlwd;                  // duerfen die Raender gesetzt werden?
    sal_Bool bRightAlwd;
    sal_Bool bFillerTopBorder;          // bekommt eine linke/rechter Filler-
    sal_Bool bFillerBottomBorder;       // Zelle eine obere/untere Umrandung?
    sal_Bool bInhLeftBorder;
    sal_Bool bInhRightBorder;
    sal_Bool bBordersSet;               // die Umrandung wurde bereits gesetzt
    sal_Bool bForceFrame;
    sal_Bool bTableAdjustOfTag;         // stammt nTableAdjust aus <TABLE>?
    sal_Bool bHeadlineRepeat;           // Ueberschrift wiederholen
    sal_Bool bIsParentHead;
    sal_Bool bHasParentSection;
    sal_Bool bMakeTopSubTable;
    sal_Bool bHasToFly;
    sal_Bool bFixedCols;
    sal_Bool bColSpec;                  // Gab es COL(GROUP)-Elemente?
    sal_Bool bPrcWidth;                 // Breite ist eine %-Angabe

    SwHTMLParser *pParser;          // der aktuelle Parser
    HTMLTable *pTopTable;           // die Tabelle auf dem Top-Level
    HTMLTableCnts *pParentContents;

    _HTMLTableContext *pContext;    // der Kontext der Tabelle

    SwHTMLTableLayout *pLayoutInfo;


    // die folgenden Parameter stammen aus der dem <TABLE>-Tag
    sal_uInt16 nWidth;                  // die Breite der Tabelle
    sal_uInt16 nHeight;                 // absolute Hoehe der Tabelle
    SvxAdjust eTableAdjust;         // ::com::sun::star::drawing::Alignment der Tabelle
    SwVertOrient eVertOri;          // Default vertikale Ausr. der Zellen
    sal_uInt16 nBorder;                 // Breite der auesseren Umrandung
    HTMLTableFrame eFrame;          // Rahmen um die Tabelle
    HTMLTableRules eRules;          // Ramhen in der Tabelle
    sal_Bool bTopCaption;               // Ueberschrift ueber der Tabelle

    void InitCtor( const HTMLTableOptions *pOptions );

    // Korigieren des Row-Spans fuer alle Zellen oberhalb der
    // angegeben Zelle und der Zelle selbst, fuer die den anegebenen
    // Inhalt besitzen. Die angegeben Zelle bekommt den Row-Span 1
    void FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, const HTMLTableCnts *pCnts );

    // Schuetzen der angegeben Zelle und den darunterliegenden
    void ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan );

    // Suchen des SwStartNodes der logisch vorhergehenden Box
    // bei nRow==nCell==USHRT_MAX wird der allerletzte Start-Node
    // der Tabelle zurueckgegeben
    const SwStartNode* GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    sal_uInt16 GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                            sal_Bool bSwBorders=sal_True ) const;
    sal_uInt16 GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                               sal_Bool bSwBorders=sal_True ) const;

    // Anpassen des ::com::sun::star::frame::Frame-Formates einer Box
    void FixFrameFmt( SwTableBox *pBox, sal_uInt16 nRow, sal_uInt16 nCol,
                      sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                      sal_Bool bFirstPara=sal_True, sal_Bool bLastPara=sal_True ) const;
    void FixFillerFrameFmt( SwTableBox *pBox, sal_Bool bRight ) const;

    // den Inhalt (Lines/Boxen) eine Tabelle erstellen
    void _MakeTable( SwTableBox *pUpper=0 );

    // Anlegen einer neuen SwTableBox, die einen SwStartNode enthaelt
    SwTableBox *NewTableBox( const SwStartNode *pStNd,
                             SwTableLine *pUpper ) const;

    // Erstellen einer SwTableLine aus den Zellen des Rechtecks
    // (nTopRow/nLeftCol) inklusive bis (nBottomRow/nRightRow) exklusive
    SwTableLine *MakeTableLine( SwTableBox *pUpper,
                                sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                sal_uInt16 nBottomRow, sal_uInt16 nRightCol );

    // Erstellen einer SwTableBox aus den Zellen des Rechtecks
    // (nTopRow/nLeftCol) inklusive bis (nBottomRow/nRightRow) exklusive
    SwTableBox *MakeTableBox( SwTableLine *pUpper,
                              sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                              sal_uInt16 nBottomRow, sal_uInt16 nRightCol );

    // Erstellen einer SwTableBox aus dem Inhalt einer Zelle
    SwTableBox *MakeTableBox( SwTableLine *pUpper,
                              HTMLTableCnts *pCnts,
                              sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                              sal_uInt16 nBootomRow, sal_uInt16 nRightCol );

    // der Autolayout-Algorithmus

    // Setzen der Umrandung anhand der Vorgaben der Parent-Tabelle
    void InheritBorders( const HTMLTable *pParent,
                         sal_uInt16 nRow, sal_uInt16 nCol,
                         sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                         sal_Bool bFirstPara, sal_Bool bLastPara );

    // Linke und rechte Umrandung der umgebenen Tabelle erben
    void InheritVertBorders( const HTMLTable *pParent,
                             sal_uInt16 nCol, sal_uInt16 nColSpan );


    // Setzen der Umrandung anhand der Benutzervorgaben
    void SetBorders();

    // wurde die Umrandung der Tabelle schon gesetzt
    sal_Bool BordersSet() const { return bBordersSet; }

    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
    const SvxBrushItem *GetInhBGBrush() const { return pInhBGBrush; }

    sal_uInt16 GetBorderWidth( const SvxBorderLine& rBLine,
                           sal_Bool bWithDistance=sal_False ) const;

public:

    sal_Bool bFirstCell;                // wurde schon eine Zelle angelegt?

    HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
               sal_Bool bParHead, sal_Bool bHasParentSec,
               sal_Bool bTopTbl, sal_Bool bHasToFly,
               const HTMLTableOptions *pOptions );

    ~HTMLTable();

    // Ermitteln einer Zelle
    inline HTMLTableCell *GetCell( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    // Ueberschrift setzen/ermitteln
    inline void SetCaption( const SwStartNode *pStNd, sal_Bool bTop );
    const SwStartNode *GetCaptionStartNode() const { return pCaptionStartNode; }
    sal_Bool IsTopCaption() const { return bTopCaption; }

    SvxAdjust GetTableAdjust( sal_Bool bAny ) const
    {
        return (bTableAdjustOfTag || bAny) ? eTableAdjust : SVX_ADJUST_END;
    }
    SwVertOrient GetVertOri() const { return eVertOri; }

    sal_uInt16 GetHSpace() const { return nHSpace; }
    sal_uInt16 GetVSpace() const { return nVSpace; }

    sal_Bool HasPrcWidth() const { return bPrcWidth; }
    sal_uInt8 GetPrcWidth() const { return bPrcWidth ? (sal_uInt8)nWidth : 0; }

    sal_uInt16 GetMinWidth() const
    {
        sal_uInt32 nMin = pLayoutInfo->GetMin();
        return nMin < USHRT_MAX ? (sal_uInt16)nMin : USHRT_MAX;
    }

    // von Zeilen oder Spalten geerbtes ::com::sun::star::drawing::Alignment holen
    SvxAdjust GetInheritedAdjust() const;
    SwVertOrient GetInheritedVertOri() const;

    // Einfuegen einer Zelle an der aktuellen Position
    void InsertCell( HTMLTableCnts *pCnts, sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                     sal_uInt16 nWidth, sal_Bool bRelWidth, sal_uInt16 nHeight,
                     SwVertOrient eVertOri, SvxBrushItem *pBGBrush,
                     sal_Bool bHasNumFmt, sal_uInt32 nNumFmt,
                     sal_Bool bHasValue, double nValue, sal_Bool bNoWrap );

    // Start/Ende einer neuen Zeile bekanntgeben
    void OpenRow( SvxAdjust eAdjust, SwVertOrient eVertOri,
                  SvxBrushItem *pBGBrush );
    void CloseRow( sal_Bool bEmpty );

    // Ende einer neuen Section bekanntgeben
    inline void CloseSection( sal_Bool bHead );

    // Ende einer Spalten-Gruppe bekanntgeben
    inline void CloseColGroup( sal_uInt16 nSpan, sal_uInt16 nWidth, sal_Bool bRelWidth,
                               SvxAdjust eAdjust, SwVertOrient eVertOri );

    // Einfuegen einer Spalte
    void InsertCol( sal_uInt16 nSpan, sal_uInt16 nWidth, sal_Bool bRelWidth,
                    SvxAdjust eAdjust, SwVertOrient eVertOri );

    // Beenden einer Tab-Definition (MUSS fuer ALLE Tabs aufgerufen werden)
    void CloseTable();

    // SwTable konstruieren (inkl. der Child-Tabellen)
    void MakeTable( SwTableBox *pUpper, sal_uInt16 nAbsAvail,
                    sal_uInt16 nRelAvail=0, sal_uInt16 nAbsLeftSpace=0,
                    sal_uInt16 nAbsRightSpace=0, sal_uInt16 nInhAbsSpace=0 );

    inline sal_Bool IsNewDoc() const { return pParser->IsNewDoc(); }

    void SetHasParentSection( sal_Bool bSet ) { bHasParentSection = bSet; }
    sal_Bool HasParentSection() const { return bHasParentSection; }

    void SetParentContents( HTMLTableCnts *pCnts ) { pParentContents = pCnts; }
    HTMLTableCnts *GetParentContents() const { return pParentContents; }

    void MakeParentContents();

    sal_Bool GetIsParentHeader() const { return bIsParentHead; }

    sal_Bool IsMakeTopSubTable() const { return bMakeTopSubTable; }
    void SetHasToFly() { bHasToFly=sal_True; }
    sal_Bool HasToFly() const { return bHasToFly; }

    void SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                   sal_uInt16 nLeft, sal_uInt16 nRight,
                   const SwTable *pSwTab=0, sal_Bool bFrcFrame=sal_False );

    _HTMLTableContext *GetContext() const { return pContext; }

    SwHTMLTableLayout *CreateLayoutInfo();

    sal_Bool HasColTags() const { return bColSpec; }

    sal_uInt16 IncGrfsThatResize() { return pSwTable ? ((SwTable *)pSwTable)->IncGrfsThatResize() : 0; }

    void RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth );

    const SwTable *GetSwTable() const { return pSwTable; }

    void SetBGBrush( const SvxBrushItem& rBrush ) { delete pBGBrush; pBGBrush = new SvxBrushItem( rBrush ); }

    const String& GetId() const { return aId; }
    const String& GetClass() const { return aClass; }
    const String& GetStyle() const { return aStyle; }

    void IncBoxCount() { nBoxes++; }
    sal_Bool IsOverflowing() const { return nBoxes > 64000; }
};

SV_IMPL_PTRARR(HTMLTableCells,HTMLTableCellPtr)
SV_IMPL_PTRARR(HTMLTableRows,HTMLTableRowPtr)
SV_IMPL_PTRARR(HTMLTableColumns,HTMLTableColumnPtr)

/*  */


void HTMLTableCnts::InitCtor()
{
    pNext = 0;
    pLayoutInfo = 0;

    bNoBreak = sal_False;
}

HTMLTableCnts::HTMLTableCnts( const SwStartNode* pStNd ):
    pStartNode(pStNd), pTable(0)
{
    InitCtor();
}

HTMLTableCnts::HTMLTableCnts( HTMLTable* pTab ):
    pStartNode(0), pTable(pTab)
{
    InitCtor();
}

HTMLTableCnts::~HTMLTableCnts()
{
    delete pTable;              // die Tabellen brauchen wir nicht mehr
    delete pNext;
}

void HTMLTableCnts::Add( HTMLTableCnts* pNewCnts )
{
    HTMLTableCnts *pCnts = this;

    while( pCnts->pNext )
        pCnts = pCnts->pNext;

    pCnts->pNext = pNewCnts;
}

inline void HTMLTableCnts::SetTableBox( SwTableBox *pBox )
{
    ASSERT( pLayoutInfo, "Da sit noch keine Layout-Info" );
    if( pLayoutInfo )
        pLayoutInfo->SetTableBox( pBox );
}

SwHTMLTableLayoutCnts *HTMLTableCnts::CreateLayoutInfo()
{
    if( !pLayoutInfo )
    {
        SwHTMLTableLayoutCnts *pNextInfo = pNext ? pNext->CreateLayoutInfo() : 0;
        SwHTMLTableLayout *pTableInfo = pTable ? pTable->CreateLayoutInfo() : 0;

        pLayoutInfo = new SwHTMLTableLayoutCnts( pStartNode, pTableInfo,
                                                 bNoBreak, pNextInfo );
    }

    return pLayoutInfo;
}

/*  */

HTMLTableCell::HTMLTableCell():
    pContents(0),
    pBGBrush(0),
    nRowSpan(1), nColSpan(1), nWidth( 0 ),
    eVertOri( VERT_NONE ),
    bProtected(sal_False), bRelWidth( sal_False ),
    bHasNumFmt(sal_False), nNumFmt(0),
    bHasValue(sal_False), nValue(0)
{}

HTMLTableCell::~HTMLTableCell()
{
    // der Inhalt ist in mehrere Zellen eingetragen, darf aber nur einmal
    // geloescht werden
    if( 1==nRowSpan && 1==nColSpan )
    {
        delete pContents;
        delete pBGBrush;
    }
}

void HTMLTableCell::Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                         SwVertOrient eVert, SvxBrushItem *pBrush,
                         sal_Bool bHasNF, sal_uInt32 nNF, sal_Bool bHasV, double nVal,
                         sal_Bool bNWrap )
{
    pContents = pCnts;
    nRowSpan = nRSpan;
    nColSpan = nCSpan;
    bProtected = sal_False;
    eVertOri = eVert;
    pBGBrush = pBrush;

    bHasNumFmt = bHasNF;
    bHasValue = bHasV;
    nNumFmt = nNF;
    nValue = nVal;

    bNoWrap = bNWrap;
}

inline void HTMLTableCell::SetWidth( sal_uInt16 nWdth, sal_Bool bRelWdth )
{
    nWidth = nWdth;
    bRelWidth = bRelWdth;
}

void HTMLTableCell::SetProtected()
{
    // Die Inhalte dieser Zelle mussen nich irgenwo anders verankert
    // sein, weil sie nicht geloescht werden!!!

    // Inhalt loeschen
    pContents = 0;

    // Hintergrundfarbe kopieren.
    if( pBGBrush )
        pBGBrush = new SvxBrushItem( *pBGBrush );

    nRowSpan = 1;
    nColSpan = 1;
    bProtected = sal_True;
}

inline sal_Bool HTMLTableCell::GetNumFmt( sal_uInt32& rNumFmt ) const
{
    rNumFmt = nNumFmt;
    return bHasNumFmt;
}

inline sal_Bool HTMLTableCell::GetValue( double& rValue ) const
{
    rValue = nValue;
    return bHasValue;
}

SwHTMLTableLayoutCell *HTMLTableCell::CreateLayoutInfo()
{
    SwHTMLTableLayoutCnts *pCntInfo = pContents ? pContents->CreateLayoutInfo() : 0;

    return new SwHTMLTableLayoutCell( pCntInfo, nRowSpan, nColSpan, nWidth,
                                      bRelWidth, bNoWrap );
}

/*  */

HTMLTableRow::HTMLTableRow( sal_uInt16 nCells ):
    pCells(new HTMLTableCells), bIsEndOfGroup(sal_False),
    eAdjust(SVX_ADJUST_END), eVertOri(VERT_TOP), pBGBrush(0),
    bBottomBorder(sal_False), nHeight(0), nEmptyRows(0), bSplitable( sal_False )
{
    for( sal_uInt16 i=0; i<nCells; i++ )
    {
        pCells->Insert( new HTMLTableCell, pCells->Count() );
    }

    ASSERT( nCells==pCells->Count(),
            "Zellenzahl in neuer HTML-Tabellenzeile stimmt nicht" );
}

HTMLTableRow::~HTMLTableRow()
{
    delete pCells;
    delete pBGBrush;
}

inline void HTMLTableRow::SetHeight( sal_uInt16 nHght )
{
    if( nHght > nHeight  )
        nHeight = nHght;
}

inline HTMLTableCell *HTMLTableRow::GetCell( sal_uInt16 nCell ) const
{
    ASSERT( nCell<pCells->Count(),
        "ungueltiger Zellen-Index in HTML-Tabellenzeile" );
    return (*pCells)[nCell];
}

void HTMLTableRow::Expand( sal_uInt16 nCells, sal_Bool bOneCell )
{
    // die Zeile wird mit einer einzigen Zelle aufgefuellt, wenn
    // bOneCell gesetzt ist. Das geht, nur fuer Zeilen, in die keine
    // Zellen mehr eingefuegt werden!

    sal_uInt16 nColSpan = nCells-pCells->Count();
    for( sal_uInt16 i=pCells->Count(); i<nCells; i++ )
    {
        HTMLTableCell *pCell = new HTMLTableCell;
        if( bOneCell )
            pCell->SetColSpan( nColSpan );

        pCells->Insert( pCell, pCells->Count() );
        nColSpan--;
    }

    ASSERT( nCells==pCells->Count(),
            "Zellenzahl in expandierter HTML-Tabellenzeile stimmt nicht" );
}

void HTMLTableRow::Shrink( sal_uInt16 nCells )
{
    ASSERT( nCells < pCells->Count(), "Anzahl Zellen falsch" );

#ifdef DEBUG
    sal_uInt16 nEnd = pCells->Count();
    for( sal_uInt16 i=nCells; i<nEnd; i++ )
    {
        HTMLTableCell *pCell = (*pCells)[i];
        ASSERT( pCell->GetRowSpan() == 1,
                "RowSpan von zu loesender Zelle ist falsch" );
        ASSERT( pCell->GetColSpan() == nEnd - i,
                    "ColSpan von zu loesender Zelle ist falsch" );
        ASSERT( !pCell->GetContents(), "Zu loeschende Zelle hat Inhalt" );
    }
#endif

    pCells->DeleteAndDestroy( nCells, pCells->Count()-nCells );
}

/*  */

HTMLTableColumn::HTMLTableColumn():
    bIsEndOfGroup(sal_False),
    nWidth(0), bRelWidth(sal_False),
    eAdjust(SVX_ADJUST_END), eVertOri(VERT_TOP),
    bLeftBorder(sal_False)
{
    for( sal_uInt16 i=0; i<6; i++ )
        aFrmFmts[i] = 0;
}

inline void HTMLTableColumn::SetWidth( sal_uInt16 nWdth, sal_Bool bRelWdth )
{
    if( bRelWidth==bRelWdth )
    {
        if( nWdth > nWidth )
            nWidth = nWdth;
    }
    else
        nWidth = nWdth;
    bRelWidth = bRelWdth;
}

inline SwHTMLTableLayoutColumn *HTMLTableColumn::CreateLayoutInfo()
{
    return new SwHTMLTableLayoutColumn( nWidth, bRelWidth, bLeftBorder );
}

inline sal_uInt16 HTMLTableColumn::GetFrmFmtIdx( sal_Bool bBorderLine,
                                             SwVertOrient eVertOri ) const
{
    ASSERT( VERT_TOP != eVertOri, "Top ist nicht erlaubt" );
    sal_uInt16 n = bBorderLine ? 3 : 0;
    switch( eVertOri )
    {
    case VERT_CENTER:   n+=1;   break;
    case VERT_BOTTOM:   n+=2;   break;
    }
    return n;
}

inline void HTMLTableColumn::SetFrmFmt( SwFrmFmt *pFmt, sal_Bool bBorderLine,
                                        SwVertOrient eVertOri )
{
    aFrmFmts[GetFrmFmtIdx(bBorderLine,eVertOri)] = pFmt;
}

inline SwFrmFmt *HTMLTableColumn::GetFrmFmt( sal_Bool bBorderLine,
                                             SwVertOrient eVertOri ) const
{
    return aFrmFmts[GetFrmFmtIdx(bBorderLine,eVertOri)];
}

/*  */


void HTMLTable::InitCtor( const HTMLTableOptions *pOptions )
{
    pResizeDrawObjs = 0;
    pDrawObjPrcWidths = 0;

    pRows = new HTMLTableRows;
    pColumns = new HTMLTableColumns;
    nRows = 0;
    nCurRow = 0; nCurCol = 0;

    pBox1 = 0;
    pBoxFmt = 0; pLineFmt = 0;
    pLineFrmFmtNoHeight = 0;
    pInhBGBrush = 0;

    pPrevStNd = 0;
    pSwTable = 0;

    bTopBorder = sal_False; bRightBorder = sal_False;
    bTopAlwd = sal_True; bRightAlwd = sal_True;
    bFillerTopBorder = sal_False; bFillerBottomBorder = sal_False;
    bInhLeftBorder = sal_False; bInhRightBorder = sal_False;
    bBordersSet = sal_False;
    bForceFrame = sal_False;
    bHeadlineRepeat = sal_False;

    nLeftMargin = 0;
    nRightMargin = 0;

    const Color& rBorderColor = pOptions->aBorderColor;

    long nBorderOpt = (long)pOptions->nBorder;
    long nPWidth = nBorderOpt==USHRT_MAX ? NETSCAPE_DFLT_BORDER
                                         : nBorderOpt;
    long nPHeight = nBorderOpt==USHRT_MAX ? 0 : nBorderOpt;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );

    // nBorder gibt die Breite der Umrandung an, wie sie in die
    // Breitenberechnung in Netscape einfliesst. Wenn pOption->nBorder
    // == USHRT_MAX, wurde keine BORDER-Option angegeben. Trotzdem fliesst
    // eine 1 Pixel breite Umrandung in die Breitenberechnung mit ein.
    nBorder = (sal_uInt16)nPWidth;
    if( nBorderOpt==USHRT_MAX )
        nPWidth = 0;

    // HACK: ein Pixel-breite Linien sollen zur Haarlinie werden, wenn
    // wir mit doppelter Umrandung arbeiten
    if( pOptions->nCellSpacing!=0 && nBorderOpt==1 )
    {
        nPWidth = 1;
        nPHeight = 1;
    }

    SvxCSS1Parser::SetBorderWidth( aTopBorderLine, (sal_uInt16)nPHeight,
                                   pOptions->nCellSpacing!=0, sal_True );
    aTopBorderLine.SetColor( rBorderColor );
    aBottomBorderLine = aTopBorderLine;

    if( nPWidth == nPHeight )
    {
        aLeftBorderLine = aTopBorderLine;
    }
    else
    {
        SvxCSS1Parser::SetBorderWidth( aLeftBorderLine, (sal_uInt16)nPWidth,
                                       pOptions->nCellSpacing!=0, sal_True );
        aLeftBorderLine.SetColor( rBorderColor );
    }
    aRightBorderLine = aLeftBorderLine;

    if( pOptions->nCellSpacing != 0 )
    {
        aBorderLine.SetOutWidth( DEF_DOUBLE_LINE7_OUT );
        aBorderLine.SetInWidth( DEF_DOUBLE_LINE7_IN );
        aBorderLine.SetDistance( DEF_DOUBLE_LINE7_DIST );
    }
    else
    {
        aBorderLine.SetOutWidth( DEF_LINE_WIDTH_1 );
    }
    aBorderLine.SetColor( rBorderColor );

    if( nCellPadding )
    {
        if( nCellPadding==USHRT_MAX )
            nCellPadding = MIN_BORDER_DIST; // default
        else
        {
            nCellPadding = pParser->ToTwips( nCellPadding );
            if( nCellPadding<MIN_BORDER_DIST  )
                nCellPadding = MIN_BORDER_DIST;
        }
    }
    if( nCellSpacing )
    {
        if( nCellSpacing==USHRT_MAX )
            nCellSpacing = NETSCAPE_DFLT_CELLSPACING;
        nCellSpacing = pParser->ToTwips( nCellSpacing );
    }

    nPWidth = pOptions->nHSpace;
    nPHeight = pOptions->nVSpace;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
    nHSpace = (sal_uInt16)nPWidth;
    nVSpace = (sal_uInt16)nPHeight;

    bColSpec = sal_False;

    pBGBrush = pParser->CreateBrushItem(
                    pOptions->bBGColor ? &(pOptions->aBGColor) : 0,
                    pOptions->aBGImage, aEmptyStr, aEmptyStr, aEmptyStr );

    pContext = 0;
    pParentContents = 0;

    aId = pOptions->aId;
    aClass = pOptions->aClass;
    aStyle = pOptions->aStyle;
}

HTMLTable::HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
                      sal_Bool bParHead,
                      sal_Bool bHasParentSec, sal_Bool bTopTbl, sal_Bool bHasToFlw,
                      const HTMLTableOptions *pOptions ) :
    nCols( pOptions->nCols ),
    nFilledCols( 0 ),
    nWidth( pOptions->nWidth ), bPrcWidth( pOptions->bPrcWidth ),
    nHeight( pTopTab ? 0 : pOptions->nHeight ),
    nBoxes( 1 ),
    eTableAdjust( pOptions->eAdjust ),
    bTableAdjustOfTag( pTopTab ? sal_False : pOptions->bTableAdjust ),
    eVertOri( pOptions->eVertOri ),
    eFrame( pOptions->eFrame ), eRules( pOptions->eRules ),
    nCellPadding( pOptions->nCellPadding ),
    nCellSpacing( pOptions->nCellSpacing ),
    pParser( pPars ), pTopTable( pTopTab ? pTopTab : this ),
    pCaptionStartNode( 0 ),
    bFirstCell( !pTopTab ),
    bIsParentHead( bParHead ),
    bHasParentSection( bHasParentSec ), bMakeTopSubTable( bTopTbl ),
    bHasToFly( bHasToFlw ),
    bFixedCols( pOptions->nCols>0 ), bTopCaption( sal_False ),
    pLayoutInfo( 0 )
{
    InitCtor( pOptions );

    for( sal_uInt16 i=0; i<nCols; i++ )
        pColumns->Insert( new HTMLTableColumn, pColumns->Count() );
}


HTMLTable::~HTMLTable()
{
    delete pResizeDrawObjs;
    delete pDrawObjPrcWidths;

    delete pRows;
    delete pColumns;
    delete pBGBrush;
    delete pInhBGBrush;

    delete pContext;

    // pLayoutInfo wurde entweder bereits geloescht oder muss aber es
    // in den Besitz der SwTable uebergegangen.
}

SwHTMLTableLayout *HTMLTable::CreateLayoutInfo()
{
    sal_uInt16 nW = bPrcWidth ? nWidth : pParser->ToTwips( nWidth );

    sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, sal_True );
    sal_uInt16 nLeftBorderWidth =
        ((*pColumns)[0])->bLeftBorder ? GetBorderWidth( aLeftBorderLine, sal_True ) : 0;
    sal_uInt16 nRightBorderWidth =
        bRightBorder ? GetBorderWidth( aRightBorderLine, sal_True ) : 0;
    sal_uInt16 nInhLeftBorderWidth = 0;
    sal_uInt16 nInhRightBorderWidth = 0;

    pLayoutInfo = new SwHTMLTableLayout(
                        pSwTable,
                        nRows, nCols, bFixedCols, bColSpec,
                        nW, bPrcWidth, nBorder, nCellPadding,
                        nCellSpacing, eTableAdjust,
                        nLeftMargin, nRightMargin,
                        nBorderWidth, nLeftBorderWidth, nRightBorderWidth,
                        nInhLeftBorderWidth, nInhRightBorderWidth );

    sal_Bool bExportable = sal_True;
    sal_uInt16 i;
    for( i=0; i<nRows; i++ )
    {
        HTMLTableRow *pRow = (*pRows)[i];
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
            SwHTMLTableLayoutCell *pLayoutCell =
                pRow->GetCell(j)->CreateLayoutInfo();

            pLayoutInfo->SetCell( pLayoutCell, i, j );

            if( bExportable )
            {
                SwHTMLTableLayoutCnts *pLayoutCnts =
                    pLayoutCell->GetContents();
                bExportable = !pLayoutCnts ||
                              ( pLayoutCnts->GetStartNode() &&
                                !pLayoutCnts->GetNext() );
            }
        }
    }

    pLayoutInfo->SetExportable( bExportable );

    for( i=0; i<nCols; i++ )
        pLayoutInfo->SetColumn( ((*pColumns)[i])->CreateLayoutInfo(), i );

    return pLayoutInfo;
}

inline void HTMLTable::SetCaption( const SwStartNode *pStNd, sal_Bool bTop )
{
    pCaptionStartNode = pStNd;
    bTopCaption = bTop;
}

void HTMLTable::FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol,
                            const HTMLTableCnts *pCnts )
{
    sal_uInt16 nRowSpan=1;
    HTMLTableCell *pCell;
    while( ( pCell=GetCell(nRow,nCol), pCell->GetContents()==pCnts ) )
    {
        pCell->SetRowSpan( nRowSpan );
        if( pLayoutInfo )
            pLayoutInfo->GetCell(nRow,nCol)->SetRowSpan( nRowSpan );

        if( !nRow ) break;
        nRowSpan++; nRow--;
    }
}

void HTMLTable::ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan )
{
    for( sal_uInt16 i=0; i<nRowSpan; i++ )
    {
        GetCell(nRow+i,nCol)->SetProtected();
        if( pLayoutInfo )
            pLayoutInfo->GetCell(nRow+i,nCol)->SetProtected();
    }
}


// Suchen des SwStartNodes der letzten belegten Vorgaengerbox
const SwStartNode* HTMLTable::GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCol ) const
{
    const HTMLTableCnts *pPrevCnts = 0;

    if( 0==nRow )
    {
        // immer die Vorgaenger-Zelle
        if( nCol>0 )
            pPrevCnts = GetCell( 0, nCol-1 )->GetContents();
        else
            return pPrevStNd;
    }
    else if( USHRT_MAX==nRow && USHRT_MAX==nCol )
        // der Contents der letzten Zelle
        pPrevCnts = GetCell( nRows-1, nCols-1 )->GetContents();
    else
    {
        sal_uInt16 i;
        HTMLTableRow *pPrevRow = (*pRows)[nRow-1];

        // evtl. eine Zelle in der aktuellen Zeile
        i = nCol;
        while( i )
        {
            i--;
            if( 1 == pPrevRow->GetCell(i)->GetRowSpan() )
            {
                pPrevCnts = GetCell(nRow,i)->GetContents();
                break;
            }
        }

        // sonst die letzte gefuellte Zelle der Zeile davor suchen
        if( !pPrevCnts )
        {
            i = nCols;
            while( !pPrevCnts && i )
            {
                i--;
                pPrevCnts = pPrevRow->GetCell(i)->GetContents();
            }
        }
    }
    ASSERT( pPrevCnts, "keine gefuellte Vorgaenger-Zelle gefunden" );
    if( !pPrevCnts )
    {
        pPrevCnts = GetCell(0,0)->GetContents();
        if( !pPrevCnts )
            return pPrevStNd;
    }

    while( pPrevCnts->Next() )
        pPrevCnts = pPrevCnts->Next();

    return ( pPrevCnts->GetStartNode() ? pPrevCnts->GetStartNode()
               : pPrevCnts->GetTable()->GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX ) );
}


static sal_Bool IsBoxEmpty( const SwTableBox *pBox )
{
    const SwStartNode *pSttNd = pBox->GetSttNd();
    if( pSttNd &&
        pSttNd->GetIndex() + 2 == pSttNd->EndOfSectionIndex() )
    {
        const SwCntntNode *pCNd =
            pSttNd->GetNodes()[pSttNd->GetIndex()+1]->GetCntntNode();
        if( pCNd && !pCNd->Len() )
            return sal_True;
    }

    return sal_False;
}

sal_uInt16 HTMLTable::GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                   sal_Bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellPadding;

    if( nRow == 0 )
    {
        nSpace += nBorder + nCellSpacing;
        if( bSwBorders )
        {
            sal_uInt16 nTopBorderWidth =
                GetBorderWidth( aTopBorderLine, sal_True );
            if( nSpace < nTopBorderWidth )
                nSpace = nTopBorderWidth;
        }
    }
    else if( bSwBorders && ((*pRows)[nRow+nRowSpan-1])->bBottomBorder &&
             nSpace < MIN_BORDER_DIST )
    {
        ASSERT( !nCellPadding, "GetTopCellSpace: CELLPADDING!=0" );
        // Wenn die Gegenueberliegende Seite umrandet ist muessen
        // wir zumindest den minimalen Abstand zum Inhalt
        // beruecksichtigen. (Koennte man zusaetzlich auch an
        // nCellPadding festmachen.)
        nSpace = MIN_BORDER_DIST;
    }

    return nSpace;
}

sal_uInt16 HTMLTable::GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                      sal_Bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellSpacing + nCellPadding;

    if( nRow+nRowSpan == nRows )
    {
        nSpace += nBorder;

        if( bSwBorders )
        {
            sal_uInt16 nBottomBorderWidth =
                GetBorderWidth( aBottomBorderLine, sal_True );
            if( nSpace < nBottomBorderWidth )
                nSpace = nBottomBorderWidth;
        }
    }
    else if( bSwBorders )
    {
        if( ((*pRows)[nRow+nRowSpan+1])->bBottomBorder )
        {
            sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, sal_True );
            if( nSpace < nBorderWidth )
                nSpace = nBorderWidth;
        }
        else if( nRow==0 && bTopBorder && nSpace < MIN_BORDER_DIST )
        {
            ASSERT( GetBorderWidth( aTopBorderLine, sal_True ) > 0,
                    "GetBottomCellSpace: |aTopLine| == 0" );
            ASSERT( !nCellPadding, "GetBottomCellSpace: CELLPADDING!=0" );
            // Wenn die Gegenueberliegende Seite umrandet ist muessen
            // wir zumindest den minimalen Abstand zum Inhalt
            // beruecksichtigen. (Koennte man zusaetzlich auch an
            // nCellPadding festmachen.)
            nSpace = MIN_BORDER_DIST;
        }
    }

    return nSpace;
}

void HTMLTable::FixFrameFmt( SwTableBox *pBox,
                             sal_uInt16 nRow, sal_uInt16 nCol,
                             sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                             sal_Bool bFirstPara, sal_Bool bLastPara ) const
{
    SwFrmFmt *pFrmFmt = 0;      // ::com::sun::star::frame::Frame-Format
    SwVertOrient eVOri = VERT_NONE;
    const SvxBrushItem *pBGBrush = 0;   // Hintergrund
    sal_Bool bTopLine = sal_False, bBottomLine = sal_False, bLastBottomLine = sal_False;
    sal_Bool bReUsable = sal_False;     // Format nochmals verwendbar?
    sal_uInt16 nEmptyRows = 0;
    sal_Bool bHasNumFmt = sal_False;
    sal_Bool bHasValue = sal_False;
    sal_uInt32 nNumFmt;
    double nValue;

    HTMLTableColumn *pColumn = (*pColumns)[nCol];

    if( pBox->GetSttNd() )
    {
        // die Hintergrundfarbe/-grafik bestimmen
        const HTMLTableCell *pCell = GetCell( nRow, nCol );
        pBGBrush = pCell->GetBGBrush();
        if( !pBGBrush )
        {
            // Wenn die Zelle ueber mehrere Zeilen geht muss ein evtl.
            // an der Zeile gesetzter Hintergrund an die Zelle uebernommen
            // werden.
#ifndef FIX56334
            // Wenn es sich um eine Tabelle in der Tabelle handelt und
            // die Zelle ueber die gesamte Heoehe der Tabelle geht muss
            // ebenfalls der Hintergrund der Zeile uebernommen werden, weil
            // die Line von der GC (zu Recht) wegoptimiert wird.
            if( nRowSpan > 1 || (this != pTopTable && nRowSpan==nRows) )
#else
            if( nRowSpan > 1 )
#endif
            {
                pBGBrush = ((*pRows)[nRow])->GetBGBrush();
                if( !pBGBrush && this != pTopTable )
                {
                    pBGBrush = GetBGBrush();
                    if( !pBGBrush )
                        pBGBrush = GetInhBGBrush();
                }
            }
        }

        bTopLine = 0==nRow && bTopBorder && bFirstPara;
        if( ((*pRows)[nRow+nRowSpan-1])->bBottomBorder && bLastPara )
        {
            nEmptyRows = ((*pRows)[nRow+nRowSpan-1])->GetEmptyRows();
            if( nRow+nRowSpan == nRows )
                bLastBottomLine = sal_True;
            else
                bBottomLine = sal_True;
        }

        eVOri = pCell->GetVertOri();
        bHasNumFmt = pCell->GetNumFmt( nNumFmt );
        if( bHasNumFmt )
            bHasValue = pCell->GetValue( nValue );

        if( nColSpan==1 && !bTopLine && !bLastBottomLine && !nEmptyRows &&
            !pBGBrush && !bHasNumFmt )
        {
            pFrmFmt = pColumn->GetFrmFmt( bBottomLine, eVOri );
            bReUsable = !pFrmFmt;
        }
    }

    if( !pFrmFmt )
    {
        pFrmFmt = pBox->ClaimFrmFmt();

        // die Breite der Box berechnen
        SwTwips nFrmWidth = (SwTwips)pLayoutInfo->GetColumn(nCol)
                                                ->GetRelColWidth();
        for( sal_uInt16 i=1; i<nColSpan; i++ )
            nFrmWidth += (SwTwips)pLayoutInfo->GetColumn(nCol+i)
                                             ->GetRelColWidth();

        // die Umrandung nur an Edit-Boxen setzen (bei der oberen und unteren
        // Umrandung muss beruecks. werden, ob es sich um den ersten oder
        // letzen Absatz der Zelle handelt)
        if( pBox->GetSttNd() )
        {
            sal_Bool bSet = (nCellPadding > 0);

            SvxBoxItem aBoxItem;
            long nInnerFrmWidth = nFrmWidth;

            if( bTopLine )
            {
                aBoxItem.SetLine( &aTopBorderLine, BOX_LINE_TOP );
                bSet = sal_True;
            }
            if( bLastBottomLine )
            {
                aBoxItem.SetLine( &aBottomBorderLine, BOX_LINE_BOTTOM );
                bSet = sal_True;
            }
            else if( bBottomLine )
            {
                if( nEmptyRows && !aBorderLine.GetInWidth() )
                {
                    // Leere Zeilen koennen zur Zeit nur dann ueber
                    // dicke Linien simuliert werden, wenn die Linie
                    // einfach ist.
                    SvxBorderLine aThickBorderLine( aBorderLine );

                    sal_uInt16 nBorderWidth = aBorderLine.GetOutWidth();
                    nBorderWidth *= (nEmptyRows + 1);
                    SvxCSS1Parser::SetBorderWidth( aThickBorderLine,
                                                   nBorderWidth, sal_False );
                    aBoxItem.SetLine( &aThickBorderLine, BOX_LINE_BOTTOM );
                }
                else
                {
                    aBoxItem.SetLine( &aBorderLine, BOX_LINE_BOTTOM );
                }
                bSet = sal_True;
            }
            if( ((*pColumns)[nCol])->bLeftBorder )
            {
                const SvxBorderLine& rBorderLine =
                    0==nCol ? aLeftBorderLine : aBorderLine;
                aBoxItem.SetLine( &rBorderLine, BOX_LINE_LEFT );
                nInnerFrmWidth -= GetBorderWidth( rBorderLine );
                bSet = sal_True;
            }
            if( nCol+nColSpan == nCols && bRightBorder )
            {
                aBoxItem.SetLine( &aRightBorderLine, BOX_LINE_RIGHT );
                nInnerFrmWidth -= GetBorderWidth( aRightBorderLine );
                bSet = sal_True;
            }

            if( bSet )
            {
                // fix #30588#: BorderDist nicht mehr Bestandteil
                // einer Zelle mit fixer Breite
                sal_uInt16 nBDist =
                    (2*nCellPadding <= nInnerFrmWidth) ? nCellPadding
                                                      : (nInnerFrmWidth / 2);
                // wir setzen das Item nur, wenn es eine Umrandung gibt
                // oder eine ::com::sun::star::sheet::Border-Distanz vorgegeben ist. Fehlt letztere,
                // dann gibt es eine Umrandung, und wir muessen die Distanz
                // setzen
                aBoxItem.SetDistance( nBDist ? nBDist : MIN_BORDER_DIST );
                pFrmFmt->SetAttr( aBoxItem );
            }
            else
                pFrmFmt->ResetAttr( RES_BOX );

            if( pBGBrush )
            {
                pFrmFmt->SetAttr( *pBGBrush );
            }
            else
                pFrmFmt->ResetAttr( RES_BACKGROUND );

            // fix #41003#: Format nur setzten, wenn es auch einen Value
            // gibt oder die Box leer ist.
            if( bHasNumFmt && (bHasValue || IsBoxEmpty(pBox)) )
            {
                sal_Bool bLock = pFrmFmt->GetDoc()->GetNumberFormatter()
                                     ->IsTextFormat( nNumFmt );
                SfxItemSet aItemSet( *pFrmFmt->GetAttrSet().GetPool(),
                                     RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
                SvxAdjust eAdjust = SVX_ADJUST_END;
                SwCntntNode *pCNd = 0;
                if( !bLock )
                {
                    const SwStartNode *pSttNd = pBox->GetSttNd();
                    pCNd = pSttNd->GetNodes()[pSttNd->GetIndex()+1]
                                 ->GetCntntNode();
                    const SfxPoolItem *pItem;
                    if( pCNd && pCNd->GetpSwAttrSet() &&
                        SFX_ITEM_SET==pCNd->GetpSwAttrSet()->GetItemState(
                            RES_PARATR_ADJUST, sal_False, &pItem ) )
                    {
                        eAdjust = ((const SvxAdjustItem *)pItem)
                            ->GetAdjust();
                    }
                }
                aItemSet.Put( SwTblBoxNumFormat(nNumFmt) );
                if( bHasValue )
                    aItemSet.Put( SwTblBoxValue(nValue) );

                if( bLock )
                    pFrmFmt->LockModify();
                pFrmFmt->SetAttr( aItemSet );
                if( bLock )
                    pFrmFmt->UnlockModify();
                else if( pCNd && SVX_ADJUST_END != eAdjust )
                {
                    SvxAdjustItem aAdjItem( eAdjust );
                    pCNd->SetAttr( aAdjItem );
                }
            }
            else
                pFrmFmt->ResetAttr( RES_BOXATR_FORMAT );

            ASSERT( eVOri != VERT_TOP, "VERT_TOP ist nicht erlaubt!" );
            if( VERT_NONE != eVOri )
            {
                pFrmFmt->SetAttr( SwFmtVertOrient( 0, eVOri ) );
            }
            else
                pFrmFmt->ResetAttr( RES_VERT_ORIENT );

            if( bReUsable )
                pColumn->SetFrmFmt( pFrmFmt, bBottomLine, eVOri );
        }
        else
        {
            pFrmFmt->ResetAttr( RES_BOX );
            pFrmFmt->ResetAttr( RES_BACKGROUND );
            pFrmFmt->ResetAttr( RES_VERT_ORIENT );
            pFrmFmt->ResetAttr( RES_BOXATR_FORMAT );
        }
    }
    else
    {
        ASSERT( pBox->GetSttNd() ||
                SFX_ITEM_SET!=pFrmFmt->GetAttrSet().GetItemState(
                                    RES_VERT_ORIENT, sal_False ),
                "Box ohne Inhalt hat vertikale Ausrichtung" );
        pBox->ChgFrmFmt( (SwTableBoxFmt*)pFrmFmt );
    }

}

void HTMLTable::FixFillerFrameFmt( SwTableBox *pBox, sal_Bool bRight ) const
{
    SwFrmFmt *pFrmFmt = pBox->ClaimFrmFmt();

    if( bFillerTopBorder || bFillerBottomBorder ||
        (!bRight && bInhLeftBorder) || (bRight && bInhRightBorder) )
    {
        SvxBoxItem aBoxItem;
        if( bFillerTopBorder )
            aBoxItem.SetLine( &aTopBorderLine, BOX_LINE_TOP );
        if( bFillerBottomBorder )
            aBoxItem.SetLine( &aBottomBorderLine, BOX_LINE_BOTTOM );
        if( !bRight && bInhLeftBorder )
            aBoxItem.SetLine( &aInhLeftBorderLine, BOX_LINE_LEFT );
        if( bRight && bInhRightBorder )
            aBoxItem.SetLine( &aInhRightBorderLine, BOX_LINE_RIGHT );
        aBoxItem.SetDistance( MIN_BORDER_DIST );
        pFrmFmt->SetAttr( aBoxItem );
    }
    else
    {
        pFrmFmt->ResetAttr( RES_BOX );
    }

    if( GetInhBGBrush() )
        pFrmFmt->SetAttr( *GetInhBGBrush() );
    else
        pFrmFmt->ResetAttr( RES_BACKGROUND );

    pFrmFmt->ResetAttr( RES_VERT_ORIENT );
    pFrmFmt->ResetAttr( RES_BOXATR_FORMAT );
}

SwTableBox *HTMLTable::NewTableBox( const SwStartNode *pStNd,
                                    SwTableLine *pUpper ) const
{
    SwTableBox *pBox;

    if( pTopTable->pBox1 &&
        pTopTable->pBox1->GetSttNd() == pStNd )
    {
        // wenn der StartNode dem StartNode der initial angelegten Box
        // entspricht nehmen wir diese Box
        pBox = pTopTable->pBox1;
        pBox->SetUpper( pUpper );
        pTopTable->pBox1 = 0;
    }
    else
        pBox = new SwTableBox( pBoxFmt, *pStNd, pUpper );

    return pBox;
}


static void ResetLineFrmFmtAttrs( SwFrmFmt *pFrmFmt )
{
    pFrmFmt->ResetAttr( RES_FRM_SIZE );
    pFrmFmt->ResetAttr( RES_BACKGROUND );
    ASSERT( SFX_ITEM_SET!=pFrmFmt->GetAttrSet().GetItemState(
                                RES_VERT_ORIENT, sal_False ),
            "Zeile hat vertikale Ausrichtung" );
}

// !!! kann noch vereinfacht werden
SwTableLine *HTMLTable::MakeTableLine( SwTableBox *pUpper,
                                       sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                       sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableLine *pLine;
    if( this==pTopTable && !pUpper && 0==nTopRow )
        pLine = (pSwTable->GetTabLines())[0];
    else
        pLine = new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                     : pLineFmt,
                                 0, pUpper );

    HTMLTableRow *pTopRow = (*pRows)[nTopRow];
    sal_uInt16 nHeight = pTopRow->GetHeight();
    const SvxBrushItem *pBGBrush = 0;
#ifndef FIX56334
    if( this == pTopTable || nTopRow>0 || nBottomRow<nRows )
    {
        // An der Line eine Frabe zu setzen macht keinen Sinn, wenn sie
        // die auesserste und gleichzeitig einzige Zeile einer Tabelle in
        // der Tabelle ist.
#endif
        pBGBrush = pTopRow->GetBGBrush();

        if( !pBGBrush && this != pTopTable )
        {
            // Ein an einer Tabellen in der Tabelle gesetzter Hintergrund
            // wird an den Rows gesetzt. Das gilt auch fuer den Hintergrund
            // der Zelle, in dem die Tabelle vorkommt.
            pBGBrush = GetBGBrush();
            if( !pBGBrush )
                pBGBrush = GetInhBGBrush();
        }
#ifndef FIX56334
    }
#endif
    if( nTopRow==nBottomRow-1 && (nHeight || pBGBrush) )
    {
        SwTableLineFmt *pFrmFmt = (SwTableLineFmt*)pLine->ClaimFrmFmt();
        ResetLineFrmFmtAttrs( pFrmFmt );

        if( nHeight )
        {
            // Tabellenhoehe einstellen. Da es sich um eine
            // Mindesthoehe handelt, kann sie genauso wie in
            // Netscape berechnet werden, also ohne Beruecksichtigung
            // der tatsaechlichen Umrandungsbreite.
            nHeight += GetTopCellSpace( nTopRow, 1, sal_False ) +
                       GetBottomCellSpace( nTopRow, 1, sal_False );

            pFrmFmt->SetAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nHeight ) );
        }

        if( pBGBrush )
        {
            pFrmFmt->SetAttr( *pBGBrush );
        }

    }
    else if( !pLineFrmFmtNoHeight )
    {
        // sonst muessen wir die Hoehe aus dem Attribut entfernen
        // und koennen uns das Format merken
        pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

        ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
    }


    SwTableBoxes& rBoxes = pLine->GetTabBoxes();

    sal_uInt16 nStartCol = nLeftCol;
    while( nStartCol<nRightCol )
    {
        for( sal_uInt16 nRow=nTopRow; nRow<nBottomRow; nRow++ )
            (*pRows)[nRow]->SetSplitable( sal_True );

        sal_uInt16 nCol = nStartCol;
        sal_uInt16 nSplitCol = nRightCol;
        sal_Bool bSplitted = sal_False;
        while( !bSplitted )
        {
            ASSERT( nCol < nRightCol, "Zu weit gelaufen" );

            // Kann hinter der aktuellen HTML-Tabellen-Spalte gesplittet
            // werden? Wenn ja, koennte der enstehende Bereich auch noch
            // in Zeilen zerlegt werden, wenn man die naechste Spalte
            // hinzunimmt?
            sal_Bool bSplit = sal_True;
            sal_Bool bHoriSplitMayContinue = sal_False;
            sal_Bool bHoriSplitPossible = sal_False;
            for( sal_uInt16 nRow=nTopRow; nRow<nBottomRow; nRow++ )
            {
                HTMLTableCell *pCell = GetCell(nRow,nCol);
                // Gibt es in allen vorhergehenden Spalten und der gerade
                // btrachteten Zeile eine gemeinsame Zeile?
                sal_Bool bHoriSplit = (*pRows)[nRow]->IsSplitable() &&
                                  nRow+1 < nBottomRow &&
                                  1 == pCell->GetRowSpan();
                (*pRows)[nRow]->SetSplitable( bHoriSplit );

                bSplit &= ( 1 == pCell->GetColSpan() );
                if( bSplit )
                {
                    bHoriSplitPossible |= bHoriSplit;

                    // Gilt das eventuell auch noch, wenn man die naechste
                    // Spalte hinzunimmt?
                    bHoriSplit &= (nCol+1 < nRightCol &&
                                   1 == GetCell(nRow,nCol+1)->GetRowSpan());
                    bHoriSplitMayContinue |= bHoriSplit;
                }
            }

#ifndef PRODUCT
            if( nCol == nRightCol-1 )
            {
                ASSERT( bSplit, "Split-Flag falsch" );
                ASSERT( !bHoriSplitMayContinue,
                        "HoriSplitMayContinue-Flag falsch" );
                HTMLTableCell *pCell = GetCell( nTopRow, nStartCol );
                ASSERT( pCell->GetRowSpan() != (nBottomRow-nTopRow) ||
                        !bHoriSplitPossible, "HoriSplitPossible-Flag falsch" );
            }
#endif
            ASSERT( !bHoriSplitMayContinue || bHoriSplitPossible,
                    "bHoriSplitMayContinue, aber nicht bHoriSplitPossible" );

            if( bSplit )
            {
                SwTableBox* pBox = 0;
                HTMLTableCell *pCell = GetCell( nTopRow, nStartCol );
                if( pCell->GetRowSpan() == (nBottomRow-nTopRow) &&
                    pCell->GetColSpan() == (nCol+1-nStartCol) )
                {
                    // Die HTML-Tabellen-Zellen bilden genau eine Box.
                    // Dann muss hinter der Box gesplittet werden
                    nSplitCol = nCol + 1;

                    // eventuell ist die Zelle noch leer
                    if( !pCell->GetContents() )
                    {
                        ASSERT( 1==pCell->GetRowSpan(),
                                "leere Box ist nicht 1 Zeile hoch" );
                        const SwStartNode* pPrevStNd =
                            GetPrevBoxStartNode( nTopRow, nStartCol );
                        HTMLTableCnts *pCnts = new HTMLTableCnts(
                            pParser->InsertTableSection(pPrevStNd) );
                        SwHTMLTableLayoutCnts *pCntsLayoutInfo =
                            pCnts->CreateLayoutInfo();

                        pCell->SetContents( pCnts );
                        pLayoutInfo->GetCell( nTopRow, nStartCol )
                                   ->SetContents( pCntsLayoutInfo );

                        // ggf. COLSPAN beachten
                        for( sal_uInt16 j=nStartCol+1; j<nSplitCol; j++ )
                        {
                            GetCell(nTopRow,j)->SetContents( pCnts );
                            pLayoutInfo->GetCell( nTopRow, j )
                                       ->SetContents( pCntsLayoutInfo );
                        }
                    }
                    pBox = MakeTableBox( pLine, pCell->GetContents(),
                                         nTopRow, nStartCol,
                                         nBottomRow, nSplitCol );
                    bSplitted = sal_True;
                }
                else if( bHoriSplitPossible && bHoriSplitMayContinue )
                {
                    // Bis hierher lies sich die Box noch in Zeilen zerlegen
                    // und evetutell gillt das auch noch, wenn man die
                    // naechste hinzunimmt. Dann suchen wir weiter, merken
                    // uns aber die Position als moegliche Split-Position.
                    nSplitCol = nCol + 1;
                }
                else
                {
                    // Wenn sich die Box bis hier her noch in Zeilen
                    // zerlegen lies, durch hinzunahmen noch einer
                    // Spalte aber nicht mehr, dann wird genau hinter
                    // dieser Splate gesplittet. (#55447#: Das gilt
                    // insbesondere auch fuer die letzte Spalte).
                    // Hinter der aktuellen Spalte wurd ausserdem gesplittet,
                    // wenn bisher noch gar keine andere Split-Position
                    // gefunden wurde.
                    // In allen anderen Faellen wird die bisher gemerkte
                    // Split-Position benutzt.
                    if( bHoriSplitPossible || nSplitCol > nCol+1 )
                    {
                        ASSERT( !bHoriSplitMayContinue,
                                "bHoriSplitMayContinue==sal_True" );
                        ASSERT( bHoriSplitPossible || nSplitCol == nRightCol,
                                "bHoriSplitPossible-Flag sollte gesetzt sein" );

                        nSplitCol = nCol + 1;
                    }

                    // Wenn die enstehende Box nicht mehr in Zeilen zerlegt
                    // werden kann, wenn man die naechste Spalte hinzunimmt
                    // muss man ebenfalls Splitten.
                    pBox = MakeTableBox( pLine, nTopRow, nStartCol,
                                         nBottomRow, nSplitCol );
                    bSplitted = sal_True;
                }
                if( pBox )
                    rBoxes.C40_INSERT( SwTableBox, pBox, rBoxes.Count() );
            }
            nCol++;
        }
        nStartCol = nSplitCol;
    }

    return pLine;
}

// Fuellen einer SwTableBox
// !!! kann noch vereinfacht werden
SwTableBox *HTMLTable::MakeTableBox( SwTableLine *pUpper,
                                     sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                     sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableBox *pBox = new SwTableBox( pBoxFmt, 0, pUpper );
    FixFrameFmt( pBox, nTopRow, nLeftCol,
                 nTopRow-nBottomRow, nRightCol-nLeftCol );

    SwTableLines& rLines = pBox->GetTabLines();
    sal_Bool bSplitted = sal_False;

    while( !bSplitted )
    {
        sal_uInt16 nStartRow = nTopRow;
        for( sal_uInt16 i=nTopRow; i<nBottomRow; i++ )
        {
            // kann hinter der aktuellen HTML-Tabellen-Zeile gesplittet werden?
            sal_Bool bSplit = sal_True;
            HTMLTableRow *pRow = (*pRows)[i];
            for( sal_uInt16 j=nLeftCol; j<nRightCol; j++ )
            {
                bSplit = ( 1 == pRow->GetCell(j)->GetRowSpan() );
                if( !bSplit )
                    break;
            }
            if( bSplit && (nStartRow>nTopRow || i+1<nBottomRow) )
            {
                // kein Spezialfall fuer die erste Zeile neotig, da wir hier
                // in einer Box und nicht in der Tabelle sind
                SwTableLine *pLine = MakeTableLine( pBox, nStartRow, nLeftCol, i+1, nRightCol );

                rLines.C40_INSERT( SwTableLine, pLine, rLines.Count() );

                nStartRow = i+1;
                bSplitted = sal_True;
            }
        }
        if( !bSplitted )
        {
            // jetzt muessen wir die Zelle mit Gewalt Teilen

            nStartRow = nTopRow;
            while( nStartRow < nBottomRow )
            {
                sal_uInt16 nMaxRowSpan=0;
                HTMLTableRow *pStartRow = (*pRows)[nStartRow];
                HTMLTableCell *pCell;
                for( i=nLeftCol; i<nRightCol; i++ )
                    if( ( pCell=pStartRow->GetCell(i),
                          pCell->GetRowSpan() > nMaxRowSpan ) )
                        nMaxRowSpan = pCell->GetRowSpan();

                nStartRow += nMaxRowSpan;
                if( nStartRow<nBottomRow )
                {
                    HTMLTableRow *pPrevRow = (*pRows)[nStartRow-1];
                    for( i=nLeftCol; i<nRightCol; i++ )
                    {
                        if( pPrevRow->GetCell(i)->GetRowSpan() > 1 )
                        {
                            // Die Inhalte sind in der Zeile darueber (also
                            // der durch FixRowSpan bearbeiteten) verankert
                            // und koennen/mussen deshalb in den mit
                            // ProtectRowSpan bearbeitetebn Zeilen
                            // geloescht oder kopiert werden
                            HTMLTableCell *pCell = GetCell( nStartRow, i );
                            FixRowSpan( nStartRow-1, i, pCell->GetContents() );
                            ProtectRowSpan( nStartRow, i,
                                            pCell->GetRowSpan() );
                        }
                    }
                }
            }
            // und jetzt nochmal von vorne ...
        }
    }

    return pBox;
}


SwTableBox *HTMLTable::MakeTableBox( SwTableLine *pUpper,
                                     HTMLTableCnts *pCnts,
                                     sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                     sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableBox *pBox;
    sal_uInt16 nColSpan = nRightCol - nLeftCol;
    sal_uInt16 nRowSpan = nBottomRow - nTopRow;

    if( !pCnts->Next() )
    {
        // nur eine Inhalts-Section
        if( pCnts->GetStartNode() )
        {
            // und die ist keine Tabelle
            pBox = NewTableBox( pCnts->GetStartNode(), pUpper );
            pCnts->SetTableBox( pBox );
        }
        else
        {
            pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                   nRightCol-nLeftCol );
            // und die ist eine Tabelle: dann bauen wir eine neue
            // Box und fuegen die Zeilen der Tabelle in die Zeilen
            // der Box ein
            pBox = new SwTableBox( pBoxFmt, 0, pUpper );
            sal_uInt16 nAbs, nRel;
            pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
            sal_uInt16 nLSpace = pLayoutInfo->GetLeftCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nRSpace = pLayoutInfo->GetRightCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nInhSpace = pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
            pCnts->GetTable()->MakeTable( pBox, nAbs, nRel, nLSpace, nRSpace,
                                          nInhSpace );
        }
    }
    else
    {
        // mehrere Inhalts Sections: dann brauchen wir eine Box mit Zeilen
        pBox = new SwTableBox( pBoxFmt, 0, pUpper );
        SwTableLines& rLines = pBox->GetTabLines();
        sal_Bool bFirstPara = sal_True;

        while( pCnts )
        {
            if( pCnts->GetStartNode() )
            {
                // normale Absaetze werden zu einer Box in einer Zeile
                SwTableLine *pLine =
                    new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                         : pLineFmt, 0, pBox );
                if( !pLineFrmFmtNoHeight )
                {
                    // Wenn es noch kein Line-Format ohne Hoehe gibt, koennen
                    // wir uns dieses her als soleches merken
                    pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

                    ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
                }

                SwTableBox* pCntBox = NewTableBox( pCnts->GetStartNode(),
                                                   pLine );
                pCnts->SetTableBox( pCntBox );
                FixFrameFmt( pCntBox, nTopRow, nLeftCol, nRowSpan, nColSpan,
                             bFirstPara, 0==pCnts->Next() );
                pLine->GetTabBoxes().C40_INSERT( SwTableBox, pCntBox,
                                             pLine->GetTabBoxes().Count() );

                rLines.C40_INSERT( SwTableLine, pLine, rLines.Count() );
            }
            else
            {
                pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                       nRightCol-nLeftCol );
                // Tabellen werden direkt eingetragen
                sal_uInt16 nAbs, nRel;
                pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
                sal_uInt16 nLSpace = pLayoutInfo->GetLeftCellSpace( nLeftCol,
                                                                nColSpan );
                sal_uInt16 nRSpace = pLayoutInfo->GetRightCellSpace( nLeftCol,
                                                                 nColSpan );
                sal_uInt16 nInhSpace = pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
                pCnts->GetTable()->MakeTable( pBox, nAbs, nRel, nLSpace,
                                              nRSpace, nInhSpace );
            }

            pCnts = pCnts->Next();
            bFirstPara = sal_False;
        }
    }

    FixFrameFmt( pBox, nTopRow, nLeftCol, nRowSpan, nColSpan );

    return pBox;
}

void HTMLTable::InheritBorders( const HTMLTable *pParent,
                                sal_uInt16 nRow, sal_uInt16 nCol,
                                sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                                sal_Bool bFirstPara, sal_Bool bLastPara )
{
    ASSERT( nRows>0 && nCols>0 && nCurRow==nRows,
            "Wurde CloseTable nicht aufgerufen?" );

    // Die Child-Tabelle muss einen Rahmen bekommen, wenn die umgebende
    // Zelle einen Rand an der betreffenden Seite besitzt.
    // Der obere bzw. untere Rand wird nur gesetzt, wenn die Tabelle
    // ale erster bzw. letzter Absatz in der Zelle vorkommt. Ansonsten
    // Fuer den linken/rechten Rand kann noch nicht entschieden werden,
    // ob eine Umrandung der Tabelle noetig/moeglich ist, weil das davon
    // abhaengt, ob "Filler"-Zellen eingefuegt werden. Hier werden deshalb
    // erstmal nur Informationen gesammelt
    //
    if( 0==nRow && pParent->bTopBorder && bFirstPara )
    {
        bTopBorder = sal_True;
        bFillerTopBorder = sal_True; // auch Filler bekommt eine Umrandung
        aTopBorderLine = pParent->aTopBorderLine;
    }
    if( ((*pParent->pRows)[nRow+nRowSpan-1])->bBottomBorder && bLastPara )
    {
        ((*pRows)[nRows-1])->bBottomBorder = sal_True;
        bFillerBottomBorder = sal_True; // auch Filler bekommt eine Umrandung
        aBottomBorderLine =
            nRow+nRowSpan==pParent->nRows ? pParent->aBottomBorderLine
                                          : pParent->aBorderLine;
    }


    // Die Child Tabelle darf keinen oberen oder linken Rahmen bekommen,
    // wenn der bereits durch die umgebende Tabelle gesetzt ist.
    // Sie darf jedoch immer einen oberen Rand bekommen, wenn die Tabelle
    // nicht der erste Absatz in der Zelle ist.
    bTopAlwd = ( !bFirstPara || (pParent->bTopAlwd &&
                 (0==nRow || !((*pParent->pRows)[nRow-1])->bBottomBorder)) );

    // die Child-Tabelle muss die Farbe der Zelle erben, in der sie
    // vorkommt, wenn sie keine eigene besitzt
    const SvxBrushItem *pInhBG = pParent->GetCell(nRow,nCol)->GetBGBrush();
    if( !pInhBG && pParent != pTopTable &&
        pParent->GetCell(nRow,nCol)->GetRowSpan() == pParent->nRows )
    {
        // die ganze umgebende Tabelle ist eine Tabelle in der Tabelle
        // und besteht nur aus einer Line, die bei der GC (zu Recht)
        // wegoptimiert wird. Deshalb muss der Hintergrund der Line in
        // diese Tabelle uebernommen werden.
        pInhBG = ((*pParent->pRows)[nRow])->GetBGBrush();
        if( !pInhBG )
            pInhBG = pParent->GetBGBrush();
        if( !pInhBG )
            pInhBG = pParent->GetInhBGBrush();
    }
    if( pInhBG )
        pInhBGBrush = new SvxBrushItem( *pInhBG );
}

void HTMLTable::InheritVertBorders( const HTMLTable *pParent,
                                 sal_uInt16 nCol, sal_uInt16 nColSpan )
{
    sal_uInt16 nInhLeftBorderWidth = 0;
    sal_uInt16 nInhRightBorderWidth = 0;

    if( nCol+nColSpan==pParent->nCols && pParent->bRightBorder )
    {
        bInhRightBorder = sal_True; // erstmal nur merken
        aInhRightBorderLine = pParent->aRightBorderLine;
        nInhRightBorderWidth =
            GetBorderWidth( aInhRightBorderLine, sal_True ) + MIN_BORDER_DIST;
    }

    if( ((*pParent->pColumns)[nCol])->bLeftBorder )
    {
        bInhLeftBorder = sal_True;  // erstmal nur merken
        aInhLeftBorderLine = 0==nCol ? pParent->aLeftBorderLine
                                     : pParent->aBorderLine;
        nInhLeftBorderWidth =
            GetBorderWidth( aInhLeftBorderLine, sal_True ) + MIN_BORDER_DIST;
    }

    if( !bInhLeftBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhLeftBorderWidth = 2 * MIN_BORDER_DIST;
    if( !bInhRightBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhRightBorderWidth = 2 * MIN_BORDER_DIST;
    pLayoutInfo->SetInhBorderWidths( nInhLeftBorderWidth,
                                     nInhRightBorderWidth );

    bRightAlwd = ( pParent->bRightAlwd &&
                  (nCol+nColSpan==pParent->nCols ||
                   !((*pParent->pColumns)[nCol+nColSpan])->bLeftBorder) );
}

void HTMLTable::SetBorders()
{
    sal_uInt16 i;
    for( i=1; i<nCols; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_COLS==eRules ||
            ((HTML_TR_ROWS==eRules || HTML_TR_GROUPS==eRules) &&
             ((*pColumns)[i-1])->IsEndOfGroup()) )
            ((*pColumns)[i])->bLeftBorder = sal_True;

    for( i=0; i<nRows-1; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_ROWS==eRules ||
            ((HTML_TR_COLS==eRules || HTML_TR_GROUPS==eRules) &&
             ((*pRows)[i])->IsEndOfGroup()) )
            ((*pRows)[i])->bBottomBorder = sal_True;

    if( bTopAlwd && (HTML_TF_ABOVE==eFrame || HTML_TF_HSIDES==eFrame ||
                     HTML_TF_BOX==eFrame) )
        bTopBorder = sal_True;
    if( HTML_TF_BELOW==eFrame || HTML_TF_HSIDES==eFrame ||
        HTML_TF_BOX==eFrame )
        ((*pRows)[nRows-1])->bBottomBorder = sal_True;
    if( /*bRightAlwd &&*/ (HTML_TF_RHS==eFrame || HTML_TF_VSIDES==eFrame ||
                      HTML_TF_BOX==eFrame) )
        bRightBorder = sal_True;
    if( HTML_TF_LHS==eFrame || HTML_TF_VSIDES==eFrame || HTML_TF_BOX==eFrame )
        ((*pColumns)[0])->bLeftBorder = sal_True;

    for( i=0; i<nRows; i++ )
    {
        HTMLTableRow *pRow = (*pRows)[i];
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
            HTMLTableCell *pCell = pRow->GetCell(j);
            if( pCell->GetContents()  )
            {
                HTMLTableCnts *pCnts = pCell->GetContents();
                sal_Bool bFirstPara = sal_True;
                while( pCnts )
                {
                    HTMLTable *pTable = pCnts->GetTable();
                    if( pTable && !pTable->BordersSet() )
                    {
                        pTable->InheritBorders( this, i, j,
                                                pCell->GetRowSpan(),
                                                pCell->GetColSpan(),
                                                bFirstPara,
                                                0==pCnts->Next() );
                        pTable->SetBorders();
                    }
                    bFirstPara = sal_False;
                    pCnts = pCnts->Next();
                }
            }
        }
    }

    bBordersSet = sal_True;
}

sal_uInt16 HTMLTable::GetBorderWidth( const SvxBorderLine& rBLine,
                                  sal_Bool bWithDistance ) const
{
    sal_uInt16 nWidth = rBLine.GetOutWidth() + rBLine.GetInWidth() +
                    rBLine.GetDistance();
    if( bWithDistance )
    {
        if( nCellPadding )
            nWidth += nCellPadding;
        else if( nWidth )
            nWidth += MIN_BORDER_DIST;
    }

    return nWidth;
}

inline HTMLTableCell *HTMLTable::GetCell( sal_uInt16 nRow,
                                          sal_uInt16 nCell ) const
{
    ASSERT( nRow<pRows->Count(),
        "ungueltiger Zeilen-Index in HTML-Tabelle" );
    return ((*pRows)[nRow])->GetCell( nCell );
}



SvxAdjust HTMLTable::GetInheritedAdjust() const
{
    SvxAdjust eAdjust = (nCurCol<nCols ? ((*pColumns)[nCurCol])->GetAdjust()
                                       : SVX_ADJUST_END );
    if( SVX_ADJUST_END==eAdjust )
        eAdjust = ((*pRows)[nCurRow])->GetAdjust();

    return eAdjust;
}

SwVertOrient HTMLTable::GetInheritedVertOri() const
{
    // VERT_TOP ist der default!
    SwVertOrient eVOri = ((*pRows)[nCurRow])->GetVertOri();
    if( VERT_TOP==eVOri && nCurCol<nCols )
        eVOri = ((*pColumns)[nCurCol])->GetVertOri();
    if( VERT_TOP==eVOri )
        eVOri = eVertOri;

    ASSERT( eVertOri != VERT_TOP, "VERT_TOP ist nicht erlaubt!" );
    return eVOri;
}

void HTMLTable::InsertCell( HTMLTableCnts *pCnts,
                            sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                            sal_uInt16 nWidth, sal_Bool bRelWidth, sal_uInt16 nHeight,
                            SwVertOrient eVertOri, SvxBrushItem *pBGBrush,
                            sal_Bool bHasNumFmt, sal_uInt32 nNumFmt,
                            sal_Bool bHasValue, double nValue, sal_Bool bNoWrap )
{
    if( !nRowSpan || (sal_uInt32)nCurRow + nRowSpan > USHRT_MAX )
        nRowSpan = 1;

    if( !nColSpan || (sal_uInt32)nCurCol + nColSpan > USHRT_MAX )
        nColSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nColSpan;       // benoetigte Spalten
    sal_uInt16 nRowsReq = nCurRow + nRowSpan;       // benoetigte Zeilen
    sal_uInt16 i, j;

    // falls wir mehr Spalten benoetigen als wir zur Zeit haben,
    // muessen wir in allen Zeilen noch Zellen hinzufuegen
    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
            pColumns->Insert( new HTMLTableColumn, pColumns->Count() );
        for( i=0; i<nRows; i++ )
            ((*pRows)[i])->Expand( nColsReq, i<nCurRow );
        nCols = nColsReq;
        ASSERT( pColumns->Count()==nCols,
                "Anzahl der Spalten nach Expandieren stimmt nicht" );
    }
    if( nColsReq > nFilledCols )
        nFilledCols = nColsReq;

    // falls wir mehr Zeilen benoetigen als wir zur Zeit haben,
    // muessen wir noch neue Zeilen hinzufuegen
    if( nRows < nRowsReq )
    {
        for( i=nRows; i<nRowsReq; i++ )
            pRows->Insert( new HTMLTableRow(nCols), pRows->Count() );
        nRows = nRowsReq;
        ASSERT( nRows==pRows->Count(), "Zeilenzahl in Insert stimmt nicht" );
    }

    // Testen, ob eine Ueberschneidung vorliegt und diese
    // gegebenfalls beseitigen
    sal_uInt16 nSpanedCols = 0;
    if( nCurRow>0 )
    {
        HTMLTableRow *pCurRow = (*pRows)[nCurRow];
        for( i=nCurCol; i<nColsReq; i++ )
        {
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            if( pCell->GetContents() )
            {
                // Der Inhalt reicht von einer weiter oben stehenden Zelle
                // hier herein. Inhalt und Farbe der Zelle sind deshalb in
                // jedem Fall noch dort verankert und koennen deshalb
                // ueberschrieben werden bzw. von ProtectRowSpan geloescht
                // (Inhalt) oder kopiert (Farbe) werden.
                nSpanedCols = i + pCell->GetColSpan();
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
                if( pCell->GetRowSpan() > nRowSpan )
                    ProtectRowSpan( nRowsReq, i,
                                    pCell->GetRowSpan()-nRowSpan );
            }
        }
        for( i=nColsReq; i<nSpanedCols; i++ )
        {
            // Auch diese Inhalte sind in jedem Fall nich in der Zeile
            // darueber verankert.
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            FixRowSpan( nCurRow-1, i, pCell->GetContents() );
            ProtectRowSpan( nCurRow, i, pCell->GetRowSpan() );
        }
    }

    // die Zellen fuellen
    for( i=nColSpan; i>0; i-- )
        for( j=nRowSpan; j>0; j-- )
            GetCell( nRowsReq-j, nColsReq-i )
                ->Set( pCnts, j, i, eVertOri, pBGBrush,
                       bHasNumFmt, nNumFmt, bHasValue, nValue, bNoWrap );

    Size aTwipSz( bRelWidth ? 0 : nWidth, nHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    // die Breite nur in die erste Zelle setzen!
    if( nWidth )
    {
        sal_uInt16 nTmp = bRelWidth ? nWidth : (sal_uInt16)aTwipSz.Width();
        GetCell( nCurRow, nCurCol )->SetWidth( nTmp, bRelWidth );
    }

    // Ausserdem noch die Hoehe merken
    if( nHeight && 1==nRowSpan )
    {
        if( nHeight < MINLAY )
            nHeight = MINLAY;
        ((*pRows)[nCurRow])->SetHeight( (sal_uInt16)aTwipSz.Height() );
    }

    // den Spaltenzaehler hinter die neuen Zellen setzen
    nCurCol = nColsReq;
    if( nSpanedCols > nCurCol )
        nCurCol = nSpanedCols;

    // und die naechste freie Zelle suchen
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

inline void HTMLTable::CloseSection( sal_Bool bHead )
{
    // die vorhergende Section beenden, falls es schon eine Zeile gibt
    ASSERT( nCurRow<=nRows, "ungeultige aktuelle Zeile" );
    if( nCurRow>0 && nCurRow<=nRows )
        ((*pRows)[nCurRow-1])->SetEndOfGroup();
    if( bHead /*&& nCurRow==1*/ )
        bHeadlineRepeat = sal_True;
}

void HTMLTable::OpenRow( SvxAdjust eAdjust, SwVertOrient eVertOri,
                         SvxBrushItem *pBGBrush )
{
    sal_uInt16 nRowsReq = nCurRow+1;    // Anzahl benoetigter Zeilen;

    // die naechste Zeile anlegen, falls sie nicht schon da ist
    if( nRows<nRowsReq )
    {
        for( sal_uInt16 i=nRows; i<nRowsReq; i++ )
            pRows->Insert( new HTMLTableRow(nCols), pRows->Count() );
        nRows = nRowsReq;
        ASSERT( nRows==pRows->Count(),
                "Zeilenzahl in OpenRow stimmt nicht" );
    }

    HTMLTableRow *pCurRow = ((*pRows)[nCurRow]);
    pCurRow->SetAdjust( eAdjust );
    pCurRow->SetVertOri( eVertOri );
    if( pBGBrush )
        ((*pRows)[nCurRow])->SetBGBrush( pBGBrush );

    // den Spaltenzaehler wieder an den Anfang setzen
    nCurCol=0;

    // und die naechste freie Zelle suchen
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

void HTMLTable::CloseRow( sal_Bool bEmpty )
{
    ASSERT( nCurRow<nRows, "aktulle Zeile hinter dem Tabellenende" );

    // leere Zellen bekommen einfach einen etwas dickeren unteren Rand!
    if( bEmpty )
    {
        if( nCurRow > 0 )
            ((*pRows)[nCurRow-1])->IncEmptyRows();
        return;
    }

    HTMLTableRow *pRow = (*pRows)[nCurRow];

    // den COLSPAN aller leeren Zellen am Zeilenende so anpassen, dass
    // eine Zelle daraus wird. Das kann man hier machen (und auf keinen
    // Fall frueher), weill jetzt keine Zellen mehr in die Zeile eingefuegt
    // werden.
    sal_uInt16 i=nCols;
    while( i )
    {
        HTMLTableCell *pCell = pRow->GetCell(--i);
        if( !pCell->GetContents() )
        {
            sal_uInt16 nColSpan = nCols-i;
            if( nColSpan > 1 )
                pCell->SetColSpan( nColSpan );
        }
        else
            break;
    }


    nCurRow++;
}

inline void HTMLTable::CloseColGroup( sal_uInt16 nSpan, sal_uInt16 nWidth,
                                      sal_Bool bRelWidth, SvxAdjust eAdjust,
                                      SwVertOrient eVertOri )
{
    if( nSpan )
        InsertCol( nSpan, nWidth, bRelWidth, eAdjust, eVertOri );

    ASSERT( nCurCol<=nCols, "ungueltige Spalte" );
    if( nCurCol>0 && nCurCol<=nCols )
        ((*pColumns)[nCurCol-1])->SetEndOfGroup();
}

void HTMLTable::InsertCol( sal_uInt16 nSpan, sal_uInt16 nWidth, sal_Bool bRelWidth,
                           SvxAdjust eAdjust, SwVertOrient eVertOri )
{
    sal_uInt16 i;

    if( !nSpan )
        nSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nSpan;      // benoetigte Spalten

    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
            pColumns->Insert( new HTMLTableColumn, pColumns->Count() );
        nCols = nColsReq;
    }

    Size aTwipSz( bRelWidth ? 0 : nWidth, 0 );
    if( aTwipSz.Width() && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    for( i=nCurCol; i<nColsReq; i++ )
    {
        HTMLTableColumn *pCol = (*pColumns)[i];
        sal_uInt16 nTmp = bRelWidth ? nWidth : (sal_uInt16)aTwipSz.Width();
        pCol->SetWidth( nTmp, bRelWidth );
        pCol->SetAdjust( eAdjust );
        pCol->SetVertOri( eVertOri );
    }

    bColSpec = sal_True;

    nCurCol = nColsReq;
}


void HTMLTable::CloseTable()
{
    sal_uInt16 i;

    // Die Anzahl der Tabellenzeilen richtet sich nur nach den
    // <TR>-Elementen (d.h. nach nCurRow). Durch ROWSPAN aufgespannte
    // Zeilen hinter Zeile nCurRow muessen wir deshalb loeschen
    // und vor allem aber den ROWSPAN in den darueberliegenden Zeilen
    // anpassen.
    if( nRows>nCurRow )
    {
        HTMLTableRow *pPrevRow = (*pRows)[nCurRow-1];
        HTMLTableCell *pCell;
        for( i=0; i<nCols; i++ )
            if( ( pCell=pPrevRow->GetCell(i), pCell->GetRowSpan() > 1 ) )
            {
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
                ProtectRowSpan( nCurRow, i, (*pRows)[nCurRow]->GetCell(i)->GetRowSpan() );
            }
        for( i=nRows-1; i>=nCurRow; i-- )
            pRows->DeleteAndDestroy(i);
        nRows = nCurRow;
    }

    // falls die Tabelle keine Spalte hat, muessen wir eine hinzufuegen
    if( 0==nCols )
    {
        pColumns->Insert( new HTMLTableColumn, pColumns->Count() );
        for( i=0; i<nRows; i++ )
            ((*pRows)[i])->Expand(1);
        nCols = 1;
        nFilledCols = 1;
    }

    // falls die Tabelle keine Zeile hat, muessen wir eine hinzufuegen
    if( 0==nRows )
    {
        pRows->Insert( new HTMLTableRow(nCols), pRows->Count() );
        nRows = 1;
        nCurRow = 1;
    }

    if( nFilledCols < nCols )
    {
        pColumns->DeleteAndDestroy( nFilledCols, nCols-nFilledCols );
        for( i=0; i<nRows; i++ )
            ((*pRows)[i])->Shrink( nFilledCols );
        nCols = nFilledCols;
    }
}

void HTMLTable::_MakeTable( SwTableBox *pBox )
{
    SwTableLines& rLines = (pBox ? pBox->GetTabLines()
                                 : ((SwTable *)pSwTable)->GetTabLines() );

    // jetzt geht's richtig los ...
    sal_uInt16 nStartRow = 0;

    for( sal_uInt16 i=0; i<nRows; i++ )
    {
        // kann hinter der aktuellen HTML-Tabellen-Zeile gesplittet werden?
        sal_Bool bSplit = sal_True;
        HTMLTableRow *pRow = (*pRows)[i];
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
            bSplit = ( 1 == pRow->GetCell(j)->GetRowSpan() );
            if( !bSplit )
                break;
        }

        if( bSplit )
        {
            SwTableLine *pLine = MakeTableLine( pBox, nStartRow, 0, i+1, nCols );
            if( pBox || nStartRow>0 )
            {
                rLines.C40_INSERT( SwTableLine, pLine, rLines.Count() );
            }
            nStartRow = i+1;
        }
    }
}

/* Wie werden Tabellen ausgerichtet?

erste Zeile: ohne Absatz-Einzuege
zweite Zeile: mit Absatz-Einzuegen

ALIGN=          LEFT            RIGHT           CENTER          -
-------------------------------------------------------------------------
xxx bei Tabellen mit WIDTH=nn% ist die Prozent-Angabe von Bedeutung:
xxx nn = 100        HORI_FULL       HORI_FULL       HORI_FULL       HORI_FULL %
xxx             HORI_NONE       HORI_NONE       HORI_NONE %     HORI_NONE %
xxx nn < 100        Rahmen F        Rahmen F        HORI_CENTER %   HORI_LEFT %
xxx             Rahmen F        Rahmen F        HORI_CENTER %   HORI_NONE %

bei Tabellen mit WIDTH=nn% ist die Prozent-Angabe von Bedeutung:
nn = 100        HORI_LEFT       HORI_RIGHT      HORI_CENTER %   HORI_LEFT %
                HORI_LEFT_AND   HORI_RIGHT      HORI_CENTER %   HORI_LEFT_AND %
nn < 100        Rahmen F        Rahmen F        HORI_CENTER %   HORI_LEFT %
                Rahmen F        Rahmen F        HORI_CENTER %   HORI_NONE %

sonst die berechnete Breite w
w = avail*      HORI_LEFT       HORI_RIGHT      HORI_CENTER     HORI_LEFT
                HORI_LEDT_AND   HORI_RIGHT      HORI_CENTER     HORI_LEFT_AND
w < avail       Rahmen L        Rahmen L        HORI_CENTER     HORI_LEFT
                Rahmen L        Rahmen L        HORI_CENTER     HORI_NONE

xxx *) wenn fuer die Tabelle keine Groesse angegeben wurde, wird immer
xxx   HORI_FULL genommen

*/

void HTMLTable::MakeTable( SwTableBox *pBox, sal_uInt16 nAbsAvail,
                           sal_uInt16 nRelAvail, sal_uInt16 nAbsLeftSpace,
                           sal_uInt16 nAbsRightSpace, sal_uInt16 nInhAbsSpace )
{
    ASSERT( nRows>0 && nCols>0 && nCurRow==nRows,
            "Wurde CloseTable nicht aufgerufen?" );

    ASSERT( (pLayoutInfo==0) == (this==pTopTable),
            "Top-Tabelle hat keine Layout-Info oder umgekehrt" );

    if( this==pTopTable )
    {
        // Umrandung der Tabelle und aller in ihr enthaltenen berechnen
        SetBorders();

        // Schritt 1: Die benoetigten Layout-Strukturen werden angelegt
        // (inklusive Tabellen in Tabellen).
        CreateLayoutInfo();

        // Schritt 2: Die minimalen und maximalen Spaltenbreiten werden
        // berechnet (inklusive Tabellen in Tabellen). Da wir noch keine
        // Boxen haben, arabeiten wir noch auf den Start-Nodes.
        pLayoutInfo->AutoLayoutPass1();
    }

    // Schritt 3: Die tatsaechlichen Spaltenbreiten dieser Tabelle werden
    // berechnet (nicht von Tabellen in Tabellen). Dies muss jetzt schon
    // sein, damit wir entscheiden koennen ob Filler-Zellen benoetigt werden
    // oder nicht (deshalb war auch Pass1 schon noetig).
    pLayoutInfo->AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace,
                                  nAbsRightSpace, nInhAbsSpace );

    if( this!=pTopTable )
    {
        // die linke und rechte Umrandung der Tabelle kann jetzt entgueltig
        // festgelegt werden
        if( pLayoutInfo->GetRelRightFill() == 0 )
        {
            if( !bRightBorder )
            {
                // linke Umrandung von auesserer Tabelle uebernehmen
                if( bInhRightBorder )
                {
                    bRightBorder = sal_True;
                    aRightBorderLine = aInhRightBorderLine;
                }
            }
            else
            {
                // Umrandung nur setzen, wenn es erlaubt ist
                bRightBorder = bRightAlwd;
            }
        }

        if( pLayoutInfo->GetRelLeftFill() == 0 &&
            !((*pColumns)[0])->bLeftBorder &&
            bInhLeftBorder )
        {
            // ggf. rechte Umrandung von auesserer Tabelle uebernehmen
            ((*pColumns)[0])->bLeftBorder = sal_True;
            aLeftBorderLine = aInhLeftBorderLine;
        }
    }

    // Fuer die Top-Table muss die Ausrichtung gesetzt werden
    if( this==pTopTable )
    {
        SwHoriOrient eHoriOri;
        if( bForceFrame )
        {
            // Die Tabelle soll in einen Rahmen und ist auch schmaler
            // als der verfuegbare Platz und nicht 100% breit.
            // Dann kommt sie in einen Rahmen
            eHoriOri = bPrcWidth ? HORI_FULL : HORI_LEFT;
        }
        else switch( eTableAdjust )
        {
            // Die Tabelle passt entweder auf die Seite, soll aber in keinen
            // Rahmen oder sie ist Breiter als die Seite und soll deshalb
            // in keinen Rahmen

        case SVX_ADJUST_RIGHT:
            // in rechtsbuendigen Tabellen kann nicht auf den rechten
            // Rand Ruecksicht genommen werden
            eHoriOri = HORI_RIGHT;
            break;
        case SVX_ADJUST_CENTER:
            // zentrierte Tabellen nehmen keine Ruecksicht auf Raender!
            eHoriOri = HORI_CENTER;
            break;
        case SVX_ADJUST_LEFT:
        default:
            // linksbuendige Tabellen nehmen nur auf den linken Rand
            // Ruecksicht
            eHoriOri = nLeftMargin ? HORI_LEFT_AND_WIDTH : HORI_LEFT;
            break;
        }

        // das Tabellenform holen und anpassen
        SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();
        pFrmFmt->SetAttr( SwFmtHoriOrient(0,eHoriOri) );
        if( HORI_LEFT_AND_WIDTH==eHoriOri )
        {
            ASSERT( nLeftMargin || nRightMargin,
                    "Da gibt's wohl noch Reste von relativen Breiten" );

            // The right margin will be ignored anyway.
            SvxLRSpaceItem aLRItem( pSwTable->GetFrmFmt()->GetLRSpace() );
            aLRItem.SetLeft( nLeftMargin );
            aLRItem.SetRight( nRightMargin );
            pFrmFmt->SetAttr( aLRItem );
        }

        if( bPrcWidth && HORI_FULL!=eHoriOri )
        {
            pFrmFmt->LockModify();
            SwFmtFrmSize aFrmSize( pFrmFmt->GetFrmSize() );
            aFrmSize.SetWidthPercent( (sal_uInt8)nWidth );
            pFrmFmt->SetAttr( aFrmSize );
            pFrmFmt->UnlockModify();
        }
    }

    // die Default Line- und Box-Formate holen
    if( this==pTopTable )
    {
        // die erste Box merken und aus der ersten Zeile ausketten
        SwTableLine *pLine1 = (pSwTable->GetTabLines())[0];
        pBox1 = (pLine1->GetTabBoxes())[0];
        pLine1->GetTabBoxes().Remove(0);

        pLineFmt = (SwTableLineFmt*)pLine1->GetFrmFmt();
        pBoxFmt = (SwTableBoxFmt*)pBox1->GetFrmFmt();
    }
    else
    {
        pLineFmt = (SwTableLineFmt*)pTopTable->pLineFmt;
        pBoxFmt = (SwTableBoxFmt*)pTopTable->pBoxFmt;
    }

    // ggf. muessen fuer Tabellen in Tabellen "Filler"-Zellen eingefuegt
    // werden
    if( this != pTopTable &&
        ( pLayoutInfo->GetRelLeftFill() > 0  ||
          pLayoutInfo->GetRelRightFill() > 0 ) )
    {
        ASSERT( pBox, "kein TableBox fuer Tabelle in Tabelle" );

        SwTableLines& rLines = pBox->GetTabLines();

        // dazu brauchen wir erstmal ein eine neue Table-Line in der Box
        SwTableLine *pLine =
            new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                 : pLineFmt, 0, pBox );
        rLines.C40_INSERT( SwTableLine, pLine, rLines.Count() );

        // Sicherstellen, dass wie ein Format ohne Hoehe erwischt haben
        if( !pLineFrmFmtNoHeight )
        {
            // sonst muessen wir die Hoehe aus dem Attribut entfernen
            // und koennen uns das Format merken
            pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

            ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
        }

        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwTableBox *pNewBox;

        // ggf. links eine Zelle einfuegen
        if( pLayoutInfo->GetRelLeftFill() > 0 )
        {
            // pPrevStNd ist der Vorgaenger-Start-Node der Tabelle. Den
            // "Filler"-Node fuegen wir einfach dahinter ein ...
            pPrevStNd = pParser->InsertTableSection( pPrevStNd );

            pNewBox = NewTableBox( pPrevStNd, pLine );
            rBoxes.C40_INSERT( SwTableBox, pNewBox, rBoxes.Count() );
            FixFillerFrameFmt( pNewBox, sal_False );
            pLayoutInfo->SetLeftFillerBox( pNewBox );
        }

        // jetzt die Tabelle bearbeiten
        pNewBox = new SwTableBox( pBoxFmt, 0, pLine );
        rBoxes.C40_INSERT( SwTableBox, pNewBox, rBoxes.Count() );

        SwFrmFmt *pFrmFmt = pNewBox->ClaimFrmFmt();
        pFrmFmt->ResetAttr( RES_BOX );
        pFrmFmt->ResetAttr( RES_BACKGROUND );
        pFrmFmt->ResetAttr( RES_VERT_ORIENT );
        pFrmFmt->ResetAttr( RES_BOXATR_FORMAT );


        _MakeTable( pNewBox );

        // und noch ggf. rechts eine Zelle einfuegen
        if( pLayoutInfo->GetRelRightFill() > 0 )
        {
            const SwStartNode *pStNd =
                GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX );
            pStNd = pParser->InsertTableSection( pStNd );

            pNewBox = NewTableBox( pStNd, pLine );
            rBoxes.C40_INSERT( SwTableBox, pNewBox, rBoxes.Count() );

            FixFillerFrameFmt( pNewBox, sal_True );
            pLayoutInfo->SetRightFillerBox( pNewBox );
        }
    }
    else
    {
        _MakeTable( pBox );
    }

    // zum Schluss fuehren wir noch eine Garbage-Collection fuer die
    // Top-Level-Tabelle durch
    if( this==pTopTable )
    {
        if( 1==nRows && nHeight && 1==pSwTable->GetTabLines().Count() )
        {
            // Hoehe einer einzeiligen Tabelle als Mindesthoehe der
            // Zeile setzen. (War mal fixe Hoehe, aber das gibt manchmal
            // Probleme (fix #34972#) und ist auch nicht Netscape 4.0
            // konform
            nHeight = pParser->ToTwips( nHeight );
            if( nHeight < MINLAY )
                nHeight = MINLAY;

            (pSwTable->GetTabLines())[0]->ClaimFrmFmt();
            (pSwTable->GetTabLines())[0]->GetFrmFmt()
                ->SetAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nHeight ) );
        }

        if( GetBGBrush() )
            pSwTable->GetFrmFmt()->SetAttr( *GetBGBrush() );

        ((SwTable *)pSwTable)->SetHeadlineRepeat( bHeadlineRepeat );
        ((SwTable *)pSwTable)->GCLines();

        sal_Bool bIsInFlyFrame = pContext && pContext->GetFrmFmt();
        if( bIsInFlyFrame && !nWidth )
        {
            SvxAdjust eTblAdjust = GetTableAdjust(sal_False);
            if( eTblAdjust != SVX_ADJUST_LEFT &&
                eTblAdjust != SVX_ADJUST_RIGHT )
            {
                // Wenn eine Tabelle ohne Breitenangabe nicht links oder
                // rechts umflossen werden soll, dann stacken wir sie
                // in einem Rahmen mit 100%-Breite, damit ihre Groesse
                // angepasst wird. Der Rahmen darf nicht angepasst werden.
                ASSERT( HasToFly(), "Warum ist die Tabelle in einem Rahmen?" );
                sal_uInt32 nMin = pLayoutInfo->GetMin();
                if( nMin > USHRT_MAX )
                    nMin = USHRT_MAX;
                SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, (SwTwips)nMin, MINLAY );
                aFlyFrmSize.SetWidthPercent( 100 );
                pContext->GetFrmFmt()->SetAttr( aFlyFrmSize );
                bIsInFlyFrame = sal_False;
            }
            else
            {
                // Links und rechts ausgerichtete Tabellen ohne Breite
                // duerfen leider nicht in der Breite angepasst werden, denn
                // sie wuerden nur schrumpfen aber nie wachsen.
                pLayoutInfo->SetMustNotRecalc( sal_True );
                if( pContext->GetFrmFmt()->GetAnchor().GetCntntAnchor()
                    ->nNode.GetNode().FindTableNode() )
                {
                    sal_uInt32 nMax = pLayoutInfo->GetMax();
                    if( nMax > USHRT_MAX )
                        nMax = USHRT_MAX;
                    SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, (SwTwips)nMax, MINLAY );
                    pContext->GetFrmFmt()->SetAttr( aFlyFrmSize );
                    bIsInFlyFrame = sal_False;
                }
                else
                {
                    pLayoutInfo->SetMustNotResize( sal_True );
                }
            }
        }
        pLayoutInfo->SetMayBeInFlyFrame( bIsInFlyFrame );

        // Nur Tabellen mit relativer Breite oder ohne Breite muessen
        // angepasst werden.
        pLayoutInfo->SetMustResize( bPrcWidth || !nWidth );

        pLayoutInfo->SetWidths();

        ((SwTable *)pSwTable)->SetHTMLTableLayout( pLayoutInfo );

        if( pResizeDrawObjs )
        {
            sal_uInt16 nCount = pResizeDrawObjs->Count();
            for( sal_uInt16 i=0; i<nCount; i++ )
            {
                SdrObject *pObj = (*pResizeDrawObjs)[i];
                sal_uInt16 nRow = (*pDrawObjPrcWidths)[3*i];
                sal_uInt16 nCol = (*pDrawObjPrcWidths)[3*i+1];
                sal_uInt8 nPrcWidth = (sal_uInt8)(*pDrawObjPrcWidths)[3*i+2];

                SwHTMLTableLayoutCell *pLayoutCell =
                    pLayoutInfo->GetCell( nRow, nCol );
                sal_uInt16 nColSpan = pLayoutCell->GetColSpan();

                sal_uInt16 nWidth, nDummy;
                pLayoutInfo->GetAvail( nCol, nColSpan, nWidth, nDummy );
                nWidth -= pLayoutInfo->GetLeftCellSpace( nCol, nColSpan );
                nWidth -= pLayoutInfo->GetRightCellSpace( nCol, nColSpan );
                nWidth = ((long)nWidth * nPrcWidth) / 100;

                pParser->ResizeDrawObject( pObj, nWidth );
            }
        }
    }
}

void HTMLTable::SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                          sal_uInt16 nLeft, sal_uInt16 nRight,
                          const SwTable *pSwTab, sal_Bool bFrcFrame )
{
    pPrevStNd = pStNd;
    pSwTable = pSwTab;
    pContext = pCntxt;

    nLeftMargin = nLeft;
    nRightMargin = nRight;

    bForceFrame = bFrcFrame;
}

void HTMLTable::RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    if( !pResizeDrawObjs )
        pResizeDrawObjs = new SdrObjects;
    pResizeDrawObjs->C40_INSERT( SdrObject, pObj, pResizeDrawObjs->Count() );

    if( !pDrawObjPrcWidths )
        pDrawObjPrcWidths = new SvUShorts;
    pDrawObjPrcWidths->Insert( nCurRow, pDrawObjPrcWidths->Count() );
    pDrawObjPrcWidths->Insert( nCurCol, pDrawObjPrcWidths->Count() );
    pDrawObjPrcWidths->Insert( (sal_uInt16)nPrcWidth, pDrawObjPrcWidths->Count() );
}

void HTMLTable::MakeParentContents()
{
    if( !GetContext() && !HasParentSection() )
    {
        SetParentContents(
            pParser->InsertTableContents( GetIsParentHeader() ) );

        SetHasParentSection( sal_True );
    }
}

_HTMLTableContext::~_HTMLTableContext()
{
    delete pPos;
}

void _HTMLTableContext::SavePREListingXMP( SwHTMLParser& rParser )
{
    bRestartPRE = rParser.IsReadPRE();
    bRestartXMP = rParser.IsReadXMP();
    bRestartListing = rParser.IsReadListing();
    rParser.FinishPREListingXMP();
}

void _HTMLTableContext::RestorePREListingXMP( SwHTMLParser& rParser )
{
    rParser.FinishPREListingXMP();

    if( bRestartPRE )
        rParser.StartPRE();

    if( bRestartXMP )
        rParser.StartXMP();

    if( bRestartListing )
        rParser.StartListing();
}

/*  */

const SwStartNode *SwHTMLParser::InsertTableSection
    ( const SwStartNode *pPrevStNd )
{
    ASSERT( pPrevStNd, "Start-Node ist NULL" );

    pCSS1Parser->SetTDTagStyles();
    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TABLE );

    const SwStartNode *pStNd;
    if( pTable && pTable->bFirstCell )
    {
        SwNode *pNd = pDoc->GetNodes()[pPam->GetPoint()->nNode];
        pNd->GetTxtNode()->ChgFmtColl( pColl );
        pStNd = pNd->FindTableBoxStartNode();
        pTable->bFirstCell = sal_False;
    }
    else
    {
        const SwNode* pNd;
        if( pPrevStNd->IsTableNode() )
            pNd = pPrevStNd;
        else
            pNd = pPrevStNd->EndOfSectionNode();
        SwNodeIndex nIdx( *pNd, 1 );
        pStNd = pDoc->GetNodes().MakeTextSection( nIdx, SwTableBoxStartNode,
                                                  pColl );
        pTable->IncBoxCount();
    }

    pDoc->GetNodes()[pStNd->GetIndex()+1]
        ->GetCntntNode()->SetAttr( SvxFontHeightItem(40) );

    return pStNd;
}

const SwStartNode *SwHTMLParser::InsertTableSection( sal_uInt16 nPoolId )
{
    switch( nPoolId )
    {
    case RES_POOLCOLL_TABLE_HDLN:
        pCSS1Parser->SetTHTagStyles();
        break;
    case RES_POOLCOLL_TABLE:
        pCSS1Parser->SetTDTagStyles();
        break;
    }

    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( nPoolId );

    SwNode *pNd = pDoc->GetNodes()[pPam->GetPoint()->nNode];
    const SwStartNode *pStNd;
    if( pTable && pTable->bFirstCell )
    {
        pNd->GetTxtNode()->ChgFmtColl( pColl );
        pTable->bFirstCell = sal_False;
        pStNd = pNd->FindTableBoxStartNode();
    }
    else
    {
        SwTableNode *pTblNd = pNd->FindTableNode();
        SwNodeIndex aIdx( *pTblNd->EndOfSectionNode() );
        pStNd = pDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
                                                  pColl );

        pPam->GetPoint()->nNode = pStNd->GetIndex() + 1;
        SwTxtNode *pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        pPam->GetPoint()->nContent.Assign( pTxtNd, 0 );
        pTable->IncBoxCount();
    }

    return pStNd;
}

const SwStartNode *SwHTMLParser::InsertEmptyTableSection()
{
    ASSERT( !pTable || !pTable->bFirstCell,
            "erste Zelle der Tabelle nicht genutzt" );

    const SwNode *pNd = pDoc->GetNodes()[pPam->GetPoint()->nNode];
    const SwTableNode *pTblNd = pNd->FindTableNode();
    pTable->IncBoxCount();

    return pDoc->GetNodes().MakeEmptySection(
            SwNodeIndex( *pTblNd->EndOfSectionNode() ), SwTableBoxStartNode );
}

SwStartNode *SwHTMLParser::InsertTempTableCaptionSection()
{
    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TEXT );
    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    rIdx = pDoc->GetNodes().GetEndOfExtras();
    SwStartNode *pStNd = pDoc->GetNodes().MakeTextSection( rIdx,
                                          SwNormalStartNode, pColl );

    rIdx = pStNd->GetIndex() + 1;
    pPam->GetPoint()->nContent.Assign( rIdx.GetNode().GetTxtNode(), 0 );

    return pStNd;
}


xub_StrLen SwHTMLParser::StripTrailingLF()
{
    xub_StrLen nStripped = 0;

    xub_StrLen nLen = pPam->GetPoint()->nContent.GetIndex();
    if( nLen )
    {
        SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        // vorsicht, wenn Kommentare nicht uebrlesen werden!!!
        if( pTxtNd )
        {
            xub_StrLen nPos = nLen;
            xub_StrLen nLFCount = 0;
            while( nPos && '\x0a' == (pTxtNd->GetTxt()).GetChar(--nPos) )
                nLFCount++;

            if( nLFCount )
            {
// MIB 6.6.97: Warum sollte man bei leeren Absaetzen nur ein LF loeschen?
// Das stimmt doch irgendwi nicht ...
//              if( nLFCount == nLen )
//              {
//                  // nur Lfs, dann nur ein LF loeschen
//                  nLFCount = 1;
//              }
//              else if( nLFCount > 2 )
                if( nLFCount > 2 )
                {
                    // Bei Netscape entspricht ein Absatz-Ende zwei LFs
                    // (mit einem kommt man in die naechste Zeile, das
                    // zweite erzeugt eine Leerzeile) Diesen Abstand
                    // erreichen wie aber schon mit dem unteren
                    // Absatz-Abstand. Wenn nach den <BR> ein neuer
                    // Absatz aufgemacht wird, wird das Maximum des Abstands,
                    // der sich aus den BR und dem P ergibt genommen.
                    // Deshalb muessen wir 2 bzw. alle bei weniger
                    // als zweien loeschen
                    nLFCount = 2;
                }

                nPos = nLen - nLFCount;
                SwIndex nIdx( pTxtNd, nPos );
                pTxtNd->Erase( nIdx, nLFCount );
                nStripped = nLFCount;
            }
        }
    }

    return nStripped;
}

SvxBrushItem* SwHTMLParser::CreateBrushItem( const Color *pColor,
                                             const String& rImageURL,
                                             const String& rStyle,
                                             const String& rId,
                                             const String& rClass )
{
    SvxBrushItem *pBrushItem = 0;

    if( rStyle.Len() || rId.Len() || rClass.Len() )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_BACKGROUND,
                                                  RES_BACKGROUND );
        SvxCSS1PropertyInfo aPropInfo;

        if( rClass.Len() )
        {
            SvxCSS1MapEntry *pClass = pCSS1Parser->GetClass( rClass );
            if( pClass )
                aItemSet.Put( pClass->GetItemSet() );
        }

        if( rId.Len() )
        {
            SvxCSS1MapEntry *pId = pCSS1Parser->GetId( rId );
            if( pId )
                aItemSet.Put( pId->GetItemSet() );
        }

        pCSS1Parser->ParseStyleOption( rStyle, aItemSet, aPropInfo );
        const SfxPoolItem *pItem = 0;
        if( SFX_ITEM_SET == aItemSet.GetItemState( RES_BACKGROUND, sal_False,
                                                   &pItem ) )
        {
            pBrushItem = new SvxBrushItem( *((const SvxBrushItem *)pItem) );
        }
    }

    if( !pBrushItem && (pColor || rImageURL.Len()) )
    {
        pBrushItem = new SvxBrushItem();

        if( pColor )
            pBrushItem->SetColor(*pColor);

        if( rImageURL.Len() )
        {
            pBrushItem->SetGraphicLink( INetURLObject::RelToAbs(rImageURL) );
            pBrushItem->SetGraphicPos( GPOS_TILED );
        }
    }

    return pBrushItem;
}

/*  */

class _SectionSaveStruct : public SwPendingStackData
{
    sal_uInt16 nBaseFontStMinSave, nFontStMinSave, nFontStHeadStartSave;
    sal_uInt16 nDefListDeepSave, nContextStMinSave, nContextStAttrMinSave;

public:

    HTMLTable *pTable;

    _SectionSaveStruct( SwHTMLParser& rParser );
    virtual ~_SectionSaveStruct();

    sal_uInt16 GetContextStAttrMin() const { return nContextStAttrMinSave; }

    void Restore( SwHTMLParser& rParser );
};

_SectionSaveStruct::_SectionSaveStruct( SwHTMLParser& rParser ) :
    nBaseFontStMinSave(0), nFontStMinSave(0), nFontStHeadStartSave(0),
    nDefListDeepSave(0), nContextStMinSave(0), nContextStAttrMinSave(0),
    pTable( 0 )
{
    // Font-Stacks einfrieren
    nBaseFontStMinSave = rParser.nBaseFontStMin;
    rParser.nBaseFontStMin = rParser.aBaseFontStack.Count();

    nFontStMinSave = rParser.nFontStMin;
    nFontStHeadStartSave = rParser.nFontStHeadStart;
    rParser.nFontStMin = rParser.aFontStack.Count();

    // Kontext-Stack einfrieren
    nContextStMinSave = rParser.nContextStMin;
    nContextStAttrMinSave = rParser.nContextStAttrMin;
    rParser.nContextStMin = rParser.aContexts.Count();
    rParser.nContextStAttrMin = rParser.nContextStMin;

    // und noch ein par Zaehler retten
    nDefListDeepSave = rParser.nDefListDeep;
    rParser.nDefListDeep = 0;
}

_SectionSaveStruct::~_SectionSaveStruct()
{}

void _SectionSaveStruct::Restore( SwHTMLParser& rParser )
{
    // Font-Stacks wieder auftauen
    sal_uInt16 nMin = rParser.nBaseFontStMin;
    if( rParser.aBaseFontStack.Count() > nMin )
        rParser.aBaseFontStack.Remove( nMin,
                                rParser.aBaseFontStack.Count() - nMin );
    rParser.nBaseFontStMin = nBaseFontStMinSave;


    nMin = rParser.nFontStMin;
    if( rParser.aFontStack.Count() > nMin )
        rParser.aFontStack.Remove( nMin,
                            rParser.aFontStack.Count() - nMin );
    rParser.nFontStMin = nFontStMinSave;
    rParser.nFontStHeadStart = nFontStHeadStartSave;

    // Der Kontext-Stack muss schon aufgeraeumt sein!
    ASSERT( rParser.aContexts.Count() == rParser.nContextStMin &&
            rParser.aContexts.Count() == rParser.nContextStAttrMin,
            "Der Kontext-Stack wurde nicht aufgeraeumt" );
    rParser.nContextStMin = nContextStMinSave;
    rParser.nContextStAttrMin = nContextStAttrMinSave;

    // und noch ein par Zaehler rekonstruieren
    rParser.nDefListDeep = nDefListDeepSave;

    // und ein par Flags zuruecksetzen
    rParser.bNoParSpace = sal_False;
    rParser.nOpenParaToken = 0;

    if( rParser.aParaAttrs.Count() )
        rParser.aParaAttrs.Remove( 0, rParser.aParaAttrs.Count() );
}

/*  */

class _CellSaveStruct : public _SectionSaveStruct
{
    String aStyle, aId, aClass;
    String aBGImage;
    Color aBGColor;

    HTMLTableCnts* pCnts;           // Liste aller Inhalte
    HTMLTableCnts* pCurrCnts;   // der aktuelle Inhalt oder 0
    SwNodeIndex *pNoBreakEndParaIdx;// Absatz-Index eines </NOBR>

    double nValue;

    sal_uInt32 nNumFmt;

    sal_uInt16 nRowSpan, nColSpan, nWidth, nHeight;
    xub_StrLen nNoBreakEndCntntPos;     // Zeichen-Index eines </NOBR>

    SvxAdjust eAdjust;
    SwVertOrient eVertOri;

    sal_Bool bHead : 1;
    sal_Bool bPrcWidth : 1;
    sal_Bool bHasNumFmt : 1;
    sal_Bool bHasValue : 1;
    sal_Bool bBGColor : 1;
    sal_Bool bNoWrap : 1;       // NOWRAP-Option
    sal_Bool bNoBreak : 1;      // NOBREAK-Tag

public:

    _CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable, sal_Bool bHd,
                     sal_Bool bReadOpt );

    virtual ~_CellSaveStruct();

    void AddContents( HTMLTableCnts *pNewCnts );
    HTMLTableCnts *GetFirstContents() { return pCnts; }

    void ClearIsInSection() { pCurrCnts = 0; }
    sal_Bool IsInSection() const { return pCurrCnts!=0; }
    HTMLTableCnts *GetCurrContents() const { return pCurrCnts; }

    void InsertCell( SwHTMLParser& rParser, HTMLTable *pCurTable );

    sal_Bool IsHeaderCell() const { return bHead; }

    void StartNoBreak( const SwPosition& rPos );
    void EndNoBreak( const SwPosition& rPos );
    void CheckNoBreak( const SwPosition& rPos, SwDoc *pDoc );
};


_CellSaveStruct::_CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable,
                                  sal_Bool bHd, sal_Bool bReadOpt ) :
    _SectionSaveStruct( rParser ),
    pCnts( 0 ), pCurrCnts( 0 ), pNoBreakEndParaIdx( 0 ),
    nValue( 0.0 ),
    nNumFmt( 0 ),
    nRowSpan( 1 ), nColSpan( 1 ), nWidth( 0 ), nHeight( 0 ),
    nNoBreakEndCntntPos( 0 ),
    eAdjust( pCurTable->GetInheritedAdjust() ),
    bHead( bHd ),
    eVertOri( pCurTable->GetInheritedVertOri() ),
    bPrcWidth( sal_False ), bBGColor( sal_False ),
    bHasNumFmt( sal_False ), bHasValue( sal_False ),
    bNoWrap( sal_False ), bNoBreak( sal_False )
{
    String aNumFmt, aValue;

    if( bReadOpt )
    {
        const HTMLOptions *pOptions = rParser.GetOptions();
        for( sal_uInt16 i = pOptions->Count(); i; )
        {
            const HTMLOption *pOption = (*pOptions)[--i];
            switch( pOption->GetToken() )
            {
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_COLSPAN:
                nColSpan = (sal_uInt16)pOption->GetNumber();
                break;
            case HTML_O_ROWSPAN:
                nRowSpan = (sal_uInt16)pOption->GetNumber();
                break;
            case HTML_O_ALIGN:
                eAdjust = (SvxAdjust)pOption->GetEnum(
                                        aHTMLPAlignTable, eAdjust );
                break;
            case HTML_O_VALIGN:
                eVertOri = (SwVertOrient)pOption->GetEnum(
                                        aHTMLTblVAlignTable, eVertOri );
                break;
            case HTML_O_WIDTH:
                nWidth = (sal_uInt16)pOption->GetNumber();  // nur fuer Netscape
                bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                break;
            case HTML_O_HEIGHT:
                nHeight = (sal_uInt16)pOption->GetNumber(); // nur fuer Netscape
                if( pOption->GetString().Search('%') != STRING_NOTFOUND)
                    nHeight = 0;    // keine %-Angaben beruecksichtigen
                break;
            case HTML_O_BGCOLOR:
                // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netscape
                // ignorieren, bei allen anderen Tags *wirklich* nicht.
                if( pOption->GetString().Len() )
                {
                    pOption->GetColor( aBGColor );
                    bBGColor = sal_True;
                }
                break;
            case HTML_O_BACKGROUND:
                aBGImage = pOption->GetString();
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
            case HTML_O_SDNUM:
                aNumFmt = pOption->GetString();
                bHasNumFmt = sal_True;
                break;
            case HTML_O_SDVAL:
                bHasValue = sal_True;
                aValue = pOption->GetString();
                break;
            case HTML_O_NOWRAP:
                bNoWrap = sal_True;
                break;
            }
        }

        if( aId.Len() )
            rParser.InsertBookmark( aId );
    }

    if( bHasNumFmt )
    {
        LanguageType eLang;
        nValue = rParser.GetTableDataOptionsValNum(
                            nNumFmt, eLang, aValue, aNumFmt,
                            *rParser.pDoc->GetNumberFormatter() );
    }

    // einen neuen Kontext anlegen, aber das ::com::sun::star::drawing::Alignment-Attribut
    // nicht dort verankern, weil es noch ger keine Section gibt, in der
    // es gibt.
    sal_uInt16 nToken, nColl;
    if( bHead )
    {
        nToken = HTML_TABLEHEADER_ON;
        nColl = RES_POOLCOLL_TABLE_HDLN;
    }
    else
    {
        nToken = HTML_TABLEDATA_ON;
        nColl = RES_POOLCOLL_TABLE;
    }
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken, nColl, aEmptyStr, sal_True );
    if( SVX_ADJUST_END != eAdjust )
        rParser.InsertAttr( &rParser.aAttrTab.pAdjust, SvxAdjustItem(eAdjust),
                            pCntxt );

    if( rParser.HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( rParser.pDoc->GetAttrPool(),
                             rParser.pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( rParser.ParseStyleOptions( aStyle, aId, aClass, aItemSet,
                                       aPropInfo ) )
            rParser.InsertAttrs( aItemSet, aPropInfo, pCntxt );
    }

    rParser.SplitPREListingXMP( pCntxt );

    rParser.PushContext( pCntxt );
}

_CellSaveStruct::~_CellSaveStruct()
{
    delete pNoBreakEndParaIdx;
}

void _CellSaveStruct::AddContents( HTMLTableCnts *pNewCnts )
{
    if( pCnts )
        pCnts->Add( pNewCnts );
    else
        pCnts = pNewCnts;

    pCurrCnts = pNewCnts;
}

void _CellSaveStruct::InsertCell( SwHTMLParser& rParser,
                                  HTMLTable *pCurTable )
{
#ifndef PRODUCT
    // Die Attribute muessen schon beim Auefrauemen des Kontext-Stacks
    // entfernt worden sein, sonst ist etwas schiefgelaufen. Das
    // Checken wir mal eben ...
    // MIB 8.1.98: Wenn ausserhalb einer Zelle Attribute geoeffnet
    // wurden stehen diese noch in der Attribut-Tabelle und werden erst
    // ganz zum Schluss durch die CleanContext-Aufrufe in BuildTable
    // geloescht. Damit es in diesem Fall keine Asserts gibt findet dann
    // keine Ueberpruefung statt. Erkennen tut man diesen Fall an
    // nContextStAttrMin: Der gemerkte Wert nContextStAttrMinSave ist der
    // Wert, den nContextStAttrMin beim Start der Tabelle hatte. Und
    // der aktuelle Wert von nContextStAttrMin entspricht der Anzahl der
    // Kontexte, die beim Start der Zelle vorgefunden wurden. Sind beide
    // Werte unterschiedlich, wurden ausserhalb der Zelle Kontexte
    // angelegt und wir ueberpruefen nichts.

    if( rParser.nContextStAttrMin == GetContextStAttrMin() )
    {
        _HTMLAttr** pTbl = (_HTMLAttr**)&rParser.aAttrTab;

        for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
            nCnt--; ++pTbl )
        {
            ASSERT( !*pTbl, "Die Attribut-Tabelle ist nicht leer" );
        }
    }
#endif

    // jetzt muessen wir noch die Zelle an der aktuellen Position einfuegen
    SvxBrushItem *pBrushItem =
        rParser.CreateBrushItem( bBGColor ? &aBGColor : 0, aBGImage,
                                 aStyle, aId, aClass );
    pCurTable->InsertCell( pCnts, nRowSpan, nColSpan, nWidth,
                           bPrcWidth, nHeight, eVertOri, pBrushItem,
                           bHasNumFmt, nNumFmt, bHasValue, nValue,
                           bNoWrap );
    Restore( rParser );
}

void _CellSaveStruct::StartNoBreak( const SwPosition& rPos )
{
    if( !pCnts ||
        (!rPos.nContent.GetIndex() && pCurrCnts==pCnts &&
         pCnts->GetStartNode() &&
         pCnts->GetStartNode()->GetIndex() + 1 ==
            rPos.nNode.GetIndex()) )
    {
        bNoBreak = sal_True;
    }
}

void _CellSaveStruct::EndNoBreak( const SwPosition& rPos )
{
    if( bNoBreak )
    {
        delete pNoBreakEndParaIdx;
        pNoBreakEndParaIdx = new SwNodeIndex( rPos.nNode );
        nNoBreakEndCntntPos = rPos.nContent.GetIndex();
        bNoBreak = sal_False;
    }
}

void _CellSaveStruct::CheckNoBreak( const SwPosition& rPos, SwDoc *pDoc )
{
    if( pCnts && pCurrCnts==pCnts )
    {
        if( bNoBreak )
        {
            // <NOBR> wurde nicht beendet
            pCnts->SetNoBreak();
        }
        else if( pNoBreakEndParaIdx &&
                 pNoBreakEndParaIdx->GetIndex() == rPos.nNode.GetIndex() )
        {
            if( nNoBreakEndCntntPos == rPos.nContent.GetIndex() )
            {
                // <NOBR> wurde unmittelbar vor dem Zellen-Ende beendet
                pCnts->SetNoBreak();
            }
            else if( nNoBreakEndCntntPos + 1 == rPos.nContent.GetIndex() )
            {
                const SwTxtNode *pTxtNd =
                    pDoc->GetNodes()[rPos.nNode]->GetTxtNode();
                if( pTxtNd )
                {
                    register sal_Unicode cLast =
                            pTxtNd->GetTxt().GetChar(nNoBreakEndCntntPos);
                    if( ' '==cLast || '\x0a'==cLast )
                    {
                        // Zwischem dem </NOBR> und dem Zellen-Ende gibt es nur
                        // ein Blank oder einen Zeilenumbruch.
                        pCnts->SetNoBreak();
                    }
                }
            }
        }
    }
}



HTMLTableCnts *SwHTMLParser::InsertTableContents(
                                        sal_Bool bHead )
{
    // eine neue Section anlegen, der PaM steht dann darin
    const SwStartNode *pStNd =
        InsertTableSection( bHead ? RES_POOLCOLL_TABLE_HDLN
                                           : RES_POOLCOLL_TABLE );

    if( GetNumInfo().GetNumRule() )
    {
        // 1. Absatz auf nicht numeriert setzen
        SetNodeNum( GetNumInfo().GetLevel() | NO_NUMLEVEL );
    }

    // Attributierungs-Anfang neu setzen
    const SwNodeIndex& rSttPara = pPam->GetPoint()->nNode;
    xub_StrLen nSttCnt = pPam->GetPoint()->nContent.GetIndex();

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
        nCnt--; ++pTbl )
    {

        _HTMLAttr *pAttr = *pTbl;
        while( pAttr )
        {
            ASSERT( !pAttr->GetPrev(), "Attribut hat Previous-Liste" );
            pAttr->nSttPara = rSttPara;
            pAttr->nEndPara = rSttPara;
            pAttr->nSttCntnt = nSttCnt;
            pAttr->nEndCntnt = nSttCnt;

            pAttr = pAttr->GetNext();
        }
    }

    return new HTMLTableCnts( pStNd );
}

sal_uInt16 SwHTMLParser::IncGrfsThatResizeTable()
{
    return pTable ? pTable->IncGrfsThatResize() : 0;
}

void SwHTMLParser::RegisterDrawObjectToTable( HTMLTable *pCurTable,
                                        SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    pCurTable->RegisterDrawObject( pObj, nPrcWidth );
}

void SwHTMLParser::BuildTableCell( HTMLTable *pCurTable, sal_Bool bReadOptions,
                                   sal_Bool bHead )
{
    if( !IsParserWorking() && !pPendStack )
        return;

    _CellSaveStruct* pSaveStruct;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    if( pPendStack )
    {
        pSaveStruct = (_CellSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        // <TH> bzw. <TD> wurde bereits gelesen
        if( pTable->IsOverflowing() )
        {
            SaveState( 0 );
            return;
        }

        if( !pCurTable->GetContext() )
        {
//          MIB: 5.9.2000: Do we need this any longer?
//          bIsWriterDoc = sal_True;
            sal_Bool bTopTable = pTable==pCurTable;

            // die Tabelle besitzt noch keinen Inhalt, d.h. die eigentliche
            // Tabelle muss erst noch angelegt werden

            String aStyle, aId, aClass;
            static sal_uInt16 aWhichIds[] =
            {
                RES_PARATR_SPLIT,   RES_PARATR_SPLIT,
                RES_PAGEDESC,       RES_PAGEDESC,
                RES_BREAK,          RES_BREAK,
                RES_BACKGROUND,     RES_BACKGROUND,
                RES_KEEP,           RES_KEEP,
                RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
                0
            };

            SfxItemSet aItemSet( pDoc->GetAttrPool(), aWhichIds );
            SvxCSS1PropertyInfo aPropInfo;

            sal_Bool bStyleParsed = ParseStyleOptions( pCurTable->GetStyle(),
                                                   pCurTable->GetId(),
                                                   pCurTable->GetClass(),
                                                   aItemSet, aPropInfo );
            const SfxPoolItem *pItem = 0;
            if( bStyleParsed )
            {
                if( SFX_ITEM_SET == aItemSet.GetItemState(
                                        RES_BACKGROUND, sal_False, &pItem ) )
                {
                    pCurTable->SetBGBrush( *(const SvxBrushItem *)pItem );
                    aItemSet.ClearItem( RES_BACKGROUND );
                }
                if( SFX_ITEM_SET == aItemSet.GetItemState(
                                        RES_PARATR_SPLIT, sal_False, &pItem ) )
                {
                    aItemSet.Put(
                        SwFmtLayoutSplit( ((const SvxFmtSplitItem *)pItem)
                                                ->GetValue() ) );
                    aItemSet.ClearItem( RES_PARATR_SPLIT );
                }
            }

            // Den linken/rechten Absatzeinzug ermitteln
            sal_uInt16 nLeftSpace = 0;
            sal_uInt16 nRightSpace = 0;
            short nIndent;
            GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

            // die aktuelle Position an die wir irgendwann zurueckkehren
            SwPosition *pSavePos = 0;
            sal_Bool bForceFrame = sal_False;
            sal_Bool bAppended = sal_False;
            sal_Bool bParentLFStripped = sal_False;
            if( bTopTable )
            {
                // Sind wir in einer Tabelle in der Tabelle?
                const SwNode *pNd = pDoc->GetNodes()[pPam->GetPoint()->nNode];
                const SwTableNode *pTblNd = pNd->FindTableNode();

                SvxAdjust eTblAdjust = pTable->GetTableAdjust(sal_False);

                // Wenn das Table-Tag in einem Rahmen stand, den es nicht
                // mehr gibt, muessen wir die Tabelle in einen Rahmen
                // stecken.
                if( pCurTable->IsMakeTopSubTable() && pTblNd!=0 )
                    pCurTable->SetHasToFly();

                // Wenn die Tabelle links oder rechts ausgerivchtet ist,
                // oder in einen Rahmen soll, dann kommt sie auch in einen
                // solchen.
                bForceFrame = eTblAdjust == SVX_ADJUST_LEFT ||
                              eTblAdjust == SVX_ADJUST_RIGHT ||
                              pCurTable->HasToFly();

                // Da Tabelle in Tabelle nicht geht, muss die Tabelle
                // entweder in einen Rahmen, oder sie steht in einem
                // Rahmen sie steht in keiner anderen Tabelle.
                ASSERT( bForceFrame || !pTblNd,
                        "Tabelle in der Tabelle geht nur mit oder in Rahmen" );

                // Entweder kommt die Tabelle in keinen Rahmen und befindet
                // sich in keinem Rahmen (wird also durch Zellen simuliert),
                // oder es gibt bereits Inhalt an der entsprechenden Stelle.
                ASSERT( !bForceFrame || pCurTable->HasParentSection(),
                        "Tabelle im Rahmen hat keine Umgebung!" );
//              SCHOEN WAER'S, aber wie bekommen den Inhalt nicht zurueck
//              in die umgebende Zelle
//              if( bForceFrame && !pCurTable->HasParentSection() )
//              {
//                  pCurTable->SetParentContents(
//                      InsertTableContents( sal_False, SVX_ADJUST_END ) );
//                  pCurTable->SetHasParentSection( sal_True );
//              }

                sal_Bool bAppend = sal_False;
                if( bForceFrame )
                {
                    // Wenn die Tabelle in einen Rahmen kommt, muss
                    // nur ein neuer Absatz aufgemacht werden, wenn
                    // der Absatz Rahmen ohne Umlauf enthaelt.
                    bAppend = HasCurrentParaFlys(sal_True);
                }
                else
                {
                    // Sonst muss ein neuer Absatz aufgemacht werden,
                    // wenn der Absatz nicht leer ist, oder Rahmen
                    // oder ::com::sun::star::text::Bookmarks enthaelt.
                    bAppend =
                        pPam->GetPoint()->nContent.GetIndex() ||
                        HasCurrentParaFlys() ||
                        HasCurrentParaBookmarks();
                }
                if( bAppend )
                {
                    if( !pPam->GetPoint()->nContent.GetIndex() )
                    {
                        pDoc->SetTxtFmtColl( *pPam,
                            pCSS1Parser->GetTxtCollFromPool(RES_POOLCOLL_STANDARD) );
                        _HTMLAttr* pTmp =
                            new _HTMLAttr( *pPam->GetPoint(),
                                            SvxFontHeightItem( 40 ) );
                        aSetAttrTab.Insert( pTmp, aSetAttrTab.Count() );
                        pTmp = new _HTMLAttr( *pPam->GetPoint(),
                                            SvxULSpaceItem( 0, 0 ) );
                        aSetAttrTab.Insert( pTmp, 0 ); // ja, 0, weil schon
                                                        // vom Tabellenende vorher
                                                        // was gesetzt sein kann.
                    }
                    AppendTxtNode( AM_NOSPACE );
                    bAppended = sal_True;
                }
                else if( aParaAttrs.Count() )
                {
                    if( !bForceFrame )
                    {
                        // Der Absatz wird gleich hinter die Tabelle
                        // verschoben. Deshalb entfernen wir alle harten
                        // Attribute des Absatzes

                        for( sal_uInt16 i=0; i<aParaAttrs.Count(); i++ )
                            aParaAttrs[i]->Invalidate();
                    }

                    aParaAttrs.Remove( 0, aParaAttrs.Count() );
                }

                pSavePos = new SwPosition( *pPam->GetPoint() );
            }
            else if( pCurTable->HasParentSection() )
            {
                bParentLFStripped = StripTrailingLF() > 0;

                // Absaetze bzw. ueberschriften beeenden
                nOpenParaToken = 0;
                nFontStHeadStart = nFontStMin;

                // die harten Attribute an diesem Absatz werden nie mehr ungueltig
                if( aParaAttrs.Count() )
                    aParaAttrs.Remove( 0, aParaAttrs.Count() );

#ifndef NUM_RELSPACE
                if( GetNumInfo().GetNumRule() )
                    UpdateNumRuleInTable();
#endif
            }

            // einen Tabellen Kontext anlegen
            _HTMLTableContext *pTCntxt =
                        new _HTMLTableContext( pSavePos, nContextStMin,
                                               nContextStAttrMin );

            // alle noch offenen Attribute beenden und hinter der Tabelle
            // neu aufspannen
            _HTMLAttrs *pPostIts = 0;
            if( !bForceFrame && (bTopTable || pCurTable->HasParentSection()) )
            {
                SplitAttrTab( pTCntxt->aAttrTab, bTopTable );
                // Wenn wir einen schon vorhandenen Absatz verwenden, duerfen
                // in den keine PostIts eingefuegt werden, weil der Absatz
                // ja hinter die Tabelle wandert. Sie werden deshalb in den
                // ersten Absatz der Tabelle verschoben.
                // Bei Tabellen in Tabellen duerfen ebenfalls keine PostIts
                // in einen noch leeren Absatz eingefuegt werden, weil
                // der sonat nicht geloescht wird.
                if( (bTopTable && !bAppended) ||
                    (!bTopTable && !bParentLFStripped &&
                     !pPam->GetPoint()->nContent.GetIndex()) )
                    pPostIts = new _HTMLAttrs;
                SetAttr( bTopTable, bTopTable, pPostIts );
            }
            else
            {
                SaveAttrTab( pTCntxt->aAttrTab );
                if( bTopTable && !bAppended )
                {
                    pPostIts = new _HTMLAttrs;
                    SetAttr( sal_True, sal_True, pPostIts );
                }
            }
            bNoParSpace = sal_False;

            // Aktuelle Numerierung retten und auschalten.
            pTCntxt->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
            pTCntxt->SavePREListingXMP( *this );

            if( bTopTable )
            {
                if( bForceFrame )
                {
                    // Die Tabelle soll in einen Rahmen geschaufelt werden.

                    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
                    if( !pCurTable->IsNewDoc() )
                        Reader::ResetFrmFmtAttrs( aFrmSet );

                    SwSurround eSurround = SURROUND_NONE;
                    SwHoriOrient eHori;

                    switch( pCurTable->GetTableAdjust(sal_True) )
                    {
                    case SVX_ADJUST_RIGHT:
                        eHori = HORI_RIGHT;
                        eSurround = SURROUND_LEFT;
                        break;
                    case SVX_ADJUST_CENTER:
                        eHori = HORI_CENTER;
                        break;
                    case SVX_ADJUST_LEFT:
                        eSurround = SURROUND_RIGHT;
                    default:
                        eHori = HORI_LEFT;
                        break;
                    }
                    SetAnchorAndAdjustment( VERT_NONE, eHori, aFrmSet,
                                            sal_True );
                    aFrmSet.Put( SwFmtSurround(eSurround) );

                    SwFmtFrmSize aFrmSize( ATT_VAR_SIZE, 20*MM50, MINLAY );
                    aFrmSize.SetWidthPercent( 100 );
                    aFrmSet.Put( aFrmSize );

                    sal_uInt16 nSpace = pCurTable->GetHSpace();
                    if( nSpace )
                        aFrmSet.Put( SvxLRSpaceItem(nSpace,nSpace) );
                    nSpace = pCurTable->GetVSpace();
                    if( nSpace )
                        aFrmSet.Put( SvxULSpaceItem(nSpace,nSpace) );

                    RndStdIds eAnchorId = ((const SwFmtAnchor&)aFrmSet.
                                                Get( RES_ANCHOR )).
                                                GetAnchorId();
                    SwFrmFmt *pFrmFmt =  pDoc->MakeFlySection(
                                eAnchorId, pPam->GetPoint(), &aFrmSet );

                    pTCntxt->SetFrmFmt( pFrmFmt );
                    const SwFmtCntnt& rFlyCntnt = pFrmFmt->GetCntnt();
                    pPam->GetPoint()->nNode = *rFlyCntnt.GetCntntIdx();
                    SwCntntNode *pCNd =
                        pDoc->GetNodes().GoNext( &(pPam->GetPoint()->nNode) );
                    pPam->GetPoint()->nContent.Assign( pCNd, 0 );

                    // automatisch verankerte Rahmen muessen noch um
                    // eine Position nach vorne verschoben werden.
                    //if( FLY_AUTO_CNTNT==eAnchorId )
                    //  aMoveFlyFrms.C40_INSERT( SwFrmFmt, pFrmFmt,
                    //                           aMoveFlyFrms.Count() );
                }

                // eine SwTable mit einer Box anlegen und den PaM in den
                // Inhalt der Box-Section bewegen (der Ausrichtungs-Parameter
                // ist erstmal nur ein Dummy und wird spaeter noch richtig
                // gesetzt)
                ASSERT( !pPam->GetPoint()->nContent.GetIndex(),
                        "Der Absatz hinter der Tabelle ist nicht leer!" );
                const SwTable* pSwTable = pDoc->InsertTable( *pPam->GetPoint(),
                                                         1, 1, HORI_LEFT );

                if( bForceFrame )
                {
                    SwNodeIndex aDstIdx( pPam->GetPoint()->nNode );
                    pPam->Move( fnMoveBackward );
                    pDoc->GetNodes().Delete( aDstIdx );
                }
                else
                {
                    if( bStyleParsed )
                    {
                        pCSS1Parser->SetFmtBreak( aItemSet, aPropInfo );
                        pSwTable->GetFrmFmt()->SetAttr( aItemSet );
                    }
                    pPam->Move( fnMoveBackward );
                }

                const SwNode *pNd = pDoc->GetNodes()[pPam->GetPoint()->nNode];
                if( !bAppended && !bForceFrame )
                {
                    SwTxtNode* pOldTxtNd =
                        pDoc->GetNodes()[pSavePos->nNode]->GetTxtNode();
                    ASSERT( pOldTxtNd, "Wieso stehen wir in keinem Txt-Node?" );
                    SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();

                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pOldTxtNd->GetSwAttrSet()
                            .GetItemState( RES_PAGEDESC, sal_False, &pItem ) &&
                        ((SwFmtPageDesc *)pItem)->GetPageDesc() )
                    {
                        pFrmFmt->SetAttr( *pItem );
                        pOldTxtNd->ResetAttr( RES_PAGEDESC );
                    }
                    if( SFX_ITEM_SET == pOldTxtNd->GetSwAttrSet()
                            .GetItemState( RES_BREAK, sal_True, &pItem ) )
                    {
                        switch( ((SvxFmtBreakItem *)pItem)->GetBreak() )
                        {
                        case SVX_BREAK_PAGE_BEFORE:
                        case SVX_BREAK_PAGE_AFTER:
                        case SVX_BREAK_PAGE_BOTH:
                            pFrmFmt->SetAttr( *pItem );
                            pOldTxtNd->ResetAttr( RES_BREAK );
                        }
                    }
                }

                if( !bAppended && pPostIts )
                {
                    // noch vorhandene PostIts in den ersten Absatz
                    // der Tabelle setzen
                    InsertAttrs( *pPostIts );
                    delete pPostIts;
                    pPostIts = 0;
                }

                pTCntxt->SetTableNode( (SwTableNode *)pNd->FindTableNode() );

                pCurTable->SetTable( pTCntxt->GetTableNode(), pTCntxt,
                                     nLeftSpace, nRightSpace,
                                     pSwTable, bForceFrame );

                ASSERT( !pPostIts, "ubenutzte PostIts" );
            }
            else
            {
                // noch offene Bereiche muessen noch entfernt werden
                if( EndSections( bParentLFStripped ) )
                    bParentLFStripped = sal_False;

                if( pCurTable->HasParentSection() )
                {
                    // dannach entfernen wir ein ggf. zu viel vorhandenen
                    // leeren Absatz, aber nur, wenn er schon vor dem
                    // entfernen von LFs leer war
                    if( !bParentLFStripped )
                        StripTrailingPara();

                    if( pPostIts )
                    {
                        // noch vorhandene PostIts an das Ende des jetzt
                        // aktuellen Absatzes schieben
                        InsertAttrs( *pPostIts );
                        delete pPostIts;
                        pPostIts = 0;
                    }
                }

                const SwNode *pNd = pDoc->GetNodes()[pPam->GetPoint()->nNode];
                const SwStartNode *pStNd = (pTable->bFirstCell ? pNd->FindTableNode()
                                                            : pNd->FindTableBoxStartNode() );

                pCurTable->SetTable( pStNd, pTCntxt, nLeftSpace, nRightSpace );
            }

            // Den Kontext-Stack einfrieren, denn es koennen auch mal
            // irgendwo ausserhalb von Zellen Attribute gesetzt werden.
            // Darf nicht frueher passieren, weil eventuell noch im
            // Stack gesucht wird!!!
            nContextStMin = aContexts.Count();
            nContextStAttrMin = nContextStMin;
        }

        pSaveStruct = new _CellSaveStruct( *this, pCurTable, bHead,
                                            bReadOptions );

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // Token nach <TABLE>

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        ASSERT( pPendStack || !bCallNextToken || pSaveStruct->IsInSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken && pSaveStruct->IsInSection() )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
        case HTML_TABLEROW_ON:
        case HTML_TABLEROW_OFF:
        case HTML_THEAD_ON:
        case HTML_THEAD_OFF:
        case HTML_TFOOT_ON:
        case HTML_TFOOT_OFF:
        case HTML_TBODY_ON:
        case HTML_TBODY_OFF:
        case HTML_TABLE_OFF:
            SkipToken(-1);
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
            bDone = sal_True;
            break;
        case HTML_TABLE_ON:
            {
                sal_Bool bTopTable = sal_False;
                sal_Bool bHasToFly = sal_False;
                SvxAdjust eTabAdjust = SVX_ADJUST_END;
                if( !pPendStack )
                {
                    // nur wenn eine neue Tabelle aufgemacht wird, aber
                    // nicht wenn nach einem Pending in der Tabelle
                    // weitergelesen wird!
                    pSaveStruct->pTable = pTable;

                    // HACK: Eine Section fuer eine Tabelle anlegen, die
                    // in einen Rahmen kommt.
                    if( !pSaveStruct->IsInSection() )
                    {
                        // Diese Schleife muss vorwartes sein, weil die
                        // erste Option immer gewinnt.
                        sal_Bool bNeedsSection = sal_False;
                        const HTMLOptions *pOptions = GetOptions();
                        for( sal_uInt16 i=0; i<pOptions->Count(); i++ )
                        {
                            const HTMLOption *pOption = (*pOptions)[i];
                            if( HTML_O_ALIGN==pOption->GetToken() )
                            {
                                SvxAdjust eAdjust =
                                    (SvxAdjust)pOption->GetEnum(
                                            aHTMLPAlignTable, SVX_ADJUST_END );
                                bNeedsSection = SVX_ADJUST_LEFT == eAdjust ||
                                                SVX_ADJUST_RIGHT == eAdjust;
                                break;
                            }
                        }
                        if( bNeedsSection )
                        {
                            pSaveStruct->AddContents(
                                InsertTableContents(bHead  ) );
                        }
                    }
                    else
                    {
                        // Wenn wir mitlerweile in einem Rahmen stehen
                        // koennen wir erneut eine echte Tabelle aufmachen.
                        // Wir erkennen das daran, dass wir keinen
                        // Tabellen-Node mehr finden.
                        bTopTable = pDoc->GetNodes()[pPam->GetPoint()->nNode]
                                        ->FindTableNode() == 0;

                        // Wenn im aktuellen Absatz Flys verankert sind,
                        // muss die neue Tabelle in einen Rahmen.
                        bHasToFly = HasCurrentParaFlys(sal_False,sal_True);
                    }

                    // in der Zelle kann sich ein Bereich befinden!
                    eTabAdjust = aAttrTab.pAdjust
                        ? ((const SvxAdjustItem&)aAttrTab.pAdjust->GetItem()).
                                                 GetAdjust()
                        : SVX_ADJUST_END;
                }

                HTMLTable *pSubTable = BuildTable( eTabAdjust,
                                                   bHead,
                                                   pSaveStruct->IsInSection(),
                                                   bTopTable, bHasToFly );
                if( SVPAR_PENDING != GetStatus() )
                {
                    // nur wenn die Tabelle wirklich zu Ende ist!
                    if( pSubTable )
                    {
                        ASSERT( pSubTable->GetTableAdjust(sal_False)!= SVX_ADJUST_LEFT &&
                                pSubTable->GetTableAdjust(sal_False)!= SVX_ADJUST_RIGHT,
                                "links oder rechts ausgerichtete Tabellen gehoehren in Rahmen" );


                        HTMLTableCnts *pParentContents =
                            pSubTable->GetParentContents();
                        if( pParentContents )
                        {
                            ASSERT( !pSaveStruct->IsInSection(),
                                    "Wo ist die Section geblieben" );

                            // Wenn jetzt keine Tabelle kommt haben wir eine
                            // Section
                            pSaveStruct->AddContents( pParentContents );
                        }

                        const SwStartNode *pCapStNd =
                                pSubTable->GetCaptionStartNode();

                        if( pSubTable->GetContext() )
                        {
                            ASSERT( !pSubTable->GetContext()->GetFrmFmt(),
                                    "Tabelle steht im Rahmen" );

                            if( pCapStNd && pSubTable->IsTopCaption() )
                            {
                                pSaveStruct->AddContents(
                                    new HTMLTableCnts(pCapStNd) );
                            }

                            pSaveStruct->AddContents(
                                new HTMLTableCnts(pSubTable) );

                            if( pCapStNd && !pSubTable->IsTopCaption() )
                            {
                                pSaveStruct->AddContents(
                                    new HTMLTableCnts(pCapStNd) );
                            }

                            // Jetzt haben wir keine Section mehr
                            pSaveStruct->ClearIsInSection();
                        }
                        else if( pCapStNd )
                        {
                            // Da wir diese Sction nicht mehr loeschen
                            // koennen (sie koeente zur erster Box
                            // gehoeren), fuegen wir sie ein.
                            pSaveStruct->AddContents(
                                new HTMLTableCnts(pCapStNd) );

                            // Jetzt haben wir keine Section mehr
                            pSaveStruct->ClearIsInSection();
                        }
                    }

                    pTable = pSaveStruct->pTable;
                }
            }
            break;

        case HTML_NOBR_ON:
            // HACK fuer MS: Steht das <NOBR> zu beginn der Zelle?
            pSaveStruct->StartNoBreak( *pPam->GetPoint() );
            break;

        case HTML_NOBR_OFF:
                pSaveStruct->EndNoBreak( *pPam->GetPoint() );
            break;

        case HTML_COMMENT:
            // Mit Kommentar-Feldern werden Spaces nicht mehr geloescht
            // ausserdem wollen wir fuer einen Kommentar keine neue Zelle
            // anlegen !!!
            NextToken( nToken );
            break;

        case HTML_MARQUEE_ON:
            if( !pSaveStruct->IsInSection() )
            {
                // eine neue Section anlegen, der PaM steht dann darin
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }
            bCallNextToken = sal_True;
            NewMarquee( pCurTable );
            break;

        case HTML_TEXTTOKEN:
            // keine Section fuer einen leeren String anlegen
            if( !pSaveStruct->IsInSection() && 1==aToken.Len() &&
                ' '==aToken.GetChar(0) )
                break;
        default:
            if( !pSaveStruct->IsInSection() )
            {
                // eine neue Section anlegen, der PaM steht dann darin
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }

            if( IsParserWorking() || bPending )
                NextToken( nToken );
            break;
        }

        ASSERT( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableCell: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( bHead ? HTML_TABLEHEADER_ON
                                               : HTML_TABLEDATA_ON, pPendStack );
        pPendStack->pData = pSaveStruct;

        return;
    }

    // Falls der Inhalt der Zelle leer war, muessen wir noch einen
    // leeren Inhalt anlegen. Ausserdem legen wir einen leeren Inhalt
    // an, wenn die Zelle mit einer Tabelle aufgehoert hat und keine
    // COL-Tags hatte (sonst wurde sie wahrscheinlich von uns exportiert,
    // und dann wollen wir natuerlich keinen zusaetzlichen Absatz haben).
    if( !pSaveStruct->GetFirstContents() ||
        (!pSaveStruct->IsInSection() && !pCurTable->HasColTags()) )
    {
        ASSERT( pSaveStruct->GetFirstContents() ||
                !pSaveStruct->IsInSection(),
                "Section oder nicht, das ist hier die Frage" );
        const SwStartNode *pStNd =
            InsertTableSection( pSaveStruct->IsHeaderCell()
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE );
        pDoc->GetNodes()[pStNd->GetIndex()+1]
            ->GetCntntNode()->SetAttr( SvxFontHeightItem(40) );
        pSaveStruct->AddContents( new HTMLTableCnts(pStNd) );
        pSaveStruct->ClearIsInSection();
    }

    sal_Bool bLFStripped = sal_False;
    if( pSaveStruct->IsInSection() )
    {
        pSaveStruct->CheckNoBreak( *pPam->GetPoint(), pDoc );

        // Alle noch offenen Kontexte beenden. Wir nehmen hier
        // AttrMin, weil nContxtStMin evtl. veraendert wurde.
        // Da es durch EndContext wieder restauriert wird, geht das.
        while( aContexts.Count() > nContextStAttrMin+1 )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            EndContext( pCntxt );
            delete pCntxt;
        }

        // LFs am Absatz-Ende entfernen
        if( StripTrailingLF()==0 && !pPam->GetPoint()->nContent.GetIndex() )
            StripTrailingPara();

#ifndef NUM_RELSPACE
        if( GetNumInfo().GetNumRule() )
            UpdateNumRuleInTable();
#endif

        // falls fuer die Zelle eine Ausrichtung gesetzt wurde, muessen
        // wir die beenden
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }
    else
    {
        // Alle noch offenen Kontexte beenden
        while( aContexts.Count() > nContextStAttrMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }
    }

    // auch eine Numerierung muss beendet werden
    GetNumInfo().Clear();

    SetAttr( sal_False );

    pSaveStruct->InsertCell( *this, pCurTable );

    // wir stehen jetzt (wahrschenlich) vor <TH>, <TD>, <TR> oder </TABLE>
    delete pSaveStruct;
}


class _RowSaveStruct : public SwPendingStackData
{
public:
    SvxAdjust eAdjust;
    SwVertOrient eVertOri;
    sal_Bool bHasCells;

    _RowSaveStruct() :
        bHasCells( sal_False ), eAdjust( SVX_ADJUST_END ), eVertOri( VERT_TOP )
    {}
};


void SwHTMLParser::BuildTableRow( HTMLTable *pCurTable, sal_Bool bReadOptions,
                                  SvxAdjust eGrpAdjust,
                                  SwVertOrient eGrpVertOri )
{
    // <TR> wurde bereist gelesen

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    _RowSaveStruct* pSaveStruct;

    sal_Bool bPending = sal_False;
    if( pPendStack )
    {
        pSaveStruct = (_RowSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        SvxAdjust eAdjust = eGrpAdjust;
        SwVertOrient eVertOri = eGrpVertOri;
        Color aBGColor;
        String aBGImage, aStyle, aId, aClass;
        sal_Bool bBGColor = sal_False;
        pSaveStruct = new _RowSaveStruct;

        if( bReadOptions )
        {
            const HTMLOptions *pOptions = GetOptions();
            for( sal_uInt16 i = pOptions->Count(); i; )
            {
                const HTMLOption *pOption = (*pOptions)[--i];
                switch( pOption->GetToken() )
                {
                case HTML_O_ID:
                    aId = pOption->GetString();
                    break;
                case HTML_O_ALIGN:
                    eAdjust = (SvxAdjust)pOption->GetEnum(
                                    aHTMLPAlignTable, eAdjust );
                    break;
                case HTML_O_VALIGN:
                    eVertOri = (SwVertOrient)pOption->GetEnum(
                                    aHTMLTblVAlignTable, eVertOri );
                    break;
                case HTML_O_BGCOLOR:
                    // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netsc.
                    // ignorieren, bei allen anderen Tags *wirklich* nicht.
                    if( pOption->GetString().Len() )
                    {
                        pOption->GetColor( aBGColor );
                        bBGColor = sal_True;
                    }
                    break;
                case HTML_O_BACKGROUND:
                    aBGImage = pOption->GetString();
                    break;
                case HTML_O_STYLE:
                    aStyle = pOption->GetString();
                    break;
                case HTML_O_CLASS:
                    aClass= pOption->GetString();
                    break;
                }
            }
        }

        if( aId.Len() )
            InsertBookmark( aId );

        SvxBrushItem *pBrushItem =
            CreateBrushItem( bBGColor ? &aBGColor : 0, aBGImage, aStyle,
                             aId, aClass );
        pCurTable->OpenRow( eAdjust, eVertOri, pBrushItem );
        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        ASSERT( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = sal_True;
            }
//          else
//          {
//              NextToken( nToken );
//          }
            break;
        case HTML_TABLEROW_ON:
        case HTML_THEAD_ON:
        case HTML_THEAD_OFF:
        case HTML_TBODY_ON:
        case HTML_TBODY_OFF:
        case HTML_TFOOT_ON:
        case HTML_TFOOT_OFF:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
        case HTML_TABLEROW_OFF:
            bDone = sal_True;
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            BuildTableCell( pCurTable, sal_True, HTML_TABLEHEADER_ON==nToken );
            if( SVPAR_PENDING != GetStatus() )
            {
                pSaveStruct->bHasCells = sal_True;
                bDone = pTable->IsOverflowing();
            }
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_CAPTION_OFF:
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
        case HTML_COLGROUP_ON:
        case HTML_COLGROUP_OFF:
        case HTML_COL_ON:
        case HTML_COL_OFF:
            // wo keine Zelle anfing kann auch keine aufhoehren, oder?
            // und die ganzen anderen Tokens haben hier auch nicht zu
            // suchen und machen nur die Tabelle kaputt
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( sal_False );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( sal_False );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_COMMENT:
            NextToken( nToken );
            break;
        case HTML_MAP_ON:
            // eine Image-Map fuegt nichts ein, deshalb koennen wir sie
            // problemlos auch ohne Zelle parsen
            NextToken( nToken );
            break;
        case HTML_TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.Len() && ' '==aToken.GetChar(0) )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
            break;
        }

        ASSERT( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableRow: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_TABLEROW_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseRow( !pSaveStruct->bHasCells );
        delete pSaveStruct;
    }

    // wir stehen jetzt (wahrscheinlich) vor <TR> oder </TABLE>
}

void SwHTMLParser::BuildTableSection( HTMLTable *pCurTable,
                                      sal_Bool bReadOptions,
                                      sal_Bool bHead )
{
    // <THEAD>, <TBODY> bzw. <TFOOT> wurde bereits gelesen
    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    _RowSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_RowSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        pSaveStruct = new _RowSaveStruct;

        if( bReadOptions )
        {
            const HTMLOptions *pOptions = GetOptions();
            for( sal_uInt16 i = pOptions->Count(); i; )
            {
                const HTMLOption *pOption = (*pOptions)[--i];
                switch( pOption->GetToken() )
                {
                case HTML_O_ID:
                    InsertBookmark( pOption->GetString() );
                    break;
                case HTML_O_ALIGN:
                    pSaveStruct->eAdjust =
                        (SvxAdjust)pOption->GetEnum( aHTMLPAlignTable,
                                                     pSaveStruct->eAdjust );
                    break;
                case HTML_O_VALIGN:
                    pSaveStruct->eVertOri =
                        (SwVertOrient)pOption->GetEnum( aHTMLTblVAlignTable,
                                                        pSaveStruct->eVertOri );
                    break;
                }
            }
        }

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        ASSERT( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = sal_True;
            }
//          else
//          {
//              NextToken( nToken );
//          }
            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
        case HTML_THEAD_OFF:
        case HTML_TBODY_OFF:
        case HTML_TFOOT_OFF:
            bDone = sal_True;
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_CAPTION_OFF:
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken( -1 );
            BuildTableRow( pCurTable, sal_False, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_TABLEROW_ON:
            BuildTableRow( pCurTable, sal_True, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( sal_False );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( sal_False );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_TEXTTOKEN:
            // Blank-Strings sind Folge von CR+LF und kein Text
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.Len() && ' '==aToken.GetChar(0) )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        ASSERT( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableSection: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( bHead ? HTML_THEAD_ON
                                               : HTML_TBODY_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseSection( bHead );
        delete pSaveStruct;
    }

    // wir stehen jetzt (wahrscheinlich) vor <TBODY>,... oder </TABLE>
}

struct _TblColGrpSaveStruct : public SwPendingStackData
{
    sal_uInt16 nColGrpSpan;
    sal_uInt16 nColGrpWidth;
    sal_Bool bRelColGrpWidth;
    SvxAdjust eColGrpAdjust;
    SwVertOrient eColGrpVertOri;

    inline _TblColGrpSaveStruct();


    inline void CloseColGroup( HTMLTable *pTable );
};

inline _TblColGrpSaveStruct::_TblColGrpSaveStruct() :
    nColGrpSpan( 1 ), nColGrpWidth( 0 ),
    bRelColGrpWidth( sal_False ), eColGrpAdjust( SVX_ADJUST_END ),
    eColGrpVertOri( VERT_TOP )
{}


inline void _TblColGrpSaveStruct::CloseColGroup( HTMLTable *pTable )
{
    pTable->CloseColGroup( nColGrpSpan, nColGrpWidth,
                            bRelColGrpWidth, eColGrpAdjust, eColGrpVertOri );
}

void SwHTMLParser::BuildTableColGroup( HTMLTable *pCurTable,
                                       sal_Bool bReadOptions )
{
    // <COLGROUP> wurde bereits gelesen, wenn bReadOptions

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    _TblColGrpSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_TblColGrpSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {

        pSaveStruct = new _TblColGrpSaveStruct;
        if( bReadOptions )
        {
            const HTMLOptions *pColGrpOptions = GetOptions();
            for( sal_uInt16 i = pColGrpOptions->Count(); i; )
            {
                const HTMLOption *pColGrpOption = (*pColGrpOptions)[--i];
                switch( pColGrpOption->GetToken() )
                {
                case HTML_O_ID:
                    InsertBookmark( pColGrpOption->GetString() );
                    break;
                case HTML_O_SPAN:
                    pSaveStruct->nColGrpSpan = (sal_uInt16)pColGrpOption->GetNumber();
                    break;
                case HTML_O_WIDTH:
                    pSaveStruct->nColGrpWidth = (sal_uInt16)pColGrpOption->GetNumber();
                    pSaveStruct->bRelColGrpWidth =
                        (pColGrpOption->GetString().Search('*') != STRING_NOTFOUND);
                    break;
                case HTML_O_ALIGN:
                    pSaveStruct->eColGrpAdjust =
                        (SvxAdjust)pColGrpOption->GetEnum( aHTMLPAlignTable,
                                                pSaveStruct->eColGrpAdjust );
                    break;
                case HTML_O_VALIGN:
                    pSaveStruct->eColGrpVertOri =
                        (SwVertOrient)pColGrpOption->GetEnum( aHTMLTblVAlignTable,
                                                pSaveStruct->eColGrpVertOri );
                    break;
                }
            }
        }
        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        ASSERT( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = sal_True;
            }
//          else
//          {
//              NextToken( nToken );
//          }
            break;
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
        case HTML_COLGROUP_OFF:
            bDone = sal_True;
            break;
        case HTML_COL_ON:
            {
                sal_uInt16 nColSpan = 1;
                sal_uInt16 nColWidth = pSaveStruct->nColGrpWidth;
                sal_Bool bRelColWidth = pSaveStruct->bRelColGrpWidth;
                SvxAdjust eColAdjust = pSaveStruct->eColGrpAdjust;
                SwVertOrient eColVertOri = pSaveStruct->eColGrpVertOri;

                const HTMLOptions *pColOptions = GetOptions();
                for( sal_uInt16 i = pColOptions->Count(); i; )
                {
                    const HTMLOption *pColOption = (*pColOptions)[--i];
                    switch( pColOption->GetToken() )
                    {
                    case HTML_O_ID:
                        InsertBookmark( pColOption->GetString() );
                        break;
                    case HTML_O_SPAN:
                        nColSpan = (sal_uInt16)pColOption->GetNumber();
                        break;
                    case HTML_O_WIDTH:
                        nColWidth = (sal_uInt16)pColOption->GetNumber();
                        bRelColWidth =
                            (pColOption->GetString().Search('*') != STRING_NOTFOUND);
                        break;
                    case HTML_O_ALIGN:
                        eColAdjust =
                            (SvxAdjust)pColOption->GetEnum( aHTMLPAlignTable,
                                                            eColAdjust );
                        break;
                    case HTML_O_VALIGN:
                        eColVertOri =
                            (SwVertOrient)pColOption->GetEnum( aHTMLTblVAlignTable,
                                                            eColVertOri );
                        break;
                    }
                }
                pCurTable->InsertCol( nColSpan, nColWidth, bRelColWidth,
                                      eColAdjust, eColVertOri );

                // die Angaben in <COLGRP> sollen ignoriert werden, wenn
                // <COL>-Elemente existieren
                pSaveStruct->nColGrpSpan = 0;
            }
            break;
        case HTML_COL_OFF:
            break;      // Ignorieren
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.Len() && ' '==aToken.GetChar(0) )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        ASSERT( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableColGrp: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_COL_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
    }
    else
    {
        pSaveStruct->CloseColGroup( pCurTable );
        delete pSaveStruct;
    }
}

class _CaptionSaveStruct : public _SectionSaveStruct
{
    SwPosition aSavePos;
    SwHTMLNumRuleInfo aNumRuleInfo; // gueltige Numerierung

public:

    _HTMLAttrTable aAttrTab;        // und die Attribute

    _CaptionSaveStruct( SwHTMLParser& rParser, const SwPosition& rPos ) :
        _SectionSaveStruct( rParser ), aSavePos( rPos )
    {
        rParser.SaveAttrTab( aAttrTab );

        // Die aktuelle Numerierung wurde gerettet und muss nur
        // noch beendet werden.
#ifndef NUM_RELSPACE
        if( rParser.GetNumInfo().GetNumRule() )
            rParser.UpdateNumRuleInTable();
#endif
        aNumRuleInfo.Set( rParser.GetNumInfo() );
        rParser.GetNumInfo().Clear();
    }

    const SwPosition& GetPos() const { return aSavePos; }

    void RestoreAll( SwHTMLParser& rParser )
    {
        // Die alten Stack wiederherstellen
        Restore( rParser );

        // Die alte Attribut-Tabelle wiederherstellen
        rParser.RestoreAttrTab( aAttrTab );

        // Die alte Numerierung wieder aufspannen
        rParser.GetNumInfo().Set( aNumRuleInfo );
    }

    virtual ~_CaptionSaveStruct();
};

_CaptionSaveStruct::~_CaptionSaveStruct()
{}

void SwHTMLParser::BuildTableCaption( HTMLTable *pCurTable )
{
    // <CAPTION> wurde bereits gelesen

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    _CaptionSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_CaptionSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        ASSERT( !pPendStack, "Wo kommt hier ein Pending-Stack her?" );

        SaveState( nToken );
    }
    else
    {
        if( pTable->IsOverflowing() )
        {
            SaveState( 0 );
            return;
        }

        sal_Bool bTop = sal_True;
        const HTMLOptions *pOptions = GetOptions();
        for ( sal_uInt16 i = pOptions->Count(); i; )
        {
            const HTMLOption *pOption = (*pOptions)[--i];
            if( HTML_O_ALIGN == pOption->GetToken() )
            {
                if( pOption->GetString().EqualsIgnoreCaseAscii(sHTML_VA_bottom))
                    bTop = sal_False;
            }
        }

        // Alte PaM-Position retten.
        pSaveStruct = new _CaptionSaveStruct( *this, *pPam->GetPoint() );

        // Eine Text-Section im Icons-Bereich als Container fuer die
        // Ueberschrift anlegen und PaM dort reinstellen.
        const SwStartNode *pStNd;
        if( pTable == pCurTable )
            pStNd = InsertTempTableCaptionSection();
        else
            pStNd = InsertTableSection( RES_POOLCOLL_TEXT );

        _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_CAPTION_ON );

        // Tabellen-Ueberschriften sind immer zentriert.
        NewAttr( &aAttrTab.pAdjust, SvxAdjustItem(SVX_ADJUST_CENTER) );

        _HTMLAttrs &rAttrs = pCntxt->GetAttrs();
        rAttrs.Insert( aAttrTab.pAdjust, rAttrs.Count() );

        PushContext( pCntxt );

        // StartNode der Section an der Tabelle merken.
        pCurTable->SetCaption( pStNd, bTop );

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    // </CAPTION> wird laut DTD benoetigt
    sal_Bool bDone = sal_False;
    while( IsParserWorking() && !bDone )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pPendStack )
            {
                pSaveStruct->pTable = pTable;
                sal_Bool bHasToFly = pSaveStruct->pTable!=pCurTable;
                BuildTable( pCurTable->GetTableAdjust( sal_True ),
                            sal_False, sal_True, sal_True, bHasToFly );
            }
            else
            {
                BuildTable( SVX_ADJUST_END );
            }
            if( SVPAR_PENDING != GetStatus() )
            {
                pTable = pSaveStruct->pTable;
            }
            break;
        case HTML_TABLE_OFF:
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
            SkipToken( -1 );
            bDone = sal_True;
            break;

        case HTML_CAPTION_OFF:
            bDone = sal_True;
            break;
        default:
            int nNxtToken = nToken;
            if( pPendStack )
            {
                SwPendingStack* pTmp = pPendStack->pNext;
                delete pPendStack;
                pPendStack = pTmp;

                ASSERT( !pTmp, "weiter kann es nicht gehen!" );
                nNxtToken = 0;  // neu lesen
            }

            if( IsParserWorking() )
                NextToken( nToken );
            break;
        }

        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING==GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_CAPTION_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
        return;
    }

    // Alle noch offenen Kontexte beenden
    while( aContexts.Count() > nContextStAttrMin+1 )
    {
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }

    // LF am Absatz-Ende entfernen
    sal_Bool bLFStripped = StripTrailingLF() > 0;

    if( pTable==pCurTable )
    {
        // Beim spaeteren verschieben der Beschriftung vor oder hinter
        // die Tabelle wird der letzte Absatz nicht mitverschoben.
        // Deshalb muss sich am Ende der Section immer ein leerer
        // Absatz befinden.
        if( pPam->GetPoint()->nContent.GetIndex() || bLFStripped )
            AppendTxtNode( AM_NOSPACE );
    }
    else
    {
        // LFs am Absatz-Ende entfernen
        if( !pPam->GetPoint()->nContent.GetIndex() && !bLFStripped )
            StripTrailingPara();
    }

    // falls fuer die Zelle eine Ausrichtung gesetzt wurde, muessen
    // wir die beenden
    _HTMLAttrContext *pCntxt = PopContext();
    EndContext( pCntxt );
    delete pCntxt;

    SetAttr( sal_False );

    // Stacks und Attribut-Tabelle wiederherstellen
    pSaveStruct->RestoreAll( *this );

    // PaM wiederherstellen.
    *pPam->GetPoint() = pSaveStruct->GetPos();

    delete pSaveStruct;
}

class _TblSaveStruct : public SwPendingStackData
{
public:
    HTMLTable *pCurTable;

    _TblSaveStruct( HTMLTable *pCurTbl ) :
        pCurTable( pCurTbl )
    {}

    virtual ~_TblSaveStruct();

    // Aufbau der Tabelle anstossen und die Tabelle ggf. in einen
    // Rahmen packen. Wenn sal_True zurueckgegeben wird muss noch ein
    // Absatz eingefuegt werden!
    void MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc );
};

_TblSaveStruct::~_TblSaveStruct()
{}


void _TblSaveStruct::MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc )
{
    pCurTable->MakeTable( 0, nWidth );

    _HTMLTableContext *pTCntxt = pCurTable->GetContext();
    ASSERT( pTCntxt, "Wo ist der Tabellen-Kontext" );

    SwTableNode *pTblNd = pTCntxt->GetTableNode();
    ASSERT( pTblNd, "Wo ist der Tabellen-Node" );

    if( pDoc->GetRootFrm() && pTblNd )
    {
        // Existiert schon ein Layout, dann muss an dieser Tabelle die
        // BoxFrames neu erzeugt werden.

        if( pTCntxt->GetFrmFmt() )
        {
            pTCntxt->GetFrmFmt()->DelFrms();
            pTblNd->DelFrms();
            pTCntxt->GetFrmFmt()->MakeFrms();
        }
        else
        {
            pTblNd->DelFrms();
            SwNodeIndex aIdx( *pTblNd->EndOfSectionNode(), 1 );
            ASSERT( aIdx.GetIndex() <= pTCntxt->GetPos()->nNode.GetIndex(),
                    "unerwarteter Node fuer das Tabellen-Layout" );
            pTblNd->MakeFrms( &aIdx );
        }
    }

    rPos = *pTCntxt->GetPos();
}


HTMLTableOptions::HTMLTableOptions( const HTMLOptions *pOptions,
                                    SvxAdjust eParentAdjust ) :
    nCols( 0 ),
    nWidth( 0 ), nHeight( 0 ),
    nCellPadding( USHRT_MAX ), nCellSpacing( USHRT_MAX ),
    nBorder( USHRT_MAX ),
    nHSpace( 0 ), nVSpace( 0 ),
    eAdjust( eParentAdjust ), eVertOri( VERT_CENTER ),
    eFrame( HTML_TF_VOID ), eRules( HTML_TR_NONE ),
    bPrcWidth( sal_False ),
    bTableAdjust( sal_False ),
    bBGColor( sal_False ),
    aBorderColor( COL_GRAY )
{
    sal_Bool bBorderColor = sal_False;
    sal_Bool bHasFrame = sal_False, bHasRules = sal_False;

    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_COLS:
            nCols = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_WIDTH:
            nWidth = (sal_uInt16)pOption->GetNumber();
            bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            if( bPrcWidth && nWidth>100 )
                nWidth = 100;
            break;
        case HTML_O_HEIGHT:
            nHeight = (sal_uInt16)pOption->GetNumber();
            if( pOption->GetString().Search('%') != STRING_NOTFOUND )
                nHeight = 0;    // keine %-Anagben benutzen!!!
            break;
        case HTML_O_CELLPADDING:
            nCellPadding = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_CELLSPACING:
            nCellSpacing = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_ALIGN:
            {
                sal_uInt16 nAdjust = eAdjust;
                if( pOption->GetEnum( nAdjust, aHTMLPAlignTable ) )
                {
                    eAdjust = (SvxAdjust)nAdjust;
                    bTableAdjust = sal_True;
                }
            }
            break;
        case HTML_O_VALIGN:
            eVertOri = (SwVertOrient)pOption->GetEnum( aHTMLTblVAlignTable,
                                                       eVertOri );
            break;
        case HTML_O_BORDER:
            // BORDER und BORDER=BORDER wie BORDER=1 behandeln
            if( pOption->GetString().Len() &&
                !pOption->GetString().EqualsIgnoreCaseAscii(sHTML_O_border) )
                nBorder = (sal_uInt16)pOption->GetNumber();
            else
                nBorder = 1;

            if( !bHasFrame )
                eFrame = ( nBorder ? HTML_TF_BOX : HTML_TF_VOID );
            if( !bHasRules )
                eRules = ( nBorder ? HTML_TR_ALL : HTML_TR_NONE );
            break;
        case HTML_O_FRAME:
            eFrame = pOption->GetTableFrame();
            bHasFrame = sal_True;
            break;
        case HTML_O_RULES:
            eRules = pOption->GetTableRules();
            bHasRules = sal_True;
            break;
        case HTML_O_BGCOLOR:
            // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netscape
            // ignorieren, bei allen anderen Tags *wirklich* nicht.
            if( pOption->GetString().Len() )
            {
                pOption->GetColor( aBGColor );
                bBGColor = sal_True;
            }
            break;
        case HTML_O_BACKGROUND:
            aBGImage = pOption->GetString();
            break;
        case HTML_O_BORDERCOLOR:
            pOption->GetColor( aBorderColor );
            bBorderColor = sal_True;
            break;
        case HTML_O_BORDERCOLORDARK:
            if( !bBorderColor )
                pOption->GetColor( aBorderColor );
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_HSPACE:
            nHSpace = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_VSPACE:
            nVSpace = (sal_uInt16)pOption->GetNumber();
            break;
        }
    }

    if( nCols && !nWidth )
    {
        nWidth = 100;
        bPrcWidth = sal_True;
    }

    // Wenn BORDER=0 oder kein BORDER gegeben ist, daan darf es auch
    // keine Umrandung geben
    if( 0==nBorder || USHRT_MAX==nBorder )
    {
        eFrame = HTML_TF_VOID;
        eRules = HTML_TR_NONE;
    }
}


HTMLTable *SwHTMLParser::BuildTable( SvxAdjust eParentAdjust,
                                     sal_Bool bIsParentHead,
                                     sal_Bool bHasParentSection,
                                     sal_Bool bMakeTopSubTable,
                                     sal_Bool bHasToFly )
{
    if( !IsParserWorking() && !pPendStack )
        return 0;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    _TblSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_TblSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        HTMLTableOptions *pTblOptions =
            new HTMLTableOptions( GetOptions(), eParentAdjust );

        if( pTblOptions->aId.Len() )
            InsertBookmark( pTblOptions->aId );

        // Wenn die Tabelle in einem Rahmen steht oder link oder rechts
        // ausgerichtet ist, wird in jedem Fall eine "echte" Tabelle daraus.
        if( bMakeTopSubTable || bHasToFly ||
            (pTblOptions->bTableAdjust &&
             (SVX_ADJUST_LEFT==pTblOptions->eAdjust ||
              SVX_ADJUST_RIGHT==pTblOptions->eAdjust)) )
        {
            pTable = 0;
        }

        HTMLTable *pCurTable = new HTMLTable( this, pTable,
                                              bIsParentHead,
                                              bHasParentSection,
                                              bMakeTopSubTable,
                                              bHasToFly,
                                              pTblOptions );
        if( !pTable )
            pTable = pCurTable;

        pSaveStruct = new _TblSaveStruct( pCurTable );

        delete pTblOptions;

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    HTMLTable *pCurTable = pSaveStruct->pCurTable;

    // </TABLE> wird laut DTD benoetigt
    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        ASSERT( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext() )
            {
                // Wenn noch keine Tabelle eingefuegt wurde,
                // die naechste Tabelle lesen
                SkipToken( -1 );
                bDone = sal_True;
            }
//          else
//          {
//              NextToken( nToken );
//          }
            break;
        case HTML_TABLE_OFF:
            bDone = sal_True;
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_COL_ON:
            SkipToken( -1 );
            BuildTableColGroup( pCurTable, sal_False );
            break;
        case HTML_COLGROUP_ON:
            BuildTableColGroup( pCurTable, sal_True );
            break;
        case HTML_TABLEROW_ON:
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken( -1 );
            BuildTableSection( pCurTable, sal_False, sal_False );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
            BuildTableSection( pCurTable, sal_True, HTML_THEAD_ON==nToken );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( sal_False );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( sal_False );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_TEXTTOKEN:
            // Blank-Strings sind u. U. eine Folge von CR+LF und kein Text
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.Len() && ' '==aToken.GetChar(0) )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
            break;
        }

        ASSERT( !bPending || !pPendStack,
                "SwHTMLParser::BuildTable: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_TABLE_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
        return 0;
    }

    _HTMLTableContext *pTCntxt = pCurTable->GetContext();
    if( pTCntxt )
    {
        // Die Tabelle wurde auch angelegt

        // Tabellen-Struktur anpassen
        pCurTable->CloseTable();

        // ausserhalb von Zellen begonnene Kontexte beenden
        // muss vor(!) dem Umsetzten der Attribut Tabelle existieren,
        // weil die aktuelle danach nicht mehr existiert
        while( aContexts.Count() > nContextStAttrMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }

        nContextStMin = pTCntxt->GetContextStMin();
        nContextStAttrMin = pTCntxt->GetContextStAttrMin();

        if( pTable==pCurTable )
        {
            // Tabellen-Beschriftung setzen
            const SwStartNode *pCapStNd = pTable->GetCaptionStartNode();
            if( pCapStNd )
            {
                // Der letzte Absatz der Section wird nie mitkopiert. Deshalb
                // muss die Section mindestens zwei Absaetze enthalten.

                if( pCapStNd->EndOfSectionIndex() - pCapStNd->GetIndex() > 2 )
                {
                    // Start-Node und letzten Absatz nicht mitkopieren.
                    SwNodeRange aSrcRg( *pCapStNd, 1,
                                    *pCapStNd->EndOfSectionNode(), -1 );

                    sal_Bool bTop = pTable->IsTopCaption();
                    SwStartNode *pTblStNd = pTCntxt->GetTableNode();

                    ASSERT( pTblStNd, "Wo ist der Tabellen-Node" );
                    ASSERT( pTblStNd==pPam->GetNode()->FindTableNode(),
                            "Stehen wir in der falschen Tabelle?" );

                    SwNode* pNd;
                    if( bTop )
                        pNd = pTblStNd;
                    else
                        pNd = pTblStNd->EndOfSectionNode();
                    SwNodeIndex aDstIdx( *pNd, bTop ? 0 : 1 );

                    pDoc->Move( aSrcRg, aDstIdx );

                    // Wenn die Caption vor der Tabelle eingefuegt wurde muss
                    // eine an der Tabelle gestzte Seitenvorlage noch in den
                    // ersten Absatz der Ueberschrift verschoben werden.
                    // Ausserdem muessen alle gemerkten Indizes, die auf den
                    // Tabellen-Node zeigen noch verschoben werden.
                    if( bTop )
                    {
                        MovePageDescAttrs( pTblStNd, aSrcRg.aStart.GetIndex(),
                                           sal_False );
                    }
                }

                // Die Section wird jetzt nicht mehr gebraucht.
                pPam->SetMark();
                pPam->DeleteMark();
                pDoc->DeleteSection( (SwStartNode *)pCapStNd );
                pTable->SetCaption( 0, sal_False );
            }

            // SwTable aufbereiten
            sal_uInt16 nBrowseWidth = (sal_uInt16)GetCurrentBrowseWidth();
            pSaveStruct->MakeTable( nBrowseWidth, *pPam->GetPoint(), pDoc );

#ifdef TEST_RESIZE
            const SwTable *pSwTable = pTable->GetSwTable();
            SwHTMLTableLayout *pLayoutInfo =
                pSwTable ? ((SwTable *)pSwTable)->GetHTMLTableLayout() : 0;
            if( pLayoutInfo )
            {
                ViewShell *pVSh = CheckActionViewShell();
                if( pVSh )
                {
                    CallEndAction( sal_False, sal_False );
                    CallStartAction( pVSh, sal_False );

                    sal_uInt16 nNewBrwoseWidth =
                        (sal_uInt16)GetCurrentBrowseWidth();
                    if( nBrowseWidth != nNewBrowseWidth )
                        pLayoutInfo->Resize( nNewBrowseWidth );
                }
            }
#endif
        }

        GetNumInfo().Set( pTCntxt->GetNumInfo() );
        pTCntxt->RestorePREListingXMP( *this );
        RestoreAttrTab( pTCntxt->aAttrTab );

        if( pTable==pCurTable )
        {
            // oberen Absatz-Abstand einstellen
            bUpperSpace = sal_True;
            SetTxtCollAttrs();

            nParaCnt -= Min(nParaCnt, pTCntxt->GetTableNode()->GetTable().GetTabSortBoxes().Count());

            // ggfs. eine Tabelle anspringen
            if( JUMPTO_TABLE == eJumpTo && pTable->GetSwTable() &&
                pTable->GetSwTable()->GetFrmFmt()->GetName() == sJmpMark )
            {
                bChkJumpMark = sal_True;
                eJumpTo = JUMPTO_NONE;
            }

            // fix #37886#: Wenn Import abgebrochen wurde kein erneutes Show
            // aufrufen, weil die ViewShell schon geloescht wurde!
            // fix #41669#: Genuegt nicht. Auch im ACCEPTING_STATE darf
            // kein Show aufgerufen werden, weil sonst waehrend des
            // Reschedules der Parser zerstoert wird, wenn noch ein
            // DataAvailable-Link kommt. Deshalb: Nur im WORKING-State.
            if( !nParaCnt && SVPAR_WORKING == GetStatus() )
                Show();
        }
    }
    else if( pTable==pCurTable )
    {
        // Es wurde gar keine Tabelle gelesen.

        // Dann muss eine evtl gelesene Beschriftung noch geloescht werden.
        const SwStartNode *pCapStNd = pCurTable->GetCaptionStartNode();
        if( pCapStNd )
        {
            pPam->SetMark();
            pPam->DeleteMark();
            pDoc->DeleteSection( (SwStartNode *)pCapStNd );
            pCurTable->SetCaption( 0, sal_False );
        }
    }

    if( pTable == pCurTable  )
    {
        delete pSaveStruct->pCurTable;
        pSaveStruct->pCurTable = 0;
        pTable = 0;
    }

    HTMLTable* pRetTbl = pSaveStruct->pCurTable;
    delete pSaveStruct;

    return pRetTbl;
}



/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmltab.cxx,v 1.1.1.1 2000-09-18 17:14:56 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.295  2000/09/18 16:04:46  willem.vandorp
      OpenOffice header added.

      Revision 1.294  2000/09/05 14:01:28  mib
      #78294#: removed support for frameset documents

      Revision 1.293  2000/06/26 09:52:30  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.292  2000/04/28 14:29:12  mib
      unicode

      Revision 1.291  2000/04/10 12:20:57  mib
      unicode

      Revision 1.290  2000/03/21 15:06:18  os
      UNOIII

      Revision 1.289  2000/03/03 15:21:01  os
      StarView remainders removed

      Revision 1.288  2000/02/11 14:37:28  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.287  2000/02/07 08:24:58  mib
      #72727#: HORI_LEFT_AND_WIDTH

      Revision 1.286  1999/11/19 16:40:20  os
      modules renamed

      Revision 1.285  1999/09/17 12:14:09  mib
      support of multiple and non system text encodings

      Revision 1.284  1999/07/29 06:21:52  MIB
      fix LHS means left border and RHS right border, not vice versa


      Rev 1.283   29 Jul 1999 08:21:52   MIB
   fix LHS means left border and RHS right border, not vice versa

      Rev 1.282   10 Jun 1999 10:34:26   JP
   have to change: no AppWin from SfxApp

      Rev 1.281   09 Jun 1999 19:37:00   JP
   have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp

      Rev 1.280   20 Apr 1999 10:07:50   MIB
   bCallNextToken auch in Rows etc. auswerten

      Rev 1.279   15 Apr 1999 13:48:20   MIB
   #41833#: Styles fuer A-Tag

      Rev 1.278   12 Apr 1999 09:23:28   MIB
   Korrekt horizontal splitten (auch #64460#)

      Rev 1.277   09 Apr 1999 10:30:36   MIB
   #64522#: Styles fuer TD/TH auswerten

      Rev 1.276   08 Apr 1999 11:13:10   MIB
   #56334#: Optimierung fuer Hintergrundfarben wieder ausgebaut

      Rev 1.275   06 Apr 1999 12:07:14   MIB
   #64308#: ALIGN=? an Tabelle

      Rev 1.274   01 Apr 1999 10:20:08   MIB
   bug fix: Leere BGColor nicht auswerten

      Rev 1.273   01 Apr 1999 08:57:02   MIB
   #56334#: Keine Spezialbehandlung der Hintergruende fuer die GC mehr noetig

      Rev 1.272   30 Mar 1999 11:58:36   MIB
   #62977#: Nicht mehr Spalten einfuegen als Zellen je Zeile vorhanden

      Rev 1.271   30 Mar 1999 11:44:26   MIB
   #62977#: Nicht mehr Spalten einfuegen als Zellen je Zeile vorhanden (nur>529)

      Rev 1.270   26 Mar 1999 11:39:46   MIB
   #63049#: Sofortige Numerierungs-Aktualisierung in Tabellen ist jetzt unnoetig

      Rev 1.269   17 Mar 1999 16:47:10   MIB
   #63049#: Numerierungen mit relativen Abstaenden

      Rev 1.268   10 Mar 1999 15:44:10   MIB
   #62682#: Beim Setzen der Control-Groesse wenn noetig auf die ViewShell warten

      Rev 1.267   25 Feb 1999 16:00:42   MIB
   #61949#: globale Shell entsorgt

      Rev 1.266   27 Jan 1999 09:43:56   OS
   #56371# TF_ONE51

      Rev 1.265   11 Jan 1999 11:17:08   MIB
   Attribut-Tabellen-Ueberpruefung in NonPro wegen Asserts nicht immer

      Rev 1.264   05 Jan 1999 14:48:00   MIB
   #60429#: Zeilenhoehe korrekt berechnen

      Rev 1.263   11 Nov 1998 17:42:10   MIB
   #59059#: Beim Abbrechen Pendung-Stack noch aufraeumen

      Rev 1.262   09 Nov 1998 11:35:18   MIB
   #59159#: Pending-Stack von fremden Token nicht loeschen

      Rev 1.261   03 Nov 1998 15:37:10   MIB
   #58840#: bHoriSplittAll-Assert war falsch

      Rev 1.260   25 Sep 1998 08:36:04   MIB
   #45631#: Tabellen nach 64000 Zellen beenden

      Rev 1.259   14 Sep 1998 11:41:12   MIB
   #51238#: Beim Setzen des Zeilen-Hintergrundes ROWSPAN>1 beachten

      Rev 1.258   31 Aug 1998 11:19:44   MIB
   ##: Beim vertikalen Zerlegen auch letzte Spalte auf moegliche vert. Zerlegeung pruefen

      Rev 1.257   27 Aug 1998 09:45:52   MIB
   #54170#: Line-Format-Attribute zuruecksetzen, bevor Hoehe oder Hintergrund gesetzt wird

      Rev 1.256   26 Aug 1998 09:30:36   MIB
   #55144#: Umlauf bei Tabellen in Rahmen richtig setzen und beachten

      Rev 1.255   21 Jul 1998 16:55:46   JP
   Bug #53456#: nie per Add eine Box im Format anmelden, sondern per Box::ChgFrmFmt

      Rev 1.254   05 Jun 1998 13:56:54   JP
   Bug #42487#: Sprung zum Mark schon waehrend des Ladens ausfuehren

      Rev 1.253   12 May 1998 15:50:24   JP
   rund um Flys/DrawObjs im Doc/FESh umgestellt/optimiert

      Rev 1.252   15 Apr 1998 14:56:46   MIB
   Zwei-seitige Printing-Extensions

      Rev 1.251   27 Mar 1998 18:02:42   MIB
   HSPACE/VSPACE

      Rev 1.250   24 Mar 1998 14:31:24   MIB
   obsolete ParseStyleOptions-Methode entfernt

      Rev 1.249   11 Mar 1998 18:28:54   MIB
   fix #47846#: auto-gebundenen Rahmen korrekt verankern

      Rev 1.248   02 Mar 1998 18:43:16   MIB
   fix #45489#: Umrandung ueber mehrere Ebenenen vererben

      Rev 1.247   27 Feb 1998 16:42:38   MIB
   fix #46450#: Horizontale une vertikale Ausrichtung von Zellen mit NumFmt

      Rev 1.246   27 Feb 1998 14:05:10   MIB
   Auto-gebundene Rahmen

      Rev 1.245   25 Feb 1998 12:17:36   MIB
   I like ME and its 0xffs

      Rev 1.244   25 Feb 1998 10:49:54   MIB
   fix: Tabellen-Ueberschriften in Sub-Tabellen gleich in eine eigene Box packen

      Rev 1.243   24 Feb 1998 17:45:50   MIB
   Attribute von Tabellen-Zellen ueber AttrTab setzen

      Rev 1.242   20 Feb 1998 18:52:04   MIB
   fix #45328#: Bessere Behandlung von Styles an Tabellen-Zellen

      Rev 1.241   20 Feb 1998 12:25:12   MIB
   fix #45631#: Weniger ::com::sun::star::frame::Frame-Formate anlegen

      Rev 1.240   19 Feb 1998 15:14:52   MIB
   fix #47394#: Tabellen-Zellen wurden unnoetig geteilt

      Rev 1.239   18 Feb 1998 10:57:30   MIB
   fix #45153#: Bei fixer Tabellenhoehe Tabelle in Tabelle beachten

      Rev 1.238   13 Feb 1998 18:48:42   HR
   C40_INSERT

      Rev 1.237   29 Jan 1998 21:34:18   JP
   GetEndOfIcons ersetzt durch GetEndOfExtras, das auf GetEndOfRedlines mappt

      Rev 1.236   19 Jan 1998 16:27:14   MIB
   Numerierungs-Umbau

      Rev 1.235   26 Nov 1997 19:09:42   MA
   includes

      Rev 1.234   17 Nov 1997 10:16:40   JP
   Umstellung Numerierung

      Rev 1.233   09 Oct 1997 14:34:14   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.232   19 Sep 1997 08:40:34   MIB
   fix #41185#: Laufschrift an Tabellenzellen-Breite anpassen

      Rev 1.231   16 Sep 1997 14:55:28   MIB
   ITEMID_BOXINFOITEM (voreubergendend) definieren

      Rev 1.230   16 Sep 1997 13:05:08   MIB
   Abs.-Pos. Rahmen am Tabellen-Ende beenden

      Rev 1.229   16 Sep 1997 11:19:12   MIB
   Kopf-/Fusszeilen ohne Moven von Nodes, autom. Beenden von Bereichen/Rahmen

      Rev 1.228   12 Sep 1997 11:54:02   MIB
   fix #41253#: Script in PRE (keine Schleifen)

      Rev 1.227   09 Sep 1997 14:10:48   MIB
   Ueberall Browse-View-Breite statt Seitenbreite verwenden

      Rev 1.226   08 Sep 1997 10:38:54   MIB
   Keine Schleifen fuer PRE mehr (auch fix #41253#) (nicht freigeschaltet)

      Rev 1.225   04 Sep 1997 15:45:30   MIB
   fix: <NOBR> auch beachten, wenn danach ein <BR> folgt

      Rev 1.224   04 Sep 1997 09:37:40   MIB
   fix #42771#: Tabellen in Rahmen beachten

      Rev 1.223   29 Aug 1997 16:49:44   OS
   DLL-Umstellung

      Rev 1.222   15 Aug 1997 12:47:38   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.221   12 Aug 1997 13:42:50   OS
   Header-Umstellung

      Rev 1.220   11 Aug 1997 14:04:42   OM
   Headerfile-Umstellung

      Rev 1.219   07 Aug 1997 15:08:18   OM
   Headerfile-Umstellung

      Rev 1.218   04 Aug 1997 13:54:24   MIB
   aboslute psoitioning (fuer fast alle Zeichen-Attribute/-Vorlagen)

      Rev 1.217   31 Jul 1997 10:44:32   MIB
   DIV-Stack weg

      Rev 1.216   25 Jul 1997 11:08:30   MIB
   fix #42109#: Kommentare bei Tabellen in Tabellen beachten

      Rev 1.215   21 Jul 1997 10:55:22   MIB
   fix #41869#: Keile zusaetlichen Splaten bei selbst exportierten Tabellen

      Rev 1.214   16 Jul 1997 18:33:04   MIB
   fix #41669#: Kein Show im Accepted-State, weil Parser sonst zerst. werden kann

      Rev 1.213   15 Jul 1997 13:11:54   MIB
   fix: Tabellen-Ueberschriften immer zentrieren

      Rev 1.212   11 Jul 1997 11:51:30   MIB
   fix: Bei RULES=COLS auch Zeilen-Gruppen umranden

      Rev 1.211   10 Jul 1997 10:30:30   MIB
   fix #41516#: Keine Umrandung bei FRAME ohne BORDER

      Rev 1.210   09 Jul 1997 08:45:16   MIB
   'Ueberschrift wiederholen' uanhaengig von </THEAD>-Position abktivieren

      Rev 1.209   08 Jul 1997 14:16:42   MIB
   Meta-Tags als PostIts und die immer im ersten Body-Absatz verankern

      Rev 1.208   03 Jul 1997 09:37:00   MIB
   HTML-Tabellen: Bei letzter Grafik ohne Groesse sofort anpassen

      Rev 1.207   01 Jul 1997 12:39:14   MIB
   fix: Beim Vergeben von Filler-Boxen tatsaechliche Umrandung betrachten

      Rev 1.206   28 Jun 1997 11:32:00   MIB
   fix: Bei Umrandung und CELLPADDING=0 minimalen Abstand zum Inhalte beachten

      Rev 1.205   27 Jun 1997 15:57:40   MIB
   fix: Pending-Stack bei bCallNextToken beachten

      Rev 1.204   26 Jun 1997 11:54:52   MIB
   fix #41003#: Nimber-Format nur bei Value oder leerer Zelle setzen

      Rev 1.203   26 Jun 1997 10:56:32   MIB
   fix #40941#: Tabellen In Tabellen in Rahmen, wenn sie ausweichen muessen

      Rev 1.202   24 Jun 1997 08:59:54   MIB
   fix: Beim Splitten von Zelle auch Tabellen-Layout anpassen

      Rev 1.201   20 Jun 1997 13:36:08   MIB
   Auch Grafiken in Tabellen asynchron laden

      Rev 1.200   19 Jun 1997 11:57:04   MIB
   Tabellen weichen jetzt Rahmen aus

      Rev 1.199   16 Jun 1997 08:27:56   MIB
   CLASS/ID auch fier Tabellen

      Rev 1.198   12 Jun 1997 15:55:36   MIB
   358-Tabellen korrekt importieren

      Rev 1.197   12 Jun 1997 09:13:42   MIB
   fix: PostIts auch vor Tabellen in Rahmen in die Tabelle verschieben

      Rev 1.196   09 Jun 1997 18:14:08   MIB
   fix: leere Zellen und Zellen, die nur Grafiken enthalten, bekommen einen 2pt-Font

      Rev 1.195   09 Jun 1997 12:56:46   MIB
   fix: Kein GPF beim berechnen der Exportierbarkeit des Layouts mehr

      Rev 1.194   09 Jun 1997 09:07:22   MIB
   fix: Auch aus leeren Absatzen 2 LF entfernen

      Rev 1.193   05 Jun 1997 17:42:38   MIB
   fix: <NOBR> auch mit Space dahinter, Anpassungen an Export-Moeglichkeit

      Rev 1.192   02 Jun 1997 12:01:18   MIB
   fix: Bei vorhandener Umrandung deren Breite doch beruecksichtigen

      Rev 1.191   30 May 1997 18:02:42   MIB
   NOWRAP/NOBR, Einhalten der Mindestbreite bei rel. Tabellen

      Rev 1.190   28 May 1997 19:14:26   MIB
   fix: Layout-Info nicht im Destr. loeschen, TEST_RESIZE entfernt

      Rev 1.189   28 May 1997 16:11:52   MIB
   fix: Numerierung ueber Tabellen-Ueberschrift retten

      Rev 1.188   28 May 1997 15:35:34   MIB
   fix: -Angabe aus dem Format fuer Rahmen zu nehmen war keine gute Idee

      Rev 1.187   27 May 1997 11:47:08   MIB
   Netscape-konformere Beruecksichtigung von BORDER/CELLPADDING/CELLSPACING

      Rev 1.186   15 May 1997 15:42:36   MIB
   HTML-Tabellen-Layout an SwTable merken

      Rev 1.185   06 May 1997 12:07:54   MA
   swtablehxx aufgeteilt

      Rev 1.184   30 Apr 1997 18:10:02   MIB
   Ermitteln der verfuegbaren Breite ueber Layout

      Rev 1.183   30 Apr 1997 14:54:46   MIB
   Tabellen-Layout ausgelagert

      Rev 1.182   23 Apr 1997 14:57:04   MIB
   memory leak beseitig (wie 1.171.1.0)

      Rev 1.181   21 Apr 1997 18:08:18   MIB
   fix #38931#: 1. Absaetze hinter Tab in Tab nicht numerieren, Zelle hinter T in T

      Rev 1.180   16 Apr 1997 18:23:12   MIB
   fix #37769#: Tabellenbreite sinnvoll begrenzen und USHRT_MAX-Ueberlauf vermeiden

      Rev 1.179   16 Apr 1997 15:19:26   MIB
   fix #36787#: Ersatzdarstellung von Frames, Applets etc. auch in Tabellen ignorieren

      Rev 1.178   14 Apr 1997 16:44:48   MIB
   fixes #36418#, #36819#: Tabellen mit relativen/nicht relativen Spalten gleichz.

      Rev 1.177   11 Apr 1997 17:58:52   MIB
   fix #36820#: Bei Tabellen in Rahmen Absatz-Abstand beachten

      Rev 1.176   10 Apr 1997 16:40:00   MIB
   fix #37987#: Andere Berechnung der Spalten-Mindestbreite

      Rev 1.175   09 Apr 1997 16:50:50   MIB
   fix #38575#: Nicht immer einen Absatz vor Tabellen in Rahmen aufmachen

      Rev 1.174   08 Apr 1997 11:07:02   MIB
   fix #37219#: In Tabellen-Beschriftung alles erlauben

      Rev 1.173   04 Apr 1997 11:04:22   AMA
   Chg: Ein weiterer Parameter fuer GetMinMaxSize: das absolute Minimum

      Rev 1.172   01 Apr 1997 10:47:08   MIB
   fix #38284#: keine gelegentlich vertikale Ausrichtung an Zellen ohne Inhalt mehr

      Rev 1.171   18 Mar 1997 19:06:44   MIB
   fix #37886#: Kein Show beim Abbrechen mehr aufrufen, weil ViewShell tot sein koennte

      Rev 1.170   18 Mar 1997 09:42:50   MIB
   fix #37795#: Tabellen in Tabellen-Ueberschriften abfangen

      Rev 1.169   05 Mar 1997 14:48:36   MIB
   Absatz-Abstaende verbessert

      Rev 1.168   22 Feb 1997 20:30:32   MIB
   Tabellen-Layout an Netscape 4.0 Preview 2 anpassen

      Rev 1.167   21 Feb 1997 15:28:26   MIB
   fix #36692#: Fuer ::com::sun::star::form::Forms in Tabellen ausserhalb von Zellen keine neuen Absaetze

      Rev 1.166   20 Feb 1997 17:04:46   MIB
   bug fix: SaveState auch fuer Col-Groups


*************************************************************************/

