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

#include <FrameControl.hxx>

#include <vcl/menubtn.hxx>

class SwPageFrm;

/** Class for the page break control window.

    This control shows a line indicating a manual page break and a
    button providing a few actions on that page break.
  */
class SwPageBreakWin : public MenuButton, public SwFrameControl
{
    PopupMenu*            m_pPopupMenu;
    vcl::Window*               m_pLine;
    bool                  m_bIsAppearing;
    int                   m_nFadeRate;
    int                   m_nDelayAppearing; ///< Before we show the control, let it transparent for a few timer ticks to avoid appearing with every mouse over.
    Timer                 m_aFadeTimer;
    bool                  m_bDestroyed;

    const Point*          m_pMousePt;

public:
    SwPageBreakWin( SwEditWin* pEditWin, const SwPageFrm* pPageFrm );
    virtual ~SwPageBreakWin( );

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void Select( ) SAL_OVERRIDE;
    virtual void MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void Activate( ) SAL_OVERRIDE;

    void UpdatePosition( const Point* pEvtPt = NULL );

    virtual void ShowAll( bool bShow ) SAL_OVERRIDE;
    virtual bool Contains( const Point &rDocPt ) const SAL_OVERRIDE;

    const SwPageFrm* GetPageFrame( );

    void SetReadonly( bool bReadonly ) SAL_OVERRIDE;

    void Fade( bool bFadeIn );

private:
    /// Hide the button (used when the popup menu is closed by clicking outside)
    DECL_LINK( HideHandler, void * );
    DECL_LINK( FadeHandler, void * );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
