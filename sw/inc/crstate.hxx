/*************************************************************************
 *
 *  $RCSfile: crstate.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:38:32 $
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
#ifndef _CRSTATE_HXX
#define _CRSTATE_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif

enum SwFillMode
{
    FILL_TAB,       // default, Auffuellen mit Tabulatoren
    FILL_SPACE,     // ... mit Tabulatoren und Spaces
    FILL_MARGIN,    // nur links, zentriert, rechts Ausrichten
    FILL_INDENT     // durch linken Absatzeinzug
};

struct SwFillCrsrPos
{
    SwRect aCrsr;           // Position und Groesse des Shadowcursors
    USHORT nParaCnt;        // Anzahl der einzufuegenden Absaetze
    USHORT nTabCnt;         // Anzahl der Tabs bzw. Groesse des Einzugs
    USHORT nSpaceCnt;       // Anzahl der einzufuegenden Leerzeichen
    USHORT nColumnCnt;      // Anzahl der notwendigen Spaltenumbrueche
    SwHoriOrient eOrient;   // Absatzausrichtung
    SwFillMode eMode;       // Gewuenschte Auffuellregel
    SwFillCrsrPos( SwFillMode eMd = FILL_TAB ) :
        nParaCnt( 0 ), nTabCnt( 0 ), nSpaceCnt( 0 ), nColumnCnt( 0 ),
        eOrient( HORI_NONE ), eMode( eMd )
    {}
};

// Multiportion types: two lines, bidirectional, 270 degrees rotation,
//                     ruby portion and 90 degrees rotation
#define MT_TWOLINE  0
#define MT_BIDI     1
#define MT_ROT_270  3
#define MT_RUBY     4
#define MT_ROT_90   7

struct Sw2LinesPos
{
    SwRect aLine;           // Position and size of the line
    SwRect aPortion;        // Position and size of the multi portion
    SwRect aPortion2;       // needed for nested multi portions
    BYTE nMultiType;        // Multiportion type
};

/**
 *  SwSpecialPos. This structure is used to pass some additional information
 *  during the call of SwTxtFrm::GetCharRect(). An SwSpecialPos defines a position
 *  inside a portion which does not have a representation in the core string or
 *  which is only represented by one position,  e.g., field portions,
 *  number portions, ergo sum and quo vadis portions.
 *
 *  nCharOfst       - The offset inside the special portion. Fields and its
 *                    follow fields are treated as one long special portion.
 *  nLineOfst       - The number of lines between the beginning of the special
 *                    portion and nCharOfst. A line offset required to be
 *                    nCharOfst relative to the beginning of the line.
 *  nExtendRange    - Setting this identifies portions which are in front or
 *                    behind the core string (number portion, quo vadis)
 *
 *  Examples 1)
 *
 *      Get the position of the second character inside a number portion:
 *          nCharOfst = 2; nLineOfst = 0; nExtendRange = SP_EXTEND_RANGE_BEFORE;
 *          Call SwTxtFrm:::GetCharRect with core string position 0.
 *
 *  Example 2)
 *
 *      Field A - Length = 5
 *      Follow field B - Length = 9
 *      Get the position of the third character in follow field B, core position
 *      of field A is 33.
 *          nCharOfst = 7; nLineOfst = 0; nExtendRange = SP_EXTEND_RANGE_NONE;
 *          Call SwTxtFrm:::GetCharRect with core string position 33.
 */

#define SP_EXTEND_RANGE_NONE    0
#define SP_EXTEND_RANGE_BEFORE  1
#define SP_EXTEND_RANGE_BEHIND  2

struct SwSpecialPos
{
    xub_StrLen nCharOfst;
    USHORT nLineOfst;
    BYTE nExtendRange;
};

// CrsrTravelling-Staties (fuer GetCrsrOfst)
enum CrsrMoveState
{
    MV_NONE,            // default
    MV_UPDOWN,          // Crsr Up/Down
    MV_RIGHTMARGIN,     // an rechten Rand
    MV_LEFTMARGIN,      // an linken Rand
    MV_SETONLYTEXT,     // mit dem Cursr nur im Text bleiben
    MV_TBLSEL           // nicht in wiederholte Headlines
};

// struct fuer spaetere Erweiterungen
struct SwCrsrMoveState
{
    SwFillCrsrPos   *pFill;     // fuer das automatische Auffuellen mit Tabs etc.
    Sw2LinesPos     *p2Lines;   // for selections inside/around 2line portions
    SwSpecialPos*   pSpecialPos; // for positions inside fields
    Point aRealHeight;          // enthaelt dann die Position/Hoehe des Cursors
    CrsrMoveState eState;
    BYTE            nCursorBidiLevel;
    BOOL bStop          :1;
    BOOL bRealHeight    :1;     // Soll die reale Hoehe berechnet werden?
    BOOL bFieldInfo     :1;     // Sollen Felder erkannt werden?
    BOOL bPosCorr       :1;     // Point musste korrigiert werden
    BOOL bFtnNoInfo     :1;     // Fussnotennumerierung erkannt
    BOOL bExactOnly     :1;     // GetCrsrOfst nur nach Exakten Treffern
                                // suchen lassen, sprich niemals in das
                                // GetCntntPos laufen.
    BOOL bFillRet       :1;     // wird nur im FillModus temp. genutzt
    BOOL bSetInReadOnly :1;     // ReadOnlyBereiche duerfen betreten werden
    BOOL bRealWidth     :1;     // Calculation of the width required
    BOOL b2Lines        :1;     // Check 2line portions and fill p2Lines
    BOOL bNoScroll      :1;     // No scrolling of undersized textframes

    SwCrsrMoveState( CrsrMoveState eSt = MV_NONE ) :
        pFill( NULL ),
        p2Lines( NULL ),
        pSpecialPos( NULL ),
        eState( eSt ),
        nCursorBidiLevel( 0 ),
        bStop( FALSE ),
        bRealHeight( FALSE ),
        bFieldInfo( FALSE ),
        bPosCorr( FALSE ),
        bFtnNoInfo( FALSE ),
        bExactOnly( FALSE ),
        bSetInReadOnly( FALSE ),
        bRealWidth( FALSE ),
        b2Lines( FALSE ),
        bNoScroll( FALSE )
    {}
    SwCrsrMoveState( SwFillCrsrPos *pInitFill ) :
        pFill( pInitFill ),
        pSpecialPos( NULL ),
        eState( MV_SETONLYTEXT ),
        nCursorBidiLevel( 0 ),
        bStop( FALSE ),
        bRealHeight( FALSE ),
        bFieldInfo( FALSE ),
        bPosCorr( FALSE ),
        bFtnNoInfo( FALSE ),
        bExactOnly( FALSE ),
        bSetInReadOnly( FALSE ),
        bRealWidth( FALSE ),
        b2Lines( FALSE ),
        bNoScroll( FALSE )
    {}
};


#endif


