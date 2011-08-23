/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _TBLSEL_HXX
#define _TBLSEL_HXX


#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
namespace binfilter {

class SwCrsrShell;
class SwCursor;
class SwTableCursor;
class SwFrm;
class SwTabFrm;
class SwTableBox;
class SwTableLine;
class SwLayoutFrm;
class SwPaM;
class SwNode;
class SwTable;
class SwCellFrm;

SV_DECL_PTRARR( SwCellFrms, SwCellFrm*, 16, 16 )
SV_DECL_PTRARR_SORT( SwSelBoxes, SwTableBoxPtr, 10, 20 )


//Sucht alle Boxen zusammen, die in der Tabelle selektiert sind.
//Je nach enum-Parameter wird die Selektion in der angegebenen Richtung
//erweitert.
//Die Boxen werden ueber das Layout zusammengsucht, es wird auch bei
//aufgespaltenen Tabellen korrekt gearbeitet (siehe: MakeSelUnions()).
enum SwTblSearchType
{
    TBLSEARCH_NONE		= 0x1,       // keine Erweiterung
    TBLSEARCH_ROW		= 0x2,       // erweiter auf Zeilen
    TBLSEARCH_COL  		= 0x3,       // erweiter auf Spalten

    // als Flag zu den anderen Werten!!
    TBLSEARCH_PROTECT			= 0x8,		// auch geschuetzte Boxen einsammeln
    TBLSEARCH_NO_UNION_CORRECT	= 0x10		// die zusammenges. Union nicht korrigieren
};

//wie vor, jedoch wird nicht von der Selektion sondern von den
//Start- EndFrms ausgegangen.
void GetTblSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, const SwTblSearchType = TBLSEARCH_NONE );

// suche fuer eine AutoSumme die beteiligten Boxen zusammen
BOOL GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

// teste, ob die Selektion ausgeglichen ist
SV_DECL_PTRARR( SwChartBoxes, SwTableBoxPtr, 16, 16)//STRIP008 ;
SV_DECL_PTRARR_DEL( SwChartLines, SwChartBoxes*, 25, 50)//STRIP008 ;

BOOL ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd,
                    SwChartLines* pGetCLines = 0 );

// teste ob die Celle in die SSelection gehoert
// (wurde eine Funktion, damit GetTblSel() und MakeTblCrsr() immer
// das "gleiche Verstaendnis" fuer die Selektion haben)
BOOL IsFrmInTblSel( const SwRect& rUnion, const SwFrm* pCell );

// bestimme die Boxen, die zusammen gefasst werden sollen.
// Dabei wird auf Layout Basis das Rechteck "angepasst". D.H. es
// werden Boxen zugefuegt wenn welche an den Seiten ueberlappen
// Zusaetzlich wird die neue Box erzeugt und mit dem entsprechenden
// Inhalt gefuellt.
void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
                  SwTableBox** ppMergeBox );

// teste ob die selektierten Boxen ein gueltiges Merge erlauben
USHORT CheckMergeSel( const SwPaM& rPam );
USHORT CheckMergeSel( const SwSelBoxes& rBoxes );

BOOL IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam );

// teste ob ein Split oder InsertCol dazu fuehrt, das eine Box
// kleiner als MINLAY wird.
BOOL CheckSplitCells( const SwCrsrShell& rShell, USHORT nDiv,
                        const SwTblSearchType = TBLSEARCH_NONE );
BOOL CheckSplitCells( const SwCursor& rCrsr, USHORT nDiv,
                        const SwTblSearchType = TBLSEARCH_NONE );

//Fuer das Arbeiten auf TabSelektion auch fuer aufgespaltene Tabellen.
class SwSelUnion
{
    SwRect   aUnion;		//Das die Sel umschliessende Rechteck.
    SwTabFrm *pTable;		//Die (Follow-)Table zu der Union.

public:
    SwSelUnion( const SwRect &rRect, SwTabFrm *pTab ) :
        aUnion( rRect ), pTable( pTab ) {}

    const SwRect&	GetUnion() const { return aUnion; }
          SwRect&	GetUnion()		 { return aUnion; }
    const SwTabFrm *GetTable() const { return pTable; }
          SwTabFrm *GetTable()		 { return pTable; }
};

