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
#include "FrameControl.hxx"
#include <vcl/timer.hxx>
#include <optional>

class Menu;
class SwPageFrame;

/** Class for the page break control window.

    This control shows a line indicating a manual page break and a
    button providing a few actions on that page break.
  */
class SwPageBreakWin : public SwFrameMenuButtonBase
{
    std::unique_ptr<weld::MenuButton> m_xMenuButton;
    VclPtr<vcl::Window>   m_pLine;
    bool                  m_bIsAppearing;
    int                   m_nFadeRate;
    int                   m_nDelayAppearing; ///< Before we show the control, let it transparent for a few timer ticks to avoid appearing with every mouse over.
    Timer                 m_aFadeTimer;
    bool                  m_bDestroyed;

    std::optional<Point> m_xMousePt;

public:
    SwPageBreakWin( SwEditWin* pEditWin, const SwFrame *pFrame );
    virtual ~SwPageBreakWin() override;
    virtual void dispose() override;

    void execute(const OString& rIdent);
    void UpdatePosition(const std::optional<Point>& xEvtPt = std::optional<Point>());

    virtual void ShowAll( bool bShow ) override;
    virtual bool Contains( const Point &rDocPt ) const override;

    void SetReadonly( bool bReadonly ) override;

    void Fade( bool bFadeIn );

    virtual FactoryFunction GetUITestFactory() const override;

private:
    DECL_LINK( FadeHandler, Timer *, void );
    /// Hide the button when the menu is toggled closed, e.g by clicking outside
    DECL_LINK(ToggleHdl, weld::ToggleButton&, void);
    DECL_LINK(SelectHdl, const OString&, void);
    void PaintButton();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
