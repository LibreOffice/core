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
#include <vcl/weld.hxx>

class VCL_DLLPUBLIC InterimItemWindow : public Control
{
public:
    virtual ~InterimItemWindow() override;
    virtual void dispose() override;

    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;
    virtual void GetFocus() override;

protected:
    InterimItemWindow(vcl::Window* pParent, const OUString& rUIXMLDescription, const OString& rID);

    // pass keystrokes from our child window through this to handle focus changes correctly
    // returns true if keystroke is consumed
    bool ChildKeyInput(const KeyEvent& rKEvt);

    std::unique_ptr<weld::Builder> m_xBuilder;
    VclPtr<vcl::Window> m_xVclContentArea;
    std::unique_ptr<weld::Container> m_xContainer;

private:
    virtual void ImplPaintToDevice(OutputDevice* pTargetOutDev, const Point& rPos) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
