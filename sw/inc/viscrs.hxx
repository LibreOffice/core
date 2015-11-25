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
#ifndef INCLUDED_SW_INC_VISCRS_HXX
#define INCLUDED_SW_INC_VISCRS_HXX

#include <config_features.h>

#include <vcl/cursor.hxx>
#include "swcrsr.hxx"
#include "swrect.hxx"
#include "swregion.hxx"

class SwCursorShell;
class SwShellCursor;
class SwTextInputField;

// From here classes/methods for non-text cursor.

class SwVisibleCursor
{
    friend void _InitCore();
    friend void _FinitCore();

    bool m_bIsVisible;
    bool m_bIsDragCursor;

    vcl::Cursor m_aTextCursor;
    const SwCursorShell* m_pCursorShell;

    /// For LibreOfficeKit only - remember what page we were at the last time.
    sal_uInt16 m_nPageLastTime;

    void _SetPosAndShow();

public:
    SwVisibleCursor( const SwCursorShell * pCShell );
    ~SwVisibleCursor();

    void Show();
    void Hide();

    bool IsVisible() const { return m_bIsVisible; }
    void SetDragCursor( bool bFlag = true ) { m_bIsDragCursor = bFlag; }
};

// From here classes/methods for selections.

namespace sdr { namespace overlay { class OverlayObject; }}
namespace sw { namespace overlay { class OverlayRangesOutline; }}

class SwSelPaintRects : public SwRects
{
    friend void _InitCore();
    friend void _FinitCore();

    static long s_nPixPtX, s_nPixPtY;
    static MapMode *s_pMapMode;

    const SwCursorShell* m_pCursorShell;

#if HAVE_FEATURE_DESKTOP || defined(ANDROID)
    sdr::overlay::OverlayObject*    m_pCursorOverlay;

    // access to m_pCursorOverlay for swapContent
    sdr::overlay::OverlayObject* getCursorOverlay() const { return m_pCursorOverlay; }
    void setCursorOverlay(sdr::overlay::OverlayObject* pNew) { m_pCursorOverlay = pNew; }
#endif

    bool m_bShowTextInputFieldOverlay;
    sw::overlay::OverlayRangesOutline* m_pTextInputFieldOverlay;

    void HighlightInputField();

public:
    SwSelPaintRects( const SwCursorShell& rCSh );
    virtual ~SwSelPaintRects();

    virtual void FillRects() = 0;
    /// Fill rStart and rEnd with a rectangle that represents the start and end for selection handles.
    virtual void FillStartEnd(SwRect& rStart, SwRect& rEnd) const = 0;

    // #i75172# in SwCursorShell::CreateCursor() the content of SwSelPaintRects is exchanged. To
    // make a complete swap access to m_pCursorOverlay is needed there
    void swapContent(SwSelPaintRects& rSwap);

    void Show(std::vector<OString>* pSelectionRectangles = nullptr);
    void Hide();
    void Invalidate( const SwRect& rRect );

    inline void SetShowTextInputFieldOverlay( const bool bShow )
    {
        m_bShowTextInputFieldOverlay = bShow;
    }

    const SwCursorShell* GetShell() const { return m_pCursorShell; }
    // check current MapMode of the shell and set possibly the static members.
    // Optional set the parameters pX, pY
    static void Get1PixelInLogic( const SwViewShell& rSh,
                                    long* pX = nullptr, long* pY = nullptr );
};

class SwShellCursor : public virtual SwCursor, public SwSelPaintRects
{
private:
    // Document positions of start/end characters of a SSelection.
    Point m_MarkPt;
    Point m_PointPt;
    const SwPosition* m_pInitialPoint; // For assignment of GetPoint() to m_PointPt.

    using SwCursor::UpDown;

public:
    SwShellCursor( const SwCursorShell& rCursorSh, const SwPosition &rPos );
    SwShellCursor( const SwCursorShell& rCursorSh, const SwPosition &rPos,
                    const Point& rPtPos, SwPaM* pRing = nullptr );
    // note: *intentionally* links the new shell cursor into the old one's Ring
    SwShellCursor( SwShellCursor& );
    virtual ~SwShellCursor();

    virtual void FillRects() override;   // For Table- und normal cursors.
    /// @see SwSelPaintRects::FillStartEnd(), override for text selections.
    virtual void FillStartEnd(SwRect& rStart, SwRect& rEnd) const override;

    void Show();            // Update and display all selections.
    void Hide();            // Hide all selections.
    void Invalidate( const SwRect& rRect );

    const Point& GetPtPos() const   { return (SwPaM::GetPoint() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }
          Point& GetPtPos()         { return (SwPaM::GetPoint() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }
    const Point& GetMkPos() const   { return (SwPaM::GetMark() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }
          Point& GetMkPos()         { return (SwPaM::GetMark() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }
    const Point& GetSttPos() const  { return (SwPaM::Start() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }
          Point& GetSttPos()        { return (SwPaM::Start() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }
    const Point& GetEndPos() const  { return (SwPaM::End() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }
          Point& GetEndPos()        { return (SwPaM::End() == m_pInitialPoint) ? m_PointPt : m_MarkPt; }

    virtual void SetMark() override;

    virtual SwCursor* Create( SwPaM* pRing = nullptr ) const override;

    virtual short MaxReplaceArived() override; //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTableBoxContent( const SwPosition* pPos = nullptr ) override;

    bool UpDown( bool bUp, sal_uInt16 nCnt = 1 );

    // true: Cursor can be set to this position.
    virtual bool IsAtValidPos( bool bPoint = true ) const override;

    virtual bool IsReadOnlyAvailable() const override;

    DECL_FIXEDMEMPOOL_NEWDEL( SwShellCursor )
};

class SwShellTableCursor : public virtual SwShellCursor, public virtual SwTableCursor
{
    /// Left edge of the selection start (top left cell).
    SwRect m_aStart;
    /// Right edge of the selection end (bottom right cell).
    SwRect m_aEnd;
    // The Selection has the same order as the table boxes, i.e.
    // if something is deleted from the one array at a certain position
    // it has to be deleted from the other one as well!!

public:
    SwShellTableCursor( const SwCursorShell& rCursorSh, const SwPosition& rPos );
    SwShellTableCursor( const SwCursorShell& rCursorSh,
                    const SwPosition &rMkPos, const Point& rMkPt,
                    const SwPosition &rPtPos, const Point& rPtPt );
    virtual ~SwShellTableCursor();

    virtual void FillRects() override;   // For table and normal cursor.
    /// @see SwSelPaintRects::FillStartEnd(), override for table selections.
    virtual void FillStartEnd(SwRect& rStart, SwRect& rEnd) const override;

    // Check if SPoint is within table SSelection.
    bool IsInside( const Point& rPt ) const;

    virtual void SetMark() override;
    virtual SwCursor* Create( SwPaM* pRing = nullptr ) const override;

    virtual short MaxReplaceArived() override; //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTableBoxContent( const SwPosition* pPos = nullptr ) override;

    // true: Cursor can be set to this position.
    virtual bool IsAtValidPos( bool bPoint = true ) const override;

};

#endif // INCLUDED_SW_INC_VISCRS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
