/*************************************************************************
 *
 *  $RCSfile: crstate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-28 14:26:32 $
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
    Point aRealHeight;          // enthaelt dann die Position/Hoehe des Cursors
    CrsrMoveState eState;
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

    SwCrsrMoveState( CrsrMoveState eSt = MV_NONE ) :
        pFill( NULL ),
        eState( eSt ),
        bStop( FALSE ),
        bRealHeight( FALSE ),
        bFieldInfo( FALSE ),
        bPosCorr( FALSE ),
        bFtnNoInfo( FALSE ),
        bExactOnly( FALSE ),
        bSetInReadOnly( FALSE ),
        bRealWidth( FALSE )
    {}
    SwCrsrMoveState( SwFillCrsrPos *pInitFill ) :
        pFill( pInitFill ),
        eState( MV_SETONLYTEXT ),
        bStop( FALSE ),
        bRealHeight( FALSE ),
        bFieldInfo( FALSE ),
        bPosCorr( FALSE ),
        bFtnNoInfo( FALSE ),
        bExactOnly( FALSE ),
        bSetInReadOnly( FALSE ),
        bRealWidth( FALSE )
    {}
};


#endif


