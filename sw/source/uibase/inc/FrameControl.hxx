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
class SwPageFrm;
class SwFrm;
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

    virtual const SwFrm* GetFrame() = 0;
    virtual SwEditWin*   GetEditWin() = 0;
};

class SwFrameControl
{
    VclPtr<vcl::Window> mxWindow;
    ISwFrameControl    *mpIFace;
public:
    SwFrameControl( const VclPtr<vcl::Window> &pWindow );
    virtual ~SwFrameControl();

    const SwFrm* GetFrame()   { return mpIFace->GetFrame(); }
    SwEditWin*   GetEditWin() { return mpIFace->GetEditWin(); }
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
    const SwFrm*          m_pFrm;

protected:
    virtual ~SwFrameMenuButtonBase() { disposeOnce(); }
    virtual void dispose() SAL_OVERRIDE;

public:
    SwFrameMenuButtonBase( SwEditWin* pEditWin, const SwFrm* pFrm );

    virtual const SwFrm* GetFrame()   SAL_OVERRIDE { return m_pFrm; }
    virtual SwEditWin*   GetEditWin() SAL_OVERRIDE { return m_pEditWin; }
    const SwPageFrm*     GetPageFrame();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