SV_DECL_PTRARR_DEL( SwSelUnions, SwSelUnion*, 10, 20 )

//Ermittelt die von einer Tabellenselektion betroffenen Tabellen und die
//Union-Rechteckte der Selektionen - auch fuer aufgespaltene Tabellen.
//Wenn ein Parameter != TBLSEARCH_NONE uebergeben wird, so wird die
//Selektion in der angegebenen Richtung erweitert.
void MakeSelUnions( SwSelUnions&, const SwLayoutFrm *pStart,
                    const SwLayoutFrm *pEnd,
                    const SwTblSearchType = TBLSEARCH_NONE );


// -------------------------------------------------------------------
// Diese Klassen kopieren die aktuelle Tabellen-Selektion (rBoxes)
// unter Beibehaltung der Tabellen-Strubktur in eine eigene Struktur

class _FndBox;
class _FndLine;

SV_DECL_PTRARR_DEL( _FndBoxes, _FndBox*, 10, 20 )
SV_DECL_PTRARR_DEL( _FndLines, _FndLine*,10, 20 )

class _FndBox
{
    SwTableBox* pBox;
    _FndLines aLines;
    _FndLine* pUpper;

    SwTableLine *pLineBefore;	//Zum Loeschen/Restaurieren des Layouts.
    SwTableLine *pLineBehind;

public:
    _FndBox( SwTableBox* pB, _FndLine* pFL ) :
        pBox(pB), pUpper(pFL), pLineBefore( 0 ), pLineBehind( 0 ) {}

    const _FndLines&	GetLines() const	{ return aLines; }
        _FndLines& 		GetLines() 			{ return aLines; }
    const SwTableBox* 	GetBox() const 		{ return pBox; }
        SwTableBox* 	GetBox() 			{ return pBox; }
    const _FndLine* 	GetUpper() const 	{ return pUpper; }
        _FndLine* 		GetUpper() 			{ return pUpper; }

    void SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable );
    void SetTableLines( const SwTable &rTable );
    void DelFrms ( SwTable &rTable );
    void MakeFrms( SwTable &rTable );
    void MakeNewFrms( SwTable &rTable, const USHORT nNumber,
                                       const BOOL bBehind );
    BOOL AreLinesToRestore( const SwTable &rTable ) const;

    //Auch die Charts zu der Tabelle muessen etwas von den Veraenderungen
    //mitbekommen.
    void SaveChartData( const SwTable &rTable );
    void RestoreChartData( const SwTable &rTable );
};


class _FndLine
{
    SwTableLine* pLine;
    _FndBoxes aBoxes;
    _FndBox* pUpper;
public:
    _FndLine(SwTableLine* pL, _FndBox* pFB=0) : pLine(pL), pUpper(pFB) {}
    const _FndBoxes& 	GetBoxes() const 	{ return aBoxes; }
        _FndBoxes& 		GetBoxes() 			{ return aBoxes; }
    const SwTableLine* 	GetLine() const 	{ return pLine; }
        SwTableLine* 	GetLine() 			{ return pLine; }
    const _FndBox*	 	GetUpper() const 	{ return pUpper; }
        _FndBox* 		GetUpper() 			{ return pUpper; }

    void SetUpper( _FndBox* pUp ) { pUpper = pUp; }
};


struct _FndPara
{
    _FndBox* pFndBox;
    _FndLine* pFndLine;
    const SwSelBoxes& rBoxes;

    _FndPara( const SwSelBoxes& rBxs, _FndBox* pFB )
        : rBoxes( rBxs ), pFndBox( pFB ) {}
    _FndPara( const _FndPara& rPara, _FndBox* pFB )
        : rBoxes(rPara.rBoxes), pFndLine(rPara.pFndLine), pFndBox(pFB) {}
    _FndPara( const _FndPara& rPara, _FndLine* pFL )
        : rBoxes(rPara.rBoxes), pFndLine(pFL), pFndBox(rPara.pFndBox) {}
};

BOOL _FndBoxCopyCol( const SwTableBox*& rpBox, void* pPara );
BOOL _FndLineCopyCol( const SwTableLine*& rpLine, void* pPara );


} //namespace binfilter
#endif	//  _TBLSEL_HXX
