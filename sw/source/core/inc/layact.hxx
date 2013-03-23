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
#pragma once
#if 1
#include "swtypes.hxx"
#include "swrect.hxx"

class SwRootFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwFlyFrm;
class SwCntntFrm;
class SwTabFrm;
class SwViewImp;
class SwCntntNode;
class SwWait;
// #i28701#
class SfxProgress;

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

    SfxProgress * pProgress;

    //Wenn ein Absatz - oder was auch immer - bei der Formatierung mehr
    //als eine Seite rueckwaerts floss traegt er seine neue Seitennummer
    //hier ein. Die Steuerung der InternalAction kann dann geeignet reagieren.
    sal_uInt16 nPreInvaPage;

    sal_uLong nStartTicks;  //Startzeitpunkt der Aktion, vergeht zu viel Zeit kann
                        //der WaitCrsr per CheckWaitCrsr() eingeschaltet werden.

    sal_uInt16 nInputType;  //Bei welchem Input soll die Verarbeitung abgebrochen
                        //werden?
    sal_uInt16 nEndPage;    //StatBar Steuerung.
    sal_uInt16 nCheckPageNum; //CheckPageDesc() wurde verzoegert wenn != USHRT_MAX
                          //ab dieser Seite Checken.

    sal_Bool bPaint;        //Painten oder nur Formatieren?
    sal_Bool bComplete;     //Alles bis zum sichtbaren Bereich Formatieren oder
                        //oder nur den sichtbaren Bereich?
    sal_Bool bCalcLayout;   //Vollstaendige Reformatierung?
    sal_Bool bAgain;        //Zur automatisch wiederholten Action wenn Seiten
                        //geloscht werden.
    sal_Bool bNextCycle;    //Wiederaufsetzen bei der ersten Ungueltigen Seite.
    sal_Bool bInput;        //Zum Abbrechen der Verarbeitung wenn ein Input anliegt.
    sal_Bool bIdle;         //True wenn die Layaction vom Idler ausgeloest wurde.
    sal_Bool bReschedule;   //Soll das Reschedule - abhaengig vom Progress -
                        //gerufen werden?
    sal_Bool bCheckPages;   //CheckPageDescs() ausfuehren oder verzoegern.
    sal_Bool bUpdateExpFlds;//Wird gesetzt wenn nach dem Formatierien noch eine
                        //Runde fuer den ExpFld laufen muss.
    sal_Bool bBrowseActionStop; //Action fruehzeitig beenden (per bInput) und den
                            //Rest dem Idler ueberlassen.
    sal_Bool bWaitAllowed;      //Wartecursor erlaubt?
    bool bPaintExtraData;   //Anzeige von Zeilennumerierung o. ae. eingeschaltet?
    sal_Bool bActionInProgress; // wird in Action() anfangs gesetzt und zum Schluss geloescht

    // OD 14.04.2003 #106346# - new flag for content formatting on interrupt.
    sal_Bool    mbFormatCntntOnInterrupt;

#ifdef _LAYACT_CXX

    void PaintCntnt( const SwCntntFrm *, const SwPageFrm *,
                     const SwRect &rOldRect, long nOldBottom );
    sal_Bool PaintWithoutFlys( const SwRect &, const SwCntntFrm *,
                           const SwPageFrm * );
    inline sal_Bool _PaintCntnt( const SwCntntFrm *, const SwPageFrm *,
                             const SwRect & );

    sal_Bool FormatLayout( SwLayoutFrm *, sal_Bool bAddRect = sal_True );
    sal_Bool FormatLayoutTab( SwTabFrm *, sal_Bool bAddRect = sal_True );
    sal_Bool FormatCntnt( const SwPageFrm* pPage );
    void _FormatCntnt( const SwCntntFrm* pCntnt,
                       const SwPageFrm* pPage );
    sal_Bool IsShortCut( SwPageFrm *& );

    sal_Bool TurboAction();
    sal_Bool _TurboAction( const SwCntntFrm * );
    void InternalAction();

    SwPageFrm *CheckFirstVisPage( SwPageFrm *pPage );

    sal_Bool RemoveEmptyBrowserPages();

    inline void CheckIdleEnd();
    inline sal_uLong GetStartTicks() { return nStartTicks; }

#endif

public:
    SwLayAction( SwRootFrm *pRt, SwViewImp *pImp );
    ~SwLayAction();

