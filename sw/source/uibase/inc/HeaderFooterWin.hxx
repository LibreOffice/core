/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_HEADERFOOTERWIN_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_HEADERFOOTERWIN_HXX

#include "FrameControl.hxx"
#include <vcl/builder.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/timer.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

/**
 *  Button painter helper class used to paint a runtime button positioned to a writer frame.
 *  See header/footer button.
 */
class SwFrameButtonPainter
{
public:

    static void PaintButton(drawinglayer::primitive2d::Primitive2DContainer& rSeq,
                            const tools::Rectangle& rRect, bool bOnTop);
};

/** Class for the header and footer separator control window.

    This control is showing the header / footer style name and provides
    a few useful actions to the user.
  */
class SwHeaderFooterWin : public SwFrameMenuButtonBase
{
    VclBuilder            m_aBuilder;
    OUString              m_sLabel;
    bool const            m_bIsHeader;
    VclPtr<PopupMenu>     m_pPopupMenu;
    VclPtr<vcl::Window>   m_pLine;
    bool                  m_bIsAppearing;
    int                   m_nFadeRate;
    Timer                 m_aFadeTimer;

public:
    SwHeaderFooterWin( SwEditWin *pEditWin, const SwFrame *pFrame, bool bHeader );
    virtual ~SwHeaderFooterWin( ) override;
    virtual void dispose() override;

    void SetOffset( Point aOffset, long nXLineStart, long nXLineEnd );

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void Select( ) override;

    virtual void ShowAll( bool bShow ) override;
    virtual bool Contains( const Point &rDocPt ) const override;

    bool IsHeader() { return m_bIsHeader; };
    bool IsEmptyHeaderFooter( );

    void ExecuteCommand(const OString &rIdent);

    void SetReadonly( bool bReadonly ) override;

private:
    DECL_LINK( FadeHandler, Timer *, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
