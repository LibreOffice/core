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

#include "FrameControl.hxx"
#include <vcl/builder.hxx>
#include <vcl/menubtn.hxx>
#include <boost/optional.hpp>

class Menu;
class SwPageFrame;

/** Class for the page break control window.

    This control shows a line indicating a manual page break and a
    button providing a few actions on that page break.
  */
class SwPageBreakWin : public SwFrameMenuButtonBase
{
    VclBuilder            m_aBuilder;
    VclPtr<PopupMenu>     m_pPopupMenu;
    VclPtr<vcl::Window>   m_pLine;
    bool                  m_bIsAppearing;
    int                   m_nFadeRate;
    int                   m_nDelayAppearing; ///< Before we show the control, let it transparent for a few timer ticks to avoid appearing with every mouse over.
    Timer                 m_aFadeTimer;
    bool                  m_bDestroyed;

    boost::optional<Point> m_xMousePt;

public:
    SwPageBreakWin( SwEditWin* pEditWin, const SwFrame *pFrame );
    virtual ~SwPageBreakWin() override;
    virtual void dispose() override;

    virtual void Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect ) override;
    virtual void Select( ) override;
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void Activate( ) override;

    void UpdatePosition(const boost::optional<Point>& xEvtPt = boost::optional<Point>());

    virtual void ShowAll( bool bShow ) override;
    virtual bool Contains( const Point &rDocPt ) const override;

    void SetReadonly( bool bReadonly ) override;

    void Fade( bool bFadeIn );

private:
    /// Hide the button (used when the popup menu is closed by clicking outside)
    DECL_LINK( HideHandler, Menu *, bool );
    DECL_LINK( FadeHandler, Timer *, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