#ifdef _LAYACT_CXX
    void SetIdle            ( sal_Bool bNew )   { bIdle = bNew; }
    void SetCheckPages      ( sal_Bool bNew )   { bCheckPages = bNew; }
    void SetBrowseActionStop(sal_Bool bNew  )   { bBrowseActionStop = bNew; }
    void SetNextCycle       ( sal_Bool bNew )   { bNextCycle = bNew; }

    sal_Bool IsWaitAllowed()        const       { return bWaitAllowed; }
    sal_Bool IsNextCycle()          const       { return bNextCycle; }
    sal_Bool IsInput()              const       { return bInput; }
    sal_Bool IsWait()               const       { return 0 != pWait;  }
    sal_Bool IsPaint()              const       { return bPaint; }
    sal_Bool IsIdle()               const       { return bIdle;  }
    sal_Bool IsReschedule()         const       { return bReschedule;  }
    bool IsPaintExtraData()     const       { return bPaintExtraData;}
    sal_Bool IsStopPrt()          const;
    sal_Bool IsInterrupt()        const { return IsInput() || IsStopPrt(); }

    sal_uInt16 GetInputType()    const { return nInputType; }
#endif

    //Einstellen der Action auf das gewuenschte Verhalten.
    void SetPaint       ( sal_Bool bNew )   { bPaint = bNew; }
    void SetComplete    ( sal_Bool bNew )   { bComplete = bNew; }
    void SetStatBar     ( sal_Bool bNew );
    void SetInputType   ( sal_uInt16 nNew ) { nInputType = nNew; }
    void SetCalcLayout  ( sal_Bool bNew )   { bCalcLayout = bNew; }
    void SetReschedule  ( sal_Bool bNew )   { bReschedule = bNew; }
    void SetWaitAllowed ( sal_Bool bNew )   { bWaitAllowed = bNew; }

    void SetAgain()         { bAgain = sal_True; }
    void SetUpdateExpFlds() {bUpdateExpFlds = sal_True; }
    void SetProgress(SfxProgress * _pProgress = NULL)
    { pProgress = _pProgress; }

    inline void SetCheckPageNum( sal_uInt16 nNew );
    inline void SetCheckPageNumDirect( sal_uInt16 nNew ) { nCheckPageNum = nNew; }

    void Action();  //Jetzt gehts loos...
    void Reset();   //Zurueck auf CTor-Defaults.

    sal_Bool IsAgain()      const { return bAgain; }
    sal_Bool IsComplete()   const { return bComplete; }
    sal_Bool IsExpFlds()    const { return bUpdateExpFlds; }
    sal_Bool IsCalcLayout() const { return bCalcLayout;  }
    sal_Bool IsCheckPages() const { return bCheckPages;  }
    sal_Bool IsBrowseActionStop() const { return bBrowseActionStop; }
    sal_Bool IsActionInProgress() const { return bActionInProgress; }

    sal_uInt16 GetCheckPageNum() const { return nCheckPageNum; }

    //Auch andere sollen den Wartecrsr einschalten koennen.
    void CheckWaitCrsr();

    // #i28701# - method is now public;
    // delete 2nd parameter, because its not used;
    sal_Bool FormatLayoutFly( SwFlyFrm * );
    // #i28701# - method is now public
    sal_Bool _FormatFlyCntnt( const SwFlyFrm * );

};

class SwLayIdle
{

    SwRootFrm *pRoot;
    SwViewImp  *pImp;           // Hier Meldet sich der Idler an und ab.
    SwCntntNode *pCntntNode;    // Hier wird die aktuelle Cursorposition
    xub_StrLen  nTxtPos;        // zwischengespeichert.
    sal_Bool        bPageValid;     // Konnte die Seite alles validiert werden?
    sal_Bool        bAllValid;      // Konnte alles validiert werden?

#ifdef DBG_UTIL
    bool m_bIndicator;
#endif

#ifdef _LAYACT_CXX

#ifdef DBG_UTIL
    void ShowIdle( ColorData eName );
#endif

    enum IdleJobType{ ONLINE_SPELLING, AUTOCOMPLETE_WORDS, WORD_COUNT, SMART_TAGS };    // SMARTTAGS
    sal_Bool _DoIdleJob( const SwCntntFrm*, IdleJobType );
    sal_Bool DoIdleJob( IdleJobType, sal_Bool bVisAreaOnly );

#endif

public:
    SwLayIdle( SwRootFrm *pRt, SwViewImp *pImp );
    ~SwLayIdle();
};

inline void SwLayAction::SetCheckPageNum( sal_uInt16 nNew )
{
    if ( nNew < nCheckPageNum )
        nCheckPageNum = nNew;
}

#endif  //_LAYACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
