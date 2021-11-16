/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

class VCL_DLLPUBLIC InterimItemWindow : public Control
{
public:
    virtual ~InterimItemWindow() override;
    virtual void dispose() override;

    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;
    // throw away cached size request of child so GetOptimalSize will
    // fetch it anew
    void InvalidateChildSizeCache();
    virtual void StateChanged(StateChangedType nStateChange) override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual void GetFocus() override;

    bool ControlHasFocus() const;

    virtual void Draw(OutputDevice* pDevice, const Point& rPos,
                      SystemTextColorFlags nFlags) override;

protected:
    // bAllowCycleFocusOut of true allows focus to be moved out of the Control
    // via tab key into a parent window or sibling window, false means focus
    // remains inside the InterimItemWindow and cycles back to the first child
    // of this control on reaching pass the last child. This is suitable when
    // the Control is the toplevel control and has no siblings or parent
    InterimItemWindow(vcl::Window* pParent, const OUString& rUIXMLDescription, const OString& rID,
                      bool bAllowCycleFocusOut = true, sal_uInt64 nLOKWindowId = 0);

    void InitControlBase(weld::Widget* pWidget);

    // pass keystrokes from our child window through this to handle focus changes correctly
    // returns true if keystroke is consumed
    bool ChildKeyInput(const KeyEvent& rKEvt);

    virtual void Layout();

    // unclip a "SysObj" which is a native window element hosted in a vcl::Window
    // if the SysObj is logically "visible" in the vcl::Window::IsVisible sense but
    // is partially or wholly clipped out due to being overlapped or scrolled out
    // of view. The clip state is flagged as dirty after this and vcl will restore
    // the clip state the next time it evaluates the clip status
    void UnclipVisibleSysObj();

    std::unique_ptr<weld::Builder> m_xBuilder;
    VclPtr<vcl::Window> m_xVclContentArea;
    std::unique_ptr<weld::Container> m_xContainer;
    weld::Widget* m_pWidget;

private:
    Idle m_aLayoutIdle;

    void StartIdleLayout();

    DECL_DLLPRIVATE_LINK(DoLayout, Timer*, void);

    virtual void ImplPaintToDevice(::OutputDevice* pTargetOutDev, const Point& rPos) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
