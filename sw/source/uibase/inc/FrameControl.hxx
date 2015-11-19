/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FRAMECONTROL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FRAMECONTROL_HXX

class SwEditWin;
class SwPageFrame;
class SwFrame;
class Point;

/// Abstract interface to be implemented by writer FrameControls
class ISwFrameControl
{
public:
    virtual ~ISwFrameControl();
    virtual void SetReadonly( bool bReadonly ) = 0;
    virtual void ShowAll( bool bShow ) = 0;

    /// Returns true if the point is inside the control.
    virtual bool Contains( const Point &rDocPt ) const = 0;

    virtual const SwFrame* GetFrame() = 0;
    virtual SwEditWin*   GetEditWin() = 0;
};

class SwFrameControl
{
    VclPtr<vcl::Window> mxWindow;
    ISwFrameControl    *mpIFace;
public:
    SwFrameControl( const VclPtr<vcl::Window> &pWindow );
    virtual ~SwFrameControl();

    vcl::Window* GetWindow()  { return mxWindow.get(); }

    void SetReadonly( bool bReadonly ) { mpIFace->SetReadonly( bReadonly ); }
    void ShowAll( bool bShow )         { mpIFace->ShowAll( bShow ); }
    bool Contains( const Point &rDocPt ) const { return mpIFace->Contains( rDocPt ); }
};

#include <vcl/menubtn.hxx>

/** Class sharing some MenuButton code
  */
class SwFrameMenuButtonBase : public MenuButton, public ISwFrameControl
{
    VclPtr<SwEditWin>     m_pEditWin;
    const SwFrame*          m_pFrame;

protected:
    virtual ~SwFrameMenuButtonBase() { disposeOnce(); }
    virtual void dispose() override;

public:
    SwFrameMenuButtonBase( SwEditWin* pEditWin, const SwFrame* pFrame );

    virtual const SwFrame* GetFrame()   override { return m_pFrame; }
    virtual SwEditWin*   GetEditWin() override { return m_pEditWin; }
    const SwPageFrame*     GetPageFrame();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
