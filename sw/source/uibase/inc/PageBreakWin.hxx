/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_PAGEBREAKWIN_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_PAGEBREAKWIN_HXX

#include "edtwin.hxx"
#include "DashedLine.hxx"
#include "FrameControl.hxx"
#include <vcl/timer.hxx>
#include <optional>

class SwPageBreakWin;

/** Class for the page break control window.

    This control shows a line indicating a manual page break and a
    button providing a few actions on that page break.
  */
class SwBreakDashedLine final : public SwDashedLine, public ISwFrameControl
{
private:
    VclPtr<SwPageBreakWin> m_pWin;
    VclPtr<SwEditWin> m_pEditWin;
    std::optional<Point> m_xMousePt;
    ::tools::Rectangle m_aBtnRect;
    const SwFrame* m_pFrame;

    SwPageBreakWin& GetOrCreateWin();

public:
    SwBreakDashedLine(SwEditWin* pEditWin, const SwFrame *pFrame);

    virtual ~SwBreakDashedLine() override { disposeOnce(); }
    virtual void dispose() override { m_pWin.disposeAndClear(); m_pEditWin.clear(); SwDashedLine::dispose(); }

    virtual void MouseMove(const MouseEvent& rMEvt) override;

    virtual const SwFrame* GetFrame() override { return m_pFrame; }
    virtual SwEditWin* GetEditWin() override { return m_pEditWin; }
    virtual void ShowAll(bool bShow) override;
    virtual bool Contains(const Point &rDocPt) const override;
    virtual void SetReadonly(bool bReadonly) override;

    void execute(std::u16string_view rIdent);

    virtual FactoryFunction GetUITestFactory() const override;

    void UpdatePosition(const std::optional<Point>& xEvtPt = std::optional<Point>());
    void DestroyWin();
};

class SwPageBreakWin final : public InterimItemWindow
{
    std::unique_ptr<weld::MenuButton> m_xMenuButton;
    VclPtr<SwBreakDashedLine> m_pLine;
    VclPtr<SwEditWin>     m_pEditWin;
    VclPtr<VirtualDevice> m_xVirDev;
    const SwFrame*        m_pFrame;
    bool                  m_bIsAppearing;
    int                   m_nFadeRate;
    int                   m_nDelayAppearing; ///< Before we show the control, let it transparent for a few timer ticks to avoid appearing with every mouse over.
    Timer                 m_aFadeTimer;
    bool                  m_bDestroyed;

public:
    SwPageBreakWin(SwBreakDashedLine* pLine, SwEditWin* pEditWin, const SwFrame *pFrame);
    virtual ~SwPageBreakWin() override;
    virtual void dispose() override;

    void Fade( bool bFadeIn );

    void SetRectanglePixel(const ::tools::Rectangle& rRect);

private:
    DECL_LINK( FadeHandler, Timer *, void );
    /// Hide the button when the menu is toggled closed, e.g by clicking outside
    DECL_LINK(ToggleHdl, weld::Toggleable&, void);
    DECL_LINK(SelectHdl, const OUString&, void);
    void PaintButton();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
