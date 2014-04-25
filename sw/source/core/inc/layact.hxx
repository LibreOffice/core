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

    bool bPaint;        // painting or only formatting?
    bool bComplete;     //Alles bis zum sichtbaren Bereich Formatieren oder
                        // or only the visible area?
    bool bCalcLayout;   //Vollstaendige Reformatierung?
    bool bAgain;        //Zur automatisch wiederholten Action wenn Seiten
                        //geloscht werden.
    bool bNextCycle;    //Wiederaufsetzen bei der ersten Ungueltigen Seite.
    bool bInput;        //Zum Abbrechen der Verarbeitung wenn ein Input anliegt.
    bool bIdle;         //True wenn die Layaction vom Idler ausgeloest wurde.
    bool bReschedule;   //Soll das Reschedule - abhaengig vom Progress -
                        //gerufen werden?
    bool bCheckPages;   //CheckPageDescs() ausfuehren oder verzoegern.
    bool bUpdateExpFlds;//Wird gesetzt wenn nach dem Formatierien noch eine
                        //Runde fuer den ExpFld laufen muss.
    bool bBrowseActionStop; //Action fruehzeitig beenden (per bInput) und den
                            //Rest dem Idler ueberlassen.
    bool bWaitAllowed;      //Wartecursor erlaubt?
    bool bPaintExtraData;   //Anzeige von Zeilennumerierung o. ae. eingeschaltet?
    bool bActionInProgress; // wird in Action() anfangs gesetzt und zum Schluss geloescht

    // OD 14.04.2003 #106346# - new flag for content formatting on interrupt.
    bool    mbFormatCntntOnInterrupt;

#ifdef _LAYACT_CXX

    void PaintCntnt( const SwCntntFrm *, const SwPageFrm *,
                     const SwRect &rOldRect, long nOldBottom );
    bool PaintWithoutFlys( const SwRect &, const SwCntntFrm *,
                           const SwPageFrm * );
    inline bool _PaintCntnt( const SwCntntFrm *, const SwPageFrm *,
                             const SwRect & );

    bool FormatLayout( SwLayoutFrm *, bool bAddRect = true );
    bool FormatLayoutTab( SwTabFrm *, bool bAddRect = true );
    bool FormatCntnt( const SwPageFrm* pPage );
    void _FormatCntnt( const SwCntntFrm* pCntnt,
                       const SwPageFrm* pPage );
    bool IsShortCut( SwPageFrm *& );

    bool TurboAction();
    bool _TurboAction( const SwCntntFrm * );
    void InternalAction();

    SwPageFrm *CheckFirstVisPage( SwPageFrm *pPage );

    bool RemoveEmptyBrowserPages();

    inline void CheckIdleEnd();
    inline sal_uLong GetStartTicks() { return nStartTicks; }

#endif

public:
    SwLayAction( SwRootFrm *pRt, SwViewImp *pImp );
    ~SwLayAction();

#ifdef _LAYACT_CXX
    void SetIdle            ( bool bNew )   { bIdle = bNew; }
    void SetCheckPages      ( bool bNew )   { bCheckPages = bNew; }
    void SetBrowseActionStop( bool bNew )   { bBrowseActionStop = bNew; }
    void SetNextCycle       ( bool bNew )   { bNextCycle = bNew; }

    bool IsWaitAllowed()        const       { return bWaitAllowed; }
    bool IsNextCycle()          const       { return bNextCycle; }
    bool IsInput()              const       { return bInput; }
    bool IsWait()               const       { return 0 != pWait;  }
    bool IsPaint()              const       { return bPaint; }
    bool IsIdle()               const       { return bIdle;  }
    bool IsReschedule()         const       { return bReschedule;  }
    bool IsPaintExtraData()     const       { return bPaintExtraData;}
    bool IsInterrupt()          const       { return IsInput(); }

    sal_uInt16 GetInputType()    const { return nInputType; }
#endif

    // adjusting Action to the wanted behaviour
    void SetPaint       ( bool bNew )   { bPaint = bNew; }
    void SetComplete    ( bool bNew )   { bComplete = bNew; }
    void SetStatBar     ( bool bNew );
    void SetInputType   ( sal_uInt16 nNew ) { nInputType = nNew; }
    void SetCalcLayout  ( bool bNew )   { bCalcLayout = bNew; }
    void SetReschedule  ( bool bNew )   { bReschedule = bNew; }
    void SetWaitAllowed ( bool bNew )   { bWaitAllowed = bNew; }

    void SetAgain()         { bAgain = true; }
    void SetUpdateExpFlds() {bUpdateExpFlds = true; }

    inline void SetCheckPageNum( sal_uInt16 nNew );
    inline void SetCheckPageNumDirect( sal_uInt16 nNew ) { nCheckPageNum = nNew; }

    void Action();  // here it begins
    void Reset();   // back to CTor-defaults

    bool IsAgain()      const { return bAgain; }
    bool IsComplete()   const { return bComplete; }
    bool IsExpFlds()    const { return bUpdateExpFlds; }
    bool IsCalcLayout() const { return bCalcLayout;  }
    bool IsCheckPages() const { return bCheckPages;  }
    bool IsBrowseActionStop() const { return bBrowseActionStop; }
    bool IsActionInProgress() const { return bActionInProgress; }

    sal_uInt16 GetCheckPageNum() const { return nCheckPageNum; }

    // others should be able to activate the WaitCrsr, too
    void CheckWaitCrsr();

    // #i28701# - method is now public;
    // delete 2nd parameter, because its not used;
    bool FormatLayoutFly( SwFlyFrm * );
    // #i28701# - method is now public
    bool _FormatFlyCntnt( const SwFlyFrm * );

};

class SwLayIdle
{

    SwRootFrm *pRoot;
    SwViewImp  *pImp;           // Hier Meldet sich der Idler an und ab.
    SwCntntNode *pCntntNode;    // Hier wird die aktuelle Cursorposition
    sal_Int32  nTxtPos;        // zwischengespeichert.
    bool        bPageValid;     // Konnte die Seite alles validiert werden?
    bool        bAllValid;      // Konnte alles validiert werden?

#ifdef DBG_UTIL
    bool m_bIndicator;
#endif

#ifdef _LAYACT_CXX

#ifdef DBG_UTIL
    void ShowIdle( ColorData eName );
#endif

    enum IdleJobType{ ONLINE_SPELLING, AUTOCOMPLETE_WORDS, WORD_COUNT, SMART_TAGS };
    bool _DoIdleJob( const SwCntntFrm*, IdleJobType );
    bool DoIdleJob( IdleJobType, bool bVisAreaOnly );

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
