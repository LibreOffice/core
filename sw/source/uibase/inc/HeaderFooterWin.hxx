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

#include <FrameControl.hxx>

#include <vcl/menubtn.hxx>

/** Class for the header and footer separator control window.

    This control is showing the header / footer style name and provides
    a few useful actions to the user.
  */
class SwHeaderFooterWin : public MenuButton, public SwFrameControl
{
    OUString         m_sLabel;
    bool                  m_bIsHeader;
    PopupMenu*            m_pPopupMenu;
    vcl::Window*               m_pLine;
    bool                  m_bIsAppearing;
    int                   m_nFadeRate;
    Timer                 m_aFadeTimer;

public:
    SwHeaderFooterWin( SwEditWin* pEditWin, const SwPageFrm* pPageFrm, bool bHeader );
    virtual ~SwHeaderFooterWin( );

    void SetOffset( Point aOffset, long nXLineStart, long nXLineEnd );

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void Select( ) SAL_OVERRIDE;

    virtual void ShowAll( bool bShow ) SAL_OVERRIDE;
    virtual bool Contains( const Point &rDocPt ) const SAL_OVERRIDE;

    bool IsHeader() { return m_bIsHeader; };
    bool IsEmptyHeaderFooter( );
    const SwPageFrm* GetPageFrame( );

    void ExecuteCommand(sal_uInt16 nSlot);

    void SetReadonly( bool bReadonly ) SAL_OVERRIDE;

private:
    DECL_LINK( FadeHandler, void * );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
