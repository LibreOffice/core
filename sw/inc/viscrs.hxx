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

#include <config_feature_desktop.h>

#include <vcl/cursor.hxx>
#include "swcrsr.hxx"
#include "swrect.hxx"
#include "swregion.hxx"
#include "swdllapi.h"

#include <optional>

namespace sdr::overlay { class OverlaySelection; }

class SwCursorShell;
class SfxViewShell;
class SwContentControlButton;

// From here classes/methods for non-text cursor.

class SW_DLLPUBLIC SwVisibleCursor
{
    friend void InitCore();
    friend void FinitCore();

    const SwCursorShell* m_pCursorShell;
    vcl::Cursor m_aTextCursor;

    /// For LibreOfficeKit only - remember what page we were at the last time.
    sal_uInt16 m_nPageLastTime;
    SwRect m_aLastLOKRect;

    bool m_bIsVisible;
    bool m_bIsDragCursor;

public:
    SwVisibleCursor( const SwCursorShell * pCShell );
    ~SwVisibleCursor();

    void Show();
    void Hide();

    bool IsVisible() const { return m_bIsVisible; }
    void SetDragCursor( bool bFlag = true ) { m_bIsDragCursor = bFlag; }
    void SetPosAndShow(SfxViewShell const * pViewShell);
    const vcl::Cursor& GetTextCursor() const;

    std::optional<OString> getLOKPayload(int nType, int nViewId) const;
};

// From here classes/methods for selections.

namespace sw::overlay { class OverlayRangesOutline; }
class MapMode;

/**
 * SwSelPaintRects is a base class of SwShellCursor, which stores the overlay for the
 * cursor/selection: a list of rectangles to be painted as an overlay. It is also capable of
 * painting other overlays as needed (text input, content control).
 */
class SwSelPaintRects : public SwRects
{
    friend void InitCore();
    friend void FinitCore();

    static tools::Long s_nPixPtX, s_nPixPtY;
    static MapMode *s_pMapMode;

    const SwCursorShell* m_pCursorShell;

#if HAVE_FEATURE_DESKTOP || defined(ANDROID)
    std::unique_ptr<sdr::overlay::OverlaySelection> m_pCursorOverlay;
#endif

    bool m_bShowTextInputFieldOverlay;
    std::unique_ptr<sw::overlay::OverlayRangesOutline> m_pTextInputFieldOverlay;
    bool m_bShowContentControlOverlay;
    std::unique_ptr<sw::overlay::OverlayRangesOutline> m_pContentControlOverlay;
    VclPtr<SwContentControlButton> m_pContentControlButton;

    void HighlightInputField();
    void HighlightContentControl();

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

    void SetShowTextInputFieldOverlay( const bool bShow )
    {
        m_bShowTextInputFieldOverlay = bShow;
    }

    void SetShowContentControlOverlay(const bool bShow) { m_bShowContentControlOverlay = bShow; }

    const VclPtr<SwContentControlButton>& GetContentControlButton() const;

    const SwCursorShell* GetShell() const { return m_pCursorShell; }
    // check current MapMode of the shell and set possibly the static members.
    // Optional set the parameters pX, pY
    static void Get1PixelInLogic( const SwViewShell& rSh,
                                    tools::Long* pX = nullptr, tools::Long* pY = nullptr );

    std::optional<OString> getLOKPayload(int nType, int nViewId) const;
};

/// Represents the current text cursor of one opened edit window.
///
/// See SwShellTableCursor for a table cursor that extends this class.
class SAL_DLLPUBLIC_RTTI SwShellCursor : public virtual SwCursor, public SwSelPaintRects
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
                    const Point& rPtPos, SwPaM* pRing );
    // note: *intentionally* links the new shell cursor into the old one's Ring
    SwShellCursor( SwShellCursor& );
    virtual ~SwShellCursor() override;

    virtual void FillRects() override;   // For Table- and normal cursors.
    /// @see SwSelPaintRects::FillStartEnd(), override for text selections.
    virtual void FillStartEnd(SwRect& rStart, SwRect& rEnd) const override;

    void Show(SfxViewShell const * pViewShell); // Update and display all selections.
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
    virtual void SaveTableBoxContent( const SwPosition* pPos ) override;

    bool UpDown( bool bUp, sal_uInt16 nCnt );

    // true: Cursor can be set to this position.
    virtual bool IsAtValidPos( bool bPoint = true ) const override;

    virtual bool IsReadOnlyAvailable() const override;

    SwShellCursor* GetNext()             { return dynamic_cast<SwShellCursor *>(GetNextInRing()); }
    const SwShellCursor* GetNext() const { return dynamic_cast<SwShellCursor const *>(GetNextInRing()); }
    SwShellCursor* GetPrev()             { return dynamic_cast<SwShellCursor *>(GetPrevInRing()); }
    const SwShellCursor* GetPrev() const { return dynamic_cast<SwShellCursor const *>(GetPrevInRing()); }
};

class SwShellTableCursor final : public virtual SwShellCursor, public virtual SwTableCursor
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
    virtual ~SwShellTableCursor() override;

    virtual void FillRects() override;   // For table and normal cursor.
    /// @see SwSelPaintRects::FillStartEnd(), override for table selections.
    virtual void FillStartEnd(SwRect& rStart, SwRect& rEnd) const override;

    // Check if SPoint is within table SSelection.
    bool Contains( const Point& rPt ) const;

    virtual void SetMark() override;
    virtual SwCursor* Create( SwPaM* pRing = nullptr ) const override;

    virtual short MaxReplaceArived() override; //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTableBoxContent( const SwPosition* pPos ) override;

    // true: Cursor can be set to this position.
    virtual bool IsAtValidPos( bool bPoint = true ) const override;

    SwShellTableCursor* GetNext()             { return dynamic_cast<SwShellTableCursor *>(GetNextInRing()); }
    const SwShellTableCursor* GetNext() const { return dynamic_cast<SwShellTableCursor const *>(GetNextInRing()); }
    SwShellTableCursor* GetPrev()             { return dynamic_cast<SwShellTableCursor *>(GetPrevInRing()); }
    const SwShellTableCursor* GetPrev() const { return dynamic_cast<SwShellTableCursor const *>(GetPrevInRing()); }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
