/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

    bool bIsVisible : 1;
    bool bIsDragCrsr : 1;

    Cursor aTxtCrsr;
    const SwCrsrShell* pCrsrShell;

    void _SetPosAndShow();

public:
    SwVisCrsr( const SwCrsrShell * pCShell );
    ~SwVisCrsr();

    void Show();
    void Hide();

    bool IsVisible() const { return bIsVisible; }
    void SetDragCrsr( bool bFlag = true ) { bIsDragCrsr = bFlag; }
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
