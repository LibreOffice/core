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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_LAYACT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_LAYACT_HXX
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

// The usage of LayAction is always the same:

// 1. Generation of the LayAction object.
// 2. Specifying the wanted bahaviour via the Set-methods
// 3. Calling Action()
// 4. Soon after that the destruction of the object

// Das Objekt meldet sich im CTor beim SwViewImp an und erst im DTor
// wieder ab! Es handelt sich mithin um ein typisches Stackobjekt.

class SwLayAction
{
    SwRootFrm  *pRoot;
    SwViewImp  *pImp;   // here the action logs in and off

    // for the sake of optimization, so that the tables stick a bit better to the Crsr
    // when hitting return/backspace in front of one
    // Wenn der erste TabFrm, der sich Paintet (pro Seite) traegt sich im
    // Pointer ein. Die CntntFrms unterhalb der Seite brauchen sich
    // dann nicht mehr bei der Shell zum Painten anmelden.
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
    sal_uInt16 nEndPage;    //StatBar control
    sal_uInt16 nCheckPageNum; //CheckPageDesc() was delayed if != USHRT_MAX
                          // check from this page on

    sal_Bool bPaint;        // painting or only formatting?
    sal_Bool bComplete;     //Alles bis zum sichtbaren Bereich Formatieren oder
                        // or only the visible area?
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

    // adjusting Action to the wanted behaviour
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

    void Action();  // here it begins
    void Reset();   // back to CTor-defaults

    sal_Bool IsAgain()      const { return bAgain; }
    sal_Bool IsComplete()   const { return bComplete; }
    sal_Bool IsExpFlds()    const { return bUpdateExpFlds; }
    sal_Bool IsCalcLayout() const { return bCalcLayout;  }
    sal_Bool IsCheckPages() const { return bCheckPages;  }
    sal_Bool IsBrowseActionStop() const { return bBrowseActionStop; }
    sal_Bool IsActionInProgress() const { return bActionInProgress; }

    sal_uInt16 GetCheckPageNum() const { return nCheckPageNum; }

    // others should be able to activate the WaitCrsr, too
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
    sal_Int32  nTxtPos;        // zwischengespeichert.
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

#endif // INCLUDED_SW_SOURCE_CORE_INC_LAYACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
