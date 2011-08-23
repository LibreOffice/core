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
#ifndef _VISCRS_HXX
#define _VISCRS_HXX

#ifndef _CURSOR_HXX //autogen
#include <vcl/cursor.hxx>
#endif
#include "swcrsr.hxx"
#include "swrect.hxx"
#include "swregion.hxx"
class Window;
namespace binfilter {

class SwCrsrShell;
class SwShellCrsr;



// --------  Ab hier Klassen / Methoden fuer den nicht Text-Cursor ------

class SwVisCrsr
#ifdef SW_CRSR_TIMER
                : private Timer
#endif
{
    friend void _InitCore();
    friend void _FinitCore();

    BOOL bIsVisible : 1;
    BOOL bIsDragCrsr : 1;

#ifdef SW_CRSR_TIMER
    BOOL bTimerOn : 1;
#endif

    Cursor aTxtCrsr;
    const SwCrsrShell* pCrsrShell;

#ifdef SW_CRSR_TIMER
#endif
    void _SetPosAndShow();

public:
    ~SwVisCrsr();

    void Show();
    void Hide();

    FASTBOOL IsVisible() const { return bIsVisible; }
    FASTBOOL IsDragCrsr() const { return bIsDragCrsr; }
    void SetDragCrsr( BOOL bFlag = TRUE ) { bIsDragCrsr = bFlag; }

#ifdef SW_CRSR_TIMER
#endif
};


// ------ Ab hier Klassen / Methoden fuer die Selectionen -------

class SwSelPaintRects : public SwRects
{
    friend void _InitCore();
    friend void _FinitCore();

    static long nPixPtX, nPixPtY;
    static MapMode *pMapMode;

    // die Shell
    const SwCrsrShell* pCShell;


    virtual void FillRects() = 0;

public:
    ~SwSelPaintRects();

    void Show();
    void Hide();

    const SwCrsrShell* GetShell() const { return pCShell; }
};


class SwShellCrsr : public virtual SwCursor, public SwSelPaintRects
{
    // Dokument-Positionen der Start/End-Charakter einer SSelection
    Point aMkPt, aPtPt;
    const SwPosition* pPt;		// fuer Zuordung vom GetPoint() zum aPtPt

    virtual void FillRects();	// fuer Table- und normalen Crsr

public:
    SwShellCrsr( const SwCrsrShell& rCrsrSh, const SwPosition &rPos );
    virtual ~SwShellCrsr();

    virtual operator SwShellCrsr* ();

    void Show();			// Update und zeige alle Selektionen an

    const Point& GetPtPos() const	{ return( SwPaM::GetPoint() == pPt ? aPtPt : aMkPt ); }
          Point& GetPtPos() 		{ return( SwPaM::GetPoint() == pPt ? aPtPt : aMkPt ); }
    const Point& GetMkPos() const 	{ return( SwPaM::GetMark() == pPt ? aPtPt : aMkPt ); }
          Point& GetMkPos() 		{ return( SwPaM::GetMark() == pPt ? aPtPt : aMkPt ); }
    const Point& GetSttPos() const	{ return( SwPaM::Start() == pPt ? aPtPt : aMkPt ); }
          Point& GetSttPos() 		{ return( SwPaM::Start() == pPt ? aPtPt : aMkPt ); }
    const Point& GetEndPos() const	{ return( SwPaM::End() == pPt ? aPtPt : aMkPt ); }
          Point& GetEndPos() 		{ return( SwPaM::End() == pPt ? aPtPt : aMkPt ); }




    FASTBOOL UpDown( BOOL bUp, USHORT nCnt = 1 );

    // TRUE: an die Position kann der Cursor gesetzt werden

#ifdef DBG_UTIL
// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//				am sichtbaren Cursor
#endif

    DECL_FIXEDMEMPOOL_NEWDEL( SwShellCrsr )
};



class SwShellTableCrsr : public virtual SwShellCrsr, public virtual SwTableCursor
{
    // die Selection hat die gleiche Reihenfolge wie die
    // TabellenBoxen. D.h., wird aus dem einen Array an einer Position
    // etwas geloescht, dann muss es auch im anderen erfolgen!!


public:
    SwShellTableCrsr( const SwCrsrShell& rCrsrSh, const SwPosition& rPos );
    virtual ~SwShellTableCrsr();

     virtual operator SwShellTableCrsr* ();


    // Pruefe, ob sich der SPoint innerhalb der Tabellen-SSelection befindet

    virtual void SetMark();
    virtual operator SwShellCrsr* ();
    virtual operator SwTableCursor* ();

    // TRUE: an die Position kann der Cursor gesetzt werden

// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//				am sichtbaren Cursor
};



} //namespace binfilter
#endif	// _VISCRS_HXX
