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

#include <vcl/inputtypes.hxx>

#include <ctime>

#include <swtypes.hxx>
#include <swrect.hxx>

class SwRootFrame;
class SwLayoutFrame;
class SwPageFrame;
class SwFlyFrame;
class SwContentFrame;
class SwTabFrame;
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
    SwRootFrame  *m_pRoot;
    SwViewShellImp  *m_pImp; // here the action logs in and off

    // For the sake of optimization, so that the tables stick a bit better to
    // the Cursor when hitting return/backspace in front of one.
    // The first TabFrame that paints itself (per page) adds itself to the pointer.
    // The ContentFrames beneath the page do not need to deregister at the Shell for
    // painting.
    const SwTabFrame *m_pOptTab;

    std::unique_ptr<SwWait> m_pWait;

    // If a paragraph (or anything else) moved more than one page when
    // formatting, it adds its new page number here.
    // The InternalAction can then take the appropriate steps.
    sal_uInt16 m_nPreInvaPage;

    std::clock_t m_nStartTicks;      // The Action's starting time; if too much time passes the
                                // WaitCursor can be enabled via CheckWaitCursor()

    VclInputFlags m_nInputType;      // Which input should terminate processing
    sal_uInt16 m_nEndPage;        // StatBar control
    sal_uInt16 m_nCheckPageNum;   // CheckPageDesc() was delayed if != USHRT_MAX
                                // check from this page onwards

    bool m_bPaint;         // painting or only formatting?
    bool m_bComplete;      // Format everything or just the visible Area?
    bool m_bCalcLayout;    // Complete reformatting?
    bool m_bAgain;         // For the automatically repeated Action if Pages are deleted
    bool m_bNextCycle;     // Reset on the first invalid Page
    bool m_bInput;         // For terminating processing on input
    bool m_bIdle;          // True if the LayAction was triggered by the Idler
    bool m_bReschedule;    // Call Reschedule depending on Progress?
    bool m_bCheckPages;    // Run CheckPageDescs() or delay it
    bool m_bUpdateExpFields; // Is set if, after Formatting, we need to do another round for ExpField
    bool m_bBrowseActionStop; // Terminate Action early (as per bInput) and leave the rest to the Idler
    bool m_bWaitAllowed;      // Waitcursor allowed?
    bool m_bPaintExtraData;   // Painting line numbers (or similar) enabled?
    bool m_bActionInProgress; // Is set in Action() at the beginning and deleted at the end

    // OD 14.04.2003 #106346# - new flag for content formatting on interrupt.
    bool    mbFormatContentOnInterrupt;

    void PaintContent( const SwContentFrame *, const SwPageFrame *,
                     const SwRect &rOldRect, long nOldBottom );
    bool PaintWithoutFlys( const SwRect &, const SwContentFrame *,
                           const SwPageFrame * );
    inline bool PaintContent_( const SwContentFrame *, const SwPageFrame *,
                             const SwRect & );

    bool FormatLayout( OutputDevice* pRenderContext, SwLayoutFrame *, bool bAddRect = true );
    bool FormatLayoutTab( SwTabFrame *, bool bAddRect );
    bool FormatContent( const SwPageFrame* pPage );
    void FormatContent_( const SwContentFrame* pContent,
                       const SwPageFrame* pPage );
    bool IsShortCut( SwPageFrame *& );

    bool TurboAction();
    bool TurboAction_( const SwContentFrame * );
    void InternalAction(OutputDevice* pRenderContext);

    static SwPageFrame *CheckFirstVisPage( SwPageFrame *pPage );

    bool RemoveEmptyBrowserPages();

    inline void CheckIdleEnd();

public:
    SwLayAction( SwRootFrame *pRt, SwViewShellImp *pImp );
    ~SwLayAction();

    void SetIdle            ( bool bNew )   { m_bIdle = bNew; }
    void SetCheckPages      ( bool bNew )   { m_bCheckPages = bNew; }
    void SetBrowseActionStop( bool bNew )   { m_bBrowseActionStop = bNew; }
    void SetNextCycle       ( bool bNew )   { m_bNextCycle = bNew; }

    bool IsWaitAllowed()        const       { return m_bWaitAllowed; }
    bool IsNextCycle()          const       { return m_bNextCycle; }
    bool IsInput()              const       { return m_bInput; }
    bool IsPaint()              const       { return m_bPaint; }
    bool IsIdle()               const       { return m_bIdle;  }
    bool IsReschedule()         const       { return m_bReschedule;  }
    bool IsPaintExtraData()     const       { return m_bPaintExtraData;}
    bool IsInterrupt()          const       { return IsInput(); }

    VclInputFlags GetInputType()    const { return m_nInputType; }

    // adjusting Action to the wanted behaviour
    void SetPaint       ( bool bNew )   { m_bPaint = bNew; }
    void SetComplete    ( bool bNew )   { m_bComplete = bNew; }
    void SetStatBar     ( bool bNew );
    void SetInputType   ( VclInputFlags nNew ) { m_nInputType = nNew; }
    void SetCalcLayout  ( bool bNew )   { m_bCalcLayout = bNew; }
    void SetReschedule  ( bool bNew )   { m_bReschedule = bNew; }
    void SetWaitAllowed ( bool bNew )   { m_bWaitAllowed = bNew; }

    void SetAgain()         { m_bAgain = true; }
    void SetUpdateExpFields() {m_bUpdateExpFields = true; }

    inline void SetCheckPageNum( sal_uInt16 nNew );
    void SetCheckPageNumDirect( sal_uInt16 nNew ) { m_nCheckPageNum = nNew; }

    void Action(OutputDevice* pRenderContext); // here it begins
    void Reset();   // back to CTor-defaults

    bool IsAgain()      const { return m_bAgain; }
    bool IsComplete()   const { return m_bComplete; }
    bool IsExpFields()    const { return m_bUpdateExpFields; }
    bool IsCalcLayout() const { return m_bCalcLayout;  }
    bool IsCheckPages() const { return m_bCheckPages;  }
    bool IsBrowseActionStop() const { return m_bBrowseActionStop; }
    bool IsActionInProgress() const { return m_bActionInProgress; }

    sal_uInt16 GetCheckPageNum() const { return m_nCheckPageNum; }

    // others should be able to activate the WaitCursor, too
    void CheckWaitCursor();

    // #i28701# - method is now public;
    // delete 2nd parameter, because it's not used;
    void FormatLayoutFly( SwFlyFrame * );
    // #i28701# - method is now public
    void FormatFlyContent( const SwFlyFrame * );

};

class SwLayIdle
{

    SwRootFrame *pRoot;
    SwViewShellImp  *pImp;           // The Idler registers and deregisters here
    SwContentNode *pContentNode;    // The current cursor position is saved here
    sal_Int32  nTextPos;
    bool        bPageValid;     // Were we able to evaluate everything on the whole page?

#ifdef DBG_UTIL
    bool m_bIndicator;
#endif

#ifdef DBG_UTIL
    void ShowIdle( Color eName );
#endif

    enum IdleJobType{ ONLINE_SPELLING, AUTOCOMPLETE_WORDS, WORD_COUNT, SMART_TAGS };
    bool DoIdleJob_( const SwContentFrame*, IdleJobType );
    bool DoIdleJob( IdleJobType, bool bVisAreaOnly );

public:
    SwLayIdle( SwRootFrame *pRt, SwViewShellImp *pImp );
    ~SwLayIdle();
};

inline void SwLayAction::SetCheckPageNum( sal_uInt16 nNew )
{
    if ( nNew < m_nCheckPageNum )
        m_nCheckPageNum = nNew;
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_LAYACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
