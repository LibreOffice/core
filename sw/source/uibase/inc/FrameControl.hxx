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

#include <vcl/InterimItemWindow.hxx>
#include <vcl/virdev.hxx>

#include "edtwin.hxx"

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

    /// Returns true if the control has focus
    virtual bool IsFocused() const = 0;

    virtual const SwFrame* GetFrame() = 0;
    virtual SwEditWin*   GetEditWin() = 0;
};

class SwFrameControl final
{
    VclPtr<vcl::Window> mxWindow;
    ISwFrameControl    *mpIFace;
public:
    SwFrameControl( const VclPtr<vcl::Window> &pWindow );
    ~SwFrameControl();

    vcl::Window* GetWindow()  { return mxWindow.get(); }
    ISwFrameControl* GetIFacePtr() { return mpIFace; }

    void SetReadonly( bool bReadonly ) { mpIFace->SetReadonly( bReadonly ); }
    void ShowAll( bool bShow )         { mpIFace->ShowAll( bShow ); }
    bool Contains( const Point &rDocPt ) const { return mpIFace->Contains( rDocPt ); }
    bool HasFocus() const { return mpIFace->IsFocused(); }
};

/** Class sharing some MenuButton code
  */
class SwFrameMenuButtonBase : public InterimItemWindow, public ISwFrameControl
{
protected:
    VclPtr<VirtualDevice> m_xVirDev;
private:
    VclPtr<SwEditWin>     m_pEditWin;
    const SwFrame*          m_pFrame;

protected:
    virtual ~SwFrameMenuButtonBase() override { disposeOnce(); }
    virtual void dispose() override;

    void SetVirDevFont();

public:
    SwFrameMenuButtonBase(SwEditWin* pEditWin, const SwFrame* pFrame,
                          const OUString& rUIXMLDescription, const OString& rID);

    virtual const SwFrame* GetFrame()   override { return m_pFrame; }
    virtual SwEditWin*   GetEditWin() override { return m_pEditWin; }
    virtual bool IsFocused() const override { return ControlHasFocus(); }
    const SwPageFrame*     GetPageFrame() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
