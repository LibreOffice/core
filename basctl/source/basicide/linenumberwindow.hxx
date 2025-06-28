/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/window.hxx>

namespace basctl
{
class ModulWindow;

class LineNumberWindow : public vcl::Window
{
private:
    VclPtr<ModulWindow> m_pModulWindow;
    int m_nWidth;
    tools::Long m_nCurYOffset;
    int m_nBaseWidth;
    Color m_FontColor;
    Color m_HighlightColor;
    virtual void DataChanged(DataChangedEvent const& rDCEvt) override;

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

public:
    LineNumberWindow(vcl::Window* pParent, ModulWindow* pModulWin);
    virtual ~LineNumberWindow() override;
    virtual void dispose() override;

    void DoScroll(tools::Long nVertScroll);

    void SyncYOffset();
    tools::Long& GetCurYOffset() { return m_nCurYOffset; }

    int GetWidth() const { return m_nWidth; }
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
