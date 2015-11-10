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

#include <sal/config.h>

#include <ctime>

#include "swtypes.hxx"
#include "swrect.hxx"

class SwRootFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwFlyFrm;
class SwContentFrm;
class SwTabFrm;
class SwViewShellImp;
class SwContentNode;
class SwWait;

/**
 * The usage of LayAction is always the same:
 *
 * 1. Generation of the LayAction object.
 * 2. Specifying the wanted behaviour via the Set-methods
 * 3. Calling Action()
 * 4. Soon after that the destruction of the object
 *
 * The object registers at the SwViewShellImp in the ctor and deregisters not until
 * the dtor!
 * It's a typical stack object.
 */
class SwLayAction
{
    SwRootFrm  *pRoot;
    SwViewShellImp  *pImp; // here the action logs in and off

    // For the sake of optimization, so that the tables stick a bit better to
    // the Crsr when hitting return/backspace in front of one.
    // The first TabFrm that paints itself (per page) adds itself to the pointer.
    // The ContentFrms beneath the page do not need to deregister at the Shell for
    // painting.
    const SwTabFrm *pOptTab;

    SwWait *pWait;

    // If a paragraph (or anything else) moved more than one page when
    // formatting, it adds its new page number here.
    // The InternalAction can then take the appropriate steps.
    sal_uInt16 nPreInvaPage;

    std::clock_t nStartTicks;      // The Action's starting time; if too much time passes the
                                // WaitCrsr can be enabled via CheckWaitCrsr()

    VclInputFlags nInputType;      // Which input should terminate processing
    sal_uInt16 nEndPage;        // StatBar control
    sal_uInt16 nCheckPageNum;   // CheckPageDesc() was delayed if != USHRT_MAX
                                // check from this page onwards

    bool bPaint;         // painting or only formatting?
    bool bComplete;      // Format everything or just the visible Area?
    bool bCalcLayout;    // Complete reformatting?
    bool bAgain;         // For the automatically repeated Action if Pages are deleted
    bool bNextCycle;     // Reset on the first invalid Page
    bool bInput;         // For terminating processing on input
    bool bIdle;          // True if the LayAction was triggered by the Idler
    bool bReschedule;    // Call Reschedule depending on Progress?
    bool bCheckPages;    // Run CheckPageDescs() or delay it
    bool bUpdateExpFields; // Is set if, after Formatting, we need to do another round for ExpField
    bool bBrowseActionStop; // Terminate Action early (as per bInput) and leave the rest to the Idler
    bool bWaitAllowed;      // Waitcursor allowed?
    bool bPaintExtraData;   // Painting line numbers (or similar) enabled?
    bool bActionInProgress; // Is set in Action() at the beginning and deleted at the end

    // OD 14.04.2003 #106346# - new flag for content formatting on interrupt.
    bool    mbFormatContentOnInterrupt;

    void PaintContent( const SwContentFrm *, const SwPageFrm *,
                     const SwRect &rOldRect, long nOldBottom );
    bool PaintWithoutFlys( const SwRect &, const SwContentFrm *,
                           const SwPageFrm * );
    inline bool _PaintContent( const SwContentFrm *, const SwPageFrm *,
                             const SwRect & );

    bool FormatLayout( OutputDevice* pRenderContext, SwLayoutFrm *, bool bAddRect = true );
    bool FormatLayoutTab( SwTabFrm *, bool bAddRect = true );
    bool FormatContent( const SwPageFrm* pPage );
    void _FormatContent( const SwContentFrm* pContent,
                       const SwPageFrm* pPage );
    bool IsShortCut( SwPageFrm *& );

    bool TurboAction();
    bool _TurboAction( const SwContentFrm * );
    void InternalAction(OutputDevice* pRenderContext);

    static SwPageFrm *CheckFirstVisPage( SwPageFrm *pPage );

    bool RemoveEmptyBrowserPages();

    inline void CheckIdleEnd();
    inline std::clock_t GetStartTicks() { return nStartTicks; }

public:
    SwLayAction( SwRootFrm *pRt, SwViewShellImp *pImp );
    ~SwLayAction();

    void SetIdle            ( bool bNew )   { bIdle = bNew; }
    void SetCheckPages      ( bool bNew )   { bCheckPages = bNew; }
    void SetBrowseActionStop( bool bNew )   { bBrowseActionStop = bNew; }
    void SetNextCycle       ( bool bNew )   { bNextCycle = bNew; }

    bool IsWaitAllowed()        const       { return bWaitAllowed; }
    bool IsNextCycle()          const       { return bNextCycle; }
    bool IsInput()              const       { return bInput; }
    bool IsWait()               const       { return nullptr != pWait;  }
    bool IsPaint()              const       { return bPaint; }
    bool IsIdle()               const       { return bIdle;  }
    bool IsReschedule()         const       { return bReschedule;  }
    bool IsPaintExtraData()     const       { return bPaintExtraData;}
    bool IsInterrupt()          const       { return IsInput(); }

    VclInputFlags GetInputType()    const { return nInputType; }

    // adjusting Action to the wanted behaviour
    void SetPaint       ( bool bNew )   { bPaint = bNew; }
    void SetComplete    ( bool bNew )   { bComplete = bNew; }
    void SetStatBar     ( bool bNew );
    void SetInputType   ( VclInputFlags nNew ) { nInputType = nNew; }
    void SetCalcLayout  ( bool bNew )   { bCalcLayout = bNew; }
    void SetReschedule  ( bool bNew )   { bReschedule = bNew; }
    void SetWaitAllowed ( bool bNew )   { bWaitAllowed = bNew; }

    void SetAgain()         { bAgain = true; }
    void SetUpdateExpFields() {bUpdateExpFields = true; }

    inline void SetCheckPageNum( sal_uInt16 nNew );
    inline void SetCheckPageNumDirect( sal_uInt16 nNew ) { nCheckPageNum = nNew; }

    void Action(OutputDevice* pRenderContext); // here it begins
    void Reset();   // back to CTor-defaults

    bool IsAgain()      const { return bAgain; }
    bool IsComplete()   const { return bComplete; }
    bool IsExpFields()    const { return bUpdateExpFields; }
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
    bool _FormatFlyContent( const SwFlyFrm * );

};

class SwLayIdle
{

    SwRootFrm *pRoot;
    SwViewShellImp  *pImp;           // The Idler registers and deregisters here
    SwContentNode *pContentNode;    // The current cursor position is saved here
    sal_Int32  nTextPos;
    bool        bPageValid;     // Were we able to evaluate everything on the whole page?
    bool        bAllValid;      // Were we able to evaluate everything?

#ifdef DBG_UTIL
    bool m_bIndicator;
#endif

#ifdef DBG_UTIL
    void ShowIdle( ColorData eName );
#endif

    enum IdleJobType{ ONLINE_SPELLING, AUTOCOMPLETE_WORDS, WORD_COUNT, SMART_TAGS };
    bool _DoIdleJob( const SwContentFrm*, IdleJobType );
    bool DoIdleJob( IdleJobType, bool bVisAreaOnly );

public:
    SwLayIdle( SwRootFrm *pRt, SwViewShellImp *pImp );
    ~SwLayIdle();
};

inline void SwLayAction::SetCheckPageNum( sal_uInt16 nNew )
{
    if ( nNew < nCheckPageNum )
        nCheckPageNum = nNew;
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_LAYACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
