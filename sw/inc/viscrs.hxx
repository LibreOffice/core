/*************************************************************************
 *
 *  $RCSfile: viscrs.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:29 $
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
#ifndef _VISCRS_HXX
#define _VISCRS_HXX

#ifndef _CURSOR_HXX //autogen
#include <vcl/cursor.hxx>
#endif
#include "swcrsr.hxx"
#include "swrect.hxx"
#include "swregion.hxx"

class SwCrsrShell;
class SwShellCrsr;
class Window;


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
    virtual void Timeout();
#endif
    void _SetPosAndShow();

public:
    SwVisCrsr( const SwCrsrShell * pCShell );
    ~SwVisCrsr();

    void Show();
    void Hide();

    FASTBOOL IsVisible() const { return bIsVisible; }
    FASTBOOL IsDragCrsr() const { return bIsDragCrsr; }
    void SetDragCrsr( BOOL bFlag = TRUE ) { bIsDragCrsr = bFlag; }

#ifdef SW_CRSR_TIMER
    FASTBOOL ChgTimerFlag( BOOL bTimerOn = TRUE );
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

    void Paint( const SwRect& rRect );

    virtual void Paint( const Rectangle& rRect );
    virtual void FillRects() = 0;

public:
    SwSelPaintRects( const SwCrsrShell& rCSh );
    ~SwSelPaintRects();

    void Show();
    void Hide();
    void Invalidate( const SwRect& rRect );

    const SwCrsrShell* GetShell() const { return pCShell; }
    // check current MapMode of the shell and set possibly the static members.
    // Optional set the parameters pX, pY
    static void Get1PixelInLogic( const ViewShell& rSh,
                                    long* pX = 0, long* pY = 0 );
};


class SwShellCrsr : public virtual SwCursor, public SwSelPaintRects
{
    // Dokument-Positionen der Start/End-Charakter einer SSelection
    Point aMkPt, aPtPt;
    const SwPosition* pPt;      // fuer Zuordung vom GetPoint() zum aPtPt

    virtual void FillRects();   // fuer Table- und normalen Crsr

public:
    SwShellCrsr( const SwCrsrShell& rCrsrSh, const SwPosition &rPos );
    SwShellCrsr( const SwCrsrShell& rCrsrSh, const SwPosition &rPos,
                    const Point& rPtPos, SwPaM* pRing = 0 );
    SwShellCrsr( SwShellCrsr& );
    virtual ~SwShellCrsr();

    virtual operator SwShellCrsr* ();

    void Show();            // Update und zeige alle Selektionen an
    void Hide();            // verstecke alle Selektionen
    void Invalidate( const SwRect& rRect );

    const Point& GetPtPos() const   { return( SwPaM::GetPoint() == pPt ? aPtPt : aMkPt ); }
          Point& GetPtPos()         { return( SwPaM::GetPoint() == pPt ? aPtPt : aMkPt ); }
    const Point& GetMkPos() const   { return( SwPaM::GetMark() == pPt ? aPtPt : aMkPt ); }
          Point& GetMkPos()         { return( SwPaM::GetMark() == pPt ? aPtPt : aMkPt ); }
    const Point& GetSttPos() const  { return( SwPaM::Start() == pPt ? aPtPt : aMkPt ); }
          Point& GetSttPos()        { return( SwPaM::Start() == pPt ? aPtPt : aMkPt ); }
    const Point& GetEndPos() const  { return( SwPaM::End() == pPt ? aPtPt : aMkPt ); }
          Point& GetEndPos()        { return( SwPaM::End() == pPt ? aPtPt : aMkPt ); }

    virtual void SetMark();

    virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    virtual FASTBOOL MaxReplaceArived();
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

    FASTBOOL UpDown( BOOL bUp, USHORT nCnt = 1 );

    // TRUE: an die Position kann der Cursor gesetzt werden
    virtual FASTBOOL IsAtValidPos( BOOL bPoint = TRUE ) const;

#ifndef PRODUCT
// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//              am sichtbaren Cursor
    virtual FASTBOOL IsSelOvr( int eFlags =
                                ( SELOVER_CHECKNODESSECTION |
                                  SELOVER_TOGGLE | SELOVER_CHANGEPOS ));
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
    SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
                    const SwPosition &rMkPos, const Point& rMkPt,
                    const SwPosition &rPtPos, const Point& rPtPt );
    virtual ~SwShellTableCrsr();

    virtual operator SwShellTableCrsr* ();

    virtual void FillRects();   // fuer Table- und normalen Crsr

    // Pruefe, ob sich der SPoint innerhalb der Tabellen-SSelection befindet
    FASTBOOL IsInside( const Point& rPt ) const;

    virtual void SetMark();
    virtual SwCursor* Create( SwPaM* pRing = 0 ) const;
    virtual operator SwShellCrsr* ();
    virtual operator SwTableCursor* ();
    virtual FASTBOOL MaxReplaceArived();
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

    // TRUE: an die Position kann der Cursor gesetzt werden
    virtual FASTBOOL IsAtValidPos( BOOL bPoint = TRUE ) const;

#ifndef PRODUCT
// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//              am sichtbaren Cursor
    virtual FASTBOOL IsSelOvr( int eFlags =
                                ( SELOVER_CHECKNODESSECTION |
                                  SELOVER_TOGGLE | SELOVER_CHANGEPOS ));
#endif
};



#endif  // _VISCRS_HXX
