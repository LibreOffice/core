/*************************************************************************
 *
 *  $RCSfile: layact.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _LAYACT_HXX
#define _LAYACT_HXX

#include "swtypes.hxx"
#include "swrect.hxx"

class SwRootFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwFlyFrm;
class SwFlyInCntFrm;
class SwCntntFrm;
class SwTabFrm;
class SwViewImp;
class SwCntntNode;
class SwWait;

//Die Verwendung der LayAction laeuft immer gleich ab:
//
// 1. Erzeugen des LayAction-Objektes.
// 2. Einstellen des gewuenschten Verhaltens mittels der Set-Methoden
// 3. Aufruf von Action()
// 4. Baldiges Zerstoeren des Objektes.
//
// Das Objekt meldet sich im CTor beim SwViewImp an und erst im DTor
// wieder ab! Es handelt sich mithin um ein typisches Stackobjekt.


class SwLayAction
{
    SwRootFrm  *pRoot;
    SwViewImp  *pImp;   //Hier Meldet sich die Action an und ab.

    //Zur Optimierung, damit die Tabellen etwas besser am Crsr kleben beim
    //Return/Backspace davor.
    //Wenn der erste TabFrm, der sich Paintet (pro Seite) traegt sich im
    //Pointer ein. Die CntntFrms unterhalb der Seite brauchen sich
    //dann nicht mehr bei der Shell zum Painten anmelden.
    const SwTabFrm *pOptTab;

    SwWait *pWait;

    //Wenn ein Absatz - oder was auch immer - bei der Formatierung mehr
    //als eine Seite rueckwaerts floss traegt er seine neue Seitennummer
    //hier ein. Die Steuerung der InternalAction kann dann geeignet reagieren.
    USHORT nPreInvaPage;

    ULONG nStartTicks;  //Startzeitpunkt der Aktion, vergeht zu viel Zeit kann
                        //der WaitCrsr per CheckWaitCrsr() eingeschaltet werden.

    USHORT nInputType;  //Bei welchem Input soll die Verarbeitung abgebrochen
                        //werden?
    USHORT nEndPage;    //StatBar Steuerung.
    USHORT nCheckPageNum; //CheckPageDesc() wurde verzoegert wenn != USHRT_MAX
                          //ab dieser Seite Checken.

    BOOL bPaint;        //Painten oder nur Formatieren?
    BOOL bComplete;     //Alles bis zum sichtbaren Bereich Formatieren oder
                        //oder nur den sichtbaren Bereich?
    BOOL bCalcLayout;   //Vollstaendige Reformatierung?
    BOOL bAgain;        //Zur automatisch wiederholten Action wenn Seiten
                        //geloscht werden.
    BOOL bNextCycle;    //Wiederaufsetzen bei der ersten Ungueltigen Seite.
    BOOL bInput;        //Zum Abbrechen der Verarbeitung wenn ein Input anliegt.
    BOOL bIdle;         //True wenn die Layaction vom Idler ausgeloest wurde.
    BOOL bReschedule;   //Soll das Reschedule - abhaengig vom Progress -
                        //gerufen werden?
    BOOL bCheckPages;   //CheckPageDescs() ausfuehren oder verzoegern.
    BOOL bUpdateExpFlds;//Wird gesetzt wenn nach dem Formatierien noch eine
                        //Runde fuer den ExpFld laufen muss.
    BOOL bBrowseActionStop; //Action fruehzeitig beenden (per bInput) und den
                            //Rest dem Idler ueberlassen.
    BOOL bWaitAllowed;      //Wartecursor erlaubt?
    BOOL bPaintExtraData;   //Anzeige von Zeilennumerierung o. ae. eingeschaltet?
    BOOL bActionInProgress; // wird in Action() anfangs gesetzt und zum Schluss geloescht

#ifdef _LAYACT_CXX

    void _AddScrollRect( const SwCntntFrm *, const SwPageFrm *,
                         const SwTwips, const SwTwips );
    void PaintCntnt( const SwCntntFrm *, const SwPageFrm *,
                     const SwRect &rOldRect, long nOldBottom );
    BOOL PaintWithoutFlys( const SwRect &, const SwCntntFrm *,
                           const SwPageFrm * );
    inline BOOL _PaintCntnt( const SwCntntFrm *, const SwPageFrm *,
                             const SwRect & );

    void ChkFlyAnchor( SwFlyFrm *, const SwPageFrm * );

    void FormatFlyLayout( const SwPageFrm * );
    BOOL FormatFlyCntnt( const SwPageFrm * );
    BOOL _FormatFlyCntnt( const SwFlyFrm * );
    BOOL __FormatFlyCntnt( const SwCntntFrm * );
    void FormatFlyInCnt( SwFlyInCntFrm * );
    BOOL FormatLayout( SwLayoutFrm *, BOOL bAddRect = TRUE );
    BOOL FormatLayoutTab( SwTabFrm *, BOOL bAddRect = TRUE );
    BOOL FormatLayoutFly( SwFlyFrm *, BOOL bAddRect = TRUE );
    BOOL FormatCntnt( const SwPageFrm * );
    void _FormatCntnt( const SwCntntFrm *, const SwPageFrm *pPage );
    BOOL IsShortCut( SwPageFrm *& );

    BOOL TurboAction();
    BOOL _TurboAction( const SwCntntFrm * );
    void InternalAction();

    SwPageFrm *CheckFirstVisPage( SwPageFrm *pPage );

    BOOL RemoveEmptyBrowserPages();

    inline void CheckIdleEnd();
    inline ULONG GetStartTicks() { return nStartTicks; }
#endif

public:
    SwLayAction( SwRootFrm *pRt, SwViewImp *pImp );
    ~SwLayAction();

#ifdef _LAYACT_CXX
    void SetIdle            ( BOOL bNew )   { bIdle = bNew; }
    void SetCheckPages      ( BOOL bNew )   { bCheckPages = bNew; }
    void SetBrowseActionStop(BOOL bNew  )   { bBrowseActionStop = bNew; }
    void SetNextCycle       ( BOOL bNew )   { bNextCycle = bNew; }

    BOOL IsWaitAllowed()        const       { return bWaitAllowed; }
    BOOL IsNextCycle()          const       { return bNextCycle; }
    BOOL IsInput()              const       { return bInput; }
    BOOL IsWait()               const       { return 0 != pWait;  }
    BOOL IsPaint()              const       { return bPaint; }
    BOOL IsIdle()               const       { return bIdle;  }
    BOOL IsReschedule()         const       { return bReschedule;  }
    BOOL IsPaintExtraData()     const       { return bPaintExtraData;}

    USHORT GetInputType()    const { return nInputType; }
#endif

    //Einstellen der Action auf das gewuenschte Verhalten.
    void SetPaint       ( BOOL bNew )   { bPaint = bNew; }
    void SetComplete    ( BOOL bNew )   { bComplete = bNew; }
    void SetStatBar     ( BOOL bNew );
    void SetInputType   ( USHORT nNew ) { nInputType = nNew; }
    void SetCalcLayout  ( BOOL bNew )   { bCalcLayout = bNew; }
    void SetReschedule  ( BOOL bNew )   { bReschedule = bNew; }
    void SetWaitAllowed ( BOOL bNew )   { bWaitAllowed = bNew; }

    void SetAgain()         { bAgain = TRUE; }
    void SetUpdateExpFlds() {bUpdateExpFlds = TRUE; }

    inline void SetCheckPageNum( USHORT nNew );
    inline void SetCheckPageNumDirect( USHORT nNew ) { nCheckPageNum = nNew; }

    void Action();  //Jetzt gehts loos...
    void Reset();   //Zurueck auf CTor-Defaults.

    BOOL IsAgain()      const { return bAgain; }
    BOOL IsComplete()   const { return bComplete; }
    BOOL IsExpFlds()    const { return bUpdateExpFlds; }
    BOOL IsCalcLayout() const { return bCalcLayout;  }
    BOOL IsCheckPages() const { return bCheckPages;  }
    BOOL IsBrowseActionStop() const { return bBrowseActionStop; }
    BOOL IsActionInProgress() const { return bActionInProgress; }

    USHORT GetCheckPageNum() const { return nCheckPageNum; }

    //Auch andere sollen den Wartecrsr einschalten koennen.
    void CheckWaitCrsr();
};


class SwLayIdle
{

    SwRootFrm *pRoot;
    SwViewImp  *pImp;           // Hier Meldet sich der Idler an und ab.
    SwCntntNode *pCntntNode;    // Hier wird die aktuelle Cursorposition
    xub_StrLen  nTxtPos;        // zwischengespeichert.
    BOOL        bPageValid;     // Konnte die Seite alles validiert werden?
    BOOL        bAllValid;      // Konnte alles validiert werden?

#ifndef PRODUCT
    BOOL bIndicator;
#endif

#ifdef _LAYACT_CXX

#ifndef PRODUCT
    void ShowIdle( ColorData eName );
#endif
    BOOL _FormatSpelling( const SwCntntFrm * );
    BOOL FormatSpelling( BOOL );

    BOOL _CollectAutoCmplWords( const SwCntntFrm *, BOOL bOnlyVisArea );
    BOOL CollectAutoCmplWords( BOOL );

#endif

public:
    SwLayIdle( SwRootFrm *pRt, SwViewImp *pImp );
    ~SwLayIdle();
};

inline void SwLayAction::SetCheckPageNum( USHORT nNew )
{
    if ( nNew < nCheckPageNum )
        nCheckPageNum = nNew;
}

#endif  //_LAYACT_HXX
