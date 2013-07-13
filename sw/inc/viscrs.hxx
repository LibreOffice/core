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
#ifndef _VISCRS_HXX
#define _VISCRS_HXX

#include <vcl/cursor.hxx>
#include "swcrsr.hxx"
#include "swrect.hxx"
#include "swregion.hxx"

class SwCrsrShell;
class SwShellCrsr;

// From here classes/methods for non-text cursor.

class SwVisCrsr
{
    friend void _InitCore();
    friend void _FinitCore();

    bool m_bIsVisible;
    bool m_bIsDragCrsr;

    Cursor m_aTxtCrsr;
    const SwCrsrShell* m_pCrsrShell;

    void _SetPosAndShow();

public:
    SwVisCrsr( const SwCrsrShell * pCShell );
    ~SwVisCrsr();

    void Show();
    void Hide();

    bool IsVisible() const { return m_bIsVisible; }
    void SetDragCrsr( bool bFlag = true ) { m_bIsDragCrsr = bFlag; }
};


// From here classes/methods for selections.

namespace sdr { namespace overlay { class OverlayObject; }}

class SwSelPaintRects : public SwRects
{
    friend void _InitCore();
    friend void _FinitCore();

    static long nPixPtX, nPixPtY;
    static MapMode *pMapMode;

    const SwCrsrShell* pCShell;

    virtual void Paint( const Rectangle& rRect );
    virtual void FillRects() = 0;

    sdr::overlay::OverlayObject*    mpCursorOverlay;

    // access to mpCursorOverlay for swapContent
    sdr::overlay::OverlayObject* getCursorOverlay() const { return mpCursorOverlay; }
    void setCursorOverlay(sdr::overlay::OverlayObject* pNew) { mpCursorOverlay = pNew; }

public:
    SwSelPaintRects( const SwCrsrShell& rCSh );
    virtual ~SwSelPaintRects();

    // in SwCrsrShell::CreateCrsr() the content of SwSelPaintRects is exchanged. To
    // make a complete swap access to mpCursorOverlay is needed there
    void swapContent(SwSelPaintRects& rSwap);

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
    // Document positions of start/end characters of a SSelection.
    Point aMkPt, aPtPt;
    const SwPosition* pPt;      // For assignment of GetPoint() to aPtPt.

    virtual void FillRects();   // For Table- und normal cursors.

    using SwCursor::UpDown;

public:
    SwShellCrsr( const SwCrsrShell& rCrsrSh, const SwPosition &rPos );
    SwShellCrsr( const SwCrsrShell& rCrsrSh, const SwPosition &rPos,
                    const Point& rPtPos, SwPaM* pRing = 0 );
    SwShellCrsr( SwShellCrsr& );
    virtual ~SwShellCrsr();

    void Show();            // Update and display all selections.
    void Hide();            // Hide all selections.
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

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

    sal_Bool UpDown( sal_Bool bUp, sal_uInt16 nCnt = 1 );

    // sal_True: Cursor can be set to this position.
    virtual sal_Bool IsAtValidPos( sal_Bool bPoint = sal_True ) const;

    virtual bool IsReadOnlyAvailable() const;

    DECL_FIXEDMEMPOOL_NEWDEL( SwShellCrsr )
};



class SwShellTableCrsr : public virtual SwShellCrsr, public virtual SwTableCursor
{
    // The Selection has the same order as the table boxes, i.e.
    // if something is deleted from the one array at a certain position
    // it has to be deleted from the other one as well!!

public:
    SwShellTableCrsr( const SwCrsrShell& rCrsrSh, const SwPosition& rPos );
    SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
                    const SwPosition &rMkPos, const Point& rMkPt,
                    const SwPosition &rPtPos, const Point& rPtPt );
    virtual ~SwShellTableCrsr();

    virtual void FillRects();   // For table and normal cursor.

    // Check if SPoint is within table SSelection.
    sal_Bool IsInside( const Point& rPt ) const;

    virtual void SetMark();
    virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

    // sal_True: Cursor can be set to this position.
    virtual sal_Bool IsAtValidPos( sal_Bool bPoint = sal_True ) const;

};

#endif  // _VISCRS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
